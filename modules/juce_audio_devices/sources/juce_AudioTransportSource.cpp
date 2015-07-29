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
                                      int readAheadSize, TimeSliceThread* readAheadThread,
                                      double sourceSampleRateToCorrectFor, int maxNumChannels)
{
    if (source == newSource)
    {
        if (source == nullptr)
            return;

        setSource (nullptr, 0, nullptr); // deselect and reselect to avoid releasing resources wrongly
    }

    readAheadBufferSize = readAheadSize;
    sourceSampleRate = sourceSampleRateToCorrectFor;

    ResamplingAudioSource* newResamplerSource = nullptr;
    BufferingAudioSource* newBufferingSource = nullptr;
    PositionableAudioSource* newPositionableSource = nullptr;
    AudioSource* newMasterSource = nullptr;

    ScopedPointer<ResamplingAudioSource> oldResamplerSource (resamplerSource);
    ScopedPointer<BufferingAudioSource> oldBufferingSource (bufferingSource);
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

        inputStreamEOF = false;
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

    return 0.0;
}

double AudioTransportSource::getLengthInSeconds() const
{
    if (sampleRate > 0.0)
        return getTotalLength() / sampleRate;

    return 0.0;
}

void AudioTransportSource::setNextReadPosition (int64 newPosition)
{
    if (positionableSource != nullptr)
    {
        if (sampleRate > 0 && sourceSampleRate > 0)
            newPosition = (int64) (newPosition * sourceSampleRate / sampleRate);

        positionableSource->setNextReadPosition (newPosition);

        if (resamplerSource != nullptr)
            resamplerSource->flushBuffers();

        inputStreamEOF = false;
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

    inputStreamEOF = false;
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

        if (positionableSource->getNextReadPosition() > positionableSource->getTotalLength() + 1
              && ! positionableSource->isLooping())
        {
            playing = false;
            inputStreamEOF = true;
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
