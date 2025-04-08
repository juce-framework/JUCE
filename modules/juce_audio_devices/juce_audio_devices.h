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

  ID:                 juce_audio_devices
  vendor:             juce
  version:            8.0.7
  name:               JUCE audio and MIDI I/O device classes
  description:        Classes to play and record from audio and MIDI I/O devices
  website:            http://www.juce.com/juce
  license:            AGPLv3/Commercial
  minimumCppStandard: 17

  dependencies:       juce_audio_basics, juce_events
  OSXFrameworks:      CoreAudio CoreMIDI AudioToolbox
  iOSFrameworks:      CoreAudio CoreMIDI AudioToolbox AVFoundation
  linuxPackages:      alsa

 END_JUCE_MODULE_DECLARATION

*******************************************************************************/


#pragma once
#define JUCE_AUDIO_DEVICES_H_INCLUDED

#include <juce_events/juce_events.h>
#include <juce_audio_basics/juce_audio_basics.h>

#if JUCE_MODULE_AVAILABLE_juce_graphics
#include <juce_graphics/juce_graphics.h>
#endif

//==============================================================================
/** Config: JUCE_USE_WINRT_MIDI
    Enables the use of the Windows Runtime API for MIDI, allowing connections
    to Bluetooth Low Energy devices on Windows 10 version 1809 (October 2018
    Update) and later. If you enable this flag then older versions of Windows
    will automatically fall back to using the regular Win32 MIDI API.

    You will need version 10.0.14393.0 of the Windows Standalone SDK to compile
    and you may need to add the path to the WinRT headers. The path to the
    headers will be something similar to
    "C:\Program Files (x86)\Windows Kits\10\Include\10.0.14393.0\winrt".
*/
#ifndef JUCE_USE_WINRT_MIDI
 #define JUCE_USE_WINRT_MIDI 0
#endif

/** Config: JUCE_ASIO
    Enables ASIO audio devices (MS Windows only).
    Turning this on means that you'll need to have the Steinberg ASIO SDK installed
    on your Windows build machine.

    See the comments in the ASIOAudioIODevice class's header file for more
    info about this.
*/
#ifndef JUCE_ASIO
 #define JUCE_ASIO 0
#endif

/** Config: JUCE_WASAPI
    Enables WASAPI audio devices (Windows Vista and above).
*/
#ifndef JUCE_WASAPI
 #define JUCE_WASAPI 1
#endif

/** Config: JUCE_DIRECTSOUND
    Enables DirectSound audio (MS Windows only).
*/
#ifndef JUCE_DIRECTSOUND
 #define JUCE_DIRECTSOUND 1
#endif

/** Config: JUCE_ALSA
    Enables ALSA audio devices (Linux only).
*/
#ifndef JUCE_ALSA
 #define JUCE_ALSA 1
#endif

/** Config: JUCE_JACK
    Enables JACK audio devices.
*/
#ifndef JUCE_JACK
 #define JUCE_JACK 0
#endif

/** Config: JUCE_BELA
    Enables Bela audio devices on Bela boards.
*/
#ifndef JUCE_BELA
 #define JUCE_BELA 0
#endif

/** Config: JUCE_USE_ANDROID_OBOE
    Enables Oboe devices (Android only).
*/
#ifndef JUCE_USE_ANDROID_OBOE
 #define JUCE_USE_ANDROID_OBOE 1
#endif

/** Config: JUCE_USE_OBOE_STABILIZED_CALLBACK
    If JUCE_USE_ANDROID_OBOE is enabled, enabling this will wrap output audio
    streams in the oboe::StabilizedCallback class. This class attempts to keep
    the CPU spinning to avoid it being scaled down on certain devices.
    (Android only).
*/
#ifndef JUCE_USE_ANDROID_OBOE_STABILIZED_CALLBACK
 #define JUCE_USE_ANDROID_OBOE_STABILIZED_CALLBACK 0
#endif

/** Config: JUCE_USE_ANDROID_OPENSLES
    Enables OpenSLES devices (Android only).
*/
#ifndef JUCE_USE_ANDROID_OPENSLES
 #if ! JUCE_USE_ANDROID_OBOE
  #define JUCE_USE_ANDROID_OPENSLES 1
 #else
  #define JUCE_USE_ANDROID_OPENSLES 0
 #endif
#endif

/** Config: JUCE_DISABLE_AUDIO_MIXING_WITH_OTHER_APPS
    Turning this on gives your app exclusive access to the system's audio
    on platforms which support it (currently iOS only).
*/
#ifndef JUCE_DISABLE_AUDIO_MIXING_WITH_OTHER_APPS
 #define JUCE_DISABLE_AUDIO_MIXING_WITH_OTHER_APPS 0
#endif

//==============================================================================
#include "midi_io/juce_MidiDevices.h"
#include "midi_io/juce_MidiMessageCollector.h"

namespace juce
{
    /** Available modes for the WASAPI audio device.

        Pass one of these to the AudioIODeviceType::createAudioIODeviceType_WASAPI()
        method to create a WASAPI AudioIODeviceType object in this mode.
    */
    enum class WASAPIDeviceMode
    {
        shared,
        exclusive,
        sharedLowLatency
    };
}

#include "audio_io/juce_AudioIODevice.h"
#include "audio_io/juce_AudioIODeviceType.h"
#include "audio_io/juce_SystemAudioVolume.h"
#include "sources/juce_AudioSourcePlayer.h"
#include "sources/juce_AudioTransportSource.h"
#include "audio_io/juce_AudioDeviceManager.h"

#if JUCE_IOS
 #include "native/juce_Audio_ios.h"
#endif
