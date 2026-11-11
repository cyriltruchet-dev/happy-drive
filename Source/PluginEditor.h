#pragma once
#include "PluginProcessor.h"

// Potard rotatif avec cabochon coloré, façon 500-series
class HappyLook : public juce::LookAndFeel_V4
{
public:
    void drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float pos,
                           float startAngle, float endAngle, juce::Slider& s) override
    {
        auto b = juce::Rectangle<float> ((float) x, (float) y, (float) w, (float) h).reduced (6.f);
        const float r = juce::jmin (b.getWidth(), b.getHeight()) * 0.5f;
        const auto c = b.getCentre();
        const auto cap = s.findColour (juce::Slider::thumbColourId);

        g.setColour (juce::Colour (0xff1b2733));
        g.fillEllipse (c.x - r - 4.f, c.y - r - 4.f, 2.f * (r + 4.f), 2.f * (r + 4.f));

        g.setGradientFill (juce::ColourGradient (cap.brighter (0.5f), c.x - r * 0.4f, c.y - r * 0.5f,
                                                 cap.darker (0.5f),   c.x + r * 0.6f, c.y + r * 0.8f, true));
        g.fillEllipse (c.x - r, c.y - r, 2.f * r, 2.f * r);

        const float a = startAngle + pos * (endAngle - startAngle);
        g.setColour (juce::Colour (0xff16212b));
        g.drawLine (juce::Line<float> (c.getPointOnCircumference (r * 0.35f, a),
                                       c.getPointOnCircumference (r * 0.88f, a)), 4.f);
    }
};

class HappyDriveEditor : public juce::AudioProcessorEditor
{
public:
    explicit HappyDriveEditor (HappyDriveProcessor&);
    ~HappyDriveEditor() override;
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    struct Knob
    {
        juce::Slider slider;
        juce::Label label;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attach;
    };

    HappyLook look;
    std::array<Knob, 6> knobs;
    juce::ToggleButton bypass { "Bypass" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttach;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HappyDriveEditor)
};
