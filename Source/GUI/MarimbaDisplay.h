#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

namespace ExtasisGUI
{

class MarimbaDisplay : public juce::Component, public juce::Timer
{
public:
    MarimbaDisplay();
    ~MarimbaDisplay() override;

    void paint(juce::Graphics& g) override;
    void timerCallback() override;

    void setPresetName(const juce::String& name);
    void setParameterReadout(const juce::String& paramName, const juce::String& paramValue);
    void triggerStrikeAnimation(float hardness, float decay);

private:
    juce::String currentPreset = "01: MicroFreak Marimbita";
    juce::String activeParam = "READY";
    juce::String activeValue = "--";

    float animDecay = 0.0f;
    float animHardness = 0.5f;
    float animPhase = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MarimbaDisplay)
};

} // namespace ExtasisGUI
