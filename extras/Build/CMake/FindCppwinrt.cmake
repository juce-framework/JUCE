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

if(JUCE_CPPWINRT_PACKAGE_LOCATION)
    set(initial_search_dir ${JUCE_CPPWINRT_PACKAGE_LOCATION})
else()
    set(initial_search_dir "$ENV{USERPROFILE}/AppData/Local/PackageManagement/NuGet/Packages")
endif()

file(GLOB subdirs "${initial_search_dir}/Microsoft.Windows.CppWinRT.*")

if(subdirs)
    list(GET subdirs 0 search_dir)
    list(LENGTH subdirs num_midi2_packages)

    if(num_midi2_packages GREATER 1)
        message(WARNING "Multiple Windows MIDI Services packages found in the local NuGet folder. Proceeding with ${search_dir}.")
    endif()

    find_path(cppwinrt_root_dir bin/cppwinrt.exe HINTS ${search_dir})
    set(cppwinrt_exe "${cppwinrt_root_dir}/bin/cppwinrt.exe")
elseif(NOT cppwinrt_FIND_QUIETLY)
    message(WARNING "Cppwinrt wasn't found in the local packages folder.\n"
            "\n"
            "In order to use cppwinrt functionality, you must install this package."
            "To install NuGet and the CppWinRT package containing the statically linked library, "
            "open a PowerShell and issue the following commands"
            "\n"
            "> Register-PackageSource -provider NuGet -name nugetRepository -location https://www.nuget.org/api/v2\n"
            "> Install-Package Microsoft.Windows.CppWinRT -Scope CurrentUser -Source nugetRepository\n"
            "\n"
            "Alternatively you can use the JUCE_CPPWINRT_PACKAGE_LOCATION CMake variable to specify the directory "
            "where this find script is looking for the *Microsoft.Windows.CppWinRT* package directory.")
endif()

find_package_handle_standard_args(CppWinRT DEFAULT_MSG cppwinrt_exe)

if(CppWinRT_FOUND)
    mark_as_advanced(cppwinrt_root_dir cppwinrt_exe)

    if(NOT TARGET jcppwinrt)
        add_executable(jcppwinrt IMPORTED GLOBAL)
        set_target_properties(jcppwinrt PROPERTIES IMPORTED_LOCATION "${cppwinrt_exe}")
    endif()

    if(NOT TARGET juce_winrt_headers)
        get_target_property(cppwinrt_command jcppwinrt IMPORTED_LOCATION)
        set(cppwinrt_INCLUDE_DIRS "${CMAKE_BINARY_DIR}/cppwinrt_projection")

        execute_process(COMMAND "${cppwinrt_command}" -input sdk -output "${cppwinrt_INCLUDE_DIRS}"
            OUTPUT_VARIABLE command_output
            ERROR_VARIABLE command_output
            RESULT_VARIABLE result_variable)

        if(result_variable)
            message(FATAL_ERROR "Failed to run CppWinRT to generate Windows SDK headers\n${command_output}")
        endif()

        add_library(juce_winrt_headers INTERFACE)
        add_library(juce::juce_winrt_headers ALIAS juce_winrt_headers)
        target_link_libraries(juce_winrt_headers INTERFACE runtimeobject)
        target_include_directories(juce_winrt_headers INTERFACE ${cppwinrt_INCLUDE_DIRS})
    endif()
endif()
