/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#include "../../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_BufferingAudioSource.h"
#include "../../core/juce_Singleton.h"
#include "../../containers/juce_Array.h"
#include "../../utilities/juce_DeletedAtShutdown.h"
#include "../../events/juce_Timer.h"


//==============================================================================
class SharedBufferingAudioSourceThread  : public DeletedAtShutdown,
                                          public Thread,
                                          private Timer
{
public:
    SharedBufferingAudioSourceThread()
        : Thread ("Audio Buffer")
    {
    }

    ~SharedBufferingAudioSourceThread()
    {
        stopThread (10000);
        clearSingletonInstance();
    }

    juce_DeclareSingleton (SharedBufferingAudioSourceThread, false);

    void addSource (BufferingAudioSource* source)
    {
        const ScopedLock sl (lock);

        if (! sources.contains (source))
        {
            sources.add (source);
            startThread();

            stopTimer();
        }

        notify();
    }

    void removeSource (BufferingAudioSource* source)
    {
        const ScopedLock sl (lock);
        sources.removeValue (source);

        if (sources.size() == 0)
            startTimer (5000);
    }

private:
    Array <BufferingAudioSource*> sources;
    CriticalSection lock;

    void run()
    {
        while (! threadShouldExit())
        {
            bool busy = false;

            for (int i = sources.size(); --i >= 0;)
            {
                if (threadShouldExit())
                    return;

                const ScopedLock sl (lock);

                BufferingAudioSource* const b = sources[i];

                if (b != nullptr && b->readNextBufferChunk())
                    busy = true;
            }

            if (! busy)
                wait (500);
        }
    }

    void timerCallback()
    {
        stopTimer();

        if (sources.size() == 0)
            deleteInstance();
    }

    JUCE_DECLARE_NON_COPYABLE (SharedBufferingAudioSourceThread);
};

juce_ImplementSingleton (SharedBufferingAudioSourceThread)

//==============================================================================
BufferingAudioSource::BufferingAudioSource (PositionableAudioSource* source_,
                                            const bool deleteSourceWhenDeleted,
                                            const int numberOfSamplesToBuffer_,
                                            const int numberOfChannels_)
    : source (source_, deleteSourceWhenDeleted),
      numberOfSamplesToBuffer (jmax (1024, numberOfSamplesToBuffer_)),
      numberOfChannels (numberOfChannels_),
      buffer (numberOfChannels_, 0),
      bufferValidStart (0),
      bufferValidEnd (0),
      nextPlayPos (0),
      wasSourceLooping (false)
{
    jassert (source_ != nullptr);

    jassert (numberOfSamplesToBuffer_ > 1024); // not much point using this class if you're
                                               //  not using a larger buffer..
}

BufferingAudioSource::~BufferingAudioSource()
{
    SharedBufferingAudioSourceThread* const thread = SharedBufferingAudioSourceThread::getInstanceWithoutCreating();

    if (thread != nullptr)
        thread->removeSource (this);
}

//==============================================================================
void BufferingAudioSource::prepareToPlay (int samplesPerBlockExpected, double sampleRate_)
{
    source->prepareToPlay (samplesPerBlockExpected, sampleRate_);

    sampleRate = sampleRate_;

    buffer.setSize (numberOfChannels, jmax (samplesPerBlockExpected * 2, numberOfSamplesToBuffer));
    buffer.clear();

    bufferValidStart = 0;
    bufferValidEnd = 0;

    SharedBufferingAudioSourceThread::getInstance()->addSource (this);

    while (bufferValidEnd - bufferValidStart < jmin (((int) sampleRate_) / 4,
                                                     buffer.getNumSamples() / 2))
    {
        SharedBufferingAudioSourceThread::getInstance()->notify();
        Thread::sleep (5);
    }
}

void BufferingAudioSource::releaseResources()
{
    SharedBufferingAudioSourceThread* const thread = SharedBufferingAudioSourceThread::getInstanceWithoutCreating();

    if (thread != nullptr)
        thread->removeSource (this);

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
                const int startBufferIndex = (int) ((validStart + nextPlayPos) % buffer.getNumSamples());
                const int endBufferIndex = (int) ((validEnd + nextPlayPos) % buffer.getNumSamples());

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

        if (source->isLooping() && nextPlayPos > 0)
            nextPlayPos %= source->getTotalLength();
    }

    SharedBufferingAudioSourceThread* const thread = SharedBufferingAudioSourceThread::getInstanceWithoutCreating();

    if (thread != nullptr)
        thread->notify();
}

int64 BufferingAudioSource::getNextReadPosition() const
{
    return (source->isLooping() && nextPlayPos > 0)
                    ? nextPlayPos % source->getTotalLength()
                    : nextPlayPos;
}

void BufferingAudioSource::setNextReadPosition (int64 newPosition)
{
    const ScopedLock sl (bufferStartPosLock);

    nextPlayPos = newPosition;

    SharedBufferingAudioSourceThread* const thread = SharedBufferingAudioSourceThread::getInstanceWithoutCreating();

    if (thread != nullptr)
        thread->notify();
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

    if (sectionToReadStart != sectionToReadEnd)
    {
        const int bufferIndexStart = (int) (sectionToReadStart % buffer.getNumSamples());
        const int bufferIndexEnd = (int) (sectionToReadEnd % buffer.getNumSamples());

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

        const ScopedLock sl2 (bufferStartPosLock);

        bufferValidStart = newBVS;
        bufferValidEnd = newBVE;

        return true;
    }
    else
    {
        return false;
    }
}

void BufferingAudioSource::readBufferSection (const int64 start, const int length, const int bufferOffset)
{
    if (source->getNextReadPosition() != start)
        source->setNextReadPosition (start);

    AudioSourceChannelInfo info;
    info.buffer = &buffer;
    info.startSample = bufferOffset;
    info.numSamples = length;

    source->getNextAudioBlock (info);
}

END_JUCE_NAMESPACE
