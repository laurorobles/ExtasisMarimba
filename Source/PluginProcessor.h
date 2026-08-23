#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "DSP/MarimbaSynthEngine.h"
#include <functional>

class ExtasisMarimbaAudioProcessor : public juce::AudioProcessor
{
public:
    ExtasisMarimbaAudioProcessor();
    ~ExtasisMarimbaAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }
    ExtasisDSP::MarimbaSynthEngine& getSynthEngine() { return synthEngine; }

    void loadPreset(int presetIndex);
    int getPresetCount() const { return static_cast<int>(presetNames.size()); }
    juce::String getPresetTitle(int index) const;

    // Real-time audio stream callback for visualizer
    std::function<void(const float*, int)> onAudioBlockProcessed;

private:
    juce::AudioProcessorValueTreeState apvts;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    ExtasisDSP::MarimbaSynthEngine synthEngine;

    int currentPresetIndex = 0;
    std::vector<juce::String> presetNames {
        "01: Concert Marimba",
        "02: Wooden Rosewood Bar",
        "03: African Balafon / Kalimba",
        "04: Glass & Metal Bar",
        "05: Plucked Mallet Synth"
    };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ExtasisMarimbaAudioProcessor)
};
