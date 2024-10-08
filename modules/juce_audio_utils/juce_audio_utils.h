/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/


/*******************************************************************************
 The block below describes the properties of this module, and is read by
 the Projucer to automatically generate project code that uses it.
 For details about the syntax and how to create or use a module, see the
 JUCE Module Format.md file.


 BEGIN_JUCE_MODULE_DECLARATION

  ID:                 juce_audio_utils
  vendor:             juce
  version:            8.0.2
  name:               JUCE extra audio utility classes
  description:        Classes for audio-related GUI and miscellaneous tasks.
  website:            http://www.juce.com/juce
  license:            AGPLv3/Commercial
  minimumCppStandard: 17

  dependencies:       juce_audio_processors, juce_audio_formats, juce_audio_devices
  OSXFrameworks:      CoreAudioKit DiscRecording
  iOSFrameworks:      CoreAudioKit

 END_JUCE_MODULE_DECLARATION

*******************************************************************************/


#pragma once
#define JUCE_AUDIO_UTILS_H_INCLUDED

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_processors/juce_audio_processors.h>

//==============================================================================
/** Config: JUCE_USE_CDREADER
    Enables the AudioCDReader class (on supported platforms).
*/
#ifndef JUCE_USE_CDREADER
#define JUCE_USE_CDREADER 0
#endif

/** Config: JUCE_USE_CDBURNER
    Enables the AudioCDBurner class (on supported platforms).
*/
#ifndef JUCE_USE_CDBURNER
#define JUCE_USE_CDBURNER 0
#endif

//==============================================================================
#include "gui/juce_AudioDeviceSelectorComponent.h"
#include "gui/juce_AudioThumbnailBase.h"
#include "gui/juce_AudioThumbnail.h"
#include "gui/juce_AudioThumbnailCache.h"
#include "gui/juce_AudioVisualiserComponent.h"
#include "gui/juce_KeyboardComponentBase.h"
#include "gui/juce_MidiKeyboardComponent.h"
#include "gui/juce_MPEKeyboardComponent.h"
#include "gui/juce_AudioAppComponent.h"
#include "gui/juce_BluetoothMidiDevicePairingDialogue.h"
#include "players/juce_SoundPlayer.h"
#include "players/juce_AudioProcessorPlayer.h"
#include "audio_cd/juce_AudioCDBurner.h"
#include "audio_cd/juce_AudioCDReader.h"
