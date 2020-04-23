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

add_subdirectory(CMake)
add_subdirectory(DemoRunner)

function(_juce_add_pips)
    file(GLOB_RECURSE headers
        CONFIGURE_DEPENDS LIST_DIRECTORIES false
        "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

    foreach(header IN ITEMS ${headers})
        juce_add_pip(${header})
    endforeach()
endfunction()

add_subdirectory(Audio)
add_subdirectory(BLOCKS)
add_subdirectory(DSP)
add_subdirectory(GUI)
add_subdirectory(Plugins)
add_subdirectory(Utilities)
