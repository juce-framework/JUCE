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

#ifndef __JUCE_REVERBAUDIOSOURCE_JUCEHEADER__
#define __JUCE_REVERBAUDIOSOURCE_JUCEHEADER__

#include "juce_AudioSource.h"
#include "../effects/juce_Reverb.h"


//==============================================================================
/**
    An AudioSource that uses the Reverb class to apply a reverb to another AudioSource.

    @see Reverb
*/
class JUCE_API  ReverbAudioSource   : public AudioSource
{
public:
    /** Creates a ReverbAudioSource to process a given input source.

        @param inputSource              the input source to read from - this must not be null
        @param deleteInputWhenDeleted   if true, the input source will be deleted when
                                        this object is deleted
    */
    ReverbAudioSource (AudioSource* inputSource,
                       bool deleteInputWhenDeleted);

    /** Destructor. */
    ~ReverbAudioSource();

    //==============================================================================
    /** Returns the parameters from the reverb. */
    const Reverb::Parameters& getParameters() const noexcept    { return reverb.getParameters(); }

    /** Changes the reverb's parameters. */
    void setParameters (const Reverb::Parameters& newParams);

    void setBypassed (bool isBypassed) noexcept;
    bool isBypassed() const noexcept                            { return bypass; }

    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate);
    void releaseResources();
    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill);

private:
    //==============================================================================
    CriticalSection lock;
    OptionalScopedPointer<AudioSource> input;
    Reverb reverb;
    volatile bool bypass;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ReverbAudioSource)
};


#endif   // __JUCE_REVERBAUDIOSOURCE_JUCEHEADER__
