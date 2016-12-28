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

#ifndef JUCE_AUDIOSOURCEPLAYER_H_INCLUDED
#define JUCE_AUDIOSOURCEPLAYER_H_INCLUDED


//==============================================================================
/**
    Wrapper class to continuously stream audio from an audio source to an
    AudioIODevice.

    This object acts as an AudioIODeviceCallback, so can be attached to an
    output device, and will stream audio from an AudioSource.
*/
class JUCE_API  AudioSourcePlayer  : public AudioIODeviceCallback
{
public:
    //==============================================================================
    /** Creates an empty AudioSourcePlayer. */
    AudioSourcePlayer();

    /** Destructor.

        Make sure this object isn't still being used by an AudioIODevice before
        deleting it!
    */
    virtual ~AudioSourcePlayer();

    //==============================================================================
    /** Changes the current audio source to play from.

        If the source passed in is already being used, this method will do nothing.
        If the source is not null, its prepareToPlay() method will be called
        before it starts being used for playback.

        If there's another source currently playing, its releaseResources() method
        will be called after it has been swapped for the new one.

        @param newSource                the new source to use - this will NOT be deleted
                                        by this object when no longer needed, so it's the
                                        caller's responsibility to manage it.
    */
    void setSource (AudioSource* newSource);

    /** Returns the source that's playing.
        May return nullptr if there's no source.
    */
    AudioSource* getCurrentSource() const noexcept      { return source; }

    /** Sets a gain to apply to the audio data.
        @see getGain
    */
    void setGain (float newGain) noexcept;

    /** Returns the current gain.
        @see setGain
    */
    float getGain() const noexcept                      { return gain; }

    //==============================================================================
    /** Implementation of the AudioIODeviceCallback method. */
    void audioDeviceIOCallback (const float** inputChannelData,
                                int totalNumInputChannels,
                                float** outputChannelData,
                                int totalNumOutputChannels,
                                int numSamples) override;

    /** Implementation of the AudioIODeviceCallback method. */
    void audioDeviceAboutToStart (AudioIODevice* device) override;

    /** Implementation of the AudioIODeviceCallback method. */
    void audioDeviceStopped() override;

    /** An alternative method for initialising the source without an AudioIODevice. */
    void prepareToPlay (double sampleRate, int blockSize);

private:
    //==============================================================================
    CriticalSection readLock;
    AudioSource* source;
    double sampleRate;
    int bufferSize;
    float* channels [128];
    float* outputChans [128];
    const float* inputChans [128];
    AudioSampleBuffer tempBuffer;
    float lastGain, gain;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioSourcePlayer)
};


#endif   // JUCE_AUDIOSOURCEPLAYER_H_INCLUDED
