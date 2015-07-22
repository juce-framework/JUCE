/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

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

BufferingAudioSource::BufferingAudioSource (PositionableAudioSource* s,
                                            TimeSliceThread& thread,
                                            const bool deleteSourceWhenDeleted,
                                            const int bufferSizeSamples,
                                            const int numChannels)
    : source (s, deleteSourceWhenDeleted),
      backgroundThread (thread),
      numberOfSamplesToBuffer (jmax (1024, bufferSizeSamples)),
      numberOfChannels (numChannels),
      bufferValidStart (0),
      bufferValidEnd (0),
      nextPlayPos (0),
      sampleRate (0),
      wasSourceLooping (false),
      isPrepared (false)
{
    jassert (source != nullptr);

    jassert (numberOfSamplesToBuffer > 1024); // not much point using this class if you're
                                              //  not using a larger buffer..
}

BufferingAudioSource::~BufferingAudioSource()
{
    releaseResources();
}

//==============================================================================
void BufferingAudioSource::prepareToPlay (int samplesPerBlockExpected, double newSampleRate)
{
    const int bufferSizeNeeded = jmax (samplesPerBlockExpected * 2, numberOfSamplesToBuffer);

    if (newSampleRate != sampleRate
         || bufferSizeNeeded != buffer.getNumSamples()
         || ! isPrepared)
    {
        backgroundThread.removeTimeSliceClient (this);

        isPrepared = true;
        sampleRate = newSampleRate;

        source->prepareToPlay (samplesPerBlockExpected, newSampleRate);

        buffer.setSize (numberOfChannels, bufferSizeNeeded);
        buffer.clear();

        bufferValidStart = 0;
        bufferValidEnd = 0;

        backgroundThread.addTimeSliceClient (this);

        while (bufferValidEnd - bufferValidStart < jmin (((int) newSampleRate) / 4,
                                                         buffer.getNumSamples() / 2))
        {
            backgroundThread.moveToFrontOfQueue (this);
            Thread::sleep (5);
        }
    }
}

void BufferingAudioSource::releaseResources()
{
    isPrepared = false;
    backgroundThread.removeTimeSliceClient (this);

    buffer.setSize (numberOfChannels, 0);
    source->releaseResources();
}

void BufferingAudioSource::getNextAudioBlock (const AudioSourceChannelInfo& info)
{
    const ScopedLock sl (bufferStartPosLock);

    const int validStart = (int) (jlimit (bufferValidStart, bufferValidEnd, nextPlayPos) - nextPlayPos);
    const int validEnd   = (int) (jlimit (bufferValidStart, bufferValidEnd, nextPlayPos + info.numSamples) - nextPlayPos);

    if (validStart == validEnd)
    {
        // total cache miss
        info.clearActiveBufferRegion();
    }
    else
    {
        if (validStart > 0)
            info.buffer->clear (info.startSample, validStart);  // partial cache miss at start

        if (validEnd < info.numSamples)
            info.buffer->clear (info.startSample + validEnd,
                                info.numSamples - validEnd);    // partial cache miss at end

        if (validStart < validEnd)
        {
            for (int chan = jmin (numberOfChannels, info.buffer->getNumChannels()); --chan >= 0;)
            {
                jassert (buffer.getNumSamples() > 0);
                const int startBufferIndex = (int) ((validStart + nextPlayPos) % buffer.getNumSamples());
                const int endBufferIndex   = (int) ((validEnd + nextPlayPos)   % buffer.getNumSamples());

                if (startBufferIndex < endBufferIndex)
                {
                    info.buffer->copyFrom (chan, info.startSample + validStart,
                                           buffer,
                                           chan, startBufferIndex,
                                           validEnd - validStart);
                }
                else
                {
                    const int initialSize = buffer.getNumSamples() - startBufferIndex;

                    info.buffer->copyFrom (chan, info.startSample + validStart,
                                           buffer,
                                           chan, startBufferIndex,
                                           initialSize);

                    info.buffer->copyFrom (chan, info.startSample + validStart + initialSize,
                                           buffer,
                                           chan, 0,
                                           (validEnd - validStart) - initialSize);
                }
            }
        }

        nextPlayPos += info.numSamples;
    }
}

int64 BufferingAudioSource::getNextReadPosition() const
{
    jassert (source->getTotalLength() > 0);
    return (source->isLooping() && nextPlayPos > 0)
                    ? nextPlayPos % source->getTotalLength()
                    : nextPlayPos;
}

void BufferingAudioSource::setNextReadPosition (int64 newPosition)
{
    const ScopedLock sl (bufferStartPosLock);

    nextPlayPos = newPosition;
    backgroundThread.moveToFrontOfQueue (this);
}

bool BufferingAudioSource::readNextBufferChunk()
{
    int64 newBVS, newBVE, sectionToReadStart, sectionToReadEnd;

    {
        const ScopedLock sl (bufferStartPosLock);

        if (wasSourceLooping != isLooping())
        {
            wasSourceLooping = isLooping();
            bufferValidStart = 0;
            bufferValidEnd = 0;
        }

        newBVS = jmax ((int64) 0, nextPlayPos);
        newBVE = newBVS + buffer.getNumSamples() - 4;
        sectionToReadStart = 0;
        sectionToReadEnd = 0;

        const int maxChunkSize = 2048;

        if (newBVS < bufferValidStart || newBVS >= bufferValidEnd)
        {
            newBVE = jmin (newBVE, newBVS + maxChunkSize);

            sectionToReadStart = newBVS;
            sectionToReadEnd = newBVE;

            bufferValidStart = 0;
            bufferValidEnd = 0;
        }
        else if (std::abs ((int) (newBVS - bufferValidStart)) > 512
                  || std::abs ((int) (newBVE - bufferValidEnd)) > 512)
        {
            newBVE = jmin (newBVE, bufferValidEnd + maxChunkSize);

            sectionToReadStart = bufferValidEnd;
            sectionToReadEnd = newBVE;

            bufferValidStart = newBVS;
            bufferValidEnd = jmin (bufferValidEnd, newBVE);
        }
    }

    if (sectionToReadStart == sectionToReadEnd)
        return false;

    jassert (buffer.getNumSamples() > 0);
    const int bufferIndexStart = (int) (sectionToReadStart % buffer.getNumSamples());
    const int bufferIndexEnd   = (int) (sectionToReadEnd   % buffer.getNumSamples());

    if (bufferIndexStart < bufferIndexEnd)
    {
        readBufferSection (sectionToReadStart,
                           (int) (sectionToReadEnd - sectionToReadStart),
                           bufferIndexStart);
    }
    else
    {
        const int initialSize = buffer.getNumSamples() - bufferIndexStart;

        readBufferSection (sectionToReadStart,
                           initialSize,
                           bufferIndexStart);

        readBufferSection (sectionToReadStart + initialSize,
                           (int) (sectionToReadEnd - sectionToReadStart) - initialSize,
                           0);
    }

    {
        const ScopedLock sl2 (bufferStartPosLock);

        bufferValidStart = newBVS;
        bufferValidEnd = newBVE;
    }

    return true;
}

void BufferingAudioSource::readBufferSection (const int64 start, const int length, const int bufferOffset)
{
    if (source->getNextReadPosition() != start)
        source->setNextReadPosition (start);

    AudioSourceChannelInfo info (&buffer, bufferOffset, length);
    source->getNextAudioBlock (info);
}

int BufferingAudioSource::useTimeSlice()
{
    return readNextBufferChunk() ? 1 : 100;
}
