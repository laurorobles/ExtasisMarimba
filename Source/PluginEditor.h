#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <BinaryData.h>
#include "PluginProcessor.h"
#include "GUI/MarimbaLookAndFeel.h"
#include "GUI/MarimbaDisplay.h"
#include "GUI/LogoTriggerComponent.h"

class ExtasisMarimbaAudioProcessorEditor : public juce::AudioProcessorEditor,
                                           public juce::Slider::Listener,
                                           public juce::ComboBox::Listener
{
public:
    explicit ExtasisMarimbaAudioProcessorEditor(ExtasisMarimbaAudioProcessor&);
    ~ExtasisMarimbaAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

    void sliderValueChanged(juce::Slider* slider) override;
    void comboBoxChanged(juce::ComboBox* comboBox) override;

private:
    ExtasisMarimbaAudioProcessor& processorRef;
    ExtasisGUI::MarimbaLookAndFeel customLookAndFeel;

    // Display
    ExtasisGUI::MarimbaDisplay display;

    // Preset Selection
    juce::ComboBox presetBox;
    juce::TextButton prevPresetBtn { "<" };
    juce::TextButton nextPresetBtn { ">" };

    // Center Click & Drag Logo Trigger
    ExtasisGUI::LogoTriggerComponent logoTrigger;
    juce::TextButton noteDownBtn { "◀ -1" };
    juce::TextButton noteUpBtn { "+1 ▶" };
    juce::ComboBox triggerVelBox;

    // Knobs & Labels
    struct KnobControl
    {
        std::unique_ptr<juce::Slider> slider;
        std::unique_ptr<juce::Label> label;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    };

    std::map<juce::String, KnobControl> controls;

    void createKnob(const juce::String& paramId, const juce::String& labelText, const juce::String& suffix = "");

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ExtasisMarimbaAudioProcessorEditor)
};
