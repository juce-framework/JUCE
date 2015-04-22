/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

#ifndef JUCE_AUDIO_DEVICES_H_INCLUDED
#define JUCE_AUDIO_DEVICES_H_INCLUDED

#include "../juce_events/juce_events.h"
#include "../juce_audio_basics/juce_audio_basics.h"
#include "../juce_audio_formats/juce_audio_formats.h"

//=============================================================================
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
    Enables WASAPI audio devices (Windows Vista and above). See also the
    JUCE_WASAPI_EXCLUSIVE flag.
*/
#ifndef JUCE_WASAPI
 #define JUCE_WASAPI 1
#endif

/** Config: JUCE_WASAPI_EXCLUSIVE
    Enables WASAPI audio devices in exclusive mode (Windows Vista and above).
*/
#ifndef JUCE_WASAPI_EXCLUSIVE
 #define JUCE_WASAPI_EXCLUSIVE 0
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
    Enables JACK audio devices (Linux only).
*/
#ifndef JUCE_JACK
 #define JUCE_JACK 0
#endif

/** Config: JUCE_USE_ANDROID_OPENSLES
    Enables OpenSLES devices (Android only).
*/
#ifndef JUCE_USE_ANDROID_OPENSLES
 #if JUCE_ANDROID_API_VERSION > 8
  #define JUCE_USE_ANDROID_OPENSLES 1
 #else
  #define JUCE_USE_ANDROID_OPENSLES 0
 #endif
#endif

//=============================================================================
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

//=============================================================================
namespace juce
{

#include "audio_io/juce_AudioIODevice.h"
#include "audio_io/juce_AudioIODeviceType.h"
#include "audio_io/juce_SystemAudioVolume.h"
#include "midi_io/juce_MidiInput.h"
#include "midi_io/juce_MidiMessageCollector.h"
#include "midi_io/juce_MidiOutput.h"
#include "sources/juce_AudioSourcePlayer.h"
#include "sources/juce_AudioTransportSource.h"
#include "audio_cd/juce_AudioCDBurner.h"
#include "audio_cd/juce_AudioCDReader.h"
#include "audio_io/juce_AudioDeviceManager.h"

}

#endif   // JUCE_AUDIO_DEVICES_H_INCLUDED
