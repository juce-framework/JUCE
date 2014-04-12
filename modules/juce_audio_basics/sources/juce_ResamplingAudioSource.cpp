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

ResamplingAudioSource::ResamplingAudioSource (AudioSource* const inputSource,
                                              const bool deleteInputWhenDeleted,
                                              const int numChannels_)
    : input (inputSource, deleteInputWhenDeleted),
      ratio (1.0),
      lastRatio (1.0),
      bufferPos (0),
      sampsInBuffer (0),
      subSampleOffset (0),
      numChannels (numChannels_)
{
    jassert (input != nullptr);
    zeromem (coefficients, sizeof (coefficients));
}

ResamplingAudioSource::~ResamplingAudioSource() {}

void ResamplingAudioSource::setResamplingRatio (const double samplesInPerOutputSample)
{
    jassert (samplesInPerOutputSample > 0);

    const SpinLock::ScopedLockType sl (ratioLock);
    ratio = jmax (0.0, samplesInPerOutputSample);
}

void ResamplingAudioSource::prepareToPlay (int samplesPerBlockExpected,
                                           double sampleRate)
{
    const SpinLock::ScopedLockType sl (ratioLock);

    input->prepareToPlay (samplesPerBlockExpected, sampleRate);

    buffer.setSize (numChannels, roundToInt (samplesPerBlockExpected * ratio) + 32);
    buffer.clear();
    sampsInBuffer = 0;
    bufferPos = 0;
    subSampleOffset = 0.0;

    filterStates.calloc ((size_t) numChannels);
    srcBuffers.calloc ((size_t) numChannels);
    destBuffers.calloc ((size_t) numChannels);
    createLowPass (ratio);
    resetFilters();
}

void ResamplingAudioSource::releaseResources()
{
    input->releaseResources();
    buffer.setSize (numChannels, 0);
}

void ResamplingAudioSource::getNextAudioBlock (const AudioSourceChannelInfo& info)
{
    double localRatio;

    {
        const SpinLock::ScopedLockType sl (ratioLock);
        localRatio = ratio;
    }

    if (lastRatio != localRatio)
    {
        createLowPass (localRatio);
        lastRatio = localRatio;
    }

    const int sampsNeeded = roundToInt (info.numSamples * localRatio) + 2;

    int bufferSize = buffer.getNumSamples();

    if (bufferSize < sampsNeeded + 8)
    {
        bufferPos %= bufferSize;
        bufferSize = sampsNeeded + 32;
        buffer.setSize (buffer.getNumChannels(), bufferSize, true, true);
    }

    bufferPos %= bufferSize;

    int endOfBufferPos = bufferPos + sampsInBuffer;
    const int channelsToProcess = jmin (numChannels, info.buffer->getNumChannels());

    while (sampsNeeded > sampsInBuffer)
    {
        endOfBufferPos %= bufferSize;

        int numToDo = jmin (sampsNeeded - sampsInBuffer,
                            bufferSize - endOfBufferPos);

        AudioSourceChannelInfo readInfo (&buffer, endOfBufferPos, numToDo);
        input->getNextAudioBlock (readInfo);

        if (localRatio > 1.0001)
        {
            // for down-sampling, pre-apply the filter..

            for (int i = channelsToProcess; --i >= 0;)
                applyFilter (buffer.getWritePointer (i, endOfBufferPos), numToDo, filterStates[i]);
        }

        sampsInBuffer += numToDo;
        endOfBufferPos += numToDo;
    }

    for (int channel = 0; channel < channelsToProcess; ++channel)
    {
        destBuffers[channel] = info.buffer->getWritePointer (channel, info.startSample);
        srcBuffers[channel] = buffer.getReadPointer (channel);
    }

    int nextPos = (bufferPos + 1) % bufferSize;
    for (int m = info.numSamples; --m >= 0;)
    {
        const float alpha = (float) subSampleOffset;

        for (int channel = 0; channel < channelsToProcess; ++channel)
            *destBuffers[channel]++ = srcBuffers[channel][bufferPos]
                                        + alpha * (srcBuffers[channel][nextPos] - srcBuffers[channel][bufferPos]);

        subSampleOffset += localRatio;

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

    if (localRatio < 0.9999)
    {
        // for up-sampling, apply the filter after transposing..
        for (int i = channelsToProcess; --i >= 0;)
            applyFilter (info.buffer->getWritePointer (i, info.startSample), info.numSamples, filterStates[i]);
    }
    else if (localRatio <= 1.0001 && info.numSamples > 0)
    {
        // if the filter's not currently being applied, keep it stoked with the last couple of samples to avoid discontinuities
        for (int i = channelsToProcess; --i >= 0;)
        {
            const float* const endOfBuffer = info.buffer->getReadPointer (i, info.startSample + info.numSamples - 1);
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

    const double n = 1.0 / std::tan (double_Pi * jmax (0.001, proportionalRate));
    const double nSquared = n * n;
    const double c1 = 1.0 / (1.0 + std::sqrt (2.0) * n + nSquared);

    setFilterCoefficients (c1,
                           c1 * 2.0f,
                           c1,
                           1.0,
                           c1 * 2.0 * (1.0 - nSquared),
                           c1 * (1.0 - std::sqrt (2.0) * n + nSquared));
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
    filterStates.clear ((size_t) numChannels);
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
