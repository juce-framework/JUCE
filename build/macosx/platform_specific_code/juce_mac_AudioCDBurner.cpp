/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#include "../../../src/juce_core/basics/juce_StandardHeader.h"
#include <Carbon/Carbon.h>

BEGIN_JUCE_NAMESPACE

#include "../../../src/juce_appframework/audio/audio_file_formats/juce_AudioCDBurner.h"


//==============================================================================
AudioCDBurner::AudioCDBurner (const int deviceIndex)
    : internal (0)
{
}

AudioCDBurner::~AudioCDBurner()
{
}

AudioCDBurner* AudioCDBurner::openDevice (const int deviceIndex)
{
    return 0;
}

const StringArray AudioCDBurner::findAvailableDevices()
{
    StringArray s;

    return s;
}

bool AudioCDBurner::isDiskPresent() const
{
    return false;
}

int AudioCDBurner::getNumAvailableAudioBlocks() const
{
    return 0;
}

bool AudioCDBurner::addAudioTrack (AudioFormatReader& source, int numSamples)
{
    return false;
}

bool AudioCDBurner::addAudioTrack (AudioSource& source, int numSamples)
{
    return false;
}

const String AudioCDBurner::burn (BurnProgressListener* listener,
                                  const bool ejectDiscAfterwards)
{
    return String::empty;
}

END_JUCE_NAMESPACE