/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

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
template <typename Type>
LadderFilter<Type>::LadderFilter()  : state (2)
{
    setSampleRate (Type (1000));    // intentionally setting unrealistic default
                                    // sample rate to catch missing initialisation bugs
    setResonance (Type (0));
    setDrive (Type (1.2));
    setMode (Mode::LPF12);
}

//==============================================================================
template <typename Type>
void LadderFilter<Type>::setMode (Mode newValue) noexcept
{
    switch (newValue)
    {
        case Mode::LPF12:   A = {{ Type (0), Type (0),  Type (1), Type (0),  Type (0) }}; comp = Type (0.5);  break;
        case Mode::HPF12:   A = {{ Type (1), Type (-2), Type (1), Type (0),  Type (0) }}; comp = Type (0);    break;
        case Mode::LPF24:   A = {{ Type (0), Type (0),  Type (0), Type (0),  Type (1) }}; comp = Type (0.5);  break;
        case Mode::HPF24:   A = {{ Type (1), Type (-4), Type (6), Type (-4), Type (1) }}; comp = Type (0);    break;
        default:            jassertfalse;                                                                     break;
    }

    static constexpr auto outputGain = Type (1.2);

    for (auto& a : A)
        a *= outputGain;

    mode = newValue;
    reset();
}

//==============================================================================
template <typename Type>
void LadderFilter<Type>::prepare (const juce::dsp::ProcessSpec& spec)
{
    setSampleRate (Type (spec.sampleRate));
    setNumChannels (spec.numChannels);
    reset();
}

//==============================================================================
template <typename Type>
void LadderFilter<Type>::reset() noexcept
{
    for (auto& s : state)
        s.fill (Type (0));

    cutoffTransformSmoother.setCurrentAndTargetValue (cutoffTransformSmoother.getTargetValue());
    scaledResonanceSmoother.setCurrentAndTargetValue (scaledResonanceSmoother.getTargetValue());
}

//==============================================================================
template <typename Type>
void LadderFilter<Type>::setCutoffFrequencyHz (Type newValue) noexcept
{
    jassert (newValue > Type (0));
    cutoffFreqHz = newValue;
    updateCutoffFreq();
}

//==============================================================================
template <typename Type>
void LadderFilter<Type>::setResonance (Type newValue) noexcept
{
    jassert (newValue >= Type (0) && newValue <= Type (1));
    resonance = newValue;
    updateResonance();
}

//==============================================================================
template <typename Type>
void LadderFilter<Type>::setDrive (Type newValue) noexcept
{
    jassert (newValue >= Type (1));

    drive = newValue;
    gain = std::pow (drive, Type (-2.642))   * Type (0.6103) + Type (0.3903);
    drive2 = drive                           * Type (0.04)   + Type (0.96);
    gain2 = std::pow (drive2, Type (-2.642)) * Type (0.6103) + Type (0.3903);
}

//==============================================================================
template <typename Type>
Type LadderFilter<Type>::processSample (Type inputValue, size_t channelToUse) noexcept
{
    auto& s = state[channelToUse];

    const auto a1 = cutoffTransformValue;
    const auto g =  a1 * Type (-1) + Type (1);
    const auto b0 = g * Type (0.76923076923);
    const auto b1 = g * Type (0.23076923076);

    const auto dx = gain * saturationLUT (drive * inputValue);
    const auto a = dx + scaledResonanceValue * Type (-4) * (gain2 * saturationLUT (drive2 * s[4]) - dx * comp);

    const auto b = b1 * s[0] + a1 * s[1] + b0 * a;
    const auto c = b1 * s[1] + a1 * s[2] + b0 * b;
    const auto d = b1 * s[2] + a1 * s[3] + b0 * c;
    const auto e = b1 * s[3] + a1 * s[4] + b0 * d;

    s[0] = a;
    s[1] = b;
    s[2] = c;
    s[3] = d;
    s[4] = e;

    return a * A[0] + b * A[1] + c * A[2] + d * A[3] + e * A[4];
}

//==============================================================================
template <typename Type>
void LadderFilter<Type>::updateSmoothers() noexcept
{
    cutoffTransformValue = cutoffTransformSmoother.getNextValue();
    scaledResonanceValue = scaledResonanceSmoother.getNextValue();
}

//==============================================================================
template <typename Type>
void LadderFilter<Type>::setSampleRate (Type newValue) noexcept
{
    jassert (newValue > Type (0));
    cutoffFreqScaler = Type (-2.0 * juce::MathConstants<double>::pi) / newValue;

    static constexpr Type smootherRampTimeSec = Type (0.05);
    cutoffTransformSmoother.reset (newValue, smootherRampTimeSec);
    scaledResonanceSmoother.reset (newValue, smootherRampTimeSec);

    updateCutoffFreq();
}

//==============================================================================
template class LadderFilter<float>;
template class LadderFilter<double>;

} // namespace dsp
} // namespace juce
