/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#include "../../../juce_core/basics/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_AudioTransportSource.h"
#include "../../../juce_core/threads/juce_ScopedLock.h"


//==============================================================================
AudioTransportSource::AudioTransportSource()
    : source (0),
      resamplerSource (0),
      bufferingSource (0),
      positionableSource (0),
      masterSource (0),
      gain (1.0f),
      lastGain (1.0f),
      playing (false),
      stopped (true),
      sampleRate (44100.0),
      sourceSampleRate (0.0),
      speed (1.0),
      blockSize (128),
      readAheadBufferSize (0),
      isPrepared (false)
{
}

AudioTransportSource::~AudioTransportSource()
{
    setSource (0);

    releaseResources();
}

void AudioTransportSource::setSource (PositionableAudioSource* const newSource,
                                      int readAheadBufferSize_,
                                      double sourceSampleRateToCorrectFor)
{
    if (source == newSource)
    {
        if (source == 0
            || (readAheadBufferSize_ == readAheadBufferSize
                  && sourceSampleRate == sourceSampleRateToCorrectFor))
        {
            deleteAndZero (resamplerSource);
            deleteAndZero (bufferingSource);
            masterSource = 0;
            positionableSource = 0;
            return;
        }
        else
        {
            setSource (0, 0, 0); // deselect and reselect to avoid releasing resources wrongly
        }
    }

    readAheadBufferSize = readAheadBufferSize_;
    sourceSampleRate = sourceSampleRateToCorrectFor;

    ResamplingAudioSource* newResamplerSource = 0;
    BufferingAudioSource* newBufferingSource = 0;
    PositionableAudioSource* newPositionableSource = 0;
    AudioSource* newMasterSource = 0;

    ResamplingAudioSource* oldResamplerSource = resamplerSource;
    BufferingAudioSource* oldBufferingSource = bufferingSource;
    AudioSource* oldMasterSource = masterSource;

    if (newSource != 0)
    {
        newPositionableSource = newSource;

        if (readAheadBufferSize_ > 0)
            newPositionableSource = newBufferingSource
                = new BufferingAudioSource (newPositionableSource, false, readAheadBufferSize_);

        newPositionableSource->setNextReadPosition (0);

        if (sourceSampleRateToCorrectFor != 0)
            newMasterSource = newResamplerSource
                = new ResamplingAudioSource (newPositionableSource, false);
        else
            newMasterSource = newPositionableSource;

        if (isPrepared)
        {
            if (newResamplerSource != 0 && sourceSampleRate > 0 && sampleRate > 0)
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

    if (oldMasterSource != 0)
        oldMasterSource->releaseResources();

    if (oldResamplerSource != 0)
        delete oldResamplerSource;

    if (oldBufferingSource != 0)
        delete oldBufferingSource;
}

void AudioTransportSource::start()
{
    if ((! playing) && masterSource != 0)
    {
        callbackLock.enter();
        playing = true;
        stopped = false;
        callbackLock.exit();

        sendChangeMessage (this);
    }
}

void AudioTransportSource::stop()
{
    if (playing)
    {
        callbackLock.enter();
        playing = false;
        callbackLock.exit();

        int n = 500;
        while (--n >= 0 && ! stopped)
            Thread::sleep (2);

        sendChangeMessage (this);
    }
}

void AudioTransportSource::setPosition (double newPosition)
{
    if (sampleRate > 0.0)
        setNextReadPosition (roundDoubleToInt (newPosition * sampleRate));
}

double AudioTransportSource::getCurrentPosition() const
{
    if (sampleRate > 0.0)
        return getNextReadPosition() / sampleRate;
    else
        return 0.0;
}

void AudioTransportSource::setNextReadPosition (int newPosition)
{
    if (positionableSource != 0)
    {
        if (sampleRate > 0 && sourceSampleRate > 0)
            newPosition = roundDoubleToInt (newPosition * sourceSampleRate / sampleRate);

        positionableSource->setNextReadPosition (newPosition);
    }
}

int AudioTransportSource::getNextReadPosition() const
{
    if (positionableSource != 0)
    {
        const double ratio = (sampleRate > 0 && sourceSampleRate > 0) ? sampleRate / sourceSampleRate : 1.0;

        return roundDoubleToInt (positionableSource->getNextReadPosition() * ratio);
    }

    return 0;
}

int AudioTransportSource::getTotalLength() const
{
    const ScopedLock sl (callbackLock);

    if (positionableSource != 0)
    {
        const double ratio = (sampleRate > 0 && sourceSampleRate > 0) ? sampleRate / sourceSampleRate : 1.0;

        return roundDoubleToInt (positionableSource->getTotalLength() * ratio);
    }

    return 0;
}

bool AudioTransportSource::isLooping() const
{
    const ScopedLock sl (callbackLock);

    return positionableSource != 0
            && positionableSource->isLooping();
}

void AudioTransportSource::setGain (const float newGain) throw()
{
    gain = newGain;
}

void AudioTransportSource::prepareToPlay (int samplesPerBlockExpected,
                                          double sampleRate_)
{
    const ScopedLock sl (callbackLock);

    sampleRate = sampleRate_;
    blockSize = samplesPerBlockExpected;

    if (masterSource != 0)
        masterSource->prepareToPlay (samplesPerBlockExpected, sampleRate);

    if (resamplerSource != 0 && sourceSampleRate != 0)
        resamplerSource->setResamplingRatio (sourceSampleRate / sampleRate);

    isPrepared = true;
}

void AudioTransportSource::releaseResources()
{
    const ScopedLock sl (callbackLock);

    if (masterSource != 0)
        masterSource->releaseResources();

    isPrepared = false;
}

void AudioTransportSource::getNextAudioBlock (const AudioSourceChannelInfo& info)
{
    const ScopedLock sl (callbackLock);

    if (masterSource != 0 && ! stopped)
    {
        masterSource->getNextAudioBlock (info);

        if (! playing)
        {
            // just stopped playing, so fade out the last block..
            for (int i = info.buffer->getNumChannels(); --i >= 0;)
                info.buffer->applyGainRamp (i, 0, jmin (256, info.numSamples), 1.0f, 0.0f);
        }

        if (positionableSource->getNextReadPosition() > positionableSource->getTotalLength() + 1
             && ! positionableSource->isLooping())
        {
            playing = false;
            sendChangeMessage (this);
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
