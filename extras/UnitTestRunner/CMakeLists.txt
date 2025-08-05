# ==============================================================================
#
#  This file is part of the JUCE framework.
#  Copyright (c) Raw Material Software Limited
#
#  JUCE is an open source framework subject to commercial or open source
#  licensing.
#
#  By downloading, installing, or using the JUCE framework, or combining the
#  JUCE framework with any other source code, object code, content or any other
#  copyrightable work, you agree to the terms of the JUCE End User Licence
#  Agreement, and all incorporated terms including the JUCE Privacy Policy and
#  the JUCE Website Terms of Service, as applicable, which will bind you. If you
#  do not agree to the terms of these agreements, we will not license the JUCE
#  framework to you, and you must discontinue the installation or download
#  process and cease use of the JUCE framework.
#
#  JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
#  JUCE Privacy Policy: https://juce.com/juce-privacy-policy
#  JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/
#
#  Or:
#
#  You may also use this code under the terms of the AGPLv3:
#  https://www.gnu.org/licenses/agpl-3.0.en.html
#
#  THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
#  WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
#  MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.
#
# ==============================================================================

juce_add_console_app(UnitTestRunner)

juce_generate_juce_header(UnitTestRunner)

target_sources(UnitTestRunner PRIVATE Source/Main.cpp)

target_compile_definitions(UnitTestRunner PRIVATE
    JUCE_PLUGINHOST_AU=1
    JUCE_PLUGINHOST_LV2=1
    JUCE_PLUGINHOST_VST3=1
    JUCE_UNIT_TESTS=1
    JUCE_USE_CURL=0
    JUCE_WEB_BROWSER=0
    # This is a temporary workaround to allow builds to complete on Xcode 15.
    # Add -Wl,-ld_classic to the OTHER_LDFLAGS build setting if you need to
    # deploy to older versions of macOS.
    JUCE_SILENCE_XCODE_15_LINKER_WARNING=1)

target_link_libraries(UnitTestRunner PRIVATE
    juce::juce_analytics
    juce::juce_audio_utils
    juce::juce_build_tools
    juce::juce_dsp
    juce::juce_midi_ci
    juce::juce_opengl
    juce::juce_osc
    juce::juce_product_unlocking
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags)
