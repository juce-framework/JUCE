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
    if (approximatelyEqual (msPerSample, 0.0))
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
