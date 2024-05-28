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

    if(NOT (TARGET juce_ara_sdk
            AND (CMAKE_SYSTEM_NAME STREQUAL "Windows"
                 OR CMAKE_SYSTEM_NAME STREQUAL "Darwin"
                 OR CMAKE_SYSTEM_NAME STREQUAL "Linux")))
        list(REMOVE_ITEM headers
             "${CMAKE_CURRENT_SOURCE_DIR}/ARAPluginDemo.h")
    endif()

    if(NOT TARGET juce_vst2_sdk)
        list(REMOVE_ITEM headers "${CMAKE_CURRENT_SOURCE_DIR}/ReaperEmbeddedViewPluginDemo.h")
    endif()

    if((CMAKE_SYSTEM_NAME STREQUAL "Windows")
        AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/WebViewPluginDemo.h")

        if(NOT ("${JUCE_CMAKE_UTILS_DIR}" IN_LIST CMAKE_MODULE_PATH))
            list(APPEND CMAKE_MODULE_PATH "${JUCE_CMAKE_UTILS_DIR}")
        endif()

        find_package(WebView2)

        if(NOT WebView2_FOUND)
            list(REMOVE_ITEM headers "${CMAKE_CURRENT_SOURCE_DIR}/WebViewPluginDemo.h")

            message(WARNING "The WebViewPluginDemo was not enabled because the WebView2 package wasn't found.")
        endif()
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
