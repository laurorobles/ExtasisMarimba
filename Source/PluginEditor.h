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
        licenseInput.setFont (juce::FontOptions (juce::Font::getDefaultMonospacedFontName(), 14.0f, juce::Font::bold));
        licenseInput.setJustification (juce::Justification::centred);
        licenseInput.setTextToShowWhenEmpty ("EXTM-XXXX-XXXX-XXXX-XXXX", juce::Colour(0xff718093));
        licenseInput.setColour (juce::TextEditor::backgroundColourId, juce::Colour (0xff101316));
        licenseInput.setColour (juce::TextEditor::textColourId, juce::Colour (0xffffca68));
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
        demoButton.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff3a424e));
        demoButton.setColour (juce::TextButton::textColourOffId, juce::Colours::white);
        demoButton.onClick = [this]() {
            if (onContinueDemo) onContinueDemo();
        };

        addAndMakeVisible (statusLabel);
        statusLabel.setFont (juce::FontOptions (juce::Font::getDefaultMonospacedFontName(), 11.0f, juce::Font::bold));
        statusLabel.setJustificationType (juce::Justification::centred);
    }

    void paint (juce::Graphics& g) override
    {
        // 1. Deep Dimming Scrim Backdrop
        g.fillAll (juce::Colour (0xf6080a0d));

        int modalW = 540;
        int modalH = 280;
        int modalX = (getWidth() - modalW) / 2;
        int modalY = (getHeight() - modalH) / 2;

        // 2. Drop Shadow
        g.setColour (juce::Colours::black.withAlpha (0.85f));
        g.fillRoundedRectangle ((float)(modalX + 6), (float)(modalY + 6), (float)modalW, (float)modalH, 12.0f);

        // 3. Card Body Gradient
        juce::ColourGradient cardGrad (juce::Colour (0xff242830), (float)modalX, (float)modalY,
                                       juce::Colour (0xff15181d), (float)modalX, (float)(modalY + modalH), false);
        g.setGradientFill (cardGrad);
        g.fillRoundedRectangle ((float)modalX, (float)modalY, (float)modalW, (float)modalH, 12.0f);

        // Glowing border (Red for expired, Amber Gold for standard activation)
        g.setColour (isExpired ? juce::Colour (0xffff4757).withAlpha (0.95f) : juce::Colour (0xffffa834).withAlpha (0.95f));
        g.drawRoundedRectangle ((float)modalX, (float)modalY, (float)modalW, (float)modalH, 12.0f, 1.5f);

        // 4. Modal Header Strip
        g.setColour (juce::Colour (0xff1a1d23));
        g.fillRoundedRectangle ((float)modalX + 1.0f, (float)modalY + 1.0f, (float)modalW - 2.0f, 44.0f, 12.0f);
        g.fillRect ((float)modalX + 1.0f, (float)modalY + 24.0f, (float)modalW - 2.0f, 21.0f);
        g.setColour (juce::Colour (0xff333842));
        g.drawHorizontalLine (modalY + 45, (float)modalX, (float)(modalX + modalW));

        g.setFont (juce::FontOptions (juce::Font::getDefaultMonospacedFontName(), 15.0f, juce::Font::bold));
        g.setColour (isExpired ? juce::Colour (0xffff4757) : juce::Colour (0xffffa834));
        g.drawText ("EXTASIS MARIMBA", modalX + 24, modalY + 12, 175, 22, juce::Justification::left);
        
        g.setFont (juce::FontOptions (juce::Font::getDefaultMonospacedFontName(), 11.5f, juce::Font::bold));
        g.setColour (isExpired ? juce::Colour (0xffff6b81) : juce::Colour (0xffdcdde1));
        g.drawText (isExpired ? "// DEMO EVALUATION EXPIRED" : "// PRODUCT ACTIVATION", modalX + 195, modalY + 13, 260, 22, juce::Justification::left);

        // 5. Explanatory Body Text
        g.setFont (juce::FontOptions (11.5f, isExpired ? juce::Font::bold : juce::Font::plain));
        g.setColour (isExpired ? juce::Colour (0xffffa4a4) : juce::Colour (0xffc8d6e5));
        g.drawFittedText (isExpired ? "Evaluation period has expired (10 minutes).\nEnter your 16-character license key to unlock full version:"
                                    : "Please enter your 16-character license key to register Extasis Marimba:",
                          modalX + 24, modalY + 54, modalW - 48, 36, juce::Justification::centred, 2);

        // 6. Footer Link
        g.setFont (juce::FontOptions (juce::Font::getDefaultMonospacedFontName(), 9.5f, juce::Font::plain));
        g.setColour (juce::Colour (0xff718093));
        g.drawText ("extasisrecords.bandcamp.com",
                    modalX + 20, modalY + modalH - 24, modalW - 40, 16, juce::Justification::centred);
    }

    void resized() override
    {
        int modalW = 540;
        int modalH = 280;
        int modalX = (getWidth() - modalW) / 2;
        int modalY = (getHeight() - modalH) / 2;

        licenseInput.setBounds (modalX + 45, modalY + 98, modalW - 90, 36);
        if (isExpired)
        {
            activateButton.setBounds (modalX + 145, modalY + 148, modalW - 290, 36);
            demoButton.setVisible (false);
        }
        else
        {
            activateButton.setBounds (modalX + 45, modalY + 148, 215, 36);
            demoButton.setBounds (modalX + 280, modalY + 148, 215, 36);
            demoButton.setVisible (true);
        }
        statusLabel.setBounds (modalX + 30, modalY + 196, modalW - 60, 24);
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
