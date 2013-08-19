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

#ifndef JUCE_BUFFERINGAUDIOFORMATREADER_H_INCLUDED
#define JUCE_BUFFERINGAUDIOFORMATREADER_H_INCLUDED

//==============================================================================
/**
    An AudioFormatReader that uses a background thread to pre-read data from
    another reader.

    @see AudioFormatReader
*/
class JUCE_API  BufferingAudioReader  : public AudioFormatReader,
                                        private TimeSliceClient
{
public:
    /** Creates a reader.

        @param sourceReader     the source reader to wrap. This BufferingAudioReader
                                takes ownership of this object and will delete it later
                                when no longer needed
        @param timeSliceThread  the thread that should be used to do the background reading.
                                Make sure that the thread you supply is running, and won't
                                be deleted while the reader object still exists.
        @param samplesToBuffer  the total number of samples to buffer ahead.
    */
    BufferingAudioReader (AudioFormatReader* sourceReader,
                          TimeSliceThread& timeSliceThread,
                          int samplesToBuffer);

    ~BufferingAudioReader();

    /** Sets a number of milliseconds that the reader can block for in its readSamples()
        method before giving up and returning silence.
        A value of less that 0 means "wait forever".
        The default timeout is 0.
    */
    void setReadTimeout (int timeoutMilliseconds) noexcept;

    bool readSamples (int** destSamples, int numDestChannels, int startOffsetInDestBuffer,
                      int64 startSampleInFile, int numSamples) override;

private:
    ScopedPointer<AudioFormatReader> source;
    TimeSliceThread& thread;
    int64 nextReadPosition;
    const int numBlocks;
    int timeoutMs;

    enum { samplesPerBlock = 32768 };

    struct BufferedBlock
    {
        BufferedBlock (AudioFormatReader& reader, int64 pos, int numSamples);

        Range<int64> range;
        AudioSampleBuffer buffer;
    };

    CriticalSection lock;
    OwnedArray<BufferedBlock> blocks;

    BufferedBlock* getBlockContaining (int64 pos) const noexcept;
    int useTimeSlice() override;
    bool readNextBufferChunk();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BufferingAudioReader)
};


#endif   // JUCE_BUFFERINGAUDIOFORMATREADER_H_INCLUDED
