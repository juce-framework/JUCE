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

#ifndef __JUCE_MIXERAUDIOSOURCE_JUCEHEADER__
#define __JUCE_MIXERAUDIOSOURCE_JUCEHEADER__

#include "juce_AudioSource.h"
#include "../../threads/juce_CriticalSection.h"
#include "../../containers/juce_VoidArray.h"
#include "../../containers/juce_BitArray.h"


//==============================================================================
/**
    An AudioSource that mixes together the output of a set of other AudioSources.

    Input sources can be added and removed while the mixer is running as long as their
    prepareToPlay() and releaseResources() methods are called before and after adding
    them to the mixer.
*/
class JUCE_API  MixerAudioSource  : public AudioSource
{
public:
    //==============================================================================
    /** Creates a MixerAudioSource.
    */
    MixerAudioSource();

    /** Destructor. */
    ~MixerAudioSource();

    //==============================================================================
    /** Adds an input source to the mixer.

        If the mixer is running you'll need to make sure that the input source
        is ready to play by calling its prepareToPlay() method before adding it.
        If the mixer is stopped, then its input sources will be automatically
        prepared when the mixer's prepareToPlay() method is called.

        @param newInput             the source to add to the mixer
        @param deleteWhenRemoved    if true, then this source will be deleted when
                                    the mixer is deleted or when removeAllInputs() is
                                    called (unless the source is previously removed
                                    with the removeInputSource method)
    */
    void addInputSource (AudioSource* newInput,
                         const bool deleteWhenRemoved);

    /** Removes an input source.

        If the mixer is running, this will remove the source but not call its
        releaseResources() method, so the caller might want to do this manually.

        @param input            the source to remove
        @param deleteSource     whether to delete this source after it's been removed
    */
    void removeInputSource (AudioSource* input,
                            const bool deleteSource);

    /** Removes all the input sources.

        If the mixer is running, this will remove the sources but not call their
        releaseResources() method, so the caller might want to do this manually.

        Any sources which were added with the deleteWhenRemoved flag set will be
        deleted by this method.
    */
    void removeAllInputs();

    //==============================================================================
    /** Implementation of the AudioSource method.

        This will call prepareToPlay() on all its input sources.
    */
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate);

    /** Implementation of the AudioSource method.

        This will call releaseResources() on all its input sources.
    */
    void releaseResources();

    /** Implementation of the AudioSource method. */
    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill);


    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    //==============================================================================
    VoidArray inputs;
    BitArray inputsToDelete;
    CriticalSection lock;
    AudioSampleBuffer tempBuffer;
    double currentSampleRate;
    int bufferSizeExpected;

    MixerAudioSource (const MixerAudioSource&);
    const MixerAudioSource& operator= (const MixerAudioSource&);
};


#endif   // __JUCE_MIXERAUDIOSOURCE_JUCEHEADER__
