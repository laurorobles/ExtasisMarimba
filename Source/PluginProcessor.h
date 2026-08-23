#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "DSP/MarimbaSynthEngine.h"
#include "LicenseManager.h"
#include <functional>
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

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }
    ExtasisDSP::MarimbaSynthEngine& getSynthEngine() { return synthEngine; }

    // Audition Trigger (Thread-safe)
    void triggerAuditionNote(int noteNumber = 60, float velocity = 1.0f);
    void releaseAuditionNote(int noteNumber = 60);

    // Licensing & Demo Timer
    bool isLicensed() const { return isPluginLicensed; }
    void checkLicenseState();
    bool isDemoExpired() const { return isExpired; }
    int getRemainingDemoSeconds() const { return std::max(0, 600 - (demoSampleCount / (int)currentSampleRate)); }

    // Real-time audio stream callback for visualizer
    std::function<void(const float*, int)> onAudioBlockProcessed;

private:
    juce::AudioProcessorValueTreeState apvts;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    ExtasisDSP::MarimbaSynthEngine synthEngine;

    // Audition Note queue
    struct TriggerEvent { int note; float vel; bool isNoteOn; };
    static constexpr int FIFO_SIZE = 16;
    std::array<TriggerEvent, FIFO_SIZE> triggerFifo;
    std::atomic<int> fifoWriteIdx { 0 };
    std::atomic<int> fifoReadIdx { 0 };

    int currentProgramIndex = 0;
    double currentSampleRate = 44100.0;

    // License tracking
    bool isPluginLicensed = false;
    bool isExpired = false;
    int demoSampleCount = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ExtasisMarimbaAudioProcessor)
};
