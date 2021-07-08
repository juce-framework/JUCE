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
namespace IIR
{

template <typename NumericType>
std::array<NumericType, 4> ArrayCoefficients<NumericType>::makeFirstOrderLowPass (double sampleRate,
                                                                                  NumericType frequency)
{
    jassert (sampleRate > 0.0);
    jassert (frequency > 0 && frequency <= static_cast<float> (sampleRate * 0.5));

    auto n = std::tan (MathConstants<NumericType>::pi * frequency / static_cast<NumericType> (sampleRate));

    return { { n, n, n + 1, n - 1 } };
}

template <typename NumericType>
std::array<NumericType, 4> ArrayCoefficients<NumericType>::makeFirstOrderHighPass (double sampleRate,
                                                                                   NumericType frequency)
{
    jassert (sampleRate > 0.0);
    jassert (frequency > 0 && frequency <= static_cast<float> (sampleRate * 0.5));

    auto n = std::tan (MathConstants<NumericType>::pi * frequency / static_cast<NumericType> (sampleRate));

    return { { 1, -1, n + 1, n - 1 } };
}

template <typename NumericType>
std::array<NumericType, 4> ArrayCoefficients<NumericType>::makeFirstOrderAllPass (double sampleRate,
                                                                                  NumericType frequency)
{
    jassert (sampleRate > 0.0);
    jassert (frequency > 0 && frequency <= static_cast<float> (sampleRate * 0.5));

    auto n = std::tan (MathConstants<NumericType>::pi * frequency / static_cast<NumericType> (sampleRate));

    return { { n - 1, n + 1, n + 1, n - 1 } };
}

template <typename NumericType>
std::array<NumericType, 6> ArrayCoefficients<NumericType>::makeLowPass (double sampleRate,
                                                                        NumericType frequency)
{
    return makeLowPass (sampleRate, frequency, inverseRootTwo);
}

template <typename NumericType>
std::array<NumericType, 6> ArrayCoefficients<NumericType>::makeLowPass (double sampleRate,
                                                                        NumericType frequency,
                                                                        NumericType Q)
{
    jassert (sampleRate > 0.0);
    jassert (frequency > 0 && frequency <= static_cast<float> (sampleRate * 0.5));
    jassert (Q > 0.0);

    auto n = 1 / std::tan (MathConstants<NumericType>::pi * frequency / static_cast<NumericType> (sampleRate));
    auto nSquared = n * n;
    auto invQ = 1 / Q;
    auto c1 = 1 / (1 + invQ * n + nSquared);

    return { { c1, c1 * 2, c1,
               1, c1 * 2 * (1 - nSquared),
               c1 * (1 - invQ * n + nSquared) } };
}

template <typename NumericType>
std::array<NumericType, 6> ArrayCoefficients<NumericType>::makeHighPass (double sampleRate,
                                                                         NumericType frequency)
{
    return makeHighPass (sampleRate, frequency, inverseRootTwo);
}

template <typename NumericType>
std::array<NumericType, 6> ArrayCoefficients<NumericType>::makeHighPass (double sampleRate,
                                                                         NumericType frequency,
                                                                         NumericType Q)
{
    jassert (sampleRate > 0.0);
    jassert (frequency > 0 && frequency <= static_cast<float> (sampleRate * 0.5));
    jassert (Q > 0.0);

    auto n = std::tan (MathConstants<NumericType>::pi * frequency / static_cast<NumericType> (sampleRate));
    auto nSquared = n * n;
    auto invQ = 1 / Q;
    auto c1 = 1 / (1 + invQ * n + nSquared);

    return { { c1, c1 * -2, c1,
               1, c1 * 2 * (nSquared - 1),
               c1 * (1 - invQ * n + nSquared) } };
}

template <typename NumericType>
std::array<NumericType, 6> ArrayCoefficients<NumericType>::makeBandPass (double sampleRate,
                                                                         NumericType frequency)
{
    return makeBandPass (sampleRate, frequency, inverseRootTwo);
}

template <typename NumericType>
std::array<NumericType, 6> ArrayCoefficients<NumericType>::makeBandPass (double sampleRate,
                                                                         NumericType frequency,
                                                                         NumericType Q)
{
    jassert (sampleRate > 0.0);
    jassert (frequency > 0 && frequency <= static_cast<float> (sampleRate * 0.5));
    jassert (Q > 0.0);

    auto n = 1 / std::tan (MathConstants<NumericType>::pi * frequency / static_cast<NumericType> (sampleRate));
    auto nSquared = n * n;
    auto invQ = 1 / Q;
    auto c1 = 1 / (1 + invQ * n + nSquared);

    return { { c1 * n * invQ, 0,
               -c1 * n * invQ, 1,
               c1 * 2 * (1 - nSquared),
               c1 * (1 - invQ * n + nSquared) } };
}

template <typename NumericType>
std::array<NumericType, 6> ArrayCoefficients<NumericType>::makeNotch (double sampleRate,
                                                                      NumericType frequency)
{
    return makeNotch (sampleRate, frequency, inverseRootTwo);
}

template <typename NumericType>
std::array<NumericType, 6> ArrayCoefficients<NumericType>::makeNotch (double sampleRate,
                                                                      NumericType frequency,
                                                                      NumericType Q)
{
    jassert (sampleRate > 0.0);
    jassert (frequency > 0 && frequency <= static_cast<float> (sampleRate * 0.5));
    jassert (Q > 0.0);

    auto n = 1 / std::tan (MathConstants<NumericType>::pi * frequency / static_cast<NumericType> (sampleRate));
    auto nSquared = n * n;
    auto invQ = 1 / Q;
    auto c1 = 1 / (1 + n * invQ + nSquared);
    auto b0 = c1 * (1 + nSquared);
    auto b1 = 2 * c1 * (1 - nSquared);

    return { { b0, b1, b0, 1, b1, c1 * (1 - n * invQ + nSquared) } };
}

template <typename NumericType>
std::array<NumericType, 6> ArrayCoefficients<NumericType>::makeAllPass (double sampleRate,
                                                                        NumericType frequency)
{
    return makeAllPass (sampleRate, frequency, inverseRootTwo);
}

template <typename NumericType>
std::array<NumericType, 6> ArrayCoefficients<NumericType>::makeAllPass (double sampleRate,
                                                                        NumericType frequency,
                                                                        NumericType Q)
{
    jassert (sampleRate > 0);
    jassert (frequency > 0 && frequency <= sampleRate * 0.5);
    jassert (Q > 0);

    auto n = 1 / std::tan (MathConstants<NumericType>::pi * frequency / static_cast<NumericType> (sampleRate));
    auto nSquared = n * n;
    auto invQ = 1 / Q;
    auto c1 = 1 / (1 + invQ * n + nSquared);
    auto b0 = c1 * (1 - n * invQ + nSquared);
    auto b1 = c1 * 2 * (1 - nSquared);

    return { { b0, b1, 1, 1, b1, b0 } };
}

template <typename NumericType>
std::array<NumericType, 6> ArrayCoefficients<NumericType>::makeLowShelf (double sampleRate,
                                                                         NumericType cutOffFrequency,
                                                                         NumericType Q,
                                                                         NumericType gainFactor)
{
    jassert (sampleRate > 0.0);
    jassert (cutOffFrequency > 0.0 && cutOffFrequency <= sampleRate * 0.5);
    jassert (Q > 0.0);

    auto A = jmax (static_cast<NumericType> (0.0), std::sqrt (gainFactor));
    auto aminus1 = A - 1;
    auto aplus1 = A + 1;
    auto omega = (2 * MathConstants<NumericType>::pi * jmax (cutOffFrequency, static_cast<NumericType> (2.0))) / static_cast<NumericType> (sampleRate);
    auto coso = std::cos (omega);
    auto beta = std::sin (omega) * std::sqrt (A) / Q;
    auto aminus1TimesCoso = aminus1 * coso;

    return { { A * (aplus1 - aminus1TimesCoso + beta),
               A * 2 * (aminus1 - aplus1 * coso),
               A * (aplus1 - aminus1TimesCoso - beta),
               aplus1 + aminus1TimesCoso + beta,
               -2 * (aminus1 + aplus1 * coso),
               aplus1 + aminus1TimesCoso - beta } };
}

template <typename NumericType>
std::array<NumericType, 6> ArrayCoefficients<NumericType>::makeHighShelf (double sampleRate,
                                                                          NumericType cutOffFrequency,
                                                                          NumericType Q,
                                                                          NumericType gainFactor)
{
    jassert (sampleRate > 0);
    jassert (cutOffFrequency > 0 && cutOffFrequency <= static_cast<NumericType> (sampleRate * 0.5));
    jassert (Q > 0);

    auto A = jmax (static_cast<NumericType> (0.0), std::sqrt (gainFactor));
    auto aminus1 = A - 1;
    auto aplus1 = A + 1;
    auto omega = (2 * MathConstants<NumericType>::pi * jmax (cutOffFrequency, static_cast<NumericType> (2.0))) / static_cast<NumericType> (sampleRate);
    auto coso = std::cos (omega);
    auto beta = std::sin (omega) * std::sqrt (A) / Q;
    auto aminus1TimesCoso = aminus1 * coso;

    return { { A * (aplus1 + aminus1TimesCoso + beta),
               A * -2 * (aminus1 + aplus1 * coso),
               A * (aplus1 + aminus1TimesCoso - beta),
               aplus1 - aminus1TimesCoso + beta,
               2 * (aminus1 - aplus1 * coso),
               aplus1 - aminus1TimesCoso - beta } };
}

template <typename NumericType>
std::array<NumericType, 6> ArrayCoefficients<NumericType>::makePeakFilter (double sampleRate,
                                                                           NumericType frequency,
                                                                           NumericType Q,
                                                                           NumericType gainFactor)
{
    jassert (sampleRate > 0);
    jassert (frequency > 0 && frequency <= static_cast<NumericType> (sampleRate * 0.5));
    jassert (Q > 0);
    jassert (gainFactor > 0);

    auto A = jmax (static_cast<NumericType> (0.0), std::sqrt (gainFactor));
    auto omega = (2 * MathConstants<NumericType>::pi * jmax (frequency, static_cast<NumericType> (2.0))) / static_cast<NumericType> (sampleRate);
    auto alpha = std::sin (omega) / (Q * 2);
    auto c2 = -2 * std::cos (omega);
    auto alphaTimesA = alpha * A;
    auto alphaOverA = alpha / A;

    return { { 1 + alphaTimesA, c2, 1 - alphaTimesA, 1 + alphaOverA, c2, 1 - alphaOverA } };
}

template struct ArrayCoefficients<float>;
template struct ArrayCoefficients<double>;

//==============================================================================
template <typename NumericType>
Coefficients<NumericType>::Coefficients()
{
    assign ({ NumericType(), NumericType(), NumericType(),
              NumericType(), NumericType(), NumericType() });
}

template <typename NumericType>
Coefficients<NumericType>::Coefficients (NumericType b0, NumericType b1,
                                         NumericType a0, NumericType a1)
{
    assign ({ b0, b1,
              a0, a1 });
}

template <typename NumericType>
Coefficients<NumericType>::Coefficients (NumericType b0, NumericType b1, NumericType b2,
                                         NumericType a0, NumericType a1, NumericType a2)
{
    assign ({ b0, b1, b2,
              a0, a1, a2 });
}

template <typename NumericType>
Coefficients<NumericType>::Coefficients (NumericType b0, NumericType b1, NumericType b2, NumericType b3,
                                         NumericType a0, NumericType a1, NumericType a2, NumericType a3)
{
    assign ({ b0, b1, b2, b3,
              a0, a1, a2, a3 });
}

template <typename NumericType>
typename Coefficients<NumericType>::Ptr Coefficients<NumericType>::makeFirstOrderLowPass (double sampleRate,
                                                                                          NumericType frequency)
{
    return *new Coefficients (ArrayCoeffs::makeFirstOrderLowPass (sampleRate, frequency));
}

template <typename NumericType>
typename Coefficients<NumericType>::Ptr Coefficients<NumericType>::makeFirstOrderHighPass (double sampleRate,
                                                                                           NumericType frequency)
{
    return *new Coefficients (ArrayCoeffs::makeFirstOrderHighPass (sampleRate, frequency));
}

template <typename NumericType>
typename Coefficients<NumericType>::Ptr Coefficients<NumericType>::makeFirstOrderAllPass (double sampleRate,
                                                                                          NumericType frequency)
{
    return *new Coefficients (ArrayCoeffs::makeFirstOrderAllPass (sampleRate, frequency));
}

template <typename NumericType>
typename Coefficients<NumericType>::Ptr Coefficients<NumericType>::makeLowPass (double sampleRate,
                                                                                NumericType frequency)
{
    return *new Coefficients (ArrayCoeffs::makeLowPass (sampleRate, frequency));
}

template <typename NumericType>
typename Coefficients<NumericType>::Ptr Coefficients<NumericType>::makeLowPass (double sampleRate,
                                                                                NumericType frequency,
                                                                                NumericType Q)
{
    return *new Coefficients (ArrayCoeffs::makeLowPass (sampleRate, frequency, Q));
}

template <typename NumericType>
typename Coefficients<NumericType>::Ptr Coefficients<NumericType>::makeHighPass (double sampleRate,
                                                                                 NumericType frequency)
{
    return *new Coefficients (ArrayCoeffs::makeHighPass (sampleRate, frequency));
}

template <typename NumericType>
typename Coefficients<NumericType>::Ptr Coefficients<NumericType>::makeHighPass (double sampleRate,
                                                                                 NumericType frequency,
                                                                                 NumericType Q)
{
    return *new Coefficients (ArrayCoeffs::makeHighPass (sampleRate, frequency, Q));
}

template <typename NumericType>
typename Coefficients<NumericType>::Ptr Coefficients<NumericType>::makeBandPass (double sampleRate,
                                                                                 NumericType frequency)
{
    return *new Coefficients (ArrayCoeffs::makeBandPass (sampleRate, frequency));
}

template <typename NumericType>
typename Coefficients<NumericType>::Ptr Coefficients<NumericType>::makeBandPass (double sampleRate,
                                                                                 NumericType frequency,
                                                                                 NumericType Q)
{
    return *new Coefficients (ArrayCoeffs::makeBandPass (sampleRate, frequency, Q));
}

template <typename NumericType>
typename Coefficients<NumericType>::Ptr Coefficients<NumericType>::makeNotch (double sampleRate,
                                                                              NumericType frequency)
{
    return *new Coefficients (ArrayCoeffs::makeNotch (sampleRate, frequency));
}

template <typename NumericType>
typename Coefficients<NumericType>::Ptr Coefficients<NumericType>::makeNotch (double sampleRate,
                                                                              NumericType frequency,
                                                                              NumericType Q)
{
    return *new Coefficients (ArrayCoeffs::makeNotch (sampleRate, frequency, Q));
}

template <typename NumericType>
typename Coefficients<NumericType>::Ptr Coefficients<NumericType>::makeAllPass (double sampleRate,
                                                                                NumericType frequency)
{
    return *new Coefficients (ArrayCoeffs::makeAllPass (sampleRate, frequency));
}

template <typename NumericType>
typename Coefficients<NumericType>::Ptr Coefficients<NumericType>::makeAllPass (double sampleRate,
                                                                                NumericType frequency,
                                                                                NumericType Q)
{
    return *new Coefficients (ArrayCoeffs::makeAllPass (sampleRate, frequency, Q));
}

template <typename NumericType>
typename Coefficients<NumericType>::Ptr Coefficients<NumericType>::makeLowShelf (double sampleRate,
                                                                                 NumericType cutOffFrequency,
                                                                                 NumericType Q,
                                                                                 NumericType gainFactor)
{
    return *new Coefficients (ArrayCoeffs::makeLowShelf (sampleRate, cutOffFrequency, Q, gainFactor));
}

template <typename NumericType>
typename Coefficients<NumericType>::Ptr Coefficients<NumericType>::makeHighShelf (double sampleRate,
                                                                                  NumericType cutOffFrequency,
                                                                                  NumericType Q,
                                                                                  NumericType gainFactor)
{
    return *new Coefficients (ArrayCoeffs::makeHighShelf (sampleRate, cutOffFrequency, Q, gainFactor));
}

template <typename NumericType>
typename Coefficients<NumericType>::Ptr Coefficients<NumericType>::makePeakFilter (double sampleRate,
                                                                                   NumericType frequency,
                                                                                   NumericType Q,
                                                                                   NumericType gainFactor)
{
    return *new Coefficients (ArrayCoeffs::makePeakFilter (sampleRate, frequency, Q, gainFactor));
}

template <typename NumericType>
size_t Coefficients<NumericType>::getFilterOrder() const noexcept
{
    return (static_cast<size_t> (coefficients.size()) - 1) / 2;
}

template <typename NumericType>
double Coefficients<NumericType>::getMagnitudeForFrequency (double frequency, double sampleRate) const noexcept
{
    constexpr Complex<double> j (0, 1);
    const auto order = getFilterOrder();
    const auto* coefs = coefficients.begin();

    jassert (frequency >= 0 && frequency <= sampleRate * 0.5);

    Complex<double> numerator = 0.0, denominator = 0.0, factor = 1.0;
    Complex<double> jw = std::exp (-MathConstants<double>::twoPi * frequency * j / sampleRate);

    for (size_t n = 0; n <= order; ++n)
    {
        numerator += static_cast<double> (coefs[n]) * factor;
        factor *= jw;
    }

    denominator = 1.0;
    factor = jw;

    for (size_t n = order + 1; n <= 2 * order; ++n)
    {
        denominator += static_cast<double> (coefs[n]) * factor;
        factor *= jw;
    }

    return std::abs (numerator / denominator);
}

template <typename NumericType>
void Coefficients<NumericType>::getMagnitudeForFrequencyArray (const double* frequencies, double* magnitudes,
                                                               size_t numSamples, double sampleRate) const noexcept
{
    constexpr Complex<double> j (0, 1);
    const auto order = getFilterOrder();
    const auto* coefs = coefficients.begin();

    jassert (order >= 0);

    for (size_t i = 0; i < numSamples; ++i)
    {
        jassert (frequencies[i] >= 0 && frequencies[i] <= sampleRate * 0.5);

        Complex<double> numerator = 0.0, denominator = 0.0, factor = 1.0;
        Complex<double> jw = std::exp (-MathConstants<double>::twoPi * frequencies[i] * j / sampleRate);

        for (size_t n = 0; n <= order; ++n)
        {
            numerator += static_cast<double> (coefs[n]) * factor;
            factor *= jw;
        }

        denominator = 1.0;
        factor = jw;

        for (size_t n = order + 1; n <= 2 * order; ++n)
        {
            denominator += static_cast<double> (coefs[n]) * factor;
            factor *= jw;
        }

        magnitudes[i] = std::abs(numerator / denominator);
    }
}

template <typename NumericType>
double Coefficients<NumericType>::getPhaseForFrequency (double frequency, double sampleRate) const noexcept
{
    constexpr Complex<double> j (0, 1);
    const auto order = getFilterOrder();
    const auto* coefs = coefficients.begin();

    jassert (frequency >= 0 && frequency <= sampleRate * 0.5);

    Complex<double> numerator = 0.0, denominator = 0.0, factor = 1.0;
    Complex<double> jw = std::exp (-MathConstants<double>::twoPi * frequency * j / sampleRate);

    for (size_t n = 0; n <= order; ++n)
    {
        numerator += static_cast<double> (coefs[n]) * factor;
        factor *= jw;
    }

    denominator = 1.0;
    factor = jw;

    for (size_t n = order + 1; n <= 2 * order; ++n)
    {
        denominator += static_cast<double> (coefs[n]) * factor;
        factor *= jw;
    }

    return std::arg (numerator / denominator);
}

template <typename NumericType>
void Coefficients<NumericType>::getPhaseForFrequencyArray (double* frequencies, double* phases,
                                                           size_t numSamples, double sampleRate) const noexcept
{
    jassert (sampleRate > 0);

    constexpr Complex<double> j (0, 1);
    const auto order = getFilterOrder();
    const auto* coefs = coefficients.begin();
    auto invSampleRate = 1 / sampleRate;

    jassert (order >= 0);

    for (size_t i = 0; i < numSamples; ++i)
    {
        jassert (frequencies[i] >= 0 && frequencies[i] <= sampleRate * 0.5);

        Complex<double> numerator = 0.0, denominator = 0.0, factor = 1.0;
        Complex<double> jw = std::exp (-MathConstants<double>::twoPi * frequencies[i] * j * invSampleRate);

        for (size_t n = 0; n <= order; ++n)
        {
            numerator += static_cast<double> (coefs[n]) * factor;
            factor *= jw;
        }

        denominator = 1.0;
        factor = jw;

        for (size_t n = order + 1; n <= 2 * order; ++n)
        {
            denominator += static_cast<double> (coefs[n]) * factor;
            factor *= jw;
        }

        phases[i] = std::arg (numerator / denominator);
    }
}

template struct Coefficients<float>;
template struct Coefficients<double>;

} // namespace IIR
} // namespace dsp
} // namespace juce
