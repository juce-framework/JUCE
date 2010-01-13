/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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
#include "../../threads/juce_ScopedLock.h"
#include "../../core/juce_Singleton.h"
#include "../../containers/juce_VoidArray.h"
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

    juce_DeclareSingleton (SharedBufferingAudioSourceThread, false)

    void addSource (BufferingAudioSource* source)
    {
        const ScopedLock sl (lock);

        if (! sources.contains ((void*) source))
        {
            sources.add ((void*) source);
            startThread();

            stopTimer();
        }

        notify();
    }

    void removeSource (BufferingAudioSource* source)
    {
        const ScopedLock sl (lock);
        sources.removeValue ((void*) source);

        if (sources.size() == 0)
            startTimer (5000);
    }

private:
    VoidArray sources;
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

                BufferingAudioSource* const b = (BufferingAudioSource*) sources[i];

                if (b != 0 && b->readNextBufferChunk())
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

    SharedBufferingAudioSourceThread (const SharedBufferingAudioSourceThread&);
    const SharedBufferingAudioSourceThread& operator= (const SharedBufferingAudioSourceThread&);
};

juce_ImplementSingleton (SharedBufferingAudioSourceThread)

//==============================================================================
BufferingAudioSource::BufferingAudioSource (PositionableAudioSource* source_,
                                            const bool deleteSourceWhenDeleted_,
                                            int numberOfSamplesToBuffer_)
    : source (source_),
      deleteSourceWhenDeleted (deleteSourceWhenDeleted_),
      numberOfSamplesToBuffer (jmax (1024, numberOfSamplesToBuffer_)),
      buffer (2, 0),
      bufferValidStart (0),
      bufferValidEnd (0),
      nextPlayPos (0),
      wasSourceLooping (false)
{
    jassert (source_ != 0);

    jassert (numberOfSamplesToBuffer_ > 1024); // not much point using this class if you're
                                               //  not using a larger buffer..
}

BufferingAudioSource::~BufferingAudioSource()
{
    SharedBufferingAudioSourceThread* const thread = SharedBufferingAudioSourceThread::getInstanceWithoutCreating();

    if (thread != 0)
        thread->removeSource (this);

    if (deleteSourceWhenDeleted)
        delete source;
}

//==============================================================================
void BufferingAudioSource::prepareToPlay (int samplesPerBlockExpected, double sampleRate_)
{
    source->prepareToPlay (samplesPerBlockExpected, sampleRate_);

    sampleRate = sampleRate_;

    buffer.setSize (2, jmax (samplesPerBlockExpected * 2, numberOfSamplesToBuffer));
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

    if (thread != 0)
        thread->removeSource (this);

    buffer.setSize (2, 0);
    source->releaseResources();
}

void BufferingAudioSource::getNextAudioBlock (const AudioSourceChannelInfo& info)
{
    const ScopedLock sl (bufferStartPosLock);

    const int validStart = jlimit (bufferValidStart, bufferValidEnd, nextPlayPos) - nextPlayPos;
    const int validEnd   = jlimit (bufferValidStart, bufferValidEnd, nextPlayPos + info.numSamples) - nextPlayPos;

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
            for (int chan = jmin (2, info.buffer->getNumChannels()); --chan >= 0;)
            {
                const int startBufferIndex = (validStart + nextPlayPos) % buffer.getNumSamples();
                const int endBufferIndex = (validEnd + nextPlayPos) % buffer.getNumSamples();

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

    if (thread != 0)
        thread->notify();
}

int BufferingAudioSource::getNextReadPosition() const
{
    return (source->isLooping() && nextPlayPos > 0)
                    ? nextPlayPos % source->getTotalLength()
                    : nextPlayPos;
}

void BufferingAudioSource::setNextReadPosition (int newPosition)
{
    const ScopedLock sl (bufferStartPosLock);

    nextPlayPos = newPosition;

    SharedBufferingAudioSourceThread* const thread = SharedBufferingAudioSourceThread::getInstanceWithoutCreating();

    if (thread != 0)
        thread->notify();
}

bool BufferingAudioSource::readNextBufferChunk()
{
    bufferStartPosLock.enter();

    if (wasSourceLooping != isLooping())
    {
        wasSourceLooping = isLooping();
        bufferValidStart = 0;
        bufferValidEnd = 0;
    }

    int newBVS = jmax (0, nextPlayPos);
    int newBVE = newBVS + buffer.getNumSamples() - 4;
    int sectionToReadStart = 0;
    int sectionToReadEnd = 0;

    const int maxChunkSize = 2048;

    if (newBVS < bufferValidStart || newBVS >= bufferValidEnd)
    {
        newBVE = jmin (newBVE, newBVS + maxChunkSize);

        sectionToReadStart = newBVS;
        sectionToReadEnd = newBVE;

        bufferValidStart = 0;
        bufferValidEnd = 0;
    }
    else if (abs (newBVS - bufferValidStart) > 512
                || abs (newBVE - bufferValidEnd) > 512)
    {
        newBVE = jmin (newBVE, bufferValidEnd + maxChunkSize);

        sectionToReadStart = bufferValidEnd;
        sectionToReadEnd = newBVE;

        bufferValidStart = newBVS;
        bufferValidEnd = jmin (bufferValidEnd, newBVE);
    }

    bufferStartPosLock.exit();

    if (sectionToReadStart != sectionToReadEnd)
    {
        const int bufferIndexStart = sectionToReadStart % buffer.getNumSamples();
        const int bufferIndexEnd = sectionToReadEnd % buffer.getNumSamples();

        if (bufferIndexStart < bufferIndexEnd)
        {
            readBufferSection (sectionToReadStart,
                               sectionToReadEnd - sectionToReadStart,
                               bufferIndexStart);
        }
        else
        {
            const int initialSize = buffer.getNumSamples() - bufferIndexStart;

            readBufferSection (sectionToReadStart,
                               initialSize,
                               bufferIndexStart);

            readBufferSection (sectionToReadStart + initialSize,
                               (sectionToReadEnd - sectionToReadStart) - initialSize,
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

void BufferingAudioSource::readBufferSection (int start, int length, int bufferOffset)
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
