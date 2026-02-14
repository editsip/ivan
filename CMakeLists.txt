cmake_minimum_required(VERSION 3.22)
project(Ivan VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Fetch JUCE
include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.4
)
FetchContent_MakeAvailable(JUCE)

# Plugin target
juce_add_plugin(Ivan
    COMPANY_NAME           "editsip"
    IS_SYNTH               FALSE
    NEEDS_MIDI_INPUT       FALSE
    NEEDS_MIDI_OUTPUT      FALSE
    IS_MIDI_EFFECT         FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE Edsp
    PLUGIN_CODE             Ivan
    FORMATS                 AU VST3 Standalone
    PRODUCT_NAME            "Ivan"
    BUNDLE_ID               "com.editsip.ivan"
    MICROPHONE_PERMISSION_ENABLED TRUE
    MICROPHONE_PERMISSION_TEXT "Ivan needs microphone access for live input processing."
)

# Source files
target_sources(Ivan PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h
    Source/UI/IvanLookAndFeel.cpp
    Source/UI/IvanLookAndFeel.h
    Source/UI/KnobComponent.cpp
    Source/UI/KnobComponent.h
    Source/DSP/TubeStage.cpp
    Source/DSP/TubeStage.h
    Source/DSP/ToneStack.cpp
    Source/DSP/ToneStack.h
    Source/DSP/PowerAmp.cpp
    Source/DSP/PowerAmp.h
    Source/DSP/CabinetSim.cpp
    Source/DSP/CabinetSim.h
    Source/DSP/Compressor.cpp
    Source/DSP/Compressor.h
    Source/DSP/NoiseGate.cpp
    Source/DSP/NoiseGate.h
    Source/DSP/Oversampling.h
    Source/DSP/SignalChain.cpp
    Source/DSP/SignalChain.h
)

target_compile_definitions(Ivan PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)

target_link_libraries(Ivan
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
