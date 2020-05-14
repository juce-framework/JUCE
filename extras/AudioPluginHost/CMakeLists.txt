# ==============================================================================
#
#  This file is part of the JUCE 6 technical preview.
#  Copyright (c) 2020 - Raw Material Software Limited
#
#  You may use this code under the terms of the GPL v3
#  (see www.gnu.org/licenses).
#
#  For this technical preview, this file is not subject to commercial licensing.
#
#  JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
#  EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
#  DISCLAIMED.
#
# ==============================================================================

juce_add_gui_app(AudioPluginHost
    BUNDLE_ID                       com.juce.pluginhost
    ICON_BIG                        "${CMAKE_CURRENT_SOURCE_DIR}/Source/JUCEAppIcon.png"
    MICROPHONE_PERMISSION_ENABLED   TRUE
    PLUGINHOST_AU                   TRUE)

juce_generate_juce_header(AudioPluginHost)

target_sources(AudioPluginHost PRIVATE
    Source/HostStartup.cpp
    Source/Plugins/IOConfigurationWindow.cpp
    Source/Plugins/InternalPlugins.cpp
    Source/Plugins/PluginGraph.cpp
    Source/UI/GraphEditorPanel.cpp
    Source/UI/MainHostWindow.cpp)

target_compile_definitions(AudioPluginHost PRIVATE
    PIP_JUCE_EXAMPLES_DIRECTORY_STRING="${JUCE_SOURCE_DIR}/examples"
    JUCE_ALSA=1
    JUCE_DIRECTSOUND=1
    JUCE_PLUGINHOST_LADSPA=1
    JUCE_PLUGINHOST_VST=0
    JUCE_PLUGINHOST_VST3=1
    JUCE_USE_CAMERA=0
    JUCE_USE_CDBURNER=0
    JUCE_USE_CDREADER=0
    JUCE_USE_CURL=0
    JUCE_USE_FLAC=0
    JUCE_USE_OGGVORBIS=1
    JUCE_WASAPI=1
    JUCE_WEB_BROWSER=0)

target_link_libraries(AudioPluginHost PRIVATE
    juce::juce_audio_utils
    juce::juce_cryptography
    juce::juce_dsp
    juce::juce_opengl
    juce::juce_video
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags)

juce_add_bundle_resources_directory(AudioPluginHost ../../examples/Assets)
