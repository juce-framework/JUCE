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

#ifndef __JUCE_AUDIO_UTILS_JUCEHEADER__
#define __JUCE_AUDIO_UTILS_JUCEHEADER__

#include "../juce_gui_basics/juce_gui_basics.h"
#include "../juce_audio_devices/juce_audio_devices.h"
#include "../juce_audio_formats/juce_audio_formats.h"
#include "../juce_audio_processors/juce_audio_processors.h"

//=============================================================================
namespace juce
{

#ifndef __JUCE_AUDIODEVICESELECTORCOMPONENT_JUCEHEADER__
 #include "gui/juce_AudioDeviceSelectorComponent.h"
#endif
#ifndef __JUCE_AUDIOTHUMBNAILBASE_JUCEHEADER__
 #include "gui/juce_AudioThumbnailBase.h"
#endif
#ifndef __JUCE_AUDIOTHUMBNAIL_JUCEHEADER__
 #include "gui/juce_AudioThumbnail.h"
#endif
#ifndef __JUCE_AUDIOTHUMBNAILCACHE_JUCEHEADER__
 #include "gui/juce_AudioThumbnailCache.h"
#endif
#ifndef __JUCE_MIDIKEYBOARDCOMPONENT_JUCEHEADER__
 #include "gui/juce_MidiKeyboardComponent.h"
#endif
#ifndef __JUCE_AUDIOPROCESSORPLAYER_JUCEHEADER__
 #include "players/juce_AudioProcessorPlayer.h"
#endif

}

#endif   // __JUCE_AUDIO_UTILS_JUCEHEADER__
