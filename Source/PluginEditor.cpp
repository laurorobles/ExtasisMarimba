#include "PluginEditor.h"

ExtasisMarimbaAudioProcessorEditor::ExtasisMarimbaAudioProcessorEditor(ExtasisMarimbaAudioProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    setLookAndFeel(&customLookAndFeel);

    // 1. Display Setup & Real-Time Audio Stream
    addAndMakeVisible(display);
    processorRef.onAudioBlockProcessed = [this](const float* samples, int numSamples) {
        display.pushAudioSamples(samples, numSamples);
    };

    // 2. Preset Selector & Management
    refreshPresetList();
    presetBox.setSelectedId(processorRef.getCurrentProgram() + 1, juce::dontSendNotification);
    presetBox.addListener(this);
    addAndMakeVisible(presetBox);

    prevPresetBtn.onClick = [this]() {
        int cur = presetBox.getSelectedId() - 1;
        int count = processorRef.getNumPrograms();
        int next = (cur - 1 + count) % count;
        presetBox.setSelectedId(next + 1, juce::sendNotification);
    };
    addAndMakeVisible(prevPresetBtn);

    nextPresetBtn.onClick = [this]() {
        int cur = presetBox.getSelectedId() - 1;
        int count = processorRef.getNumPrograms();
        int next = (cur + 1) % count;
        presetBox.setSelectedId(next + 1, juce::sendNotification);
    };
    addAndMakeVisible(nextPresetBtn);

    savePresetBtn.onClick = [this]() {
        auto* alert = new juce::AlertWindow("SAVE USER PRESET", "Enter a name for your marimba preset:", juce::AlertWindow::NoIcon);
        alert->addTextEditor("presetName", "My Custom Marimba");
        alert->addButton("SAVE", 1, juce::KeyPress(juce::KeyPress::returnKey));
        alert->addButton("CANCEL", 0, juce::KeyPress(juce::KeyPress::escapeKey));

        alert->enterModalState(true, juce::ModalCallbackFunction::create([this, alert](int result) {
            if (result == 1)
            {
                auto name = alert->getTextEditorContents("presetName").trim();
                if (name.isNotEmpty())
                {
                    processorRef.saveUserPreset(name);
                    refreshPresetList();
                    presetBox.setText("[User] " + name, juce::dontSendNotification);
                    display.setPatchName("[User] " + name);
                    display.setParameterReadout("PRESET SAVED", name);
                }
            }
            delete alert;
        }));
    };
    addAndMakeVisible(savePresetBtn);

    // 3. Logo Trigger Button (Thread-Safe Audition)
    triggerButton.onNoteOn = [this](int midiNote, float vel) {
        processorRef.triggerAuditionNote(midiNote, vel);

        float h = 0.65f;
        float d = 0.55f;
        auto itH = controls.find("hardness");
        auto itD = controls.find("decay");
        if (itH != controls.end() && itH->second.slider != nullptr)
            h = static_cast<float>(itH->second.slider->getValue());
        if (itD != controls.end() && itD->second.slider != nullptr)
            d = static_cast<float>(itD->second.slider->getValue());

        display.triggerStrikeAnimation(h, d);
    };

    triggerButton.onNoteOff = [this](int midiNote) {
        processorRef.releaseAuditionNote(midiNote);
    };

    triggerButton.onStatusChange = [this](const juce::String& title, const juce::String& desc) {
        display.setParameterReadout(title, desc);
    };
    addAndMakeVisible(triggerButton);

    // 4. License Badge & Overlay
    addAndMakeVisible(licenseBadgeButton);
    licenseBadgeButton.onClick = [this]() {
        showActivationModal = true;
        activationOverlay.setVisible(true);
        activationOverlay.isExpired = processorRef.isDemoExpired();
        activationOverlay.resized();
        activationOverlay.repaint();
    };

    addChildComponent(activationOverlay);
    activationOverlay.onActivate = [this](const juce::String& key) {
        if (LicenseManager::validateSerial(key))
        {
            LicenseManager::saveLicense(key);
            processorRef.checkLicenseState();
            updateLicenseState();
            activationOverlay.statusLabel.setText("LICENSE ACTIVATED SUCCESSFULLY!", juce::dontSendNotification);
            activationOverlay.statusLabel.setColour(juce::Label::textColourId, juce::Colour(0xff2ecc71));
            juce::Timer::callAfterDelay(1200, [this]() {
                activationOverlay.setVisible(false);
                showActivationModal = false;
            });
        }
        else
        {
            activationOverlay.statusLabel.setText("INVALID SERIAL KEY. PLEASE TRY AGAIN.", juce::dontSendNotification);
            activationOverlay.statusLabel.setColour(juce::Label::textColourId, juce::Colour(0xffff4757));
        }
    };

    activationOverlay.onContinueDemo = [this]() {
        activationOverlay.setVisible(false);
        showActivationModal = false;
    };

    updateLicenseState();

    // 5. Create all Rotary Controls across 4 Sections
    // Section 1: Mallet & Attack
    createKnob("hardness", "HARDNESS", "%");
    createKnob("noise", "RUBBER NOISE", "%");
    createKnob("click", "CLICK / SNAP", "%");
    createKnob("attack", "ATTACK", "ms");

    // Section 2: Wood & Modal FM
    createKnob("decay", "BAR DECAY", "%");
    createKnob("material", "MATERIAL", "%");
    createKnob("overtones", "OVERTONES", "%");
    createKnob("organic", "ORGANIC DRIFT", "%");

    // Section 3: Resonator & Buzz (La Cachimba)
    createKnob("tube", "TUBE CAVITY", "%");
    createKnob("buzz", "CACHIMBA BUZZ", "%");
    createKnob("buzzVel", "BUZZ DYNAMICS", "%");
    createKnob("cutoff", "SEM FILTER", "Hz");

    // Section 4: Master & FX
    createKnob("spread", "SPREAD", "%");
    createKnob("drive", "WARM DRIVE", "%");
    createKnob("ambience", "AMBIENCE", "%");
    createKnob("volume", "MASTER VOL", "");

    display.setPatchName(presetBox.getText());

    setSize(920, 520);
    startTimerHz(4);
}

ExtasisMarimbaAudioProcessorEditor::~ExtasisMarimbaAudioProcessorEditor()
{
    stopTimer();
    setLookAndFeel(nullptr);
    processorRef.onAudioBlockProcessed = nullptr;
}

void ExtasisMarimbaAudioProcessorEditor::refreshPresetList()
{
    presetBox.clear(juce::dontSendNotification);
    auto allNames = processorRef.getAllPresetNames();
    for (int i = 0; i < allNames.size(); ++i)
    {
        presetBox.addItem(allNames[i], i + 1);
    }
}

void ExtasisMarimbaAudioProcessorEditor::updateLicenseState()
{
    isActivated = processorRef.isLicensed();
    if (isActivated)
    {
        licenseBadgeButton.setButtonText("LICENSED");
        licenseBadgeButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff27ae60));
        licenseBadgeButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    }
    else
    {
        licenseBadgeButton.setButtonText("DEMO");
        licenseBadgeButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xffe67e22));
        licenseBadgeButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    }
}

void ExtasisMarimbaAudioProcessorEditor::timerCallback()
{
    if (!processorRef.isLicensed() && processorRef.isDemoExpired() && !activationOverlay.isVisible())
    {
        activationOverlay.isExpired = true;
        activationOverlay.setVisible(true);
        activationOverlay.resized();
        activationOverlay.repaint();
    }
}

void ExtasisMarimbaAudioProcessorEditor::createKnob(const juce::String& paramId, const juce::String& labelText, const juce::String& suffix)
{
    auto& ctrl = controls[paramId];

    ctrl.slider = std::make_unique<juce::Slider>(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow);
    ctrl.slider->setTextValueSuffix(suffix.isEmpty() ? "" : " " + suffix);
    ctrl.slider->addListener(this);
    addAndMakeVisible(*ctrl.slider);

    ctrl.label = std::make_unique<juce::Label>("", labelText);
    ctrl.label->setJustificationType(juce::Justification::centred);
    addAndMakeVisible(*ctrl.label);

    ctrl.attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processorRef.getAPVTS(), paramId, *ctrl.slider);
}

void ExtasisMarimbaAudioProcessorEditor::sliderValueChanged(juce::Slider* slider)
{
    for (auto& pair : controls)
    {
        if (pair.second.slider.get() == slider)
        {
            auto paramId = pair.first;
            auto val = slider->getValue();

            if (paramId == "noise")
                display.setParameterReadout("RUBBER MALLET NOISE", juce::String((int)(val * 100)) + "% BANDPASS 3.5kHz");
            else if (paramId == "buzz")
                display.setParameterReadout("CACHIMBA MEMBRANE BUZZ", juce::String((int)(val * 100)) + "% TRADITIONAL CHIAPAS");
            else if (paramId == "buzzVel")
                display.setParameterReadout("BUZZ DYNAMICS", juce::String((int)(val * 100)) + "% VELOCITY RESPONSE");
            else if (paramId == "organic")
                display.setParameterReadout("MARIMBA DE PUEBLO", juce::String((int)(val * 100)) + "% ARTISANAL IMPERFECTION");
            else if (paramId == "tube")
                display.setParameterReadout("RESONATOR TUBE", juce::String((int)(val * 100)) + "% ACOUSTIC BEATING");
            else if (pair.second.label != nullptr)
                display.setParameterReadout(pair.second.label->getText(), slider->getTextFromValue(val));
            
            auto itH = controls.find("hardness");
            auto itD = controls.find("decay");
            if (itH != controls.end() && itH->second.slider != nullptr &&
                itD != controls.end() && itD->second.slider != nullptr)
            {
                float h = static_cast<float>(itH->second.slider->getValue());
                float d = static_cast<float>(itD->second.slider->getValue());
                display.triggerStrikeAnimation(h, d);
            }
            break;
        }
    }
}

void ExtasisMarimbaAudioProcessorEditor::comboBoxChanged(juce::ComboBox* comboBox)
{
    if (comboBox == &presetBox)
    {
        int idx = presetBox.getSelectedId() - 1;
        processorRef.setCurrentProgram(idx);
        display.setPatchName(presetBox.getText());
    }
}

void ExtasisMarimbaAudioProcessorEditor::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // 1. Chassis: Dark Brushed Metal
    g.fillAll(ExtasisGUI::MarimbaLookAndFeel::getBackgroundDark());

    // Bevel borders
    g.setColour(juce::Colour(0xff2a2d33));
    g.drawRect(bounds.reduced(0.5f), 1.0f);

    // 2. Top Header Bar
    auto topBar = bounds.removeFromTop(44.0f);
    g.setColour(ExtasisGUI::MarimbaLookAndFeel::getPanelBackground());
    g.fillRect(topBar);

    g.setColour(ExtasisGUI::MarimbaLookAndFeel::getPanelBorder());
    g.drawHorizontalLine(44, 0.0f, (float)getWidth());

    // Header Title & Brand Subtitle
    g.setFont(juce::FontOptions(17.0f, juce::Font::bold));
    g.setColour(ExtasisGUI::MarimbaLookAndFeel::getAmberGold());
    g.drawText("EXTASIS MARIMBA", 20, 0, 190, 44, juce::Justification::centredLeft);

    g.setFont(juce::FontOptions(10.5f, juce::Font::plain));
    g.setColour(juce::Colour(0xff8e96a4));
    g.drawText("- MEXICAN PHYSICAL-FM SYNTHESIZER", 215, 0, 360, 44, juce::Justification::centredLeft);

    // Amber horizontal strip
    g.setColour(ExtasisGUI::MarimbaLookAndFeel::getAmberGold());
    g.fillRect(20, 42, getWidth() - 40, 2);

    // 3. Modular Sections Framing
    auto drawSectionBox = [&g](juce::Rectangle<float> r, const juce::String& title) {
        g.setColour(juce::Colour(0xff1b1e22));
        g.fillRoundedRectangle(r, 5.0f);
        g.setColour(juce::Colour(0xff333842));
        g.drawRoundedRectangle(r, 5.0f, 1.0f);

        // Section Title Header
        g.setColour(ExtasisGUI::MarimbaLookAndFeel::getAmberGold());
        g.setFont(juce::FontOptions(10.5f, juce::Font::bold));
        g.drawText(title, (int)r.getX() + 10, (int)r.getY() + 4, (int)r.getWidth() - 20, 16, juce::Justification::left);

        g.setColour(juce::Colour(0x33ffa834));
        g.drawHorizontalLine((int)r.getY() + 20, r.getX() + 8.0f, r.getRight() - 8.0f);
    };

    // Bottom 4 Modular Sections
    drawSectionBox(juce::Rectangle<float>(20, 230, 210, 275), "1. MALLET & ATTACK");
    drawSectionBox(juce::Rectangle<float>(240, 230, 210, 275), "2. WOOD & MODAL FM");
    drawSectionBox(juce::Rectangle<float>(460, 230, 220, 275), "3. RESONATOR & BUZZ");
    drawSectionBox(juce::Rectangle<float>(690, 230, 210, 275), "4. MASTER & FX");
}

void ExtasisMarimbaAudioProcessorEditor::resized()
{
    // Header License Badge
    licenseBadgeButton.setBounds(getWidth() - 110, 10, 90, 24);

    // Display on top left (x=20, y=52, w=570, h=168)
    display.setBounds(20, 52, 570, 168);

    // Preset & Trigger Right Module (x=606, y=52, w=294, h=168)
    // Row 1: Preset dropdown + Prev/Next + Save
    presetBox.setBounds(606, 52, 172, 30);
    prevPresetBtn.setBounds(782, 52, 28, 30);
    nextPresetBtn.setBounds(814, 52, 28, 30);
    savePresetBtn.setBounds(846, 52, 54, 30);

    // Row 2: Trigger Pad with Logo
    triggerButton.setBounds(606, 88, 294, 132);

    // Knobs Layout inside the 4 sections
    auto placeKnob = [this](const juce::String& id, int x, int y, int w = 82, int h = 88) {
        if (controls.find(id) != controls.end())
        {
            controls[id].slider->setBounds(x, y + 16, w, h - 16);
            controls[id].label->setBounds(x - 2, y, w + 4, 16);
        }
    };

    // Section 1: Mallet & Attack (x=20)
    placeKnob("hardness", 30, 260, 84, 88);
    placeKnob("noise", 130, 260, 84, 88);
    placeKnob("click", 30, 375, 84, 88);
    placeKnob("attack", 130, 375, 84, 88);

    // Section 2: Wood & Modal FM (x=240)
    placeKnob("decay", 250, 260, 84, 88);
    placeKnob("material", 350, 260, 84, 88);
    placeKnob("overtones", 250, 375, 84, 88);
    placeKnob("organic", 350, 375, 84, 88);

    // Section 3: Resonator & Buzz / La Cachimba (x=460)
    placeKnob("tube", 472, 260, 84, 88);
    placeKnob("buzz", 578, 260, 84, 88);
    placeKnob("buzzVel", 472, 375, 84, 88);
    placeKnob("cutoff", 578, 375, 84, 88);

    // Section 4: Master & FX (x=690)
    placeKnob("spread", 702, 260, 84, 88);
    placeKnob("drive", 802, 260, 84, 88);
    placeKnob("ambience", 702, 375, 84, 88);
    placeKnob("volume", 802, 375, 84, 88);

    // Overlay full bounds
    activationOverlay.setBounds(getLocalBounds());
}
