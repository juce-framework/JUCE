# ==============================================================================
#
#  This file is part of the JUCE library.
#  Copyright (c) 2020 - Raw Material Software Limited
#
#  JUCE is an open source library subject to commercial or open-source
#  licensing.
#
#  By using JUCE, you agree to the terms of both the JUCE 6 End-User License
#  Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).
#
#  End User License Agreement: www.juce.com/juce-6-licence
#  Privacy Policy: www.juce.com/juce-privacy-policy
#
#  Or: You may also use this code under the terms of the GPL v3 (see
#  www.gnu.org/licenses).
#
#  JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
#  EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
#  DISCLAIMED.
#
# ==============================================================================

set(CMAKE_FOLDER examples)

add_subdirectory(CMake)
add_subdirectory(DemoRunner)

function(_juce_add_pips)
    file(GLOB headers
        CONFIGURE_DEPENDS LIST_DIRECTORIES false
        "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

    if((CMAKE_SYSTEM_NAME STREQUAL "Linux") OR (CMAKE_SYSTEM_NAME MATCHES ".*BSD"))
        list(REMOVE_ITEM headers
            "${CMAKE_CURRENT_SOURCE_DIR}/CameraDemo.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/PushNotificationsDemo.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/VideoDemo.h")
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        list(REMOVE_ITEM headers
            "${CMAKE_CURRENT_SOURCE_DIR}/PushNotificationsDemo.h")
    endif()

    foreach(header IN ITEMS ${headers})
        juce_add_pip(${header} added_target)
        target_link_libraries(${added_target} PUBLIC
            juce::juce_recommended_config_flags
            juce::juce_recommended_lto_flags
            juce::juce_recommended_warning_flags)
    endforeach()
endfunction()

add_subdirectory(Audio)
add_subdirectory(BLOCKS)
add_subdirectory(DSP)
add_subdirectory(GUI)
add_subdirectory(Plugins)
add_subdirectory(Utilities)
