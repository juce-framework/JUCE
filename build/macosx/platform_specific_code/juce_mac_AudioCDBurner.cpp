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

/*#include "../../../src/juce_core/basics/juce_StandardHeader.h"
#include <Carbon/Carbon.h>

extern "C"
{
    int juce_findDiskBurnerDevices (const char** devices);
    void* juce_openDiskBurnerDevice (int deviceIndex);
    void juce_deleteDiskBurnerDevice (void* diskBurnerDevice);
    bool juce_isDiskPresentInDevice (void* diskBurnerDevice);
    int juce_getNumAvailableAudioBlocks (void* diskBurnerDevice);
    typedef bool (*diskBurnCallback) (void* userRef, float progress);
    void juce_performBurn (void* diskBurnerDevice, diskBurnCallback callback);
}

BEGIN_JUCE_NAMESPACE

#include "../../../src/juce_appframework/audio/audio_file_formats/juce_AudioCDBurner.h"

//==============================================================================
AudioCDBurner::AudioCDBurner (const int deviceIndex)
    : internal (0)
{
    internal = juce_openDiskBurnerDevice (deviceIndex);
}

AudioCDBurner::~AudioCDBurner()
{
    if (internal != 0)
        juce_deleteDiskBurnerDevice (internal);
}

AudioCDBurner* AudioCDBurner::openDevice (const int deviceIndex)
{
    AudioCDBurner* b = new AudioCDBurner (deviceIndex);

    if (b->internal == 0)
        deleteAndZero (b);

    return b;
}

const StringArray AudioCDBurner::findAvailableDevices()
{
    int num = juce_findDiskBurnerDevices (0);
    char** names = (char**) juce_calloc (sizeof (char*) * num);
    for (int i = num; --i >= 0;)
        names[i] = (char*) juce_calloc (2048);

    juce_findDiskBurnerDevices ((const char**) names);

    StringArray s;

    for (int i = num; --i >= 0;)
    {
        s.add (String::fromUTF8 ((juce::uint8*) names[i]));
        juce_free (names[i]);
    }

    return s;
}

bool AudioCDBurner::isDiskPresent() const
{
    return juce_isDiskPresentInDevice (internal);
}

int AudioCDBurner::getNumAvailableAudioBlocks() const
{
    return juce_getNumAvailableAudioBlocks (internal);
}

bool AudioCDBurner::addAudioTrack (AudioFormatReader& source, int numSamples)
{
    juce_BeginAudioTrack (internal);
    juce_AddSamplesToAudioTrack (internal, samples, num);
    juce_EndAudioTrack (internal);
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

END_JUCE_NAMESPACE*/