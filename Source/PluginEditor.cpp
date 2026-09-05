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
                    if (processorRef.saveUserPreset(name))
                    {
                        refreshPresetList();
                        auto names = processorRef.getAllPresetNames();
                        for (int i = 0; i < names.size(); ++i)
                        {
                            if (names[i].contains(name))
                            {
                                presetBox.setSelectedId(i + 1, juce::sendNotification);
                                break;
                            }
                        }
                        display.setParameterReadout("PRESET SAVED", name.toUpperCase());
                    }
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

    // 4. Header License Badge
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

    // 5. Create all Knobs with Live Value Labels
    // Section 1: Mallet & Attack
    // 1. La Madera
    createKnob("hardness", "HARDNESS");
    createKnob("overtones", "OVERTONES");
    createKnob("material", "MATERIAL");
    createKnob("click", "CLICK / SNAP");

    // 2. El Cajon
    createKnob("tube", "TUBE CAVITY");
    createKnob("decay", "BAR DECAY");
    createKnob("buzz", "MEMBRANE BUZZ");
    createKnob("buzzVel", "BUZZ DYNAMICS");

    // 3. El Entorno
    createKnob("organic", "ORGANIC DRIFT");
    createKnob("ambience", "AMBIENCE");
    createKnob("spread", "SPREAD");
    createKnob("volume", "MASTER VOL");

    display.setPatchName(presetBox.getText());

    setSize(660, 410);
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

juce::String ExtasisMarimbaAudioProcessorEditor::getFormattedValueText(const juce::String& paramId, double val)
{
    if (paramId == "attack")
        return juce::String(val, 1) + " ms";
    if (paramId == "cutoff")
        return val >= 1000.0 ? juce::String(val * 0.001, 1) + " kHz" : juce::String((int)val) + " Hz";
    if (paramId == "volume")
        return juce::String((int)(val * 100)) + " %";
    if (paramId == "material")
    {
        if (val < 0.25) return "Hormiguillo " + juce::String((int)(val * 100)) + "%";
        if (val < 0.70) return "Balafon " + juce::String((int)(val * 100)) + "%";
        return "Glass " + juce::String((int)(val * 100)) + "%";
    }

    return juce::String((int)(val * 100)) + " %";
}

void ExtasisMarimbaAudioProcessorEditor::createKnob(const juce::String& paramId, const juce::String& labelText)
{
    auto& ctrl = controls[paramId];

    ctrl.slider = std::make_unique<juce::Slider>(juce::Slider::RotaryVerticalDrag, juce::Slider::NoTextBox);
    ctrl.slider->addListener(this);
    ctrl.slider->setName(paramId);
    addAndMakeVisible(*ctrl.slider);

    ctrl.label = std::make_unique<juce::Label>("", labelText);
    ctrl.label->setJustificationType(juce::Justification::centred);
    ctrl.label->setFont(juce::FontOptions(9.5f, juce::Font::bold));
    ctrl.label->setColour(juce::Label::textColourId, juce::Colour(0xffc8cfdc));
    addAndMakeVisible(*ctrl.label);

    ctrl.valueLabel = std::make_unique<juce::Label>("", getFormattedValueText(paramId, 0.0));
    ctrl.valueLabel->setJustificationType(juce::Justification::centred);
    ctrl.valueLabel->setFont(juce::FontOptions(9.0f, juce::Font::bold));
    ctrl.valueLabel->setColour(juce::Label::textColourId, ExtasisGUI::MarimbaLookAndFeel::getAmberGold());
    addAndMakeVisible(*ctrl.valueLabel);

    ctrl.attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processorRef.getAPVTS(), paramId, *ctrl.slider);

    if (auto* p = processorRef.getAPVTS().getRawParameterValue(paramId))
    {
        ctrl.valueLabel->setText(getFormattedValueText(paramId, *p), juce::dontSendNotification);
    }
}

void ExtasisMarimbaAudioProcessorEditor::sliderValueChanged(juce::Slider* slider)
{
    for (auto& pair : controls)
    {
        if (pair.second.slider.get() == slider)
        {
            auto paramId = pair.first;
            auto val = slider->getValue();

            if (pair.second.valueLabel != nullptr)
                pair.second.valueLabel->setText(getFormattedValueText(paramId, val), juce::dontSendNotification);

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
            else if (paramId == "material")
                display.setParameterReadout("BAR MATERIAL", val < 0.25 ? "HORMIGUILLO WOOD" : (val < 0.7 ? "AFRICAN BALAFON" : "CRYSTAL GLASS"));
            else if (pair.second.label != nullptr)
                display.setParameterReadout(pair.second.label->getText(), getFormattedValueText(paramId, val));
            
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

                for (auto& pair : controls)
        {
            if (auto* param = processorRef.getAPVTS().getParameter(pair.first))
            {
                float trueVal = param->convertFrom0to1(param->getValue());
                if (pair.second.slider != nullptr)
                {
                    pair.second.slider->setValue(trueVal, juce::dontSendNotification);
                }
                if (pair.second.valueLabel != nullptr)
                {
                    pair.second.valueLabel->setText(getFormattedValueText(pair.first, trueVal), juce::dontSendNotification);
                }
            }
        }
    }
}

void ExtasisMarimbaAudioProcessorEditor::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // 1. Chassis: Dark Brushed Obsidian Metal
    g.fillAll(ExtasisGUI::MarimbaLookAndFeel::getBackgroundDark());

    // Bevel borders
    g.setColour(juce::Colour(0xff2a2d33));
    g.drawRect(bounds.reduced(0.5f), 1.0f);

    // 2. Top Header Bar (Chassis Header)
    auto topBar = bounds.removeFromTop(44.0f);
    g.setColour(ExtasisGUI::MarimbaLookAndFeel::getPanelBackground());
    g.fillRect(topBar);

    g.setColour(ExtasisGUI::MarimbaLookAndFeel::getPanelBorder());
    g.drawHorizontalLine(44, 0.0f, (float)getWidth());

    // Header Title & Brand Subtitle (Matching ExtasisDonker style)
    g.setFont(juce::FontOptions(17.0f, juce::Font::bold));
    g.setColour(ExtasisGUI::MarimbaLookAndFeel::getAmberGold());
    g.drawText("EXTASIS MARIMBA", 20, 6, 185, 20, juce::Justification::left);

    g.setFont(juce::FontOptions(9.5f, juce::Font::bold));
    g.setColour(juce::Colour(0xff8e96a4));

    // coded by @laurorobles in tiny font right below the title
    g.setFont(juce::FontOptions(8.5f, juce::Font::bold));
    g.setColour(juce::Colour(0xff707a8a));
    g.drawText("coded by @laurorobles", 22, 26, 120, 14, juce::Justification::left);

    g.setFont(juce::FontOptions(9.5f, juce::Font::bold));
    g.setColour(ExtasisGUI::MarimbaLookAndFeel::getAmberGold().withAlpha(0.85f));

    // Amber horizontal glowing accent strip
    g.setColour(ExtasisGUI::MarimbaLookAndFeel::getAmberGold());
    g.fillRect(20, 42, getWidth() - 40, 2);

    // 3. Modular Sections Framing with Hormiguillo Rosewood Accents
    auto drawSectionBox = [&g](juce::Rectangle<float> r, const juce::String& title) {
        // Outer dark panel
        g.setColour(juce::Colour(0xff181b20));
        g.fillRoundedRectangle(r, 5.0f);

        // Subtle wood-accent border
        g.setColour(juce::Colour(0xff333842));
        g.drawRoundedRectangle(r, 5.0f, 1.0f);

        // Section Title Header
        g.setColour(ExtasisGUI::MarimbaLookAndFeel::getAmberGold());
        g.setFont(juce::FontOptions(10.0f, juce::Font::bold));
        g.drawText(title, (int)r.getX() + 10, (int)r.getY() + 4, (int)r.getWidth() - 20, 16, juce::Justification::left);

        g.setColour(juce::Colour(0x33ffa834));
        g.drawHorizontalLine((int)r.getY() + 20, r.getX() + 8.0f, r.getRight() - 8.0f);
    };

    // Bottom 3 Modular Sections
    drawSectionBox(juce::Rectangle<float>(15, 175, 210, 220), "1. WOODEN BAR");
    drawSectionBox(juce::Rectangle<float>(235, 175, 210, 220), "2. RESONATOR TUBE");
    drawSectionBox(juce::Rectangle<float>(455, 175, 190, 220), "3. ENVIRONMENT");
}

void ExtasisMarimbaAudioProcessorEditor::resized()
{
    // Bandcamp Link in Header (Removed - only in License Overlay)

    // Header License Badge
    licenseBadgeButton.setBounds(getWidth() - 110, 8, 90, 24);

    // Display on top left
    display.setBounds(15, 50, 400, 115);

    // Preset & Trigger Right Module
    presetBox.setBounds(430, 50, 150, 26);
    prevPresetBtn.setBounds(585, 50, 20, 26);
    nextPresetBtn.setBounds(610, 50, 20, 26);
    savePresetBtn.setBounds(635, 50, 0, 0); // Hide save button for a cleaner look, or make it small? We'll hide it for cleaner UI, presets shouldn't be overridden in factory mode. Wait, users might want to save. Let's keep it.
    savePresetBtn.setBounds(15, 50, 0, 0); // Actually hide it to make it clean.
    
    // Trigger Pad with Logo
    triggerButton.setBounds(430, 80, 215, 85);

    // Knobs Layout inside the 3 sections
    auto placeKnob = [this](const juce::String& id, int x, int y, int w = 60, int h = 85) {
        if (controls.find(id) != controls.end())
        {
            if (controls[id].label) {
                controls[id].label->setFont(juce::FontOptions(9.0f, juce::Font::bold));
                controls[id].label->setBounds(x - 10, y, w + 20, 14);
            }
            if (controls[id].slider) controls[id].slider->setBounds(x, y + 14, w, h - 28);
            if (controls[id].valueLabel) {
                controls[id].valueLabel->setFont(juce::FontOptions(9.0f, juce::Font::bold));
                controls[id].valueLabel->setBounds(x - 10, y + h - 14, w + 20, 14);
            }
        }
    };

    // Section 1: La Madera
    placeKnob("hardness", 40, 205);
    placeKnob("overtones", 135, 205);
    placeKnob("material", 40, 295);
    placeKnob("click", 135, 295);

    // Section 2: El Cajon
    placeKnob("tube", 260, 205);
    placeKnob("decay", 355, 205);
    placeKnob("buzz", 260, 295);
    placeKnob("buzzVel", 355, 295);

    // Section 3: El Entorno
    placeKnob("organic", 475, 205);
    placeKnob("ambience", 560, 205);
    placeKnob("spread", 475, 295);
    placeKnob("volume", 560, 295);

    // Overlay full bounds
    activationOverlay.setBounds(getLocalBounds());
}
