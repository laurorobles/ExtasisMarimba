#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "GUI/MarimbaLookAndFeel.h"
#include "GUI/MarimbaDisplay.h"
#include "GUI/MarimbaTriggerButton.h"
#include <map>

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

    // Header Quick Actions & Open Source Link
    juce::TextButton initBtn { "INIT" };
    juce::TextButton randBtn { "RND" };
    juce::HyperlinkButton ossBadgeLink { "GPLv3 // FREE", juce::URL("https://github.com/laurorobles/ExtasisMarimba") };

    // Preset Selection & Management
    juce::ComboBox presetBox;
    juce::TextButton prevPresetBtn { "<" };
    juce::TextButton nextPresetBtn { ">" };
    juce::TextButton savePresetBtn { "SAVE" };

    // Logo Trigger Pad
    ExtasisGUI::MarimbaTriggerButton triggerButton;

    void refreshPresetList();
    void syncAllControlsFromAPVTS();
    void syncDisplayModalParams();
    juce::String getFormattedValueText(const juce::String& paramId, double val);

    // Knobs & Labels
    struct KnobControl
    {
        std::unique_ptr<juce::Slider> slider;
        std::unique_ptr<juce::Label> label;
        std::unique_ptr<juce::Label> valueLabel;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    };

    std::map<juce::String, KnobControl> controls;

    void createKnob(const juce::String& paramId, const juce::String& labelText, double defaultVal);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ExtasisMarimbaAudioProcessorEditor)
};
