#include "PluginEditor.h"

PaduLookAndFeel::PaduLookAndFeel()
{
    setColour (juce::Slider::textBoxTextColourId, juce::Colour (0xfff3f1eb));
    setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour (juce::Label::textColourId, juce::Colour (0xffd7d4cc));
}

void PaduLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                         float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                                         juce::Slider&)
{
    auto area = juce::Rectangle<float> ((float) x, (float) y, (float) width, (float) height).reduced (7.0f);
    auto radius = juce::jmin (area.getWidth(), area.getHeight()) * 0.5f;
    auto centre = area.getCentre();
    auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    g.setColour (juce::Colour (0xff14171b));
    g.fillEllipse (area);
    g.setColour (juce::Colour (0xff343941));
    g.drawEllipse (area, 2.0f);

    juce::Path arc;
    arc.addCentredArc (centre.x, centre.y, radius - 3.0f, radius - 3.0f, 0.0f,
                       rotaryStartAngle, angle, true);
    g.setColour (juce::Colour (0xffff9b36));
    g.strokePath (arc, juce::PathStrokeType (3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    juce::Path p;
    p.addRoundedRectangle (-2.0f, -radius + 10.0f, 4.0f, radius * 0.42f, 2.0f);
    g.setColour (juce::Colour (0xffffd29c));
    g.fillPath (p, juce::AffineTransform::rotation (angle).translated (centre.x, centre.y));
}

PaduAmpAudioProcessorEditor::PaduAmpAudioProcessorEditor (PaduAmpAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    setLookAndFeel (&lookAndFeel);
    setSize (860, 420);
    setResizable (true, true);
    setResizeLimits (720, 360, 1200, 620);

    title.setText ("PADU AMP", juce::dontSendNotification);
    title.setFont (juce::Font (32.0f, juce::Font::bold));
    title.setColour (juce::Label::textColourId, juce::Colour (0xffffa244));
    addAndMakeVisible (title);

    subtitle.setText ("SATURATED GUITAR AMP", juce::dontSendNotification);
    subtitle.setFont (juce::Font (12.0f));
    subtitle.setColour (juce::Label::textColourId, juce::Colour (0xff8e949c));
    addAndMakeVisible (subtitle);

    setupKnob (input, inputL, "INPUT");
    setupKnob (gain, gainL, "GAIN");
    setupKnob (drive, driveL, "DRIVE");
    setupKnob (bass, bassL, "BASS");
    setupKnob (mid, midL, "MID");
    setupKnob (treble, trebleL, "TREBLE");
    setupKnob (presence, presenceL, "PRESENCE");
    setupKnob (mix, mixL, "MIX");
    setupKnob (output, outputL, "OUTPUT");

    inputA = std::make_unique<Attachment> (processor.apvts, "input", input);
    gainA = std::make_unique<Attachment> (processor.apvts, "gain", gain);
    driveA = std::make_unique<Attachment> (processor.apvts, "drive", drive);
    bassA = std::make_unique<Attachment> (processor.apvts, "bass", bass);
    midA = std::make_unique<Attachment> (processor.apvts, "mid", mid);
    trebleA = std::make_unique<Attachment> (processor.apvts, "treble", treble);
    presenceA = std::make_unique<Attachment> (processor.apvts, "presence", presence);
    mixA = std::make_unique<Attachment> (processor.apvts, "mix", mix);
    outputA = std::make_unique<Attachment> (processor.apvts, "output", output);
}

PaduAmpAudioProcessorEditor::~PaduAmpAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

void PaduAmpAudioProcessorEditor::setupKnob (juce::Slider& s, juce::Label& l, const juce::String& name)
{
    s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 72, 20);
    s.setDoubleClickReturnValue (true, 0.0);
    addAndMakeVisible (s);

    l.setText (name, juce::dontSendNotification);
    l.setJustificationType (juce::Justification::centred);
    l.setFont (juce::Font (12.0f, juce::Font::bold));
    addAndMakeVisible (l);
}

void PaduAmpAudioProcessorEditor::paint (juce::Graphics& g)
{
    juce::ColourGradient bg (juce::Colour (0xff20242a), 0, 0,
                              juce::Colour (0xff0e1013), 0, (float) getHeight(), false);
    g.setGradientFill (bg);
    g.fillAll();

    g.setColour (juce::Colour (0xff2f343b));
    g.drawRoundedRectangle (getLocalBounds().toFloat().reduced (12.0f), 16.0f, 1.2f);

    auto ampPanel = juce::Rectangle<float> (24.0f, 82.0f, (float) getWidth() - 48.0f, (float) getHeight() - 106.0f);
    g.setColour (juce::Colour (0xff171a1f));
    g.fillRoundedRectangle (ampPanel, 14.0f);
    g.setColour (juce::Colour (0xff2c3138));
    g.drawRoundedRectangle (ampPanel, 14.0f, 1.0f);

    g.setColour (juce::Colour (0xffff9b36).withAlpha (0.5f));
    g.fillRect (24, 74, getWidth() - 48, 2);
}

void PaduAmpAudioProcessorEditor::resized()
{
    title.setBounds (28, 18, 260, 38);
    subtitle.setBounds (31, 51, 240, 18);

    auto area = getLocalBounds().reduced (30, 92);
    const int gap = 8;
    const int columns = 9;
    const int w = (area.getWidth() - gap * (columns - 1)) / columns;
    juce::Slider* knobs[] = { &input, &gain, &drive, &bass, &mid, &treble, &presence, &mix, &output };
    juce::Label* labels[] = { &inputL, &gainL, &driveL, &bassL, &midL, &trebleL, &presenceL, &mixL, &outputL };

    for (int i = 0; i < columns; ++i)
    {
        int x = area.getX() + i * (w + gap);
        labels[i]->setBounds (x, area.getY(), w, 24);
        knobs[i]->setBounds (x, area.getY() + 24, w, juce::jmin (190, area.getHeight() - 24));
    }
}
