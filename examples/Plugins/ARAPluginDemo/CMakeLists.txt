# modified copy of Example Audio Plugin CMakeLists.txt

# To get started on a new plugin, copy this entire folder (containing this file and C++ sources) to
# a convenient location, and then start making modifications.

# The first line of any CMake project should be a call to `cmake_minimum_required`, which checks
# that the installed CMake will be able to understand the following CMakeLists, and ensures that
# CMake's behaviour is compatible with the named version. This is a standard CMake command, so more
# information can be found in the CMake docs.

cmake_minimum_required(VERSION 3.15)

# The top-level CMakeLists.txt file for a project must contain a literal, direct call to the
# `project()` command. `project()` sets up some helpful variables that describe source/binary
# directories, and the current project version. This is a standard CMake command.

project(ARAPluginDemo VERSION 1.0.0)

# If you've installed JUCE somehow (via a package manager, or directly using the CMake install
# target), you'll need to tell this project that it depends on the installed copy of JUCE. If you've
# included JUCE directly in your source tree (perhaps as a submodule), you'll need to tell CMake to
# include that subdirectory as part of the build.

# find_package(JUCE CONFIG REQUIRED)        # If you've installed JUCE to your system
# or
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../../.." "${CMAKE_CURRENT_BINARY_DIR}/JUCE")	# If you've put JUCE in a subdirectory called JUCE

# If you are building a VST2 or AAX plugin, CMake needs to be told where to find these SDKs on your
# system. This setup should be done before calling `juce_add_plugin`.

# juce_set_vst2_sdk_path(...)
# juce_set_aax_sdk_path(...)

# `juce_add_plugin` adds a static library target with the name passed as the first argument
# (AudioPluginExample here). This target is a normal CMake target, but has a lot of extra properties set
# up by default. As well as this shared code static library, this function adds targets for each of
# the formats specified by the FORMATS arguments. This function accepts many optional arguments.
# Check the readme at `docs/CMake API.md` in the JUCE repo for the full list.

juce_add_plugin(ARAPluginDemo
    # VERSION ...                               # Set this if the plugin version is different to the project version
    # ICON_BIG ...                              # ICON_* arguments specify a path to an image file to use as an icon for the Standalone
    # ICON_SMALL ...
    COMPANY_NAME "ARA Demo Company"             # Specify the name of the plugin's author
    COMPANY_WEBSITE "https://www.arademocompany.com"
    COMPANY_EMAIL "info@arademocompany.com"
    COMPANY_COPYRIGHT "Copyright (c) 2012-2021, ARA Demo Company, All Rights Reserved."
    BUNDLE_ID "com.arademocompany.ARAPluginDemo"
    IS_SYNTH FALSE                              # Is this a synth or an effect?
    NEEDS_MIDI_INPUT FALSE                      # Does the plugin need midi input?
    NEEDS_MIDI_OUTPUT FALSE                     # Does the plugin need midi output?
    IS_MIDI_EFFECT FALSE                        # Is this plugin a MIDI effect?
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE           # Does the editor need keyboard focus?
    COPY_PLUGIN_AFTER_BUILD TRUE                # Should the plugin be installed to a default location after building?
    PLUGIN_MANUFACTURER_CODE ADeC               # A four-character manufacturer id with at least one upper-case character
    PLUGIN_CODE AraD                            # A unique four-character plugin id with exactly one upper-case character
                                                # GarageBand 10.3 requires the first letter to be upper-case, and the remaining letters to be lower-case
                                                # However this ID has been assigned before that behaviour was discovered, and remains as-is for backwards
                                                # compatibility - GarageBand 10.3 does not support ARA anyways.
    DESCRIPTION "Example plug-in for JUCE_ARA, showcasing ARA's UI integration features"
    FORMATS AU VST3                             # The formats to build. Other valid formats are: AAX Unity VST AU AUv3
    # PRODUCT_NAME ARAPluginDemo                # The name of the final executable, which can differ from the target name
    VST3_CATEGORIES "Tools"
    AU_SANDBOX_SAFE TRUE
    APP_SANDBOX_ENABLED TRUE
    HARDENED_RUNTIME_ENABLED TRUE
    IS_ARA_EFFECT TRUE                          # Are we using ARA?
    # ARA_FACTORY_ID "..."                      # Optional custom ARA factory ID (otherwise generated from bundle ID and version)
    # ARA_DOCUMENT_ARCHIVE_ID "..."             # Optional custom ARA document archive ID (otherwise generated from bundle ID)
                                                # This must be manually updated whenever changing the archive in non-backwards compatible ways!
                                                # Note that if changing the document archive ID, you'll also have to add the previous ID to JucePlugin_ARACompatibleArchiveIDs!
    # ARA_COMPATIBLE_ARCHIVE_IDS "...", "..."   # Initially empty, but will indicate upwards compatibility when changing ARA_DOCUMENT_ARCHIVE_ID.
    # ARA_ANALYSIS_TYPES                        # If providing analyzable ARA content types to the host, define them as OR'd values here - defaults to 0.
    # ARA_TRANSFORMATION_FLAGS                  # If supporting time-stretching or other ARA playback transformations, define them as OR'd values here - otherwise 0.
)

# TODO path to the ARA SDK - create a git submodule and use the proper path there once ARA is open-sourced
if(NOT JUCE_GLOBAL_ARA_SDK_PATH)
    message(FATAL_ERROR "Path to ARA SDK not set, specify -DJUCE_GLOBAL_ARA_SDK_PATH=/path/to/sdk")
endif()
_juce_make_absolute(JUCE_GLOBAL_ARA_SDK_PATH)
if((NOT EXISTS "${JUCE_GLOBAL_ARA_SDK_PATH}")
   OR (NOT EXISTS "${JUCE_GLOBAL_ARA_SDK_PATH}/ARA_API")
   OR (NOT EXISTS "${JUCE_GLOBAL_ARA_SDK_PATH}/ARA_Library"))
    message(FATAL_ERROR "Could not find ARA SDK at the specified path: ${JUCE_GLOBAL_ARA_SDK_PATH}")
endif()
target_include_directories(ARAPluginDemo PRIVATE "${JUCE_GLOBAL_ARA_SDK_PATH}")

# `juce_generate_juce_header` will create a JuceHeader.h for a given target, which will be generated
# into your build tree. This should be included with `#include <JuceHeader.h>`. The include path for
# this header will be automatically added to the target. The main function of the JuceHeader is to
# include all your JUCE module headers; if you're happy to include module headers directly, you
# probably don't need to call this.

#juce_generate_juce_header(ARAPluginDemo)

# `target_sources` adds source files to a target. We pass the target that needs the sources as the
# first argument, then a visibility parameter for the sources which should normally be PRIVATE.
# Finally, we supply a list of source files that will be built into the target. This is a standard
# CMake command.

target_sources(ARAPluginDemo
    PRIVATE
        Source/ARAPluginDemoAudioModification.h
        Source/ARAPluginDemoAudioProcessor.cpp
        Source/ARAPluginDemoAudioProcessor.h
        Source/ARAPluginDemoAudioProcessorEditor.cpp
        Source/ARAPluginDemoAudioProcessorEditor.h
        Source/ARAPluginDemoDocumentController.cpp
        Source/ARAPluginDemoDocumentController.h
        Source/ARAPluginDemoPlaybackRenderer.cpp
        Source/ARAPluginDemoPlaybackRenderer.h
        Source/DocumentView.cpp
        Source/DocumentView.h
        Source/MusicalContextView.cpp
        Source/MusicalContextView.h
        Source/PlaybackRegionView.cpp
        Source/PlaybackRegionView.h
        Source/RegionSequenceHeaderView.cpp
        Source/RegionSequenceHeaderView.h
        Source/RegionSequenceViewContainer.cpp
        Source/RegionSequenceViewContainer.h)

# `target_compile_definitions` adds some preprocessor definitions to our target. In a Projucer
# project, these might be passed in the 'Preprocessor Definitions' field. JUCE modules also make use
# of compile definitions to switch certain features on/off, so if there's a particular feature you
# need that's not on by default, check the module header for the correct flag to set here. These
# definitions will be visible both to your code, and also the JUCE module code, so for new
# definitions, pick unique names that are unlikely to collide! This is a standard CMake command.

target_compile_definitions(ARAPluginDemo
    PUBLIC
        # JUCE_WEB_BROWSER and JUCE_USE_CURL would be on by default, but you might not need them.
        JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_plugin` call
        JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_plugin` call
        JUCE_VST3_CAN_REPLACE_VST2=0)

# If your target needs extra binary assets, you can add them here. The first argument is the name of
# a new static library target that will include all the binary resources. There is an optional
# `NAMESPACE` argument that can specify the namespace of the generated binary data class. Finally,
# the SOURCES argument should be followed by a list of source files that should be built into the
# static library. These source files can be of any kind (wav data, images, fonts, icons etc.).
# Conversion to binary-data will happen when your target is built.

# juce_add_binary_data(AudioPluginData SOURCES ...)

# `target_link_libraries` links libraries and JUCE modules to other libraries or executables. Here,
# we're linking our executable target to the `juce::juce_audio_utils` module. Inter-module
# dependencies are resolved automatically, so `juce_core`, `juce_events` and so on will also be
# linked automatically. If we'd generated a binary data target above, we would need to link to it
# here too. This is a standard CMake command.

target_link_libraries(ARAPluginDemo
    PRIVATE
        # ARAPluginDemo           # If we'd created a binary data target, we'd link to it here
        juce::juce_audio_utils
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)
