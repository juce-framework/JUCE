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

#ifndef JUCE_BUFFERINGAUDIOSOURCE_H_INCLUDED
#define JUCE_BUFFERINGAUDIOSOURCE_H_INCLUDED


//==============================================================================
/**
    An AudioSource which takes another source as input, and buffers it using a thread.

    Create this as a wrapper around another thread, and it will read-ahead with
    a background thread to smooth out playback. You can either create one of these
    directly, or use it indirectly using an AudioTransportSource.

    @see PositionableAudioSource, AudioTransportSource
*/
class JUCE_API  BufferingAudioSource  : public PositionableAudioSource,
                                        private TimeSliceClient
{
public:
    //==============================================================================
    /** Creates a BufferingAudioSource.

        @param source                       the input source to read from
        @param backgroundThread             a background thread that will be used for the
                                            background read-ahead. This object must not be deleted
                                            until after any BufferingAudioSources that are using it
                                            have been deleted!
        @param deleteSourceWhenDeleted      if true, then the input source object will
                                            be deleted when this object is deleted
        @param numberOfSamplesToBuffer      the size of buffer to use for reading ahead
        @param numberOfChannels             the number of channels that will be played
        @param prefillBufferOnPrepareToPlay if true, then calling prepareToPlay on this object will
                                            block until the buffer has been filled
    */
    BufferingAudioSource (PositionableAudioSource* source,
                          TimeSliceThread& backgroundThread,
                          bool deleteSourceWhenDeleted,
                          int numberOfSamplesToBuffer,
                          int numberOfChannels = 2,
                          bool prefillBufferOnPrepareToPlay = true);

    /** Destructor.

        The input source may be deleted depending on whether the deleteSourceWhenDeleted
        flag was set in the constructor.
    */
    ~BufferingAudioSource();

    //==============================================================================
    /** Implementation of the AudioSource method. */
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;

    /** Implementation of the AudioSource method. */
    void releaseResources() override;

    /** Implementation of the AudioSource method. */
    void getNextAudioBlock (const AudioSourceChannelInfo&) override;

    //==============================================================================
    /** Implements the PositionableAudioSource method. */
    void setNextReadPosition (int64 newPosition) override;

    /** Implements the PositionableAudioSource method. */
    int64 getNextReadPosition() const override;

    /** Implements the PositionableAudioSource method. */
    int64 getTotalLength() const override       { return source->getTotalLength(); }

    /** Implements the PositionableAudioSource method. */
    bool isLooping() const override             { return source->isLooping(); }

    /** A useful function to block until the next the buffer info can be filled.

        This is useful for offline rendering.
    */
    bool waitForNextAudioBlockReady (const AudioSourceChannelInfo& info, const uint32 timeout);

private:
    //==============================================================================
    OptionalScopedPointer<PositionableAudioSource> source;
    TimeSliceThread& backgroundThread;
    int numberOfSamplesToBuffer, numberOfChannels;
    AudioSampleBuffer buffer;
    CriticalSection bufferStartPosLock;
    WaitableEvent bufferReadyEvent;
    int64 volatile bufferValidStart, bufferValidEnd, nextPlayPos;
    double volatile sampleRate;
    bool wasSourceLooping, isPrepared, prefillBuffer;

    bool readNextBufferChunk();
    void readBufferSection (int64 start, int length, int bufferOffset);
    int useTimeSlice() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BufferingAudioSource)
};


#endif   // JUCE_BUFFERINGAUDIOSOURCE_H_INCLUDED
