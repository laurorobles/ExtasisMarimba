#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "LicenseManager.h"
#include "GUI/MarimbaLookAndFeel.h"
#include "GUI/MarimbaDisplay.h"
#include "GUI/MarimbaTriggerButton.h"

class ActivationOverlayComponent : public juce::Component
{
public:
    std::function<void(const juce::String&)> onActivate;
    std::function<void()> onContinueDemo;

    bool isExpired = false;

    juce::TextEditor licenseInput;
    juce::TextButton activateButton;
    juce::TextButton demoButton;
    juce::Label statusLabel;

    ActivationOverlayComponent()
    {
        addAndMakeVisible (licenseInput);
        licenseInput.setMultiLine (false);
        licenseInput.setFont (juce::FontOptions (13.0f, juce::Font::bold));
        licenseInput.setJustification (juce::Justification::centred);
        licenseInput.setTextToShowWhenEmpty ("EXTM-XXXX-XXXX-XXXX-XXXX", juce::Colour(0xff718093));
        licenseInput.setColour (juce::TextEditor::backgroundColourId, juce::Colour (0xff14171a));
        licenseInput.setColour (juce::TextEditor::textColourId, juce::Colours::white);
        licenseInput.setColour (juce::TextEditor::outlineColourId, juce::Colour (0xffffa834));
        licenseInput.setColour (juce::TextEditor::focusedOutlineColourId, juce::Colour (0xffffca68));

        addAndMakeVisible (activateButton);
        activateButton.setButtonText ("ACTIVATE LICENSE");
        activateButton.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff27ae60));
        activateButton.setColour (juce::TextButton::textColourOffId, juce::Colours::white);
        activateButton.onClick = [this]() {
            if (onActivate) onActivate (licenseInput.getText().trim());
        };

        addAndMakeVisible (demoButton);
        demoButton.setButtonText ("CONTINUE IN DEMO MODE");
        demoButton.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff3d3d3d));
        demoButton.setColour (juce::TextButton::textColourOffId, juce::Colours::white);
        demoButton.onClick = [this]() {
            if (onContinueDemo) onContinueDemo();
        };

        addAndMakeVisible (statusLabel);
        statusLabel.setFont (juce::FontOptions (11.5f, juce::Font::bold));
        statusLabel.setJustificationType (juce::Justification::centred);
    }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (juce::Colour (0xee0f141a));

        int modalW = 500;
        int modalH = 260;
        int modalX = (getWidth() - modalW) / 2;
        int modalY = (getHeight() - modalH) / 2;

        g.setColour (juce::Colours::black.withAlpha (0.7f));
        g.fillRoundedRectangle ((float)(modalX + 6), (float)(modalY + 6), (float)modalW, (float)modalH, 12.0f);

        juce::ColourGradient cardGrad (juce::Colour (0xff282c34), (float)modalX, (float)modalY,
                                       juce::Colour (0xff1c2025), (float)modalX, (float)(modalY + modalH), false);
        g.setGradientFill (cardGrad);
        g.fillRoundedRectangle ((float)modalX, (float)modalY, (float)modalW, (float)modalH, 12.0f);

        g.setColour (isExpired ? juce::Colour (0xffff5252).withAlpha (0.9f) : juce::Colour (0xffffa834).withAlpha (0.9f));
        g.drawRoundedRectangle ((float)modalX, (float)modalY, (float)modalW, (float)modalH, 12.0f, 1.5f);

        g.setColour (juce::Colour (0xff21252b));
        g.fillRoundedRectangle ((float)modalX + 1.0f, (float)modalY + 1.0f, (float)modalW - 2.0f, 44.0f, 12.0f);
        g.fillRect ((float)modalX + 1.0f, (float)modalY + 24.0f, (float)modalW - 2.0f, 21.0f);
        g.setColour (juce::Colour (0xff3a3f4b));
        g.drawHorizontalLine (modalY + 45, (float)modalX, (float)(modalX + modalW));

        g.setFont (juce::FontOptions (15.0f, juce::Font::bold));
        g.setColour (isExpired ? juce::Colour (0xffff5252) : juce::Colour (0xffffa834));
        g.drawText ("EXTASIS MARIMBA", modalX + 20, modalY + 12, 180, 22, juce::Justification::left);
        
        g.setFont (juce::FontOptions (12.0f, juce::Font::plain));
        g.setColour (juce::Colour (0xffdcdde1));
        g.drawText (isExpired ? "— Demo Expired" : "— Product Activation", modalX + 185, modalY + 13, 200, 22, juce::Justification::left);

        g.setFont (juce::FontOptions (11.5f, isExpired ? juce::Font::bold : juce::Font::plain));
        g.setColour (isExpired ? juce::Colour (0xffff6b6b) : juce::Colour (0xffc8d6e5));
        g.drawText (isExpired ? "Demo evaluation period has expired (10 minutes).\nEnter your license key to unlock and continue making music:"
                              : "Please enter your 16-character license key to unlock the full version:",
                    modalX + 20, modalY + 54, modalW - 40, 26, juce::Justification::centred);

        g.setFont (juce::FontOptions (9.5f, juce::Font::plain));
        g.setColour (juce::Colour (0xff718093));
        g.drawText ("extasisrecords.bandcamp.com",
                    modalX + 20, modalY + modalH - 24, modalW - 40, 16, juce::Justification::centred);
    }

    void resized() override
    {
        int modalW = 500;
        int modalH = 260;
        int modalX = (getWidth() - modalW) / 2;
        int modalY = (getHeight() - modalH) / 2;

        licenseInput.setBounds (modalX + 45, modalY + 86, modalW - 90, 32);
        if (isExpired)
        {
            activateButton.setBounds (modalX + 120, modalY + 130, modalW - 240, 32);
            demoButton.setVisible (false);
        }
        else
        {
            activateButton.setBounds (modalX + 45, modalY + 130, 195, 32);
            demoButton.setBounds (modalX + 260, modalY + 130, 195, 32);
            demoButton.setVisible (true);
        }
        statusLabel.setBounds (modalX + 30, modalY + 172, modalW - 60, 24);
    }
};

class ExtasisMarimbaAudioProcessorEditor : public juce::AudioProcessorEditor,
                                           public juce::Slider::Listener,
                                           public juce::ComboBox::Listener,
                                           public juce::Timer
{
public:
    explicit ExtasisMarimbaAudioProcessorEditor(ExtasisMarimbaAudioProcessor&);
    ~ExtasisMarimbaAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

    void sliderValueChanged(juce::Slider* slider) override;
    void comboBoxChanged(juce::ComboBox* comboBox) override;

private:
    ExtasisMarimbaAudioProcessor& processorRef;
    ExtasisGUI::MarimbaLookAndFeel customLookAndFeel;

    // Display
    ExtasisGUI::MarimbaDisplay display;

    // Preset Selection & Management
    juce::ComboBox presetBox;
    juce::TextButton prevPresetBtn { "<" };
    juce::TextButton nextPresetBtn { ">" };
    juce::TextButton savePresetBtn { "SAVE" };

    // Logo Trigger Pad
    ExtasisGUI::MarimbaTriggerButton triggerButton;

    // Header Links & License
    juce::HyperlinkButton bandcampLinkBtn { "extasisrecords.bandcamp.com", juce::URL("https://extasisrecords.bandcamp.com") };
    juce::TextButton licenseBadgeButton;
    ActivationOverlayComponent activationOverlay;
    bool isActivated = false;
    bool showActivationModal = false;

    void refreshPresetList();
    void updateLicenseState();
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

    void createKnob(const juce::String& paramId, const juce::String& labelText);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ExtasisMarimbaAudioProcessorEditor)
};
