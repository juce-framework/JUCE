/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/


/*******************************************************************************
 The block below describes the properties of this module, and is read by
 the Projucer to automatically generate project code that uses it.
 For details about the syntax and how to create or use a module, see the
 JUCE Module Format.md file.


 BEGIN_JUCE_MODULE_DECLARATION

  ID:                 juce_audio_plugin_client
  vendor:             juce
  version:            7.0.0
  name:               JUCE audio plugin wrapper classes
  description:        Classes for building VST, VST3, AU, AUv3 and AAX plugins.
  website:            http://www.juce.com/juce
  license:            GPL/Commercial
  minimumCppStandard: 14

  dependencies:       juce_audio_processors

 END_JUCE_MODULE_DECLARATION

*******************************************************************************/


#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

/** Config: JUCE_VST3_CAN_REPLACE_VST2

    Enable this if you want your VST3 plug-in to load and save VST2 compatible
    state. This allows hosts to replace VST2 plug-ins with VST3 plug-ins. If
    you change this option then your VST3 plug-in will be incompatible with
    previous versions.
*/
#ifndef JUCE_VST3_CAN_REPLACE_VST2
 #define JUCE_VST3_CAN_REPLACE_VST2 1
#endif

/** Config: JUCE_FORCE_USE_LEGACY_PARAM_IDS

    Enable this if you want to force JUCE to use a continuous parameter
    index to identify a parameter in a DAW (this was the default in old
    versions of JUCE). This is index is usually used by the DAW to save
    automation data and enabling this may mess up user's DAW projects.
*/
#ifndef JUCE_FORCE_USE_LEGACY_PARAM_IDS
 #define JUCE_FORCE_USE_LEGACY_PARAM_IDS 0
#endif

/** Config: JUCE_FORCE_LEGACY_PARAMETER_AUTOMATION_TYPE

    Enable this if you want to force JUCE to use a legacy scheme for
    identifying plug-in parameters as either continuous or discrete.
    DAW projects with automation data written by an AudioUnit, VST3 or
    AAX plug-in built with JUCE version 5.1.1 or earlier may load
    incorrectly when opened by an AudioUnit, VST3 or AAX plug-in built
    with JUCE version 5.2.0 and later.
*/
#ifndef JUCE_FORCE_LEGACY_PARAMETER_AUTOMATION_TYPE
 #define JUCE_FORCE_LEGACY_PARAMETER_AUTOMATION_TYPE 0
#endif

/** Config: JUCE_USE_STUDIO_ONE_COMPATIBLE_PARAMETERS

    Enable this if you want JUCE to use parameter ids which are compatible
    with Studio One, as Studio One ignores any parameter ids which are negative.
    Enabling this option will make JUCE generate only positive parameter ids.
    Note that if you have already released a plug-in prior to JUCE 4.3.0 then
    enabling this will change your parameter ids, making your plug-in
    incompatible with old automation data.
*/
#ifndef JUCE_USE_STUDIO_ONE_COMPATIBLE_PARAMETERS
 #define JUCE_USE_STUDIO_ONE_COMPATIBLE_PARAMETERS 1
#endif

/** Config: JUCE_AU_WRAPPERS_SAVE_PROGRAM_STATES

    Enable this if you want to receive get/setProgramStateInformation calls,
    instead of get/setStateInformation calls, from the AU and AUv3 plug-in
    wrappers. In JUCE version 5.4.5 and earlier this was the default behaviour,
    so if you have modified the default implementations of get/setProgramStateInformation
    (where the default implementations simply call through to get/setStateInformation)
    then you may need to enable this configuration option to maintain backwards
    compatibility with previously saved state.
*/
#ifndef JUCE_AU_WRAPPERS_SAVE_PROGRAM_STATES
 #define JUCE_AU_WRAPPERS_SAVE_PROGRAM_STATES 0
#endif

/** Config: JUCE_STANDALONE_FILTER_WINDOW_USE_KIOSK_MODE

    Enable this if you want your standalone plugin window to use kiosk mode.
    By default, kiosk mode is enabled on iOS and Android.
*/

#ifndef JUCE_STANDALONE_FILTER_WINDOW_USE_KIOSK_MODE
 #define JUCE_STANDALONE_FILTER_WINDOW_USE_KIOSK_MODE (JUCE_IOS || JUCE_ANDROID)
#endif

#include "utility/juce_CreatePluginFilter.h"

#if JucePlugin_Enable_ARA
 #include "ARA/juce_ARA_Wrapper.h"
#endif
