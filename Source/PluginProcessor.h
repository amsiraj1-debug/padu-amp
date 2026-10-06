#pragma once

#include <JuceHeader.h>

class PaduAmpAudioProcessor final : public juce::AudioProcessor
{
public:
    PaduAmpAudioProcessor();
    ~PaduAmpAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    void updateFilters();

    double currentSampleRate = 44100.0;
    juce::dsp::IIR::Filter<float> lowShelfL, lowShelfR;
    juce::dsp::IIR::Filter<float> midPeakL, midPeakR;
    juce::dsp::IIR::Filter<float> highShelfL, highShelfR;
    juce::dsp::IIR::Filter<float> presenceL, presenceR;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PaduAmpAudioProcessor)
};
