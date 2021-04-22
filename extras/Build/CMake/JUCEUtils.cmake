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

# ==================================================================================================
# JUCE/CMake Compatibility Module
#
# In this file, functions intended for use by end-users have the prefix `juce_`.
# Functions beginning with an underscore should be considered private and susceptible to
# change, so don't call them directly.
#
# See the readme at `docs/CMake API.md` for more information about CMake usage,
# including documentation of the public functions in this file.
# ==================================================================================================

include_guard(GLOBAL)
cmake_minimum_required(VERSION 3.12)

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

define_property(TARGET PROPERTY JUCE_COPY_PLUGIN_AFTER_BUILD INHERITED
    BRIEF_DOCS "Whether or not plugins should be copied after building"
    FULL_DOCS "Whether or not plugins should be copied after building")
set_property(GLOBAL PROPERTY JUCE_COPY_PLUGIN_AFTER_BUILD FALSE)

# ==================================================================================================

function(_juce_add_interface_library target)
    add_library(${target} INTERFACE)
    target_sources(${target} INTERFACE ${ARGN})
endfunction()

# ==================================================================================================

function(_juce_create_pkgconfig_target name)
    if(TARGET juce::pkgconfig_${name})
        return()
    endif()

    find_package(PkgConfig REQUIRED)
    pkg_check_modules(${name} ${ARGN})

    add_library(pkgconfig_${name} INTERFACE)
    add_library(juce::pkgconfig_${name} ALIAS pkgconfig_${name})
    install(TARGETS pkgconfig_${name} EXPORT JUCE)

    set(pairs
        "INCLUDE_DIRECTORIES\;INCLUDE_DIRS"
        "LINK_LIBRARIES\;LINK_LIBRARIES"
        "LINK_OPTIONS\;LDFLAGS_OTHER"
        "COMPILE_OPTIONS\;CFLAGS_OTHER")

    foreach(pair IN LISTS pairs)
        list(GET pair 0 key)
        list(GET pair 1 value)

        if(${name}_${value})
            set_target_properties(pkgconfig_${name} PROPERTIES INTERFACE_${key} "${${name}_${value}}")
        endif()
    endforeach()
endfunction()

# ==================================================================================================

set(JUCE_CMAKE_UTILS_DIR ${CMAKE_CURRENT_LIST_DIR}
    CACHE INTERNAL "The path to the folder holding this file and other resources")

include("${JUCE_CMAKE_UTILS_DIR}/JUCEHelperTargets.cmake")
include("${JUCE_CMAKE_UTILS_DIR}/JUCECheckAtomic.cmake")

_juce_create_atomic_target(juce_atomic_wrapper)

# Tries to discover the target platform architecture, which is necessary for
# naming VST3 bundle folders correctly.
function(_juce_find_linux_target_architecture result)
    set(test_file "${JUCE_CMAKE_UTILS_DIR}/juce_runtime_arch_detection.cpp")
    try_compile(compile_result "${CMAKE_CURRENT_BINARY_DIR}" "${test_file}"
        OUTPUT_VARIABLE compile_output)
    string(REGEX REPLACE ".*JUCE_ARCH ([a-zA-Z0-9_-]*).*" "\\1" match_result "${compile_output}")
    set("${result}" "${match_result}" PARENT_SCOPE)
endfunction()

if((CMAKE_SYSTEM_NAME STREQUAL "Linux") OR (CMAKE_SYSTEM_NAME MATCHES ".*BSD"))
    _juce_create_pkgconfig_target(JUCE_CURL_LINUX_DEPS libcurl)
    _juce_create_pkgconfig_target(JUCE_BROWSER_LINUX_DEPS webkit2gtk-4.0 gtk+-x11-3.0)

    # If you really need to override the detected arch for some reason,
    # you can configure the build with -DJUCE_LINUX_TARGET_ARCHITECTURE=<custom arch>
    if(NOT DEFINED JUCE_LINUX_TARGET_ARCHITECTURE)
        _juce_find_linux_target_architecture(target_arch)
        set(JUCE_LINUX_TARGET_ARCHITECTURE "${target_arch}"
            CACHE INTERNAL "The target architecture, used to name internal folders in VST3 bundles")
    endif()
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    find_program(JUCE_XCRUN xcrun)

    if(NOT JUCE_XCRUN)
        message(WARNING "failed to find xcrun; older resource-based AU plug-ins may not work correctly")
    endif()
endif()

# We set up default/fallback copy dirs here. If you need different copy dirs, use
# set_directory_properties or set_target_properties to adjust the values of `JUCE_*_COPY_DIR` at
# the appropriate scope.

function(_juce_set_default_properties)
    if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        set_property(GLOBAL PROPERTY JUCE_VST_COPY_DIR  "$ENV{HOME}/Library/Audio/Plug-Ins/VST")
        set_property(GLOBAL PROPERTY JUCE_VST3_COPY_DIR "$ENV{HOME}/Library/Audio/Plug-Ins/VST3")
        set_property(GLOBAL PROPERTY JUCE_AU_COPY_DIR   "$ENV{HOME}/Library/Audio/Plug-Ins/Components")
        set_property(GLOBAL PROPERTY JUCE_AAX_COPY_DIR  "/Library/Application Support/Avid/Audio/Plug-Ins")
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            set_property(GLOBAL PROPERTY JUCE_VST_COPY_DIR "$ENV{ProgramW6432}/Steinberg/Vstplugins")
            set(prefix "$ENV{CommonProgramW6432}")
        else()
            set_property(GLOBAL PROPERTY JUCE_VST_COPY_DIR "$ENV{programfiles\(x86\)}/Steinberg/Vstplugins")
            set(prefix "$ENV{CommonProgramFiles\(x86\)}")
        endif()

        set_property(GLOBAL PROPERTY JUCE_VST3_COPY_DIR "${prefix}/VST3")
        set_property(GLOBAL PROPERTY JUCE_AAX_COPY_DIR  "${prefix}/Avid/Audio/Plug-Ins")
    elseif((CMAKE_SYSTEM_NAME STREQUAL "Linux") OR (CMAKE_SYSTEM_NAME MATCHES ".*BSD"))
        set_property(GLOBAL PROPERTY JUCE_VST_COPY_DIR  "$ENV{HOME}/.vst")
        set_property(GLOBAL PROPERTY JUCE_VST3_COPY_DIR "$ENV{HOME}/.vst3")
    endif()
endfunction()

_juce_set_default_properties()

# ==================================================================================================

function(_juce_add_standard_defs juce_target)
    target_compile_definitions(${juce_target} INTERFACE
        JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1
        $<IF:$<CONFIG:DEBUG>,DEBUG=1 _DEBUG=1,NDEBUG=1 _NDEBUG=1>
        $<$<PLATFORM_ID:Android>:JUCE_ANDROID=1>)
endfunction()

# ==================================================================================================

macro(_juce_make_absolute path)
    if(NOT IS_ABSOLUTE "${${path}}")
        get_filename_component("${path}" "${${path}}" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_LIST_DIR}")
    endif()
endmacro()

macro(_juce_make_absolute_and_check path)
    _juce_make_absolute("${path}")

    if(NOT EXISTS "${${path}}")
        message(FATAL_ERROR "No file at path ${${path}}")
    endif()
endmacro()

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

# This creates an imported interface library with a random name, and then adds
# the fields from a JUCE module header to the target as INTERFACE_ properties.
# We can extract properties later using `_juce_get_metadata`.
# This way, the interface library ends up behaving a bit like a dictionary,
# and we don't have to parse the module header from scratch every time we
# want to find a specific key.
function(_juce_extract_metadata_block delim_str file_with_block out_dict)
    string(RANDOM LENGTH 16 random_string)
    set(target_name "${random_string}_dict")
    set(${out_dict} "${target_name}" PARENT_SCOPE)
    add_library(${target_name} INTERFACE IMPORTED)

    if(NOT EXISTS ${file_with_block})
        message(FATAL_ERROR "Unable to find file ${file_with_block}")
    endif()

    file(STRINGS ${file_with_block} module_header_contents)

    set(last_written_key)
    set(append NO)

    foreach(line IN LISTS module_header_contents)
        if(NOT append)
            if(line MATCHES " *BEGIN_${delim_str} *")
                set(append YES)
            endif()

            continue()
        endif()

        if(append AND (line MATCHES " *END_${delim_str} *"))
            break()
        endif()

        if(line MATCHES "^ *([a-zA-Z]+):")
            set(last_written_key "${CMAKE_MATCH_1}")
        endif()

        string(REGEX REPLACE "^ *${last_written_key}: *" "" line "${line}")
        string(REGEX REPLACE "[ ,]+" ";" line "${line}")

        set_property(TARGET ${target_name} APPEND PROPERTY
            "INTERFACE_JUCE_${last_written_key}" "${line}")
    endforeach()
endfunction()

# Fetches properties attached to a metadata target.
function(_juce_get_metadata target key out_var)
    get_target_property(content "${target}" "INTERFACE_JUCE_${key}")

    if(NOT "${content}" STREQUAL "content-NOTFOUND")
        set(${out_var} "${content}" PARENT_SCOPE)
    endif()
endfunction()

# ==================================================================================================

function(_juce_should_build_module_source filename output_var)
    get_filename_component(trimmed_name "${filename}" NAME_WE)

    set(result TRUE)

    set(pairs
        "OSX\;Darwin"
        "Windows\;Windows"
        "Linux\;Linux"
        "Android\;Android"
        "iOS\;iOS")

    foreach(pair IN LISTS pairs)
        list(GET pair 0 suffix)
        list(GET pair 1 system_name)

        if((trimmed_name MATCHES "_${suffix}$") AND NOT (CMAKE_SYSTEM_NAME STREQUAL "${system_name}"))
            set(result FALSE)
        endif()
    endforeach()

    set("${output_var}" "${result}" PARENT_SCOPE)
endfunction()

function(_juce_module_sources module_path output_path built_sources other_sources)
    get_filename_component(module_parent_path ${module_path} DIRECTORY)
    get_filename_component(module_glob ${module_path} NAME)

    file(GLOB_RECURSE all_module_files
        CONFIGURE_DEPENDS LIST_DIRECTORIES FALSE
        RELATIVE "${module_parent_path}"
        "${module_path}/*")

    set(base_path "${module_glob}/${module_glob}")

    set(module_cpp ${all_module_files})
    list(FILTER module_cpp INCLUDE REGEX "^${base_path}[^/]*\\.(c|cc|cpp|cxx|s|asm)$")

    if(APPLE)
        set(module_mm ${all_module_files})
        list(FILTER module_mm INCLUDE REGEX "^${base_path}[^/]*\\.mm$")

        if(module_mm)
            set(module_mm_replaced ${module_mm})
            list(TRANSFORM module_mm_replaced REPLACE "\\.mm$" ".cpp")
            list(REMOVE_ITEM module_cpp ${module_mm_replaced})
        endif()

        set(module_apple_files ${all_module_files})
        list(FILTER module_apple_files INCLUDE REGEX "${base_path}[^/]*\\.(m|mm|metal|r|swift)$")
        list(APPEND module_cpp ${module_apple_files})
    endif()

    set(headers ${all_module_files})

    set(module_files_to_build)

    foreach(filename IN LISTS module_cpp)
        _juce_should_build_module_source("${filename}" should_build_file)

        if(should_build_file)
            list(APPEND module_files_to_build "${filename}")
        endif()
    endforeach()

    if(NOT "${module_files_to_build}" STREQUAL "")
        list(REMOVE_ITEM headers ${module_files_to_build})
    endif()

    foreach(source_list IN ITEMS module_files_to_build headers)
        list(TRANSFORM ${source_list} PREPEND "${output_path}/")
    endforeach()

    set(${built_sources} ${module_files_to_build} PARENT_SCOPE)
    set(${other_sources} ${headers} PARENT_SCOPE)
endfunction()

# ==================================================================================================

function(_juce_get_all_plugin_kinds out)
    set(${out} AU AUv3 AAX Standalone Unity VST VST3 PARENT_SCOPE)
endfunction()

function(_juce_get_platform_plugin_kinds out)
    set(result Standalone)

    if(APPLE AND (CMAKE_GENERATOR STREQUAL "Xcode"))
        list(APPEND result AUv3)
    endif()

    if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        list(APPEND result AU)
    endif()

    if(NOT CMAKE_SYSTEM_NAME STREQUAL "iOS" AND NOT CMAKE_SYSTEM_NAME STREQUAL "Android")
        list(APPEND result AAX Unity VST)

        if(NOT MINGW AND NOT MSYS)
            list(APPEND result VST3)
        endif()
    endif()

    set(${out} ${result} PARENT_SCOPE)
endfunction()

function(_juce_add_plugin_definitions target visibility)
    _juce_get_all_plugin_kinds(options)
    cmake_parse_arguments(JUCE_ARG "${options}" "" "" ${ARGN})

    foreach(opt IN LISTS options)
        set(flag_value 0)

        if(JUCE_ARG_${opt})
            set(flag_value 1)
        endif()

        target_compile_definitions(${target} ${visibility} "JucePlugin_Build_${opt}=${flag_value}")
    endforeach()
endfunction()

# ==================================================================================================

function(_juce_add_au_resource_fork shared_code_target au_target)
    if(NOT JUCE_XCRUN)
        return()
    endif()

    get_target_property(product_name ${shared_code_target} JUCE_PRODUCT_NAME)
    get_target_property(module_sources juce::juce_audio_plugin_client_AU INTERFACE_SOURCES)

    list(FILTER module_sources INCLUDE REGEX "/juce_audio_plugin_client_AU.r$")

    if(NOT module_sources)
        message(FATAL_ERROR "Failed to find AU resource file input")
    endif()

    list(GET module_sources 0 au_rez_sources)

    get_target_property(juce_library_code ${shared_code_target} JUCE_GENERATED_SOURCES_DIRECTORY)
    # We don't want our AU AppConfig.h to end up on peoples' include paths if we can help it
    set(secret_au_resource_dir "${juce_library_code}/${au_target}/secret")
    set(secret_au_plugindefines "${secret_au_resource_dir}/JucePluginDefines.h")

    set(au_rez_output "${secret_au_resource_dir}/${product_name}.rsrc")

    target_sources(${au_target} PRIVATE "${au_rez_output}")
    set_source_files_properties("${au_rez_output}" PROPERTIES
        GENERATED TRUE
        MACOSX_PACKAGE_LOCATION Resources)

    set(defs_file $<GENEX_EVAL:$<TARGET_PROPERTY:${shared_code_target},JUCE_DEFS_FILE>>)

    # Passing all our compile definitions using generator expressions is really painful
    # because some of the definitions have pipes and quotes and dollars and goodness-knows
    # what else that the shell would very much like to claim for itself, thank you very much.
    # CMake definitely knows how to escape all these things, because it's perfectly happy to pass
    # them to compiler invocations, but I have no idea how to get it to escape them
    # in a custom command.
    # In the end, it's simplest to generate a special single-purpose appconfig just for the
    # resource compiler.
    add_custom_command(OUTPUT "${secret_au_plugindefines}"
        COMMAND juce::juceaide auplugindefines "${defs_file}" "${secret_au_plugindefines}"
        DEPENDS "${defs_file}"
        VERBATIM)

    add_custom_command(OUTPUT "${au_rez_output}"
        COMMAND "${JUCE_XCRUN}" Rez
            -d "ppc_$ppc" -d "i386_$i386" -d "ppc64_$ppc64" -d "x86_64_$x86_64"
            -I "${secret_au_resource_dir}"
            -I "/System/Library/Frameworks/CoreServices.framework/Frameworks/CarbonCore.framework/Versions/A/Headers"
            -I "${CMAKE_OSX_SYSROOT}/System/Library/Frameworks/AudioUnit.framework/Headers"
            -isysroot "${CMAKE_OSX_SYSROOT}"
            "${au_rez_sources}"
            -useDF
            -o "${au_rez_output}"
        DEPENDS "${secret_au_plugindefines}"
        VERBATIM)

    set(au_resource_directory "$<TARGET_BUNDLE_DIR:${au_target}>/Contents/Resources")
endfunction()

# ==================================================================================================

# Takes a target, a link visibility, and a variable-length list of framework
# names. On macOS, finds the requested frameworks using `find_library` and
# links them. On iOS, links directly with `-framework Name`.
function(_juce_link_frameworks target visibility)
    foreach(framework IN LISTS ARGN)
        if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
            find_library("juce_found_${framework}" "${framework}" REQUIRED)
            target_link_libraries("${target}" "${visibility}" "${juce_found_${framework}}")
        elseif(CMAKE_SYSTEM_NAME STREQUAL "iOS")
            # CoreServices is only available on iOS 12+, we must link it weakly on earlier platforms
            if((framework STREQUAL "CoreServices") AND (CMAKE_OSX_DEPLOYMENT_TARGET LESS 12.0))
                set(framework_flags "-weak_framework ${framework}")
            else()
                set(framework_flags "-framework ${framework}")
            endif()

            target_link_libraries("${target}" "${visibility}" "${framework_flags}")
        endif()
    endforeach()
endfunction()

# ==================================================================================================

function(_juce_add_plugin_wrapper_target format path out_path)
    _juce_module_sources("${path}" "${out_path}" out_var headers)
    list(FILTER out_var EXCLUDE REGEX "/juce_audio_plugin_client_utils.cpp$")
    set(target_name juce_audio_plugin_client_${format})

    _juce_add_interface_library("${target_name}" ${out_var})
    _juce_add_plugin_definitions("${target_name}" INTERFACE ${format})
    _juce_add_standard_defs("${target_name}")

    target_compile_features("${target_name}" INTERFACE cxx_std_11)
    add_library("juce::${target_name}" ALIAS "${target_name}")

    if(format STREQUAL "AUv3")
        _juce_link_frameworks("${target_name}" INTERFACE AVFoundation)

        if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
            _juce_link_frameworks("${target_name}" INTERFACE AudioUnit)
        endif()
    elseif(format STREQUAL "AU")
        _juce_link_frameworks("${target_name}" INTERFACE AudioUnit CoreAudioKit)
    endif()
endfunction()

# ==================================================================================================

function(_juce_link_libs_from_metadata module_name dict key)
    _juce_get_metadata("${dict}" "${key}" libs)

    if(libs)
        target_link_libraries(${module_name} INTERFACE ${libs})
    endif()
endfunction()

# ==================================================================================================

function(juce_add_module module_path)
    set(one_value_args INSTALL_PATH ALIAS_NAMESPACE)
    cmake_parse_arguments(JUCE_ARG "" "${one_value_args}" "" ${ARGN})

    _juce_make_absolute(module_path)

    get_filename_component(module_name ${module_path} NAME)
    get_filename_component(module_parent_path ${module_path} DIRECTORY)

    set(module_header_name "${module_name}.h")

    if(NOT EXISTS "${module_path}/${module_header_name}")
        set(module_header_name "${module_header_name}pp")
    endif()

    if(NOT EXISTS "${module_path}/${module_header_name}")
        message(FATAL_ERROR "Could not locate module header for module '${module_path}'")
    endif()

    set(base_path "${module_parent_path}")

    _juce_module_sources("${module_path}" "${base_path}" globbed_sources headers)

    if(${module_name} STREQUAL "juce_audio_plugin_client")
        _juce_get_platform_plugin_kinds(plugin_kinds)

        foreach(kind IN LISTS plugin_kinds)
            _juce_add_plugin_wrapper_target(${kind} "${module_path}" "${base_path}")
        endforeach()

        set(utils_source
            "${base_path}/${module_name}/juce_audio_plugin_client_utils.cpp")
        add_library(juce_audio_plugin_client_utils INTERFACE)
        target_sources(juce_audio_plugin_client_utils INTERFACE "${utils_source}")

        if(JUCE_ARG_ALIAS_NAMESPACE)
            add_library(${JUCE_ARG_ALIAS_NAMESPACE}::juce_audio_plugin_client_utils
                ALIAS juce_audio_plugin_client_utils)
        endif()

        file(GLOB_RECURSE all_module_files
            CONFIGURE_DEPENDS LIST_DIRECTORIES FALSE
            RELATIVE "${module_parent_path}"
            "${module_path}/*")
    else()
        list(APPEND all_module_sources ${globbed_sources})
    endif()

    _juce_add_interface_library(${module_name} ${all_module_sources})

    set_property(GLOBAL APPEND PROPERTY _juce_module_names ${module_name})

    set_target_properties(${module_name} PROPERTIES
        INTERFACE_JUCE_MODULE_SOURCES   "${globbed_sources}"
        INTERFACE_JUCE_MODULE_HEADERS   "${headers}"
        INTERFACE_JUCE_MODULE_PATH      "${base_path}")

    if(JUCE_ENABLE_MODULE_SOURCE_GROUPS)
        target_sources(${module_name} INTERFACE ${headers})
    endif()

    if(${module_name} STREQUAL "juce_core")
        _juce_add_standard_defs(${module_name})

        target_link_libraries(juce_core INTERFACE juce::juce_atomic_wrapper)

        if(CMAKE_SYSTEM_NAME MATCHES ".*BSD")
            target_link_libraries(juce_core INTERFACE execinfo)
        elseif(CMAKE_SYSTEM_NAME STREQUAL "Android")
            target_sources(juce_core INTERFACE "${ANDROID_NDK}/sources/android/cpufeatures/cpu-features.c")
            target_include_directories(juce_core INTERFACE "${ANDROID_NDK}/sources/android/cpufeatures")
            target_link_libraries(juce_core INTERFACE android log)
        endif()
    endif()

    if(${module_name} STREQUAL "juce_audio_processors")
        add_library(juce_vst3_headers INTERFACE)

        target_compile_definitions(juce_vst3_headers INTERFACE "$<$<TARGET_EXISTS:juce_vst3_sdk>:JUCE_CUSTOM_VST3_SDK=1>")

        target_include_directories(juce_vst3_headers INTERFACE
            "$<$<TARGET_EXISTS:juce_vst3_sdk>:$<TARGET_PROPERTY:juce_vst3_sdk,INTERFACE_INCLUDE_DIRECTORIES>>"
            "$<$<NOT:$<TARGET_EXISTS:juce_vst3_sdk>>:${base_path}/juce_audio_processors/format_types/VST3_SDK>")

        target_link_libraries(juce_audio_processors INTERFACE juce_vst3_headers)

        if(JUCE_ARG_ALIAS_NAMESPACE)
            add_library(${JUCE_ARG_ALIAS_NAMESPACE}::juce_vst3_headers ALIAS juce_vst3_headers)
        endif()
    endif()

    target_include_directories(${module_name} INTERFACE ${base_path})

    target_compile_definitions(${module_name} INTERFACE JUCE_MODULE_AVAILABLE_${module_name}=1)

    if((CMAKE_SYSTEM_NAME STREQUAL "Linux") OR (CMAKE_SYSTEM_NAME MATCHES ".*BSD"))
        target_compile_definitions(${module_name} INTERFACE LINUX=1)
    endif()

    _juce_extract_metadata_block(JUCE_MODULE_DECLARATION "${module_path}/${module_header_name}" metadata_dict)

    _juce_get_metadata("${metadata_dict}" minimumCppStandard module_cpp_standard)

    if(module_cpp_standard)
        target_compile_features(${module_name} INTERFACE cxx_std_${module_cpp_standard})
    else()
        target_compile_features(${module_name} INTERFACE cxx_std_11)
    endif()

    if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        _juce_get_metadata("${metadata_dict}" OSXFrameworks module_osxframeworks)

        foreach(module_framework IN LISTS module_osxframeworks)
            if(module_framework STREQUAL "")
                continue()
            endif()

            _juce_link_frameworks("${module_name}" INTERFACE "${module_framework}")
        endforeach()

        _juce_link_libs_from_metadata("${module_name}" "${metadata_dict}" OSXLibs)
    elseif(CMAKE_SYSTEM_NAME STREQUAL "iOS")
        _juce_get_metadata("${metadata_dict}" iOSFrameworks module_iosframeworks)

        foreach(module_framework IN LISTS module_iosframeworks)
            if(module_framework STREQUAL "")
                continue()
            endif()

            _juce_link_frameworks("${module_name}" INTERFACE "${module_framework}")
        endforeach()

        _juce_link_libs_from_metadata("${module_name}" "${metadata_dict}" iOSLibs)
    elseif((CMAKE_SYSTEM_NAME STREQUAL "Linux") OR (CMAKE_SYSTEM_NAME MATCHES ".*BSD"))
        _juce_get_metadata("${metadata_dict}" linuxPackages module_linuxpackages)

        if(module_linuxpackages)
            _juce_create_pkgconfig_target(${module_name}_LINUX_DEPS ${module_linuxpackages})
            target_link_libraries(${module_name} INTERFACE juce::pkgconfig_${module_name}_LINUX_DEPS)
        endif()

        _juce_link_libs_from_metadata("${module_name}" "${metadata_dict}" linuxLibs)
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        if((CMAKE_CXX_COMPILER_ID STREQUAL "MSVC") OR (CMAKE_CXX_SIMULATE_ID STREQUAL "MSVC"))
            if(module_name STREQUAL "juce_gui_basics")
                target_compile_options(${module_name} INTERFACE /bigobj)
            endif()

            _juce_link_libs_from_metadata("${module_name}" "${metadata_dict}" windowsLibs)
        elseif(MSYS OR MINGW)
            if(module_name STREQUAL "juce_gui_basics")
                target_compile_options(${module_name} INTERFACE "-Wa,-mbig-obj")
            endif()

            _juce_link_libs_from_metadata("${module_name}" "${metadata_dict}" mingwLibs)
        endif()
    endif()

    _juce_get_metadata("${metadata_dict}" dependencies module_dependencies)
    target_link_libraries(${module_name} INTERFACE ${module_dependencies})

    _juce_get_metadata("${metadata_dict}" searchpaths module_searchpaths)

    if(NOT module_searchpaths STREQUAL "")
        foreach(module_searchpath IN LISTS module_searchpaths)
            target_include_directories(${module_name}
                INTERFACE "${module_path}/${module_searchpath}")
        endforeach()
    endif()

    if(JUCE_ARG_INSTALL_PATH)
        install(DIRECTORY "${module_path}" DESTINATION "${JUCE_ARG_INSTALL_PATH}")
    endif()

    if(JUCE_ARG_ALIAS_NAMESPACE)
        add_library(${JUCE_ARG_ALIAS_NAMESPACE}::${module_name} ALIAS ${module_name})
    endif()
endfunction()

function(juce_add_modules)
    set(one_value_args INSTALL_PATH ALIAS_NAMESPACE)
    cmake_parse_arguments(JUCE_ARG "" "${one_value_args}" "" ${ARGN})

    foreach(path IN LISTS JUCE_ARG_UNPARSED_ARGUMENTS)
        juce_add_module(${path}
            INSTALL_PATH "${JUCE_ARG_INSTALL_PATH}"
            ALIAS_NAMESPACE "${JUCE_ARG_ALIAS_NAMESPACE}")
    endforeach()
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
    set(${output} "${${output}}${key}${US}${ARGN}${RS}" PARENT_SCOPE)
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
    _juce_append_target_property(file_content SUPPRESS_AU_PLIST_RESOURCE_USAGE     ${target} JUCE_SUPPRESS_AU_PLIST_RESOURCE_USAGE)
    _juce_append_target_property(file_content HARDENED_RUNTIME_ENABLED             ${target} JUCE_HARDENED_RUNTIME_ENABLED)
    _juce_append_target_property(file_content APP_SANDBOX_ENABLED                  ${target} JUCE_APP_SANDBOX_ENABLED)
    _juce_append_target_property(file_content APP_SANDBOX_INHERIT                  ${target} JUCE_APP_SANDBOX_INHERIT)
    _juce_append_target_property(file_content HARDENED_RUNTIME_OPTIONS             ${target} JUCE_HARDENED_RUNTIME_OPTIONS)
    _juce_append_target_property(file_content APP_SANDBOX_OPTIONS                  ${target} JUCE_APP_SANDBOX_OPTIONS)
    _juce_append_target_property(file_content APP_GROUPS_ENABLED                   ${target} JUCE_APP_GROUPS_ENABLED)
    _juce_append_target_property(file_content APP_GROUP_IDS                        ${target} JUCE_APP_GROUP_IDS)
    _juce_append_target_property(file_content IS_PLUGIN                            ${target} JUCE_IS_PLUGIN)
    _juce_append_target_property(file_content ICLOUD_PERMISSIONS_ENABLED           ${target} JUCE_ICLOUD_PERMISSIONS_ENABLED)

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
        _juce_make_absolute_and_check(name)
        set(newline_delimited_input "${newline_delimited_input}${name}\n")
    endforeach()

    set(input_file_list "${juce_binary_data_folder}/input_file_list")
    file(WRITE "${input_file_list}" "${newline_delimited_input}")

    add_custom_command(OUTPUT ${binary_file_names}
        COMMAND juce::juceaide binarydata "${JUCE_ARG_NAMESPACE}" "${JUCE_ARG_HEADER_NAME}"
            ${juce_binary_data_folder} "${input_file_list}"
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
        DEPENDS "${input_file_list}"
        VERBATIM)

    target_sources(${target} PRIVATE "${binary_file_names}")
    target_include_directories(${target} INTERFACE ${juce_binary_data_folder})
    target_compile_features(${target} PRIVATE cxx_std_11)

    # This fixes an issue where Xcode is unable to find binary data during archive.
    if(CMAKE_GENERATOR STREQUAL "Xcode")
        set_target_properties(${target} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY "./")
    endif()

    if(JUCE_ARG_HEADER_NAME STREQUAL "BinaryData.h")
        target_compile_definitions(${target} INTERFACE JUCE_TARGET_HAS_BINARY_DATA=1)
    endif()
endfunction()

# ==================================================================================================

# math(EXPR ... OUTPUT_FORMAT HEXADECIMAL) wasn't added until 3.13, but we need 3.12 for vcpkg
# compatibility
function(_juce_dec_to_hex num out_var)
    while(num)
        math(EXPR digit "${num} % 16")
        math(EXPR num "${num} / 16")

        if(digit GREATER_EQUAL 10)
            math(EXPR ascii_code "${digit} + 55")
            string(ASCII "${ascii_code}" digit)
        endif()

        set(result "${digit}${result}")
    endwhile()

    set(${out_var} "${result}" PARENT_SCOPE)
endfunction()

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

    math(EXPR decimal "(${version_major} << 16) + (${version_minor} << 8) + ${version_patch}")
    _juce_dec_to_hex(${decimal} hex)
    set(${out_var} "${hex}" PARENT_SCOPE)
endfunction()

function(_juce_to_char_literal str out_var)
    string(APPEND str "    ") # Make sure there are at least 4 characters in the string.

    # Round-tripping through a file is the simplest way to convert a string to hex...
    string(SUBSTRING "${str}" 0 4 four_chars)
    string(RANDOM LENGTH 16 random_string)
    set(scratch_file "${CMAKE_CURRENT_BINARY_DIR}/${random_string}_ascii_conversion.txt")

    file(WRITE "${scratch_file}" "${four_chars}")
    file(READ "${scratch_file}" four_chars_hex HEX)
    file(REMOVE "${scratch_file}")

    set(${out_var} ${four_chars_hex} PARENT_SCOPE)
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

    execute_process(COMMAND "${juceaide_location}" ${ARGN} RESULT_VARIABLE result_variable)

    if(result_variable)
        message(FATAL_ERROR "Running juceaide failed")
    endif()
endfunction()

function(_juce_set_output_name target name)
    if(NOT CMAKE_SYSTEM_NAME STREQUAL "Android")
        set_target_properties(${target} PROPERTIES
            OUTPUT_NAME ${name}
            XCODE_ATTRIBUTE_PRODUCT_NAME ${name})
    endif()
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

        set(generated_icon "${juce_library_code}/Icon.icns")
        # To get compiled properly, we need the icon before the plist is generated!
        _juce_execute_juceaide(macicon "${generated_icon}" ${icon_args})
        set_source_files_properties(${generated_icon} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        if(NOT icon_args)
            return()
        endif()

        set(generated_icon "${juce_library_code}/icon.ico")
        _juce_execute_juceaide(winicon "${generated_icon}" ${icon_args})
    elseif(CMAKE_SYSTEM_NAME STREQUAL "iOS")
        get_target_property(generated_icon ${source_target} JUCE_CUSTOM_XCASSETS_FOLDER)

        if(icon_args AND (NOT generated_icon))
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
        XCODE_ATTRIBUTE_CODE_SIGN_ENTITLEMENTS "${entitlements_file}")
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

    set_target_properties(${dest_target} PROPERTIES
        XCODE_ATTRIBUTE_ENABLE_HARDENED_RUNTIME
            "$<TARGET_PROPERTY:${source_target},JUCE_HARDENED_RUNTIME_ENABLED>"
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

        set(install_path "$(LOCAL_APPS_DIR)")

        if(juce_kind_string STREQUAL "AUv3 AppExtension")
            set(install_path "${install_path}/${product_name}.app")

            if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
                set(install_path "${install_path}/PlugIns")
            else()
                set(install_path "${install_path}/Contents/PlugIns")
            endif()
        endif()

        set_target_properties(${dest_target} PROPERTIES
            XCODE_ATTRIBUTE_INSTALL_PATH "${install_path}"
            XCODE_ATTRIBUTE_SKIP_INSTALL "NO")
    endif()
endfunction()

function(_juce_add_resources_rc source_target dest_target)
    if(NOT CMAKE_SYSTEM_NAME STREQUAL "Windows")
        return()
    endif()

    get_target_property(juce_library_code ${source_target} JUCE_GENERATED_SOURCES_DIRECTORY)
    set(input_info_file "$<TARGET_PROPERTY:${source_target},JUCE_INFO_FILE>")

    get_target_property(generated_icon ${source_target} JUCE_ICON_FILE)
    set(dependency)

    if(generated_icon)
        set(dependency DEPENDS "${generated_icon}")
    endif()

    set(resource_rc_file "${juce_library_code}/resources.rc")

    add_custom_command(OUTPUT "${resource_rc_file}"
        COMMAND juce::juceaide rcfile "${input_info_file}" "${resource_rc_file}"
        ${dependency}
        VERBATIM)

    target_sources(${dest_target} PRIVATE "${resource_rc_file}")
endfunction()

function(_juce_configure_app_bundle source_target dest_target)
    set_target_properties(${dest_target} PROPERTIES
        JUCE_TARGET_KIND_STRING "App"
        MACOSX_BUNDLE TRUE
        WIN32_EXECUTABLE TRUE)

    _juce_add_resources_rc(${source_target} ${dest_target})

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

function(juce_enable_copy_plugin_step shared_code_target)
    get_target_property(active_targets "${shared_code_target}" JUCE_ACTIVE_PLUGIN_TARGETS)

    foreach(target IN LISTS active_targets)
        get_target_property(source "${target}" JUCE_PLUGIN_ARTEFACT_FILE)

        if(source)
            get_target_property(dest   "${target}" JUCE_PLUGIN_COPY_DIR)

            if(dest)
                _juce_copy_dir("${target}" "${source}" "$<GENEX_EVAL:${dest}>")
            else()
                message(WARNING "Target '${target}' requested copy but no destination is set")
            endif()
        endif()
    endforeach()
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

    if(kind STREQUAL "VST3")
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
                LIBRARY_OUTPUT_DIRECTORY "${output_path}/Contents/${JUCE_LINUX_TARGET_ARCHITECTURE}-linux")
        endif()

        _juce_set_copy_properties(${shared_code_target} ${target_name} "${output_path}" JUCE_VST3_COPY_DIR)
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
        target_link_libraries(${target_name} PRIVATE "-Wl,--no-undefined")
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
    add_dependencies(${shared_code_target}_All ${target_name})

    _juce_configure_bundle(${shared_code_target} ${target_name})
    _juce_set_plugin_target_properties(${shared_code_target} ${kind})
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
    if(CMAKE_VERSION VERSION_LESS "3.15.0")
        message(FATAL_ERROR "Plugin targets require CMake 3.15 or higher")
    endif()

    _juce_set_output_name(${target} $<TARGET_PROPERTY:${target},JUCE_PRODUCT_NAME>_SharedCode)

    target_link_libraries(${target} PRIVATE juce::juce_audio_plugin_client_utils)

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
    elseif((AAX IN_LIST active_formats) AND (NOT TARGET juce_aax_sdk))
        message(FATAL_ERROR "Use juce_set_aax_sdk_path to set up the AAX sdk before adding AAX targets")
    endif()

    _juce_add_standard_defs(${target})
    _juce_add_plugin_definitions(${target} PRIVATE ${active_formats})

    # The plugin wrappers need to know what other modules are available, especially
    # juce_audio_utils and juce_gui_basics. We achieve this by searching for
    # JUCE_MODULE_AVAILABLE_ private compile definitions, and reexporting them in
    # the interface compile definitions.
    # Unfortunately this requires CMake 3.15.
    _juce_get_module_definitions(${target} ON enabled_modules)
    target_compile_definitions(${target} INTERFACE ${enabled_modules})

    target_compile_definitions(${target} PRIVATE JUCE_SHARED_CODE=1)

    get_target_property(project_version_string ${target} JUCE_VERSION)
    _juce_version_code(${project_version_string} project_version_hex)

    get_target_property(project_manufacturer_code ${target} JUCE_PLUGIN_MANUFACTURER_CODE)
    get_target_property(project_plugin_code ${target} JUCE_PLUGIN_CODE)

    get_target_property(use_legacy_compatibility_plugin_code ${target} JUCE_USE_LEGACY_COMPATIBILITY_PLUGIN_CODE)

    if(use_legacy_compatibility_plugin_code)
        set(project_manufacturer_code "project_manufacturer_code-NOTFOUND")
    endif()

    _juce_to_char_literal(${project_manufacturer_code} project_manufacturer_code)
    _juce_to_char_literal(${project_plugin_code} project_plugin_code)

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
        JucePlugin_VersionCode=0x${project_version_hex}
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
        JucePlugin_VSTNumMidiOutputs=$<TARGET_PROPERTY:${target},JUCE_VST_NUM_MIDI_OUTS>)

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

    if(TARGET ${target}_AU)
        _juce_add_au_resource_fork(${target} ${target}_AU)
    endif()

    if(TARGET ${target}_AAX)
        target_link_libraries(${target}_AAX PRIVATE juce_aax_sdk)
    endif()

    if((TARGET ${target}_AUv3) AND (TARGET ${target}_Standalone))
        add_dependencies(${target}_Standalone ${target}_AUv3)
        # Copy the AUv3 into the Standalone app bundle
        _juce_copy_dir(${target}_Standalone
            "$<TARGET_BUNDLE_DIR:${target}_AUv3>"
            "$<TARGET_BUNDLE_CONTENT_DIR:${target}_Standalone>/PlugIns")
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

    _juce_set_property_if_not_set(${target} VERSION ${PROJECT_VERSION})

    get_target_property(final_version ${target} JUCE_VERSION)

    if(NOT final_version)
        message(FATAL_ERROR "Target ${target} must have its VERSION argument set, or must be part of a project with a PROJECT_VERSION")
    endif()

    get_target_property(custom_xcassets ${target} JUCE_CUSTOM_XCASSETS_FOLDER)

    set(needs_storyboard TRUE)

    if(custom_xcassets)
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

    _juce_set_property_if_not_set(${target} HARDENED_RUNTIME_ENABLED NO)
    _juce_set_property_if_not_set(${target} APP_SANDBOX_ENABLED NO)
    _juce_set_property_if_not_set(${target} APP_SANDBOX_INHERIT NO)

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
endfunction()

# ==================================================================================================

function(_juce_initialise_target target)
    set(one_value_args
        VERSION
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
        NEEDS_STORE_KIT                 # Set this true if you want in-app-purchases on Mac
        PUSH_NOTIFICATIONS_ENABLED
        HARDENED_RUNTIME_ENABLED
        APP_SANDBOX_ENABLED
        APP_SANDBOX_INHERIT

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
        DOCUMENT_EXTENSIONS
        AAX_CATEGORY
        IPHONE_SCREEN_ORIENTATIONS      # iOS only
        IPAD_SCREEN_ORIENTATIONS        # iOS only
        APP_GROUP_IDS)                  # iOS only

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

    if(JUCE_ENABLE_MODULE_SOURCE_GROUPS)
        get_property(all_modules GLOBAL PROPERTY _juce_module_names)

        foreach(module_name IN LISTS all_modules)
            get_target_property(path ${module_name} INTERFACE_JUCE_MODULE_PATH)
            get_target_property(header_files ${module_name} INTERFACE_JUCE_MODULE_HEADERS)
            get_target_property(source_files ${module_name} INTERFACE_JUCE_MODULE_SOURCES)
            source_group(TREE ${path} PREFIX "JUCE Modules" FILES ${header_files} ${source_files})
            set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
        endforeach()
    endif()
endfunction()

# ==================================================================================================

function(juce_add_console_app target)
    add_executable(${target})
    target_compile_definitions(${target} PRIVATE JUCE_STANDALONE_APPLICATION=1)

    if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        target_compile_definitions(${target} PRIVATE _CONSOLE=1)
    endif()

    _juce_initialise_target(${target} ${ARGN})
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
        message(FATAL_ERROR "PIP headers must declare a `name` field")
    endif()

    string(MAKE_C_IDENTIFIER "${JUCE_PIP_NAME}" pip_name_sanitised)

    if(NOT JUCE_PIP_NAME STREQUAL pip_name_sanitised)
        message(FATAL_ERROR "PIP `name` value '${JUCE_PIP_NAME}' must be a valid C identifier")
    endif()

    if(TARGET "${JUCE_PIP_NAME}")
        # We already added a target with this name, let's try using the filename instead
        get_filename_component(JUCE_PIP_NAME "${header}" NAME_WE)
    endif()

    if(TARGET "${JUCE_PIP_NAME}")
        message(FATAL_ERROR "Could not create a unique target name for PIP ${header}")
    endif()

    _juce_get_metadata("${metadata_dict}" type pip_kind)

    if(NOT pip_kind)
        message(FATAL_ERROR "PIP headers must declare a `type` field")
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

    if(pip_kind STREQUAL "AudioProcessor")
        set(source_main "${JUCE_CMAKE_UTILS_DIR}/PIPAudioProcessor.cpp.in")

        # We add AAX/VST2 targets too, if the user has set up those SDKs

        set(extra_formats)

        if(TARGET juce_aax_sdk)
            list(APPEND extra_formats AAX)
        endif()

        if(TARGET juce_vst2_sdk)
            list(APPEND extra_formats VST)
        endif()

        # Standalone plugins might want to access the mic
        list(APPEND extra_target_args MICROPHONE_PERMISSION_ENABLED TRUE)

        juce_add_plugin(${JUCE_PIP_NAME}
            FORMATS AU AUv3 VST3 Unity Standalone ${extra_formats}
            ${extra_target_args})
    elseif(pip_kind STREQUAL "Component")
        set(source_main "${JUCE_CMAKE_UTILS_DIR}/PIPComponent.cpp.in")
        juce_add_gui_app(${JUCE_PIP_NAME} ${extra_target_args})
    elseif(pip_kind STREQUAL "Console")
        set(source_main "${JUCE_CMAKE_UTILS_DIR}/PIPConsole.cpp.in")
        juce_add_console_app(${JUCE_PIP_NAME} ${extra_target_args})
    else()
        message(FATAL_ERROR "PIP kind must be either AudioProcessor, Component, or Console")
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
            message(FATAL_ERROR "No such module: ${module}")
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

    if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        add_library(juce_aax_sdk STATIC IMPORTED GLOBAL)
        set_target_properties(juce_aax_sdk PROPERTIES
            IMPORTED_LOCATION_DEBUG "${path}/Libs/Debug/libAAXLibrary_libcpp.a"
            IMPORTED_LOCATION "${path}/Libs/Release/libAAXLibrary_libcpp.a")
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        add_library(juce_aax_sdk INTERFACE IMPORTED GLOBAL)
    else()
        return()
    endif()

    target_include_directories(juce_aax_sdk INTERFACE
        "${path}"
        "${path}/Interfaces"
        "${path}/Interfaces/ACF")
    target_compile_definitions(juce_aax_sdk INTERFACE JucePlugin_AAXLibs_path="${path}/Libs")
    set_target_properties(juce_aax_sdk PROPERTIES INTERFACE_JUCE_AAX_DEFAULT_ICON "${path}/Utilities/PlugIn.ico")
endfunction()

function(juce_set_vst2_sdk_path path)
    if(TARGET juce_vst2_sdk)
        message(FATAL_ERROR "juce_set_vst2_sdk_path should only be called once")
    endif()

    _juce_make_absolute(path)

    if(NOT EXISTS "${path}")
        message(FATAL_ERROR "Could not find VST2 SDK at the specified path: ${path}")
    endif()

    add_library(juce_vst2_sdk INTERFACE IMPORTED GLOBAL)

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

    target_include_directories(juce_vst3_sdk INTERFACE "${path}")
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
