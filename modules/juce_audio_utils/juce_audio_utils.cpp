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

#if JUCE_MODULE_AVAILABLE_juce_gui_extra
 #include <juce_gui_extra/juce_gui_extra.h>
#endif

#if JUCE_MAC
  #import <DiscRecording/DiscRecording.h>
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
#include "gui/juce_MidiKeyboardComponent.cpp"
#include "gui/juce_AudioAppComponent.cpp"
#include "players/juce_SoundPlayer.cpp"
#include "players/juce_AudioProcessorPlayer.cpp"
#include "audio_cd/juce_AudioCDReader.cpp"

#if JUCE_MAC
 #include "native/juce_mac_BluetoothMidiDevicePairingDialogue.mm"

 #if JUCE_USE_CDREADER
  #include "native/juce_mac_AudioCDReader.mm"
 #endif

 #if JUCE_USE_CDBURNER
  #include "native/juce_mac_AudioCDBurner.mm"
 #endif

#elif JUCE_IOS
 #include "native/juce_ios_BluetoothMidiDevicePairingDialogue.mm"

#elif JUCE_ANDROID
 #include "native/juce_android_BluetoothMidiDevicePairingDialogue.cpp"

#elif JUCE_LINUX
 #if JUCE_USE_CDREADER
  #include "native/juce_linux_AudioCDReader.cpp"
 #endif

 #include "native/juce_linux_BluetoothMidiDevicePairingDialogue.cpp"

#elif JUCE_WINDOWS
 #include "native/juce_win_BluetoothMidiDevicePairingDialogue.cpp"

 #if JUCE_USE_CDREADER
  #include "native/juce_win32_AudioCDReader.cpp"
 #endif

 #if JUCE_USE_CDBURNER
  #include "native/juce_win32_AudioCDBurner.cpp"
 #endif

#endif
