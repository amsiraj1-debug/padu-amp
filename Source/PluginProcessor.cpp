#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

namespace
{
float dbToGain (float db) { return juce::Decibels::decibelsToGain (db); }
}

PaduAmpAudioProcessor::PaduAmpAudioProcessor()
    : AudioProcessor (BusesProperties()
                        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMETERS", createParameterLayout())
{
}

juce::AudioProcessorValueTreeState::ParameterLayout PaduAmpAudioProcessor::createParameterLayout()
{
    using APF = juce::AudioParameterFloat;
    using Range = juce::NormalisableRange<float>;
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add (std::make_unique<APF> (juce::ParameterID { "input", 1 }, "Input", Range (-24.0f, 24.0f, 0.01f), 0.0f, "dB"));
    layout.add (std::make_unique<APF> (juce::ParameterID { "gain", 1 }, "Gain", Range (0.0f, 20.0f, 0.01f), 6.0f));
    layout.add (std::make_unique<APF> (juce::ParameterID { "drive", 1 }, "Drive", Range (0.0f, 1.0f, 0.001f), 0.45f));
    layout.add (std::make_unique<APF> (juce::ParameterID { "bass", 1 }, "Bass", Range (-12.0f, 12.0f, 0.01f), 0.0f, "dB"));
    layout.add (std::make_unique<APF> (juce::ParameterID { "mid", 1 }, "Mid", Range (-12.0f, 12.0f, 0.01f), 0.0f, "dB"));
    layout.add (std::make_unique<APF> (juce::ParameterID { "treble", 1 }, "Treble", Range (-12.0f, 12.0f, 0.01f), 0.0f, "dB"));
    layout.add (std::make_unique<APF> (juce::ParameterID { "presence", 1 }, "Presence", Range (-12.0f, 12.0f, 0.01f), 0.0f, "dB"));
    layout.add (std::make_unique<APF> (juce::ParameterID { "mix", 1 }, "Mix", Range (0.0f, 100.0f, 0.01f), 100.0f, "%"));
    layout.add (std::make_unique<APF> (juce::ParameterID { "output", 1 }, "Output", Range (-24.0f, 12.0f, 0.01f), -3.0f, "dB"));

    return layout;
}

void PaduAmpAudioProcessor::prepareToPlay (double sampleRate, int)
{
    currentSampleRate = sampleRate;
    lowShelfL.reset(); lowShelfR.reset();
    midPeakL.reset();  midPeakR.reset();
    highShelfL.reset(); highShelfR.reset();
    presenceL.reset(); presenceR.reset();
    updateFilters();
}

void PaduAmpAudioProcessor::updateFilters()
{
    const float bass = apvts.getRawParameterValue ("bass")->load();
    const float mid = apvts.getRawParameterValue ("mid")->load();
    const float treble = apvts.getRawParameterValue ("treble")->load();
    const float presence = apvts.getRawParameterValue ("presence")->load();

    auto low = juce::dsp::IIR::Coefficients<float>::makeLowShelf (currentSampleRate, 120.0, 0.707, dbToGain (bass));
    auto midC = juce::dsp::IIR::Coefficients<float>::makePeakFilter (currentSampleRate, 750.0, 0.8, dbToGain (mid));
    auto high = juce::dsp::IIR::Coefficients<float>::makeHighShelf (currentSampleRate, 3500.0, 0.707, dbToGain (treble));
    auto pres = juce::dsp::IIR::Coefficients<float>::makePeakFilter (currentSampleRate, 5200.0, 1.1, dbToGain (presence));

    lowShelfL.coefficients = low; lowShelfR.coefficients = low;
    midPeakL.coefficients = midC; midPeakR.coefficients = midC;
    highShelfL.coefficients = high; highShelfR.coefficients = high;
    presenceL.coefficients = pres; presenceR.coefficients = pres;
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool PaduAmpAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& mainOut = layouts.getMainOutputChannelSet();
    if (mainOut != juce::AudioChannelSet::mono() && mainOut != juce::AudioChannelSet::stereo())
        return false;
    return mainOut == layouts.getMainInputChannelSet();
}
#endif

void PaduAmpAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int inChannels = getTotalNumInputChannels();
    const int outChannels = getTotalNumOutputChannels();
    for (int ch = inChannels; ch < outChannels; ++ch)
        buffer.clear (ch, 0, buffer.getNumSamples());

    updateFilters();

    const float input = dbToGain (apvts.getRawParameterValue ("input")->load());
    const float gain = apvts.getRawParameterValue ("gain")->load();
    const float drive = apvts.getRawParameterValue ("drive")->load();
    const float mix = apvts.getRawParameterValue ("mix")->load() / 100.0f;
    const float output = dbToGain (apvts.getRawParameterValue ("output")->load());

    juce::AudioBuffer<float> dry;
    dry.makeCopyOf (buffer, true);

    const float pre = input * juce::jmap (gain, 0.0f, 20.0f, 1.0f, 12.0f);
    const float shape = 1.0f + drive * 10.0f;
    const float norm = 1.0f / std::tanh (shape);

    for (int ch = 0; ch < inChannels; ++ch)
    {
        auto* data = buffer.getWritePointer (ch);
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            float x = data[i] * pre;
            x = std::tanh (x * shape) * norm;
            if (ch == 0)
            {
                x = lowShelfL.processSample (x);
                x = midPeakL.processSample (x);
                x = highShelfL.processSample (x);
                x = presenceL.processSample (x);
            }
            else
            {
                x = lowShelfR.processSample (x);
                x = midPeakR.processSample (x);
                x = highShelfR.processSample (x);
                x = presenceR.processSample (x);
            }
            data[i] = x;
        }
    }

    for (int ch = 0; ch < inChannels; ++ch)
    {
        auto* wet = buffer.getWritePointer (ch);
        const auto* d = dry.getReadPointer (ch);
        for (int i = 0; i < buffer.getNumSamples(); ++i)
            wet[i] = (d[i] * (1.0f - mix) + wet[i] * mix) * output;
    }
}

void PaduAmpAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void PaduAmpAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessorEditor* PaduAmpAudioProcessor::createEditor()
{
    return new PaduAmpAudioProcessorEditor (*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PaduAmpAudioProcessor();
}
