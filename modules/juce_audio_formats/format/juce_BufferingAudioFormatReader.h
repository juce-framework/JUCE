/*
  ==============================================================================

   This file is part of the JUCE 7 technical preview.
   Copyright (c) 2022 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For the technical preview this file cannot be licensed commercially.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

//==============================================================================
/**
    An AudioFormatReader that uses a background thread to pre-read data from
    another reader.

    @see AudioFormatReader

    @tags{Audio}
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

    ~BufferingAudioReader() override;

    /** Sets a number of milliseconds that the reader can block for in its readSamples()
        method before giving up and returning silence.

        A value of less that 0 means "wait forever". The default timeout is 0.
    */
    void setReadTimeout (int timeoutMilliseconds) noexcept;

    //==============================================================================
    bool readSamples (int** destSamples, int numDestChannels, int startOffsetInDestBuffer,
                      int64 startSampleInFile, int numSamples) override;

private:
    struct BufferedBlock
    {
        BufferedBlock (AudioFormatReader& reader, int64 pos, int numSamples);

        Range<int64> range;
        AudioBuffer<float> buffer;
    };

    int useTimeSlice() override;
    BufferedBlock* getBlockContaining (int64 pos) const noexcept;
    bool readNextBufferChunk();

    static constexpr int samplesPerBlock = 32768;

    std::unique_ptr<AudioFormatReader> source;
    TimeSliceThread& thread;
    std::atomic<int64> nextReadPosition { 0 };
    const int numBlocks;
    int timeoutMs = 0;

    CriticalSection lock;
    OwnedArray<BufferedBlock> blocks;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BufferingAudioReader)
};

} // namespace juce
