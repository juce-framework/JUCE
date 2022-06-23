/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
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

template <typename NumericType>
double FIR::Coefficients<NumericType>::Coefficients::getMagnitudeForFrequency (double frequency, double theSampleRate) const noexcept
{
    jassert (theSampleRate > 0.0);
    jassert (frequency >= 0.0 && frequency <= theSampleRate * 0.5);

    constexpr Complex<double> j (0, 1);
    auto order = getFilterOrder();

    Complex<double> numerator = 0.0, factor = 1.0;
    Complex<double> jw = std::exp (-MathConstants<double>::twoPi * frequency * j / theSampleRate);

    const auto* coefs = coefficients.begin();

    for (size_t n = 0; n <= order; ++n)
    {
        numerator += static_cast<double> (coefs[n]) * factor;
        factor *= jw;
    }

    return std::abs (numerator);
}

//==============================================================================
template <typename NumericType>
void FIR::Coefficients<NumericType>::Coefficients::getMagnitudeForFrequencyArray (double* frequencies, double* magnitudes,
                                                                        size_t numSamples, double theSampleRate) const noexcept
{
    jassert (theSampleRate > 0.0);

    constexpr Complex<double> j (0, 1);
    const auto* coefs = coefficients.begin();
    auto order = getFilterOrder();

    for (size_t i = 0; i < numSamples; ++i)
    {
        jassert (frequencies[i] >= 0.0 && frequencies[i] <= theSampleRate * 0.5);

        Complex<double> numerator = 0.0;
        Complex<double> factor = 1.0;
        Complex<double> jw = std::exp (-MathConstants<double>::twoPi * frequencies[i] * j / theSampleRate);

        for (size_t n = 0; n <= order; ++n)
        {
            numerator += static_cast<double> (coefs[n]) * factor;
            factor *= jw;
        }

        magnitudes[i] = std::abs (numerator);
    }
}

//==============================================================================
template <typename NumericType>
double FIR::Coefficients<NumericType>::Coefficients::getPhaseForFrequency (double frequency, double theSampleRate) const noexcept
{
    jassert (theSampleRate > 0.0);
    jassert (frequency >= 0.0 && frequency <= theSampleRate * 0.5);

    constexpr Complex<double> j (0, 1);

    Complex<double> numerator = 0.0;
    Complex<double> factor = 1.0;
    Complex<double> jw = std::exp (-MathConstants<double>::twoPi * frequency * j / theSampleRate);

    const auto* coefs = coefficients.begin();
    auto order = getFilterOrder();

    for (size_t n = 0; n <= order; ++n)
    {
        numerator += static_cast<double> (coefs[n]) * factor;
        factor *= jw;
    }

    return std::arg (numerator);
}

//==============================================================================
template <typename NumericType>
void FIR::Coefficients<NumericType>::Coefficients::getPhaseForFrequencyArray (double* frequencies, double* phases,
                                                                    size_t numSamples, double theSampleRate) const noexcept
{
    jassert (theSampleRate > 0.0);

    constexpr Complex<double> j (0, 1);
    const auto* coefs = coefficients.begin();
    auto order = getFilterOrder();

    for (size_t i = 0; i < numSamples; ++i)
    {
        jassert (frequencies[i] >= 0.0 && frequencies[i] <= theSampleRate * 0.5);

        Complex<double> numerator = 0.0, factor = 1.0;
        Complex<double> jw = std::exp (-MathConstants<double>::twoPi * frequencies[i] * j / theSampleRate);

        for (size_t n = 0; n <= order; ++n)
        {
            numerator += static_cast<double> (coefs[n]) * factor;
            factor *= jw;
        }

        phases[i] = std::arg (numerator);
    }
}

//==============================================================================
template <typename NumericType>
void FIR::Coefficients<NumericType>::Coefficients::normalise() noexcept
{
    auto magnitude = static_cast<NumericType> (0);

    auto* coefs = coefficients.getRawDataPointer();
    auto n = static_cast<size_t> (coefficients.size());

    for (size_t i = 0; i < n; ++i)
    {
        auto c = coefs[i];
        magnitude += c * c;
    }

    auto magnitudeInv = 1 / (4 * std::sqrt (magnitude));

    FloatVectorOperations::multiply (coefs, magnitudeInv, static_cast<int> (n));
}

//==============================================================================
template struct FIR::Coefficients<float>;
template struct FIR::Coefficients<double>;

} // namespace dsp
} // namespace juce
