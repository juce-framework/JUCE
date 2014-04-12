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

BufferingAudioReader::BufferingAudioReader (AudioFormatReader* sourceReader,
                                            TimeSliceThread& timeSliceThread,
                                            int samplesToBuffer)
    : AudioFormatReader (nullptr, sourceReader->getFormatName()),
      source (sourceReader), thread (timeSliceThread),
      nextReadPosition (0),
      numBlocks (1 + (samplesToBuffer / samplesPerBlock))
{
    sampleRate            = source->sampleRate;
    lengthInSamples       = source->lengthInSamples;
    numChannels           = source->numChannels;
    metadataValues        = source->metadataValues;
    bitsPerSample         = 32;
    usesFloatingPointData = true;

    for (int i = 3; --i >= 0;)
        readNextBufferChunk();

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

bool BufferingAudioReader::readSamples (int** destSamples, int numDestChannels, int startOffsetInDestBuffer,
                                        int64 startSampleInFile, int numSamples)
{
    const uint32 startTime = Time::getMillisecondCounter();
    clearSamplesBeyondAvailableLength (destSamples, numDestChannels, startOffsetInDestBuffer,
                                       startSampleInFile, numSamples, lengthInSamples);

    const ScopedLock sl (lock);
    nextReadPosition = startSampleInFile;

    while (numSamples > 0)
    {
        if (const BufferedBlock* const block = getBlockContaining (startSampleInFile))
        {
            const int offset = (int) (startSampleInFile - block->range.getStart());
            const int numToDo = jmin (numSamples, (int) (block->range.getEnd() - startSampleInFile));

            for (int j = 0; j < numDestChannels; ++j)
            {
                if (float* dest = (float*) destSamples[j])
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
        }
        else
        {
            if (timeoutMs >= 0 && Time::getMillisecondCounter() >= startTime + (uint32) timeoutMs)
            {
                for (int j = 0; j < numDestChannels; ++j)
                    if (float* dest = (float*) destSamples[j])
                        FloatVectorOperations::clear (dest + startOffsetInDestBuffer, numSamples);

                break;
            }
            else
            {
                ScopedUnlock ul (lock);
                Thread::yield();
            }
        }
    }

    return true;
}

BufferingAudioReader::BufferedBlock::BufferedBlock (AudioFormatReader& reader, int64 pos, int numSamples)
    : range (pos, pos + numSamples),
      buffer ((int) reader.numChannels, numSamples)
{
    reader.read (&buffer, 0, numSamples, pos, true, true);
}

BufferingAudioReader::BufferedBlock* BufferingAudioReader::getBlockContaining (int64 pos) const noexcept
{
    for (int i = blocks.size(); --i >= 0;)
    {
        BufferedBlock* const b = blocks.getUnchecked(i);

        if (b->range.contains (pos))
            return b;
    }

    return nullptr;
}

int BufferingAudioReader::useTimeSlice()
{
    return readNextBufferChunk() ? 1 : 100;
}

bool BufferingAudioReader::readNextBufferChunk()
{
    const int64 pos = nextReadPosition;
    const int64 startPos = ((pos - 1024) / samplesPerBlock) * samplesPerBlock;
    const int64 endPos = startPos + numBlocks * samplesPerBlock;

    OwnedArray<BufferedBlock> newBlocks;

    for (int i = blocks.size(); --i >= 0;)
        if (blocks.getUnchecked(i)->range.intersects (Range<int64> (startPos, endPos)))
            newBlocks.add (blocks.getUnchecked(i));

    if (newBlocks.size() == numBlocks)
    {
        newBlocks.clear (false);
        return false;
    }

    for (int64 p = startPos; p < endPos; p += samplesPerBlock)
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
        newBlocks.removeObject (blocks.getUnchecked(i), false);

    return true;
}
