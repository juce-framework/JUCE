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

cmake_minimum_required(VERSION 3.15)

project(JUCE VERSION 7.0.2 LANGUAGES C CXX)

include(CMakeDependentOption)

set_property(GLOBAL PROPERTY USE_FOLDERS YES)

set(JUCE_MODULES_DIR "${JUCE_SOURCE_DIR}/modules" CACHE INTERNAL
    "The path to JUCE modules")

# This option will disable most of the JUCE helper functions and tools. This option exists to
# facilitate existing CMake builds which handle things like bundle creation, icons, plists, and
# binary data independently of JUCE. This option is not recommended - use at your own risk!

option(JUCE_MODULES_ONLY "Only configure the JUCE modules" OFF)

include(extras/Build/CMake/JUCEModuleSupport.cmake)

# This option controls whether dummy targets are added to the build, where these targets contain all
# of the source files for each JUCE module. If you're planning to use an IDE and want to be able to
# browse all of JUCE's source files, this may be useful. However, it will increase the size of
# generated IDE projects and might slow down configuration a bit. If you enable this, you should
# probably also add `set_property(GLOBAL PROPERTY USE_FOLDERS YES)` to your top level CMakeLists,
# otherwise the module sources will be added directly to the top level of the project, instead of in
# a nice 'Modules' subfolder.

cmake_dependent_option(JUCE_ENABLE_MODULE_SOURCE_GROUPS
    "Show all module sources in IDE projects" OFF
    "NOT JUCE_MODULES_ONLY" OFF)

add_subdirectory(modules)

if(JUCE_MODULES_ONLY)
    return()
endif()

include(extras/Build/CMake/JUCEUtils.cmake)

set_directory_properties(PROPERTIES
    JUCE_COMPANY_NAME       "JUCE"
    JUCE_COMPANY_WEBSITE    "https://juce.com"
    JUCE_COMPANY_EMAIL      "info@juce.com"
    JUCE_COMPANY_COPYRIGHT  "Copyright (c) 2020 - Raw Material Software Limited")

option(JUCE_COPY_PLUGIN_AFTER_BUILD
    "Whether or not plugins should be installed to the system after building" OFF)
set_property(GLOBAL PROPERTY JUCE_COPY_PLUGIN_AFTER_BUILD ${JUCE_COPY_PLUGIN_AFTER_BUILD})

set(CMAKE_CXX_EXTENSIONS FALSE)

juce_disable_default_flags()

add_subdirectory(extras/Build)

# If you want to build the JUCE examples with VST2/AAX/ARA support, you'll need to make the
# VST2/AAX/ARA headers visible to the juce_audio_processors module. You can either set the paths on
# the command line, (e.g. -DJUCE_GLOBAL_AAX_SDK_PATH=/path/to/sdk) if you're just building the JUCE
# examples, or you can call the `juce_set_*_sdk_path` functions in your own CMakeLists after
# importing JUCE.

if(JUCE_GLOBAL_AAX_SDK_PATH)
    juce_set_aax_sdk_path("${JUCE_GLOBAL_AAX_SDK_PATH}")
endif()

if(JUCE_GLOBAL_VST2_SDK_PATH)
    juce_set_vst2_sdk_path("${JUCE_GLOBAL_VST2_SDK_PATH}")
endif()

# The ARA_SDK path should point to the "Umbrella installer" ARA_SDK directory.
# The directory can be obtained by recursively cloning https://github.com/Celemony/ARA_SDK and
# checking out the tag releases/2.1.0.
if(JUCE_GLOBAL_ARA_SDK_PATH)
    juce_set_ara_sdk_path("${JUCE_GLOBAL_ARA_SDK_PATH}")
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
              "${JUCE_CMAKE_UTILS_DIR}/JUCECheckAtomic.cmake"
              "${JUCE_CMAKE_UTILS_DIR}/JUCEHelperTargets.cmake"
              "${JUCE_CMAKE_UTILS_DIR}/JUCEModuleSupport.cmake"
              "${JUCE_CMAKE_UTILS_DIR}/JUCEUtils.cmake"
              "${JUCE_CMAKE_UTILS_DIR}/JuceLV2Defines.h.in"
              "${JUCE_CMAKE_UTILS_DIR}/LaunchScreen.storyboard"
              "${JUCE_CMAKE_UTILS_DIR}/PIPAudioProcessor.cpp.in"
              "${JUCE_CMAKE_UTILS_DIR}/PIPAudioProcessorWithARA.cpp.in"
              "${JUCE_CMAKE_UTILS_DIR}/PIPComponent.cpp.in"
              "${JUCE_CMAKE_UTILS_DIR}/PIPConsole.cpp.in"
              "${JUCE_CMAKE_UTILS_DIR}/RecentFilesMenuTemplate.nib"
              "${JUCE_CMAKE_UTILS_DIR}/UnityPluginGUIScript.cs.in"
              "${JUCE_CMAKE_UTILS_DIR}/checkBundleSigning.cmake"
              "${JUCE_CMAKE_UTILS_DIR}/copyDir.cmake"
              "${JUCE_CMAKE_UTILS_DIR}/juce_runtime_arch_detection.cpp"
        DESTINATION "${JUCE_INSTALL_DESTINATION}")

install(EXPORT LV2_HELPER NAMESPACE juce:: DESTINATION "${JUCE_INSTALL_DESTINATION}")
