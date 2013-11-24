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

AudioSampleBuffer::AudioSampleBuffer (const int numChans,
                                      const int numSamples) noexcept
  : numChannels (numChans),
    size (numSamples)
{
    jassert (numSamples >= 0);
    jassert (numChans > 0);

    allocateData();
}

AudioSampleBuffer::AudioSampleBuffer (const AudioSampleBuffer& other) noexcept
  : numChannels (other.numChannels),
    size (other.size),
    allocatedBytes (other.allocatedBytes)
{
    if (allocatedBytes == 0)
    {
        allocateChannels (other.channels, 0);
    }
    else
    {
        allocateData();

        for (int i = 0; i < numChannels; ++i)
            FloatVectorOperations::copy (channels[i], other.channels[i], size);
    }
}

void AudioSampleBuffer::allocateData()
{
    const size_t channelListSize = sizeof (float*) * (size_t) (numChannels + 1);
    allocatedBytes = (size_t) numChannels * (size_t) size * sizeof (float) + channelListSize + 32;
    allocatedData.malloc (allocatedBytes);
    channels = reinterpret_cast <float**> (allocatedData.getData());

    float* chan = (float*) (allocatedData + channelListSize);
    for (int i = 0; i < numChannels; ++i)
    {
        channels[i] = chan;
        chan += size;
    }

    channels [numChannels] = nullptr;
}

AudioSampleBuffer::AudioSampleBuffer (float* const* dataToReferTo,
                                      const int numChans,
                                      const int numSamples) noexcept
    : numChannels (numChans),
      size (numSamples),
      allocatedBytes (0)
{
    jassert (numChans > 0);
    allocateChannels (dataToReferTo, 0);
}

AudioSampleBuffer::AudioSampleBuffer (float* const* dataToReferTo,
                                      const int numChans,
                                      const int startSample,
                                      const int numSamples) noexcept
    : numChannels (numChans),
      size (numSamples),
      allocatedBytes (0)
{
    jassert (numChans > 0);
    allocateChannels (dataToReferTo, startSample);
}

void AudioSampleBuffer::setDataToReferTo (float** dataToReferTo,
                                          const int newNumChannels,
                                          const int newNumSamples) noexcept
{
    jassert (newNumChannels > 0);

    allocatedBytes = 0;
    allocatedData.free();

    numChannels = newNumChannels;
    size = newNumSamples;

    allocateChannels (dataToReferTo, 0);
}

void AudioSampleBuffer::allocateChannels (float* const* const dataToReferTo, int offset)
{
    // (try to avoid doing a malloc here, as that'll blow up things like Pro-Tools)
    if (numChannels < (int) numElementsInArray (preallocatedChannelSpace))
    {
        channels = static_cast <float**> (preallocatedChannelSpace);
    }
    else
    {
        allocatedData.malloc ((size_t) numChannels + 1, sizeof (float*));
        channels = reinterpret_cast <float**> (allocatedData.getData());
    }

    for (int i = 0; i < numChannels; ++i)
    {
        // you have to pass in the same number of valid pointers as numChannels
        jassert (dataToReferTo[i] != nullptr);

        channels[i] = dataToReferTo[i] + offset;
    }

    channels [numChannels] = nullptr;
}

AudioSampleBuffer& AudioSampleBuffer::operator= (const AudioSampleBuffer& other) noexcept
{
    if (this != &other)
    {
        setSize (other.getNumChannels(), other.getNumSamples(), false, false, false);

        for (int i = 0; i < numChannels; ++i)
            FloatVectorOperations::copy (channels[i], other.channels[i], size);
    }

    return *this;
}

AudioSampleBuffer::~AudioSampleBuffer() noexcept
{
}

void AudioSampleBuffer::setSize (const int newNumChannels,
                                 const int newNumSamples,
                                 const bool keepExistingContent,
                                 const bool clearExtraSpace,
                                 const bool avoidReallocating) noexcept
{
    jassert (newNumChannels > 0);
    jassert (newNumSamples >= 0);

    if (newNumSamples != size || newNumChannels != numChannels)
    {
        const size_t allocatedSamplesPerChannel = ((size_t) newNumSamples + 3) & ~3u;
        const size_t channelListSize = ((sizeof (float*) * (size_t) (newNumChannels + 1)) + 15) & ~15u;
        const size_t newTotalBytes = ((size_t) newNumChannels * (size_t) allocatedSamplesPerChannel * sizeof (float))
                                        + channelListSize + 32;

        if (keepExistingContent)
        {
            HeapBlock <char, true> newData;
            newData.allocate (newTotalBytes, clearExtraSpace);

            const size_t numSamplesToCopy = (size_t) jmin (newNumSamples, size);

            float** const newChannels = reinterpret_cast <float**> (newData.getData());
            float* newChan = reinterpret_cast <float*> (newData + channelListSize);

            for (int j = 0; j < newNumChannels; ++j)
            {
                newChannels[j] = newChan;
                newChan += allocatedSamplesPerChannel;
            }

            const int numChansToCopy = jmin (numChannels, newNumChannels);
            for (int i = 0; i < numChansToCopy; ++i)
                FloatVectorOperations::copy (newChannels[i], channels[i], (int) numSamplesToCopy);

            allocatedData.swapWith (newData);
            allocatedBytes = newTotalBytes;
            channels = newChannels;
        }
        else
        {
            if (avoidReallocating && allocatedBytes >= newTotalBytes)
            {
                if (clearExtraSpace)
                    allocatedData.clear (newTotalBytes);
            }
            else
            {
                allocatedBytes = newTotalBytes;
                allocatedData.allocate (newTotalBytes, clearExtraSpace);
                channels = reinterpret_cast <float**> (allocatedData.getData());
            }

            float* chan = reinterpret_cast <float*> (allocatedData + channelListSize);
            for (int i = 0; i < newNumChannels; ++i)
            {
                channels[i] = chan;
                chan += allocatedSamplesPerChannel;
            }
        }

        channels [newNumChannels] = 0;
        size = newNumSamples;
        numChannels = newNumChannels;
    }
}

void AudioSampleBuffer::clear() noexcept
{
    for (int i = 0; i < numChannels; ++i)
        FloatVectorOperations::clear (channels[i], size);
}

void AudioSampleBuffer::clear (const int startSample,
                               const int numSamples) noexcept
{
    jassert (startSample >= 0 && startSample + numSamples <= size);

    for (int i = 0; i < numChannels; ++i)
        FloatVectorOperations::clear (channels[i] + startSample, numSamples);
}

void AudioSampleBuffer::clear (const int channel,
                               const int startSample,
                               const int numSamples) noexcept
{
    jassert (isPositiveAndBelow (channel, numChannels));
    jassert (startSample >= 0 && startSample + numSamples <= size);

    FloatVectorOperations::clear (channels [channel] + startSample, numSamples);
}

void AudioSampleBuffer::applyGain (const int channel,
                                   const int startSample,
                                   int numSamples,
                                   const float gain) noexcept
{
    jassert (isPositiveAndBelow (channel, numChannels));
    jassert (startSample >= 0 && startSample + numSamples <= size);

    if (gain != 1.0f)
    {
        float* const d = channels [channel] + startSample;

        if (gain == 0.0f)
            FloatVectorOperations::clear (d, numSamples);
        else
            FloatVectorOperations::multiply (d, gain, numSamples);
    }
}

void AudioSampleBuffer::applyGainRamp (const int channel,
                                       const int startSample,
                                       int numSamples,
                                       float startGain,
                                       float endGain) noexcept
{
    if (startGain == endGain)
    {
        applyGain (channel, startSample, numSamples, startGain);
    }
    else
    {
        jassert (isPositiveAndBelow (channel, numChannels));
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

void AudioSampleBuffer::applyGain (int startSample, int numSamples, float gain) noexcept
{
    for (int i = 0; i < numChannels; ++i)
        applyGain (i, startSample, numSamples, gain);
}

void AudioSampleBuffer::applyGain (const float gain) noexcept
{
    applyGain (0, size, gain);
}

void AudioSampleBuffer::applyGainRamp (int startSample, int numSamples,
                                       float startGain, float endGain) noexcept
{
    for (int i = 0; i < numChannels; ++i)
        applyGainRamp (i, startSample, numSamples, startGain, endGain);
}

void AudioSampleBuffer::addFrom (const int destChannel,
                                 const int destStartSample,
                                 const AudioSampleBuffer& source,
                                 const int sourceChannel,
                                 const int sourceStartSample,
                                 int numSamples,
                                 const float gain) noexcept
{
    jassert (&source != this || sourceChannel != destChannel);
    jassert (isPositiveAndBelow (destChannel, numChannels));
    jassert (destStartSample >= 0 && destStartSample + numSamples <= size);
    jassert (isPositiveAndBelow (sourceChannel, source.numChannels));
    jassert (sourceStartSample >= 0 && sourceStartSample + numSamples <= source.size);

    if (gain != 0.0f && numSamples > 0)
    {
        float* const d = channels [destChannel] + destStartSample;
        const float* const s  = source.channels [sourceChannel] + sourceStartSample;

        if (gain != 1.0f)
            FloatVectorOperations::addWithMultiply (d, s, gain, numSamples);
        else
            FloatVectorOperations::add (d, s, numSamples);
    }
}

void AudioSampleBuffer::addFrom (const int destChannel,
                                 const int destStartSample,
                                 const float* source,
                                 int numSamples,
                                 const float gain) noexcept
{
    jassert (isPositiveAndBelow (destChannel, numChannels));
    jassert (destStartSample >= 0 && destStartSample + numSamples <= size);
    jassert (source != nullptr);

    if (gain != 0.0f && numSamples > 0)
    {
        float* const d = channels [destChannel] + destStartSample;

        if (gain != 1.0f)
            FloatVectorOperations::addWithMultiply (d, source, gain, numSamples);
        else
            FloatVectorOperations::add (d, source, numSamples);
    }
}

void AudioSampleBuffer::addFromWithRamp (const int destChannel,
                                         const int destStartSample,
                                         const float* source,
                                         int numSamples,
                                         float startGain,
                                         const float endGain) noexcept
{
    jassert (isPositiveAndBelow (destChannel, numChannels));
    jassert (destStartSample >= 0 && destStartSample + numSamples <= size);
    jassert (source != nullptr);

    if (startGain == endGain)
    {
        addFrom (destChannel, destStartSample, source, numSamples, startGain);
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
                                  int numSamples) noexcept
{
    jassert (&source != this || sourceChannel != destChannel);
    jassert (isPositiveAndBelow (destChannel, numChannels));
    jassert (destStartSample >= 0 && destStartSample + numSamples <= size);
    jassert (isPositiveAndBelow (sourceChannel, source.numChannels));
    jassert (sourceStartSample >= 0 && sourceStartSample + numSamples <= source.size);

    if (numSamples > 0)
        FloatVectorOperations::copy (channels [destChannel] + destStartSample,
                                     source.channels [sourceChannel] + sourceStartSample,
                                     numSamples);
}

void AudioSampleBuffer::copyFrom (const int destChannel,
                                  const int destStartSample,
                                  const float* source,
                                  int numSamples) noexcept
{
    jassert (isPositiveAndBelow (destChannel, numChannels));
    jassert (destStartSample >= 0 && destStartSample + numSamples <= size);
    jassert (source != nullptr);

    if (numSamples > 0)
        FloatVectorOperations::copy (channels [destChannel] + destStartSample, source, numSamples);
}

void AudioSampleBuffer::copyFrom (const int destChannel,
                                  const int destStartSample,
                                  const float* source,
                                  int numSamples,
                                  const float gain) noexcept
{
    jassert (isPositiveAndBelow (destChannel, numChannels));
    jassert (destStartSample >= 0 && destStartSample + numSamples <= size);
    jassert (source != nullptr);

    if (numSamples > 0)
    {
        float* d = channels [destChannel] + destStartSample;

        if (gain != 1.0f)
        {
            if (gain == 0)
                FloatVectorOperations::clear (d, numSamples);
            else
                FloatVectorOperations::copyWithMultiply (d, source, gain, numSamples);
        }
        else
        {
            FloatVectorOperations::copy (d, source, numSamples);
        }
    }
}

void AudioSampleBuffer::copyFromWithRamp (const int destChannel,
                                          const int destStartSample,
                                          const float* source,
                                          int numSamples,
                                          float startGain,
                                          float endGain) noexcept
{
    jassert (isPositiveAndBelow (destChannel, numChannels));
    jassert (destStartSample >= 0 && destStartSample + numSamples <= size);
    jassert (source != nullptr);

    if (startGain == endGain)
    {
        copyFrom (destChannel, destStartSample, source, numSamples, startGain);
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

void AudioSampleBuffer::reverse (int channel, int startSample, int numSamples) const noexcept
{
    jassert (isPositiveAndBelow (channel, numChannels));
    jassert (startSample >= 0 && startSample + numSamples <= size);

    std::reverse (channels[channel] + startSample,
                  channels[channel] + startSample + numSamples);
}

void AudioSampleBuffer::reverse (int startSample, int numSamples) const noexcept
{
    for (int i = 0; i < numChannels; ++i)
        reverse (i, startSample, numSamples);
}

void AudioSampleBuffer::findMinMax (const int channel,
                                    const int startSample,
                                    int numSamples,
                                    float& minVal,
                                    float& maxVal) const noexcept
{
    jassert (isPositiveAndBelow (channel, numChannels));
    jassert (startSample >= 0 && startSample + numSamples <= size);

    FloatVectorOperations::findMinAndMax (channels [channel] + startSample,
                                          numSamples, minVal, maxVal);
}

float AudioSampleBuffer::getMagnitude (const int channel,
                                       const int startSample,
                                       const int numSamples) const noexcept
{
    jassert (isPositiveAndBelow (channel, numChannels));
    jassert (startSample >= 0 && startSample + numSamples <= size);

    float mn, mx;
    findMinMax (channel, startSample, numSamples, mn, mx);

    return jmax (mn, -mn, mx, -mx);
}

float AudioSampleBuffer::getMagnitude (int startSample, int numSamples) const noexcept
{
    float mag = 0.0f;

    for (int i = 0; i < numChannels; ++i)
        mag = jmax (mag, getMagnitude (i, startSample, numSamples));

    return mag;
}

float AudioSampleBuffer::getRMSLevel (const int channel,
                                      const int startSample,
                                      const int numSamples) const noexcept
{
    jassert (isPositiveAndBelow (channel, numChannels));
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

    return (float) std::sqrt (sum / numSamples);
}
