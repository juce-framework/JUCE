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

#ifndef JUCE_AUDIO_UTILS_H_INCLUDED
#define JUCE_AUDIO_UTILS_H_INCLUDED

#include "../juce_gui_basics/juce_gui_basics.h"
#include "../juce_audio_devices/juce_audio_devices.h"
#include "../juce_audio_formats/juce_audio_formats.h"
#include "../juce_audio_processors/juce_audio_processors.h"

//=============================================================================
namespace juce
{

#include "gui/juce_AudioDeviceSelectorComponent.h"
#include "gui/juce_AudioThumbnailBase.h"
#include "gui/juce_AudioThumbnail.h"
#include "gui/juce_AudioThumbnailCache.h"
#include "gui/juce_AudioVisualiserComponent.h"
#include "gui/juce_MidiKeyboardComponent.h"
#include "gui/juce_AudioAppComponent.h"
#include "players/juce_AudioProcessorPlayer.h"

}

#endif   // JUCE_AUDIO_UTILS_H_INCLUDED
