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

if(JUCE_WEBVIEW2_PACKAGE_LOCATION)
    set(initial_search_dir ${JUCE_WEBVIEW2_PACKAGE_LOCATION})
else()
    set(initial_search_dir "$ENV{USERPROFILE}/AppData/Local/PackageManagement/NuGet/Packages")
endif()

file(GLOB subdirs "${initial_search_dir}/*Microsoft.Web.WebView2*")

if(subdirs)
    list(GET subdirs 0 search_dir)
    list(LENGTH subdirs num_webview2_packages)

    if(num_webview2_packages GREATER 1)
        message(WARNING "Multiple WebView2 packages found in the local NuGet folder. Proceeding with ${search_dir}.")
    endif()

    find_path(WebView2_root_dir build/native/include/WebView2.h HINTS ${search_dir})

    set(WebView2_include_dir "${WebView2_root_dir}/build/native/include")

    if(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "arm64")
        set(WebView2_arch arm64)
    else()
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            set(WebView2_arch x64)
        elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
            set(WebView2_arch x86)
        endif()
    endif()

    set(WebView2_library "${WebView2_root_dir}/build/native/${WebView2_arch}/WebView2LoaderStatic.lib")
elseif(NOT WebView2_FIND_QUIETLY)
    message(WARNING
            "WebView2 wasn't found in the local NuGet folder."
            "\n"
            "To install NuGet and the WebView2 package containing the statically linked library, "
            "open a PowerShell and issue the following commands"
            "\n"
            "> Register-PackageSource -provider NuGet -name nugetRepository -location https://www.nuget.org/api/v2\n"
            "> Install-Package Microsoft.Web.WebView2 -Scope CurrentUser -RequiredVersion 1.0.1901.177 -Source nugetRepository\n"
            "\n"
            "Alternatively you can use the JUCE_WEBVIEW2_PACKAGE_LOCATION CMake variable to specify the directory "
            "where this find script is looking for the *Microsoft.Web.WebView2* package directory.")
endif()

find_package_handle_standard_args(WebView2 DEFAULT_MSG WebView2_include_dir WebView2_library)

if(WebView2_FOUND)
    set(WebView2_INCLUDE_DIRS ${WebView2_include_dir})
    set(WebView2_LIBRARIES ${WebView2_library})

    mark_as_advanced(WebView2_library WebView2_include_dir WebView2_root_dir)

    if(NOT TARGET juce_webview2)
        add_library(juce_webview2 INTERFACE)
        add_library(juce::juce_webview2 ALIAS juce_webview2)
        target_include_directories(juce_webview2 INTERFACE ${WebView2_INCLUDE_DIRS})
        target_link_libraries(juce_webview2 INTERFACE ${WebView2_LIBRARIES})
    endif()
endif()
