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

include(FindPackageHandleStandardArgs)

if(JUCE_WINDOWS_MIDI_SERVICES_PACKAGE_LOCATION)
    set(initial_search_dir ${JUCE_WINDOWS_MIDI_SERVICES_PACKAGE_LOCATION})
else()
    set(initial_search_dir "$ENV{USERPROFILE}/AppData/Local/PackageManagement/NuGet/Packages")
endif()

file(GLOB subdirs "${initial_search_dir}/Microsoft.Windows.Devices.Midi2.*")

if(subdirs)
    list(GET subdirs 0 search_dir)
    list(LENGTH subdirs num_midi2_packages)

    if(num_midi2_packages GREATER 1)
        message(WARNING "Multiple Windows MIDI Services packages found in the local NuGet folder. Proceeding with ${search_dir}.")
    endif()

    find_path(WindowsMIDIServices_root_dir ref/native/Microsoft.Windows.Devices.Midi2.winmd HINTS ${search_dir})
    set(WindowsMIDIServices_winmd "${WindowsMIDIServices_root_dir}/ref/native/Microsoft.Windows.Devices.Midi2.winmd")
elseif(NOT WindowsMIDIServices_FIND_QUIETLY)
    message(WARNING
            "Windows MIDI Services wasn't found in the the local NuGet folder."
            "\n"
            "TODO(reuk) At the moment the WMS package isn't in the public nuget repo so it needs to be installed from a local source.")
endif()

find_package_handle_standard_args(WindowsMIDIServices DEFAULT_MSG WindowsMIDIServices_winmd)

if(WindowsMIDIServices_FOUND)
    set(WindowsMIDIServices_INCLUDE_DIRS ${WindowsMIDIServices_include_dirs})

    mark_as_advanced(WindowsMIDIServices_include_dirs WindowsMIDIServices_root_dir)

    if(NOT TARGET juce_windows_midi_services)
        get_target_property(cppwinrt_command jcppwinrt IMPORTED_LOCATION)
        execute_process(COMMAND "${cppwinrt_command}"
                -reference sdk
                -input "${WindowsMIDIServices_winmd}"
                -output "${CMAKE_BINARY_DIR}/winmidi_projection"
            OUTPUT_VARIABLE command_output
            ERROR_VARIABLE command_output
            RESULT_VARIABLE result_variable)

        if(result_variable)
            message(FATAL_ERROR "Failed to run cppwinrt to generate Windows MIDI Services headers\n${command_output}")
        endif()

        add_library(juce_windows_midi_services INTERFACE)
        add_library(juce::juce_windows_midi_services ALIAS juce_windows_midi_services)
        target_link_libraries(juce_windows_midi_services INTERFACE juce_winrt_headers)
        target_include_directories(juce_windows_midi_services INTERFACE
            "${CMAKE_BINARY_DIR}/winmidi_projection"
            "${WindowsMIDIServices_root_dir}/build/native/include")
        target_compile_definitions(juce_windows_midi_services INTERFACE JUCE_USE_WINDOWS_MIDI_SERVICES=1)
    endif()
endif()
