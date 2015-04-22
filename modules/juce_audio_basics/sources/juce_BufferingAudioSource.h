/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

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

        @param source                   the input source to read from
        @param backgroundThread         a background thread that will be used for the
                                        background read-ahead. This object must not be deleted
                                        until after any BufferedAudioSources that are using it
                                        have been deleted!
        @param deleteSourceWhenDeleted  if true, then the input source object will
                                        be deleted when this object is deleted
        @param numberOfSamplesToBuffer  the size of buffer to use for reading ahead
        @param numberOfChannels         the number of channels that will be played
    */
    BufferingAudioSource (PositionableAudioSource* source,
                          TimeSliceThread& backgroundThread,
                          bool deleteSourceWhenDeleted,
                          int numberOfSamplesToBuffer,
                          int numberOfChannels = 2);

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

private:
    //==============================================================================
    OptionalScopedPointer<PositionableAudioSource> source;
    TimeSliceThread& backgroundThread;
    int numberOfSamplesToBuffer, numberOfChannels;
    AudioSampleBuffer buffer;
    CriticalSection bufferStartPosLock;
    int64 volatile bufferValidStart, bufferValidEnd, nextPlayPos;
    double volatile sampleRate;
    bool wasSourceLooping, isPrepared;

    bool readNextBufferChunk();
    void readBufferSection (int64 start, int length, int bufferOffset);
    int useTimeSlice() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BufferingAudioSource)
};


#endif   // JUCE_BUFFERINGAUDIOSOURCE_H_INCLUDED
