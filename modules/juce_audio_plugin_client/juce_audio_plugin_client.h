/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

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
 JUCE Module Format.txt file.


 BEGIN_JUCE_MODULE_DECLARATION

  ID:               juce_audio_plugin_client
  vendor:           juce
  version:          5.1.2
  name:             JUCE audio plugin wrapper classes
  description:      Classes for building VST, VST3, AudioUnit, AAX and RTAS plugins.
  website:          http://www.juce.com/juce
  license:          GPL/Commercial

  dependencies:     juce_gui_basics, juce_audio_basics, juce_audio_processors

 END_JUCE_MODULE_DECLARATION

*******************************************************************************/


#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

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
    with JUCE version 5.1.2 and later.
*/
#ifndef JUCE_FORCE_LEGACY_PARAMETER_AUTOMATION_TYPE
 #define JUCE_FORCE_LEGACY_PARAMETER_AUTOMATION_TYPE 0
#endif

/** Config: JUCE_USE_STUDIO_ONE_COMPATIBLE_PARAMETERS

    Enable this if you want JUCE to use parameter ids which are compatible
    with Studio One. Studio One ignores any parameter ids which are negative.
    Enabling this option will make JUCE generate only positive parameter ids.
    Note that if you have already released a plug-in prior to JUCE 4.3.0 then
    enabling this will change your parameter ids making your plug-in
    incompatible to old automation data.
*/
#ifndef JUCE_USE_STUDIO_ONE_COMPATIBLE_PARAMETERS
 #define JUCE_USE_STUDIO_ONE_COMPATIBLE_PARAMETERS 1
#endif

#include "utility/juce_PluginHostType.h"
#include "VST/juce_VSTCallbackHandler.h"
