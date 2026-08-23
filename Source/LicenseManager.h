#pragma once
#include <juce_core/juce_core.h>

class LicenseManager
{
public:
    static constexpr uint64_t SALT_1 = 0x8C3F5E7B1A9D4E20ULL;
    static constexpr uint64_t SALT_2 = 0x5D9B2A1E7F8C4362ULL;

    // Returns the license file path in AppData / Application Support
    static juce::File getLicenseFile()
    {
#if JUCE_MAC
        return juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
            .getChildFile ("Application Support")
            .getChildFile ("ExtasisMarimba")
            .getChildFile ("license.key");
#else
        return juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
            .getChildFile ("ExtasisMarimba")
            .getChildFile ("license.key");
#endif
    }

    // Validates a serial key in format "EXTM-XXXX-XXXX-XXXX-XXXX"
    static bool validateSerial (juce::String serialKey)
    {
        serialKey = serialKey.trim().toUpperCase().removeCharacters ("- \t\r\n");
        if (serialKey.startsWith ("EXTM"))
            serialKey = serialKey.substring (4);

        if (serialKey.length() != 16)
            return false;

        juce::String block1 = serialKey.substring (0, 4);
        juce::String block2 = serialKey.substring (4, 8);
        juce::String block3 = serialKey.substring (8, 12);
        juce::String block4 = serialKey.substring (12, 16);

        uint32_t val1 = (uint32_t) block1.getHexValue32();
        uint32_t val2 = (uint32_t) block2.getHexValue32();
        uint32_t val3 = (uint32_t) block3.getHexValue32();
        uint32_t val4 = (uint32_t) block4.getHexValue32();

        if (val1 == 0 && val2 == 0 && val3 == 0)
            return false;

        uint64_t seed = ((uint64_t)val1 << 32) | val1;
        uint32_t expected2 = (uint32_t)(((seed ^ SALT_1) * 0x45D9F3BULL) >> 16) & 0xFFFF;
        uint32_t expected3 = (uint32_t)((((seed << 13) | (seed >> 19)) ^ SALT_2) * 0x27D4EB2DULL >> 16) & 0xFFFF;
        uint32_t expected4 = ((val1 ^ expected2 ^ expected3 ^ 0xBEEF) * 0x119DE1ULL) & 0xFFFF;

        return (val2 == expected2 && val3 == expected3 && val4 == expected4);
    }

    // Checks if the plugin is currently registered/licensed on this machine
    static bool isLicensed()
    {
        auto licFile = getLicenseFile();
        if (!licFile.existsAsFile())
            return false;

        juce::String content = licFile.loadFileAsString().trim();
        return validateSerial (content);
    }

    // Saves the license key to disk
    static bool saveLicense (const juce::String& serialKey)
    {
        if (!validateSerial (serialKey))
            return false;

        auto licFile = getLicenseFile();
        licFile.getParentDirectory().createDirectory();
        return licFile.replaceWithText (serialKey.trim().toUpperCase());
    }

    // Removes license
    static void removeLicense()
    {
        auto licFile = getLicenseFile();
        if (licFile.existsAsFile())
            licFile.deleteFile();
    }
};
