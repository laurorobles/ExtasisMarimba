#include "PluginEditor.h"

ExtasisMarimbaAudioProcessorEditor::ExtasisMarimbaAudioProcessorEditor(ExtasisMarimbaAudioProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    setLookAndFeel(&customLookAndFeel);

    // 1. Display
    addAndMakeVisible(display);

    // 2. Preset Box & Buttons
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

    // 3. Create all rotary controls
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

    setSize(840, 520);
}

ExtasisMarimbaAudioProcessorEditor::~ExtasisMarimbaAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
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
            display.setParameterReadout(pair.second.label->getText(), slider->getTextFromValue(slider->getValue()));
            
            if (pair.first == "hardness" || pair.first == "decay")
            {
                float h = static_cast<float>(controls["hardness"].slider->getValue());
                float d = static_cast<float>(controls["decay"].slider->getValue());
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
}

void ExtasisMarimbaAudioProcessorEditor::paint(juce::Graphics& g)
{
    // Background Dark Chassis
    g.fillAll(ExtasisGUI::MarimbaLookAndFeel::getBackgroundDark());

    // Top Brand Bar
    auto topBar = getLocalBounds().removeFromTop(44);
    g.setColour(ExtasisGUI::MarimbaLookAndFeel::getPanelBackground());
    g.fillRect(topBar);

    g.setColour(ExtasisGUI::MarimbaLookAndFeel::getPanelBorder());
    g.drawHorizontalLine(44, 0, (float)getWidth());

    // Logo & Subtitle
    g.setFont(juce::Font(18.0f, juce::Font::bold));
    g.setColour(ExtasisGUI::MarimbaLookAndFeel::getAmberGold());
    g.drawText("EXTASIS MARIMBA", 18, 0, 200, 44, juce::Justification::centredLeft);

    g.setFont(juce::Font(11.0f, juce::Font::plain));
    g.setColour(juce::Colour(0xff8899aa));
    g.drawText("PHYSICAL MODELING SYNTH // MICROFREAK ENGINE", 220, 0, 300, 44, juce::Justification::centredLeft);

    // Panel Sections Backgrounds
    auto drawSection = [&](juce::Rectangle<int> rect, const juce::String& title) {
        g.setColour(ExtasisGUI::MarimbaLookAndFeel::getPanelBackground());
        g.fillRoundedRectangle(rect.toFloat(), 6.0f);
        
        g.setColour(ExtasisGUI::MarimbaLookAndFeel::getPanelBorder());
        g.drawRoundedRectangle(rect.toFloat(), 6.0f, 1.2f);

        auto headerRect = rect.removeFromTop(24);
        g.setColour(juce::Colour(0x20ffa834));
        g.fillRoundedRectangle(headerRect.toFloat(), 4.0f);

        g.setFont(juce::Font(11.5f, juce::Font::bold));
        g.setColour(ExtasisGUI::MarimbaLookAndFeel::getAmberGold());
        g.drawText(title, headerRect.reduced(8, 0), juce::Justification::centredLeft, false);
    };

    // 4 Modular Sections
    drawSection(juce::Rectangle<int>(16, 170, 180, 335), "1. MALLET & STRIKE");
    drawSection(juce::Rectangle<int>(206, 170, 240, 335), "2. MODAL RESONATOR");
    drawSection(juce::Rectangle<int>(456, 170, 180, 335), "3. SEM FILTER");
    drawSection(juce::Rectangle<int>(646, 170, 178, 335), "4. MASTER & FX");
}

void ExtasisMarimbaAudioProcessorEditor::resized()
{
    // Preset Selector on Top Right
    int rightEdge = getWidth() - 16;
    nextPresetBtn.setBounds(rightEdge - 30, 8, 30, 28);
    prevPresetBtn.setBounds(rightEdge - 64, 8, 30, 28);
    presetBox.setBounds(rightEdge - 275, 8, 205, 28);

    // Top Center Display
    display.setBounds(16, 52, getWidth() - 32, 108);

    // Knobs Layout inside sections
    auto layoutKnob = [this](const juce::String& id, int x, int y, int w = 72, int h = 88) {
        if (controls.find(id) != controls.end())
        {
            controls[id].slider->setBounds(x, y + 16, w, h - 16);
            controls[id].label->setBounds(x, y, w, 16);
        }
    };

    // Section 1: Mallet & Strike (x=16, y=170)
    layoutKnob("hardness", 28, 205, 72, 88);
    layoutKnob("click", 112, 205, 72, 88);
    layoutKnob("attack", 70, 315, 72, 88);

    // Section 2: Modal Resonator (x=206, y=170)
    layoutKnob("decay", 220, 205, 72, 88);
    layoutKnob("material", 304, 205, 72, 88);
    layoutKnob("overtones", 220, 315, 72, 88);
    layoutKnob("pipe", 304, 315, 72, 88);

    // Section 3: SEM Filter (x=456, y=170)
    layoutKnob("cutoff", 468, 205, 72, 88);
    layoutKnob("resonance", 552, 205, 72, 88);
    layoutKnob("filterEnv", 510, 315, 72, 88);

    // Section 4: Master & FX (x=646, y=170)
    layoutKnob("envDecay", 654, 205, 72, 88);
    layoutKnob("release", 740, 205, 72, 88);
    layoutKnob("spread", 654, 305, 72, 88);
    layoutKnob("drive", 740, 305, 72, 88);
    layoutKnob("ambience", 654, 405, 72, 88);
    layoutKnob("volume", 740, 405, 72, 88);
}
