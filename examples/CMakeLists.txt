# ==============================================================================
#
#  This file is part of the JUCE 7 technical preview.
#  Copyright (c) 2022 - Raw Material Software Limited
#
#  You may use this code under the terms of the GPL v3
#  (see www.gnu.org/licenses).
#
#  For the technical preview this file cannot be licensed commercially.
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
add_subdirectory(DSP)
add_subdirectory(GUI)
add_subdirectory(Plugins)
add_subdirectory(Utilities)
