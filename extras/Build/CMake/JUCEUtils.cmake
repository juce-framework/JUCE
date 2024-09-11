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

# ==================================================================================================
# JUCE Target Support Helper Functions
#
# In this file, functions intended for use by end-users have the prefix `juce_`.
# Functions beginning with an underscore should be considered private and susceptible to
# change, so don't call them directly.
#
# See the readme at `docs/CMake API.md` for more information about CMake usage,
# including documentation of the public functions in this file.
# ==================================================================================================

include_guard(GLOBAL)
cmake_minimum_required(VERSION 3.22)

define_property(TARGET PROPERTY JUCE_COMPANY_NAME INHERITED
    BRIEF_DOCS "The company name for a particular target"
    FULL_DOCS "This can be found in ProjectInfo::companyName in a generated JuceHeader.h")
set_property(GLOBAL PROPERTY JUCE_COMPANY_NAME "yourcompany")

define_property(TARGET PROPERTY JUCE_COMPANY_WEBSITE INHERITED
    BRIEF_DOCS "The company website for a particular target"
    FULL_DOCS "This will be placed in the Info.plist for the target")
set_property(GLOBAL PROPERTY JUCE_COMPANY_WEBSITE "")

define_property(TARGET PROPERTY JUCE_COMPANY_EMAIL INHERITED
    BRIEF_DOCS "The company email address for a particular target"
    FULL_DOCS "This will be placed in the Info.plist for the target")
set_property(GLOBAL PROPERTY JUCE_COMPANY_EMAIL "")

define_property(TARGET PROPERTY JUCE_COMPANY_COPYRIGHT INHERITED
    BRIEF_DOCS "The company copyright for a particular target"
    FULL_DOCS "This will be placed in the Info.plist for the target")
set_property(GLOBAL PROPERTY JUCE_COMPANY_COPYRIGHT "")

define_property(TARGET PROPERTY JUCE_VST_COPY_DIR INHERITED
    BRIEF_DOCS "Install location for VST2 plugins"
    FULL_DOCS "This is where the plugin will be copied if plugin copying is enabled")

define_property(TARGET PROPERTY JUCE_VST3_COPY_DIR INHERITED
    BRIEF_DOCS "Install location for VST3 plugins"
    FULL_DOCS "This is where the plugin will be copied if plugin copying is enabled")

define_property(TARGET PROPERTY JUCE_AU_COPY_DIR INHERITED
    BRIEF_DOCS "Install location for AU plugins"
    FULL_DOCS "This is where the plugin will be copied if plugin copying is enabled")

define_property(TARGET PROPERTY JUCE_AAX_COPY_DIR INHERITED
    BRIEF_DOCS "Install location for AAX plugins"
    FULL_DOCS "This is where the plugin will be copied if plugin copying is enabled")

define_property(TARGET PROPERTY JUCE_UNITY_COPY_DIR INHERITED
    BRIEF_DOCS "Install location for Unity plugins"
    FULL_DOCS "This is where the plugin will be copied if plugin copying is enabled")

define_property(TARGET PROPERTY JUCE_LV2_COPY_DIR INHERITED
    BRIEF_DOCS "Install location for LV2 plugins"
    FULL_DOCS "This is where the plugin will be copied if plugin copying is enabled")

define_property(TARGET PROPERTY JUCE_COPY_PLUGIN_AFTER_BUILD INHERITED
    BRIEF_DOCS "Whether or not plugins should be copied after building"
    FULL_DOCS "Whether or not plugins should be copied after building")
set_property(GLOBAL PROPERTY JUCE_COPY_PLUGIN_AFTER_BUILD FALSE)

function(_juce_available_pkgconfig_module_or_else out package alternative_package)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(package_to_be_found ${package} QUIET)

    if(package_to_be_found_FOUND)
        set(${out} ${package} PARENT_SCOPE)
    else()
        set(${out} ${alternative_package} PARENT_SCOPE)
    endif()
endfunction()

if((CMAKE_SYSTEM_NAME STREQUAL "Linux") OR (CMAKE_SYSTEM_NAME MATCHES ".*BSD"))
    _juce_create_pkgconfig_target(JUCE_CURL_LINUX_DEPS libcurl)
    _juce_available_pkgconfig_module_or_else(webkit_package_name webkit2gtk-4.1 webkit2gtk-4.0)
    _juce_create_pkgconfig_target(JUCE_BROWSER_LINUX_DEPS ${webkit_package_name} gtk+-x11-3.0)
endif()

# We set up default/fallback copy dirs here. If you need different copy dirs, use
# set_directory_properties or set_target_properties to adjust the values of `JUCE_*_COPY_DIR` at
# the appropriate scope.

function(_juce_set_default_properties)
    if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        set_property(GLOBAL PROPERTY JUCE_LV2_COPY_DIR    "$ENV{HOME}/Library/Audio/Plug-Ins/LV2")
        set_property(GLOBAL PROPERTY JUCE_VST_COPY_DIR    "$ENV{HOME}/Library/Audio/Plug-Ins/VST")
        set_property(GLOBAL PROPERTY JUCE_VST3_COPY_DIR   "$ENV{HOME}/Library/Audio/Plug-Ins/VST3")
        set_property(GLOBAL PROPERTY JUCE_AU_COPY_DIR     "$ENV{HOME}/Library/Audio/Plug-Ins/Components")
        set_property(GLOBAL PROPERTY JUCE_UNITY_COPY_DIR  "$ENV{HOME}/Library/Audio/Plug-Ins/Unity")
        set_property(GLOBAL PROPERTY JUCE_AAX_COPY_DIR    "/Library/Application Support/Avid/Audio/Plug-Ins")
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            set_property(GLOBAL PROPERTY JUCE_VST_COPY_DIR "$ENV{ProgramW6432}/Steinberg/Vstplugins")
            set(prefix "$ENV{CommonProgramW6432}")
        else()
            set_property(GLOBAL PROPERTY JUCE_VST_COPY_DIR "$ENV{programfiles\(x86\)}/Steinberg/Vstplugins")
            set(prefix "$ENV{CommonProgramFiles\(x86\)}")
        endif()

        set_property(GLOBAL PROPERTY JUCE_VST3_COPY_DIR   "${prefix}/VST3")
        set_property(GLOBAL PROPERTY JUCE_AAX_COPY_DIR    "${prefix}/Avid/Audio/Plug-Ins")
        set_property(GLOBAL PROPERTY JUCE_LV2_COPY_DIR    "$ENV{APPDATA}/LV2")
        set_property(GLOBAL PROPERTY JUCE_UNITY_COPY_DIR  "$ENV{APPDATA}/Unity")
    elseif((CMAKE_SYSTEM_NAME STREQUAL "Linux") OR (CMAKE_SYSTEM_NAME MATCHES ".*BSD"))
        set_property(GLOBAL PROPERTY JUCE_LV2_COPY_DIR   "$ENV{HOME}/.lv2")
        set_property(GLOBAL PROPERTY JUCE_VST_COPY_DIR   "$ENV{HOME}/.vst")
        set_property(GLOBAL PROPERTY JUCE_VST3_COPY_DIR  "$ENV{HOME}/.vst3")
        set_property(GLOBAL PROPERTY JUCE_UNITY_COPY_DIR "$ENV{HOME}/.unity")
    endif()
endfunction()

_juce_set_default_properties()

# ==================================================================================================

function(juce_add_bundle_resources_directory target folder)
    _juce_make_absolute(folder)

    if(NOT EXISTS "${folder}")
        message(FATAL_ERROR "Could not find resource folder ${folder}")
    endif()

    get_filename_component(folder_parent_path "${folder}" DIRECTORY)
    file(GLOB_RECURSE resources RELATIVE "${folder_parent_path}" "${folder}/*")

    foreach(file IN LISTS resources)
        target_sources(${target} PRIVATE "${folder_parent_path}/${file}")
        get_filename_component(resource_parent_path "${file}" DIRECTORY)
        set_source_files_properties("${folder_parent_path}/${file}" PROPERTIES
            HEADER_FILE_ONLY TRUE
            MACOSX_PACKAGE_LOCATION "Resources/${resource_parent_path}")
    endforeach()
endfunction()

# ==================================================================================================

function(_juce_create_linux_subprocess_helper_target)
    if(TARGET juce_linux_subprocess_helper)
        return()
    endif()

    set(source "${JUCE_CMAKE_UTILS_DIR}/juce_LinuxSubprocessHelper.cpp")

    add_executable(juce_linux_subprocess_helper ${source})
    add_executable(juce::juce_linux_subprocess_helper ALIAS juce_linux_subprocess_helper)
    target_compile_features(juce_linux_subprocess_helper PRIVATE cxx_std_17)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)
    target_link_libraries(juce_linux_subprocess_helper PRIVATE Threads::Threads ${CMAKE_DL_LIBS})
endfunction()

function(_juce_create_embedded_linux_subprocess_target output_target_name target)
    # This library needs to be created in every directory where a target wants to link against it.
    # Pre CMake 3.20 the GENERATED property is only visible in the directory of the current target,
    # and even post 3.20, CMake will not know how to generate the sources outside this directory.
    set(target_directory_key JUCE_EMBEDDED_LINUX_SUBPROCESS_TARGET)
    get_directory_property(embedded_linux_subprocess_target ${target_directory_key})

    if(embedded_linux_subprocess_target)
        set(${output_target_name} juce::${embedded_linux_subprocess_target} PARENT_SCOPE)
        return()
    endif()

    set(prefix "_juce_directory_prefix")
    set(embedded_linux_subprocess_target ${prefix}_embedded_linux_subprocess)

    while(TARGET ${embedded_linux_subprocess_target})
        set(embedded_linux_subprocess_target ${prefix}_${embedded_linux_subprocess_target})
    endwhile()

    _juce_create_linux_subprocess_helper_target()

    get_target_property(generated_sources_directory ${target} JUCE_GENERATED_SOURCES_DIRECTORY)

    if(generated_sources_directory)
        set(juce_linux_subprocess_helper_binary_dir "${generated_sources_directory}/$<CONFIG>/")
    else()
        set(juce_linux_subprocess_helper_binary_dir "${CMAKE_CURRENT_BINARY_DIR}/juce_LinuxSubprocessHelper/$<CONFIG>/")
    endif()

    set(binary_header_file  "${juce_linux_subprocess_helper_binary_dir}/juce_LinuxSubprocessHelperBinaryData.h")
    set(binary_source_file  "${juce_linux_subprocess_helper_binary_dir}/juce_LinuxSubprocessHelperBinaryData.cpp")
    set(juceaide_input_file "${juce_linux_subprocess_helper_binary_dir}/input_file_list")

    file(GENERATE OUTPUT ${juceaide_input_file} CONTENT "$<TARGET_FILE:juce_linux_subprocess_helper>")
    file(MAKE_DIRECTORY ${juce_linux_subprocess_helper_binary_dir})

    add_custom_command(
        OUTPUT
            ${binary_header_file}
            ${binary_source_file}
        COMMAND juce::juceaide binarydata "LinuxSubprocessHelperBinaryData" "${binary_header_file}"
            ${juce_linux_subprocess_helper_binary_dir} "${juceaide_input_file}"
        COMMAND
            ${CMAKE_COMMAND} -E rename "${juce_linux_subprocess_helper_binary_dir}/BinaryData1.cpp" "${binary_source_file}"
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
        DEPENDS juce_linux_subprocess_helper
        VERBATIM)

    add_library(${embedded_linux_subprocess_target} INTERFACE)
    target_sources(${embedded_linux_subprocess_target} INTERFACE ${binary_source_file})
    target_include_directories(${embedded_linux_subprocess_target} INTERFACE ${juce_linux_subprocess_helper_binary_dir})
    target_compile_definitions(${embedded_linux_subprocess_target} INTERFACE JUCE_USE_EXTERNAL_TEMPORARY_SUBPROCESS=1)
    add_library(juce::${embedded_linux_subprocess_target} ALIAS ${embedded_linux_subprocess_target})

    set_directory_properties(PROPERTIES ${target_directory_key} ${embedded_linux_subprocess_target})
    set(${output_target_name} juce::${embedded_linux_subprocess_target} PARENT_SCOPE)
endfunction()

function(juce_link_with_embedded_linux_subprocess target)
    _juce_create_embedded_linux_subprocess_target(embedded_linux_subprocess_target ${target})
    target_link_libraries(${target} PRIVATE ${embedded_linux_subprocess_target})
endfunction()

# ==================================================================================================

# Ideally, we'd check the preprocessor defs on the target to see whether
# JUCE_USE_CURL, JUCE_WEB_BROWSER, or JUCE_IN_APP_PURCHASES have been explicitly turned off,
# and then link libraries as appropriate.
# Unfortunately, this doesn't work, because linking a new library (curl/webkit/StoreKit)
# updates the target's compile defs, which results in a recursion/circular-dependency.
# Instead, we ask the user to explicitly request curl/webkit/StoreKit linking if they
# know they need it. Otherwise, we won't link anything.
# See the NEEDS_CURL, NEEDS_WEB_BROWSER, and NEEDS_STORE_KIT options in the CMake/readme.md.
function(_juce_link_optional_libraries target)
    if((CMAKE_SYSTEM_NAME STREQUAL "Linux") OR (CMAKE_SYSTEM_NAME MATCHES ".*BSD"))
        get_target_property(needs_curl ${target} JUCE_NEEDS_CURL)

        if(needs_curl)
            target_link_libraries(${target} PRIVATE juce::pkgconfig_JUCE_CURL_LINUX_DEPS)
        endif()

        get_target_property(needs_browser ${target} JUCE_NEEDS_WEB_BROWSER)

        if(needs_browser)
            target_link_libraries(${target} PRIVATE juce::pkgconfig_JUCE_BROWSER_LINUX_DEPS)

            get_target_property(is_plugin ${target} JUCE_IS_PLUGIN)

            if(is_plugin)
                juce_link_with_embedded_linux_subprocess(${target})
            endif()
        endif()
    elseif(APPLE)
        get_target_property(needs_storekit ${target} JUCE_NEEDS_STORE_KIT)

        if(needs_storekit)
            _juce_link_frameworks("${target}" PRIVATE StoreKit)
        endif()

        get_target_property(needs_camera ${target} JUCE_CAMERA_PERMISSION_ENABLED)

        if(CMAKE_SYSTEM_NAME STREQUAL "iOS" AND needs_camera)
            _juce_link_frameworks("${target}" PRIVATE ImageIO)
        endif()
    elseif(WIN32)
        get_target_property(needs_webview2 ${target} JUCE_NEEDS_WEBVIEW2)

        if (needs_webview2)
            if(NOT ("${JUCE_CMAKE_UTILS_DIR}" IN_LIST CMAKE_MODULE_PATH))
                list(APPEND CMAKE_MODULE_PATH "${JUCE_CMAKE_UTILS_DIR}")
            endif()

            find_package(WebView2 REQUIRED)
            target_link_libraries(${target} PRIVATE juce::juce_webview2)
        endif()
    endif()
endfunction()

# ==================================================================================================

function(_juce_get_module_definitions target filter out_var)
    set(compile_defs $<TARGET_GENEX_EVAL:${target},$<TARGET_PROPERTY:${target},COMPILE_DEFINITIONS>>)

    if(filter)
        set(${out_var} $<FILTER:${compile_defs},EXCLUDE,JucePlugin_Build_|JUCE_SHARED_CODE> PARENT_SCOPE)
    else()
        set(${out_var} ${compile_defs} PARENT_SCOPE)
    endif()
endfunction()

function(_juce_append_record output key)
    string(ASCII 30 RS)
    string(ASCII 31 US)

    set(prev)

    if(DEFINED "${output}")
        set(prev "${${output}}")
    endif()

    set(${output} "${prev}${key}${US}${ARGN}${RS}" PARENT_SCOPE)
endfunction()

function(_juce_append_target_property output key target property)
    get_target_property(prop ${target} ${property})

    if(prop STREQUAL "prop-NOTFOUND")
        set(prop)
    endif()

    _juce_append_record(${output} ${key} ${prop})
    set(${output} "${${output}}" PARENT_SCOPE)
endfunction()

# This is all info that should be known at configure time (i.e. no generator expressions here!)
# We use this info to generate plists and entitlements files, also at configure time.
function(_juce_write_configure_time_info target)
    _juce_append_target_property(file_content EXECUTABLE_NAME                      ${target} JUCE_PRODUCT_NAME)
    _juce_append_target_property(file_content VERSION                              ${target} JUCE_VERSION)
    _juce_append_target_property(file_content BUILD_VERSION                        ${target} JUCE_BUILD_VERSION)
    _juce_append_target_property(file_content PLIST_TO_MERGE                       ${target} JUCE_PLIST_TO_MERGE)
    _juce_append_target_property(file_content BUNDLE_ID                            ${target} JUCE_BUNDLE_ID)
    _juce_append_target_property(file_content XCODE_EXTRA_PLIST_ENTRIES            ${target} JUCE_XCODE_EXTRA_PLIST_ENTRIES)
    _juce_append_target_property(file_content MICROPHONE_PERMISSION_ENABLED        ${target} JUCE_MICROPHONE_PERMISSION_ENABLED)
    _juce_append_target_property(file_content MICROPHONE_PERMISSION_TEXT           ${target} JUCE_MICROPHONE_PERMISSION_TEXT)
    _juce_append_target_property(file_content CAMERA_PERMISSION_ENABLED            ${target} JUCE_CAMERA_PERMISSION_ENABLED)
    _juce_append_target_property(file_content CAMERA_PERMISSION_TEXT               ${target} JUCE_CAMERA_PERMISSION_TEXT)
    _juce_append_target_property(file_content BLUETOOTH_PERMISSION_ENABLED         ${target} JUCE_BLUETOOTH_PERMISSION_ENABLED)
    _juce_append_target_property(file_content BLUETOOTH_PERMISSION_TEXT            ${target} JUCE_BLUETOOTH_PERMISSION_TEXT)
    _juce_append_target_property(file_content SEND_APPLE_EVENTS_PERMISSION_ENABLED ${target} JUCE_SEND_APPLE_EVENTS_PERMISSION_ENABLED)
    _juce_append_target_property(file_content SEND_APPLE_EVENTS_PERMISSION_TEXT    ${target} JUCE_SEND_APPLE_EVENTS_PERMISSION_TEXT)
    _juce_append_target_property(file_content SHOULD_ADD_STORYBOARD                ${target} JUCE_SHOULD_ADD_STORYBOARD)
    _juce_append_target_property(file_content LAUNCH_STORYBOARD_FILE               ${target} JUCE_LAUNCH_STORYBOARD_FILE)
    _juce_append_target_property(file_content ICON_FILE                            ${target} JUCE_ICON_FILE)
    _juce_append_target_property(file_content PROJECT_NAME                         ${target} JUCE_PRODUCT_NAME)
    _juce_append_target_property(file_content COMPANY_COPYRIGHT                    ${target} JUCE_COMPANY_COPYRIGHT)
    _juce_append_target_property(file_content COMPANY_NAME                         ${target} JUCE_COMPANY_NAME)
    _juce_append_target_property(file_content DOCUMENT_EXTENSIONS                  ${target} JUCE_DOCUMENT_EXTENSIONS)
    _juce_append_target_property(file_content FILE_SHARING_ENABLED                 ${target} JUCE_FILE_SHARING_ENABLED)
    _juce_append_target_property(file_content DOCUMENT_BROWSER_ENABLED             ${target} JUCE_DOCUMENT_BROWSER_ENABLED)
    _juce_append_target_property(file_content STATUS_BAR_HIDDEN                    ${target} JUCE_STATUS_BAR_HIDDEN)
    _juce_append_target_property(file_content REQUIRES_FULL_SCREEN                 ${target} JUCE_REQUIRES_FULL_SCREEN)
    _juce_append_target_property(file_content BACKGROUND_AUDIO_ENABLED             ${target} JUCE_BACKGROUND_AUDIO_ENABLED)
    _juce_append_target_property(file_content BACKGROUND_BLE_ENABLED               ${target} JUCE_BACKGROUND_BLE_ENABLED)
    _juce_append_target_property(file_content PUSH_NOTIFICATIONS_ENABLED           ${target} JUCE_PUSH_NOTIFICATIONS_ENABLED)
    _juce_append_target_property(file_content NETWORK_MULTICAST_ENABLED            ${target} JUCE_NETWORK_MULTICAST_ENABLED)
    _juce_append_target_property(file_content PLUGIN_MANUFACTURER_CODE             ${target} JUCE_PLUGIN_MANUFACTURER_CODE)
    _juce_append_target_property(file_content PLUGIN_CODE                          ${target} JUCE_PLUGIN_CODE)
    _juce_append_target_property(file_content IPHONE_SCREEN_ORIENTATIONS           ${target} JUCE_IPHONE_SCREEN_ORIENTATIONS)
    _juce_append_target_property(file_content IPAD_SCREEN_ORIENTATIONS             ${target} JUCE_IPAD_SCREEN_ORIENTATIONS)
    _juce_append_target_property(file_content PLUGIN_NAME                          ${target} JUCE_PLUGIN_NAME)
    _juce_append_target_property(file_content PLUGIN_MANUFACTURER                  ${target} JUCE_COMPANY_NAME)
    _juce_append_target_property(file_content PLUGIN_DESCRIPTION                   ${target} JUCE_DESCRIPTION)
    _juce_append_target_property(file_content PLUGIN_AU_EXPORT_PREFIX              ${target} JUCE_AU_EXPORT_PREFIX)
    _juce_append_target_property(file_content PLUGIN_AU_MAIN_TYPE                  ${target} JUCE_AU_MAIN_TYPE_CODE)
    _juce_append_target_property(file_content IS_AU_SANDBOX_SAFE                   ${target} JUCE_AU_SANDBOX_SAFE)
    _juce_append_target_property(file_content IS_PLUGIN_SYNTH                      ${target} JUCE_IS_SYNTH)
    _juce_append_target_property(file_content IS_PLUGIN_ARA_EFFECT                 ${target} JUCE_IS_ARA_EFFECT)
    _juce_append_target_property(file_content SUPPRESS_AU_PLIST_RESOURCE_USAGE     ${target} JUCE_SUPPRESS_AU_PLIST_RESOURCE_USAGE)
    _juce_append_target_property(file_content HARDENED_RUNTIME_ENABLED             ${target} JUCE_HARDENED_RUNTIME_ENABLED)
    _juce_append_target_property(file_content APP_SANDBOX_ENABLED                  ${target} JUCE_APP_SANDBOX_ENABLED)
    _juce_append_target_property(file_content APP_SANDBOX_INHERIT                  ${target} JUCE_APP_SANDBOX_INHERIT)
    _juce_append_target_property(file_content HARDENED_RUNTIME_OPTIONS             ${target} JUCE_HARDENED_RUNTIME_OPTIONS)
    _juce_append_target_property(file_content APP_SANDBOX_OPTIONS                  ${target} JUCE_APP_SANDBOX_OPTIONS)
    _juce_append_target_property(file_content APP_SANDBOX_FILE_ACCESS_HOME_RO      ${target} JUCE_APP_SANDBOX_FILE_ACCESS_HOME_RO)
    _juce_append_target_property(file_content APP_SANDBOX_FILE_ACCESS_HOME_RW      ${target} JUCE_APP_SANDBOX_FILE_ACCESS_HOME_RW)
    _juce_append_target_property(file_content APP_SANDBOX_FILE_ACCESS_ABS_RO       ${target} JUCE_APP_SANDBOX_FILE_ACCESS_ABS_RO)
    _juce_append_target_property(file_content APP_SANDBOX_FILE_ACCESS_ABS_RW       ${target} JUCE_APP_SANDBOX_FILE_ACCESS_ABS_RW)
    _juce_append_target_property(file_content APP_SANDBOX_EXCEPTION_IOKIT          ${target} JUCE_APP_SANDBOX_EXCEPTION_IOKIT)
    _juce_append_target_property(file_content APP_GROUPS_ENABLED                   ${target} JUCE_APP_GROUPS_ENABLED)
    _juce_append_target_property(file_content APP_GROUP_IDS                        ${target} JUCE_APP_GROUP_IDS)
    _juce_append_target_property(file_content IS_PLUGIN                            ${target} JUCE_IS_PLUGIN)
    _juce_append_target_property(file_content ICLOUD_PERMISSIONS_ENABLED           ${target} JUCE_ICLOUD_PERMISSIONS_ENABLED)
    _juce_append_target_property(file_content IS_AU_PLUGIN_HOST                    ${target} JUCE_PLUGINHOST_AU)

    if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
        _juce_append_record(file_content IS_IOS 1)
    else()
        _juce_append_record(file_content IS_IOS 0)
    endif()

    get_target_property(juce_library_code ${target} JUCE_GENERATED_SOURCES_DIRECTORY)

    set(info_file "${juce_library_code}/Info.txt")
    file(WRITE "${info_file}" "${file_content}")
    set_target_properties(${target} PROPERTIES JUCE_INFO_FILE "${info_file}")
endfunction()

# In this file, we put things that CMake is only able to divine at generate time, like preprocessor definitions.
# We use the target preprocessor definitions to work out which JUCE modules should go in the JuceHeader.h.
function(_juce_write_generate_time_info target)
    _juce_get_module_definitions(${target} OFF module_defs)
    _juce_append_record(defs MODULE_DEFINITIONS ${module_defs})

    _juce_append_target_property(defs EXECUTABLE_NAME                     ${target} JUCE_PRODUCT_NAME)
    _juce_append_target_property(defs PROJECT_NAME                        ${target} JUCE_PRODUCT_NAME)
    _juce_append_target_property(defs VERSION                             ${target} JUCE_VERSION)
    _juce_append_target_property(defs COMPANY_NAME                        ${target} JUCE_COMPANY_NAME)

    get_target_property(juce_library_code ${target} JUCE_GENERATED_SOURCES_DIRECTORY)

    set(defs_file "${juce_library_code}/$<CONFIG>/Defs.txt")
    file(GENERATE OUTPUT "${defs_file}" CONTENT "${defs}")
    set_target_properties(${target} PROPERTIES JUCE_DEFS_FILE "${defs_file}")
endfunction()

# ==================================================================================================

function(juce_add_binary_data target)
    set(one_value_args NAMESPACE HEADER_NAME)
    set(multi_value_args SOURCES)
    cmake_parse_arguments(JUCE_ARG "" "${one_value_args}" "${multi_value_args}" ${ARGN})

    list(LENGTH JUCE_ARG_SOURCES num_binary_files)

    if(${num_binary_files} LESS 1)
        message(FATAL_ERROR "juce_add_binary_data must be passed at least one file to encode")
    endif()

    add_library(${target} STATIC)

    set_target_properties(${target} PROPERTIES POSITION_INDEPENDENT_CODE TRUE)

    set(juce_binary_data_folder "${CMAKE_CURRENT_BINARY_DIR}/juce_binarydata_${target}/JuceLibraryCode")

    set(binary_file_names)

    foreach(index RANGE 1 ${num_binary_files})
        list(APPEND binary_file_names "${juce_binary_data_folder}/BinaryData${index}.cpp")
    endforeach()

    file(MAKE_DIRECTORY ${juce_binary_data_folder})

    if(NOT JUCE_ARG_NAMESPACE)
        set(JUCE_ARG_NAMESPACE BinaryData)
    endif()

    if(NOT JUCE_ARG_HEADER_NAME)
        set(JUCE_ARG_HEADER_NAME BinaryData.h)
    endif()

    list(APPEND binary_file_names "${juce_binary_data_folder}/${JUCE_ARG_HEADER_NAME}")

    set(newline_delimited_input)

    foreach(name IN LISTS JUCE_ARG_SOURCES)
        _juce_make_absolute(name)
        set(newline_delimited_input "${newline_delimited_input}${name}\n")
    endforeach()

    set(input_file_list "${juce_binary_data_folder}/input_file_list")
    file(WRITE "${input_file_list}" "${newline_delimited_input}")

    add_custom_command(OUTPUT ${binary_file_names}
        COMMAND juce::juceaide binarydata "${JUCE_ARG_NAMESPACE}" "${JUCE_ARG_HEADER_NAME}"
            ${juce_binary_data_folder} "${input_file_list}"
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
        DEPENDS "${input_file_list}" ${JUCE_ARG_SOURCES}
        VERBATIM)

    target_sources(${target} PRIVATE "${binary_file_names}")
    target_include_directories(${target} INTERFACE ${juce_binary_data_folder})
    target_compile_features(${target} PRIVATE cxx_std_17)

    # This fixes an issue where Xcode is unable to find binary data during archive.
    if(CMAKE_GENERATOR STREQUAL "Xcode")
        set_target_properties(${target} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY "./")
    endif()

    if(JUCE_ARG_HEADER_NAME STREQUAL "BinaryData.h")
        target_compile_definitions(${target} INTERFACE JUCE_TARGET_HAS_BINARY_DATA=1)
    endif()
endfunction()

# ==================================================================================================

function(_juce_version_code version_in out_var)
    string(REGEX REPLACE "\\." ";" version_list ${version_in})
    list(LENGTH version_list num_version_components)

    set(version_major 0)
    set(version_minor 0)
    set(version_patch 0)

    if(num_version_components GREATER 0)
        list(GET version_list 0 version_major)
    endif()

    if(num_version_components GREATER 1)
        list(GET version_list 1 version_minor)
    endif()

    if(num_version_components GREATER 2)
        list(GET version_list 2 version_patch)
    endif()

    math(EXPR hex "(${version_major} << 16) + (${version_minor} << 8) + ${version_patch}"
        OUTPUT_FORMAT HEXADECIMAL)
    set(${out_var} "${hex}" PARENT_SCOPE)
endfunction()

function(_juce_to_char_literal str out_var help_text)
    string(LENGTH "${str}" string_length)

    if(NOT "${string_length}" EQUAL "4")
        message(WARNING "The ${help_text} code must contain exactly four characters, but it was set to '${str}'")
    endif()

    # Round-tripping through a file is the simplest way to convert a string to hex...
    string(SUBSTRING "${str}" 0 4 four_chars)
    string(RANDOM LENGTH 16 random_string)
    set(scratch_file "${CMAKE_CURRENT_BINARY_DIR}/${random_string}_ascii_conversion.txt")

    file(WRITE "${scratch_file}" "${four_chars}")
    file(READ "${scratch_file}" four_chars_hex HEX)
    file(REMOVE "${scratch_file}")

    string(SUBSTRING "${four_chars_hex}00000000" 0 8 four_chars_hex)
    set(${out_var} "${four_chars_hex}" PARENT_SCOPE)
endfunction()

# ==================================================================================================

function(juce_generate_juce_header target)
    get_target_property(juce_library_code ${target} JUCE_GENERATED_SOURCES_DIRECTORY)

    if(NOT juce_library_code)
        message(FATAL_ERROR "Target ${target} does not have a generated sources directory. Ensure it was created with a juce_add_* function")
    endif()

    set(juce_header ${juce_library_code}/JuceHeader.h)
    target_sources(${target} PRIVATE ${juce_header})

    set(defs_file $<GENEX_EVAL:$<TARGET_PROPERTY:${target},JUCE_DEFS_FILE>>)

    set(extra_args)

    add_custom_command(OUTPUT "${juce_header}"
        COMMAND juce::juceaide header "${defs_file}" "${juce_header}" ${extra_args}
        DEPENDS "${defs_file}"
        VERBATIM)
endfunction()

# ==================================================================================================

function(_juce_execute_juceaide)
    if(NOT TARGET juce::juceaide)
        message(FATAL_ERROR "The juceaide target does not exist")
    endif()

    get_target_property(juceaide_location juce::juceaide IMPORTED_LOCATION)

    if(NOT EXISTS "${juceaide_location}")
        message(FATAL_ERROR "juceaide was imported, but it doesn't exist!")
    endif()

    execute_process(COMMAND "${juceaide_location}" ${ARGN}
        RESULT_VARIABLE result_variable
        OUTPUT_VARIABLE output
        ERROR_VARIABLE output)

    if(result_variable)
        message(FATAL_ERROR "Running juceaide failed:\ncommand: ${juceaide_location} ${ARGN}\noutput: ${output}")
    endif()
endfunction()

function(_juce_set_output_name target name)
    if(NOT CMAKE_SYSTEM_NAME STREQUAL "Android")
        set_target_properties(${target} PROPERTIES
            OUTPUT_NAME ${name}
            XCODE_ATTRIBUTE_PRODUCT_NAME ${name})
    endif()
endfunction()

function(_juce_check_icon_files_exist icon_files)
    foreach(file IN LISTS icon_files)
        if(NOT EXISTS "${file}")
            message(FATAL_ERROR "Could not find icon file: ${file}")
        endif()
    endforeach()
endfunction()

function(_juce_generate_icon source_target dest_target)
    get_target_property(juce_library_code ${source_target} JUCE_GENERATED_SOURCES_DIRECTORY)
    get_target_property(juce_property_icon_big ${source_target} JUCE_ICON_BIG)
    get_target_property(juce_property_icon_small ${source_target} JUCE_ICON_SMALL)

    set(icon_args)

    if(juce_property_icon_big)
        list(APPEND icon_args "${juce_property_icon_big}")
    endif()

    if(juce_property_icon_small)
        list(APPEND icon_args "${juce_property_icon_small}")
    endif()

    set(generated_icon)

    if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        if(NOT icon_args)
            return()
        endif()

        _juce_check_icon_files_exist("${icon_args}")

        set(generated_icon "${juce_library_code}/Icon.icns")
        # To get compiled properly, we need the icon before the plist is generated!
        _juce_execute_juceaide(macicon "${generated_icon}" ${icon_args})
        set_source_files_properties(${generated_icon} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        if(NOT icon_args)
            return()
        endif()

        _juce_check_icon_files_exist("${icon_args}")

        set(generated_icon "${juce_library_code}/icon.ico")
        _juce_execute_juceaide(winicon "${generated_icon}" ${icon_args})
    elseif(CMAKE_SYSTEM_NAME STREQUAL "iOS")
        get_target_property(generated_icon ${source_target} JUCE_CUSTOM_XCASSETS_FOLDER)

        if(icon_args AND (NOT generated_icon))
            _juce_check_icon_files_exist("${icon_args}")

            set(out_path "${juce_library_code}/${dest_target}")
            set(generated_icon "${out_path}/Images.xcassets")

            # To get compiled properly, we need iOS assets at configure time!
            _juce_execute_juceaide(iosassets "${out_path}" ${icon_args})
        endif()

        if(NOT generated_icon)
            return()
        endif()

        set_target_properties(${dest_target} PROPERTIES
            XCODE_ATTRIBUTE_ASSETCATALOG_COMPILER_APPICON_NAME "AppIcon")

        get_target_property(add_storyboard ${source_target} JUCE_SHOULD_ADD_STORYBOARD)

        if(NOT add_storyboard)
            set_target_properties(${dest_target} PROPERTIES
                XCODE_ATTRIBUTE_ASSETCATALOG_COMPILER_LAUNCHIMAGE_NAME "LaunchImage")
        endif()
    endif()

    if(generated_icon)
        _juce_check_icon_files_exist("${generated_icon}")

        target_sources(${dest_target} PRIVATE ${generated_icon})
        set_target_properties(${source_target} ${dest_target} PROPERTIES
            JUCE_ICON_FILE "${generated_icon}"
            RESOURCE "${generated_icon}")
    endif()
endfunction()

function(_juce_add_xcode_entitlements source_target dest_target)
    get_target_property(juce_kind_string ${dest_target} JUCE_TARGET_KIND_STRING)
    get_target_property(input_info_file ${source_target} JUCE_INFO_FILE)

    get_target_property(juce_library_code ${source_target} JUCE_GENERATED_SOURCES_DIRECTORY)
    set(entitlements_file "${juce_library_code}/${dest_target}.entitlements")

    _juce_execute_juceaide(entitlements "${juce_kind_string}" "${input_info_file}" "${entitlements_file}")
    set_target_properties(${dest_target} PROPERTIES
        XCODE_ATTRIBUTE_CODE_SIGN_ENTITLEMENTS
        "${entitlements_file}"
        XCODE_ATTRIBUTE_ENABLE_HARDENED_RUNTIME
        "$<TARGET_PROPERTY:${source_target},JUCE_HARDENED_RUNTIME_ENABLED>")
endfunction()

function(_juce_configure_bundle source_target dest_target)
    _juce_generate_icon(${source_target} ${dest_target})
    _juce_write_configure_time_info(${source_target})

    if(NOT APPLE)
        return()
    endif()

    get_target_property(generated_icon ${source_target} JUCE_ICON_FILE)
    set(icon_dependency)

    if(generated_icon)
        set(icon_dependency "${generated_icon}")
    endif()

    get_target_property(juce_library_code ${source_target} JUCE_GENERATED_SOURCES_DIRECTORY)
    get_target_property(input_info_file ${source_target} JUCE_INFO_FILE)

    set(this_output_info_dir "${juce_library_code}/${dest_target}")
    set(this_output_pkginfo "${this_output_info_dir}/PkgInfo")
    set(this_output_plist "${this_output_info_dir}/Info.plist")

    get_target_property(juce_kind_string ${dest_target} JUCE_TARGET_KIND_STRING)

    _juce_execute_juceaide(plist "${juce_kind_string}" "${input_info_file}" "${this_output_plist}")
    set_target_properties(${dest_target} PROPERTIES
        BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST "${this_output_plist}")

    add_custom_command(OUTPUT "${this_output_pkginfo}"
        COMMAND juce::juceaide pkginfo "${juce_kind_string}" "${this_output_pkginfo}"
        VERBATIM)

    set(output_folder "$<TARGET_BUNDLE_CONTENT_DIR:${dest_target}>")

    target_sources(${dest_target} PRIVATE "${this_output_pkginfo}")
    set_source_files_properties("${this_output_pkginfo}" PROPERTIES
        HEADER_FILE_ONLY TRUE
        GENERATED TRUE)
    add_custom_command(TARGET ${dest_target} POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy "${this_output_pkginfo}" "${output_folder}"
        DEPENDS "${this_output_pkginfo}"
        VERBATIM)

    _juce_add_xcode_entitlements(${source_target} ${dest_target})

    if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
        get_target_property(add_storyboard ${source_target} JUCE_SHOULD_ADD_STORYBOARD)

        if(add_storyboard)
            get_target_property(storyboard_file ${source_target} JUCE_LAUNCH_STORYBOARD_FILE)

            if(NOT EXISTS "${storyboard_file}")
                message(FATAL_ERROR "Could not find storyboard file: ${storyboard_file}")
            endif()

            target_sources(${dest_target} PRIVATE "${storyboard_file}")
            set_property(TARGET ${dest_target} APPEND PROPERTY RESOURCE "${storyboard_file}")
        endif()
    endif()

    if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        set_target_properties(${dest_target} PROPERTIES
            XCODE_ATTRIBUTE_OTHER_CODE_SIGN_FLAGS "--timestamp")
    endif()

    set_target_properties(${dest_target} PROPERTIES
        XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY
            "$<TARGET_PROPERTY:${source_target},JUCE_TARGETED_DEVICE_FAMILY>")

    if(juce_kind_string STREQUAL "AUv3 AppExtension")
        get_target_property(source_bundle_id ${source_target} JUCE_BUNDLE_ID)

        if(source_bundle_id MATCHES "\\.([^.]+)$")
            set_target_properties(${dest_target} PROPERTIES
                XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER
                    "${source_bundle_id}.${CMAKE_MATCH_1}AUv3")
        else()
            message(FATAL_ERROR "Bundle ID should contain at least one `.`!")
        endif()
    else()
        set_target_properties(${dest_target} PROPERTIES
            XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER
                $<TARGET_PROPERTY:${source_target},JUCE_BUNDLE_ID>)
    endif()

    if(CMAKE_GENERATOR STREQUAL "Xcode")
        get_target_property(product_name ${source_target} JUCE_PRODUCT_NAME)
        set(skip_install NO)
        set(install_path "$(LOCAL_APPS_DIR)")

        if(juce_kind_string STREQUAL "AUv3 AppExtension")
            set(skip_install YES)
            set(install_path "")
        endif()

        set_target_properties(${dest_target} PROPERTIES
            XCODE_ATTRIBUTE_INSTALL_PATH "${install_path}"
            XCODE_ATTRIBUTE_SKIP_INSTALL "${skip_install}")
    endif()
endfunction()

function(_juce_add_resources_rc source_target dest_target)
    if(NOT CMAKE_SYSTEM_NAME STREQUAL "Windows")
        return()
    endif()

    if(NOT TARGET ${source_target}_rc_lib)
        get_target_property(juce_library_code ${source_target} JUCE_GENERATED_SOURCES_DIRECTORY)
        get_target_property(input_info_file ${source_target} JUCE_INFO_FILE)

        get_target_property(generated_icon ${source_target} JUCE_ICON_FILE)
        set(dependency)

        if(generated_icon)
            set(dependency DEPENDS "${generated_icon}")
        endif()

        set(resource_rc_file "${juce_library_code}/${source_target}_resources.rc")

        add_custom_command(OUTPUT "${resource_rc_file}"
            COMMAND juce::juceaide rcfile "${input_info_file}" "${resource_rc_file}"
            ${dependency}
            VERBATIM)

        add_library(${source_target}_rc_lib OBJECT ${resource_rc_file})

        set(compile_defs $<TARGET_GENEX_EVAL:${source_target},$<TARGET_PROPERTY:${source_target},COMPILE_DEFINITIONS>>)
        set(include_dirs $<TARGET_GENEX_EVAL:${source_target},$<TARGET_PROPERTY:${source_target},INCLUDE_DIRECTORIES>>)
        set(filtered $<FILTER:${compile_defs},INCLUDE,JUCE_USER_DEFINED_RC_FILE=>)
        set(has_custom_rc_include $<BOOL:${filtered}>)

        target_include_directories(${source_target}_rc_lib
            PRIVATE $<${has_custom_rc_include}:${include_dirs}>)
        set_source_files_properties(${resource_rc_file} PROPERTIES
            COMPILE_DEFINITIONS $<${has_custom_rc_include}:${compile_defs}>)
    endif()

    target_link_libraries(${dest_target} PRIVATE ${source_target}_rc_lib)
endfunction()

function(_juce_configure_app_bundle source_target dest_target)
    set_target_properties(${dest_target} PROPERTIES
        JUCE_TARGET_KIND_STRING "App"
        MACOSX_BUNDLE TRUE
        WIN32_EXECUTABLE TRUE)

    if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        set(nib_path "${JUCE_CMAKE_UTILS_DIR}/RecentFilesMenuTemplate.nib")
        target_sources("${dest_target}" PRIVATE "${nib_path}")
        set_source_files_properties("${nib_path}" PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    endif()
endfunction()

# ==================================================================================================

function(_juce_create_windows_package source_target dest_target extension default_icon x32folder x64folder)
    if(NOT CMAKE_SYSTEM_NAME STREQUAL "Windows")
        return()
    endif()

    get_target_property(products_folder ${dest_target} LIBRARY_OUTPUT_DIRECTORY)

    set(product_name $<TARGET_PROPERTY:${source_target},JUCE_PRODUCT_NAME>)
    set(output_folder "${products_folder}/${product_name}.${extension}")

    set(is_x64 $<EQUAL:${CMAKE_SIZEOF_VOID_P},8>)
    set(arch_string $<IF:${is_x64},${x64folder},${x32folder}>)

    set_target_properties(${dest_target}
        PROPERTIES
        PDB_OUTPUT_DIRECTORY "${products_folder}"
        LIBRARY_OUTPUT_DIRECTORY "${output_folder}/Contents/${arch_string}")

    get_target_property(icon_file ${source_target} JUCE_ICON_FILE)

    if(NOT icon_file)
        set(icon_file "${default_icon}")
    endif()

    if(icon_file)
        set(desktop_ini "${output_folder}/desktop.ini")
        set(plugin_ico "${output_folder}/Plugin.ico")

        file(GENERATE OUTPUT "${desktop_ini}"
            CONTENT
            "[.ShellClassInfo]\nIconResource=Plugin.ico,0\nIconFile=Plugin.ico\nIconIndex=0\n")
        add_custom_command(TARGET ${dest_target} POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E copy "${icon_file}" "${plugin_ico}"
            COMMAND attrib +s "${desktop_ini}"
            COMMAND attrib +s "${output_folder}"
            DEPENDS "${icon_file}" "${desktop_ini}"
            VERBATIM)
    endif()
endfunction()

# ==================================================================================================

function(_juce_add_unity_plugin_prefix_if_necessary name out_var)
    string(TOLOWER "${name}" lower)

    if(NOT lower MATCHES "^audioplugin")
        set(${out_var} "audioplugin_${name}" PARENT_SCOPE)
    else()
        set(${out_var} "${name}" PARENT_SCOPE)
    endif()
endfunction()

function(_juce_add_unity_script_file shared_target out_var)
    set(script_in "${JUCE_CMAKE_UTILS_DIR}/UnityPluginGUIScript.cs.in")

    get_target_property(plugin_name ${shared_target} JUCE_PLUGIN_NAME)
    get_target_property(plugin_vendor ${shared_target} JUCE_COMPANY_NAME)
    get_target_property(plugin_description ${shared_target} JUCE_DESCRIPTION)

    string(REGEX REPLACE " +" "_" plugin_class_name "${plugin_name}")

    get_target_property(juce_library_code ${shared_target} JUCE_GENERATED_SOURCES_DIRECTORY)
    _juce_add_unity_plugin_prefix_if_necessary("${plugin_name}" script_prefix)
    set(script_out "${juce_library_code}/${script_prefix}_UnityScript.cs")
    configure_file(${script_in} ${script_out})
    set(${out_var} "${script_out}" PARENT_SCOPE)
endfunction()

# ==================================================================================================

function(_juce_copy_dir target from to)
    # This is a shim to make CMake copy a whole directory, rather than just
    # the contents of a directory
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND "${CMAKE_COMMAND}"
            "-Dsrc=${from}"
            "-Ddest=${to}"
            "-P" "${JUCE_CMAKE_UTILS_DIR}/copyDir.cmake"
        VERBATIM)
endfunction()

function(_juce_set_copy_properties shared_code target from to_property)
    get_target_property(destination "${shared_code}" "${to_property}")

    if(destination)
        set_target_properties("${target}" PROPERTIES JUCE_PLUGIN_COPY_DIR "${destination}")
    endif()

    set_target_properties("${target}" PROPERTIES JUCE_PLUGIN_ARTEFACT_FILE "${from}")
endfunction()

function(_juce_adhoc_sign target)
    if(NOT CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        return()
    endif()

    get_target_property(bundle "${target}" BUNDLE)

    set(src "$<TARGET_FILE:${target}>")

    if(bundle)
        set(src "$<TARGET_BUNDLE_DIR:${target}>")
    endif()

    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND "${CMAKE_COMMAND}"
            "-Dsrc=${src}"
            "-P" "${JUCE_CMAKE_UTILS_DIR}/checkBundleSigning.cmake"
        VERBATIM)
endfunction()

function(juce_enable_copy_plugin_step shared_code_target)
    get_target_property(step_added ${shared_code_target} _JUCE_PLUGIN_COPY_STEP_ADDED)

    if(step_added)
        message(WARNING "Plugin copy step requested multiple times for ${shared_code_target}")
        return()
    endif()

    set_target_properties(${shared_code_target} PROPERTIES _JUCE_PLUGIN_COPY_STEP_ADDED TRUE)

    get_target_property(active_targets "${shared_code_target}" JUCE_ACTIVE_PLUGIN_TARGETS)

    foreach(target IN LISTS active_targets)
        get_target_property(target_kind "${target}" JUCE_TARGET_KIND_STRING)

        if(target_kind STREQUAL "App")
            continue()
        endif()

        _juce_adhoc_sign("${target}")

        get_target_property(source "${target}" JUCE_PLUGIN_ARTEFACT_FILE)

        if(NOT source)
            continue()
        endif()

        get_target_property(dest "${target}" JUCE_PLUGIN_COPY_DIR)

        if(dest)
            _juce_copy_dir("${target}" "${source}" "$<GENEX_EVAL:${dest}>")
        else()
            message(WARNING "Target '${target}' requested copy but no destination is set")
        endif()
    endforeach()
endfunction()

# ==================================================================================================

function(_juce_add_lv2_manifest_helper_target)
    if(TARGET juce_lv2_helper OR (CMAKE_SYSTEM_NAME STREQUAL "iOS") OR (CMAKE_SYSTEM_NAME STREQUAL "Android"))
        return()
    endif()

    get_target_property(module_path juce::juce_audio_plugin_client INTERFACE_JUCE_MODULE_PATH)
    set(source "${module_path}/juce_audio_plugin_client/LV2/juce_LV2ManifestHelper.cpp")
    add_executable(juce_lv2_helper "${source}")
    add_executable(juce::juce_lv2_helper ALIAS juce_lv2_helper)
    target_compile_features(juce_lv2_helper PRIVATE cxx_std_17)
    set_target_properties(juce_lv2_helper PROPERTIES BUILD_WITH_INSTALL_RPATH ON)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)
    target_link_libraries(juce_lv2_helper PRIVATE Threads::Threads ${CMAKE_DL_LIBS})
endfunction()

# ==================================================================================================

function(_juce_add_vst3_manifest_helper_target)
    if(TARGET juce_vst3_helper
       OR (CMAKE_SYSTEM_NAME STREQUAL "iOS")
       OR (CMAKE_SYSTEM_NAME STREQUAL "Android")
       OR (CMAKE_SYSTEM_NAME MATCHES ".*BSD"))
        return()
    endif()

    get_target_property(module_path juce::juce_audio_processors INTERFACE_JUCE_MODULE_PATH)
    set(vst3_dir "${module_path}/juce_audio_processors/format_types/VST3_SDK")

    set(extension "cpp")

    if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        set(extension "mm")
    endif()

    set(source "${module_path}/juce_audio_plugin_client/VST3/juce_VST3ManifestHelper.${extension}")

    add_executable(juce_vst3_helper "${source}")
    add_executable(juce::juce_vst3_helper ALIAS juce_vst3_helper)

    target_include_directories(juce_vst3_helper PRIVATE "${vst3_dir}" "${module_path}")

    add_library(juce_interface_definitions INTERFACE)
    _juce_add_standard_defs(juce_interface_definitions)
    target_link_libraries(juce_vst3_helper PRIVATE juce_interface_definitions)
    target_compile_features(juce_vst3_helper PRIVATE cxx_std_17)

    if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        _juce_link_frameworks(juce_vst3_helper PRIVATE Cocoa)
        target_compile_options(juce_vst3_helper PRIVATE -fobjc-arc)
    endif()

    if(MSYS OR MINGW)
        target_link_options(juce_vst3_helper PRIVATE -municode)
    endif()

    set_target_properties(juce_vst3_helper PROPERTIES BUILD_WITH_INSTALL_RPATH ON)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)
    target_link_libraries(juce_vst3_helper PRIVATE Threads::Threads ${CMAKE_DL_LIBS} juce_recommended_config_flags)

    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9)
        target_link_libraries(juce_vst3_helper PRIVATE stdc++fs)
    endif()
endfunction()

function(juce_enable_vst3_manifest_step shared_code_target)
    get_target_property(manifest_step_added ${shared_code_target} _JUCE_VST3_MANIFEST_STEP_ADDED)

    if(manifest_step_added)
        message(WARNING "VST3 manifest generation has already been enabled for target ${shared_code_target}. "
            "You may need to set VST3_AUTO_MANIFEST FALSE in juce_add_plugin, and/or check that you're "
            "not calling juce_enable_vst3_manifest_step multiple times.")
        return()
    endif()

    get_target_property(copy_step_added ${shared_code_target} _JUCE_PLUGIN_COPY_STEP_ADDED)

    if(copy_step_added)
        message(FATAL_ERROR "VST3 manifest generation would run after plugin copy step, so it has been disabled. "
            "If you're manually calling juce_enable_vst3_manifest_step, then you probably need to call "
            "juce_enable_copy_plugin_step too.")
    endif()

    if((MSYS OR MINGW) AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9)
        message(WARNING "VST3 manifest generation is disabled for ${shared_code_target} because the compiler is not supported.")
        return()
    endif()

    set(target_name ${shared_code_target}_VST3)
    get_target_property(product ${target_name} JUCE_PLUGIN_ARTEFACT_FILE)

    if(NOT product)
        message(FATAL_ERROR "Property JUCE_PLUGIN_ARTEFACT_FILE not set for ${target_name}")
    endif()

    # Add a target for the helper tool
    _juce_add_vst3_manifest_helper_target()

    get_target_property(target_version_string ${shared_code_target} JUCE_VERSION)

    # Use the helper tool to write out the moduleinfo.json
    add_custom_command(TARGET ${target_name} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${product}/Contents/Resources"
        COMMAND juce_vst3_helper
            -create
            -version "${target_version_string}"
            -path "${product}"
            -output "${product}/Contents/Resources/moduleinfo.json"
        VERBATIM)

    set_target_properties(${shared_code_target} PROPERTIES _JUCE_VST3_MANIFEST_STEP_ADDED TRUE)
endfunction()

# ==================================================================================================

function(_juce_disable_system_includes target)
    if(CMAKE_VERSION VERSION_GREATER "3.25")
        set_target_properties("${target}" PROPERTIES SYSTEM FALSE)
    elseif(CMAKE_VERSION VERSION_GREATER "3.23")
        set_target_properties("${target}" PROPERTIES IMPORTED_NO_SYSTEM TRUE)
    endif()
endfunction()

# ==================================================================================================

function(juce_set_aax_sdk_path path)
    if(TARGET juce_aax_sdk)
        message(FATAL_ERROR "juce_set_aax_sdk_path should only be called once")
    endif()

    _juce_make_absolute(path)

    if((NOT EXISTS "${path}")
       OR (NOT EXISTS "${path}/Interfaces")
       OR (NOT EXISTS "${path}/Interfaces/ACF"))
        message(FATAL_ERROR "Could not find AAX SDK at the specified path: ${path}")
    endif()

    if((CMAKE_SYSTEM_NAME STREQUAL "Darwin") OR (CMAKE_SYSTEM_NAME STREQUAL "Windows"))
        add_library(juce_aax_sdk INTERFACE IMPORTED GLOBAL)
    else()
        return()
    endif()

    _juce_disable_system_includes(juce_aax_sdk)
    target_include_directories(juce_aax_sdk INTERFACE
        "${path}"
        "${path}/Interfaces"
        "${path}/Interfaces/ACF")
    set_target_properties(juce_aax_sdk PROPERTIES INTERFACE_JUCE_AAX_DEFAULT_ICON "${path}/Utilities/PlugIn.ico")
endfunction()

function(_juce_init_bundled_aax_sdk)
    if(TARGET juce_aax_sdk)
        return()
    endif()

    get_target_property(module_path juce::juce_audio_plugin_client INTERFACE_JUCE_MODULE_PATH)
    juce_set_aax_sdk_path("${module_path}/juce_audio_plugin_client/AAX/SDK")
endfunction()

# ==================================================================================================

function(_juce_set_plugin_target_properties shared_code_target kind)
    set(target_name ${shared_code_target}_${kind})

    set_target_properties(${target_name} PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "$<GENEX_EVAL:$<TARGET_PROPERTY:${shared_code_target},ARCHIVE_OUTPUT_DIRECTORY>>/${kind}"
        LIBRARY_OUTPUT_DIRECTORY "$<GENEX_EVAL:$<TARGET_PROPERTY:${shared_code_target},LIBRARY_OUTPUT_DIRECTORY>>/${kind}"
        RUNTIME_OUTPUT_DIRECTORY "$<GENEX_EVAL:$<TARGET_PROPERTY:${shared_code_target},RUNTIME_OUTPUT_DIRECTORY>>/${kind}")

    get_target_property(products_folder ${target_name} LIBRARY_OUTPUT_DIRECTORY)
    set(product_name $<TARGET_PROPERTY:${shared_code_target},JUCE_PRODUCT_NAME>)

    if(kind STREQUAL "Standalone")
        get_target_property(is_bundle "${target_name}" BUNDLE)

        if(is_bundle)
            set_target_properties("${target_name}" PROPERTIES JUCE_PLUGIN_ARTEFACT_FILE "$<TARGET_BUNDLE_DIR:${target_name}>")
        else()
            set_target_properties("${target_name}" PROPERTIES JUCE_PLUGIN_ARTEFACT_FILE "$<TARGET_FILE:${target_name}>")
        endif()
    elseif(kind STREQUAL "VST3")
        set_target_properties(${target_name} PROPERTIES
            BUNDLE_EXTENSION vst3
            PREFIX ""
            SUFFIX .vst3
            BUNDLE TRUE
            XCODE_ATTRIBUTE_WRAPPER_EXTENSION vst3
            XCODE_ATTRIBUTE_LIBRARY_STYLE Bundle
            XCODE_ATTRIBUTE_GENERATE_PKGINFO_FILE YES)

        _juce_create_windows_package(${shared_code_target} ${target_name} vst3 "" x86-win x86_64-win)

        set(output_path "${products_folder}/${product_name}.vst3")

        if((CMAKE_SYSTEM_NAME STREQUAL "Linux") OR (CMAKE_SYSTEM_NAME MATCHES ".*BSD"))
            set_target_properties(${target_name} PROPERTIES
                SUFFIX .so
                LIBRARY_OUTPUT_DIRECTORY "${output_path}/Contents/${JUCE_TARGET_ARCHITECTURE}-linux")
        endif()

        _juce_set_copy_properties(${shared_code_target} ${target_name} "${output_path}" JUCE_VST3_COPY_DIR)

        _juce_adhoc_sign(${target_name})

        get_target_property(vst3_auto_manifest ${shared_code_target} JUCE_VST3_AUTO_MANIFEST)

        add_custom_command(TARGET ${target_name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E echo "removing moduleinfo.json"
            COMMAND ${CMAKE_COMMAND} -E remove -f
                "${output_path}/Contents/moduleinfo.json"
                "${output_path}/Contents/Resources/moduleinfo.json")

        if(vst3_auto_manifest)
            juce_enable_vst3_manifest_step(${shared_code_target})
        endif()
    elseif(kind STREQUAL "VST")
        set_target_properties(${target_name} PROPERTIES
            BUNDLE_EXTENSION vst
            BUNDLE TRUE
            XCODE_ATTRIBUTE_WRAPPER_EXTENSION vst
            XCODE_ATTRIBUTE_LIBRARY_STYLE Bundle
            XCODE_ATTRIBUTE_GENERATE_PKGINFO_FILE YES)

        set(output_path "$<TARGET_FILE:${target_name}>")

        if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
            set(output_path "$<TARGET_BUNDLE_DIR:${target_name}>")
        endif()

        _juce_set_copy_properties(${shared_code_target} ${target_name} "${output_path}" JUCE_VST_COPY_DIR)
    elseif(kind STREQUAL "AU")
        set_target_properties(${target_name} PROPERTIES
            BUNDLE_EXTENSION component
            XCODE_ATTRIBUTE_WRAPPER_EXTENSION component
            BUNDLE TRUE
            XCODE_ATTRIBUTE_LIBRARY_STYLE Bundle
            XCODE_ATTRIBUTE_GENERATE_PKGINFO_FILE YES)

        set(output_path "$<TARGET_BUNDLE_DIR:${target_name}>")
        _juce_set_copy_properties(${shared_code_target} ${target_name} "${output_path}" JUCE_AU_COPY_DIR)
    elseif(kind STREQUAL "AUv3")
        set_target_properties(${target_name} PROPERTIES
            XCODE_PRODUCT_TYPE "com.apple.product-type.app-extension"
            BUNDLE_EXTENSION appex
            XCODE_ATTRIBUTE_WRAPPER_EXTENSION appex
            XCODE_ATTRIBUTE_GENERATE_PKGINFO_FILE YES)
    elseif(kind STREQUAL "AAX")
        set_target_properties(${target_name} PROPERTIES
            BUNDLE_EXTENSION aaxplugin
            PREFIX ""
            SUFFIX .aaxplugin
            XCODE_ATTRIBUTE_WRAPPER_EXTENSION aaxplugin
            BUNDLE TRUE
            XCODE_ATTRIBUTE_LIBRARY_STYLE Bundle
            XCODE_ATTRIBUTE_GENERATE_PKGINFO_FILE YES)

        _juce_init_bundled_aax_sdk()
        get_target_property(default_icon juce_aax_sdk INTERFACE_JUCE_AAX_DEFAULT_ICON)
        _juce_create_windows_package(${shared_code_target} ${target_name} aaxplugin "${default_icon}" Win32 x64)

        set(output_path "${products_folder}/${product_name}.aaxplugin")
        _juce_set_copy_properties(${shared_code_target} ${target_name} "${output_path}" JUCE_AAX_COPY_DIR)
    elseif(kind STREQUAL "Unity")
        set_target_properties(${target_name} PROPERTIES
            BUNDLE_EXTENSION bundle
            XCODE_ATTRIBUTE_WRAPPER_EXTENSION bundle
            BUNDLE TRUE
            XCODE_ATTRIBUTE_LIBRARY_STYLE Bundle
            XCODE_ATTRIBUTE_GENERATE_PKGINFO_FILE YES)

        _juce_add_unity_script_file(${shared_code_target} script_file)
        target_sources(${target_name} PRIVATE "${script_file}")
        set_source_files_properties("${script_file}" PROPERTIES
            GENERATED TRUE
            MACOSX_PACKAGE_LOCATION Resources)

        if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
            set(output_path "$<TARGET_BUNDLE_DIR:${target_name}>")
            _juce_set_copy_properties(${shared_code_target} ${target_name} "${output_path}" JUCE_UNITY_COPY_DIR)
        else()
            # On windows and linux, the gui script needs to be copied next to the unity output
            add_custom_command(TARGET ${target_name} POST_BUILD
                COMMAND "${CMAKE_COMMAND}" -E copy "${script_file}" "${products_folder}"
                DEPENDS "${script_file}"
                VERBATIM)

            _juce_set_copy_properties(${shared_code_target}
                ${target_name}
                "$<TARGET_FILE:${target_name}>"
                JUCE_UNITY_COPY_DIR)
            _juce_set_copy_properties(${shared_code_target}
                ${target_name}
                "${script_file}"
                JUCE_UNITY_COPY_DIR)
        endif()
    elseif(kind STREQUAL "LV2")
        set_target_properties(${target_name} PROPERTIES BUNDLE FALSE)

        get_target_property(JUCE_LV2URI "${shared_code_target}" JUCE_LV2URI)

        if(NOT JUCE_LV2URI MATCHES "https?://.*|urn:.*")
            message(WARNING
                "LV2URI should be well-formed with an 'http' or 'urn' prefix. "
                "Check the LV2URI argument to juce_add_plugin.")
        endif()

        set(source_header "${JUCE_CMAKE_UTILS_DIR}/JuceLV2Defines.h.in")
        get_target_property(juce_library_code "${shared_code_target}" JUCE_GENERATED_SOURCES_DIRECTORY)
        configure_file("${source_header}" "${juce_library_code}/JuceLV2Defines.h")

        set(output_path "${products_folder}/${product_name}.lv2")
        set_target_properties(${target_name} PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${output_path}")

        _juce_adhoc_sign(${target_name})

        _juce_add_lv2_manifest_helper_target()

        add_custom_command(TARGET ${target_name} POST_BUILD
            COMMAND juce_lv2_helper "$<TARGET_FILE:${target_name}>"
            VERBATIM)

        _juce_set_copy_properties(${shared_code_target} ${target_name} "${output_path}" JUCE_LV2_COPY_DIR)
    endif()
endfunction()

# Place plugin wrapper targets alongside the shared code target in IDEs
function(_juce_set_plugin_folder_property shared_target wrapper_target)
    get_target_property(folder_to_use "${shared_target}" FOLDER)

    if(folder_to_use STREQUAL "folder_to_use-NOTFOUND")
        set_target_properties("${shared_target}" PROPERTIES FOLDER "${shared_target}")
    elseif(NOT folder_to_use MATCHES ".*${shared_target}$")
        set_target_properties("${shared_target}" PROPERTIES FOLDER "${folder_to_use}/${shared_target}")
    endif()

    get_target_property(folder_to_use "${shared_target}" FOLDER)
    set_target_properties("${wrapper_target}" PROPERTIES FOLDER "${folder_to_use}")
endfunction()

# Convert the cmake plugin kind ids to strings understood by ProjectType::Target::typeFromName
function(_juce_get_plugin_kind_name kind out_var)
    if(kind STREQUAL "AU")
        set(${out_var} "AU" PARENT_SCOPE)
    elseif(kind STREQUAL "AUv3")
        set(${out_var} "AUv3 AppExtension" PARENT_SCOPE)
    elseif(kind STREQUAL "AAX")
        set(${out_var} "AAX" PARENT_SCOPE)
    elseif(kind STREQUAL "LV2")
        set(${out_var} "LV2" PARENT_SCOPE)
    elseif(kind STREQUAL "Standalone")
        set(${out_var} "Standalone Plugin" PARENT_SCOPE)
    elseif(kind STREQUAL "Unity")
        set(${out_var} "Unity Plugin" PARENT_SCOPE)
    elseif(kind STREQUAL "VST")
        set(${out_var} "VST" PARENT_SCOPE)
    elseif(kind STREQUAL "VST3")
        set(${out_var} "VST3" PARENT_SCOPE)
    endif()
endfunction()

function(_juce_link_plugin_wrapper shared_code_target kind)
    set(target_name ${shared_code_target}_${kind})

    if(CMAKE_SYSTEM_NAME STREQUAL "Android")
        add_library(${target_name} SHARED)
    elseif((kind STREQUAL "Standalone") OR (kind STREQUAL "AUv3"))
        add_executable(${target_name} WIN32 MACOSX_BUNDLE)
    else()
        add_library(${target_name} MODULE)
    endif()

    if((CMAKE_SYSTEM_NAME STREQUAL "Linux") OR (CMAKE_SYSTEM_NAME MATCHES ".*BSD"))
        target_link_options(${target_name} PRIVATE "-Wl,--no-undefined")
    endif()

    # We re-export the shared code's private include dirs, because the wrapper targets need to
    # see the module headers. We don't just link publicly, because that would introduce
    # conflicting macro definitions.
    target_include_directories(${target_name} PRIVATE
        $<TARGET_PROPERTY:${shared_code_target},INCLUDE_DIRECTORIES>)

    target_link_libraries(${target_name} PRIVATE
        ${shared_code_target}
        juce::juce_audio_plugin_client_${kind})

    _juce_set_output_name(${target_name} $<TARGET_PROPERTY:${shared_code_target},JUCE_PRODUCT_NAME>)

    _juce_set_plugin_folder_property("${shared_code_target}" "${target_name}")

    _juce_get_plugin_kind_name(${kind} juce_kind_string)
    set_target_properties(${target_name} PROPERTIES
        XCODE_ATTRIBUTE_CLANG_LINK_OBJC_RUNTIME NO
        XCODE_ATTRIBUTE_COMBINE_HIDPI_IMAGES YES
        POSITION_INDEPENDENT_CODE TRUE
        VISIBILITY_INLINES_HIDDEN TRUE
        C_VISIBILITY_PRESET hidden
        CXX_VISIBILITY_PRESET hidden
        JUCE_TARGET_KIND_STRING "${juce_kind_string}")

    # Under the Xcode generator, POST_BUILD commands (including the plugin copy step) run before
    # signing, but M1 macs will only load signed binaries. Setting "adhoc_codesign" forces the
    # linker to sign bundles, so that they can be loaded even if they are copied before the "real"
    # signing step. See issue 21854 on the CMake Gitlab repo.
    if("${CMAKE_GENERATOR};${CMAKE_SYSTEM_NAME};${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "Xcode;Darwin;arm64")
        target_link_options(${target_name} PRIVATE LINKER:-adhoc_codesign)
    endif()

    add_dependencies(${shared_code_target}_All ${target_name})

    if(NOT kind STREQUAL "LV2")
        _juce_configure_bundle(${shared_code_target} ${target_name})
    else()
        _juce_write_configure_time_info(${shared_code_target})
    endif()

    _juce_set_plugin_target_properties(${shared_code_target} ${kind})
    _juce_add_resources_rc(${shared_code_target} ${target_name})
endfunction()

# ==================================================================================================

function(_juce_get_vst3_category_string target out_var)
    get_target_property(vst3_categories ${target} JUCE_VST3_CATEGORIES)

    if((NOT Fx IN_LIST vst3_categories) AND (NOT Instrument IN_LIST vst3_categories))
        get_target_property(is_synth ${target} JUCE_IS_SYNTH)

        if(is_synth)
            set(first_type Instrument)
        else()
            set(first_type Fx)
        endif()

        list(INSERT vst3_categories 0 ${first_type})
    else()
        if(Instrument IN_LIST vst3_categories)
            list(REMOVE_ITEM vst3_categories Instrument)
            list(INSERT vst3_categories 0 Instrument)
        endif()

        if(Fx IN_LIST vst3_categories)
            list(REMOVE_ITEM vst3_categories Fx)
            list(INSERT vst3_categories 0 Fx)
        endif()
    endif()

    string(REGEX REPLACE ";" "|" result "${vst3_categories}")
    set(${out_var} ${result} PARENT_SCOPE)
endfunction()

function(_juce_configure_plugin_targets target)
    _juce_set_output_name(${target} $<TARGET_PROPERTY:${target},JUCE_PRODUCT_NAME>_SharedCode)

    target_link_libraries(${target} PRIVATE juce::juce_audio_plugin_client)

    get_target_property(enabled_formats ${target} JUCE_FORMATS)

    set(active_formats)
    _juce_get_platform_plugin_kinds(plugin_kinds)

    foreach(kind IN LISTS plugin_kinds)
        if(kind IN_LIST enabled_formats)
            list(APPEND active_formats "${kind}")
        endif()
    endforeach()

    if((VST IN_LIST active_formats) AND (NOT TARGET juce_vst2_sdk))
        message(FATAL_ERROR "Use juce_set_vst2_sdk_path to set up the VST sdk before adding VST targets")
    endif()

    _juce_add_standard_defs(${target})
    _juce_add_plugin_definitions(${target} PRIVATE ${active_formats})

    # The plugin wrappers need to know what other modules are available, especially
    # juce_audio_utils and juce_gui_basics. We achieve this by searching for
    # JUCE_MODULE_AVAILABLE_ private compile definitions, and reexporting them in
    # the interface compile definitions.
    _juce_get_module_definitions(${target} ON enabled_modules)
    target_compile_definitions(${target} INTERFACE ${enabled_modules})

    target_compile_definitions(${target} PRIVATE JUCE_SHARED_CODE=1)

    get_target_property(project_version_string ${target} JUCE_VERSION)
    _juce_version_code(${project_version_string} project_version_hex)

    get_target_property(project_manufacturer_code ${target} JUCE_PLUGIN_MANUFACTURER_CODE)
    get_target_property(project_plugin_code ${target} JUCE_PLUGIN_CODE)

    get_target_property(use_legacy_compatibility_plugin_code ${target} JUCE_USE_LEGACY_COMPATIBILITY_PLUGIN_CODE)

    if(use_legacy_compatibility_plugin_code)
        set(project_manufacturer_code "proj")
    endif()

    _juce_to_char_literal(${project_manufacturer_code} project_manufacturer_code "plugin manufacturer")
    _juce_to_char_literal(${project_plugin_code} project_plugin_code "plugin")

    _juce_get_vst3_category_string(${target} vst3_category_string)

    target_compile_definitions(${target} PUBLIC
        JUCE_STANDALONE_APPLICATION=JucePlugin_Build_Standalone
        JucePlugin_IsSynth=$<BOOL:$<TARGET_PROPERTY:${target},JUCE_IS_SYNTH>>
        JucePlugin_ManufacturerCode=0x${project_manufacturer_code}
        JucePlugin_Manufacturer="$<TARGET_PROPERTY:${target},JUCE_COMPANY_NAME>"
        JucePlugin_ManufacturerWebsite="$<TARGET_PROPERTY:${target},JUCE_COMPANY_WEBSITE>"
        JucePlugin_ManufacturerEmail="$<TARGET_PROPERTY:${target},JUCE_COMPANY_EMAIL>"
        JucePlugin_PluginCode=0x${project_plugin_code}
        JucePlugin_ProducesMidiOutput=$<BOOL:$<TARGET_PROPERTY:${target},JUCE_NEEDS_MIDI_OUTPUT>>
        JucePlugin_IsMidiEffect=$<BOOL:$<TARGET_PROPERTY:${target},JUCE_IS_MIDI_EFFECT>>
        JucePlugin_WantsMidiInput=$<BOOL:$<TARGET_PROPERTY:${target},JUCE_NEEDS_MIDI_INPUT>>
        JucePlugin_EditorRequiresKeyboardFocus=$<BOOL:$<TARGET_PROPERTY:${target},JUCE_EDITOR_WANTS_KEYBOARD_FOCUS>>
        JucePlugin_Name="$<TARGET_PROPERTY:${target},JUCE_PLUGIN_NAME>"
        JucePlugin_Desc="$<TARGET_PROPERTY:${target},JUCE_DESCRIPTION>"
        JucePlugin_Version=${project_version_string}
        JucePlugin_VersionString="${project_version_string}"
        JucePlugin_VersionCode=${project_version_hex}
        JucePlugin_VSTUniqueID=JucePlugin_PluginCode
        JucePlugin_VSTCategory=$<TARGET_PROPERTY:${target},JUCE_VST2_CATEGORY>
        JucePlugin_Vst3Category="${vst3_category_string}"
        JucePlugin_AUMainType=$<TARGET_PROPERTY:${target},JUCE_AU_MAIN_TYPE_CODE>
        JucePlugin_AUSubType=JucePlugin_PluginCode
        JucePlugin_AUExportPrefix=$<TARGET_PROPERTY:${target},JUCE_AU_EXPORT_PREFIX>
        JucePlugin_AUExportPrefixQuoted="$<TARGET_PROPERTY:${target},JUCE_AU_EXPORT_PREFIX>"
        JucePlugin_AUManufacturerCode=JucePlugin_ManufacturerCode
        JucePlugin_CFBundleIdentifier=$<TARGET_PROPERTY:${target},JUCE_BUNDLE_ID>
        JucePlugin_AAXIdentifier=$<TARGET_PROPERTY:${target},JUCE_AAX_IDENTIFIER>
        JucePlugin_AAXManufacturerCode=JucePlugin_ManufacturerCode
        JucePlugin_AAXProductId=JucePlugin_PluginCode
        JucePlugin_AAXCategory=$<TARGET_PROPERTY:${target},JUCE_AAX_CATEGORY>
        JucePlugin_AAXDisableBypass=$<BOOL:$<TARGET_PROPERTY:${target},JUCE_DISABLE_AAX_BYPASS>>
        JucePlugin_AAXDisableMultiMono=$<BOOL:$<TARGET_PROPERTY:${target},JUCE_DISABLE_AAX_MULTI_MONO>>
        JucePlugin_VSTNumMidiInputs=$<TARGET_PROPERTY:${target},JUCE_VST_NUM_MIDI_INS>
        JucePlugin_VSTNumMidiOutputs=$<TARGET_PROPERTY:${target},JUCE_VST_NUM_MIDI_OUTS>
        JucePlugin_Enable_ARA=$<BOOL:$<TARGET_PROPERTY:${target},JUCE_IS_ARA_EFFECT>>
        JucePlugin_ARAFactoryID=$<TARGET_PROPERTY:${target},JUCE_ARA_FACTORY_ID>
        JucePlugin_ARADocumentArchiveID=$<TARGET_PROPERTY:${target},JUCE_ARA_DOCUMENT_ARCHIVE_ID>
        JucePlugin_ARACompatibleArchiveIDs=$<TARGET_PROPERTY:${target},JUCE_ARA_COMPATIBLE_ARCHIVE_IDS>
        JucePlugin_ARAContentTypes=$<TARGET_PROPERTY:${target},JUCE_ARA_ANALYSIS_TYPES>
        JucePlugin_ARATransformationFlags=$<TARGET_PROPERTY:${target},JUCE_ARA_TRANSFORMATION_FLAGS>)

    set_target_properties(${target} PROPERTIES
        POSITION_INDEPENDENT_CODE TRUE
        INTERFACE_POSITION_INDEPENDENT_CODE TRUE
        VISIBILITY_INLINES_HIDDEN TRUE
        C_VISIBILITY_PRESET hidden
        CXX_VISIBILITY_PRESET hidden)

    # A convenience target for building all plugin variations at once
    add_custom_target(${target}_All)
    _juce_set_plugin_folder_property("${target}" "${target}_All")

    foreach(kind IN LISTS active_formats)
        _juce_link_plugin_wrapper(${target} ${kind})

        if(TARGET ${target}_${kind})
            list(APPEND active_plugin_targets ${target}_${kind})
        endif()
    endforeach()

    set_target_properties(${target} PROPERTIES JUCE_ACTIVE_PLUGIN_TARGETS "${active_plugin_targets}")

    if(TARGET ${target}_Standalone)
        _juce_configure_app_bundle(${target} ${target}_Standalone)
    endif()

    if(TARGET ${target}_AAX)
        _juce_init_bundled_aax_sdk()
        target_link_libraries(${target}_AAX PRIVATE juce_aax_sdk)
    endif()

    if((TARGET ${target}_AUv3) AND (TARGET ${target}_Standalone))
        add_dependencies(${target}_Standalone ${target}_AUv3)
        set_target_properties(${target}_Standalone PROPERTIES
            XCODE_EMBED_APP_EXTENSIONS ${target}_AUv3)
    endif()

    get_target_property(wants_copy "${target}" JUCE_COPY_PLUGIN_AFTER_BUILD)

    if(wants_copy)
        juce_enable_copy_plugin_step("${target}")
    endif()
endfunction()

# ==================================================================================================

function(_juce_set_generic_property_if_not_set target property)
    list(LENGTH ARGN num_extra_args)

    if(num_extra_args EQUAL 0)
        return()
    endif()

    set(existing_property)
    get_target_property(existing_property ${target} ${property})

    if(existing_property STREQUAL "existing_property-NOTFOUND")
        set_target_properties(${target} PROPERTIES ${property} "${ARGN}")
    endif()
endfunction()

function(_juce_set_property_if_not_set target property)
    _juce_set_generic_property_if_not_set(${target} JUCE_${property} ${ARGN})
endfunction()

function(_juce_make_valid_4cc out_var)
    string(RANDOM LENGTH 1 ALPHABET "ABCDEFGHIJKLMNOPQRSTUVWXYZ" head)
    string(RANDOM LENGTH 3 ALPHABET "abcdefghijklmnopqrstuvwxyz" tail)
    set(${out_var} "${head}${tail}" PARENT_SCOPE)
endfunction()

# This function adds some default properties that plugin targets expect to be
# set, in order to generate the correct compile definitions.
function(_juce_set_fallback_properties target)
    _juce_set_property_if_not_set(${target} PRODUCT_NAME ${target})

    get_target_property(output_name ${target} JUCE_PRODUCT_NAME)
    _juce_set_property_if_not_set(${target} DESCRIPTION "${output_name}")
    _juce_set_property_if_not_set(${target} PLUGIN_NAME "${output_name}")

    get_target_property(real_company_name ${target} JUCE_COMPANY_NAME)
    _juce_set_property_if_not_set(${target} BUNDLE_ID "com.${real_company_name}.${target}")

    get_target_property(applied_bundle_id ${target} JUCE_BUNDLE_ID)

    if("${applied_bundle_id}" MATCHES ".* .*")
        message(WARNING "Target ${target} has JUCE_BUNDLE_ID '${applied_bundle_id}', which contains spaces. Use the BUNDLE_ID option to specify a valid ID.")
    endif()

    _juce_set_property_if_not_set(${target} VERSION ${PROJECT_VERSION})

    get_target_property(final_version ${target} JUCE_VERSION)

    if(NOT final_version)
        message(FATAL_ERROR "Target ${target} must have its VERSION argument set, or must be part of a project with a PROJECT_VERSION")
    endif()

    _juce_set_property_if_not_set(${target} BUILD_VERSION "${final_version}")

    get_target_property(custom_xcassets ${target} JUCE_CUSTOM_XCASSETS_FOLDER)
    get_target_property(custom_storyboard ${target} JUCE_LAUNCH_STORYBOARD_FILE)

    set(needs_storyboard TRUE)

    if((NOT custom_storyboard) AND custom_xcassets AND (EXISTS "${custom_xcassets}/LaunchImage.launchimage"))
        set(needs_storyboard FALSE)
    endif()

    set_target_properties(${target} PROPERTIES JUCE_SHOULD_ADD_STORYBOARD ${needs_storyboard})

    _juce_set_property_if_not_set(${target} IPHONE_SCREEN_ORIENTATIONS
        UIInterfaceOrientationPortrait UIInterfaceOrientationLandscapeLeft
        UIInterfaceOrientationLandscapeRight)

    _juce_set_property_if_not_set(${target} IPAD_SCREEN_ORIENTATIONS
        UIInterfaceOrientationPortrait UIInterfaceOrientationLandscapeLeft
        UIInterfaceOrientationLandscapeRight)

    _juce_set_property_if_not_set(${target}
        LAUNCH_STORYBOARD_FILE "${JUCE_CMAKE_UTILS_DIR}/LaunchScreen.storyboard")

    _juce_set_property_if_not_set(${target} PLUGIN_MANUFACTURER_CODE "Manu")

    # The plugin code will change on each run, unless you specify one manually!
    _juce_make_valid_4cc(random_code)
    _juce_set_property_if_not_set(${target} PLUGIN_CODE ${random_code})

    _juce_set_property_if_not_set(${target} IS_SYNTH FALSE)
    _juce_set_property_if_not_set(${target} NEEDS_MIDI_INPUT FALSE)
    _juce_set_property_if_not_set(${target} NEEDS_MIDI_OUTPUT FALSE)
    _juce_set_property_if_not_set(${target} IS_MIDI_EFFECT FALSE)
    _juce_set_property_if_not_set(${target} EDITOR_WANTS_KEYBOARD_FOCUS FALSE)
    _juce_set_property_if_not_set(${target} DISABLE_AAX_BYPASS FALSE)
    _juce_set_property_if_not_set(${target} DISABLE_AAX_MULTI_MONO FALSE)

    _juce_set_property_if_not_set(${target} PLUGINHOST_AU FALSE)

    get_target_property(bundle_id ${target} JUCE_BUNDLE_ID)
    _juce_set_property_if_not_set(${target} AAX_IDENTIFIER ${bundle_id})

    _juce_set_property_if_not_set(${target} VST_NUM_MIDI_INS 16)
    _juce_set_property_if_not_set(${target} VST_NUM_MIDI_OUTS 16)

    _juce_set_property_if_not_set(${target} AU_SANDBOX_SAFE FALSE)

    _juce_set_property_if_not_set(${target} SUPPRESS_AU_PLIST_RESOURCE_USAGE FALSE)

    _juce_set_property_if_not_set(${target} HARDENED_RUNTIME_ENABLED FALSE)
    _juce_set_property_if_not_set(${target} APP_SANDBOX_ENABLED FALSE)
    _juce_set_property_if_not_set(${target} APP_SANDBOX_INHERIT FALSE)

    _juce_set_property_if_not_set(${target} VST3_AUTO_MANIFEST TRUE)

    get_target_property(is_synth ${target} JUCE_IS_SYNTH)

    # VST3_CATEGORIES
    if(is_synth)
        _juce_set_property_if_not_set(${target} VST3_CATEGORIES Instrument Synth)
    else()
        _juce_set_property_if_not_set(${target} VST3_CATEGORIES Fx)
    endif()

    # VST2_CATEGORY
    if(is_synth)
        _juce_set_property_if_not_set(${target} VST2_CATEGORY kPlugCategSynth)
    else()
        _juce_set_property_if_not_set(${target} VST2_CATEGORY kPlugCategEffect)
    endif()

    get_target_property(is_midi_effect ${target} JUCE_IS_MIDI_EFFECT)
    get_target_property(needs_midi_input ${target} JUCE_NEEDS_MIDI_INPUT)

    # AU MAIN TYPE
    if(is_midi_effect)
        _juce_set_property_if_not_set(${target} AU_MAIN_TYPE kAudioUnitType_MIDIProcessor)
    elseif(is_synth)
        _juce_set_property_if_not_set(${target} AU_MAIN_TYPE kAudioUnitType_MusicDevice)
    elseif(needs_midi_input)
        _juce_set_property_if_not_set(${target} AU_MAIN_TYPE kAudioUnitType_MusicEffect)
    else()
        _juce_set_property_if_not_set(${target} AU_MAIN_TYPE kAudioUnitType_Effect)
    endif()

    _juce_set_property_if_not_set(${target} TARGETED_DEVICE_FAMILY "1,2")

    set(au_category_codes
        'aufx'
        'aufc'
        'augn'
        'aumi'
        'aumx'
        'aumu'
        'aumf'
        'auol'
        'auou'
        'aupn')

    set(au_category_strings
        kAudioUnitType_Effect
        kAudioUnitType_FormatConverter
        kAudioUnitType_Generator
        kAudioUnitType_MIDIProcessor
        kAudioUnitType_Mixer
        kAudioUnitType_MusicDevice
        kAudioUnitType_MusicEffect
        kAudioUnitType_OfflineEffect
        kAudioUnitType_Output
        kAudioUnitType_Panner)

    # AU export prefix
    string(MAKE_C_IDENTIFIER ${output_name} au_prefix)
    set(au_prefix "${au_prefix}AU")
    _juce_set_property_if_not_set(${target} AU_EXPORT_PREFIX ${au_prefix})

    # Find appropriate AU category code
    get_target_property(actual_au_category ${target} JUCE_AU_MAIN_TYPE)
    list(FIND au_category_strings ${actual_au_category} au_index)

    if(au_index GREATER_EQUAL 0)
        list(GET au_category_codes ${au_index} au_code_representation)
        set_target_properties(${target} PROPERTIES JUCE_AU_MAIN_TYPE_CODE ${au_code_representation})
    endif()

    # AAX category

    # The order of these strings is important, as the index of each string
    # will be used to set an appropriate bit in the category bitfield.
    set(aax_category_strings
        ePlugInCategory_None
        ePlugInCategory_EQ
        ePlugInCategory_Dynamics
        ePlugInCategory_PitchShift
        ePlugInCategory_Reverb
        ePlugInCategory_Delay
        ePlugInCategory_Modulation
        ePlugInCategory_Harmonic
        ePlugInCategory_NoiseReduction
        ePlugInCategory_Dither
        ePlugInCategory_SoundField
        ePlugInCategory_HWGenerators
        ePlugInCategory_SWGenerators
        ePlugInCategory_WrappedPlugin
        ePlugInCategory_Effect)

    if(is_synth)
        set(default_aax_category ePlugInCategory_SWGenerators)
    else()
        set(default_aax_category ePlugInCategory_None)
    endif()

    _juce_set_property_if_not_set(${target} AAX_CATEGORY ${default_aax_category})

    # Replace AAX category string with its integral representation
    get_target_property(actual_aax_category ${target} JUCE_AAX_CATEGORY)

    set(aax_category_int "")

    foreach(category_string IN LISTS actual_aax_category)
        list(FIND aax_category_strings ${category_string} aax_index)

        if(aax_index GREATER_EQUAL 0)
            if(aax_index EQUAL 0)
                set(aax_category_bit 0)
            else()
                set(aax_category_bit "1 << (${aax_index} - 1)")
            endif()

            if(aax_category_int STREQUAL "")
                set(aax_category_int 0)
            endif()

            math(EXPR aax_category_int "${aax_category_int} | (${aax_category_bit})")
        endif()
    endforeach()

    if(NOT aax_category_int STREQUAL "")
        set_target_properties(${target} PROPERTIES JUCE_AAX_CATEGORY ${aax_category_int})
    endif()

    # Ensure this matches the Projucer implementation
    get_target_property(company_website ${target} JUCE_COMPANY_WEBSITE)
    get_target_property(plugin_name ${target} JUCE_PLUGIN_NAME)
    string(MAKE_C_IDENTIFIER "${plugin_name}" plugin_name_sanitised)
    _juce_set_property_if_not_set(${target} LV2URI "${company_website}/plugins/${plugin_name_sanitised}")

    # ARA configuration
    # Analysis types
    set(ara_analysis_type_strings
        kARAContentTypeNotes
        kARAContentTypeTempoEntries
        kARAContentTypeBarSignatures
        kARAContentTypeStaticTuning
        kARAContentTypeKeySignatures
        kARAContentTypeSheetChords)

    get_target_property(actual_ara_analysis_types ${target} JUCE_ARA_ANALYSIS_TYPES)

    set(ara_analysis_types_int "")

    foreach(category_string IN LISTS actual_ara_analysis_types)
        list(FIND ara_analysis_type_strings ${category_string} ara_index)

        if(ara_index GREATER_EQUAL 0)
            set(ara_analysis_types_bit "1 << ${ara_index}")

            if(ara_analysis_types_int STREQUAL "")
                set(ara_analysis_types_int 0)
            endif()

            math(EXPR ara_analysis_types_int "${ara_analysis_types_int} | (${ara_analysis_types_bit})")
        endif()
    endforeach()

    if(NOT ara_analysis_types_int STREQUAL "")
        set_target_properties(${target} PROPERTIES JUCE_ARA_ANALYSIS_TYPES ${ara_analysis_types_int})
    endif()

    _juce_set_property_if_not_set(${target} ARA_ANALYSIS_TYPES 0)

    # Transformation flags
    set(ara_transformation_flags_strings
        kARAPlaybackTransformationNoChanges
        kARAPlaybackTransformationTimestretch
        kARAPlaybackTransformationTimestretchReflectingTempo
        kARAPlaybackTransformationContentBasedFadeAtTail
        kARAPlaybackTransformationContentBasedFadeAtHead)

    set(default_ara_transformation_flags kARAPlaybackTransformationNoChanges)

    _juce_set_property_if_not_set(${target} ARA_TRANSFORMATION_FLAGS ${default_ara_transformation_flags})

    get_target_property(actual_ara_transformation_flags ${target} JUCE_ARA_TRANSFORMATION_FLAGS)

    set(ara_transformation_flags_int "")

    foreach(transformation_string IN LISTS actual_ara_transformation_flags)
        list(FIND ara_transformation_flags_strings ${transformation_string} ara_transformation_index)

        if(ara_transformation_index GREATER_EQUAL 0)
            if(ara_transformation_index EQUAL 0)
                set(ara_transformation_bit 0)
            else()
                set(ara_transformation_bit "1 << (${ara_transformation_index} - 1)")
            endif()

            if(ara_transformation_flags_int STREQUAL "")
                set(ara_transformation_flags_int 0)
            endif()

            math(EXPR ara_transformation_flags_int "${ara_transformation_flags_int} | (${ara_transformation_bit})")
        endif()
    endforeach()

    if(NOT ara_transformation_flags_int STREQUAL "")
        set_target_properties(${target} PROPERTIES JUCE_ARA_TRANSFORMATION_FLAGS ${ara_transformation_flags_int})
    endif()

    _juce_set_property_if_not_set(${target} IS_ARA_EFFECT FALSE)
    get_target_property(final_bundle_id ${target} JUCE_BUNDLE_ID)
    _juce_set_property_if_not_set(${target} ARA_FACTORY_ID "\"${final_bundle_id}.arafactory.${final_version}\"")
    _juce_set_property_if_not_set(${target} ARA_DOCUMENT_ARCHIVE_ID "\"${final_bundle_id}.aradocumentarchive.1\"")
    _juce_set_property_if_not_set(${target} ARA_COMPATIBLE_ARCHIVE_IDS "\"\"")
endfunction()

# ==================================================================================================

function(_juce_initialise_target target)
    set(one_value_args
        VERSION
        BUILD_VERSION
        PRODUCT_NAME
        PLIST_TO_MERGE
        BUNDLE_ID
        MICROPHONE_PERMISSION_ENABLED
        MICROPHONE_PERMISSION_TEXT
        CAMERA_PERMISSION_ENABLED
        CAMERA_PERMISSION_TEXT
        SEND_APPLE_EVENTS_PERMISSION_ENABLED
        SEND_APPLE_EVENTS_PERMISSION_TEXT
        BLUETOOTH_PERMISSION_ENABLED
        BLUETOOTH_PERMISSION_TEXT
        FILE_SHARING_ENABLED            # iOS only
        DOCUMENT_BROWSER_ENABLED        # iOS only
        LAUNCH_STORYBOARD_FILE          # iOS only
        APP_GROUPS_ENABLED              # iOS only
        ICLOUD_PERMISSIONS_ENABLED      # iOS only
        STATUS_BAR_HIDDEN               # iOS only
        BACKGROUND_AUDIO_ENABLED        # iOS only
        BACKGROUND_BLE_ENABLED          # iOS only
        CUSTOM_XCASSETS_FOLDER          # iOS only
        TARGETED_DEVICE_FAMILY          # iOS only
        REQUIRES_FULL_SCREEN            # iOS only
        ICON_BIG
        ICON_SMALL
        COMPANY_COPYRIGHT
        COMPANY_NAME
        COMPANY_WEBSITE
        COMPANY_EMAIL
        NEEDS_CURL                      # Set this true if you want to link curl on Linux
        NEEDS_WEB_BROWSER               # Set this true if you want to link webkit on Linux
        NEEDS_WEBVIEW2                  # Set this true if you want to link WebView2 statically on Windows
        NEEDS_STORE_KIT                 # Set this true if you want in-app-purchases on Mac
        PUSH_NOTIFICATIONS_ENABLED
        NETWORK_MULTICAST_ENABLED
        HARDENED_RUNTIME_ENABLED
        APP_SANDBOX_ENABLED
        APP_SANDBOX_INHERIT
        VST3_AUTO_MANIFEST

        PLUGIN_NAME
        PLUGIN_MANUFACTURER_CODE
        PLUGIN_CODE
        DESCRIPTION
        IS_SYNTH
        NEEDS_MIDI_INPUT
        NEEDS_MIDI_OUTPUT
        IS_MIDI_EFFECT
        EDITOR_WANTS_KEYBOARD_FOCUS
        DISABLE_AAX_BYPASS
        DISABLE_AAX_MULTI_MONO
        AAX_IDENTIFIER
        VST_NUM_MIDI_INS
        VST_NUM_MIDI_OUTS
        VST2_CATEGORY
        AU_MAIN_TYPE
        AU_EXPORT_PREFIX
        AU_SANDBOX_SAFE
        SUPPRESS_AU_PLIST_RESOURCE_USAGE
        PLUGINHOST_AU                   # Set this true if you want to host AU plugins
        USE_LEGACY_COMPATIBILITY_PLUGIN_CODE
        LV2URI
        IS_ARA_EFFECT
        ARA_FACTORY_ID
        ARA_DOCUMENT_ARCHIVE_ID

        VST_COPY_DIR
        VST3_COPY_DIR
        AAX_COPY_DIR
        AU_COPY_DIR
        UNITY_COPY_DIR
        COPY_PLUGIN_AFTER_BUILD)

    set(multi_value_args
        FORMATS
        VST3_CATEGORIES
        HARDENED_RUNTIME_OPTIONS
        APP_SANDBOX_OPTIONS
        APP_SANDBOX_FILE_ACCESS_HOME_RO
        APP_SANDBOX_FILE_ACCESS_HOME_RW
        APP_SANDBOX_FILE_ACCESS_ABS_RO
        APP_SANDBOX_FILE_ACCESS_ABS_RW
        APP_SANDBOX_EXCEPTION_IOKIT
        DOCUMENT_EXTENSIONS
        AAX_CATEGORY
        IPHONE_SCREEN_ORIENTATIONS      # iOS only
        IPAD_SCREEN_ORIENTATIONS        # iOS only
        APP_GROUP_IDS                   # iOS only
        ARA_COMPATIBLE_ARCHIVE_IDS
        ARA_ANALYSIS_TYPES
        ARA_TRANSFORMATION_FLAGS)

    cmake_parse_arguments(JUCE_ARG "" "${one_value_args}" "${multi_value_args}" ${ARGN})

    set(base_folder "${CMAKE_CURRENT_BINARY_DIR}/${target}_artefacts")
    set(products_folder "${base_folder}/$<CONFIG>")
    set(juce_library_code "${base_folder}/JuceLibraryCode")

    set_target_properties(${target} PROPERTIES JUCE_GENERATED_SOURCES_DIRECTORY "${juce_library_code}")

    set_target_properties(${target} PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${products_folder}"
        LIBRARY_OUTPUT_DIRECTORY "${products_folder}"
        RUNTIME_OUTPUT_DIRECTORY "${products_folder}")

    if(JUCE_ARG_ICON_BIG)
        _juce_make_absolute_and_check(JUCE_ARG_ICON_BIG)
    endif()

    if(JUCE_ARG_ICON_SMALL)
        _juce_make_absolute_and_check(JUCE_ARG_ICON_SMALL)
    endif()

    set(inherited_properties
        COMPANY_NAME
        COMPANY_WEBSITE
        COMPANY_EMAIL
        COMPANY_COPYRIGHT
        VST_COPY_DIR
        VST3_COPY_DIR
        AU_COPY_DIR
        AAX_COPY_DIR
        UNITY_COPY_DIR
        COPY_PLUGIN_AFTER_BUILD)

    # Overwrite any properties that might be inherited
    foreach(prop_string IN LISTS inherited_properties)
        if(NOT ${JUCE_ARG_${prop_string}} STREQUAL "")
            set_target_properties(${target} PROPERTIES JUCE_${prop_string} "${JUCE_ARG_${prop_string}}")
        endif()
    endforeach()

    # Add each of the function arguments as target properties, so that it's easier to
    # extract them in other functions
    foreach(arg_string IN LISTS one_value_args multi_value_args)
        _juce_set_property_if_not_set(${target} ${arg_string} "${JUCE_ARG_${arg_string}}")
    endforeach()

    _juce_set_fallback_properties(${target})

    target_include_directories(${target} PRIVATE
        $<TARGET_PROPERTY:${target},JUCE_GENERATED_SOURCES_DIRECTORY>)
    target_link_libraries(${target} PUBLIC $<$<TARGET_EXISTS:juce_vst2_sdk>:juce_vst2_sdk>)

    get_target_property(is_pluginhost_au ${target} JUCE_PLUGINHOST_AU)

    if(is_pluginhost_au)
        target_compile_definitions(${target} PUBLIC JUCE_PLUGINHOST_AU=1)

        if(CMAKE_SYSTEM_NAME STREQUAL "Darwin" OR CMAKE_SYSTEM_NAME STREQUAL "iOS")
            _juce_link_frameworks("${target}" PRIVATE CoreAudioKit)
        endif()

        if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
            _juce_link_frameworks("${target}" PRIVATE AudioUnit)
        endif()
    endif()

    _juce_write_generate_time_info(${target})
    _juce_link_optional_libraries(${target})
    _juce_fixup_module_source_groups()
endfunction()

# ==================================================================================================

function(juce_add_console_app target)
    # The _NO_RESOURCERC option is private, and is only intended for use when building juceaide.
    # We can't add a resources.rc to juceaide because we need juceaide to generate the resources.rc!
    cmake_parse_arguments(JUCE_ARG "_NO_RESOURCERC" "" "" ${ARGN})

    add_executable(${target})
    target_compile_definitions(${target} PRIVATE JUCE_STANDALONE_APPLICATION=1)

    if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        target_compile_definitions(${target} PRIVATE _CONSOLE=1)
    endif()

    # When building for iOS, these properties will be read in order to populate
    # a plist for the app. We probably don't care whether these values are sane;
    # if we wanted to run on iOS, we'd use juce_gui_app instead.
    # We clear these explicitly to avoid warnings when configuring with
    # --warn-uninitialized
    set_target_properties(${target} PROPERTIES
        MACOSX_BUNDLE_BUNDLE_NAME           ""
        MACOSX_BUNDLE_BUNDLE_VERSION        ""
        MACOSX_BUNDLE_COPYRIGHT             ""
        MACOSX_BUNDLE_GUI_IDENTIFIER        ""
        MACOSX_BUNDLE_ICON_FILE             ""
        MACOSX_BUNDLE_INFO_STRING           ""
        MACOSX_BUNDLE_LONG_VERSION_STRING   ""
        MACOSX_BUNDLE_SHORT_VERSION_STRING  "")

    _juce_initialise_target(${target} ${ARGN})

    if(NOT JUCE_ARG__NO_RESOURCERC)
        set_target_properties(${target} PROPERTIES JUCE_TARGET_KIND_STRING "ConsoleApp")
        _juce_write_configure_time_info(${target})
        _juce_add_resources_rc(${target} ${target})
        _juce_add_xcode_entitlements(${target} ${target})
    endif()
endfunction()

function(juce_add_gui_app target)
    if(CMAKE_SYSTEM_NAME STREQUAL "Android")
        add_library(${target} SHARED)
    else()
        add_executable(${target})
    endif()

    target_compile_definitions(${target} PRIVATE JUCE_STANDALONE_APPLICATION=1)
    _juce_initialise_target(${target} ${ARGN})
    _juce_set_output_name(${target} $<TARGET_PROPERTY:${target},JUCE_PRODUCT_NAME>)
    set_target_properties(${target} PROPERTIES JUCE_TARGET_KIND_STRING "App")
    _juce_configure_bundle(${target} ${target})
    _juce_configure_app_bundle(${target} ${target})
    _juce_add_resources_rc(${target} ${target})
endfunction()

function(juce_add_plugin target)
    add_library(${target} STATIC)
    set_target_properties(${target} PROPERTIES JUCE_IS_PLUGIN TRUE)
    _juce_initialise_target(${target} ${ARGN})
    _juce_configure_plugin_targets(${target})
endfunction()

# ==================================================================================================

function(_juce_target_args_from_plugin_characteristics out_var)
    set(pairs
        "pluginIsSynth\;IS_SYNTH"
        "pluginWantsMidiIn\;NEEDS_MIDI_INPUT"
        "pluginProducesMidiOut\;NEEDS_MIDI_OUTPUT"
        "pluginIsMidiEffectPlugin\;IS_MIDI_EFFECT"
        "pluginEditorRequiresKeys\;EDITOR_WANTS_KEYBOARD_FOCUS")

    set(result)

    foreach(pair IN LISTS pairs)
        list(GET pair 0 old_key)

        if("${old_key}" IN_LIST ARGN)
            list(GET pair 1 new_key)
            list(APPEND result ${new_key} TRUE)
        endif()
    endforeach()

    set(${out_var} ${result} PARENT_SCOPE)
endfunction()

# ==================================================================================================

function(_juce_get_pip_targets pip out_var)
    set(test_targets "${pip}")

    _juce_get_all_plugin_kinds(plugin_kinds)

    foreach(plugin_kind IN LISTS plugin_kinds)
        list(APPEND test_targets "${JUCE_PIP_NAME}_${plugin_kind}")
    endforeach()

    set(${out_var} ${test_targets} PARENT_SCOPE)
endfunction()

function(juce_add_pip header)
    _juce_make_absolute(header)

    _juce_extract_metadata_block(JUCE_PIP_METADATA "${header}" metadata_dict)

    _juce_get_metadata("${metadata_dict}" name JUCE_PIP_NAME)

    if(NOT JUCE_PIP_NAME)
        message(FATAL_ERROR "Error in '${header}': PIP headers must declare a `name` field")
    endif()

    string(MAKE_C_IDENTIFIER "${JUCE_PIP_NAME}" pip_name_sanitised)

    if(NOT JUCE_PIP_NAME STREQUAL pip_name_sanitised)
        message(FATAL_ERROR "Error in '${header}': PIP `name` value '${JUCE_PIP_NAME}' must be a valid C identifier")
    endif()

    if(TARGET "${JUCE_PIP_NAME}")
        # We already added a target with this name, let's try using the filename instead
        get_filename_component(JUCE_PIP_NAME "${header}" NAME_WE)
    endif()

    if(TARGET "${JUCE_PIP_NAME}")
        message(FATAL_ERROR "Error in '${header}': Could not create a unique target name for PIP ${header}")
    endif()

    _juce_get_metadata("${metadata_dict}" type pip_kind)

    if(NOT pip_kind)
        message(FATAL_ERROR "Error in '${header}': PIP headers must declare a `type` field")
    endif()

    _juce_get_metadata("${metadata_dict}" pluginCharacteristics pip_charateristics)

    _juce_target_args_from_plugin_characteristics(extra_target_args ${pip_charateristics})

    list(APPEND extra_target_args
        NEEDS_CURL TRUE
        NEEDS_WEB_BROWSER TRUE)

    _juce_get_metadata("${metadata_dict}" moduleFlags pip_moduleflags)

    if("JUCE_IN_APP_PURCHASES=1" IN_LIST pip_moduleflags)
        list(APPEND extra_target_args
            BUNDLE_ID "com.rmsl.juceInAppPurchaseSample"
            NEEDS_STORE_KIT TRUE)
    endif()

    if("JUCE_PLUGINHOST_AU=1" IN_LIST pip_moduleflags)
        list(APPEND extra_target_args PLUGINHOST_AU TRUE)
    endif()

    if("JUCE_USE_WIN_WEBVIEW2_WITH_STATIC_LINKING=1" IN_LIST pip_moduleflags)
        list(APPEND extra_target_args NEEDS_WEBVIEW2 TRUE)
    endif()

    if(pip_kind STREQUAL "AudioProcessor")
        _juce_get_metadata("${metadata_dict}" documentControllerClass JUCE_PIP_DOCUMENTCONTROLLER_CLASS)

        if(JUCE_PIP_DOCUMENTCONTROLLER_CLASS)
            if(NOT TARGET juce_ara_sdk)
                message(WARNING
                    "${header} specifies a documentControllerClass, but the ARA SDK could not be located. "
                    "Use juce_set_ara_sdk_path to specify the ARA SDK location. "
                    "This PIP will not be configured.")
            endif()

            set(source_main "${JUCE_CMAKE_UTILS_DIR}/PIPAudioProcessorWithARA.cpp.in")

            juce_add_plugin(${JUCE_PIP_NAME}
                FORMATS AU VST3
                IS_ARA_EFFECT TRUE
                ${extra_target_args})
        else()
            set(source_main "${JUCE_CMAKE_UTILS_DIR}/PIPAudioProcessor.cpp.in")

            # We add VST2 targets too, if the user has set up those SDKs

            set(extra_formats)

            if(TARGET juce_vst2_sdk)
                list(APPEND extra_formats VST)
            endif()

            if(NOT (CMAKE_SYSTEM_NAME MATCHES ".*BSD"))
                list(APPEND extra_formats VST3)
            endif()

            # Standalone plugins might want to access the mic
            list(APPEND extra_target_args MICROPHONE_PERMISSION_ENABLED TRUE)

            juce_add_plugin(${JUCE_PIP_NAME}
                FORMATS AU AUv3 LV2 Standalone Unity AAX ${extra_formats}
                ${extra_target_args})
        endif()
    elseif(pip_kind STREQUAL "Component")
        set(source_main "${JUCE_CMAKE_UTILS_DIR}/PIPComponent.cpp.in")
        juce_add_gui_app(${JUCE_PIP_NAME} ${extra_target_args})
    elseif(pip_kind STREQUAL "Console")
        set(source_main "${JUCE_CMAKE_UTILS_DIR}/PIPConsole.cpp.in")
        juce_add_console_app(${JUCE_PIP_NAME} ${extra_target_args})
    else()
        message(FATAL_ERROR "Error in '${header}': PIP kind must be either AudioProcessor, Component, or Console")
    endif()

    if(NOT ARGV1 STREQUAL "")
        set("${ARGV1}" "${JUCE_PIP_NAME}" PARENT_SCOPE)
    endif()

    # Generate Main.cpp
    _juce_get_metadata("${metadata_dict}" mainClass JUCE_PIP_MAIN_CLASS)
    get_target_property(juce_library_code ${JUCE_PIP_NAME} JUCE_GENERATED_SOURCES_DIRECTORY)
    set(pip_main "${juce_library_code}/Main.cpp")

    set(JUCE_PIP_HEADER "${header}")
    configure_file(${source_main} ${pip_main})
    target_sources(${JUCE_PIP_NAME} PRIVATE ${pip_main})

    _juce_get_metadata("${metadata_dict}" dependencies pip_dependencies)

    juce_generate_juce_header(${JUCE_PIP_NAME})

    foreach(module IN LISTS pip_dependencies)
        if(module STREQUAL "")
            continue()
        endif()

        set(discovered_module)

        if(TARGET "${module}")
            set(discovered_module "${module}")
        else()
            message(FATAL_ERROR "Error in '${header}': No such module: ${module}")
        endif()

        target_link_libraries(${JUCE_PIP_NAME} PRIVATE ${discovered_module})
    endforeach()

    target_compile_definitions(${JUCE_PIP_NAME}
        PRIVATE ${pip_moduleflags}
        PUBLIC
            JUCE_VST3_CAN_REPLACE_VST2=0)

    _juce_get_pip_targets(${JUCE_PIP_NAME} pip_targets)

    # This keeps the PIPs slightly better organised in the JUCE megaproject
    if((DEFINED JUCE_SOURCE_DIR) AND (header MATCHES "^${JUCE_SOURCE_DIR}"))
        file(RELATIVE_PATH folder "${JUCE_SOURCE_DIR}" "${header}")
        get_filename_component(folder_parent "${folder}" DIRECTORY)

        foreach(target_name IN ITEMS ${pip_targets} ${JUCE_PIP_NAME}_All)
            if(TARGET "${target_name}")
                set_target_properties("${target_name}" PROPERTIES
                    FOLDER "${folder_parent}/${JUCE_PIP_NAME}")
            endif()
        endforeach()
    endif()

    # We're building JUCE itself
    if(DEFINED JUCE_SOURCE_DIR)
        # PIPs need to know about the resources folder
        target_compile_definitions(${JUCE_PIP_NAME} PRIVATE
            PIP_JUCE_EXAMPLES_DIRECTORY_STRING="${JUCE_SOURCE_DIR}/examples")

        get_filename_component(pip_parent_path "${header}" DIRECTORY)
        target_include_directories(${JUCE_PIP_NAME} PRIVATE "${pip_parent_path}")

        if((CMAKE_SYSTEM_NAME STREQUAL "iOS") AND (header MATCHES "^${JUCE_SOURCE_DIR}"))
            foreach(target_name IN LISTS pip_targets)
                if(TARGET "${target_name}")
                    juce_add_bundle_resources_directory("${target_name}" "${JUCE_SOURCE_DIR}/examples/Assets")
                endif()
            endforeach()
        endif()

        if((CMAKE_CXX_COMPILER_ID STREQUAL "MSVC") OR (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC"))
            target_compile_options(${JUCE_PIP_NAME} PRIVATE /bigobj)
        endif()
    endif()
endfunction()

# ==================================================================================================

function(juce_set_vst2_sdk_path path)
    if(TARGET juce_vst2_sdk)
        message(FATAL_ERROR "juce_set_vst2_sdk_path should only be called once")
    endif()

    _juce_make_absolute(path)

    if(NOT EXISTS "${path}")
        message(FATAL_ERROR "Could not find VST2 SDK at the specified path: ${path}")
    endif()

    add_library(juce_vst2_sdk INTERFACE IMPORTED GLOBAL)

    _juce_disable_system_includes(juce_vst2_sdk)
    # This is a bit of a hack, but we really need the VST2 paths to always follow the VST3 paths.
    target_include_directories(juce_vst2_sdk INTERFACE
        $<TARGET_PROPERTY:juce::juce_vst3_headers,INTERFACE_INCLUDE_DIRECTORIES>
        "${path}")
endfunction()

function(juce_set_vst3_sdk_path path)
    if(TARGET juce_vst3_sdk)
        message(FATAL_ERROR "juce_set_vst3_sdk_path should only be called once")
    endif()

    _juce_make_absolute(path)

    if(NOT EXISTS "${path}")
        message(FATAL_ERROR "Could not find VST3 SDK at the specified path: ${path}")
    endif()

    add_library(juce_vst3_sdk INTERFACE IMPORTED GLOBAL)

    _juce_disable_system_includes(juce_vst3_sdk)
    target_include_directories(juce_vst3_sdk INTERFACE "${path}")
endfunction()

function(juce_set_ara_sdk_path path)
    if(TARGET juce_ara_sdk)
        message(FATAL_ERROR "juce_set_ara_sdk_path should only be called once")
    endif()

    _juce_make_absolute(path)

    if(NOT EXISTS "${path}")
        message(FATAL_ERROR "Could not find ARA SDK at the specified path: ${path}")
    endif()

    add_library(juce_ara_sdk INTERFACE IMPORTED GLOBAL)

    _juce_disable_system_includes(juce_ara_sdk)
    target_include_directories(juce_ara_sdk INTERFACE "${path}")
endfunction()

# ==================================================================================================

function(juce_disable_default_flags)
    set(langs C CXX)
    set(modes DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)

    foreach(lang IN LISTS langs)
        foreach(mode IN LISTS modes)
            list(FILTER CMAKE_${lang}_FLAGS_${mode} INCLUDE REGEX "[/-]M[TD]d?")
            set(CMAKE_${lang}_FLAGS_${mode} "${CMAKE_${lang}_FLAGS_${mode}}" PARENT_SCOPE)
        endforeach()
    endforeach()
endfunction()
