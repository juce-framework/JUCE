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

#include "juce_AudioTransportSource.h"
#include "../../memory/juce_ScopedPointer.h"


//==============================================================================
AudioTransportSource::AudioTransportSource()
    : source (nullptr),
      resamplerSource (nullptr),
      bufferingSource (nullptr),
      positionableSource (nullptr),
      masterSource (nullptr),
      gain (1.0f),
      lastGain (1.0f),
      playing (false),
      stopped (true),
      sampleRate (44100.0),
      sourceSampleRate (0.0),
      blockSize (128),
      readAheadBufferSize (0),
      isPrepared (false),
      inputStreamEOF (false)
{
}

AudioTransportSource::~AudioTransportSource()
{
    setSource (nullptr);

    releaseMasterResources();
}

void AudioTransportSource::setSource (PositionableAudioSource* const newSource,
                                      int readAheadBufferSize_,
                                      double sourceSampleRateToCorrectFor,
                                      int maxNumChannels)
{
    if (source == newSource)
    {
        if (source == nullptr)
            return;

        setSource (0, 0, 0); // deselect and reselect to avoid releasing resources wrongly
    }

    readAheadBufferSize = readAheadBufferSize_;
    sourceSampleRate = sourceSampleRateToCorrectFor;

    ResamplingAudioSource* newResamplerSource = nullptr;
    BufferingAudioSource* newBufferingSource = nullptr;
    PositionableAudioSource* newPositionableSource = nullptr;
    AudioSource* newMasterSource = nullptr;

    ScopedPointer <ResamplingAudioSource> oldResamplerSource (resamplerSource);
    ScopedPointer <BufferingAudioSource> oldBufferingSource (bufferingSource);
    AudioSource* oldMasterSource = masterSource;

    if (newSource != nullptr)
    {
        newPositionableSource = newSource;

        if (readAheadBufferSize_ > 0)
            newPositionableSource = newBufferingSource
                = new BufferingAudioSource (newPositionableSource, false, readAheadBufferSize_, maxNumChannels);

        newPositionableSource->setNextReadPosition (0);

        if (sourceSampleRateToCorrectFor > 0)
            newMasterSource = newResamplerSource
                = new ResamplingAudioSource (newPositionableSource, false, maxNumChannels);
        else
            newMasterSource = newPositionableSource;

        if (isPrepared)
        {
            if (newResamplerSource != nullptr && sourceSampleRate > 0 && sampleRate > 0)
                newResamplerSource->setResamplingRatio (sourceSampleRate / sampleRate);

            newMasterSource->prepareToPlay (blockSize, sampleRate);
        }
    }

    {
        const ScopedLock sl (callbackLock);

        source = newSource;
        resamplerSource = newResamplerSource;
        bufferingSource = newBufferingSource;
        masterSource = newMasterSource;
        positionableSource = newPositionableSource;

        playing = false;
    }

    if (oldMasterSource != nullptr)
        oldMasterSource->releaseResources();
}

void AudioTransportSource::start()
{
    if ((! playing) && masterSource != nullptr)
    {
        {
            const ScopedLock sl (callbackLock);
            playing = true;
            stopped = false;
            inputStreamEOF = false;
        }

        sendChangeMessage();
    }
}

void AudioTransportSource::stop()
{
    if (playing)
    {
        {
            const ScopedLock sl (callbackLock);
            playing = false;
        }

        int n = 500;
        while (--n >= 0 && ! stopped)
            Thread::sleep (2);

        sendChangeMessage();
    }
}

void AudioTransportSource::setPosition (double newPosition)
{
    if (sampleRate > 0.0)
        setNextReadPosition ((int64) (newPosition * sampleRate));
}

double AudioTransportSource::getCurrentPosition() const
{
    if (sampleRate > 0.0)
        return getNextReadPosition() / sampleRate;
    else
        return 0.0;
}

double AudioTransportSource::getLengthInSeconds() const
{
    return getTotalLength() / sampleRate;
}

void AudioTransportSource::setNextReadPosition (int64 newPosition)
{
    if (positionableSource != nullptr)
    {
        if (sampleRate > 0 && sourceSampleRate > 0)
            newPosition = (int64) (newPosition * sourceSampleRate / sampleRate);

        positionableSource->setNextReadPosition (newPosition);
    }
}

int64 AudioTransportSource::getNextReadPosition() const
{
    if (positionableSource != nullptr)
    {
        const double ratio = (sampleRate > 0 && sourceSampleRate > 0) ? sampleRate / sourceSampleRate : 1.0;

        return (int64) (positionableSource->getNextReadPosition() * ratio);
    }

    return 0;
}

int64 AudioTransportSource::getTotalLength() const
{
    const ScopedLock sl (callbackLock);

    if (positionableSource != nullptr)
    {
        const double ratio = (sampleRate > 0 && sourceSampleRate > 0) ? sampleRate / sourceSampleRate : 1.0;

        return (int64) (positionableSource->getTotalLength() * ratio);
    }

    return 0;
}

bool AudioTransportSource::isLooping() const
{
    const ScopedLock sl (callbackLock);

    return positionableSource != nullptr
            && positionableSource->isLooping();
}

void AudioTransportSource::setGain (const float newGain) noexcept
{
    gain = newGain;
}

void AudioTransportSource::prepareToPlay (int samplesPerBlockExpected,
                                          double sampleRate_)
{
    const ScopedLock sl (callbackLock);

    sampleRate = sampleRate_;
    blockSize = samplesPerBlockExpected;

    if (masterSource != nullptr)
        masterSource->prepareToPlay (samplesPerBlockExpected, sampleRate);

    if (resamplerSource != nullptr && sourceSampleRate > 0)
        resamplerSource->setResamplingRatio (sourceSampleRate / sampleRate);

    isPrepared = true;
}

void AudioTransportSource::releaseMasterResources()
{
    const ScopedLock sl (callbackLock);

    if (masterSource != nullptr)
        masterSource->releaseResources();

    isPrepared = false;
}

void AudioTransportSource::releaseResources()
{
    releaseMasterResources();
}

void AudioTransportSource::getNextAudioBlock (const AudioSourceChannelInfo& info)
{
    const ScopedLock sl (callbackLock);

    inputStreamEOF = false;

    if (masterSource != nullptr && ! stopped)
    {
        masterSource->getNextAudioBlock (info);

        if (! playing)
        {
            // just stopped playing, so fade out the last block..
            for (int i = info.buffer->getNumChannels(); --i >= 0;)
                info.buffer->applyGainRamp (i, info.startSample, jmin (256, info.numSamples), 1.0f, 0.0f);

            if (info.numSamples > 256)
                info.buffer->clear (info.startSample + 256, info.numSamples - 256);
        }

        if (positionableSource->getNextReadPosition() > positionableSource->getTotalLength() + 1
             && ! positionableSource->isLooping())
        {
            playing = false;
            inputStreamEOF = true;
            sendChangeMessage();
        }

        stopped = ! playing;

        for (int i = info.buffer->getNumChannels(); --i >= 0;)
        {
            info.buffer->applyGainRamp (i, info.startSample, info.numSamples,
                                        lastGain, gain);
        }
    }
    else
    {
        info.clearActiveBufferRegion();
        stopped = true;
    }

    lastGain = gain;
}

END_JUCE_NAMESPACE
