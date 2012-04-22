/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#ifndef __JUCE_AUDIO_DEVICES_JUCEHEADER__
#define __JUCE_AUDIO_DEVICES_JUCEHEADER__

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

// START_AUTOINCLUDE audio_io, midi_io, sources, audio_cd
#ifndef __JUCE_AUDIODEVICEMANAGER_JUCEHEADER__
 #include "audio_io/juce_AudioDeviceManager.h"
#endif
#ifndef __JUCE_AUDIOIODEVICE_JUCEHEADER__
 #include "audio_io/juce_AudioIODevice.h"
#endif
#ifndef __JUCE_AUDIOIODEVICETYPE_JUCEHEADER__
 #include "audio_io/juce_AudioIODeviceType.h"
#endif
#ifndef __JUCE_MIDIINPUT_JUCEHEADER__
 #include "midi_io/juce_MidiInput.h"
#endif
#ifndef __JUCE_MIDIMESSAGECOLLECTOR_JUCEHEADER__
 #include "midi_io/juce_MidiMessageCollector.h"
#endif
#ifndef __JUCE_MIDIOUTPUT_JUCEHEADER__
 #include "midi_io/juce_MidiOutput.h"
#endif
#ifndef __JUCE_AUDIOSOURCEPLAYER_JUCEHEADER__
 #include "sources/juce_AudioSourcePlayer.h"
#endif
#ifndef __JUCE_AUDIOTRANSPORTSOURCE_JUCEHEADER__
 #include "sources/juce_AudioTransportSource.h"
#endif
#ifndef __JUCE_AUDIOCDBURNER_JUCEHEADER__
 #include "audio_cd/juce_AudioCDBurner.h"
#endif
#ifndef __JUCE_AUDIOCDREADER_JUCEHEADER__
 #include "audio_cd/juce_AudioCDReader.h"
#endif
// END_AUTOINCLUDE

}

#endif   // __JUCE_AUDIO_DEVICES_JUCEHEADER__
