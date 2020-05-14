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
    file(GLOB headers
        CONFIGURE_DEPENDS LIST_DIRECTORIES false
        "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

    foreach(header IN ITEMS ${headers})
        juce_add_pip(${header} added_target)
        target_link_libraries(${added_target} PRIVATE
            juce::juce_recommended_config_flags
            juce::juce_recommended_lto_flags
            juce::juce_recommended_warning_flags)

        get_target_property(active_targets ${added_target} JUCE_ACTIVE_PLUGIN_TARGETS)

        if(active_targets)
            foreach(plugin_target IN LISTS active_targets)
                target_link_libraries(${plugin_target} PRIVATE
                    juce::juce_recommended_config_flags
                    juce::juce_recommended_lto_flags
                    juce::juce_recommended_warning_flags)
            endforeach()
        endif()
    endforeach()
endfunction()

add_subdirectory(Audio)
add_subdirectory(BLOCKS)
add_subdirectory(DSP)
add_subdirectory(GUI)
add_subdirectory(Plugins)
add_subdirectory(Utilities)
