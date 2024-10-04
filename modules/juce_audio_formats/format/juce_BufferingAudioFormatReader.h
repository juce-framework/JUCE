/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

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
    bool readSamples (int* const* destSamples, int numDestChannels, int startOffsetInDestBuffer,
                      int64 startSampleInFile, int numSamples) override;

private:
    struct BufferedBlock
    {
        BufferedBlock (AudioFormatReader& reader, int64 pos, int numSamples);

        Range<int64> range;
        AudioBuffer<float> buffer;
        bool allSamplesRead = false;
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
