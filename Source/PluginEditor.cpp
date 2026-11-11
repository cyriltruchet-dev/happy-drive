#include "PluginEditor.h"

namespace
{
    struct Def { const char* id; const char* label; juce::uint32 cap; const char* suffix; };
    const Def defs[6] = {
        { "input", "Entrée",            0xffc9d1d8, " dB" },
        { "drive", "Drive harmonique",  0xffffc933, " %"  },
        { "mix",   "Mix",               0xffc9d1d8, " %"  },
        { "even",  "Paires",            0xffff7a3d, " %"  },
        { "odd",   "Impaires",          0xff2fd0c0, " %"  },
        { "out",   "Sortie",            0xffc9d1d8, " dB" }
    };
}

HappyDriveEditor::HappyDriveEditor (HappyDriveProcessor& p) : AudioProcessorEditor (&p)
{
    for (size_t i = 0; i < knobs.size(); ++i)
    {
        auto& k = knobs[i];
        k.slider.setSliderStyle (juce::Slider::RotaryVerticalDrag);
        k.slider.setRotaryParameters (juce::MathConstants<float>::pi * 1.25f,
                                      juce::MathConstants<float>::pi * 2.75f, true);
        k.slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 80, 20);
        k.slider.setTextValueSuffix (defs[i].suffix);
        k.slider.setNumDecimalPlacesToDisplay (1);
        k.slider.setColour (juce::Slider::thumbColourId, juce::Colour (defs[i].cap));
        k.slider.setColour (juce::Slider::textBoxTextColourId, juce::Colour (0xffa9bac8));
        k.slider.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        k.slider.setLookAndFeel (&look);
        k.slider.setDoubleClickReturnValue (true, p.apvts.getParameter (defs[i].id)->convertFrom0to1 (
                                                      p.apvts.getParameter (defs[i].id)->getDefaultValue()));
        addAndMakeVisible (k.slider);

        k.label.setText (defs[i].label, juce::dontSendNotification);
        k.label.setJustificationType (juce::Justification::centred);
        k.label.setColour (juce::Label::textColourId, juce::Colour (0xffe9eff4));
        k.label.setFont (juce::FontOptions (16.f, juce::Font::bold));
        addAndMakeVisible (k.label);

        k.attach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (p.apvts, defs[i].id, k.slider);
    }

    bypass.setColour (juce::ToggleButton::textColourId, juce::Colour (0xffe9eff4));
    bypass.setColour (juce::ToggleButton::tickColourId, juce::Colour (0xffffc933));
    addAndMakeVisible (bypass);
    bypassAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (p.apvts, "bypass", bypass);

    setSize (460, 440);
}

HappyDriveEditor::~HappyDriveEditor()
{
    for (auto& k : knobs) k.slider.setLookAndFeel (nullptr);
}

void HappyDriveEditor::paint (juce::Graphics& g)
{
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff2a3b4c), 0.f, 0.f,
                                             juce::Colour (0xff2a3b4c), (float) getWidth(), 0.f, false));
    g.fillAll();
    g.setColour (juce::Colour (0xff33485c));
    g.fillRect (getLocalBounds().reduced (60, 0));

    g.setColour (juce::Colour (0xffe9eff4));
    g.setFont (juce::FontOptions (36.f, juce::Font::bold));
    g.drawText ("Happy Drive", 20, 14, 220, 40, juce::Justification::left);
    g.setColour (juce::Colour (0xffffc933));
    g.setFont (juce::FontOptions (22.f));
    g.drawText ("Preamps", 214, 20, 140, 34, juce::Justification::left);
    g.setColour (juce::Colour (0xffa9bac8));
    g.setFont (juce::FontOptions (14.f));
    g.drawText ("Saturation harmonique paire et impaire", 20, 54, 320, 20, juce::Justification::left);
}

void HappyDriveEditor::resized()
{
    auto area = getLocalBounds().reduced (16);
    auto head = area.removeFromTop (70);
    bypass.setBounds (head.removeFromRight (90).removeFromTop (30));
    area.removeFromTop (10);

    const int cw = area.getWidth() / 3, ch = area.getHeight() / 2;
    for (size_t i = 0; i < knobs.size(); ++i)
    {
        auto cell = juce::Rectangle<int> (area.getX() + (int) (i % 3) * cw,
                                          area.getY() + (int) (i / 3) * ch, cw, ch);
        knobs[i].label.setBounds (cell.removeFromTop (24));
        knobs[i].slider.setBounds (cell);
    }
}
