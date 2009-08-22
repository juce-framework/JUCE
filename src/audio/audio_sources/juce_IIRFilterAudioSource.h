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

#ifndef __JUCE_IIRFILTERAUDIOSOURCE_JUCEHEADER__
#define __JUCE_IIRFILTERAUDIOSOURCE_JUCEHEADER__

#include "juce_AudioSource.h"
#include "../dsp/juce_IIRFilter.h"
#include "../../containers/juce_OwnedArray.h"


//==============================================================================
/**
    An AudioSource that performs an IIR filter on another source.
*/
class JUCE_API  IIRFilterAudioSource  : public AudioSource
{
public:
    //==============================================================================
    /** Creates a IIRFilterAudioSource for a given input source.

        @param inputSource              the input source to read from
        @param deleteInputWhenDeleted   if true, the input source will be deleted when
                                        this object is deleted
    */
    IIRFilterAudioSource (AudioSource* const inputSource,
                          const bool deleteInputWhenDeleted);

    /** Destructor. */
    ~IIRFilterAudioSource();

    //==============================================================================
    /** Changes the filter to use the same parameters as the one being passed in.
    */
    void setFilterParameters (const IIRFilter& newSettings);

    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate);
    void releaseResources();
    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill);


    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    //==============================================================================
    AudioSource* const input;
    const bool deleteInputWhenDeleted;
    OwnedArray <IIRFilter> iirFilters;

    IIRFilterAudioSource (const IIRFilterAudioSource&);
    const IIRFilterAudioSource& operator= (const IIRFilterAudioSource&);
};


#endif   // __JUCE_IIRFILTERAUDIOSOURCE_JUCEHEADER__
