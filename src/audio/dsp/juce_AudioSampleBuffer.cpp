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

#include "juce_AudioSampleBuffer.h"
#include "../audio_file_formats/juce_AudioFormatReader.h"
#include "../audio_file_formats/juce_AudioFormatWriter.h"


//==============================================================================
AudioSampleBuffer::AudioSampleBuffer (const int numChannels_,
                                      const int numSamples) throw()
  : numChannels (numChannels_),
    size (numSamples)
{
    jassert (numSamples >= 0);
    jassert (numChannels_ > 0);

    allocateData();
}

AudioSampleBuffer::AudioSampleBuffer (const AudioSampleBuffer& other) throw()
  : numChannels (other.numChannels),
    size (other.size)
{
    allocateData();
    const size_t numBytes = size * sizeof (float);

    for (int i = 0; i < numChannels; ++i)
        memcpy (channels[i], other.channels[i], numBytes);
}

void AudioSampleBuffer::allocateData()
{
    const size_t channelListSize = (numChannels + 1) * sizeof (float*);
    allocatedBytes = (int) (numChannels * size * sizeof (float) + channelListSize + 32);
    allocatedData.malloc (allocatedBytes);
    channels = (float**) allocatedData;

    float* chan = (float*) (allocatedData + channelListSize);
    for (int i = 0; i < numChannels; ++i)
    {
        channels[i] = chan;
        chan += size;
    }

    channels [numChannels] = 0;
}

AudioSampleBuffer::AudioSampleBuffer (float** dataToReferTo,
                                      const int numChannels_,
                                      const int numSamples) throw()
    : numChannels (numChannels_),
      size (numSamples),
      allocatedBytes (0)
{
    jassert (numChannels_ > 0);
    allocateChannels (dataToReferTo);
}

void AudioSampleBuffer::setDataToReferTo (float** dataToReferTo,
                                          const int newNumChannels,
                                          const int newNumSamples) throw()
{
    jassert (newNumChannels > 0);

    allocatedBytes = 0;
    allocatedData.free();

    numChannels = newNumChannels;
    size = newNumSamples;

    allocateChannels (dataToReferTo);
}

void AudioSampleBuffer::allocateChannels (float** const dataToReferTo)
{
    // (try to avoid doing a malloc here, as that'll blow up things like Pro-Tools)
    if (numChannels < numElementsInArray (preallocatedChannelSpace))
    {
        channels = (float**) preallocatedChannelSpace;
    }
    else
    {
        allocatedData.malloc (numChannels + 1, sizeof (float*));
        channels = (float**) allocatedData;
    }

    for (int i = 0; i < numChannels; ++i)
    {
        // you have to pass in the same number of valid pointers as numChannels
        jassert (dataToReferTo[i] != 0);

        channels[i] = dataToReferTo[i];
    }

    channels [numChannels] = 0;
}

const AudioSampleBuffer& AudioSampleBuffer::operator= (const AudioSampleBuffer& other) throw()
{
    if (this != &other)
    {
        setSize (other.getNumChannels(), other.getNumSamples(), false, false, false);

        const size_t numBytes = size * sizeof (float);

        for (int i = 0; i < numChannels; ++i)
            memcpy (channels[i], other.channels[i], numBytes);
    }

    return *this;
}

AudioSampleBuffer::~AudioSampleBuffer() throw()
{
}

void AudioSampleBuffer::setSize (const int newNumChannels,
                                 const int newNumSamples,
                                 const bool keepExistingContent,
                                 const bool clearExtraSpace,
                                 const bool avoidReallocating) throw()
{
    jassert (newNumChannels > 0);

    if (newNumSamples != size || newNumChannels != numChannels)
    {
        const size_t channelListSize = (newNumChannels + 1) * sizeof (float*);
        const size_t newTotalBytes = (newNumChannels * newNumSamples * sizeof (float)) + channelListSize + 32;

        if (keepExistingContent)
        {
            HeapBlock <char> newData;
            newData.allocate (newTotalBytes, clearExtraSpace);

            const int numChansToCopy = jmin (numChannels, newNumChannels);
            const size_t numBytesToCopy = sizeof (float) * jmin (newNumSamples, size);

            float** const newChannels = (float**) newData;
            float* newChan = (float*) (newData + channelListSize);
            for (int i = 0; i < numChansToCopy; ++i)
            {
                memcpy (newChan, channels[i], numBytesToCopy);
                newChannels[i] = newChan;
                newChan += newNumSamples;
            }

            allocatedData.swapWith (newData);
            allocatedBytes = (int) newTotalBytes;
            channels = (float**) allocatedData;
        }
        else
        {
            if (avoidReallocating && allocatedBytes >= newTotalBytes)
            {
                if (clearExtraSpace)
                    zeromem (allocatedData, newTotalBytes);
            }
            else
            {
                allocatedBytes = newTotalBytes;
                allocatedData.allocate (newTotalBytes, clearExtraSpace);
                channels = (float**) allocatedData;
            }

            float* chan = (float*) (allocatedData + channelListSize);
            for (int i = 0; i < newNumChannels; ++i)
            {
                channels[i] = chan;
                chan += newNumSamples;
            }
        }

        channels [newNumChannels] = 0;
        size = newNumSamples;
        numChannels = newNumChannels;
    }
}

void AudioSampleBuffer::clear() throw()
{
    for (int i = 0; i < numChannels; ++i)
        zeromem (channels[i], size * sizeof (float));
}

void AudioSampleBuffer::clear (const int startSample,
                               const int numSamples) throw()
{
    jassert (startSample >= 0 && startSample + numSamples <= size);

    for (int i = 0; i < numChannels; ++i)
        zeromem (channels [i] + startSample, numSamples * sizeof (float));
}

void AudioSampleBuffer::clear (const int channel,
                               const int startSample,
                               const int numSamples) throw()
{
    jassert (((unsigned int) channel) < (unsigned int) numChannels);
    jassert (startSample >= 0 && startSample + numSamples <= size);

    zeromem (channels [channel] + startSample, numSamples * sizeof (float));
}

void AudioSampleBuffer::applyGain (const int channel,
                                   const int startSample,
                                   int numSamples,
                                   const float gain) throw()
{
    jassert (((unsigned int) channel) < (unsigned int) numChannels);
    jassert (startSample >= 0 && startSample + numSamples <= size);

    if (gain != 1.0f)
    {
        float* d = channels [channel] + startSample;

        if (gain == 0.0f)
        {
            zeromem (d, sizeof (float) * numSamples);
        }
        else
        {
            while (--numSamples >= 0)
                *d++ *= gain;
        }
    }
}

void AudioSampleBuffer::applyGainRamp (const int channel,
                                       const int startSample,
                                       int numSamples,
                                       float startGain,
                                       float endGain) throw()
{
    if (startGain == endGain)
    {
        applyGain (channel, startSample, numSamples, startGain);
    }
    else
    {
        jassert (((unsigned int) channel) < (unsigned int) numChannels);
        jassert (startSample >= 0 && startSample + numSamples <= size);

        const float increment = (endGain - startGain) / numSamples;
        float* d = channels [channel] + startSample;

        while (--numSamples >= 0)
        {
            *d++ *= startGain;
            startGain += increment;
        }
    }
}

void AudioSampleBuffer::applyGain (const int startSample,
                                   const int numSamples,
                                   const float gain) throw()
{
    for (int i = 0; i < numChannels; ++i)
        applyGain (i, startSample, numSamples, gain);
}

void AudioSampleBuffer::addFrom (const int destChannel,
                                 const int destStartSample,
                                 const AudioSampleBuffer& source,
                                 const int sourceChannel,
                                 const int sourceStartSample,
                                 int numSamples,
                                 const float gain) throw()
{
    jassert (&source != this || sourceChannel != destChannel);
    jassert (((unsigned int) destChannel) < (unsigned int) numChannels);
    jassert (destStartSample >= 0 && destStartSample + numSamples <= size);
    jassert (((unsigned int) sourceChannel) < (unsigned int) source.numChannels);
    jassert (sourceStartSample >= 0 && sourceStartSample + numSamples <= source.size);

    if (gain != 0.0f && numSamples > 0)
    {
        float* d = channels [destChannel] + destStartSample;
        const float* s  = source.channels [sourceChannel] + sourceStartSample;

        if (gain != 1.0f)
        {
            while (--numSamples >= 0)
                *d++ += gain * *s++;
        }
        else
        {
            while (--numSamples >= 0)
                *d++ += *s++;
        }
    }
}

void AudioSampleBuffer::addFrom (const int destChannel,
                                 const int destStartSample,
                                 const float* source,
                                 int numSamples,
                                 const float gain) throw()
{
    jassert (((unsigned int) destChannel) < (unsigned int) numChannels);
    jassert (destStartSample >= 0 && destStartSample + numSamples <= size);
    jassert (source != 0);

    if (gain != 0.0f && numSamples > 0)
    {
        float* d = channels [destChannel] + destStartSample;

        if (gain != 1.0f)
        {
            while (--numSamples >= 0)
                *d++ += gain * *source++;
        }
        else
        {
            while (--numSamples >= 0)
                *d++ += *source++;
        }
    }
}

void AudioSampleBuffer::addFromWithRamp (const int destChannel,
                                         const int destStartSample,
                                         const float* source,
                                         int numSamples,
                                         float startGain,
                                         const float endGain) throw()
{
    jassert (((unsigned int) destChannel) < (unsigned int) numChannels);
    jassert (destStartSample >= 0 && destStartSample + numSamples <= size);
    jassert (source != 0);

    if (startGain == endGain)
    {
        addFrom (destChannel,
                 destStartSample,
                 source,
                 numSamples,
                 startGain);
    }
    else
    {
        if (numSamples > 0 && (startGain != 0.0f || endGain != 0.0f))
        {
            const float increment = (endGain - startGain) / numSamples;
            float* d = channels [destChannel] + destStartSample;

            while (--numSamples >= 0)
            {
                *d++ += startGain * *source++;
                startGain += increment;
            }
        }
    }
}

void AudioSampleBuffer::copyFrom (const int destChannel,
                                  const int destStartSample,
                                  const AudioSampleBuffer& source,
                                  const int sourceChannel,
                                  const int sourceStartSample,
                                  int numSamples) throw()
{
    jassert (&source != this || sourceChannel != destChannel);
    jassert (((unsigned int) destChannel) < (unsigned int) numChannels);
    jassert (destStartSample >= 0 && destStartSample + numSamples <= size);
    jassert (((unsigned int) sourceChannel) < (unsigned int) source.numChannels);
    jassert (sourceStartSample >= 0 && sourceStartSample + numSamples <= source.size);

    if (numSamples > 0)
    {
        memcpy (channels [destChannel] + destStartSample,
                source.channels [sourceChannel] + sourceStartSample,
                sizeof (float) * numSamples);
    }
}

void AudioSampleBuffer::copyFrom (const int destChannel,
                                  const int destStartSample,
                                  const float* source,
                                  int numSamples) throw()
{
    jassert (((unsigned int) destChannel) < (unsigned int) numChannels);
    jassert (destStartSample >= 0 && destStartSample + numSamples <= size);
    jassert (source != 0);

    if (numSamples > 0)
    {
        memcpy (channels [destChannel] + destStartSample,
                source,
                sizeof (float) * numSamples);
    }
}

void AudioSampleBuffer::copyFrom (const int destChannel,
                                  const int destStartSample,
                                  const float* source,
                                  int numSamples,
                                  const float gain) throw()
{
    jassert (((unsigned int) destChannel) < (unsigned int) numChannels);
    jassert (destStartSample >= 0 && destStartSample + numSamples <= size);
    jassert (source != 0);

    if (numSamples > 0)
    {
        float* d = channels [destChannel] + destStartSample;

        if (gain != 1.0f)
        {
            if (gain == 0)
            {
                zeromem (d, sizeof (float) * numSamples);
            }
            else
            {
                while (--numSamples >= 0)
                    *d++ = gain * *source++;
            }
        }
        else
        {
            memcpy (d, source, sizeof (float) * numSamples);
        }
    }
}

void AudioSampleBuffer::copyFromWithRamp (const int destChannel,
                                          const int destStartSample,
                                          const float* source,
                                          int numSamples,
                                          float startGain,
                                          float endGain) throw()
{
    jassert (((unsigned int) destChannel) < (unsigned int) numChannels);
    jassert (destStartSample >= 0 && destStartSample + numSamples <= size);
    jassert (source != 0);

    if (startGain == endGain)
    {
        copyFrom (destChannel,
                  destStartSample,
                  source,
                  numSamples,
                  startGain);
    }
    else
    {
        if (numSamples > 0 && (startGain != 0.0f || endGain != 0.0f))
        {
            const float increment = (endGain - startGain) / numSamples;
            float* d = channels [destChannel] + destStartSample;

            while (--numSamples >= 0)
            {
                *d++ = startGain * *source++;
                startGain += increment;
            }
        }
    }
}

void AudioSampleBuffer::findMinMax (const int channel,
                                    const int startSample,
                                    int numSamples,
                                    float& minVal,
                                    float& maxVal) const throw()
{
    jassert (((unsigned int) channel) < (unsigned int) numChannels);
    jassert (startSample >= 0 && startSample + numSamples <= size);

    if (numSamples <= 0)
    {
        minVal = 0.0f;
        maxVal = 0.0f;
    }
    else
    {
        const float* d = channels [channel] + startSample;

        float mn = *d++;
        float mx = mn;

        while (--numSamples > 0) // (> 0 rather than >= 0 because we've already taken the first sample)
        {
            const float samp = *d++;

            if (samp > mx)
                mx = samp;

            if (samp < mn)
                mn = samp;
        }

        maxVal = mx;
        minVal = mn;
    }
}

float AudioSampleBuffer::getMagnitude (const int channel,
                                       const int startSample,
                                       const int numSamples) const throw()
{
    jassert (((unsigned int) channel) < (unsigned int) numChannels);
    jassert (startSample >= 0 && startSample + numSamples <= size);

    float mn, mx;
    findMinMax (channel, startSample, numSamples, mn, mx);

    return jmax (mn, -mn, mx, -mx);
}

float AudioSampleBuffer::getMagnitude (const int startSample,
                                       const int numSamples) const throw()
{
    float mag = 0.0f;

    for (int i = 0; i < numChannels; ++i)
        mag = jmax (mag, getMagnitude (i, startSample, numSamples));

    return mag;
}

float AudioSampleBuffer::getRMSLevel (const int channel,
                                      const int startSample,
                                      const int numSamples) const throw()
{
    jassert (((unsigned int) channel) < (unsigned int) numChannels);
    jassert (startSample >= 0 && startSample + numSamples <= size);

    if (numSamples <= 0 || channel < 0 || channel >= numChannels)
        return 0.0f;

    const float* const data = channels [channel] + startSample;
    double sum = 0.0;

    for (int i = 0; i < numSamples; ++i)
    {
        const float sample = data [i];
        sum += sample * sample;
    }

    return (float) sqrt (sum / numSamples);
}

void AudioSampleBuffer::readFromAudioReader (AudioFormatReader* reader,
                                             const int startSample,
                                             const int numSamples,
                                             const int readerStartSample,
                                             const bool useLeftChan,
                                             const bool useRightChan) throw()
{
    jassert (reader != 0);
    jassert (startSample >= 0 && startSample + numSamples <= size);

    if (numSamples > 0)
    {
        int* chans[3];

        if (useLeftChan == useRightChan)
        {
            chans[0] = (int*) getSampleData (0, startSample);
            chans[1] = (reader->numChannels > 1 && getNumChannels() > 1) ? (int*) getSampleData (1, startSample) : 0;
        }
        else if (useLeftChan || (reader->numChannels == 1))
        {
            chans[0] = (int*) getSampleData (0, startSample);
            chans[1] = 0;
        }
        else if (useRightChan)
        {
            chans[0] = 0;
            chans[1] = (int*) getSampleData (0, startSample);
        }

        chans[2] = 0;

        reader->read (chans, 2, readerStartSample, numSamples, true);

        if (! reader->usesFloatingPointData)
        {
            for (int j = 0; j < 2; ++j)
            {
                float* const d = (float*) (chans[j]);

                if (d != 0)
                {
                    const float multiplier = 1.0f / 0x7fffffff;

                    for (int i = 0; i < numSamples; ++i)
                        d[i] = *(int*)(d + i) * multiplier;
                }
            }
        }

        if (numChannels > 1 && (chans[0] == 0 || chans[1] == 0))
        {
            // if this is a stereo buffer and the source was mono, dupe the first channel..
            memcpy (getSampleData (1, startSample),
                    getSampleData (0, startSample),
                    sizeof (float) * numSamples);
        }
    }
}

void AudioSampleBuffer::writeToAudioWriter (AudioFormatWriter* writer,
                                            const int startSample,
                                            const int numSamples) const throw()
{
    jassert (startSample >= 0 && startSample + numSamples <= size);

    if (numSamples > 0)
    {
        int* chans [3];

        if (writer->isFloatingPoint())
        {
            chans[0] = (int*) getSampleData (0, startSample);

            if (numChannels > 1)
                chans[1] = (int*) getSampleData (1, startSample);
            else
                chans[1] = 0;

            chans[2] = 0;
            writer->write ((const int**) chans, numSamples);
        }
        else
        {
            HeapBlock <int> tempBuffer (numSamples * 2);
            chans[0] = tempBuffer;

            if (numChannels > 1)
                chans[1] = chans[0] + numSamples;
            else
                chans[1] = 0;

            chans[2] = 0;

            for (int j = 0; j < 2; ++j)
            {
                int* const dest = chans[j];

                if (dest != 0)
                {
                    const float* const src = channels [j] + startSample;

                    for (int i = 0; i < numSamples; ++i)
                    {
                        const double samp = src[i];

                        if (samp <= -1.0)
                            dest[i] = INT_MIN;
                        else if (samp >= 1.0)
                            dest[i] = INT_MAX;
                        else
                            dest[i] = roundToInt (INT_MAX * samp);
                    }
                }
            }

            writer->write ((const int**) chans, numSamples);
        }
    }
}

END_JUCE_NAMESPACE
