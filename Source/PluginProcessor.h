#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>

class HappyDriveProcessor : public juce::AudioProcessor
{
public:
    HappyDriveProcessor();

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout&) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Happy Drive Preamps"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    juce::AudioProcessorValueTreeState apvts;

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();

    std::unique_ptr<juce::dsp::Oversampling<float>> oversampler;
    std::vector<juce::dsp::IIR::Filter<float>> highpass; // retire le DC des harmoniques paires
    float prev[6] {};  // entrée, drive, mix, paires, impaires, sortie
    bool firstBlock = true;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HappyDriveProcessor)
};
