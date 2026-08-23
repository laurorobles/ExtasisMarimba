cmake_minimum_required(VERSION 3.15)
project(ExtasisMarimba VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# JUCE integration
add_subdirectory(/Users/babyonk1/.gemini/antigravity/scratch/JUCE ${CMAKE_BINARY_DIR}/JUCE)

juce_add_binary_data(ExtasisMarimbaData
    SOURCES
        assets/logo.png
)

juce_add_plugin(ExtasisMarimba
    COMPANY_NAME "Extasis Records"
    BUNDLE_ID "com.laurorobles.ExtasisMarimba"
    PLUGIN_MANUFACTURER_CODE Exta
    PLUGIN_CODE Emrb
    FORMATS VST3 AU Standalone
    PRODUCT_NAME "ExtasisMarimba"
    ICON_BIG assets/logo.png
    ICON_SMALL assets/logo.png
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
)

juce_generate_juce_header(ExtasisMarimba)

target_sources(ExtasisMarimba PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h
    Source/DSP/MalletExciter.h
    Source/DSP/ModalBarResonator.h
    Source/DSP/SEMFilter.h
    Source/DSP/PercussiveEnvelope.h
    Source/DSP/StereoBodyProcessor.h
    Source/DSP/MarimbaSynthEngine.h
    Source/GUI/MarimbaLookAndFeel.h
    Source/GUI/MarimbaLookAndFeel.cpp
    Source/GUI/MarimbaDisplay.h
    Source/GUI/MarimbaDisplay.cpp
    Source/GUI/MarimbaTriggerButton.h
    Source/GUI/MarimbaTriggerButton.cpp
)

target_compile_definitions(ExtasisMarimba PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(ExtasisMarimba PRIVATE
    ExtasisMarimbaData
    juce::juce_audio_utils
    juce::juce_audio_processors
    juce::juce_dsp
    juce::juce_gui_basics
    juce::juce_gui_extra
)

# Custom post-build step to guarantee installation and app icon
add_custom_command(TARGET ExtasisMarimba_Standalone POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/assets/icon.icns"
        "$<TARGET_BUNDLE_DIR:ExtasisMarimba_Standalone>/Contents/Resources/Icon.icns"
    COMMENT "Copying icon.icns to Standalone app bundle"
)
