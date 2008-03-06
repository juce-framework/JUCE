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

#ifndef __JUCE_AUDIOCDBURNER_JUCEHEADER__
#define __JUCE_AUDIOCDBURNER_JUCEHEADER__

#include "juce_AudioFormatReader.h"
#include "../audio_sources/juce_AudioSource.h"


//==============================================================================
/**
*/
class AudioCDBurner
{
public:
    //==============================================================================
    static const StringArray findAvailableDevices();
    static AudioCDBurner* openDevice (const int deviceIndex);

    ~AudioCDBurner();

    //==============================================================================
    bool isDiskPresent() const;

    int getNumAvailableAudioBlocks() const;

    bool addAudioTrack (AudioFormatReader& source, int numSamples);
    bool addAudioTrack (AudioSource& source, int numSamples);

    /**

        Return true to cancel the current burn operation
    */
    class BurnProgressListener
    {
    public:
        BurnProgressListener() throw() {}
        virtual ~BurnProgressListener() {}

        /** Called at intervals to report on the progress of the AudioCDBurner.

            To cancel the burn, return true from this.
        */
        virtual bool audioCDBurnProgress (float proportionComplete) = 0;
    };

    const String burn (BurnProgressListener* listener,
                       const bool ejectDiscAfterwards);

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    AudioCDBurner (const int deviceIndex);

    void* internal;
};


#endif   // __JUCE_AUDIOCDBURNER_JUCEHEADER__
