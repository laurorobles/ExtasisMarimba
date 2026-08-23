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

    // 1. Mallet & Attack
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "hardness", "Mallet Hardness", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.65f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "noise", "Rubber Noise", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.40f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "click", "Strike Click", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.30f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "attack", "Attack Time", juce::NormalisableRange<float>(0.1f, 50.0f, 0.1f, 0.35f), 0.5f));

    // 2. Wood & Modal FM
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "decay", "Bar Decay", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.55f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "material", "Bar Material", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.10f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "overtones", "Overtone Mix", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.60f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "organic", "Organic Drift", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.45f));

    // 3. Resonator & Buzz (La Cachimba)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "tube", "Resonator Tube", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.70f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "buzz", "Cachimba Buzz", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.50f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "buzzVel", "Buzz Velocity", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.85f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "cutoff", "SEM Filter", juce::NormalisableRange<float>(100.0f, 20000.0f, 1.0f, 0.25f), 11000.0f));

    // 4. Master & FX
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "spread", "Stereo Spread", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.65f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "drive", "Warm Drive", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.15f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "ambience", "Body Ambience", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.25f));
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
        case 0: // 01. Chiapas Cachimba Marimba
            setVal("hardness", 0.65f);
            setVal("noise", 0.40f);
            setVal("click", 0.30f);
            setVal("attack", 0.5f);
            setVal("decay", 0.55f);
            setVal("material", 0.08f);
            setVal("overtones", 0.62f);
            setVal("organic", 0.50f);
            setVal("tube", 0.75f);
            setVal("buzz", 0.60f);
            setVal("buzzVel", 0.88f);
            setVal("cutoff", 11500.0f);
            setVal("spread", 0.65f);
            setVal("drive", 0.15f);
            setVal("ambience", 0.25f);
            setVal("volume", 0.85f);
            break;

        case 1: // 02. Hormiguillo Fiesta Marimba
            setVal("hardness", 0.75f);
            setVal("noise", 0.48f);
            setVal("click", 0.45f);
            setVal("attack", 0.3f);
            setVal("decay", 0.48f);
            setVal("material", 0.12f);
            setVal("overtones", 0.72f);
            setVal("organic", 0.40f);
            setVal("tube", 0.60f);
            setVal("buzz", 0.70f);
            setVal("buzzVel", 0.92f);
            setVal("cutoff", 13500.0f);
            setVal("spread", 0.70f);
            setVal("drive", 0.22f);
            setVal("ambience", 0.20f);
            setVal("volume", 0.85f);
            break;

        case 2: // 03. Marimba de Pueblo (Vintage)
            setVal("hardness", 0.55f);
            setVal("noise", 0.55f);
            setVal("click", 0.35f);
            setVal("attack", 0.6f);
            setVal("decay", 0.65f);
            setVal("material", 0.05f);
            setVal("overtones", 0.58f);
            setVal("organic", 0.85f);
            setVal("tube", 0.85f);
            setVal("buzz", 0.75f);
            setVal("buzzVel", 0.80f);
            setVal("cutoff", 9800.0f);
            setVal("spread", 0.60f);
            setVal("drive", 0.25f);
            setVal("ambience", 0.35f);
            setVal("volume", 0.85f);
            break;

        case 3: // 04. Deep Latin Club Pluck
            setVal("hardness", 0.80f);
            setVal("noise", 0.60f);
            setVal("click", 0.50f);
            setVal("attack", 0.2f);
            setVal("decay", 0.42f);
            setVal("material", 0.20f);
            setVal("overtones", 0.75f);
            setVal("organic", 0.30f);
            setVal("tube", 0.80f);
            setVal("buzz", 0.45f);
            setVal("buzzVel", 0.95f);
            setVal("cutoff", 12000.0f);
            setVal("spread", 0.55f);
            setVal("drive", 0.35f);
            setVal("ambience", 0.15f);
            setVal("volume", 0.88f);
            break;

        case 4: // 05. Clean Concert Rosewood
            setVal("hardness", 0.45f);
            setVal("noise", 0.20f);
            setVal("click", 0.15f);
            setVal("attack", 0.8f);
            setVal("decay", 0.70f);
            setVal("material", 0.03f);
            setVal("overtones", 0.45f);
            setVal("organic", 0.15f);
            setVal("tube", 0.85f);
            setVal("buzz", 0.0f); // Zero buzz
            setVal("buzzVel", 0.0f);
            setVal("cutoff", 8500.0f);
            setVal("spread", 0.80f);
            setVal("drive", 0.06f);
            setVal("ambience", 0.40f);
            setVal("volume", 0.85f);
            break;

        case 5: // 06. African Balafon / Kalimba
            setVal("hardness", 0.85f);
            setVal("noise", 0.50f);
            setVal("click", 0.60f);
            setVal("attack", 0.3f);
            setVal("decay", 0.45f);
            setVal("material", 0.45f);
            setVal("overtones", 0.80f);
            setVal("organic", 0.65f);
            setVal("tube", 0.40f);
            setVal("buzz", 0.80f);
            setVal("buzzVel", 0.90f);
            setVal("cutoff", 14000.0f);
            setVal("spread", 0.50f);
            setVal("drive", 0.28f);
            setVal("ambience", 0.18f);
            setVal("volume", 0.85f);
            break;

        case 6: // 07. Glass & Vibra Bars
            setVal("hardness", 0.90f);
            setVal("noise", 0.15f);
            setVal("click", 0.55f);
            setVal("attack", 0.2f);
            setVal("decay", 0.88f);
            setVal("material", 0.95f);
            setVal("overtones", 0.88f);
            setVal("organic", 0.10f);
            setVal("tube", 0.30f);
            setVal("buzz", 0.0f);
            setVal("buzzVel", 0.0f);
            setVal("cutoff", 18000.0f);
            setVal("spread", 0.75f);
            setVal("drive", 0.08f);
            setVal("ambience", 0.55f);
            setVal("volume", 0.82f);
            break;

        case 7: // 08. Hypnotic Cumbia Mallet
            setVal("hardness", 0.70f);
            setVal("noise", 0.45f);
            setVal("click", 0.40f);
            setVal("attack", 0.4f);
            setVal("decay", 0.50f);
            setVal("material", 0.15f);
            setVal("overtones", 0.65f);
            setVal("organic", 0.55f);
            setVal("tube", 0.70f);
            setVal("buzz", 0.55f);
            setVal("buzzVel", 0.85f);
            setVal("cutoff", 11000.0f);
            setVal("spread", 0.65f);
            setVal("drive", 0.24f);
            setVal("ambience", 0.22f);
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
        *apvts.getRawParameterValue("noise"),
        *apvts.getRawParameterValue("click"),
        *apvts.getRawParameterValue("attack"),
        *apvts.getRawParameterValue("decay"),
        *apvts.getRawParameterValue("material"),
        *apvts.getRawParameterValue("overtones"),
        *apvts.getRawParameterValue("organic"),
        *apvts.getRawParameterValue("tube"),
        *apvts.getRawParameterValue("buzz"),
        *apvts.getRawParameterValue("buzzVel"),
        *apvts.getRawParameterValue("cutoff"),
        0.15f, // resonance
        0.40f, // filterEnvAmt
        850.0f, // decayMs
        0.0f,   // sustain
        380.0f, // release
        true,   // snap
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
