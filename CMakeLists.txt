cmake_minimum_required(VERSION 3.22)
project(HappyDrivePreamps VERSION 1.0.0)

include(FetchContent)
FetchContent_Declare(JUCE
  GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
  GIT_TAG 8.0.4)
FetchContent_MakeAvailable(JUCE)

option(HD_COPY_PLUGIN "Copier le plugin apres compilation" ON)

if(APPLE)
  set(HD_FORMATS VST3 AU Standalone)
else()
  set(HD_FORMATS VST3 Standalone)
endif()

juce_add_plugin(HappyDrive
  COMPANY_NAME "HappyAudio"
  PLUGIN_MANUFACTURER_CODE Hapy
  PLUGIN_CODE Hdpr
  FORMATS ${HD_FORMATS}
  PRODUCT_NAME "Happy Drive Preamps"
  IS_SYNTH FALSE
  NEEDS_MIDI_INPUT FALSE
  NEEDS_MIDI_OUTPUT FALSE
  COPY_PLUGIN_AFTER_BUILD ${HD_COPY_PLUGIN})

target_sources(HappyDrive PRIVATE
  Source/PluginProcessor.cpp
  Source/PluginEditor.cpp)

target_compile_definitions(HappyDrive PUBLIC
  JUCE_WEB_BROWSER=0
  JUCE_USE_CURL=0
  JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(HappyDrive
  PRIVATE juce::juce_audio_utils juce::juce_dsp
  PUBLIC juce::juce_recommended_config_flags
         juce::juce_recommended_lto_flags
         juce::juce_recommended_warning_flags)
