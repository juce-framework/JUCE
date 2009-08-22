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

#ifndef __JUCE_RESAMPLINGAUDIOSOURCE_JUCEHEADER__
#define __JUCE_RESAMPLINGAUDIOSOURCE_JUCEHEADER__

#include "juce_AudioSource.h"
#include "../../threads/juce_CriticalSection.h"

//==============================================================================
/**
    A type of AudioSource that takes an input source and changes its sample rate.

    @see AudioSource
*/
class JUCE_API  ResamplingAudioSource  : public AudioSource
{
public:
    //==============================================================================
    /** Creates a ResamplingAudioSource for a given input source.

        @param inputSource              the input source to read from
        @param deleteInputWhenDeleted   if true, the input source will be deleted when
                                        this object is deleted
    */
    ResamplingAudioSource (AudioSource* const inputSource,
                           const bool deleteInputWhenDeleted);

    /** Destructor. */
    ~ResamplingAudioSource();

    /** Changes the resampling ratio.

        (This value can be changed at any time, even while the source is running).

        @param samplesInPerOutputSample     if set to 1.0, the input is passed through; higher
                                            values will speed it up; lower values will slow it
                                            down. The ratio must be greater than 0
    */
    void setResamplingRatio (const double samplesInPerOutputSample);

    /** Returns the current resampling ratio.

        This is the value that was set by setResamplingRatio().
    */
    double getResamplingRatio() const throw()                   { return ratio; }

    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate);
    void releaseResources();
    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill);

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    AudioSource* const input;
    const bool deleteInputWhenDeleted;
    double ratio, lastRatio;
    AudioSampleBuffer buffer;
    int bufferPos, sampsInBuffer;
    double subSampleOffset;
    double coefficients[6];
    CriticalSection ratioLock;

    void setFilterCoefficients (double c1, double c2, double c3, double c4, double c5, double c6);
    void createLowPass (const double proportionalRate);

    struct FilterState
    {
        double x1, x2, y1, y2;
    };

    FilterState filterStates[2];
    void resetFilters();

    void applyFilter (float* samples, int num, FilterState& fs);

    ResamplingAudioSource (const ResamplingAudioSource&);
    const ResamplingAudioSource& operator= (const ResamplingAudioSource&);
};


#endif   // __JUCE_RESAMPLINGAUDIOSOURCE_JUCEHEADER__
