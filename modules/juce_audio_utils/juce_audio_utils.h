/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
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

  ID:                 juce_audio_utils
  vendor:             juce
  version:            6.0.7
  name:               JUCE extra audio utility classes
  description:        Classes for audio-related GUI and miscellaneous tasks.
  website:            http://www.juce.com/juce
  license:            GPL/Commercial

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
#include "gui/juce_MidiKeyboardComponent.h"
#include "gui/juce_AudioAppComponent.h"
#include "gui/juce_BluetoothMidiDevicePairingDialogue.h"
#include "players/juce_SoundPlayer.h"
#include "players/juce_AudioProcessorPlayer.h"
#include "audio_cd/juce_AudioCDBurner.h"
#include "audio_cd/juce_AudioCDReader.h"
