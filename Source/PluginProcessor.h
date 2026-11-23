#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "DSP/MarimbaSynthEngine.h"
#include <array>
#include <atomic>

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

    // Presets & Programs
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    static juce::StringArray getFactoryPresetNames();
    juce::StringArray getAllPresetNames();
    void loadPreset(int presetIndex);
    bool saveUserPreset(const juce::String& presetName);
    bool deleteUserPreset(const juce::String& presetName);
    juce::File getPresetsDirectory() const;

    // Quick Patch Actions
    void initParameters();
    void randomizeParameters();

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }
    MarimbaSynthEngine& getSynthEngine() { return synthEngine; }

    // Audition Trigger (Lock-Free Thread-Safe)
    void triggerAuditionNote(int noteNumber = 60, float velocity = 1.0f);
    void releaseAuditionNote(int noteNumber = 60);

    // Lock-Free Real-Time Visualizer Data (Audio Thread -> UI Thread)
    static constexpr int SCOPE_RING_SIZE = 2048;
    void copyScopeSamples(float* dest, int numSamples) const;
    float getOutputRms() const { return outputRmsAtomic.load(std::memory_order_relaxed); }
    int getActiveVoiceCount() const { return synthEngine.getActiveVoiceCount(); }

private:
    juce::AudioProcessorValueTreeState apvts;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    MarimbaSynthEngine synthEngine;

    // Lock-Free Audition Note FIFO
    struct TriggerEvent { int note; float vel; bool isNoteOn; };
    static constexpr int FIFO_SIZE = 32;
    std::array<TriggerEvent, FIFO_SIZE> triggerFifo;
    std::atomic<int> fifoWriteIdx { 0 };
    std::atomic<int> fifoReadIdx { 0 };

    // Lock-Free Scope Ring Buffer (Audio Thread writes, UI Thread reads)
    std::array<float, SCOPE_RING_SIZE> scopeRingBuffer {};
    std::atomic<int> scopeWritePos { 0 };
    std::atomic<float> outputRmsAtomic { 0.0f };

    int currentProgramIndex = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ExtasisMarimbaAudioProcessor)
};
