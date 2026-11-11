#include "PluginProcessor.h"
#include "PluginEditor.h"

using FP = juce::AudioParameterFloat;
using Range = juce::NormalisableRange<float>;

juce::AudioProcessorValueTreeState::ParameterLayout HappyDriveProcessor::createLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout l;
    l.add (std::make_unique<FP> (juce::ParameterID { "input", 1 }, "Entrée",           Range (-12.f, 12.f, 0.1f), 0.f));
    l.add (std::make_unique<FP> (juce::ParameterID { "drive", 1 }, "Drive harmonique", Range (0.f, 100.f, 0.1f), 35.f));
    l.add (std::make_unique<FP> (juce::ParameterID { "mix",   1 }, "Mix",              Range (0.f, 100.f, 0.1f), 80.f));
    l.add (std::make_unique<FP> (juce::ParameterID { "even",  1 }, "Paires",           Range (0.f, 100.f, 0.1f), 50.f));
    l.add (std::make_unique<FP> (juce::ParameterID { "odd",   1 }, "Impaires",         Range (0.f, 100.f, 0.1f), 50.f));
    l.add (std::make_unique<FP> (juce::ParameterID { "out",   1 }, "Sortie",           Range (-12.f, 12.f, 0.1f), 0.f));
    l.add (std::make_unique<juce::AudioParameterBool> (juce::ParameterID { "bypass", 1 }, "Bypass", false));
    return l;
}

HappyDriveProcessor::HappyDriveProcessor()
    : AudioProcessor (BusesProperties()
                        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "STATE", createLayout())
{
}

bool HappyDriveProcessor::isBusesLayoutSupported (const BusesLayout& l) const
{
    const auto out = l.getMainOutputChannelSet();
    return (out == juce::AudioChannelSet::mono() || out == juce::AudioChannelSet::stereo())
           && out == l.getMainInputChannelSet();
}

void HappyDriveProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    const auto nch = (size_t) juce::jmax (1, getTotalNumOutputChannels());

    // 4x : évite le repliement (aliasing) créé par la saturation
    oversampler = std::make_unique<juce::dsp::Oversampling<float>> (
        nch, 2, juce::dsp::Oversampling<float>::filterHalfBandFIREquiripple, true, true);
    oversampler->initProcessing ((size_t) samplesPerBlock);
    oversampler->reset();
    setLatencySamples (juce::roundToInt (oversampler->getLatencyInSamples()));

    auto c = juce::dsp::IIR::Coefficients<float>::makeHighPass (sampleRate * 4.0, 25.0f);
    highpass.clear();
    highpass.resize (nch);
    for (auto& f : highpass) { f.coefficients = c; f.reset(); }

    firstBlock = true;
}

void HappyDriveProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    for (int ch = getTotalNumInputChannels(); ch < getTotalNumOutputChannels(); ++ch)
        buffer.clear (ch, 0, buffer.getNumSamples());

    auto get = [this] (const char* id) { return apvts.getRawParameterValue (id)->load(); };
    const bool bypass = get ("bypass") > 0.5f;

    const float cur[6] = {
        bypass ? 1.f : juce::Decibels::decibelsToGain (get ("input")),
        get ("drive") * 0.01f,
        bypass ? 0.f : get ("mix") * 0.01f,
        get ("even")  * 0.01f,
        get ("odd")   * 0.01f,
        bypass ? 1.f : juce::Decibels::decibelsToGain (get ("out"))
    };
    if (firstBlock) { std::copy (cur, cur + 6, prev); firstBlock = false; }

    const int n = buffer.getNumSamples();

    // Entrée
    buffer.applyGainRamp (0, n, prev[0], cur[0]);

    // Étage harmonique (sur-échantillonné x4)
    juce::dsp::AudioBlock<float> block (buffer);
    auto up = oversampler->processSamplesUp (block);
    const auto N = up.getNumSamples();

    for (size_t ch = 0; ch < up.getNumChannels() && ch < highpass.size(); ++ch)
    {
        auto* d = up.getChannelPointer (ch);
        auto& hp = highpass[ch];

        for (size_t i = 0; i < N; ++i)
        {
            const float t = (float) (i + 1) / (float) N;
            auto L = [&] (int k) { return prev[k] + (cur[k] - prev[k]) * t; };

            const float drive = L (1), mix = L (2), ev = L (3), od = L (4);
            const float D = 0.05f + drive * 9.f;
            const float x = d[i];

            // Impaires : saturation symétrique (tanh) → H3, H5, H7…
            const float oddH  = (std::tanh (D * x) / std::tanh (D) - x) * od * 1.2f;
            // Paires : courbe en x² → H2, H4, H6… (DC retiré par le passe-haut)
            const float evenH = 0.8f * std::tanh (0.5f * D * x * x) * ev * 0.8f;

            d[i] = x + mix * hp.processSample (oddH + evenH);
        }
    }
    oversampler->processSamplesDown (block);

    // Sortie
    buffer.applyGainRamp (0, n, prev[5], cur[5]);
    std::copy (cur, cur + 6, prev);
}

juce::AudioProcessorEditor* HappyDriveProcessor::createEditor() { return new HappyDriveEditor (*this); }

void HappyDriveProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    if (auto xml = apvts.copyState().createXml()) copyXmlToBinary (*xml, dest);
}

void HappyDriveProcessor::setStateInformation (const void* data, int size)
{
    if (auto xml = getXmlFromBinary (data, size))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new HappyDriveProcessor(); }
