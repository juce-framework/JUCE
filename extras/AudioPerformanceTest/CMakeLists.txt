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

juce_add_gui_app(AudioPerformanceTest BUNDLE_ID com.juce.AudioPerformanceTest)

juce_generate_juce_header(AudioPerformanceTest)

target_sources(AudioPerformanceTest PRIVATE
    Source/Main.cpp)

target_compile_definitions(AudioPerformanceTest PRIVATE
    JUCE_USE_CURL=0 JUCE_WEB_BROWSER=0)

target_link_libraries(AudioPerformanceTest PRIVATE
    juce::juce_audio_utils
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags)
