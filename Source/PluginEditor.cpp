#include "PluginEditor.h"

ExtasisMarimbaAudioProcessorEditor::ExtasisMarimbaAudioProcessorEditor(ExtasisMarimbaAudioProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    setLookAndFeel(&customLookAndFeel);

    // 1. Display Setup
    addAndMakeVisible(display);

    // 2. Preset Selector
    for (int i = 0; i < processorRef.getPresetCount(); ++i)
    {
        presetBox.addItem(processorRef.getPresetTitle(i), i + 1);
    }
    presetBox.setSelectedId(processorRef.getCurrentProgram() + 1, juce::dontSendNotification);
    presetBox.addListener(this);
    addAndMakeVisible(presetBox);

    prevPresetBtn.onClick = [this]() {
        int cur = processorRef.getCurrentProgram();
        int count = processorRef.getPresetCount();
        int next = (cur - 1 + count) % count;
        processorRef.setCurrentProgram(next);
        presetBox.setSelectedId(next + 1, juce::dontSendNotification);
        display.setPresetName(processorRef.getPresetTitle(next));
    };
    addAndMakeVisible(prevPresetBtn);

    nextPresetBtn.onClick = [this]() {
        int cur = processorRef.getCurrentProgram();
        int count = processorRef.getPresetCount();
        int next = (cur + 1) % count;
        processorRef.setCurrentProgram(next);
        presetBox.setSelectedId(next + 1, juce::dontSendNotification);
        display.setPresetName(processorRef.getPresetTitle(next));
    };
    addAndMakeVisible(nextPresetBtn);

    // 3. Trigger & Test Note Setup (Like ExtasisDonk)
    triggerButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff2a2215));
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
        { 36, "C2 (36) Deep Bar" },
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
    triggerVelBox.addItem("VEL: 100%", 1);
    triggerVelBox.addItem("VEL: 80%", 2);
    triggerVelBox.addItem("VEL: 60%", 3);
    triggerVelBox.addItem("VEL: 40%", 4);
    triggerVelBox.setSelectedId(1, juce::dontSendNotification);
    triggerVelBox.addListener(this);
    addAndMakeVisible(triggerVelBox);

    // 4. Create all Rotary Controls
    // Mallet Section
    createKnob("hardness", "HARDNESS", "%");
    createKnob("click", "CLICK", "%");
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
    createKnob("drive", "DRIVE", "%");
    createKnob("ambience", "AMBIENCE", "%");
    createKnob("volume", "VOLUME", "");

    setSize(880, 530);
}

ExtasisMarimbaAudioProcessorEditor::~ExtasisMarimbaAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
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
    display.setParameterReadout("TRIGGER NOTE", triggerNoteBox.getText() + " (VEL " + juce::String(int(currentTriggerVel * 100)) + "%)");
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
        display.setPresetName(processorRef.getPresetTitle(idx));
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
    // Background Dark Chassis
    g.fillAll(ExtasisGUI::MarimbaLookAndFeel::getBackgroundDark());

    // Top Brand Bar
    auto topBar = getLocalBounds().removeFromTop(48);
    g.setColour(ExtasisGUI::MarimbaLookAndFeel::getPanelBackground());
    g.fillRect(topBar);

    g.setColour(ExtasisGUI::MarimbaLookAndFeel::getPanelBorder());
    g.drawHorizontalLine(48, 0, (float)getWidth());

    // Logo & Subtitle
    g.setFont(juce::Font(18.0f, juce::Font::bold));
    g.setColour(ExtasisGUI::MarimbaLookAndFeel::getAmberGold());
    g.drawText("EXTASIS MARIMBA", 18, 2, 190, 44, juce::Justification::centredLeft);

    // Cyan/Gold divider accent
    g.setColour(ExtasisGUI::MarimbaLookAndFeel::getAmberGold().withAlpha(0.6f));
    g.fillRect(205, 14, 2, 20);

    g.setFont(juce::Font(10.5f, juce::Font::plain));
    g.setColour(juce::Colour(0xff8899aa));
    g.drawText("PHYSICAL MODELING SYNTH // MICROFREAK ENGINE", 215, 2, 310, 44, juce::Justification::centredLeft);

    // Panel Sections Backgrounds
    auto drawSection = [&](juce::Rectangle<int> rect, const juce::String& title) {
        g.setColour(ExtasisGUI::MarimbaLookAndFeel::getPanelBackground());
        g.fillRoundedRectangle(rect.toFloat(), 6.0f);
        
        g.setColour(ExtasisGUI::MarimbaLookAndFeel::getPanelBorder());
        g.drawRoundedRectangle(rect.toFloat(), 6.0f, 1.2f);

        auto headerRect = rect.removeFromTop(24);
        g.setColour(juce::Colour(0x1a2d313a));
        g.fillRoundedRectangle(headerRect.toFloat(), 4.0f);

        g.setFont(juce::Font(11.0f, juce::Font::bold));
        g.setColour(ExtasisGUI::MarimbaLookAndFeel::getAmberGold());
        g.drawText(title, headerRect.reduced(8, 0), juce::Justification::centredLeft, false);
    };

    // 4 Modular Sections (Width 880)
    drawSection(juce::Rectangle<int>(16, 178, 190, 336), "1. MALLET & STRIKE");
    drawSection(juce::Rectangle<int>(216, 178, 246, 336), "2. MODAL RESONATOR");
    drawSection(juce::Rectangle<int>(472, 178, 190, 336), "3. SEM FILTER");
    drawSection(juce::Rectangle<int>(672, 178, 192, 336), "4. MASTER & FX");
}

void ExtasisMarimbaAudioProcessorEditor::resized()
{
    // Preset Selector on Top Right
    int rightEdge = getWidth() - 16;
    nextPresetBtn.setBounds(rightEdge - 28, 10, 28, 28);
    prevPresetBtn.setBounds(rightEdge - 58, 10, 28, 28);
    presetBox.setBounds(rightEdge - 270, 10, 208, 28);

    // Top Center Display & Trigger Bar
    // Display: left side (width = 540)
    display.setBounds(16, 56, 540, 110);

    // Trigger Box on the right side of the Display (x = 566, width = 298, height = 110)
    int trigX = 566;
    int trigY = 56;
    
    triggerButton.setBounds(trigX, trigY, 110, 48);
    
    // Note Selector Controls
    noteDownBtn.setBounds(trigX + 118, trigY, 26, 26);
    triggerNoteBox.setBounds(trigX + 148, trigY, 122, 26);
    noteUpBtn.setBounds(trigX + 274, trigY, 26, 26);

    // Velocity Selector
    triggerVelBox.setBounds(trigX + 118, trigY + 30, 182, 26);

    // Trigger Quick Info Label
    // (Additional decorative button/status under trigger)

    // Knobs Layout inside sections
    auto layoutKnob = [this](const juce::String& id, int x, int y, int w = 74, int h = 88) {
        if (controls.find(id) != controls.end())
        {
            controls[id].slider->setBounds(x, y + 16, w, h - 16);
            controls[id].label->setBounds(x - 2, y, w + 4, 16);
        }
    };

    // Section 1: Mallet & Strike (x=16, y=178)
    layoutKnob("hardness", 28, 212, 74, 88);
    layoutKnob("click", 116, 212, 74, 88);
    layoutKnob("attack", 72, 322, 74, 88);

    // Section 2: Modal Resonator (x=216, y=178)
    layoutKnob("decay", 228, 212, 74, 88);
    layoutKnob("material", 316, 212, 74, 88);
    layoutKnob("overtones", 228, 322, 74, 88);
    layoutKnob("pipe", 316, 322, 74, 88);

    // Section 3: SEM Filter (x=472, y=178)
    layoutKnob("cutoff", 484, 212, 74, 88);
    layoutKnob("resonance", 572, 212, 74, 88);
    layoutKnob("filterEnv", 528, 322, 74, 88);

    // Section 4: Master & FX (x=672, y=178)
    layoutKnob("envDecay", 682, 212, 74, 88);
    layoutKnob("release", 772, 212, 74, 88);
    layoutKnob("spread", 682, 312, 74, 88);
    layoutKnob("drive", 772, 312, 74, 88);
    layoutKnob("ambience", 682, 412, 74, 88);
    layoutKnob("volume", 772, 412, 74, 88);
}
