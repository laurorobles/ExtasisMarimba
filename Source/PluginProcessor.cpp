#include "PluginProcessor.h"
#include "PluginEditor.h"

ExtasisMarimbaAudioProcessor::ExtasisMarimbaAudioProcessor()
    : AudioProcessor(BusesProperties()
                     .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Parameters", createParameterLayout())
{
    checkLicenseState();
}

ExtasisMarimbaAudioProcessor::~ExtasisMarimbaAudioProcessor()
{
}

void ExtasisMarimbaAudioProcessor::checkLicenseState()
{
    isPluginLicensed = LicenseManager::isLicensed();
    if (isPluginLicensed)
    {
        isExpired = false;
    }
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

juce::StringArray ExtasisMarimbaAudioProcessor::getFactoryPresetNames()
{
    return {
        // Traditional Mexican & Chiapaneca (01 - 08)
        "01. Chiapas Cachimba Marimba",
        "02. Hormiguillo Fiesta Marimba",
        "03. Marimba de Pueblo (Vintage)",
        "04. Zapateado Veracruzano",
        "05. Son Jarocho Wooden Bar",
        "06. Tuxtla Gutierrez Twilight",
        "07. Oaxacan Village Mallet",
        "08. Chiapaneca Raw Membrane",

        // Latin Club, Cumbia & Bass (09 - 16)
        "09. Deep Latin Club Pluck",
        "10. Cumbia Rebajada Low End",
        "11. Hypnotic Cumbia Mallet",
        "12. Latin House Strike",
        "13. Tribal Guarachero Mallet",
        "14. Afro-Cuban Bembe Bar",
        "15. Reggaeton Mallet Lead",
        "16. Baile Funk Punch Bar",

        // Concert, Acoustic & Traditional World (17 - 22)
        "17. Clean Concert Rosewood",
        "18. Soft Felt Warmth",
        "19. Orchestra Symphony Marimba",
        "20. Wooden Xylophone Snap",
        "21. African Balafon / Kalimba",
        "22. Indonesian Gamelan Gourd",

        // Experimental, Glass & Hybrid FM (23 - 30)
        "23. Glass & Vibra Bars",
        "24. Ambient Dream Chimes",
        "25. Cyberpunk Neon Mallet",
        "26. 12-Bit Lo-Fi Marimbita",
        "27. Sub-Bass Thumper Marimba",
        "28. Micro-Tuned Village Market",
        "29. Space Echo Chamber Bar",
        "30. Extasis Anthem Marimba"
    };
}

juce::File ExtasisMarimbaAudioProcessor::getPresetsDirectory() const
{
    auto dir = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
        .getChildFile("ExtasisRecords")
        .getChildFile("ExtasisMarimba")
        .getChildFile("Presets");

    if (!dir.exists())
        dir.createDirectory();

    return dir;
}

juce::StringArray ExtasisMarimbaAudioProcessor::getAllPresetNames()
{
    juce::StringArray allPresets = getFactoryPresetNames();

    auto dir = getPresetsDirectory();
    if (dir.isDirectory())
    {
        juce::Array<juce::File> userFiles;
        dir.findChildFiles(userFiles, juce::File::findFiles, false, "*.empreset;*.xml");
        userFiles.sort();

        for (const auto& file : userFiles)
        {
            allPresets.add("[User] " + file.getFileNameWithoutExtension());
        }
    }

    return allPresets;
}

int ExtasisMarimbaAudioProcessor::getNumPrograms() { return getAllPresetNames().size(); }
int ExtasisMarimbaAudioProcessor::getCurrentProgram() { return currentProgramIndex; }
void ExtasisMarimbaAudioProcessor::setCurrentProgram(int index) { loadPreset(index); }
const juce::String ExtasisMarimbaAudioProcessor::getProgramName(int index)
{
    auto names = getAllPresetNames();
    return (index >= 0 && index < names.size()) ? names[index] : juce::String();
}
void ExtasisMarimbaAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
    juce::ignoreUnused(index, newName);
}

bool ExtasisMarimbaAudioProcessor::saveUserPreset(const juce::String& presetName)
{
    if (presetName.trim().isEmpty())
        return false;

    auto dir = getPresetsDirectory();
    auto file = dir.getChildFile(presetName.trim() + ".empreset");

    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    return xml->writeTo(file);
}

bool ExtasisMarimbaAudioProcessor::deleteUserPreset(const juce::String& presetName)
{
    auto dir = getPresetsDirectory();
    auto file = dir.getChildFile(presetName + ".empreset");
    if (file.existsAsFile())
        return file.deleteFile();
    return false;
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
    currentProgramIndex = presetIndex;

    auto setVal = [this](const juce::String& paramId, float val) {
        if (auto* param = apvts.getParameter(paramId))
            param->setValueNotifyingHost(param->getNormalisableRange().convertTo0to1(val));
    };

    // User Preset Loading
    if (presetIndex >= 30)
    {
        int userIndex = presetIndex - 30;
        auto dir = getPresetsDirectory();
        if (dir.isDirectory())
        {
            juce::Array<juce::File> userFiles;
            dir.findChildFiles(userFiles, juce::File::findFiles, false, "*.empreset;*.xml");
            userFiles.sort();

            if (userIndex >= 0 && userIndex < userFiles.size())
            {
                auto xml = juce::parseXML(userFiles[userIndex]);
                if (xml != nullptr && xml->hasTagName(apvts.state.getType()))
                {
                    apvts.replaceState(juce::ValueTree::fromXml(*xml));
                }
                return;
            }
        }
    }

    switch (presetIndex)
    {
        case 0: // 01. Chiapas Cachimba Marimba
            setVal("hardness", 0.65f); setVal("noise", 0.40f); setVal("click", 0.30f); setVal("attack", 0.5f);
            setVal("decay", 0.55f); setVal("material", 0.08f); setVal("overtones", 0.62f); setVal("organic", 0.50f);
            setVal("tube", 0.75f); setVal("buzz", 0.60f); setVal("buzzVel", 0.88f); setVal("cutoff", 11500.0f);
            setVal("spread", 0.65f); setVal("drive", 0.15f); setVal("ambience", 0.25f); setVal("volume", 0.85f);
            break;
        case 1: // 02. Hormiguillo Fiesta Marimba
            setVal("hardness", 0.75f); setVal("noise", 0.48f); setVal("click", 0.45f); setVal("attack", 0.3f);
            setVal("decay", 0.48f); setVal("material", 0.12f); setVal("overtones", 0.72f); setVal("organic", 0.40f);
            setVal("tube", 0.60f); setVal("buzz", 0.70f); setVal("buzzVel", 0.92f); setVal("cutoff", 13500.0f);
            setVal("spread", 0.70f); setVal("drive", 0.22f); setVal("ambience", 0.20f); setVal("volume", 0.85f);
            break;
        case 2: // 03. Marimba de Pueblo (Vintage)
            setVal("hardness", 0.55f); setVal("noise", 0.55f); setVal("click", 0.35f); setVal("attack", 0.6f);
            setVal("decay", 0.65f); setVal("material", 0.05f); setVal("overtones", 0.58f); setVal("organic", 0.85f);
            setVal("tube", 0.85f); setVal("buzz", 0.75f); setVal("buzzVel", 0.80f); setVal("cutoff", 9800.0f);
            setVal("spread", 0.60f); setVal("drive", 0.25f); setVal("ambience", 0.35f); setVal("volume", 0.85f);
            break;
        case 3: // 04. Zapateado Veracruzano
            setVal("hardness", 0.80f); setVal("noise", 0.42f); setVal("click", 0.50f); setVal("attack", 0.2f);
            setVal("decay", 0.40f); setVal("material", 0.09f); setVal("overtones", 0.68f); setVal("organic", 0.45f);
            setVal("tube", 0.65f); setVal("buzz", 0.55f); setVal("buzzVel", 0.90f); setVal("cutoff", 12500.0f);
            setVal("spread", 0.65f); setVal("drive", 0.18f); setVal("ambience", 0.22f); setVal("volume", 0.85f);
            break;
        case 4: // 05. Son Jarocho Wooden Bar
            setVal("hardness", 0.60f); setVal("noise", 0.38f); setVal("click", 0.28f); setVal("attack", 0.4f);
            setVal("decay", 0.52f); setVal("material", 0.06f); setVal("overtones", 0.55f); setVal("organic", 0.50f);
            setVal("tube", 0.70f); setVal("buzz", 0.40f); setVal("buzzVel", 0.75f); setVal("cutoff", 10500.0f);
            setVal("spread", 0.60f); setVal("drive", 0.12f); setVal("ambience", 0.28f); setVal("volume", 0.85f);
            break;
        case 5: // 06. Tuxtla Gutierrez Twilight
            setVal("hardness", 0.50f); setVal("noise", 0.30f); setVal("click", 0.20f); setVal("attack", 0.7f);
            setVal("decay", 0.75f); setVal("material", 0.04f); setVal("overtones", 0.50f); setVal("organic", 0.60f);
            setVal("tube", 0.90f); setVal("buzz", 0.45f); setVal("buzzVel", 0.70f); setVal("cutoff", 9000.0f);
            setVal("spread", 0.75f); setVal("drive", 0.10f); setVal("ambience", 0.45f); setVal("volume", 0.85f);
            break;
        case 6: // 07. Oaxacan Village Mallet
            setVal("hardness", 0.70f); setVal("noise", 0.50f); setVal("click", 0.40f); setVal("attack", 0.3f);
            setVal("decay", 0.58f); setVal("material", 0.10f); setVal("overtones", 0.64f); setVal("organic", 0.75f);
            setVal("tube", 0.75f); setVal("buzz", 0.65f); setVal("buzzVel", 0.85f); setVal("cutoff", 11000.0f);
            setVal("spread", 0.65f); setVal("drive", 0.20f); setVal("ambience", 0.30f); setVal("volume", 0.85f);
            break;
        case 7: // 08. Chiapaneca Raw Membrane
            setVal("hardness", 0.85f); setVal("noise", 0.60f); setVal("click", 0.55f); setVal("attack", 0.2f);
            setVal("decay", 0.45f); setVal("material", 0.15f); setVal("overtones", 0.78f); setVal("organic", 0.70f);
            setVal("tube", 0.60f); setVal("buzz", 0.95f); setVal("buzzVel", 0.98f); setVal("cutoff", 14500.0f);
            setVal("spread", 0.65f); setVal("drive", 0.28f); setVal("ambience", 0.18f); setVal("volume", 0.85f);
            break;
        case 8: // 09. Deep Latin Club Pluck
            setVal("hardness", 0.80f); setVal("noise", 0.60f); setVal("click", 0.50f); setVal("attack", 0.2f);
            setVal("decay", 0.42f); setVal("material", 0.20f); setVal("overtones", 0.75f); setVal("organic", 0.30f);
            setVal("tube", 0.80f); setVal("buzz", 0.45f); setVal("buzzVel", 0.95f); setVal("cutoff", 12000.0f);
            setVal("spread", 0.55f); setVal("drive", 0.35f); setVal("ambience", 0.15f); setVal("volume", 0.88f);
            break;
        case 9: // 10. Cumbia Rebajada Low End
            setVal("hardness", 0.50f); setVal("noise", 0.55f); setVal("click", 0.30f); setVal("attack", 0.6f);
            setVal("decay", 0.80f); setVal("material", 0.05f); setVal("overtones", 0.50f); setVal("organic", 0.55f);
            setVal("tube", 0.95f); setVal("buzz", 0.50f); setVal("buzzVel", 0.80f); setVal("cutoff", 7500.0f);
            setVal("spread", 0.50f); setVal("drive", 0.30f); setVal("ambience", 0.35f); setVal("volume", 0.90f);
            break;
        case 10: // 11. Hypnotic Cumbia Mallet
            setVal("hardness", 0.70f); setVal("noise", 0.45f); setVal("click", 0.40f); setVal("attack", 0.4f);
            setVal("decay", 0.50f); setVal("material", 0.15f); setVal("overtones", 0.65f); setVal("organic", 0.55f);
            setVal("tube", 0.70f); setVal("buzz", 0.55f); setVal("buzzVel", 0.85f); setVal("cutoff", 11000.0f);
            setVal("spread", 0.65f); setVal("drive", 0.24f); setVal("ambience", 0.22f); setVal("volume", 0.85f);
            break;
        case 11: // 12. Latin House Strike
            setVal("hardness", 0.82f); setVal("noise", 0.45f); setVal("click", 0.55f); setVal("attack", 0.2f);
            setVal("decay", 0.38f); setVal("material", 0.25f); setVal("overtones", 0.80f); setVal("organic", 0.35f);
            setVal("tube", 0.70f); setVal("buzz", 0.40f); setVal("buzzVel", 0.90f); setVal("cutoff", 13000.0f);
            setVal("spread", 0.70f); setVal("drive", 0.26f); setVal("ambience", 0.20f); setVal("volume", 0.86f);
            break;
        case 12: // 13. Tribal Guarachero Mallet
            setVal("hardness", 0.90f); setVal("noise", 0.65f); setVal("click", 0.65f); setVal("attack", 0.1f);
            setVal("decay", 0.32f); setVal("material", 0.30f); setVal("overtones", 0.85f); setVal("organic", 0.40f);
            setVal("tube", 0.55f); setVal("buzz", 0.65f); setVal("buzzVel", 0.95f); setVal("cutoff", 15000.0f);
            setVal("spread", 0.60f); setVal("drive", 0.38f); setVal("ambience", 0.12f); setVal("volume", 0.88f);
            break;
        case 13: // 14. Afro-Cuban Bembe Bar
            setVal("hardness", 0.75f); setVal("noise", 0.45f); setVal("click", 0.45f); setVal("attack", 0.3f);
            setVal("decay", 0.48f); setVal("material", 0.10f); setVal("overtones", 0.68f); setVal("organic", 0.50f);
            setVal("tube", 0.75f); setVal("buzz", 0.50f); setVal("buzzVel", 0.85f); setVal("cutoff", 11500.0f);
            setVal("spread", 0.65f); setVal("drive", 0.20f); setVal("ambience", 0.25f); setVal("volume", 0.85f);
            break;
        case 14: // 15. Reggaeton Mallet Lead
            setVal("hardness", 0.78f); setVal("noise", 0.35f); setVal("click", 0.48f); setVal("attack", 0.2f);
            setVal("decay", 0.44f); setVal("material", 0.18f); setVal("overtones", 0.70f); setVal("organic", 0.30f);
            setVal("tube", 0.65f); setVal("buzz", 0.35f); setVal("buzzVel", 0.88f); setVal("cutoff", 12500.0f);
            setVal("spread", 0.68f); setVal("drive", 0.22f); setVal("ambience", 0.28f); setVal("volume", 0.85f);
            break;
        case 15: // 16. Baile Funk Punch Bar
            setVal("hardness", 0.88f); setVal("noise", 0.55f); setVal("click", 0.60f); setVal("attack", 0.1f);
            setVal("decay", 0.35f); setVal("material", 0.22f); setVal("overtones", 0.82f); setVal("organic", 0.35f);
            setVal("tube", 0.80f); setVal("buzz", 0.60f); setVal("buzzVel", 0.95f); setVal("cutoff", 14000.0f);
            setVal("spread", 0.55f); setVal("drive", 0.35f); setVal("ambience", 0.15f); setVal("volume", 0.88f);
            break;
        case 16: // 17. Clean Concert Rosewood
            setVal("hardness", 0.45f); setVal("noise", 0.20f); setVal("click", 0.15f); setVal("attack", 0.8f);
            setVal("decay", 0.70f); setVal("material", 0.03f); setVal("overtones", 0.45f); setVal("organic", 0.15f);
            setVal("tube", 0.85f); setVal("buzz", 0.0f); setVal("buzzVel", 0.0f); setVal("cutoff", 8500.0f);
            setVal("spread", 0.80f); setVal("drive", 0.06f); setVal("ambience", 0.40f); setVal("volume", 0.85f);
            break;
        case 17: // 18. Soft Felt Warmth
            setVal("hardness", 0.25f); setVal("noise", 0.15f); setVal("click", 0.08f); setVal("attack", 1.2f);
            setVal("decay", 0.85f); setVal("material", 0.02f); setVal("overtones", 0.35f); setVal("organic", 0.20f);
            setVal("tube", 0.95f); setVal("buzz", 0.0f); setVal("buzzVel", 0.0f); setVal("cutoff", 6500.0f);
            setVal("spread", 0.75f); setVal("drive", 0.05f); setVal("ambience", 0.50f); setVal("volume", 0.85f);
            break;
        case 18: // 19. Orchestra Symphony Marimba
            setVal("hardness", 0.55f); setVal("noise", 0.25f); setVal("click", 0.22f); setVal("attack", 0.5f);
            setVal("decay", 0.72f); setVal("material", 0.04f); setVal("overtones", 0.52f); setVal("organic", 0.25f);
            setVal("tube", 0.90f); setVal("buzz", 0.08f); setVal("buzzVel", 0.40f); setVal("cutoff", 9500.0f);
            setVal("spread", 0.85f); setVal("drive", 0.08f); setVal("ambience", 0.48f); setVal("volume", 0.85f);
            break;
        case 19: // 20. Wooden Xylophone Snap
            setVal("hardness", 0.92f); setVal("noise", 0.40f); setVal("click", 0.65f); setVal("attack", 0.2f);
            setVal("decay", 0.28f); setVal("material", 0.08f); setVal("overtones", 0.75f); setVal("organic", 0.35f);
            setVal("tube", 0.40f); setVal("buzz", 0.15f); setVal("buzzVel", 0.60f); setVal("cutoff", 15000.0f);
            setVal("spread", 0.70f); setVal("drive", 0.14f); setVal("ambience", 0.22f); setVal("volume", 0.85f);
            break;
        case 20: // 21. African Balafon / Kalimba
            setVal("hardness", 0.85f); setVal("noise", 0.50f); setVal("click", 0.60f); setVal("attack", 0.3f);
            setVal("decay", 0.45f); setVal("material", 0.45f); setVal("overtones", 0.80f); setVal("organic", 0.65f);
            setVal("tube", 0.40f); setVal("buzz", 0.80f); setVal("buzzVel", 0.90f); setVal("cutoff", 14000.0f);
            setVal("spread", 0.50f); setVal("drive", 0.28f); setVal("ambience", 0.18f); setVal("volume", 0.85f);
            break;
        case 21: // 22. Indonesian Gamelan Gourd
            setVal("hardness", 0.80f); setVal("noise", 0.35f); setVal("click", 0.50f); setVal("attack", 0.3f);
            setVal("decay", 0.60f); setVal("material", 0.65f); setVal("overtones", 0.85f); setVal("organic", 0.55f);
            setVal("tube", 0.75f); setVal("buzz", 0.35f); setVal("buzzVel", 0.70f); setVal("cutoff", 12000.0f);
            setVal("spread", 0.65f); setVal("drive", 0.18f); setVal("ambience", 0.35f); setVal("volume", 0.85f);
            break;
        case 22: // 23. Glass & Vibra Bars
            setVal("hardness", 0.90f); setVal("noise", 0.15f); setVal("click", 0.55f); setVal("attack", 0.2f);
            setVal("decay", 0.88f); setVal("material", 0.95f); setVal("overtones", 0.88f); setVal("organic", 0.10f);
            setVal("tube", 0.30f); setVal("buzz", 0.0f); setVal("buzzVel", 0.0f); setVal("cutoff", 18000.0f);
            setVal("spread", 0.75f); setVal("drive", 0.08f); setVal("ambience", 0.55f); setVal("volume", 0.82f);
            break;
        case 23: // 24. Ambient Dream Chimes
            setVal("hardness", 0.65f); setVal("noise", 0.12f); setVal("click", 0.25f); setVal("attack", 0.8f);
            setVal("decay", 0.95f); setVal("material", 0.80f); setVal("overtones", 0.70f); setVal("organic", 0.30f);
            setVal("tube", 0.85f); setVal("buzz", 0.10f); setVal("buzzVel", 0.40f); setVal("cutoff", 14000.0f);
            setVal("spread", 0.90f); setVal("drive", 0.12f); setVal("ambience", 0.75f); setVal("volume", 0.80f);
            break;
        case 24: // 25. Cyberpunk Neon Mallet
            setVal("hardness", 0.95f); setVal("noise", 0.55f); setVal("click", 0.70f); setVal("attack", 0.1f);
            setVal("decay", 0.52f); setVal("material", 0.75f); setVal("overtones", 0.95f); setVal("organic", 0.25f);
            setVal("tube", 0.60f); setVal("buzz", 0.75f); setVal("buzzVel", 0.95f); setVal("cutoff", 16500.0f);
            setVal("spread", 0.70f); setVal("drive", 0.45f); setVal("ambience", 0.30f); setVal("volume", 0.85f);
            break;
        case 25: // 26. 12-Bit Lo-Fi Marimbita
            setVal("hardness", 0.70f); setVal("noise", 0.65f); setVal("click", 0.45f); setVal("attack", 0.4f);
            setVal("decay", 0.45f); setVal("material", 0.18f); setVal("overtones", 0.60f); setVal("organic", 0.70f);
            setVal("tube", 0.70f); setVal("buzz", 0.55f); setVal("buzzVel", 0.85f); setVal("cutoff", 7000.0f);
            setVal("spread", 0.50f); setVal("drive", 0.40f); setVal("ambience", 0.20f); setVal("volume", 0.86f);
            break;
        case 26: // 27. Sub-Bass Thumper Marimba
            setVal("hardness", 0.65f); setVal("noise", 0.50f); setVal("click", 0.40f); setVal("attack", 0.3f);
            setVal("decay", 0.70f); setVal("material", 0.08f); setVal("overtones", 0.45f); setVal("organic", 0.40f);
            setVal("tube", 1.00f); setVal("buzz", 0.30f); setVal("buzzVel", 0.75f); setVal("cutoff", 6000.0f);
            setVal("spread", 0.35f); setVal("drive", 0.38f); setVal("ambience", 0.15f); setVal("volume", 0.92f);
            break;
        case 27: // 28. Micro-Tuned Village Market
            setVal("hardness", 0.72f); setVal("noise", 0.55f); setVal("click", 0.48f); setVal("attack", 0.3f);
            setVal("decay", 0.58f); setVal("material", 0.12f); setVal("overtones", 0.66f); setVal("organic", 1.00f);
            setVal("tube", 0.80f); setVal("buzz", 0.70f); setVal("buzzVel", 0.88f); setVal("cutoff", 11000.0f);
            setVal("spread", 0.65f); setVal("drive", 0.22f); setVal("ambience", 0.32f); setVal("volume", 0.85f);
            break;
        case 28: // 29. Space Echo Chamber Bar
            setVal("hardness", 0.60f); setVal("noise", 0.30f); setVal("click", 0.35f); setVal("attack", 0.5f);
            setVal("decay", 0.82f); setVal("material", 0.35f); setVal("overtones", 0.72f); setVal("organic", 0.45f);
            setVal("tube", 0.85f); setVal("buzz", 0.40f); setVal("buzzVel", 0.80f); setVal("cutoff", 10000.0f);
            setVal("spread", 0.85f); setVal("drive", 0.18f); setVal("ambience", 0.80f); setVal("volume", 0.82f);
            break;
        case 29: // 30. Extasis Anthem Marimba
            setVal("hardness", 0.80f); setVal("noise", 0.45f); setVal("click", 0.50f); setVal("attack", 0.2f);
            setVal("decay", 0.60f); setVal("material", 0.15f); setVal("overtones", 0.75f); setVal("organic", 0.50f);
            setVal("tube", 0.85f); setVal("buzz", 0.65f); setVal("buzzVel", 0.90f); setVal("cutoff", 13000.0f);
            setVal("spread", 0.75f); setVal("drive", 0.25f); setVal("ambience", 0.30f); setVal("volume", 0.88f);
            break;
    }
}

void ExtasisMarimbaAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    synthEngine.prepare(sampleRate, samplesPerBlock);
    demoSampleCount = 0;
    checkLicenseState();
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

    // Check License / Demo Timer (10 minutes = 600s)
    if (!isPluginLicensed)
    {
        demoSampleCount += buffer.getNumSamples();
        if (demoSampleCount > static_cast<int>(currentSampleRate * 600.0))
        {
            isExpired = true;
            buffer.clear();
            return;
        }
    }

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
        0.15f,  // resonance
        0.40f,  // filterEnvAmt
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
