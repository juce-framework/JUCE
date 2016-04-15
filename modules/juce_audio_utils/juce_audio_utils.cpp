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

#ifdef JUCE_AUDIO_UTILS_H_INCLUDED
 /* When you add this cpp file to your project, you mustn't include it in a file where you've
    already included any other headers - just put it inside a file on its own, possibly with your config
    flags preceding it, but don't include anything else. That also includes avoiding any automatic prefix
    header files that the compiler may be using.
 */
 #error "Incorrect use of JUCE cpp file"
#endif

#define JUCE_CORE_INCLUDE_JNI_HELPERS 1
#define JUCE_CORE_INCLUDE_NATIVE_HEADERS 1

#include "juce_audio_utils.h"

#if JUCE_MODULE_AVAILABLE_juce_gui_extra
 #include <juce_gui_extra/juce_gui_extra.h>
#endif

namespace juce
{

#include "gui/juce_AudioDeviceSelectorComponent.cpp"
#include "gui/juce_AudioThumbnail.cpp"
#include "gui/juce_AudioThumbnailCache.cpp"
#include "gui/juce_AudioVisualiserComponent.cpp"
#include "gui/juce_MidiKeyboardComponent.cpp"
#include "gui/juce_AudioAppComponent.cpp"
#include "players/juce_AudioProcessorPlayer.cpp"

#if JUCE_MAC
 #include "native/juce_mac_BluetoothMidiDevicePairingDialogue.mm"
#elif JUCE_IOS
 #include "native/juce_ios_BluetoothMidiDevicePairingDialogue.mm"
#elif JUCE_ANDROID
 #include "native/juce_android_BluetoothMidiDevicePairingDialogue.cpp"
#elif JUCE_LINUX
 #include "native/juce_linux_BluetoothMidiDevicePairingDialogue.cpp"
#elif JUCE_WINDOWS
 #include "native/juce_win_BluetoothMidiDevicePairingDialogue.cpp"
#endif

}
