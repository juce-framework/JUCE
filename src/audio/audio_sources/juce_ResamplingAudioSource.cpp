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

#include "juce_ResamplingAudioSource.h"
#include "../../threads/juce_ScopedLock.h"


//==============================================================================
ResamplingAudioSource::ResamplingAudioSource (AudioSource* const inputSource,
                                              const bool deleteInputWhenDeleted_)
    : input (inputSource),
      deleteInputWhenDeleted (deleteInputWhenDeleted_),
      ratio (1.0),
      lastRatio (1.0),
      buffer (2, 0),
      sampsInBuffer (0)
{
    jassert (input != 0);
}

ResamplingAudioSource::~ResamplingAudioSource()
{
    if (deleteInputWhenDeleted)
        delete input;
}

void ResamplingAudioSource::setResamplingRatio (const double samplesInPerOutputSample)
{
    jassert (samplesInPerOutputSample > 0);

    const ScopedLock sl (ratioLock);
    ratio = jmax (0.0, samplesInPerOutputSample);
}

void ResamplingAudioSource::prepareToPlay (int samplesPerBlockExpected,
                                           double sampleRate)
{
    const ScopedLock sl (ratioLock);

    input->prepareToPlay (samplesPerBlockExpected, sampleRate);

    buffer.setSize (2, roundToInt (samplesPerBlockExpected * ratio) + 32);
    buffer.clear();
    sampsInBuffer = 0;
    bufferPos = 0;
    subSampleOffset = 0.0;

    createLowPass (ratio);
    resetFilters();
}

void ResamplingAudioSource::releaseResources()
{
    input->releaseResources();
    buffer.setSize (2, 0);
}

void ResamplingAudioSource::getNextAudioBlock (const AudioSourceChannelInfo& info)
{
    const ScopedLock sl (ratioLock);

    if (lastRatio != ratio)
    {
        createLowPass (ratio);
        lastRatio = ratio;
    }

    const int sampsNeeded = roundToInt (info.numSamples * ratio) + 2;

    int bufferSize = buffer.getNumSamples();

    if (bufferSize < sampsNeeded + 8)
    {
        bufferPos %= bufferSize;
        bufferSize = sampsNeeded + 32;
        buffer.setSize (buffer.getNumChannels(), bufferSize, true, true);
    }

    bufferPos %= bufferSize;

    int endOfBufferPos = bufferPos + sampsInBuffer;

    while (sampsNeeded > sampsInBuffer)
    {
        endOfBufferPos %= bufferSize;

        int numToDo = jmin (sampsNeeded - sampsInBuffer,
                            bufferSize - endOfBufferPos);

        AudioSourceChannelInfo readInfo;
        readInfo.buffer = &buffer;
        readInfo.numSamples = numToDo;
        readInfo.startSample = endOfBufferPos;

        input->getNextAudioBlock (readInfo);

        if (ratio > 1.0001)
        {
            // for down-sampling, pre-apply the filter..

            for (int i = jmin (2, info.buffer->getNumChannels()); --i >= 0;)
                applyFilter (buffer.getSampleData (i, endOfBufferPos), numToDo, filterStates[i]);
        }

        sampsInBuffer += numToDo;
        endOfBufferPos += numToDo;
    }

    float* dl = info.buffer->getSampleData (0, info.startSample);
    float* dr = (info.buffer->getNumChannels() > 1) ? info.buffer->getSampleData (1, info.startSample) : 0;

    const float* const bl = buffer.getSampleData (0, 0);
    const float* const br = buffer.getSampleData (1, 0);

    int nextPos = (bufferPos + 1) % bufferSize;

    for (int m = info.numSamples; --m >= 0;)
    {
        const float alpha = (float) subSampleOffset;
        const float invAlpha = 1.0f - alpha;

        *dl++ = bl [bufferPos] * invAlpha + bl [nextPos] * alpha;

        if (dr != 0)
            *dr++ = br [bufferPos] * invAlpha + br [nextPos] * alpha;

        subSampleOffset += ratio;

        jassert (sampsInBuffer > 0);

        while (subSampleOffset >= 1.0)
        {
            if (++bufferPos >= bufferSize)
                bufferPos = 0;

            --sampsInBuffer;

            nextPos = (bufferPos + 1) % bufferSize;
            subSampleOffset -= 1.0;
        }
    }

    if (ratio < 0.9999)
    {
        // for up-sampling, apply the filter after transposing..

        for (int i = jmin (2, info.buffer->getNumChannels()); --i >= 0;)
            applyFilter (info.buffer->getSampleData (i, info.startSample), info.numSamples, filterStates[i]);
    }
    else if (ratio <= 1.0001)
    {
        // if the filter's not currently being applied, keep it stoked with the last couple of samples to avoid discontinuities
        for (int i = jmin (2, info.buffer->getNumChannels()); --i >= 0;)
        {
            const float* const endOfBuffer = info.buffer->getSampleData (i, info.startSample + info.numSamples - 1);
            FilterState& fs = filterStates[i];

            if (info.numSamples > 1)
            {
                fs.y2 = fs.x2 = *(endOfBuffer - 1);
            }
            else
            {
                fs.y2 = fs.y1;
                fs.x2 = fs.x1;
            }

            fs.y1 = fs.x1 = *endOfBuffer;
        }
    }

    jassert (sampsInBuffer >= 0);
}

void ResamplingAudioSource::createLowPass (const double frequencyRatio)
{
    const double proportionalRate = (frequencyRatio > 1.0) ? 0.5 / frequencyRatio
                                                           : 0.5 * frequencyRatio;

    const double n = 1.0 / tan (double_Pi * jmax (0.001, proportionalRate));
    const double nSquared = n * n;
    const double c1 = 1.0 / (1.0 + sqrt (2.0) * n + nSquared);

    setFilterCoefficients (c1,
                           c1 * 2.0f,
                           c1,
                           1.0,
                           c1 * 2.0 * (1.0 - nSquared),
                           c1 * (1.0 - sqrt (2.0) * n + nSquared));
}

void ResamplingAudioSource::setFilterCoefficients (double c1, double c2, double c3, double c4, double c5, double c6)
{
    const double a = 1.0 / c4;

    c1 *= a;
    c2 *= a;
    c3 *= a;
    c5 *= a;
    c6 *= a;

    coefficients[0] = c1;
    coefficients[1] = c2;
    coefficients[2] = c3;
    coefficients[3] = c4;
    coefficients[4] = c5;
    coefficients[5] = c6;
}

void ResamplingAudioSource::resetFilters()
{
    zeromem (filterStates, sizeof (filterStates));
}

void ResamplingAudioSource::applyFilter (float* samples, int num, FilterState& fs)
{
    while (--num >= 0)
    {
        const double in = *samples;

        double out = coefficients[0] * in
                     + coefficients[1] * fs.x1
                     + coefficients[2] * fs.x2
                     - coefficients[4] * fs.y1
                     - coefficients[5] * fs.y2;

#if JUCE_INTEL
        if (! (out < -1.0e-8 || out > 1.0e-8))
            out = 0;
#endif

        fs.x2 = fs.x1;
        fs.x1 = in;
        fs.y2 = fs.y1;
        fs.y1 = out;

        *samples++ = (float) out;
    }
}

END_JUCE_NAMESPACE
