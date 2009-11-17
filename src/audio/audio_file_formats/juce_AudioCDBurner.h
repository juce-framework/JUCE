/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

#ifndef __JUCE_AUDIOCDBURNER_JUCEHEADER__
#define __JUCE_AUDIOCDBURNER_JUCEHEADER__

#if JUCE_USE_CDBURNER

#include "juce_AudioFormatReader.h"
#include "../audio_sources/juce_AudioSource.h"


//==============================================================================
/**
*/
class AudioCDBurner
{
public:
    //==============================================================================
    /** Returns a list of available optical drives.

        Use openDevice() to open one of the items from this list.
    */
    static const StringArray findAvailableDevices();

    /** Tries to open one of the optical drives.

        The deviceIndex is an index into the array returned by findAvailableDevices().
    */
    static AudioCDBurner* openDevice (const int deviceIndex);

    /** Destructor. */
    ~AudioCDBurner();

    //==============================================================================
    /** Returns true if there's a writable disk in the drive.
    */
    bool isDiskPresent() const;

    /** Returns the number of free blocks on the disk.

        There are 75 blocks per second, at 44100Hz.
    */
    int getNumAvailableAudioBlocks() const;

    /** Adds a track to be written.

        The source passed-in here will be kept by this object, and it will
        be used and deleted at some point in the future, either during the
        burn() method or when this AudioCDBurner object is deleted. Your caller
        method shouldn't keep a reference to it or use it again after passing
        it in here.
    */
    bool addAudioTrack (AudioSource* source, int numSamples);

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
                       const bool ejectDiscAfterwards,
                       const bool peformFakeBurnForTesting);

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    AudioCDBurner (const int deviceIndex);

    void* internal;
};


#endif
#endif   // __JUCE_AUDIOCDBURNER_JUCEHEADER__
