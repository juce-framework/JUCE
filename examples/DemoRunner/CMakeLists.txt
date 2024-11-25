# ==============================================================================
#
#  This file is part of the JUCE framework examples.
#  Copyright (c) Raw Material Software Limited
#
#  The code included in this file is provided under the terms of the ISC license
#  http://www.isc.org/downloads/software-support-policy/isc-license. Permission
#  to use, copy, modify, and/or distribute this software for any purpose with or
#  without fee is hereby granted provided that the above copyright notice and
#  this permission notice appear in all copies.
#
#  THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
#  REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
#  AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
#  INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
#  LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
#  OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
#  PERFORMANCE OF THIS SOFTWARE.
#
# ==============================================================================

juce_add_gui_app(DemoRunner
    BUNDLE_ID                       com.juce.demorunner
    ICON_BIG                        ${CMAKE_CURRENT_SOURCE_DIR}/Source/JUCEAppIcon.png
    NEEDS_CURL                      TRUE
    NEEDS_WEB_BROWSER               TRUE
    MICROPHONE_PERMISSION_ENABLED   TRUE
    CAMERA_PERMISSION_ENABLED       TRUE
    BLUETOOTH_PERMISSION_ENABLED    TRUE
    FILE_SHARING_ENABLED            TRUE
    DOCUMENT_BROWSER_ENABLED        TRUE
    REQUIRES_FULL_SCREEN            FALSE
    IPAD_SCREEN_ORIENTATIONS        UIInterfaceOrientationPortrait
                                    UIInterfaceOrientationPortraitUpsideDown
                                    UIInterfaceOrientationLandscapeLeft
                                    UIInterfaceOrientationLandscapeRight
    IPHONE_SCREEN_ORIENTATIONS      UIInterfaceOrientationPortrait
                                    UIInterfaceOrientationPortraitUpsideDown
                                    UIInterfaceOrientationLandscapeLeft
                                    UIInterfaceOrientationLandscapeRight)

juce_generate_juce_header(DemoRunner)

target_sources(DemoRunner PRIVATE
    Source/Demos/DemoPIPs1.cpp
    Source/Demos/DemoPIPs2.cpp
    Source/Demos/JUCEDemos.cpp
    Source/Main.cpp
    Source/UI/DemoContentComponent.cpp
    Source/UI/MainComponent.cpp)

target_compile_definitions(DemoRunner PRIVATE
    PIP_JUCE_EXAMPLES_DIRECTORY_STRING="${JUCE_SOURCE_DIR}/examples"
    JUCE_ALLOW_STATIC_NULL_VARIABLES=0
    JUCE_CONTENT_SHARING=1
    JUCE_DEMO_RUNNER=1
    JUCE_PLUGINHOST_LV2=1
    JUCE_PLUGINHOST_VST3=1
    JUCE_PUSH_NOTIFICATIONS=1
    JUCE_STRICT_REFCOUNTEDPOINTER=1
    JUCE_UNIT_TESTS=1
    JUCE_USE_CAMERA=1
    JUCE_USE_CURL=1
    JUCE_USE_MP3AUDIOFORMAT=1
    JUCE_WEB_BROWSER=1
    # This is a temporary workaround to allow builds to complete on Xcode 15.
    # Add -Wl,-ld_classic to the OTHER_LDFLAGS build setting if you need to
    # deploy to older versions of macOS/iOS.
    JUCE_SILENCE_XCODE_15_LINKER_WARNING=1)

target_link_libraries(DemoRunner PRIVATE
    juce::juce_analytics
    juce::juce_animation
    juce::juce_audio_utils
    juce::juce_box2d
    juce::juce_dsp
    juce::juce_opengl
    juce::juce_osc
    juce::juce_product_unlocking
    juce::juce_video
    juce::juce_javascript
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags)

foreach(folder IN ITEMS ../Assets ../Audio ../DSP ../GUI ../Utilities)
    juce_add_bundle_resources_directory(DemoRunner ${folder})
endforeach()
