cmake_minimum_required(VERSION 3.22)

project(OutOfThyme VERSION 0.0.0)

# Add JUCE
if(NOT DEFINED JUCE_PATH)
    set(JUCE_PATH "$ENV{USERPROFILE}/JUCE")
endif()

if(EXISTS "${JUCE_PATH}/CMakeLists.txt")
    add_subdirectory("${JUCE_PATH}" JUCE)
else()
    find_package(JUCE CONFIG REQUIRED)
endif()

juce_add_plugin(OutOfThyme
    COMPANY_NAME "joohnnyvv"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE "john"
    PLUGIN_CODE "Thym"
    FORMATS VST3 Standalone
)

juce_generate_juce_header(OutOfThyme)

juce_add_binary_data(OutOfThymeData SOURCES Source/Assets/bg.png)

target_sources(OutOfThyme
    PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h
    Source/TapeEngine.cpp
    Source/TapeEngine.h
)

target_compile_definitions(OutOfThyme
    PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(OutOfThyme
    PRIVATE
    juce::juce_audio_utils
    juce::juce_audio_processors
    juce::juce_audio_formats
    juce::juce_audio_devices
    juce::juce_audio_basics
    juce::juce_graphics
    juce::juce_gui_basics
    juce::juce_gui_extra
    juce::juce_data_structures
    juce::juce_events
    juce::juce_core
    juce::juce_dsp
    OutOfThymeData
)

# Set C++ standard
set_target_properties(OutOfThyme PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)
