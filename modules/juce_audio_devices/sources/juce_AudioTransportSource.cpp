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

AudioTransportSource::AudioTransportSource()
{
}

AudioTransportSource::~AudioTransportSource()
{
    setSource (nullptr);
    releaseMasterResources();
}

void AudioTransportSource::setSource (PositionableAudioSource* const newSource,
                                      int readAheadSize, TimeSliceThread* readAheadThread,
                                      double sourceSampleRateToCorrectFor, int maxNumChannels)
{
    if (source == newSource)
    {
        if (source == nullptr)
            return;

        setSource (nullptr, 0, nullptr); // deselect and reselect to avoid releasing resources wrongly
    }

    ResamplingAudioSource* newResamplerSource = nullptr;
    BufferingAudioSource* newBufferingSource = nullptr;
    PositionableAudioSource* newPositionableSource = nullptr;
    AudioSource* newMasterSource = nullptr;

    std::unique_ptr<ResamplingAudioSource> oldResamplerSource (resamplerSource);
    std::unique_ptr<BufferingAudioSource> oldBufferingSource (bufferingSource);
    AudioSource* oldMasterSource = masterSource;

    if (newSource != nullptr)
    {
        newPositionableSource = newSource;

        if (readAheadSize > 0)
        {
            // If you want to use a read-ahead buffer, you must also provide a TimeSliceThread
            // for it to use!
            jassert (readAheadThread != nullptr);

            newPositionableSource = newBufferingSource
                = new BufferingAudioSource (newPositionableSource, *readAheadThread,
                                            false, readAheadSize, maxNumChannels);
        }

        newPositionableSource->setNextReadPosition (0);

        if (sourceSampleRateToCorrectFor > 0)
            newMasterSource = newResamplerSource
                = new ResamplingAudioSource (newPositionableSource, false, maxNumChannels);
        else
            newMasterSource = newPositionableSource;

        if (isPrepared)
        {
            if (newResamplerSource != nullptr && sourceSampleRateToCorrectFor > 0 && sampleRate > 0)
                newResamplerSource->setResamplingRatio (sourceSampleRateToCorrectFor / sampleRate);

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
        readAheadBufferSize = readAheadSize;
        sourceSampleRate = sourceSampleRateToCorrectFor;

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
        }

        sendChangeMessage();
    }
}

void AudioTransportSource::stop()
{
    if (playing)
    {
        playing = false;

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
        return (double) getNextReadPosition() / sampleRate;

    return 0.0;
}

double AudioTransportSource::getLengthInSeconds() const
{
    if (sampleRate > 0.0)
        return (double) getTotalLength() / sampleRate;

    return 0.0;
}

bool AudioTransportSource::hasStreamFinished() const noexcept
{
    if (positionableSource == nullptr)
        return true;

    if (positionableSource->isLooping())
        return false;

    return positionableSource->getNextReadPosition() >= positionableSource->getTotalLength();
}

void AudioTransportSource::setNextReadPosition (int64 newPosition)
{
    if (positionableSource != nullptr)
    {
        if (sampleRate > 0 && sourceSampleRate > 0)
            newPosition = (int64) ((double) newPosition * sourceSampleRate / sampleRate);

        positionableSource->setNextReadPosition (newPosition);

        if (resamplerSource != nullptr)
            resamplerSource->flushBuffers();
    }
}

int64 AudioTransportSource::getNextReadPosition() const
{
    const ScopedLock sl (callbackLock);

    if (positionableSource != nullptr)
    {
        const double ratio = (sampleRate > 0 && sourceSampleRate > 0) ? sampleRate / sourceSampleRate : 1.0;
        return (int64) ((double) positionableSource->getNextReadPosition() * ratio);
    }

    return 0;
}

int64 AudioTransportSource::getTotalLength() const
{
    const ScopedLock sl (callbackLock);

    if (positionableSource != nullptr)
    {
        const double ratio = (sampleRate > 0 && sourceSampleRate > 0) ? sampleRate / sourceSampleRate : 1.0;
        return (int64) ((double) positionableSource->getTotalLength() * ratio);
    }

    return 0;
}

bool AudioTransportSource::isLooping() const
{
    const ScopedLock sl (callbackLock);
    return positionableSource != nullptr && positionableSource->isLooping();
}

void AudioTransportSource::setGain (const float newGain) noexcept
{
    gain = newGain;
}

void AudioTransportSource::prepareToPlay (int samplesPerBlockExpected, double newSampleRate)
{
    const ScopedLock sl (callbackLock);

    sampleRate = newSampleRate;
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

        if (hasStreamFinished())
        {
            playing = false;
            sendChangeMessage();
        }

        stopped = ! playing;

        for (int i = info.buffer->getNumChannels(); --i >= 0;)
            info.buffer->applyGainRamp (i, info.startSample, info.numSamples, lastGain, gain);
    }
    else
    {
        info.clearActiveBufferRegion();
        stopped = true;
    }

    lastGain = gain;
}

} // namespace juce
