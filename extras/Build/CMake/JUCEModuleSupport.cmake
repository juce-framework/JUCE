# ==============================================================================
#
#  This file is part of the JUCE library.
#  Copyright (c) 2022 - Raw Material Software Limited
#
#  JUCE is an open source library subject to commercial or open-source
#  licensing.
#
#  By using JUCE, you agree to the terms of both the JUCE 7 End-User License
#  Agreement and JUCE Privacy Policy.
#
#  End User License Agreement: www.juce.com/juce-7-licence
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
# JUCE Modules Support Helper Functions
#
# In this file, functions intended for use by end-users have the prefix `juce_`.
# Functions beginning with an underscore should be considered private and susceptible to
# change, so don't call them directly.
#
# See the readme at `docs/CMake API.md` for more information about CMake usage,
# including documentation of the public functions in this file.
# ==================================================================================================

include_guard(GLOBAL)
cmake_minimum_required(VERSION 3.15)

# ==================================================================================================

set(JUCE_CMAKE_UTILS_DIR ${CMAKE_CURRENT_LIST_DIR}
    CACHE INTERNAL "The path to the folder holding this file and other resources")

include("${JUCE_CMAKE_UTILS_DIR}/JUCEHelperTargets.cmake")
include("${JUCE_CMAKE_UTILS_DIR}/JUCECheckAtomic.cmake")

# Tries to discover the target platform architecture, which is necessary for
# naming VST3 bundle folders and including bundled libraries from modules
function(_juce_find_target_architecture result)
    set(test_file "${JUCE_CMAKE_UTILS_DIR}/juce_runtime_arch_detection.cpp")
    try_compile(compile_result "${CMAKE_CURRENT_BINARY_DIR}" "${test_file}"
        OUTPUT_VARIABLE compile_output)
    string(REGEX REPLACE ".*JUCE_ARCH ([a-zA-Z0-9_-]*).*" "\\1" match_result "${compile_output}")
    set("${result}" "${match_result}" PARENT_SCOPE)
endfunction()

if((CMAKE_SYSTEM_NAME STREQUAL "Linux") OR (CMAKE_SYSTEM_NAME MATCHES ".*BSD") OR MSYS OR MINGW)
    # If you really need to override the detected arch for some reason,
    # you can configure the build with -DJUCE_TARGET_ARCHITECTURE=<custom arch>
    if(NOT DEFINED JUCE_TARGET_ARCHITECTURE)
        _juce_find_target_architecture(target_arch)
        set(JUCE_TARGET_ARCHITECTURE "${target_arch}"
            CACHE INTERNAL "The target architecture, used to name internal folders in VST3 bundles, and to locate bundled libraries in modules")
    endif()
endif()

# ==================================================================================================

function(_juce_add_interface_library target)
    add_library(${target} INTERFACE)
    target_sources(${target} INTERFACE ${ARGN})
endfunction()

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
        get_filename_component(${path} "${${path}}" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_LIST_DIR}")
    endif()

    string(REGEX REPLACE "\\\\" "/" ${path} "${${path}}")
endmacro()

macro(_juce_make_absolute_and_check path)
    _juce_make_absolute("${path}")

    if(NOT EXISTS "${${path}}")
        message(FATAL_ERROR "No file at path ${${path}}")
    endif()
endmacro()

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
            if(line MATCHES "[\t ]*BEGIN_${delim_str}[\t ]*")
                set(append YES)
            endif()

            continue()
        endif()

        if(append AND (line MATCHES "[\t ]*END_${delim_str}[\t ]*"))
            break()
        endif()

        if(line MATCHES "^[\t ]*([a-zA-Z]+):")
            set(last_written_key "${CMAKE_MATCH_1}")
        endif()

        string(REGEX REPLACE "^[\t ]*${last_written_key}:[\t ]*" "" line "${line}")
        string(REGEX REPLACE "[\t ,]+" ";" line "${line}")

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
    set(${out} AU AUv3 AAX LV2 Standalone Unity VST VST3 PARENT_SCOPE)
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
        list(APPEND result Unity VST VST3 LV2)
    endif()

    if(CMAKE_SYSTEM_NAME STREQUAL "Darwin" OR CMAKE_SYSTEM_NAME STREQUAL "Windows")
        list(APPEND result AAX)
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

# Takes a target, a link visibility, if it should be a weak link, and a variable-length list of
# framework names. On macOS, for non-weak links, this finds the requested frameworks using
# `find_library`.
function(_juce_link_frameworks target visibility)
    set(options WEAK)
    cmake_parse_arguments(JUCE_LINK_FRAMEWORKS "${options}" "" "" ${ARGN})
    foreach(framework IN LISTS JUCE_LINK_FRAMEWORKS_UNPARSED_ARGUMENTS)
        if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
            if(JUCE_LINK_FRAMEWORKS_WEAK)
                set(framework_flags "-weak_framework ${framework}")
            else()
                find_library("juce_found_${framework}" "${framework}" REQUIRED)
                set(framework_flags "${juce_found_${framework}}")
            endif()
        elseif(CMAKE_SYSTEM_NAME STREQUAL "iOS")
            # CoreServices is only available on iOS 12+, we must link it weakly on earlier platforms
            if(JUCE_LINK_FRAMEWORKS_WEAK OR ((framework STREQUAL "CoreServices") AND (CMAKE_OSX_DEPLOYMENT_TARGET LESS 12.0)))
                set(framework_flags "-weak_framework ${framework}")
            else()
                set(framework_flags "-framework ${framework}")
            endif()
        endif()
        if(NOT framework_flags STREQUAL "")
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

    target_compile_features("${target_name}" INTERFACE cxx_std_14)
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

function(_juce_add_library_path target path)
    if(EXISTS "${path}")
        target_link_directories(${target} INTERFACE ${path})
    endif()
endfunction()

function(_juce_add_module_staticlib_paths module_target module_path)
    if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        _juce_add_library_path(${module_target} "${module_path}/libs/MacOSX")
    elseif(CMAKE_SYSTEM_NAME STREQUAL "iOS")
        _juce_add_library_path(${module_target} "${module_path}/libs/iOS")
    elseif((CMAKE_SYSTEM_NAME STREQUAL "Linux") OR (CMAKE_SYSTEM_NAME MATCHES ".*BSD"))
        _juce_add_library_path(${module_target} "${module_path}/libs/Linux/${JUCE_TARGET_ARCHITECTURE}")
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        if(CMAKE_GENERATOR MATCHES "Visual Studio [0-9]+ (20[0-9]+)")
            set(arch "$<IF:$<EQUAL:${CMAKE_SIZEOF_VOID_P},8>,x64,Win32>")

            if(NOT CMAKE_GENERATOR_PLATFORM STREQUAL "")
                set(arch ${CMAKE_GENERATOR_PLATFORM})
            endif()

            set(runtime_lib "$<GENEX_EVAL:$<TARGET_PROPERTY:MSVC_RUNTIME_LIBRARY>>")
            set(subfolder "MDd")
            set(subfolder "$<IF:$<STREQUAL:${runtime_lib},MultiThreadedDebug>,MTd,${subfolder}>")
            set(subfolder "$<IF:$<STREQUAL:${runtime_lib},MultiThreadedDLL>,MD,${subfolder}>")
            set(subfolder "$<IF:$<STREQUAL:${runtime_lib},MultiThreaded>,MT,${subfolder}>")
            target_link_directories(${module_target} INTERFACE
                "${module_path}/libs/VisualStudio${CMAKE_MATCH_1}/${arch}/${subfolder}")
        elseif(MSYS OR MINGW)
            _juce_add_library_path(${module_target} "${module_path}/libs/MinGW/${JUCE_TARGET_ARCHITECTURE}")
        endif()
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Android")
        _juce_add_library_path(${module_target} "${module_path}/libs/Android/${CMAKE_ANDROID_ARCH_ABI}")
    endif()
endfunction()

# ==================================================================================================

function(_juce_remove_empty_list_elements arg)
    list(FILTER ${arg} EXCLUDE REGEX "^$")
    set(${arg} ${${arg}} PARENT_SCOPE)
endfunction()

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

        add_library(juce_lilv_headers INTERFACE)
        set(lv2_base_path "${base_path}/juce_audio_processors/format_types/LV2_SDK")
        target_include_directories(juce_lilv_headers INTERFACE
            "${lv2_base_path}"
            "${lv2_base_path}/lv2"
            "${lv2_base_path}/serd"
            "${lv2_base_path}/sord"
            "${lv2_base_path}/sord/src"
            "${lv2_base_path}/sratom"
            "${lv2_base_path}/lilv"
            "${lv2_base_path}/lilv/src")
        target_link_libraries(juce_audio_processors INTERFACE juce_lilv_headers)

        add_library(juce_ara_headers INTERFACE)

        target_include_directories(juce_ara_headers INTERFACE
            "$<$<TARGET_EXISTS:juce_ara_sdk>:$<TARGET_PROPERTY:juce_ara_sdk,INTERFACE_INCLUDE_DIRECTORIES>>")

        target_link_libraries(juce_audio_processors INTERFACE juce_ara_headers)

        if(JUCE_ARG_ALIAS_NAMESPACE)
            add_library(${JUCE_ARG_ALIAS_NAMESPACE}::juce_vst3_headers ALIAS juce_vst3_headers)
            add_library(${JUCE_ARG_ALIAS_NAMESPACE}::juce_lilv_headers ALIAS juce_lilv_headers)
            add_library(${JUCE_ARG_ALIAS_NAMESPACE}::juce_ara_headers ALIAS juce_ara_headers)
        endif()
    endif()

    target_include_directories(${module_name} INTERFACE ${base_path})

    target_compile_definitions(${module_name} INTERFACE JUCE_MODULE_AVAILABLE_${module_name}=1)

    if((CMAKE_SYSTEM_NAME STREQUAL "Linux") OR (CMAKE_SYSTEM_NAME MATCHES ".*BSD"))
        target_compile_definitions(${module_name} INTERFACE LINUX=1)
    endif()

    if((${module_name} STREQUAL "juce_audio_devices") AND (CMAKE_SYSTEM_NAME STREQUAL "Android"))
        add_subdirectory("${module_path}/native/oboe")
        target_link_libraries(${module_name} INTERFACE oboe)
    endif()

    if((${module_name} STREQUAL "juce_opengl") AND (CMAKE_SYSTEM_NAME STREQUAL "Android"))
        set(platform_supports_gl3 0)

        if(CMAKE_SYSTEM_VERSION VERSION_GREATER_EQUAL 18)
            set(platform_supports_gl3 1)
        endif()

        if(platform_supports_gl3)
            target_compile_definitions(${module_name} INTERFACE JUCE_ANDROID_GL_ES_VERSION_3_0=1)
        endif()

        target_link_libraries(${module_name} INTERFACE EGL $<IF:${platform_supports_gl3},GLESv3,GLESv2>)
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

        _juce_remove_empty_list_elements(module_osxframeworks)
        foreach(module_framework IN LISTS module_osxframeworks)
            _juce_link_frameworks("${module_name}" INTERFACE "${module_framework}")
        endforeach()

        _juce_get_metadata("${metadata_dict}" WeakOSXFrameworks module_weakosxframeworks)

        _juce_remove_empty_list_elements(module_weakosxframeworks)
        foreach(module_framework IN LISTS module_weakosxframeworks)
            _juce_link_frameworks("${module_name}" INTERFACE WEAK "${module_framework}")
        endforeach()

        _juce_link_libs_from_metadata("${module_name}" "${metadata_dict}" OSXLibs)
    elseif(CMAKE_SYSTEM_NAME STREQUAL "iOS")
        _juce_get_metadata("${metadata_dict}" iOSFrameworks module_iosframeworks)

        _juce_remove_empty_list_elements(module_iosframeworks)
        foreach(module_framework IN LISTS module_iosframeworks)
            _juce_link_frameworks("${module_name}" INTERFACE "${module_framework}")
        endforeach()

        _juce_get_metadata("${metadata_dict}" WeakiOSFrameworks module_weakiosframeworks)

        _juce_remove_empty_list_elements(module_weakiosframeworks)
        foreach(module_framework IN LISTS module_weakiosframeworks)
            _juce_link_frameworks("${module_name}" INTERFACE WEAK "${module_framework}")
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
        if((CMAKE_CXX_COMPILER_ID STREQUAL "MSVC") OR (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC"))
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

    _juce_add_module_staticlib_paths("${module_name}" "${module_path}")

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

# When source groups are enabled, this function sets the HEADER_FILE_ONLY property on any module
# source files that should not be built. This is called automatically by the juce_add_* functions.
function(_juce_fixup_module_source_groups)
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
