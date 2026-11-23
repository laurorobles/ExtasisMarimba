#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <vector>
#include <functional>

namespace ExtasisGUI
{

class MarimbaDisplay : public juce::Component, public juce::Timer
{
public:
    MarimbaDisplay();
    ~MarimbaDisplay() override;

    void paint(juce::Graphics& g) override;
    void timerCallback() override;

    void setPatchName(const juce::String& name);
    void setParameterReadout(const juce::String& paramName, const juce::String& valueText);
    void setModalParams(float hardness, float overtones, float tube, float buzz);
    void triggerStrikeAnimation(float hardness, float decay);

    // UI-Thread pull callbacks (reads lock-free data from AudioProcessor)
    std::function<void(float* dest, int numSamples)> onPullScopeSamples;
    std::function<float()> onPullOutputRms;
    std::function<int()> onPullActiveVoices;

private:
    juce::String currentPatchName = "01. Chiapas Cachimba Marimba";
    juce::String currentParamName = "PHYSICAL-FM ENGINE";
    juce::String currentValueText = "1:4:10 TUNED BAR READY";
    int readoutTimeoutCounter = 0;

    float animDecay = 0.0f;
    float animPhase = 0.0f;
    float smoothedRms = 0.0f;
    int activeVoices = 0;

    // Live synth parameter weights for modal resonator meters
    float paramHardness = 0.65f;
    float paramOvertones = 0.40f;
    float paramTube = 0.85f;
    float paramBuzz = 0.60f;

    // Real-time oscilloscope buffers (UI thread only — zero locks!)
    static constexpr int scopeBufferSize = 256;
    static constexpr int pullBufferSize = 512;
    std::vector<float> pullBuffer;
    std::vector<float> scopeBuffer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MarimbaDisplay)
};

} // namespace ExtasisGUI
