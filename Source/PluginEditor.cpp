#include "PluginEditor.h"

ExtasisMarimbaAudioProcessorEditor::ExtasisMarimbaAudioProcessorEditor(ExtasisMarimbaAudioProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    setLookAndFeel(&customLookAndFeel);

    // 1. Display Setup (Lock-Free UI-Thread Telemetry Pull)
    addAndMakeVisible(display);
    display.onPullScopeSamples = [this](float* dest, int numSamples) {
        processorRef.copyScopeSamples(dest, numSamples);
    };
    display.onPullOutputRms = [this]() {
        return processorRef.getOutputRms();
    };
    display.onPullActiveVoices = [this]() {
        return processorRef.getActiveVoiceCount();
    };

    // 2. Header Quick Actions (INIT, RND, Open Source Badge)
    initBtn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff232830));
    initBtn.setColour(juce::TextButton::textColourOffId, juce::Colour(0xffc8cfdc));
    initBtn.setTooltip("Reset all parameters to a clean concert marimba");
    initBtn.onClick = [this]() {
        processorRef.initParameters();
        syncAllControlsFromAPVTS();
        display.setPatchName("Init Concert Marimba");
        display.setParameterReadout("PATCH INITIALIZED", "CLEAN 1:4:10 ROSEWOOD");
    };
    addAndMakeVisible(initBtn);

    randBtn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff2d2417));
    randBtn.setColour(juce::TextButton::textColourOffId, ExtasisGUI::MarimbaLookAndFeel::getBrightAmber());
    randBtn.setTooltip("Generate a musical randomized marimba patch");
    randBtn.onClick = [this]() {
        processorRef.randomizeParameters();
        syncAllControlsFromAPVTS();
        display.setPatchName("Randomized Marimba");
        display.setParameterReadout("RANDOMIZED PATCH", "AUDITION WITH TRIGGER PAD");
    };
    addAndMakeVisible(randBtn);

    ossBadgeLink.setFont(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), 9.5f, juce::Font::bold), false);
    ossBadgeLink.setColour(juce::HyperlinkButton::textColourId, ExtasisGUI::MarimbaLookAndFeel::getLcdCyan().withAlpha(0.85f));
    ossBadgeLink.setTooltip("Extasis Marimba is 100% Free & Open Source (GPLv3) — Click to visit GitHub");
    addAndMakeVisible(ossBadgeLink);

    // 3. Preset Selector & Management
    refreshPresetList();
    presetBox.setSelectedId(processorRef.getCurrentProgram() + 1, juce::dontSendNotification);
    presetBox.addListener(this);
    addAndMakeVisible(presetBox);

    prevPresetBtn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff1e2229));
    prevPresetBtn.setColour(juce::TextButton::textColourOffId, ExtasisGUI::MarimbaLookAndFeel::getBrightAmber());
    prevPresetBtn.onClick = [this]() {
        int count = processorRef.getNumPrograms();
        if (count <= 0) return;
        int cur = juce::jmax(0, presetBox.getSelectedId() - 1);
        int next = (cur - 1 + count) % count;
        presetBox.setSelectedId(next + 1, juce::sendNotification);
    };
    addAndMakeVisible(prevPresetBtn);

    nextPresetBtn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff1e2229));
    nextPresetBtn.setColour(juce::TextButton::textColourOffId, ExtasisGUI::MarimbaLookAndFeel::getBrightAmber());
    nextPresetBtn.onClick = [this]() {
        int count = processorRef.getNumPrograms();
        if (count <= 0) return;
        int cur = juce::jmax(0, presetBox.getSelectedId() - 1);
        int next = (cur + 1) % count;
        presetBox.setSelectedId(next + 1, juce::sendNotification);
    };
    addAndMakeVisible(nextPresetBtn);

    savePresetBtn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff332512));
    savePresetBtn.setColour(juce::TextButton::textColourOffId, ExtasisGUI::MarimbaLookAndFeel::getBrightAmber());
    savePresetBtn.setTooltip("Save current settings as a user preset (.empreset)");
    savePresetBtn.onClick = [this]() {
        auto* alert = new juce::AlertWindow("SAVE USER PRESET", "Enter a name for your marimba preset:", juce::AlertWindow::NoIcon);
        alert->addTextEditor("presetName", "My Custom Marimba");
        alert->addButton("SAVE", 1, juce::KeyPress(juce::KeyPress::returnKey));
        alert->addButton("CANCEL", 0, juce::KeyPress(juce::KeyPress::escapeKey));

        juce::Component::SafePointer<ExtasisMarimbaAudioProcessorEditor> safeThis(this);
        alert->enterModalState(true, juce::ModalCallbackFunction::create([safeThis, alert](int result) {
            if (result == 1 && safeThis != nullptr)
            {
                auto name = alert->getTextEditorContents("presetName").trim();
                if (name.isNotEmpty() && safeThis->processorRef.saveUserPreset(name))
                {
                    safeThis->refreshPresetList();
                    auto names = safeThis->processorRef.getAllPresetNames();
                    auto target = "[User] " + juce::File::createLegalFileName(name);
                    for (int i = 0; i < names.size(); ++i)
                    {
                        if (names[i] == target)
                        {
                            safeThis->presetBox.setSelectedId(i + 1, juce::dontSendNotification);
                            safeThis->display.setPatchName(names[i]);
                            break;
                        }
                    }
                    safeThis->display.setParameterReadout("PRESET SAVED", name.toUpperCase());
                }
            }
            delete alert;
        }));
    };
    addAndMakeVisible(savePresetBtn);

    // 4. Logo Trigger Button (Lock-Free Audition)
    triggerButton.onNoteOn = [this](int midiNote, float vel) {
        processorRef.triggerAuditionNote(midiNote, vel);

        float h = *processorRef.getAPVTS().getRawParameterValue("hardness");
        float d = *processorRef.getAPVTS().getRawParameterValue("decay");
        display.triggerStrikeAnimation(h, d);
    };

    triggerButton.onNoteOff = [this](int midiNote) {
        processorRef.releaseAuditionNote(midiNote);
    };

    triggerButton.onStatusChange = [this](const juce::String& title, const juce::String& desc) {
        display.setParameterReadout(title, desc);
    };
    addAndMakeVisible(triggerButton);

    // 5. Create all 12 Knobs with Double-Click Reset & Live Value Labels
    // Section 1: Wooden Bar
    createKnob("hardness",  "HARDNESS",      0.65);
    createKnob("overtones", "OVERTONES",     0.40);
    createKnob("material",  "MATERIAL",      0.50);
    createKnob("click",     "CLICK / SNAP",  0.35);

    // Section 2: Resonator Tube
    createKnob("tube",      "TUBE CAVITY",   0.85);
    createKnob("decay",     "BAR DECAY",     0.55);
    createKnob("buzz",      "MEMBRANE BUZZ", 0.60);
    createKnob("buzzVel",   "BUZZ DYNAMICS", 0.65);

    // Section 3: Environment
    createKnob("organic",   "ORGANIC DRIFT", 0.45);
    createKnob("ambience",  "AMBIENCE",      0.25);
    createKnob("spread",    "SPREAD",        0.60);
    createKnob("volume",    "MASTER VOL",    0.85);

    syncDisplayModalParams();
    display.setPatchName(presetBox.getText());

    setSize(680, 420);
}

ExtasisMarimbaAudioProcessorEditor::~ExtasisMarimbaAudioProcessorEditor()
{
    display.onPullScopeSamples = nullptr;
    display.onPullOutputRms = nullptr;
    display.onPullActiveVoices = nullptr;
    setLookAndFeel(nullptr);
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

void ExtasisMarimbaAudioProcessorEditor::syncAllControlsFromAPVTS()
{
    for (auto& pair : controls)
    {
        if (auto* param = processorRef.getAPVTS().getParameter(pair.first))
        {
            float trueVal = param->convertFrom0to1(param->getValue());
            if (pair.second.slider != nullptr)
                pair.second.slider->setValue(trueVal, juce::dontSendNotification);
            if (pair.second.valueLabel != nullptr)
                pair.second.valueLabel->setText(getFormattedValueText(pair.first, trueVal), juce::dontSendNotification);
        }
    }
    syncDisplayModalParams();
}

void ExtasisMarimbaAudioProcessorEditor::syncDisplayModalParams()
{
    auto& apvts = processorRef.getAPVTS();
    display.setModalParams(
        *apvts.getRawParameterValue("hardness"),
        *apvts.getRawParameterValue("overtones"),
        *apvts.getRawParameterValue("tube"),
        *apvts.getRawParameterValue("buzz")
    );
}

juce::String ExtasisMarimbaAudioProcessorEditor::getFormattedValueText(const juce::String& paramId, double val)
{
    if (paramId == "material")
    {
        if (val < 0.35) return "Soft Wood";
        if (val <= 0.65) return "1:4:10 Wood";
        return "Glass / Metal";
    }
    if (paramId == "decay")
    {
        int ms = static_cast<int>(100.0 + val * 450.0);
        return juce::String(ms) + " ms";
    }

    return juce::String(static_cast<int>(std::round(val * 100.0))) + " %";
}

void ExtasisMarimbaAudioProcessorEditor::createKnob(const juce::String& paramId, const juce::String& labelText, double defaultVal)
{
    auto& ctrl = controls[paramId];

    ctrl.slider = std::make_unique<juce::Slider>(juce::Slider::RotaryVerticalDrag, juce::Slider::NoTextBox);
    ctrl.slider->addListener(this);
    ctrl.slider->setName(paramId);
    ctrl.slider->setDoubleClickReturnValue(true, defaultVal);
    addAndMakeVisible(*ctrl.slider);

    ctrl.label = std::make_unique<juce::Label>("", labelText);
    ctrl.label->setJustificationType(juce::Justification::centred);
    ctrl.label->setFont(juce::FontOptions(9.0f, juce::Font::bold));
    ctrl.label->setColour(juce::Label::textColourId, juce::Colour(0xffc8cfdc));
    addAndMakeVisible(*ctrl.label);

    ctrl.valueLabel = std::make_unique<juce::Label>("", getFormattedValueText(paramId, defaultVal));
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
            const auto& paramId = pair.first;
            double val = slider->getValue();

            if (pair.second.valueLabel != nullptr)
                pair.second.valueLabel->setText(getFormattedValueText(paramId, val), juce::dontSendNotification);

            int pct = static_cast<int>(std::round(val * 100.0));
            if (paramId == "hardness")
                display.setParameterReadout("MALLET HARDNESS", juce::String(pct) + "% FM TRANSIENT ENERGY");
            else if (paramId == "overtones")
                display.setParameterReadout("4x / 10x OVERTONES", juce::String(pct) + "% CENTER-TO-EDGE STRIKE");
            else if (paramId == "material")
                display.setParameterReadout("BAR CARVING", val < 0.35 ? "DARK WARM WOOD" : (val <= 0.65 ? "1:4:10 HORMIGUILLO" : "CRYSTAL / GLASS"));
            else if (paramId == "click")
                display.setParameterReadout("MALLET CLICK / SNAP", juce::String(pct) + "% 2.5kHz HPF ATTACK");
            else if (paramId == "tube")
                display.setParameterReadout("RESONATOR TUBE", juce::String(pct) + "% 20ms CAVITY BLOOM");
            else if (paramId == "decay")
                display.setParameterReadout("BAR DECAY", getFormattedValueText(paramId, val) + " KEY-TRACKED");
            else if (paramId == "buzz")
                display.setParameterReadout("LA CACHIMBA BUZZ", juce::String(pct) + "% CHIAPAS MEMBRANE");
            else if (paramId == "buzzVel")
                display.setParameterReadout("BUZZ DYNAMICS", juce::String(pct) + "% PRESSURE THRESHOLD");
            else if (paramId == "organic")
                display.setParameterReadout("MARIMBA DE PUEBLO", juce::String(pct) + "% PER-KEY IMPERFECTION");
            else if (paramId == "ambience")
                display.setParameterReadout("STEREO AMBIENCE", juce::String(pct) + "% COURTYARD REFLECTIONS");
            else if (paramId == "spread")
                display.setParameterReadout("ACOUSTIC PANNING", juce::String(pct) + "% EQUAL-POWER WIDTH");
            else if (pair.second.label != nullptr)
                display.setParameterReadout(pair.second.label->getText(), getFormattedValueText(paramId, val));

            syncDisplayModalParams();
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
        syncAllControlsFromAPVTS();
    }
}

void ExtasisMarimbaAudioProcessorEditor::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // 1. Chassis: Dark Brushed Obsidian Metal
    g.fillAll(ExtasisGUI::MarimbaLookAndFeel::getBackgroundDark());

    // Subtle outer bevel border
    g.setColour(juce::Colour(0xff2a2d33));
    g.drawRect(bounds.reduced(0.5f), 1.0f);

    // 2. Top Header Bar
    auto topBar = bounds.removeFromTop(44.0f);
    g.setColour(ExtasisGUI::MarimbaLookAndFeel::getPanelBackground());
    g.fillRect(topBar);

    g.setColour(ExtasisGUI::MarimbaLookAndFeel::getPanelBorder());
    g.drawHorizontalLine(44, 0.0f, static_cast<float>(getWidth()));

    // Header Title & Version Pill
    g.setFont(juce::FontOptions(17.5f, juce::Font::bold));
    g.setColour(ExtasisGUI::MarimbaLookAndFeel::getAmberGold());
    g.drawText("EXTASIS MARIMBA", 18, 5, 175, 21, juce::Justification::left);

    // Version badge next to title
    juce::Rectangle<float> verPill(186.0f, 8.0f, 38.0f, 15.0f);
    g.setColour(juce::Colour(0x22ffa834));
    g.fillRoundedRectangle(verPill, 3.0f);
    g.setColour(ExtasisGUI::MarimbaLookAndFeel::getAmberGold().withAlpha(0.65f));
    g.drawRoundedRectangle(verPill, 3.0f, 1.0f);
    g.setFont(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), 9.0f, juce::Font::bold));
    g.drawText("v4.0", verPill.toNearestInt(), juce::Justification::centred);

    // Developer credit & open-source subtitle
    g.setFont(juce::FontOptions(8.8f, juce::Font::bold));
    g.setColour(juce::Colour(0xff7d8898));
    g.drawText("coded by @laurorobles  //  OPEN SOURCE PHYSICAL-FM", 19, 26, 280, 14, juce::Justification::left);

    // Amber horizontal glowing accent strip
    g.setColour(ExtasisGUI::MarimbaLookAndFeel::getAmberGold());
    g.fillRect(16, 42, getWidth() - 32, 2);

    // 3. Symmetrical Modular Sections Framing
    auto drawSectionBox = [&g](juce::Rectangle<float> r, const juce::String& title) {
        g.setColour(juce::Colour(0xff181b20));
        g.fillRoundedRectangle(r, 6.0f);

        g.setColour(juce::Colour(0xff313742));
        g.drawRoundedRectangle(r, 6.0f, 1.0f);

        g.setColour(ExtasisGUI::MarimbaLookAndFeel::getAmberGold());
        g.setFont(juce::FontOptions(10.0f, juce::Font::bold));
        g.drawText(title, static_cast<int>(r.getX()) + 10, static_cast<int>(r.getY()) + 5,
                   static_cast<int>(r.getWidth()) - 20, 16, juce::Justification::left);

        g.setColour(juce::Colour(0x33ffa834));
        g.drawHorizontalLine(static_cast<int>(r.getY()) + 22, r.getX() + 8.0f, r.getRight() - 8.0f);
    };

    // 3 symmetrical 208px-wide boxes (16 + 208 + 12 + 208 + 12 + 208 + 16 = 680)
    drawSectionBox(juce::Rectangle<float>(16.0f,  184.0f, 208.0f, 222.0f), "1. WOODEN BAR");
    drawSectionBox(juce::Rectangle<float>(236.0f, 184.0f, 208.0f, 222.0f), "2. RESONATOR TUBE");
    drawSectionBox(juce::Rectangle<float>(456.0f, 184.0f, 208.0f, 222.0f), "3. ENVIRONMENT");
}

void ExtasisMarimbaAudioProcessorEditor::resized()
{
    // Header Right Controls: [GPLv3 // FREE]  [INIT]  [RND]
    ossBadgeLink.setBounds(getWidth() - 224, 11, 102, 20);
    initBtn.setBounds(getWidth() - 114, 9, 46, 24);
    randBtn.setBounds(getWidth() - 62, 9, 46, 24);

    // Top Dashboard Row (y=52, h=120)
    display.setBounds(16, 52, 416, 120);

    // Right Module: Preset Bar (aligned x=444..664, total width=220)
    presetBox.setBounds(444, 52, 130, 26);
    prevPresetBtn.setBounds(578, 52, 22, 26);
    nextPresetBtn.setBounds(602, 52, 22, 26);
    savePresetBtn.setBounds(626, 52, 38, 26);

    // Logo Trigger Pad (aligned x=444..664, y=84..172)
    triggerButton.setBounds(444, 84, 220, 88);

    // Symmetrical Knobs Layout inside the 3 208px sections
    auto placeKnob = [this](const juce::String& id, int centerX, int y, int knobW = 60, int totalH = 84) {
        auto it = controls.find(id);
        if (it != controls.end())
        {
            int x = centerX - knobW / 2;
            if (it->second.label)
                it->second.label->setBounds(centerX - 48, y, 96, 14);
            if (it->second.slider)
                it->second.slider->setBounds(x, y + 14, knobW, totalH - 28);
            if (it->second.valueLabel)
                it->second.valueLabel->setBounds(centerX - 48, y + totalH - 14, 96, 14);
        }
    };

    // Row Y coordinates inside sections (y=184..406)
    const int row1Y = 214;
    const int row2Y = 308;

    // Section 1 (x=16..224 -> cols at 68, 172)
    placeKnob("hardness",  68,  row1Y);
    placeKnob("overtones", 172, row1Y);
    placeKnob("material",  68,  row2Y);
    placeKnob("click",     172, row2Y);

    // Section 2 (x=236..444 -> cols at 288, 392)
    placeKnob("tube",      288, row1Y);
    placeKnob("decay",     392, row1Y);
    placeKnob("buzz",      288, row2Y);
    placeKnob("buzzVel",   392, row2Y);

    // Section 3 (x=456..664 -> cols at 508, 612)
    placeKnob("organic",   508, row1Y);
    placeKnob("ambience",  612, row1Y);
    placeKnob("spread",    508, row2Y);
    placeKnob("volume",    612, row2Y);
}
