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

} // namespace juce::dsp
