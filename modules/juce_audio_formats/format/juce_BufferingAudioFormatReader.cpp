/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

BufferingAudioReader::BufferingAudioReader (AudioFormatReader* sourceReader,
                                            TimeSliceThread& timeSliceThread,
                                            int samplesToBuffer)
    : AudioFormatReader (nullptr, sourceReader->getFormatName()),
      source (sourceReader), thread (timeSliceThread),
      numBlocks (1 + (samplesToBuffer / samplesPerBlock))
{
    sampleRate            = source->sampleRate;
    lengthInSamples       = source->lengthInSamples;
    numChannels           = source->numChannels;
    metadataValues        = source->metadataValues;
    bitsPerSample         = 32;
    usesFloatingPointData = true;

    timeSliceThread.addTimeSliceClient (this);
}

BufferingAudioReader::~BufferingAudioReader()
{
    thread.removeTimeSliceClient (this);
}

void BufferingAudioReader::setReadTimeout (int timeoutMilliseconds) noexcept
{
    timeoutMs = timeoutMilliseconds;
}

bool BufferingAudioReader::readSamples (int* const* destSamples, int numDestChannels, int startOffsetInDestBuffer,
                                        int64 startSampleInFile, int numSamples)
{
    auto startTime = Time::getMillisecondCounter();
    clearSamplesBeyondAvailableLength (destSamples, numDestChannels, startOffsetInDestBuffer,
                                       startSampleInFile, numSamples, lengthInSamples);

    const ScopedLock sl (lock);
    nextReadPosition = startSampleInFile;

    bool allSamplesRead = true;

    while (numSamples > 0)
    {
        if (auto block = getBlockContaining (startSampleInFile))
        {
            auto offset = (int) (startSampleInFile - block->range.getStart());
            auto numToDo = jmin (numSamples, (int) (block->range.getEnd() - startSampleInFile));

            for (int j = 0; j < numDestChannels; ++j)
            {
                if (auto* dest = (float*) destSamples[j])
                {
                    dest += startOffsetInDestBuffer;

                    if (j < (int) numChannels)
                        FloatVectorOperations::copy (dest, block->buffer.getReadPointer (j, offset), numToDo);
                    else
                        FloatVectorOperations::clear (dest, numToDo);
                }
            }

            startOffsetInDestBuffer += numToDo;
            startSampleInFile += numToDo;
            numSamples -= numToDo;

            allSamplesRead = allSamplesRead && block->allSamplesRead;
        }
        else
        {
            if (timeoutMs >= 0 && Time::getMillisecondCounter() >= startTime + (uint32) timeoutMs)
            {
                for (int j = 0; j < numDestChannels; ++j)
                    if (auto* dest = (float*) destSamples[j])
                        FloatVectorOperations::clear (dest + startOffsetInDestBuffer, numSamples);

                allSamplesRead = false;
                break;
            }
            else
            {
                ScopedUnlock ul (lock);
                Thread::yield();
            }
        }
    }

    return allSamplesRead;
}

BufferingAudioReader::BufferedBlock::BufferedBlock (AudioFormatReader& reader, int64 pos, int numSamples)
    : range (pos, pos + numSamples),
      buffer ((int) reader.numChannels, numSamples),
      allSamplesRead (reader.read (&buffer, 0, numSamples, pos, true, true))
{
}

BufferingAudioReader::BufferedBlock* BufferingAudioReader::getBlockContaining (int64 pos) const noexcept
{
    for (auto* b : blocks)
        if (b->range.contains (pos))
            return b;

    return nullptr;
}

int BufferingAudioReader::useTimeSlice()
{
    return readNextBufferChunk() ? 1 : 100;
}

bool BufferingAudioReader::readNextBufferChunk()
{
    auto pos = (nextReadPosition.load() / samplesPerBlock) * samplesPerBlock;
    auto endPos = jmin (lengthInSamples, pos + numBlocks * samplesPerBlock);

    OwnedArray<BufferedBlock> newBlocks;

    for (int i = blocks.size(); --i >= 0;)
        if (blocks.getUnchecked (i)->range.intersects (Range<int64> (pos, endPos)))
            newBlocks.add (blocks.getUnchecked (i));

    if (newBlocks.size() == numBlocks)
    {
        newBlocks.clear (false);
        return false;
    }

    for (auto p = pos; p < endPos; p += samplesPerBlock)
    {
        if (getBlockContaining (p) == nullptr)
        {
            newBlocks.add (new BufferedBlock (*source, p, samplesPerBlock));
            break; // just do one block
        }
    }

    {
        const ScopedLock sl (lock);
        newBlocks.swapWith (blocks);
    }

    for (int i = blocks.size(); --i >= 0;)
        newBlocks.removeObject (blocks.getUnchecked (i), false);

    return true;
}


//==============================================================================
//==============================================================================
#if JUCE_UNIT_TESTS

static bool isSilent (const AudioBuffer<float>& b)
{
    for (int channel = 0; channel < b.getNumChannels(); ++channel)
        if (b.findMinMax (channel, 0, b.getNumSamples()) != Range<float>{})
            return false;

    return true;
}

struct TestAudioFormatReader : public AudioFormatReader
{
    explicit TestAudioFormatReader (const AudioBuffer<float>* b)
        : AudioFormatReader (nullptr, {}),
          buffer (b)
    {
        jassert (buffer != nullptr);
        sampleRate            = 44100.0f;
        bitsPerSample         = 32;
        usesFloatingPointData = true;
        lengthInSamples       = buffer->getNumSamples();
        numChannels           = (unsigned int) buffer->getNumChannels();
    }

    bool readSamples (int* const* destChannels, int numDestChannels, int startOffsetInDestBuffer,
                      int64 startSampleInFile, int numSamples) override
    {
        clearSamplesBeyondAvailableLength (destChannels, numDestChannels, startOffsetInDestBuffer,
                                           startSampleInFile, numSamples, lengthInSamples);

        for (int j = 0; j < numDestChannels; ++j)
        {
            static_assert (sizeof (int) == sizeof (float),
                           "Int and float size must match in order for pointer arithmetic to work correctly");

            if (auto* dest = reinterpret_cast<float*> (destChannels[j]))
            {
                dest += startOffsetInDestBuffer;

                if (j < (int) numChannels)
                    FloatVectorOperations::copy (dest, buffer->getReadPointer (j, (int) startSampleInFile), numSamples);
                else
                    FloatVectorOperations::clear (dest, numSamples);
            }
        }

        return true;
    }

    const AudioBuffer<float>* buffer;
};

static AudioBuffer<float> generateTestBuffer (Random& random, int bufferSize)
{
    AudioBuffer<float> buffer { 2, bufferSize };

    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
            buffer.setSample (channel, sample, random.nextFloat());

    return buffer;
}

class BufferingAudioReaderTests final : public UnitTest
{
public:
    BufferingAudioReaderTests()  : UnitTest ("BufferingAudioReader", UnitTestCategories::audio)  {}

    void runTest() override
    {
        TimeSliceThread thread ("TestBackgroundThread");
        thread.startThread (Thread::Priority::normal);

        beginTest ("Reading samples from a blocked reader should produce silence");
        {
            struct BlockingReader final : public TestAudioFormatReader
            {
                explicit BlockingReader (const AudioBuffer<float>* b)
                    : TestAudioFormatReader (b)
                {
                }

                bool readSamples (int* const* destChannels,
                                  int numDestChannels,
                                  int startOffsetInDestBuffer,
                                  int64 startSampleInFile,
                                  int numSamples) override
                {
                    unblock.wait();
                    return TestAudioFormatReader::readSamples (destChannels, numDestChannels, startOffsetInDestBuffer, startSampleInFile, numSamples);
                }

                WaitableEvent unblock;
            };

            Random random { getRandom() };
            constexpr auto bufferSize = 1024;

            const auto source = generateTestBuffer (random, bufferSize);
            expect (! isSilent (source));

            auto* blockingReader = new BlockingReader (&source);
            BufferingAudioReader reader (blockingReader, thread, bufferSize);

            auto destination = generateTestBuffer (random, bufferSize);
            expect (! isSilent (destination));

            read (reader, destination);
            expect (isSilent (destination));

            blockingReader->unblock.signal();
        }

        beginTest ("Reading samples from a reader should produce the same samples as its source");
        {
            Random random { getRandom() };

            for (auto i = 4; i < 18; ++i)
            {
                const auto bufferSize = 1 << i;
                const auto source = generateTestBuffer (random, bufferSize);
                expect (! isSilent (source));

                BufferingAudioReader reader (new TestAudioFormatReader (&source), thread, bufferSize);
                reader.setReadTimeout (-1);

                auto destination = generateTestBuffer (random, bufferSize);
                expect (! isSilent (destination));
                expect (source != destination);

                read (reader, destination);
                expect (source == destination);
            }
        }
    }

private:
    void read (BufferingAudioReader& reader, AudioBuffer<float>& readBuffer)
    {
        constexpr int blockSize = 1024;

        const auto numSamples = readBuffer.getNumSamples();
        int readPos = 0;

        for (;;)
        {
            reader.read (&readBuffer, readPos, jmin (blockSize, numSamples - readPos), readPos, true, true);

            readPos += blockSize;

            if (readPos >= numSamples)
                break;
        }
    }
};

static BufferingAudioReaderTests bufferingAudioReaderTests;

#endif

} // namespace juce
