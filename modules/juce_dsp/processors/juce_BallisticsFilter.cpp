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

namespace juce::dsp
{

//==============================================================================
template <typename SampleType>
BallisticsFilter<SampleType>::BallisticsFilter()
{
    setAttackTime (attackTime);
    setReleaseTime (releaseTime);
}

template <typename SampleType>
void BallisticsFilter<SampleType>::setAttackTime (SampleType attackTimeMs)
{
    attackTime = attackTimeMs;
    cteAT = calculateLimitedCte (static_cast<SampleType> (attackTime));
}

template <typename SampleType>
void BallisticsFilter<SampleType>::setReleaseTime (SampleType releaseTimeMs)
{
    releaseTime = releaseTimeMs;
    cteRL = calculateLimitedCte (static_cast<SampleType> (releaseTime));
}

template <typename SampleType>
void BallisticsFilter<SampleType>::setLevelCalculationType (LevelCalculationType newLevelType)
{
    levelType = newLevelType;
    reset();
}

template <typename SampleType>
void BallisticsFilter<SampleType>::prepare (const ProcessSpec& spec)
{
    jassert (spec.sampleRate > 0);
    jassert (spec.numChannels > 0);

    sampleRate = spec.sampleRate;
    expFactor  = -2.0 * MathConstants<double>::pi * 1000.0 / sampleRate;

    setAttackTime  (attackTime);
    setReleaseTime (releaseTime);

    yold.resize (spec.numChannels);

    reset();
}

template <typename SampleType>
void BallisticsFilter<SampleType>::reset()
{
    reset (0);
}

template <typename SampleType>
void BallisticsFilter<SampleType>::reset (SampleType initialValue)
{
    for (auto& old : yold)
        old = initialValue;
}

template <typename SampleType>
SampleType BallisticsFilter<SampleType>::processSample (int channel, SampleType inputValue)
{
    jassert (isPositiveAndBelow (channel, yold.size()));

    if (levelType == LevelCalculationType::RMS)
        inputValue *= inputValue;
    else
        inputValue = std::abs (inputValue);

    SampleType cte = (inputValue > yold[(size_t) channel] ? cteAT : cteRL);

    SampleType result = inputValue + cte * (yold[(size_t) channel] - inputValue);
    yold[(size_t) channel] = result;

    if (levelType == LevelCalculationType::RMS)
        return std::sqrt (result);

    return result;
}

template <typename SampleType>
void BallisticsFilter<SampleType>::snapToZero() noexcept
{
    for (auto& old : yold)
        util::snapToZero (old);
}

template <typename SampleType>
SampleType BallisticsFilter<SampleType>::calculateLimitedCte (SampleType timeMs) const noexcept
{
    return timeMs < static_cast<SampleType> (1.0e-3) ? 0
                                                     : static_cast<SampleType> (std::exp (expFactor / timeMs));
}

//==============================================================================
template class BallisticsFilter<float>;
template class BallisticsFilter<double>;

} // namespace juce::dsp
