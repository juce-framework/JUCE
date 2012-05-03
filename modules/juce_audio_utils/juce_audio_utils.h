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
