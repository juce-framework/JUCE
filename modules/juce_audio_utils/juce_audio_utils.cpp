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

#ifdef JUCE_AUDIO_UTILS_H_INCLUDED
 /* When you add this cpp file to your project, you mustn't include it in a file where you've
    already included any other headers - just put it inside a file on its own, possibly with your config
    flags preceding it, but don't include anything else. That also includes avoiding any automatic prefix
    header files that the compiler may be using.
 */
 #error "Incorrect use of JUCE cpp file"
#endif

#define JUCE_CORE_INCLUDE_NATIVE_HEADERS 1
#define JUCE_CORE_INCLUDE_JNI_HELPERS 1
#define JUCE_CORE_INCLUDE_OBJC_HELPERS 1
#define JUCE_CORE_INCLUDE_COM_SMART_PTR 1

#include "juce_audio_utils.h"
#include <juce_gui_extra/juce_gui_extra.h>

#if JUCE_MAC
 #import <DiscRecording/DiscRecording.h>
 #import <CoreAudioKit/CABTLEMIDIWindowController.h>
#elif JUCE_IOS
 #import <CoreAudioKit/CoreAudioKit.h>
#elif JUCE_WINDOWS
 #if JUCE_USE_CDBURNER
  /* You'll need the Platform SDK for these headers - if you don't have it and don't
     need to use CD-burning, then you might just want to set the JUCE_USE_CDBURNER flag
     to 0, to avoid these includes.
  */
  #include <imapi.h>
  #include <imapierror.h>
 #endif
#endif

#include "gui/juce_AudioDeviceSelectorComponent.cpp"
#include "gui/juce_AudioThumbnail.cpp"
#include "gui/juce_AudioThumbnailCache.cpp"
#include "gui/juce_AudioVisualiserComponent.cpp"
#include "gui/juce_KeyboardComponentBase.cpp"
#include "gui/juce_MidiKeyboardComponent.cpp"
#include "gui/juce_MPEKeyboardComponent.cpp"
#include "gui/juce_AudioAppComponent.cpp"
#include "players/juce_SoundPlayer.cpp"
#include "players/juce_AudioProcessorPlayer.cpp"
#include "audio_cd/juce_AudioCDReader.cpp"

#if JUCE_MAC
 #include "native/juce_BluetoothMidiDevicePairingDialogue_mac.mm"

 #if JUCE_USE_CDREADER
  #include "native/juce_AudioCDReader_mac.mm"
 #endif

 #if JUCE_USE_CDBURNER
  #include "native/juce_AudioCDBurner_mac.mm"
 #endif

#elif JUCE_IOS
 #include "native/juce_BluetoothMidiDevicePairingDialogue_ios.mm"

#elif JUCE_ANDROID
 #include "native/juce_BluetoothMidiDevicePairingDialogue_android.cpp"

#elif JUCE_LINUX || JUCE_BSD
 #if JUCE_USE_CDREADER
  #include "native/juce_AudioCDReader_linux.cpp"
 #endif

 #include "native/juce_BluetoothMidiDevicePairingDialogue_linux.cpp"

#elif JUCE_WINDOWS
 #include "native/juce_BluetoothMidiDevicePairingDialogue_windows.cpp"

 #if JUCE_USE_CDREADER
  #include "native/juce_AudioCDReader_windows.cpp"
 #endif

 #if JUCE_USE_CDBURNER
  #include "native/juce_AudioCDBurner_windows.cpp"
 #endif

#endif
