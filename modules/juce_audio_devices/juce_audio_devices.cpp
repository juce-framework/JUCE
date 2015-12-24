/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

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

#ifdef JUCE_AUDIO_DEVICES_H_INCLUDED
 /* When you add this cpp file to your project, you mustn't include it in a file where you've
    already included any other headers - just put it inside a file on its own, possibly with your config
    flags preceding it, but don't include anything else. That also includes avoiding any automatic prefix
    header files that the compiler may be using.
 */
 #error "Incorrect use of JUCE cpp file"
#endif

#include "../juce_core/native/juce_BasicNativeHeaders.h"
#include "juce_audio_devices.h"

//==============================================================================
#if JUCE_MAC
 #define Point CarbonDummyPointName
 #define Component CarbonDummyCompName
 #import <CoreAudio/AudioHardware.h>
 #import <CoreMIDI/MIDIServices.h>
 #import <DiscRecording/DiscRecording.h>
 #import <AudioToolbox/AudioServices.h>
 #undef Point
 #undef Component

#elif JUCE_IOS
 #import <AudioToolbox/AudioToolbox.h>
 #import <AVFoundation/AVFoundation.h>
 #import <CoreMIDI/MIDIServices.h>

//==============================================================================
#elif JUCE_WINDOWS
 #if JUCE_WASAPI
  #include <MMReg.h>
 #endif

 #if JUCE_ASIO
  /* This is very frustrating - we only need to use a handful of definitions from
     a couple of the header files in Steinberg's ASIO SDK, and it'd be easy to copy
     about 30 lines of code into this cpp file to create a fully stand-alone ASIO
     implementation...

     ..unfortunately that would break Steinberg's license agreement for use of
     their SDK, so I'm not allowed to do this.

     This means that anyone who wants to use JUCE's ASIO abilities will have to:

     1) Agree to Steinberg's licensing terms and download the ASIO SDK
         (see http://www.steinberg.net/en/company/developers.html).

     2) Enable this code with a global definition #define JUCE_ASIO 1.

     3) Make sure that your header search path contains the iasiodrv.h file that
        comes with the SDK. (Only about a handful of the SDK header files are actually
        needed - so to simplify things, you could just copy these into your JUCE directory).
  */
  #include <iasiodrv.h>
 #endif

 #if JUCE_USE_CDBURNER
  /* You'll need the Platform SDK for these headers - if you don't have it and don't
     need to use CD-burning, then you might just want to set the JUCE_USE_CDBURNER flag
     to 0, to avoid these includes.
  */
  #include <imapi.h>
  #include <imapierror.h>
 #endif

//==============================================================================
#elif JUCE_LINUX
 #if JUCE_ALSA
  /* Got an include error here? If so, you've either not got ALSA installed, or you've
     not got your paths set up correctly to find its header files.

     The package you need to install to get ASLA support is "libasound2-dev".

     If you don't have the ALSA library and don't want to build Juce with audio support,
     just set the JUCE_ALSA flag to 0.
  */
  #include <alsa/asoundlib.h>
 #endif

 #if JUCE_JACK
  /* Got an include error here? If so, you've either not got jack-audio-connection-kit
     installed, or you've not got your paths set up correctly to find its header files.

     The package you need to install to get JACK support is "libjack-dev".

     If you don't have the jack-audio-connection-kit library and don't want to build
     Juce with low latency audio support, just set the JUCE_JACK flag to 0.
  */
  #include <jack/jack.h>
 #endif
 #undef SIZEOF

//==============================================================================
#elif JUCE_ANDROID

 #if JUCE_USE_ANDROID_OPENSLES
  #include <SLES/OpenSLES.h>
  #include <SLES/OpenSLES_Android.h>
  #include <SLES/OpenSLES_AndroidConfiguration.h>
 #endif

#endif

namespace juce
{

#include "audio_io/juce_AudioDeviceManager.cpp"
#include "audio_io/juce_AudioIODevice.cpp"
#include "audio_io/juce_AudioIODeviceType.cpp"
#include "midi_io/juce_MidiMessageCollector.cpp"
#include "midi_io/juce_MidiOutput.cpp"
#include "audio_cd/juce_AudioCDReader.cpp"
#include "sources/juce_AudioSourcePlayer.cpp"
#include "sources/juce_AudioTransportSource.cpp"
#include "native/juce_MidiDataConcatenator.h"

//==============================================================================
#if JUCE_MAC
 #include "../juce_core/native/juce_osx_ObjCHelpers.h"
 #include "native/juce_mac_CoreAudio.cpp"
 #include "native/juce_mac_CoreMidi.cpp"

 #if JUCE_USE_CDREADER
  #include "native/juce_mac_AudioCDReader.mm"
 #endif

 #if JUCE_USE_CDBURNER
  #include "native/juce_mac_AudioCDBurner.mm"
 #endif

//==============================================================================
#elif JUCE_IOS
 #include "native/juce_ios_Audio.cpp"
 #include "native/juce_mac_CoreMidi.cpp"

//==============================================================================
#elif JUCE_WINDOWS
 #include "../juce_core/native/juce_win32_ComSmartPtr.h"
 #include "../juce_events/native/juce_win32_HiddenMessageWindow.h"

 #if JUCE_WASAPI
  #include "native/juce_win32_WASAPI.cpp"
 #endif

 #if JUCE_DIRECTSOUND
  #include "native/juce_win32_DirectSound.cpp"
 #endif

 #include "native/juce_win32_Midi.cpp"

 #if JUCE_ASIO
  #include "native/juce_win32_ASIO.cpp"
 #endif

 #if JUCE_USE_CDREADER
  #include "native/juce_win32_AudioCDReader.cpp"
 #endif

 #if JUCE_USE_CDBURNER
  #include "native/juce_win32_AudioCDBurner.cpp"
 #endif

//==============================================================================
#elif JUCE_LINUX
 #if JUCE_ALSA
  #include "native/juce_linux_ALSA.cpp"
 #endif

 #include "native/juce_linux_Midi.cpp"

 #if JUCE_JACK
  #include "native/juce_linux_JackAudio.cpp"
 #endif

 #if JUCE_USE_CDREADER
  #include "native/juce_linux_AudioCDReader.cpp"
 #endif

//==============================================================================
#elif JUCE_ANDROID
 #include "../juce_core/native/juce_android_JNIHelpers.h"
 #include "native/juce_android_Audio.cpp"
 #include "native/juce_android_Midi.cpp"

 #if JUCE_USE_ANDROID_OPENSLES
  #include "native/juce_android_OpenSL.cpp"
 #endif

#endif

#if ! JUCE_SYSTEMAUDIOVOL_IMPLEMENTED
 // None of these methods are available. (On Windows you might need to enable WASAPI for this)
 float JUCE_CALLTYPE SystemAudioVolume::getGain()         { jassertfalse; return 0.0f; }
 bool  JUCE_CALLTYPE SystemAudioVolume::setGain (float)   { jassertfalse; return false; }
 bool  JUCE_CALLTYPE SystemAudioVolume::isMuted()         { jassertfalse; return false; }
 bool  JUCE_CALLTYPE SystemAudioVolume::setMuted (bool)   { jassertfalse; return false; }
#endif
}
