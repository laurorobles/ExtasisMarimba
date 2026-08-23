#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <vector>

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
    void pushAudioSamples(const float* samples, int numSamples);
    void triggerStrikeAnimation(float hardness, float decay);

private:
    juce::String currentPatchName = "01: MicroFreak Marimbita";
    juce::String currentParamName = "MODAL SYNTHESIS ACTIVE";
    juce::String currentValueText = "MARIMBA ENGINE READY";
    int readoutTimeoutCounter = 0;

    float animDecay = 0.0f;
    float animHardness = 0.5f;
    float animPhase = 0.0f;

    // Real-time oscilloscope buffer (Like TX81Z / ExtasisDonk)
    static constexpr int scopeBufferSize = 256;
    std::vector<float> scopeBuffer;
    std::vector<float> incomingFifo;
    juce::CriticalSection fifoLock;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MarimbaDisplay)
};

} // namespace ExtasisGUI
