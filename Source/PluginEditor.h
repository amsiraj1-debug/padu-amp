#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class PaduLookAndFeel final : public juce::LookAndFeel_V4
{
public:
    PaduLookAndFeel();
    void drawRotarySlider (juce::Graphics&, int x, int y, int width, int height,
                           float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                           juce::Slider&) override;
};

class PaduAmpAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit PaduAmpAudioProcessorEditor (PaduAmpAudioProcessor&);
    ~PaduAmpAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;

    PaduAmpAudioProcessor& processor;
    PaduLookAndFeel lookAndFeel;

    juce::Label title, subtitle;
    juce::Slider input, gain, drive, bass, mid, treble, presence, mix, output;
    juce::Label inputL, gainL, driveL, bassL, midL, trebleL, presenceL, mixL, outputL;

    std::unique_ptr<Attachment> inputA, gainA, driveA, bassA, midA, trebleA, presenceA, mixA, outputA;

    void setupKnob (juce::Slider&, juce::Label&, const juce::String&);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PaduAmpAudioProcessorEditor)
};
