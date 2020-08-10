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

cmake_minimum_required(VERSION 3.12)

project(JUCE VERSION 6.0.0 LANGUAGES C CXX)

set_property(GLOBAL PROPERTY USE_FOLDERS YES)

set(JUCE_MODULES_DIR "${JUCE_SOURCE_DIR}/modules" CACHE INTERNAL
    "The path to JUCE modules")

include(extras/Build/CMake/JUCEUtils.cmake)

juce_disable_default_flags()

set_directory_properties(PROPERTIES
    JUCE_COMPANY_NAME       "JUCE"
    JUCE_COMPANY_WEBSITE    "juce.com"
    JUCE_COMPANY_EMAIL      "info@juce.com"
    JUCE_COMPANY_COPYRIGHT  "Copyright (c) 2020 - Raw Material Software Limited")

option(JUCE_COPY_PLUGIN_AFTER_BUILD
    "Whether or not plugins should be installed to the system after building" OFF)
set_property(GLOBAL PROPERTY JUCE_COPY_PLUGIN_AFTER_BUILD ${JUCE_COPY_PLUGIN_AFTER_BUILD})

set(CMAKE_CXX_EXTENSIONS FALSE)

add_subdirectory(modules)
add_subdirectory(extras/Build)

# If you want to build the JUCE examples with VST2/AAX support, you'll need to make the VST2/AAX
# headers visible to the juce_audio_processors module. You can either set the paths on the command
# line, (e.g. -DJUCE_GLOBAL_AAX_SDK_PATH=/path/to/sdk) if you're just building the JUCE examples, or
# you can call the `juce_set_*_sdk_path` functions in your own CMakeLists after importing JUCE.

if(JUCE_GLOBAL_AAX_SDK_PATH)
    juce_set_aax_sdk_path("${JUCE_GLOBAL_AAX_SDK_PATH}")
endif()

if(JUCE_GLOBAL_VST2_SDK_PATH)
    juce_set_vst2_sdk_path("${JUCE_GLOBAL_VST2_SDK_PATH}")
endif()

# We don't build anything other than the juceaide by default, because we want to keep configuration
# speedy and the number of targets low. If you want to add targets for the extra projects and
# example PIPs (there's a lot of them!), specify -DJUCE_BUILD_EXAMPLES=ON and/or
# -DJUCE_BUILD_EXTRAS=ON when initially generating your build tree.

option(JUCE_BUILD_EXTRAS "Add build targets for the Projucer and other tools" OFF)

if(JUCE_BUILD_EXTRAS)
    add_subdirectory(extras)
endif()

option(JUCE_BUILD_EXAMPLES "Add build targets for the DemoRunner and PIPs" OFF)

if(JUCE_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# ==================================================================================================
# Install configuration

if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.14")
    set(extra_version_arg ARCH_INDEPENDENT)
endif()

include(CMakePackageConfigHelpers)
write_basic_package_version_file("${JUCE_BINARY_DIR}/JUCEConfigVersion.cmake"
    VERSION ${JUCE_VERSION}
    COMPATIBILITY ExactVersion
    ${extra_version_arg})

set(JUCE_INSTALL_DESTINATION "lib/cmake/JUCE-${JUCE_VERSION}" CACHE STRING
    "The location, relative to the install prefix, where the JUCE config file will be installed")

set(JUCE_MODULE_PATH "include/JUCE-${JUCE_VERSION}/modules")
set(UTILS_INSTALL_DIR "${JUCE_INSTALL_DESTINATION}")
set(JUCEAIDE_PATH "${JUCE_TOOL_INSTALL_DIR}/${JUCE_JUCEAIDE_NAME}")
configure_package_config_file("${JUCE_CMAKE_UTILS_DIR}/JUCEConfig.cmake.in"
    "${JUCE_BINARY_DIR}/JUCEConfig.cmake"
    PATH_VARS UTILS_INSTALL_DIR JUCEAIDE_PATH JUCE_MODULE_PATH
    INSTALL_DESTINATION "${JUCE_INSTALL_DESTINATION}")

set(JUCE_MODULE_PATH "${JUCE_MODULES_DIR}")
set(UTILS_INSTALL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/extras/Build/CMake")
get_target_property(JUCEAIDE_PATH juceaide IMPORTED_LOCATION)
configure_package_config_file("${JUCE_CMAKE_UTILS_DIR}/JUCEConfig.cmake.in"
    "${JUCE_BINARY_DIR}/JUCEExportConfig.cmake"
    PATH_VARS UTILS_INSTALL_DIR JUCEAIDE_PATH JUCE_MODULE_PATH
    INSTALL_PREFIX "${CMAKE_CURRENT_BINARY_DIR}"
    INSTALL_DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")

install(FILES "${JUCE_BINARY_DIR}/JUCEConfigVersion.cmake"
              "${JUCE_BINARY_DIR}/JUCEConfig.cmake"
              "${JUCE_CMAKE_UTILS_DIR}/LaunchScreen.storyboard"
              "${JUCE_CMAKE_UTILS_DIR}/PIPAudioProcessor.cpp.in"
              "${JUCE_CMAKE_UTILS_DIR}/PIPComponent.cpp.in"
              "${JUCE_CMAKE_UTILS_DIR}/PIPConsole.cpp.in"
              "${JUCE_CMAKE_UTILS_DIR}/RecentFilesMenuTemplate.nib"
              "${JUCE_CMAKE_UTILS_DIR}/UnityPluginGUIScript.cs.in"
              "${JUCE_CMAKE_UTILS_DIR}/copyDir.cmake"
              "${JUCE_CMAKE_UTILS_DIR}/JUCEHelperTargets.cmake"
              "${JUCE_CMAKE_UTILS_DIR}/JUCEUtils.cmake"
        DESTINATION "${JUCE_INSTALL_DESTINATION}")
