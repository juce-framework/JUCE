/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2016 - ROLI Ltd.

   Permission is granted to use this software under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license/

   Permission to use, copy, modify, and/or distribute this software for any
   purpose with or without fee is hereby granted, provided that the above
   copyright notice and this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES WITH REGARD
   TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
   FITNESS. IN NO EVENT SHALL ISC BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT,
   OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
   USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
   TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
   OF THIS SOFTWARE.

   -----------------------------------------------------------------------------

   To release a closed-source product which uses other parts of JUCE not
   licensed under the ISC terms, commercial licenses are available: visit
   www.juce.com for more information.

  ==============================================================================
*/

#ifndef JUCE_IIRFILTERAUDIOSOURCE_H_INCLUDED
#define JUCE_IIRFILTERAUDIOSOURCE_H_INCLUDED


//==============================================================================
/**
    An AudioSource that performs an IIR filter on another source.
*/
class JUCE_API  IIRFilterAudioSource  : public AudioSource
{
public:
    //==============================================================================
    /** Creates a IIRFilterAudioSource for a given input source.

        @param inputSource              the input source to read from - this must not be null
        @param deleteInputWhenDeleted   if true, the input source will be deleted when
                                        this object is deleted
    */
    IIRFilterAudioSource (AudioSource* inputSource,
                          bool deleteInputWhenDeleted);

    /** Destructor. */
    ~IIRFilterAudioSource();

    //==============================================================================
    /** Changes the filter to use the same parameters as the one being passed in. */
    void setCoefficients (const IIRCoefficients& newCoefficients);

    /** Calls IIRFilter::makeInactive() on all the filters being used internally. */
    void makeInactive();

    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void releaseResources() override;
    void getNextAudioBlock (const AudioSourceChannelInfo&) override;

private:
    //==============================================================================
    OptionalScopedPointer<AudioSource> input;
    OwnedArray<IIRFilter> iirFilters;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (IIRFilterAudioSource)
};


#endif   // JUCE_IIRFILTERAUDIOSOURCE_H_INCLUDED
