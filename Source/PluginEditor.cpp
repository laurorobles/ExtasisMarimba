#include "PluginEditor.h"

ExtasisMarimbaAudioProcessorEditor::ExtasisMarimbaAudioProcessorEditor(ExtasisMarimbaAudioProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    addAndMakeVisible(gumroadLinkBtn);
    gumroadLinkBtn.setColour(juce::HyperlinkButton::textColourId, juce::Colours::yellow);

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

    // 4. Header Links & License Badge
    bandcampLinkBtn.setFont(juce::FontOptions(10.0f, juce::Font::bold), false, juce::Justification::centredRight);
    bandcampLinkBtn.setColour(juce::HyperlinkButton::textColourId, ExtasisGUI::MarimbaLookAndFeel::getAmberGold().withAlpha(0.95f));
    addAndMakeVisible(bandcampLinkBtn);

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
    gumroadLinkBtn.setBounds(getWidth() - 110, 10, 100, 24);

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
    createKnob("hardness", "DUREZA");
    createKnob("overtones", "POSICION");
    createKnob("material", "AFINACION");
    createKnob("click", "GOLPE");

    // 2. El Cajon
    createKnob("tube", "RESONANCIA");
    createKnob("decay", "CUERPO");
    createKnob("buzz", "CHARLEO");
    createKnob("buzzVel", "SENS. CHARLEO");

    // 3. El Entorno
    createKnob("organic", "IMPERFECCION");
    createKnob("ambience", "ESPACIO");
    createKnob("spread", "ESTEREO");
    createKnob("volume", "VOLUMEN");

    display.setPatchName(presetBox.getText());

    setSize(580, 380);
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
    g.drawText("coded by @laurorobles // Extasis Records", 22, 26, 260, 14, juce::Justification::left);

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
    drawSectionBox(juce::Rectangle<float>(15, 170, 175, 195), "1. LA MADERA");
    drawSectionBox(juce::Rectangle<float>(200, 170, 175, 195), "2. EL CAJON");
    drawSectionBox(juce::Rectangle<float>(385, 170, 180, 195), "3. EL ENTORNO");
}

void ExtasisMarimbaAudioProcessorEditor::resized()
{
    // Bandcamp Link in Header
    bandcampLinkBtn.setBounds(getWidth() - 325, 8, 205, 18);

    // Header License Badge
    licenseBadgeButton.setBounds(getWidth() - 110, 8, 90, 24);

    // Display on top left
    display.setBounds(15, 45, 330, 110);

    // Preset & Trigger Right Module
    presetBox.setBounds(355, 45, 120, 26);
    prevPresetBtn.setBounds(480, 45, 20, 26);
    nextPresetBtn.setBounds(505, 45, 20, 26);
    savePresetBtn.setBounds(530, 45, 35, 26);

    // Trigger Pad with Logo
    triggerButton.setBounds(355, 75, 210, 80);

    // Knobs Layout inside the 3 sections
    auto placeKnob = [this](const juce::String& id, int x, int y, int w = 55, int h = 75) {
        if (controls.find(id) != controls.end())
        {
            if (controls[id].label) {
                controls[id].label->setFont(juce::FontOptions(8.5f, juce::Font::bold));
                controls[id].label->setBounds(x - 5, y, w + 10, 14);
            }
            if (controls[id].slider) controls[id].slider->setBounds(x, y + 14, w, h - 28);
            if (controls[id].valueLabel) {
                controls[id].valueLabel->setFont(juce::FontOptions(8.5f, juce::Font::bold));
                controls[id].valueLabel->setBounds(x - 5, y + h - 14, w + 10, 14);
            }
        }
    };

    // Section 1: La Madera
    placeKnob("hardness", 35, 200);
    placeKnob("overtones", 115, 200);
    placeKnob("material", 35, 280);
    placeKnob("click", 115, 280);

    // Section 2: El Cajon
    placeKnob("tube", 220, 200);
    placeKnob("decay", 300, 200);
    placeKnob("buzz", 220, 280);
    placeKnob("buzzVel", 300, 280);

    // Section 3: El Entorno
    placeKnob("organic", 405, 200);
    placeKnob("ambience", 485, 200);
    placeKnob("spread", 405, 280);
    placeKnob("volume", 485, 280);

    // Overlay full bounds
    activationOverlay.setBounds(getLocalBounds());
}
