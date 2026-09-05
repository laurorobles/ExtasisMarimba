#include "LicenseManager.h"
#include "PluginProcessor.h"
#include "PluginEditor.h"

ExtasisMarimbaAudioProcessor::ExtasisMarimbaAudioProcessor()
    : AudioProcessor(BusesProperties()
                     .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Parameters", createParameterLayout())
{
    apvts.state = juce::ValueTree("Parameters");
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

    // 1. La Madera
    params.push_back(std::make_unique<juce::AudioParameterFloat>("hardness", "Hardness", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.65f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("overtones", "Overtones", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.50f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("material", "Material", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.50f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("click", "Click / Snap", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.30f));

    // 2. El Cajon
    params.push_back(std::make_unique<juce::AudioParameterFloat>("tube", "Tube Cavity", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.70f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("decay", "Bar Decay", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.50f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("buzz", "Membrane Buzz", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.50f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("buzzVel", "Buzz Dynamics", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.80f));

    // 3. El Entorno
    params.push_back(std::make_unique<juce::AudioParameterFloat>("organic", "Organic Drift", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.40f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("ambience", "Ambience", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.30f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("spread", "Spread", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.50f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("volume", "Master Vol", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.85f));

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
        "09. Mexican Symphony Grand",
        "10. Pure Rosewood Concert",

        // Latin Club, Cumbia & Bass (11 - 20)
        "11. Deep Latin Club Pluck",
        "12. Cumbia Rebajada Low End",
        "13. Hypnotic Cumbia Mallet",
        "14. Latin House Strike",
        "15. Tribal Guarachero Mallet",
        "16. Afro-Cuban Bembe Bar",
        "17. Reggaeton Mallet Lead",
        "18. Baile Funk Punch Bar",
        "19. Moombahton Wood Sub",
        "20. Tropical House Pluck",

        // Concert, Acoustic & Traditional World (21 - 30)
        "21. Clean Concert Rosewood",
        "22. Soft Felt Warmth",
        "23. Orchestra Symphony Marimba",
        "24. Wooden Xylophone Snap",
        "25. African Balafon / Kalimba",
        "26. Indonesian Gamelan Gourd",
        "27. Japanese Bamboo Mallet",
        "28. Cinematic Suspense Hit",
        "29. Orchestral Glissando Wood",
        "30. Mellow Yarn Mallet",

        // Experimental, Glass & Hybrid FM (31 - 40)
        "31. Glass & Vibra Bars",
        "32. Ambient Dream Chimes",
        "33. Cyberpunk Neon Mallet",
        "34. 12-Bit Lo-Fi Marimbita",
        "35. Sub-Bass Thumper Marimba",
        "36. Micro-Tuned Village Market",
        "37. Space Echo Chamber Bar",
        "38. Extasis Anthem Marimba",
        "39. Frozen Cavern Crystals",
        "40. Granular Destroyed Wood" 
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
        // --- Traditional Mexican & Chiapaneca (01 - 10) ---
        case 0: // 01. Chiapas Cachimba Marimba 
            setVal("hardness", 0.65f); setVal("overtones", 0.40f); setVal("material", 0.52f); setVal("click", 0.35f);
            setVal("tube", 0.85f); setVal("decay", 0.55f); setVal("buzz", 0.60f); setVal("buzzVel", 0.65f);
            setVal("organic", 0.45f); setVal("ambience", 0.25f); setVal("spread", 0.60f); setVal("volume", 0.85f);
            break;
        case 1: // 02. Hormiguillo Fiesta Marimba 
            setVal("hardness", 0.80f); setVal("overtones", 0.60f); setVal("material", 0.48f); setVal("click", 0.55f);
            setVal("tube", 0.70f); setVal("decay", 0.45f); setVal("buzz", 0.50f); setVal("buzzVel", 0.75f);
            setVal("organic", 0.35f); setVal("ambience", 0.35f); setVal("spread", 0.70f); setVal("volume", 0.85f);
            break;
        case 2: // 03. Marimba de Pueblo (Vintage) 
            setVal("hardness", 0.60f); setVal("overtones", 0.35f); setVal("material", 0.80f); setVal("click", 0.45f);
            setVal("tube", 0.60f); setVal("decay", 0.30f); setVal("buzz", 0.85f); setVal("buzzVel", 0.30f);
            setVal("organic", 0.95f); setVal("ambience", 0.40f); setVal("spread", 0.25f); setVal("volume", 0.88f);
            break;
        case 3: // 04. Zapateado Veracruzano 
            setVal("hardness", 0.95f); setVal("overtones", 0.75f); setVal("material", 0.60f); setVal("click", 0.80f);
            setVal("tube", 0.50f); setVal("decay", 0.25f); setVal("buzz", 0.75f); setVal("buzzVel", 0.55f);
            setVal("organic", 0.50f); setVal("ambience", 0.15f); setVal("spread", 0.50f); setVal("volume", 0.85f);
            break;
        case 4: // 05. Son Jarocho Wooden Bar 
            setVal("hardness", 0.75f); setVal("overtones", 0.55f); setVal("material", 0.55f); setVal("click", 0.40f);
            setVal("tube", 0.40f); setVal("decay", 0.40f); setVal("buzz", 0.40f); setVal("buzzVel", 0.70f);
            setVal("organic", 0.65f); setVal("ambience", 0.30f); setVal("spread", 0.65f); setVal("volume", 0.85f);
            break;
        case 5: // 06. Tuxtla Gutierrez Twilight 
            setVal("hardness", 0.35f); setVal("overtones", 0.25f); setVal("material", 0.45f); setVal("click", 0.15f);
            setVal("tube", 0.95f); setVal("decay", 0.75f); setVal("buzz", 0.30f); setVal("buzzVel", 0.85f);
            setVal("organic", 0.40f); setVal("ambience", 0.55f); setVal("spread", 0.80f); setVal("volume", 0.85f);
            break;
        case 6: // 07. Oaxacan Village Mallet 
            setVal("hardness", 0.70f); setVal("overtones", 0.45f); setVal("material", 0.65f); setVal("click", 0.60f);
            setVal("tube", 0.80f); setVal("decay", 0.50f); setVal("buzz", 0.55f); setVal("buzzVel", 0.50f);
            setVal("organic", 0.85f); setVal("ambience", 0.35f); setVal("spread", 0.40f); setVal("volume", 0.85f);
            break;
        case 7: // 08. Chiapaneca Raw Membrane 
            setVal("hardness", 0.85f); setVal("overtones", 0.65f); setVal("material", 0.50f); setVal("click", 0.50f);
            setVal("tube", 0.90f); setVal("decay", 0.45f); setVal("buzz", 1.0f); setVal("buzzVel", 0.40f);
            setVal("organic", 0.50f); setVal("ambience", 0.20f); setVal("spread", 0.60f); setVal("volume", 0.85f);
            break;
        case 8: // 09. Mexican Symphony Grand
            setVal("hardness", 0.55f); setVal("overtones", 0.35f); setVal("material", 0.50f); setVal("click", 0.25f);
            setVal("tube", 0.95f); setVal("decay", 0.70f); setVal("buzz", 0.10f); setVal("buzzVel", 0.95f);
            setVal("organic", 0.10f); setVal("ambience", 0.50f); setVal("spread", 0.85f); setVal("volume", 0.85f);
            break;
        case 9: // 10. Pure Rosewood Concert
            setVal("hardness", 0.45f); setVal("overtones", 0.30f); setVal("material", 0.50f); setVal("click", 0.15f);
            setVal("tube", 1.00f); setVal("decay", 0.85f); setVal("buzz", 0.00f); setVal("buzzVel", 1.00f);
            setVal("organic", 0.05f); setVal("ambience", 0.40f); setVal("spread", 0.75f); setVal("volume", 0.85f);
            break;

        // --- Latin Club, Cumbia & Bass (11 - 20) ---
        case 10: // 11. Deep Latin Club Pluck 
            setVal("hardness", 0.90f); setVal("overtones", 0.85f); setVal("material", 0.40f); setVal("click", 0.70f);
            setVal("tube", 0.75f); setVal("decay", 0.20f); setVal("buzz", 0.0f); setVal("buzzVel", 1.0f);
            setVal("organic", 0.05f); setVal("ambience", 0.15f); setVal("spread", 0.30f); setVal("volume", 0.90f);
            break;
        case 11: // 12. Cumbia Rebajada Low End 
            setVal("hardness", 0.20f); setVal("overtones", 0.10f); setVal("material", 0.50f); setVal("click", 0.10f);
            setVal("tube", 1.0f); setVal("decay", 0.85f); setVal("buzz", 0.15f); setVal("buzzVel", 0.90f);
            setVal("organic", 0.20f); setVal("ambience", 0.25f); setVal("spread", 0.50f); setVal("volume", 0.90f);
            break;
        case 12: // 13. Hypnotic Cumbia Mallet 
            setVal("hardness", 0.60f); setVal("overtones", 0.50f); setVal("material", 0.50f); setVal("click", 0.40f);
            setVal("tube", 0.80f); setVal("decay", 0.45f); setVal("buzz", 0.25f); setVal("buzzVel", 0.80f);
            setVal("organic", 0.15f); setVal("ambience", 0.30f); setVal("spread", 0.45f); setVal("volume", 0.88f);
            break;
        case 13: // 14. Latin House Strike 
            setVal("hardness", 1.0f); setVal("overtones", 0.95f); setVal("material", 0.60f); setVal("click", 0.90f);
            setVal("tube", 0.60f); setVal("decay", 0.30f); setVal("buzz", 0.0f); setVal("buzzVel", 1.0f);
            setVal("organic", 0.0f); setVal("ambience", 0.40f); setVal("spread", 0.95f); setVal("volume", 0.88f);
            break;
        case 14: // 15. Tribal Guarachero Mallet 
            setVal("hardness", 0.95f); setVal("overtones", 0.70f); setVal("material", 0.85f); setVal("click", 0.85f);
            setVal("tube", 0.70f); setVal("decay", 0.25f); setVal("buzz", 0.45f); setVal("buzzVel", 0.60f);
            setVal("organic", 0.10f); setVal("ambience", 0.10f); setVal("spread", 0.25f); setVal("volume", 0.88f);
            break;
        case 15: // 16. Afro-Cuban Bembe Bar 
            setVal("hardness", 0.55f); setVal("overtones", 0.35f); setVal("material", 0.55f); setVal("click", 0.45f);
            setVal("tube", 0.85f); setVal("decay", 0.50f); setVal("buzz", 0.10f); setVal("buzzVel", 0.90f);
            setVal("organic", 0.40f); setVal("ambience", 0.25f); setVal("spread", 0.50f); setVal("volume", 0.85f);
            break;
        case 16: // 17. Reggaeton Mallet Lead 
            setVal("hardness", 0.85f); setVal("overtones", 0.80f); setVal("material", 0.50f); setVal("click", 0.75f);
            setVal("tube", 0.75f); setVal("decay", 0.35f); setVal("buzz", 0.0f); setVal("buzzVel", 1.0f);
            setVal("organic", 0.05f); setVal("ambience", 0.20f); setVal("spread", 0.65f); setVal("volume", 0.88f);
            break;
        case 17: // 18. Baile Funk Punch Bar 
            setVal("hardness", 1.0f); setVal("overtones", 0.90f); setVal("material", 0.40f); setVal("click", 1.0f);
            setVal("tube", 0.50f); setVal("decay", 0.15f); setVal("buzz", 0.20f); setVal("buzzVel", 0.70f);
            setVal("organic", 0.0f); setVal("ambience", 0.05f); setVal("spread", 0.20f); setVal("volume", 0.92f);
            break;
        case 18: // 19. Moombahton Wood Sub
            setVal("hardness", 0.40f); setVal("overtones", 0.20f); setVal("material", 0.50f); setVal("click", 0.20f);
            setVal("tube", 1.00f); setVal("decay", 0.60f); setVal("buzz", 0.05f); setVal("buzzVel", 0.90f);
            setVal("organic", 0.00f); setVal("ambience", 0.15f); setVal("spread", 0.40f); setVal("volume", 0.90f);
            break;
        case 19: // 20. Tropical House Pluck
            setVal("hardness", 0.90f); setVal("overtones", 0.80f); setVal("material", 0.45f); setVal("click", 0.80f);
            setVal("tube", 0.65f); setVal("decay", 0.30f); setVal("buzz", 0.00f); setVal("buzzVel", 1.00f);
            setVal("organic", 0.05f); setVal("ambience", 0.35f); setVal("spread", 0.80f); setVal("volume", 0.88f);
            break;

        // --- Concert, Acoustic & Traditional World (21 - 30) ---
        case 20: // 21. Clean Concert Rosewood 
            setVal("hardness", 0.40f); setVal("overtones", 0.25f); setVal("material", 0.50f); setVal("click", 0.15f);
            setVal("tube", 0.95f); setVal("decay", 0.65f); setVal("buzz", 0.0f); setVal("buzzVel", 1.0f);
            setVal("organic", 0.02f); setVal("ambience", 0.45f); setVal("spread", 0.80f); setVal("volume", 0.85f);
            break;
        case 21: // 22. Soft Felt Warmth 
            setVal("hardness", 0.10f); setVal("overtones", 0.05f); setVal("material", 0.50f); setVal("click", 0.05f);
            setVal("tube", 1.0f); setVal("decay", 0.80f); setVal("buzz", 0.0f); setVal("buzzVel", 1.0f);
            setVal("organic", 0.05f); setVal("ambience", 0.50f); setVal("spread", 0.75f); setVal("volume", 0.88f);
            break;
        case 22: // 23. Orchestra Symphony Marimba 
            setVal("hardness", 0.65f); setVal("overtones", 0.40f); setVal("material", 0.50f); setVal("click", 0.35f);
            setVal("tube", 0.85f); setVal("decay", 0.60f); setVal("buzz", 0.0f); setVal("buzzVel", 1.0f);
            setVal("organic", 0.0f); setVal("ambience", 0.40f); setVal("spread", 0.85f); setVal("volume", 0.85f);
            break;
        case 23: // 24. Wooden Xylophone Snap 
            setVal("hardness", 0.90f); setVal("overtones", 0.85f); setVal("material", 0.60f); setVal("click", 0.80f);
            setVal("tube", 0.30f); setVal("decay", 0.25f); setVal("buzz", 0.0f); setVal("buzzVel", 1.0f);
            setVal("organic", 0.15f); setVal("ambience", 0.20f); setVal("spread", 0.40f); setVal("volume", 0.85f);
            break;
        case 24: // 25. African Balafon / Kalimba 
            setVal("hardness", 0.75f); setVal("overtones", 0.60f); setVal("material", 0.75f); setVal("click", 0.65f);
            setVal("tube", 0.65f); setVal("decay", 0.35f); setVal("buzz", 0.30f); setVal("buzzVel", 0.60f);
            setVal("organic", 0.80f); setVal("ambience", 0.15f); setVal("spread", 0.50f); setVal("volume", 0.85f);
            break;
        case 25: // 26. Indonesian Gamelan Gourd 
            setVal("hardness", 0.85f); setVal("overtones", 0.90f); setVal("material", 0.95f); setVal("click", 0.85f);
            setVal("tube", 0.80f); setVal("decay", 0.55f); setVal("buzz", 0.0f); setVal("buzzVel", 1.0f);
            setVal("organic", 0.50f); setVal("ambience", 0.40f); setVal("spread", 0.60f); setVal("volume", 0.85f);
            break;
        case 26: // 27. Japanese Bamboo Mallet
            setVal("hardness", 0.80f); setVal("overtones", 0.75f); setVal("material", 0.70f); setVal("click", 0.75f);
            setVal("tube", 0.55f); setVal("decay", 0.30f); setVal("buzz", 0.00f); setVal("buzzVel", 1.00f);
            setVal("organic", 0.10f); setVal("ambience", 0.35f); setVal("spread", 0.60f); setVal("volume", 0.85f);
            break;
        case 27: // 28. Cinematic Suspense Hit
            setVal("hardness", 0.95f); setVal("overtones", 0.50f); setVal("material", 0.40f); setVal("click", 0.90f);
            setVal("tube", 0.85f); setVal("decay", 0.90f); setVal("buzz", 0.10f); setVal("buzzVel", 0.80f);
            setVal("organic", 0.20f); setVal("ambience", 0.80f); setVal("spread", 1.00f); setVal("volume", 0.88f);
            break;
        case 28: // 29. Orchestral Glissando Wood
            setVal("hardness", 0.60f); setVal("overtones", 0.45f); setVal("material", 0.50f); setVal("click", 0.35f);
            setVal("tube", 0.90f); setVal("decay", 0.75f); setVal("buzz", 0.00f); setVal("buzzVel", 1.00f);
            setVal("organic", 0.05f); setVal("ambience", 0.60f); setVal("spread", 0.90f); setVal("volume", 0.85f);
            break;
        case 29: // 30. Mellow Yarn Mallet
            setVal("hardness", 0.15f); setVal("overtones", 0.15f); setVal("material", 0.50f); setVal("click", 0.10f);
            setVal("tube", 0.90f); setVal("decay", 0.60f); setVal("buzz", 0.00f); setVal("buzzVel", 1.00f);
            setVal("organic", 0.00f); setVal("ambience", 0.35f); setVal("spread", 0.65f); setVal("volume", 0.86f);
            break;

        // --- Experimental, Glass & Hybrid FM (31 - 40) ---
        case 30: // 31. Glass & Vibra Bars 
            setVal("hardness", 0.45f); setVal("overtones", 1.0f); setVal("material", 0.50f); setVal("click", 0.20f);
            setVal("tube", 0.90f); setVal("decay", 0.90f); setVal("buzz", 0.0f); setVal("buzzVel", 1.0f);
            setVal("organic", 0.0f); setVal("ambience", 0.60f); setVal("spread", 0.80f); setVal("volume", 0.85f);
            break;
        case 31: // 32. Ambient Dream Chimes 
            setVal("hardness", 0.15f); setVal("overtones", 0.20f); setVal("material", 0.55f); setVal("click", 0.0f);
            setVal("tube", 1.0f); setVal("decay", 1.0f); setVal("buzz", 0.0f); setVal("buzzVel", 1.0f);
            setVal("organic", 0.10f); setVal("ambience", 1.0f); setVal("spread", 0.95f); setVal("volume", 0.85f);
            break;
        case 32: // 33. Cyberpunk Neon Mallet 
            setVal("hardness", 1.0f); setVal("overtones", 1.0f); setVal("material", 0.35f); setVal("click", 1.0f);
            setVal("tube", 0.50f); setVal("decay", 0.35f); setVal("buzz", 0.10f); setVal("buzzVel", 0.85f);
            setVal("organic", 0.0f); setVal("ambience", 0.25f); setVal("spread", 0.70f); setVal("volume", 0.85f);
            break;
        case 33: // 34. 12-Bit Lo-Fi Marimbita 
            setVal("hardness", 0.50f); setVal("overtones", 0.30f); setVal("material", 0.60f); setVal("click", 0.80f);
            setVal("tube", 0.45f); setVal("decay", 0.15f); setVal("buzz", 0.40f); setVal("buzzVel", 0.30f);
            setVal("organic", 0.85f); setVal("ambience", 0.0f); setVal("spread", 0.0f); setVal("volume", 0.85f);
            break;
        case 34: // 35. Sub-Bass Thumper Marimba 
            setVal("hardness", 0.05f); setVal("overtones", 0.0f); setVal("material", 0.50f); setVal("click", 0.10f);
            setVal("tube", 1.0f); setVal("decay", 0.60f); setVal("buzz", 0.0f); setVal("buzzVel", 1.0f);
            setVal("organic", 0.0f); setVal("ambience", 0.10f); setVal("spread", 0.20f); setVal("volume", 0.92f);
            break;
        case 35: // 36. Micro-Tuned Village Market 
            setVal("hardness", 0.65f); setVal("overtones", 0.45f); setVal("material", 0.75f); setVal("click", 0.55f);
            setVal("tube", 0.75f); setVal("decay", 0.45f); setVal("buzz", 0.90f); setVal("buzzVel", 0.15f);
            setVal("organic", 1.0f); setVal("ambience", 0.25f); setVal("spread", 0.45f); setVal("volume", 0.88f);
            break;
        case 36: // 37. Space Echo Chamber Bar 
            setVal("hardness", 0.55f); setVal("overtones", 0.55f); setVal("material", 0.50f); setVal("click", 0.40f);
            setVal("tube", 0.85f); setVal("decay", 0.80f); setVal("buzz", 0.20f); setVal("buzzVel", 0.85f);
            setVal("organic", 0.25f); setVal("ambience", 0.95f); setVal("spread", 0.85f); setVal("volume", 0.85f);
            break;
        case 37: // 38. Extasis Anthem Marimba 
            setVal("hardness", 0.80f); setVal("overtones", 0.60f); setVal("material", 0.50f); setVal("click", 0.60f);
            setVal("tube", 0.80f); setVal("decay", 0.50f); setVal("buzz", 0.40f); setVal("buzzVel", 0.70f);
            setVal("organic", 0.15f); setVal("ambience", 0.35f); setVal("spread", 0.75f); setVal("volume", 0.88f);
            break;
        case 38: // 39. Frozen Cavern Crystals
            setVal("hardness", 0.25f); setVal("overtones", 0.90f); setVal("material", 0.30f); setVal("click", 0.15f);
            setVal("tube", 0.95f); setVal("decay", 0.95f); setVal("buzz", 0.00f); setVal("buzzVel", 1.00f);
            setVal("organic", 0.05f); setVal("ambience", 0.90f); setVal("spread", 1.00f); setVal("volume", 0.85f);
            break;
        case 39: // 40. Granular Destroyed Wood
            setVal("hardness", 1.00f); setVal("overtones", 0.85f); setVal("material", 0.95f); setVal("click", 1.00f);
            setVal("tube", 0.25f); setVal("decay", 0.10f); setVal("buzz", 1.00f); setVal("buzzVel", 0.10f);
            setVal("organic", 1.00f); setVal("ambience", 0.05f); setVal("spread", 0.15f); setVal("volume", 0.85f);
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
        *apvts.getRawParameterValue("overtones"),
        *apvts.getRawParameterValue("material"),
        *apvts.getRawParameterValue("click"),
        *apvts.getRawParameterValue("tube"),
        *apvts.getRawParameterValue("decay"),
        *apvts.getRawParameterValue("buzz"),
        *apvts.getRawParameterValue("buzzVel"),
        *apvts.getRawParameterValue("organic"),
        *apvts.getRawParameterValue("ambience"),
        *apvts.getRawParameterValue("spread"),
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
