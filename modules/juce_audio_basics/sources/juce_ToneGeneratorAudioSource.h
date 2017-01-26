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

#ifndef JUCE_TONEGENERATORAUDIOSOURCE_H_INCLUDED
#define JUCE_TONEGENERATORAUDIOSOURCE_H_INCLUDED


//==============================================================================
/**
    A simple AudioSource that generates a sine wave.

*/
class JUCE_API  ToneGeneratorAudioSource  : public AudioSource
{
public:
    //==============================================================================
    /** Creates a ToneGeneratorAudioSource. */
    ToneGeneratorAudioSource();

    /** Destructor. */
    ~ToneGeneratorAudioSource();

    //==============================================================================
    /** Sets the signal's amplitude. */
    void setAmplitude (float newAmplitude);

    /** Sets the signal's frequency. */
    void setFrequency (double newFrequencyHz);


    //==============================================================================
    /** Implementation of the AudioSource method. */
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;

    /** Implementation of the AudioSource method. */
    void releaseResources() override;

    /** Implementation of the AudioSource method. */
    void getNextAudioBlock (const AudioSourceChannelInfo&) override;


private:
    //==============================================================================
    double frequency, sampleRate;
    double currentPhase, phasePerSample;
    float amplitude;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ToneGeneratorAudioSource)
};


#endif   // JUCE_TONEGENERATORAUDIOSOURCE_H_INCLUDED
