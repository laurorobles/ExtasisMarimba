#include "PluginProcessor.h"
#include "PluginEditor.h"

ExtasisMarimbaAudioProcessor::ExtasisMarimbaAudioProcessor()
    : AudioProcessor(BusesProperties()
                     .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Parameters", createParameterLayout())
{
}

ExtasisMarimbaAudioProcessor::~ExtasisMarimbaAudioProcessor()
{
}

juce::AudioProcessorValueTreeState::ParameterLayout ExtasisMarimbaAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // Mallet & Exciter
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "hardness", "Mallet Hardness", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.65f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "click", "Strike Click", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.35f));

    // Modal Resonator
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "decay", "Bar Decay", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.52f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "material", "Bar Material", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.12f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "overtones", "Overtone Mix", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.58f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "pipe", "Resonator Pipe", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.50f));

    // SEM Filter
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "cutoff", "Cutoff", juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 0.25f), 9500.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "resonance", "Resonance", juce::NormalisableRange<float>(0.0f, 0.95f, 0.01f), 0.15f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "filterEnv", "Filter Env Amt", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.40f));

    // Percussive Envelope
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "attack", "Attack", juce::NormalisableRange<float>(0.1f, 100.0f, 0.1f, 0.3f), 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "envDecay", "Decay", juce::NormalisableRange<float>(5.0f, 3000.0f, 1.0f, 0.35f), 800.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "sustain", "Sustain", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "release", "Release", juce::NormalisableRange<float>(5.0f, 2000.0f, 1.0f, 0.35f), 350.0f));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        "snap", "Snap Curve", true));

    // Master & FX
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "spread", "Stereo Spread", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.65f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "drive", "Warm Drive", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.15f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "ambience", "Body Ambience", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.20f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "volume", "Master Volume", juce::NormalisableRange<float>(0.0f, 1.5f, 0.01f), 0.85f));

    return { params.begin(), params.end() };
}

const juce::String ExtasisMarimbaAudioProcessor::getName() const { return "ExtasisMarimba"; }
bool ExtasisMarimbaAudioProcessor::acceptsMidi() const { return true; }
bool ExtasisMarimbaAudioProcessor::producesMidi() const { return false; }
bool ExtasisMarimbaAudioProcessor::isMidiEffect() const { return false; }
double ExtasisMarimbaAudioProcessor::getTailLengthSeconds() const { return 0.5; }

int ExtasisMarimbaAudioProcessor::getNumPrograms() { return static_cast<int>(presetNames.size()); }
int ExtasisMarimbaAudioProcessor::getCurrentProgram() { return currentPresetIndex; }
void ExtasisMarimbaAudioProcessor::setCurrentProgram(int index) { loadPreset(index); }
const juce::String ExtasisMarimbaAudioProcessor::getProgramName(int index)
{
    return (index >= 0 && index < (int)presetNames.size()) ? presetNames[index] : juce::String();
}
void ExtasisMarimbaAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
    if (index >= 0 && index < (int)presetNames.size())
        presetNames[index] = newName;
}

juce::String ExtasisMarimbaAudioProcessor::getPresetTitle(int index) const
{
    return (index >= 0 && index < (int)presetNames.size()) ? presetNames[index] : "Preset";
}

void ExtasisMarimbaAudioProcessor::triggerAuditionNote(int noteNumber, float velocity)
{
    int nextW = (fifoWriteIdx.load() + 1) % FIFO_SIZE;
    if (nextW != fifoReadIdx.load())
    {
        triggerFifo[fifoWriteIdx.load()] = { noteNumber, velocity, true };
        fifoWriteIdx.store(nextW);
    }
}

void ExtasisMarimbaAudioProcessor::releaseAuditionNote(int noteNumber)
{
    int nextW = (fifoWriteIdx.load() + 1) % FIFO_SIZE;
    if (nextW != fifoReadIdx.load())
    {
        triggerFifo[fifoWriteIdx.load()] = { noteNumber, 0.0f, false };
        fifoWriteIdx.store(nextW);
    }
}

void ExtasisMarimbaAudioProcessor::loadPreset(int presetIndex)
{
    if (presetIndex < 0 || presetIndex >= (int)presetNames.size())
        return;

    currentPresetIndex = presetIndex;

    auto setVal = [this](const juce::String& paramId, float val) {
        if (auto* param = apvts.getParameter(paramId))
            param->setValueNotifyingHost(param->getNormalisableRange().convertTo0to1(val));
    };

    switch (presetIndex)
    {
        case 0: // Concert Marimba
            setVal("hardness", 0.65f);
            setVal("click", 0.35f);
            setVal("decay", 0.52f);
            setVal("material", 0.12f);
            setVal("overtones", 0.58f);
            setVal("pipe", 0.50f);
            setVal("cutoff", 9500.0f);
            setVal("resonance", 0.15f);
            setVal("filterEnv", 0.40f);
            setVal("attack", 0.5f);
            setVal("envDecay", 800.0f);
            setVal("sustain", 0.0f);
            setVal("release", 350.0f);
            setVal("spread", 0.65f);
            setVal("drive", 0.15f);
            setVal("ambience", 0.20f);
            setVal("volume", 0.85f);
            break;
        case 1: // Wooden Rosewood Bar
            setVal("hardness", 0.45f);
            setVal("click", 0.20f);
            setVal("decay", 0.70f);
            setVal("material", 0.05f);
            setVal("overtones", 0.45f);
            setVal("pipe", 0.80f);
            setVal("cutoff", 8000.0f);
            setVal("resonance", 0.10f);
            setVal("filterEnv", 0.30f);
            setVal("attack", 0.8f);
            setVal("envDecay", 950.0f);
            setVal("sustain", 0.0f);
            setVal("release", 450.0f);
            setVal("spread", 0.80f);
            setVal("drive", 0.08f);
            setVal("ambience", 0.40f);
            setVal("volume", 0.85f);
            break;
        case 2: // African Balafon / Kalimba
            setVal("hardness", 0.80f);
            setVal("click", 0.55f);
            setVal("decay", 0.45f);
            setVal("material", 0.40f);
            setVal("overtones", 0.75f);
            setVal("pipe", 0.30f);
            setVal("cutoff", 12000.0f);
            setVal("resonance", 0.25f);
            setVal("filterEnv", 0.60f);
            setVal("attack", 0.3f);
            setVal("envDecay", 600.0f);
            setVal("sustain", 0.0f);
            setVal("release", 280.0f);
            setVal("spread", 0.50f);
            setVal("drive", 0.28f);
            setVal("ambience", 0.15f);
            setVal("volume", 0.85f);
            break;
        case 3: // Glass & Metal Bar
            setVal("hardness", 0.90f);
            setVal("click", 0.60f);
            setVal("decay", 0.85f);
            setVal("material", 0.95f);
            setVal("overtones", 0.85f);
            setVal("pipe", 0.20f);
            setVal("cutoff", 16000.0f);
            setVal("resonance", 0.30f);
            setVal("filterEnv", 0.50f);
            setVal("attack", 0.2f);
            setVal("envDecay", 1200.0f);
            setVal("sustain", 0.05f);
            setVal("release", 700.0f);
            setVal("spread", 0.75f);
            setVal("drive", 0.10f);
            setVal("ambience", 0.50f);
            setVal("volume", 0.80f);
            break;
        case 4: // Plucked Mallet Synth
            setVal("hardness", 0.50f);
            setVal("click", 0.40f);
            setVal("decay", 0.60f);
            setVal("material", 0.30f);
            setVal("overtones", 0.65f);
            setVal("pipe", 0.40f);
            setVal("cutoff", 6500.0f);
            setVal("resonance", 0.50f);
            setVal("filterEnv", 0.80f);
            setVal("attack", 1.0f);
            setVal("envDecay", 750.0f);
            setVal("sustain", 0.10f);
            setVal("release", 400.0f);
            setVal("spread", 0.60f);
            setVal("drive", 0.35f);
            setVal("ambience", 0.30f);
            setVal("volume", 0.85f);
            break;
    }
}

void ExtasisMarimbaAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    synthEngine.prepare(sampleRate, samplesPerBlock);
}

void ExtasisMarimbaAudioProcessor::releaseResources()
{
    synthEngine.reset();
}

bool ExtasisMarimbaAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    return true;
}

void ExtasisMarimbaAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // 1. Update synth parameters from APVTS
    synthEngine.setParameters(
        *apvts.getRawParameterValue("hardness"),
        *apvts.getRawParameterValue("click"),
        *apvts.getRawParameterValue("decay"),
        *apvts.getRawParameterValue("material"),
        *apvts.getRawParameterValue("overtones"),
        *apvts.getRawParameterValue("pipe"),
        *apvts.getRawParameterValue("cutoff"),
        *apvts.getRawParameterValue("resonance"),
        *apvts.getRawParameterValue("filterEnv"),
        *apvts.getRawParameterValue("attack"),
        *apvts.getRawParameterValue("envDecay"),
        *apvts.getRawParameterValue("sustain"),
        *apvts.getRawParameterValue("release"),
        *apvts.getRawParameterValue("snap") > 0.5f,
        *apvts.getRawParameterValue("spread"),
        *apvts.getRawParameterValue("drive"),
        *apvts.getRawParameterValue("ambience"),
        *apvts.getRawParameterValue("volume")
    );

    // 2. Consume Trigger events from GUI
    while (fifoReadIdx.load() != fifoWriteIdx.load())
    {
        auto ev = triggerFifo[fifoReadIdx.load()];
        if (ev.isNoteOn)
            synthEngine.noteOn(ev.note, ev.vel);
        else
            synthEngine.noteOff(ev.note);

        fifoReadIdx.store((fifoReadIdx.load() + 1) % FIFO_SIZE);
    }

    // 3. Process incoming MIDI messages
    for (const auto metadata : midiMessages)
    {
        const auto msg = metadata.getMessage();
        if (msg.isNoteOn())
        {
            synthEngine.noteOn(msg.getNoteNumber(), msg.getFloatVelocity());
        }
        else if (msg.isNoteOff())
        {
            synthEngine.noteOff(msg.getNoteNumber());
        }
        else if (msg.isAllNotesOff() || msg.isAllSoundOff())
        {
            synthEngine.allNotesOff();
        }
    }

    // 4. Render Synth Audio
    synthEngine.renderAudio(buffer, buffer.getNumSamples());

    // 5. Stream audio to Visualizer
    if (onAudioBlockProcessed != nullptr && buffer.getNumChannels() > 0)
    {
        onAudioBlockProcessed(buffer.getReadPointer(0), buffer.getNumSamples());
    }
}

bool ExtasisMarimbaAudioProcessor::hasEditor() const { return true; }
juce::AudioProcessorEditor* ExtasisMarimbaAudioProcessor::createEditor()
{
    return new ExtasisMarimbaAudioProcessorEditor(*this);
}

void ExtasisMarimbaAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void ExtasisMarimbaAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr)
    {
        if (xmlState->hasTagName(apvts.state.getType()))
        {
            apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
        }
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ExtasisMarimbaAudioProcessor();
}
