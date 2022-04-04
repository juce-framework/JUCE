/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

AudioProcessLoadMeasurer::AudioProcessLoadMeasurer()  = default;
AudioProcessLoadMeasurer::~AudioProcessLoadMeasurer() = default;

void AudioProcessLoadMeasurer::reset()
{
    reset (0, 0);
}

void AudioProcessLoadMeasurer::reset (double sampleRate, int blockSize)
{
    const SpinLock::ScopedLockType lock (mutex);

    cpuUsageProportion = 0;
    xruns = 0;

    samplesPerBlock = blockSize;
    msPerSample = (sampleRate > 0.0 && blockSize > 0) ? 1000.0 / sampleRate : 0;
}

void AudioProcessLoadMeasurer::registerBlockRenderTime (double milliseconds)
{
    const SpinLock::ScopedTryLockType lock (mutex);

    if (lock.isLocked())
        registerRenderTimeLocked (milliseconds, samplesPerBlock);
}

void AudioProcessLoadMeasurer::registerRenderTime (double milliseconds, int numSamples)
{
    const SpinLock::ScopedTryLockType lock (mutex);

    if (lock.isLocked())
        registerRenderTimeLocked (milliseconds, numSamples);
}

void AudioProcessLoadMeasurer::registerRenderTimeLocked (double milliseconds, int numSamples)
{
    if (msPerSample == 0)
        return;

    const auto maxMilliseconds = numSamples * msPerSample;
    const auto usedProportion = milliseconds / maxMilliseconds;
    const auto filterAmount = 0.2;
    const auto proportion = cpuUsageProportion.load();
    cpuUsageProportion = proportion + filterAmount * (usedProportion - proportion);

    if (milliseconds > maxMilliseconds)
        ++xruns;
}

double AudioProcessLoadMeasurer::getLoadAsProportion() const   { return jlimit (0.0, 1.0, cpuUsageProportion.load()); }
double AudioProcessLoadMeasurer::getLoadAsPercentage() const   { return 100.0 * getLoadAsProportion(); }

int AudioProcessLoadMeasurer::getXRunCount() const             { return xruns; }

AudioProcessLoadMeasurer::ScopedTimer::ScopedTimer (AudioProcessLoadMeasurer& p)
    : ScopedTimer (p, p.samplesPerBlock)
{
}

AudioProcessLoadMeasurer::ScopedTimer::ScopedTimer (AudioProcessLoadMeasurer& p, int numSamplesInBlock)
    : owner (p), startTime (Time::getMillisecondCounterHiRes()), samplesInBlock (numSamplesInBlock)
{
    // numSamplesInBlock should never be zero. Did you remember to call AudioProcessLoadMeasurer::reset(),
    // passing the expected samples per block?
    jassert (numSamplesInBlock);
}

AudioProcessLoadMeasurer::ScopedTimer::~ScopedTimer()
{
    owner.registerRenderTime (Time::getMillisecondCounterHiRes() - startTime, samplesInBlock);
}

} // namespace juce
