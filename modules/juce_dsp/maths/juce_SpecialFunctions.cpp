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

double SpecialFunctions::besselI0 (double x) noexcept
{
    auto ax = std::abs (x);

    if (ax < 3.75)
    {
        auto y = x / 3.75;
        y *= y;

        return 1.0 + y * (3.5156229 + y * (3.0899424 + y * (1.2067492
                + y * (0.2659732 + y * (0.360768e-1 + y * 0.45813e-2)))));
    }

    auto y = 3.75 / ax;

    return (std::exp (ax) / std::sqrt (ax))
             * (0.39894228 + y * (0.1328592e-1 + y * (0.225319e-2 + y * (-0.157565e-2 + y * (0.916281e-2
                 + y * (-0.2057706e-1 + y * (0.2635537e-1 + y * (-0.1647633e-1 + y * 0.392377e-2))))))));
}

void SpecialFunctions::ellipticIntegralK (double k, double& K, double& Kp) noexcept
{
    constexpr int M = 4;

    K = MathConstants<double>::halfPi;
    auto lastK = k;

    for (int i = 0; i < M; ++i)
    {
        lastK = std::pow (lastK / (1 + std::sqrt (1 - std::pow (lastK, 2.0))), 2.0);
        K *= 1 + lastK;
    }

    Kp = MathConstants<double>::halfPi;
    auto last = std::sqrt (1 - k * k);

    for (int i = 0; i < M; ++i)
    {
        last = std::pow (last / (1.0 + std::sqrt (1.0 - std::pow (last, 2.0))), 2.0);
        Kp *= 1 + last;
    }
}

Complex<double> SpecialFunctions::cde (Complex<double> u, double k) noexcept
{
    constexpr int M = 4;

    double ke[M + 1];
    double* kei = ke;
    *kei = k;

    for (int i = 0; i < M; ++i)
    {
        auto next = std::pow (*kei / (1.0 + std::sqrt (1.0 - std::pow (*kei, 2.0))), 2.0);
        *++kei = next;
    }

    // NB: the spurious cast to double here is a workaround for a very odd link-time failure
    std::complex<double> last = std::cos (u * (double) MathConstants<double>::halfPi);

    for (int i = M - 1; i >= 0; --i)
        last = (1.0 + ke[i + 1]) / (1.0 / last + ke[i + 1] * last);

    return last;
}

Complex<double> SpecialFunctions::sne (Complex<double> u, double k) noexcept
{
    constexpr int M = 4;

    double ke[M + 1];
    double* kei = ke;
    *kei = k;

    for (int i = 0; i < M; ++i)
    {
        auto next = std::pow (*kei / (1 + std::sqrt (1 - std::pow (*kei, 2.0))), 2.0);
        *++kei = next;
    }

    // NB: the spurious cast to double here is a workaround for a very odd link-time failure
    std::complex<double> last = std::sin (u * (double) MathConstants<double>::halfPi);

    for (int i = M - 1; i >= 0; --i)
        last = (1.0 + ke[i + 1]) / (1.0 / last + ke[i + 1] * last);

    return last;
}

Complex<double> SpecialFunctions::asne (Complex<double> w, double k) noexcept
{
    constexpr int M = 4;

    double ke[M + 1];
    double* kei = ke;
    *kei = k;

    for (int i = 0; i < M; ++i)
    {
        auto next = std::pow (*kei / (1.0 + std::sqrt (1.0 - std::pow (*kei, 2.0))), 2.0);
        *++kei = next;
    }

    std::complex<double> last = w;

    for (int i = 1; i <= M; ++i)
        last = 2.0 * last / ((1.0 + ke[i]) * (1.0 + std::sqrt (1.0 - std::pow (ke[i - 1] * last, 2.0))));

    return 2.0 / MathConstants<double>::pi * std::asin (last);
}

} // namespace dsp
} // namespace juce
