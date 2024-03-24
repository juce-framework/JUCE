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
void Limiter<SampleType>::setThreshold (SampleType newThreshold)
{
    thresholddB = newThreshold;
    update();
}

template <typename SampleType>
void Limiter<SampleType>::setRelease (SampleType newRelease)
{
    releaseTime = newRelease;
    update();
}

//==============================================================================
template <typename SampleType>
void Limiter<SampleType>::prepare (const ProcessSpec& spec)
{
    jassert (spec.sampleRate > 0);
    jassert (spec.numChannels > 0);

    sampleRate = spec.sampleRate;

    firstStageCompressor.prepare (spec);
    secondStageCompressor.prepare (spec);

    update();
    reset();
}

template <typename SampleType>
void Limiter<SampleType>::reset()
{
    firstStageCompressor.reset();
    secondStageCompressor.reset();

    outputVolume.reset (sampleRate, 0.001);
}

//==============================================================================
template <typename SampleType>
void Limiter<SampleType>::update()
{
    firstStageCompressor.setThreshold ((SampleType) -10.0);
    firstStageCompressor.setRatio     ((SampleType) 4.0);
    firstStageCompressor.setAttack    ((SampleType) 2.0);
    firstStageCompressor.setRelease   ((SampleType) 200.0);

    secondStageCompressor.setThreshold (thresholddB);
    secondStageCompressor.setRatio     ((SampleType) 1000.0);
    secondStageCompressor.setAttack    ((SampleType) 0.001);
    secondStageCompressor.setRelease   (releaseTime);

    auto ratioInverse = (SampleType) (1.0 / 4.0);

    auto gain = (SampleType) std::pow (10.0, 10.0 * (1.0 - ratioInverse) / 40.0);
    gain *= Decibels::decibelsToGain (-thresholddB, (SampleType) -100.0);

    outputVolume.setTargetValue (gain);
}

//==============================================================================
template class Limiter<float>;
template class Limiter<double>;

} // namespace juce::dsp
