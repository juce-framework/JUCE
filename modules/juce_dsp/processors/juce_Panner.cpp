/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{
namespace dsp
{

//==============================================================================
template <typename SampleType>
Panner<SampleType>::Panner()
{
    update();
    reset();
}

//==============================================================================
template <typename SampleType>
void Panner<SampleType>::setRule (Rule newRule)
{
    currentRule = newRule;
    update();
}

template <typename SampleType>
void Panner<SampleType>::setPan (SampleType newPan)
{
    jassert (newPan >= -1.0 && newPan <= 1.0);

    pan = jlimit (static_cast<SampleType> (-1.0), static_cast<SampleType> (1.0), newPan);
    update();
}

//==============================================================================
template <typename SampleType>
void Panner<SampleType>::prepare (const ProcessSpec& spec)
{
    jassert (spec.sampleRate > 0);
    jassert (spec.numChannels > 0);

    sampleRate = spec.sampleRate;

    reset();
}

template <typename SampleType>
void Panner<SampleType>::reset()
{
    leftVolume .reset (sampleRate, 0.05);
    rightVolume.reset (sampleRate, 0.05);
}

//==============================================================================
template <typename SampleType>
void Panner<SampleType>::update()
{
    SampleType leftValue, rightValue, boostValue;

    auto normalisedPan = static_cast<SampleType> (0.5) * (pan + static_cast<SampleType> (1.0));

    switch (currentRule)
    {
        case Rule::balanced:
            leftValue  = jmin (static_cast<SampleType> (0.5), static_cast<SampleType> (1.0) - normalisedPan);
            rightValue = jmin (static_cast<SampleType> (0.5), normalisedPan);
            boostValue = static_cast<SampleType> (2.0);
            break;

        case Rule::linear:
            leftValue  = static_cast<SampleType> (1.0) - normalisedPan;
            rightValue = normalisedPan;
            boostValue = static_cast<SampleType> (2.0);
            break;

        case Rule::sin3dB:
            leftValue  = static_cast<SampleType> (std::sin (0.5 * MathConstants<double>::pi * (1.0 - normalisedPan)));
            rightValue = static_cast<SampleType> (std::sin (0.5 * MathConstants<double>::pi * normalisedPan));
            boostValue = std::sqrt (static_cast<SampleType> (2.0));
            break;

        case Rule::sin4p5dB:
            leftValue  = static_cast<SampleType> (std::pow (std::sin (0.5 * MathConstants<double>::pi * (1.0 - normalisedPan)), 1.5));
            rightValue = static_cast<SampleType> (std::pow (std::sin (0.5 * MathConstants<double>::pi * normalisedPan), 1.5));
            boostValue = static_cast<SampleType> (std::pow (2.0, 3.0 / 4.0));
            break;

        case Rule::sin6dB:
            leftValue  = static_cast<SampleType> (std::pow (std::sin (0.5 * MathConstants<double>::pi * (1.0 - normalisedPan)), 2.0));
            rightValue = static_cast<SampleType> (std::pow (std::sin (0.5 * MathConstants<double>::pi * normalisedPan), 2.0));
            boostValue = static_cast<SampleType> (2.0);
            break;

        case Rule::squareRoot3dB:
            leftValue  = std::sqrt (static_cast<SampleType> (1.0) - normalisedPan);
            rightValue = std::sqrt (normalisedPan);
            boostValue = std::sqrt (static_cast<SampleType> (2.0));
            break;

        case Rule::squareRoot4p5dB:
            leftValue  = static_cast<SampleType> (std::pow (std::sqrt (1.0 - normalisedPan), 1.5));
            rightValue = static_cast<SampleType> (std::pow (std::sqrt (normalisedPan), 1.5));
            boostValue = static_cast<SampleType> (std::pow (2.0, 3.0 / 4.0));
            break;

        default:
            leftValue  = jmin (static_cast<SampleType> (0.5), static_cast<SampleType> (1.0) - normalisedPan);
            rightValue = jmin (static_cast<SampleType> (0.5), normalisedPan);
            boostValue = static_cast<SampleType> (2.0);
            break;
    }

    leftVolume .setTargetValue (leftValue  * boostValue);
    rightVolume.setTargetValue (rightValue * boostValue);
}

//==============================================================================
template class Panner<float>;
template class Panner<double>;

} // namespace dsp
} // namespace juce
