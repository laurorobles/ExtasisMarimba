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

    // 2. Preset Selector
    for (int i = 0; i < processorRef.getPresetCount(); ++i)
    {
        presetBox.addItem(processorRef.getPresetTitle(i), i + 1);
    }
    presetBox.setSelectedId(processorRef.getCurrentProgram() + 1, juce::dontSendNotification);
    presetBox.addListener(this);
    addAndMakeVisible(presetBox);

    prevPresetBtn.onClick = [this]() {
        int cur = presetBox.getSelectedId() - 1;
        int count = processorRef.getPresetCount();
        int next = (cur - 1 + count) % count;
        presetBox.setSelectedId(next + 1, juce::sendNotification);
    };
    addAndMakeVisible(prevPresetBtn);

    nextPresetBtn.onClick = [this]() {
        int cur = presetBox.getSelectedId() - 1;
        int count = processorRef.getPresetCount();
        int next = (cur + 1) % count;
        presetBox.setSelectedId(next + 1, juce::sendNotification);
    };
    addAndMakeVisible(nextPresetBtn);

    // 3. Trigger & Test Note Setup (Like ExtasisDonker)
    triggerButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff221b10));
    triggerButton.setColour(juce::TextButton::textColourOnId, ExtasisGUI::MarimbaLookAndFeel::getBrightAmber());
    triggerButton.setColour(juce::TextButton::textColourOffId, ExtasisGUI::MarimbaLookAndFeel::getAmberGold());
    
    triggerButton.onStateChange = [this]() {
        if (triggerButton.isDown())
        {
            playTriggerNote();
        }
        else
        {
            stopTriggerNote();
        }
    };
    addAndMakeVisible(triggerButton);

    // Note Selection ComboBox
    struct NoteItem { int midi; const char* name; };
    const NoteItem notes[] = {
        { 36, "C2 (36) Sub Bar" },
        { 41, "F2 (41)" },
        { 45, "A2 (45)" },
        { 48, "C3 (48) Low Bar" },
        { 52, "E3 (52)" },
        { 55, "G3 (55)" },
        { 57, "A3 (57)" },
        { 60, "C4 (60) Middle C" },
        { 64, "E4 (64)" },
        { 67, "G4 (67)" },
        { 69, "A4 (69)" },
        { 72, "C5 (72) Bright Bar" },
        { 76, "E5 (76)" },
        { 79, "G5 (79)" },
        { 84, "C6 (84) High Wood" }
    };

    for (int i = 0; i < 15; ++i)
    {
        triggerNoteBox.addItem(notes[i].name, i + 1);
    }
    triggerNoteBox.setSelectedId(8, juce::dontSendNotification); // Default C4 (60)
    currentTriggerNote = 60;
    triggerNoteBox.addListener(this);
    addAndMakeVisible(triggerNoteBox);

    noteDownBtn.onClick = [this]() {
        int curId = triggerNoteBox.getSelectedId();
        int nextId = (curId <= 1) ? triggerNoteBox.getNumItems() : curId - 1;
        triggerNoteBox.setSelectedId(nextId, juce::sendNotification);
    };
    addAndMakeVisible(noteDownBtn);

    noteUpBtn.onClick = [this]() {
        int curId = triggerNoteBox.getSelectedId();
        int nextId = (curId >= triggerNoteBox.getNumItems()) ? 1 : curId + 1;
        triggerNoteBox.setSelectedId(nextId, juce::sendNotification);
    };
    addAndMakeVisible(noteUpBtn);

    // Velocity Selector
    triggerVelBox.addItem("VEL: 100% (Hard)", 1);
    triggerVelBox.addItem("VEL: 80% (Norm)", 2);
    triggerVelBox.addItem("VEL: 60% (Med)", 3);
    triggerVelBox.addItem("VEL: 40% (Felt)", 4);
    triggerVelBox.setSelectedId(1, juce::dontSendNotification);
    triggerVelBox.addListener(this);
    addAndMakeVisible(triggerVelBox);

    // 4. Create all Rotary Controls
    // Mallet Section
    createKnob("hardness", "HARDNESS", "%");
    createKnob("click", "CLICK / SNAP", "%");
    createKnob("attack", "ATTACK", "ms");

    // Modal Resonator Section
    createKnob("decay", "BAR DECAY", "%");
    createKnob("material", "MATERIAL", "%");
    createKnob("overtones", "OVERTONES", "%");
    createKnob("pipe", "PIPE BODY", "%");

    // SEM Filter Section
    createKnob("cutoff", "CUTOFF", "Hz");
    createKnob("resonance", "RESO", "%");
    createKnob("filterEnv", "ENV AMT", "%");

    // Envelope & FX Section
    createKnob("envDecay", "ENV DECAY", "ms");
    createKnob("release", "RELEASE", "ms");
    createKnob("spread", "SPREAD", "%");
    createKnob("drive", "WARM DRIVE", "%");
    createKnob("ambience", "AMBIENCE", "%");
    createKnob("volume", "MASTER VOL", "");

    display.setPatchName(presetBox.getText());

    setSize(920, 520);
}

ExtasisMarimbaAudioProcessorEditor::~ExtasisMarimbaAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
    processorRef.onAudioBlockProcessed = nullptr;
}

void ExtasisMarimbaAudioProcessorEditor::playTriggerNote()
{
    processorRef.getSynthEngine().noteOn(currentTriggerNote, currentTriggerVel);

    float h = 0.65f;
    float d = 0.52f;
    auto itH = controls.find("hardness");
    auto itD = controls.find("decay");
    if (itH != controls.end() && itH->second.slider != nullptr)
        h = static_cast<float>(itH->second.slider->getValue());
    if (itD != controls.end() && itD->second.slider != nullptr)
        d = static_cast<float>(itD->second.slider->getValue());

    display.triggerStrikeAnimation(h, d);
    display.setParameterReadout("TRIGGER NOTE", triggerNoteBox.getText() + " | VEL: " + juce::String(int(currentTriggerVel * 100)) + "%");
}

void ExtasisMarimbaAudioProcessorEditor::stopTriggerNote()
{
    processorRef.getSynthEngine().noteOff(currentTriggerNote);
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
            if (pair.second.label != nullptr)
                display.setParameterReadout(pair.second.label->getText(), slider->getTextFromValue(slider->getValue()));
            
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
    else if (comboBox == &triggerNoteBox)
    {
        const int midiTable[] = { 36, 41, 45, 48, 52, 55, 57, 60, 64, 67, 69, 72, 76, 79, 84 };
        int idx = triggerNoteBox.getSelectedId() - 1;
        if (idx >= 0 && idx < 15)
        {
            currentTriggerNote = midiTable[idx];
            display.setParameterReadout("SELECT NOTE", triggerNoteBox.getText());
        }
    }
    else if (comboBox == &triggerVelBox)
    {
        int id = triggerVelBox.getSelectedId();
        if (id == 1) currentTriggerVel = 1.0f;
        else if (id == 2) currentTriggerVel = 0.8f;
        else if (id == 3) currentTriggerVel = 0.6f;
        else if (id == 4) currentTriggerVel = 0.4f;
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

    // Logo & Subtitle
    g.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 17.0f, juce::Font::bold));
    g.setColour(ExtasisGUI::MarimbaLookAndFeel::getAmberGold());
    g.drawText("EXTASIS MARIMBA", 20, 0, 200, 44, juce::Justification::centredLeft);

    g.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 10.5f, juce::Font::plain));
    g.setColour(juce::Colour(0xff8e96a4));
    g.drawText("- PHYSICAL MODELING SYNTH // MICROFREAK MODAL ENGINE", 225, 0, 450, 44, juce::Justification::centredLeft);

    // Amber horizontal strip
    g.setColour(ExtasisGUI::MarimbaLookAndFeel::getAmberGold());
    g.fillRect(20, 42, getWidth() - 40, 2);

    // 3. Preset & Trigger Right Panel Enclosure
    auto drawSectionBox = [&g](juce::Rectangle<float> r, const juce::String& title) {
        g.setColour(juce::Colour(0xff1b1e22));
        g.fillRoundedRectangle(r, 5.0f);
        g.setColour(juce::Colour(0xff333842));
        g.drawRoundedRectangle(r, 5.0f, 1.0f);

        // Section Title Header
        g.setColour(ExtasisGUI::MarimbaLookAndFeel::getAmberGold());
        g.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 10.5f, juce::Font::bold));
        g.drawText(title, (int)r.getX() + 10, (int)r.getY() + 4, (int)r.getWidth() - 20, 16, juce::Justification::left);

        g.setColour(juce::Colour(0x33ffa834));
        g.drawHorizontalLine((int)r.getY() + 20, r.getX() + 8.0f, r.getRight() - 8.0f);
    };

    // Preset & Trigger Box (x=600, y=52, w=300, h=168)
    drawSectionBox(juce::Rectangle<float>(600, 52, 300, 168), "PRESET & TRIGGER CONTROL");

    // 4. Bottom 4 Modular Sections
    drawSectionBox(juce::Rectangle<float>(20, 230, 195, 275), "1. MALLET & STRIKE");
    drawSectionBox(juce::Rectangle<float>(225, 230, 255, 275), "2. MODAL RESONATOR");
    drawSectionBox(juce::Rectangle<float>(490, 230, 195, 275), "3. SEM FILTER");
    drawSectionBox(juce::Rectangle<float>(695, 230, 205, 275), "4. MASTER & FX");
}

void ExtasisMarimbaAudioProcessorEditor::resized()
{
    // Display on top left (x=20, y=52, w=570, h=168)
    display.setBounds(20, 52, 570, 168);

    // Preset & Trigger Controls inside the top-right box (x=600, y=52, w=300, h=168)
    int boxX = 612;
    int boxY = 80;

    // Fila 1: Preset dropdown + Prev/Next
    presetBox.setBounds(boxX, boxY, 210, 28);
    prevPresetBtn.setBounds(boxX + 216, boxY, 28, 28);
    nextPresetBtn.setBounds(boxX + 248, boxY, 28, 28);

    // Fila 2: Trigger Button
    triggerButton.setBounds(boxX, boxY + 36, 276, 36);

    // Fila 3: Note selector + Velocity
    noteDownBtn.setBounds(boxX, boxY + 78, 24, 26);
    triggerNoteBox.setBounds(boxX + 28, boxY + 78, 126, 26);
    noteUpBtn.setBounds(boxX + 158, boxY + 78, 24, 26);

    triggerVelBox.setBounds(boxX + 188, boxY + 78, 88, 26);

    // Knobs Layout in Bottom Sections (y=230, h=275)
    auto placeKnob = [this](const juce::String& id, int x, int y, int w = 74, int h = 88) {
        if (controls.find(id) != controls.end())
        {
            controls[id].slider->setBounds(x, y + 16, w, h - 16);
            controls[id].label->setBounds(x - 2, y, w + 4, 16);
        }
    };

    // Section 1: Mallet & Strike (x=20)
    placeKnob("hardness", 32, 265, 74, 88);
    placeKnob("click", 120, 265, 74, 88);
    placeKnob("attack", 76, 375, 74, 88);

    // Section 2: Modal Resonator (x=225)
    placeKnob("decay", 240, 265, 74, 88);
    placeKnob("material", 330, 265, 74, 88);
    placeKnob("overtones", 240, 375, 74, 88);
    placeKnob("pipe", 330, 375, 74, 88);

    // Section 3: SEM Filter (x=490)
    placeKnob("cutoff", 502, 265, 74, 88);
    placeKnob("resonance", 592, 265, 74, 88);
    placeKnob("filterEnv", 547, 375, 74, 88);

    // Section 4: Master & FX (x=695)
    placeKnob("envDecay", 708, 260, 68, 80);
    placeKnob("release", 790, 260, 68, 80);
    placeKnob("spread", 708, 340, 68, 80);
    placeKnob("drive", 790, 340, 68, 80);
    placeKnob("ambience", 708, 420, 68, 80);
    placeKnob("volume", 790, 420, 68, 80);
}
