#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <BinaryData.h>
#include "PluginProcessor.h"
#include "GUI/MarimbaLookAndFeel.h"
#include "GUI/MarimbaDisplay.h"

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
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;

private:
    ExtasisMarimbaAudioProcessor& processorRef;
    ExtasisGUI::MarimbaLookAndFeel customLookAndFeel;

    // Display
    ExtasisGUI::MarimbaDisplay display;

    // Preset Selection
    juce::ComboBox presetBox;
    juce::TextButton prevPresetBtn { "<" };
    juce::TextButton nextPresetBtn { ">" };

    // Trigger & Test Note Controls
    juce::TextButton triggerButton { "⚡ TRIGGER MALLET" };
    juce::ComboBox triggerNoteBox;
    juce::TextButton noteDownBtn { "◀" };
    juce::TextButton noteUpBtn { "▶" };
    juce::ComboBox triggerVelBox;

    int currentTriggerNote = 60; // C4
    float currentTriggerVel = 1.0f;
    juce::Rectangle<int> logoBounds;

    // Logo Image
    juce::Image logoImage;

    // Knobs & Labels
    struct KnobControl
    {
        std::unique_ptr<juce::Slider> slider;
        std::unique_ptr<juce::Label> label;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    };

    std::map<juce::String, KnobControl> controls;

    void createKnob(const juce::String& paramId, const juce::String& labelText, const juce::String& suffix = "");
    void playTriggerNote();
    void stopTriggerNote();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ExtasisMarimbaAudioProcessorEditor)
};
