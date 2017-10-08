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

template <typename FloatType>
typename FIR::Coefficients<FloatType>::Ptr
    FilterDesign<FloatType>::designFIRLowpassWindowMethod (FloatType frequency,
                                                           double sampleRate, size_t order,
                                                           WindowingMethod type,
                                                           FloatType beta)
{
    jassert (sampleRate > 0);
    jassert (frequency > 0 && frequency <= sampleRate * 0.5);

    auto* result = new typename FIR::Coefficients<FloatType> (order + 1u);

    auto* c = result->getRawCoefficients();
    auto normalizedFrequency = frequency / sampleRate;

    for (size_t i = 0; i <= order; ++i)
    {
        if (i == order * 0.5)
        {
            c[i] = static_cast<FloatType> (normalizedFrequency * 2);
        }
        else
        {
            auto indice = double_Pi * (static_cast<double> (i) - 0.5 * static_cast<double> (order));
            c[i] = static_cast<FloatType> (std::sin (2.0 * indice * normalizedFrequency) / indice);
        }
    }

    WindowingFunction<FloatType> theWindow (order + 1, type, false, beta);
    theWindow.multiplyWithWindowingTable (c, order + 1);

    return result;
}

template <typename FloatType>
typename FIR::Coefficients<FloatType>::Ptr
    FilterDesign<FloatType>::designFIRLowpassKaiserMethod (FloatType frequency, double sampleRate,
                                                           FloatType normalizedTransitionWidth,
                                                           FloatType attenuationdB)
{
    jassert (sampleRate > 0);
    jassert (frequency > 0 && frequency <= sampleRate * 0.5);
    jassert (normalizedTransitionWidth > 0 && normalizedTransitionWidth <= 0.5);
    jassert (attenuationdB >= -100 && attenuationdB <= 0);

    FloatType beta = 0;

    if (attenuationdB < -50)
        beta = static_cast<FloatType> (0.1102 * (-attenuationdB - 8.7));
    else if (attenuationdB <= 21)
        beta = static_cast<FloatType> (0.5842 * std::pow (-attenuationdB - 21, 0.4) + 0.07886 * (-attenuationdB - 21));

    int order = attenuationdB < -21 ? roundDoubleToInt (ceil ((-attenuationdB - 7.95) / (2.285 * normalizedTransitionWidth * 2.0 * double_Pi)))
                                    : roundDoubleToInt (ceil (5.79 / (normalizedTransitionWidth * 2.0 * double_Pi)));

    jassert (order >= 0);

    return designFIRLowpassWindowMethod (frequency, sampleRate, static_cast<size_t> (order),
                                         WindowingFunction<FloatType>::kaiser, beta);
}


template <typename FloatType>
typename FIR::Coefficients<FloatType>::Ptr
    FilterDesign<FloatType>::designFIRLowpassTransitionMethod (FloatType frequency, double sampleRate, size_t order,
                                                               FloatType normalizedTransitionWidth, FloatType spline)
{
    jassert (sampleRate > 0);
    jassert (frequency > 0 && frequency <= sampleRate * 0.5);
    jassert (normalizedTransitionWidth > 0 && normalizedTransitionWidth <= 0.5);
    jassert (spline >= 1.0 && spline <= 4.0);

    auto normalizedFrequency = frequency / static_cast<FloatType> (sampleRate);

    auto* result = new typename FIR::Coefficients<FloatType> (order + 1u);
    auto* c = result->getRawCoefficients();

    for (size_t i = 0; i <= order; ++i)
    {
        if (i == order / 2)
        {
            c[i] = static_cast<FloatType> (2 * normalizedFrequency);
        }
        else
        {
            auto indice  = double_Pi * (i - 0.5 * order);
            auto indice2 = double_Pi * normalizedTransitionWidth * (i - 0.5 * order) / spline;
            c[i] = static_cast<FloatType> (std::sin (2 * indice * normalizedFrequency)
                                            / indice * std::pow (std::sin (indice2) / indice2, spline));
        }
    }

    return result;
}

template <typename FloatType>
typename FIR::Coefficients<FloatType>::Ptr
    FilterDesign<FloatType>::designFIRLowpassLeastSquaresMethod (FloatType frequency,
                                                                 double sampleRate, size_t order,
                                                                 FloatType normalizedTransitionWidth,
                                                                 FloatType stopBandWeight)
{
    jassert (sampleRate > 0);
    jassert (frequency > 0 && frequency <= sampleRate * 0.5);
    jassert (normalizedTransitionWidth > 0 && normalizedTransitionWidth <= 0.5);
    jassert (stopBandWeight >= 1.0 && stopBandWeight <= 100.0);

    auto normalizedFrequency = static_cast<double> (frequency) / sampleRate;

    auto wp = 2.0 * double_Pi * (static_cast<double> (normalizedFrequency - normalizedTransitionWidth / 2.0));
    auto ws = 2.0 * double_Pi * (static_cast<double> (normalizedFrequency + normalizedTransitionWidth / 2.0));

    auto N = order + 1;

    auto* result = new typename FIR::Coefficients<FloatType> (static_cast<size_t> (N));
    auto* c = result->getRawCoefficients();

    if (N % 2 == 1)
    {
        // Type I
        auto M = (N - 1) / 2;

        Matrix<double> b (M + 1, 1),
                       q (2 * M + 1, 1);

        auto sinc = [](double x) { return x == 0 ? 1 : std::sin (x * double_Pi) / (double_Pi * x); };

        auto factorp = wp / double_Pi;
        auto factors = ws / double_Pi;

        for (size_t i = 0; i <= M; ++i)
            b (i, 0) = factorp * sinc (factorp * i);

        q (0, 0) = factorp + stopBandWeight * (1.0 - factors);

        for (size_t i = 1; i <= 2 * M; ++i)
            q (i, 0) = factorp * sinc (factorp * i) - stopBandWeight * factors * sinc (factors * i);

        auto Q1 = Matrix<double>::toeplitz (q, M + 1);
        auto Q2 = Matrix<double>::hankel (q, M + 1, 0);

        Q1 += Q2; Q1 *= 0.5;

        Q1.solve (b);

        c[M] = static_cast<FloatType> (b (0, 0));

        for (size_t i = 1; i <= M; ++i)
        {
            c[M - i] = static_cast<FloatType> (b (i, 0) * 0.5);
            c[M + i] = static_cast<FloatType> (b (i, 0) * 0.5);
        }
    }
    else
    {
        // Type II
        auto M = N / 2;

        Matrix<double> b (M, 1);
        Matrix<double> qp (2 * M, 1);
        Matrix<double> qs (2 * M, 1);

        auto sinc = [](double x) { return x == 0 ? 1 : std::sin (x * double_Pi) / (double_Pi * x); };

        auto factorp = wp / double_Pi;
        auto factors = ws / double_Pi;

        for (size_t i = 0; i < M; ++i)
            b (i, 0) = factorp * sinc (factorp * (i + 0.5));

        for (size_t i = 0; i < 2 * M; ++i)
        {
            qp (i, 0) = 0.25 * factorp * sinc (factorp * i);
            qs (i, 0) = -0.25 * stopBandWeight * factors * sinc (factors * i);
        }

        auto Q1p = Matrix<double>::toeplitz (qp, M);
        auto Q2p = Matrix<double>::hankel (qp, M, 1);
        auto Q1s = Matrix<double>::toeplitz (qs, M);
        auto Q2s = Matrix<double>::hankel (qs, M, 1);

        auto Id = Matrix<double>::identity (M);
        Id *= (0.25 * stopBandWeight);

        Q1p += Q2p;
        Q1s += Q2s;
        Q1s += Id;

        auto& Q = Q1s;
        Q += Q1p;

        Q.solve (b);

        for (size_t i = 0; i < M; ++i)
        {
            c[M - i - 1] = static_cast<FloatType> (b (i, 0) * 0.25);
            c[M + i]     = static_cast<FloatType> (b (i, 0) * 0.25);
        }
    }

    return result;
}

template <typename FloatType>
typename FIR::Coefficients<FloatType>::Ptr
    FilterDesign<FloatType>::designFIRLowpassHalfBandEquirippleMethod (FloatType normalizedTransitionWidth,
                                                                       FloatType attenuationdB)
{
    jassert (normalizedTransitionWidth > 0 && normalizedTransitionWidth <= 0.5);
    jassert (attenuationdB >= -300 && attenuationdB <= -10);

    auto wpT = (0.5 - normalizedTransitionWidth) * double_Pi;

    auto n = roundDoubleToInt (ceil ((attenuationdB - 18.18840664 * wpT + 33.64775300) / (18.54155181 * wpT - 29.13196871)));
    auto kp = (n * wpT - 1.57111377 * n + 0.00665857) / (-1.01927560 * n + 0.37221484);
    auto A = (0.01525753 * n + 0.03682344 + 9.24760314 / (double) n) * kp + 1.01701407 + 0.73512298 / (double) n;
    auto B = (0.00233667 * n - 1.35418408 + 5.75145813 / (double) n) * kp + 1.02999650 - 0.72759508 / (double) n;

    auto hn  = FilterDesign<FloatType>::getPartialImpulseResponseHn (n, kp);
    auto hnm = FilterDesign<FloatType>::getPartialImpulseResponseHn (n - 1, kp);

    auto diff = (hn.size() - hnm.size()) / 2;

    for (int i = 0; i < diff; ++i)
    {
        hnm.add (0.0);
        hnm.insert (0, 0.0);
    }

    auto hh = hn;

    for (int i = 0; i < hn.size(); ++i)
        hh.setUnchecked (i, A * hh[i] + B * hnm[i]);

    auto* result = new typename FIR::Coefficients<FloatType> (static_cast<size_t> (hh.size()));
    auto* c = result->getRawCoefficients();

    for (int i = 0; i < hh.size(); ++i)
        c[i] = (float) hh[i];

    double NN;

    if (n % 2 == 0)
    {
        NN = 2.0 * result->getMagnitudeForFrequency (0.5, 1.0);
    }
    else
    {
        auto w01 = std::sqrt (kp * kp + (1 - kp * kp) * std::pow (std::cos (double_Pi / (2.0 * n + 1.0)), 2.0));
        auto om01 = std::acos (-w01);

        NN = -2.0 * result->getMagnitudeForFrequency (om01 / (2 * double_Pi), 1.0);
    }

    for (int i = 0; i < hh.size(); ++i)
        c[i] = static_cast<FloatType> ((A * hn[i] + B * hnm[i]) / NN);

    c[2 * n + 1] = static_cast<FloatType> (0.5);

    return result;
}

template <typename FloatType>
Array<double> FilterDesign<FloatType>::getPartialImpulseResponseHn (int n, double kp)
{
    Array<double> alpha;
    alpha.resize (2 * n + 1);

    alpha.setUnchecked (2 * n, 1.0 / std::pow (1.0 - kp * kp, n));

    if (n > 0)
        alpha.setUnchecked (2 * n - 2, -(2 * n * kp * kp + 1) * alpha[2 * n]);

    if (n > 1)
        alpha.setUnchecked (2 * n - 4, -(4 * n + 1 + (n - 1) * (2 * n - 1) * kp * kp) / (2.0 * n) * alpha[2 * n - 2]
                             - (2 * n + 1) * ((n + 1) * kp * kp + 1) / (2.0 * n) * alpha[2 * n]);

    for (int k = n; k >= 3; --k)
    {
        auto c1 = (3 * (n*(n + 2) - k * (k - 2)) + 2 * k - 3 + 2 * (k - 2)*(2 * k - 3) * kp * kp) * alpha[2 * k - 4];
        auto c2 = (3 * (n*(n + 2) - (k - 1) * (k + 1)) + 2 * (2 * k - 1) + 2 * k*(2 * k - 1) * kp * kp) * alpha[2 * k - 2];
        auto c3 = (n * (n + 2) - (k - 1) * (k + 1)) * alpha[2 * k];
        auto c4 = (n * (n + 2) - (k - 3) * (k - 1));

        alpha.setUnchecked (2 * k - 6, -(c1 + c2 + c3) / c4);
    }

    Array<double> ai;
    ai.resize (2 * n + 1 + 1);

    for (int k = 0; k <= n; ++k)
        ai.setUnchecked (2 * k + 1, alpha[2 * k] / (2.0 * k + 1.0));

    Array<double> hn;
    hn.resize (2 * n + 1 + 2 * n + 1 + 1);

    for (int k = 0; k <= n; ++k)
    {
        hn.setUnchecked (2 * n + 1 + (2 * k + 1), 0.5 * ai[2 * k + 1]);
        hn.setUnchecked (2 * n + 1 - (2 * k + 1), 0.5 * ai[2 * k + 1]);
    }

    return hn;
}

template <typename FloatType>
Array<IIR::Coefficients<FloatType>>
    FilterDesign<FloatType>::designIIRLowpassHighOrderButterworthMethod (FloatType frequency, double sampleRate,
                                                                         FloatType normalizedTransitionWidth,
                                                                         FloatType passbandAttenuationdB,
                                                                         FloatType stopbandAttenuationdB)
{
    return designIIRLowpassHighOrderGeneralMethod (0, frequency, sampleRate, normalizedTransitionWidth,
                                                   passbandAttenuationdB, stopbandAttenuationdB);
}

template <typename FloatType>
Array<IIR::Coefficients<FloatType>>
    FilterDesign<FloatType>::designIIRLowpassHighOrderChebyshev1Method (FloatType frequency, double sampleRate,
                                                                        FloatType normalizedTransitionWidth,
                                                                        FloatType passbandAttenuationdB,
                                                                        FloatType stopbandAttenuationdB)
{
    return designIIRLowpassHighOrderGeneralMethod (1, frequency, sampleRate, normalizedTransitionWidth,
                                                   passbandAttenuationdB, stopbandAttenuationdB);
}

template <typename FloatType>
Array<IIR::Coefficients<FloatType>>
    FilterDesign<FloatType>::designIIRLowpassHighOrderChebyshev2Method (FloatType frequency, double sampleRate,
                                                                        FloatType normalizedTransitionWidth,
                                                                        FloatType passbandAttenuationdB,
                                                                        FloatType stopbandAttenuationdB)
{
    return designIIRLowpassHighOrderGeneralMethod (2, frequency, sampleRate, normalizedTransitionWidth,
                                                   passbandAttenuationdB, stopbandAttenuationdB);
}

template <typename FloatType>
Array<IIR::Coefficients<FloatType>>
    FilterDesign<FloatType>::designIIRLowpassHighOrderEllipticMethod (FloatType frequency, double sampleRate,
                                                                      FloatType normalizedTransitionWidth,
                                                                      FloatType passbandAttenuationdB,
                                                                      FloatType stopbandAttenuationdB)
{
    return designIIRLowpassHighOrderGeneralMethod (3, frequency, sampleRate, normalizedTransitionWidth,
                                                   passbandAttenuationdB, stopbandAttenuationdB);
}

template <typename FloatType>
Array<IIR::Coefficients<FloatType>>
    FilterDesign<FloatType>::designIIRLowpassHighOrderGeneralMethod (int type, FloatType frequency, double sampleRate,
                                                                     FloatType normalizedTransitionWidth,
                                                                     FloatType passbandAttenuationdB,
                                                                     FloatType stopbandAttenuationdB)
{
    jassert (sampleRate > 0);
    jassert (frequency > 0 && frequency <= sampleRate * 0.5);
    jassert (normalizedTransitionWidth > 0 && normalizedTransitionWidth <= 0.5);
    jassert (passbandAttenuationdB > -20 && passbandAttenuationdB < 0);
    jassert (stopbandAttenuationdB > -300 && stopbandAttenuationdB < -20);

    auto normalizedFrequency = frequency / sampleRate;

    auto fp = normalizedFrequency - normalizedTransitionWidth / 2;
    auto fs = normalizedFrequency + normalizedTransitionWidth / 2;

    double Ap = passbandAttenuationdB;
    double As = stopbandAttenuationdB;
    auto Gp = Decibels::decibelsToGain (Ap, -300.0);
    auto Gs = Decibels::decibelsToGain (As, -300.0);
    auto epsp = std::sqrt (1.0 / (Gp * Gp) - 1.0);
    auto epss = std::sqrt (1.0 / (Gs * Gs) - 1.0);

    auto omegap = std::tan (double_Pi * fp);
    auto omegas = std::tan (double_Pi * fs);

    auto k = omegap / omegas;
    auto k1 = epsp / epss;

    int N;

    if (type == 0)
    {
        N = roundDoubleToInt (ceil (log (1.0 / k1) / log (1.0 / k)));
    }
    else if (type == 1 || type == 2)
    {
        N = roundDoubleToInt (ceil (std::acosh (1.0 / k1) / std::acosh (1.0 / k)));
    }
    else
    {
        double K, Kp, K1, K1p;

        SpecialFunctions::ellipticIntegralK (k, K, Kp);
        SpecialFunctions::ellipticIntegralK (k1, K1, K1p);

        N = roundDoubleToInt (ceil ((K1p * K) / (K1 * Kp)));
    }

    const int r = N % 2;
    const int L = (N - r) / 2;
    const double H0 = (type == 1 || type == 3) ? std::pow (Gp, 1.0 - r) : 1.0;

    Array<Complex<double>> pa, za;
    Complex<double> j (0, 1);

    if (type == 0)
    {
        if (r == 1)
            pa.add (-omegap * std::pow (epsp, -1.0 / (double) N));

        for (int i = 1; i <= L; ++i)
        {
            auto ui = (2 * i - 1.0) / (double) N;
            pa.add (omegap * std::pow (epsp, -1.0 / (double) N) * j * exp (ui * 0.5 * double_Pi * j));
        }
    }
    else if (type == 1)
    {
        auto v0 = std::asinh (1.0 / epsp) / (0.5 * N * double_Pi);

        if (r == 1)
            pa.add (-omegap * std::sinh (v0 * 0.5 * double_Pi));

        for (int i = 1; i <= L; ++i)
        {
            auto ui = (2 * i - 1.0) / (double) N;
            pa.add (omegap * j * std::cos ((ui - j * v0) * 0.5 * double_Pi));
        }
    }
    else if (type == 2)
    {
        auto v0 = std::asinh (epss) / (N * 0.5 * double_Pi);

        if (r == 1)
            pa.add(-1.0 / (k / omegap * std::sinh (v0 * 0.5 * double_Pi)));

        for (int i = 1; i <= L; ++i)
        {
            auto ui = (2 * i - 1.0) / (double) N;

            pa.add (1.0 / (k / omegap * j * std::cos ((ui - j * v0) * 0.5 * double_Pi)));
            za.add (1.0 / (k / omegap * j * std::cos (ui * 0.5 * double_Pi)));
        }
    }
    else
    {
        auto v0 = -j * (SpecialFunctions::asne (j / epsp, k1) / (double) N);

        if (r == 1)
            pa.add (omegap * j * SpecialFunctions::sne (j * v0, k));

        for (int i = 1; i <= L; ++i)
        {
            auto ui = (2 * i - 1.0) / (double) N;
            auto zetai = SpecialFunctions::cde (ui, k);

            pa.add (omegap * j * SpecialFunctions::cde (ui - j * v0, k));
            za.add (omegap * j / (k * zetai));
        }
    }

    Array<Complex<double>> p, z, g;

    if (r == 1)
    {
        p.add ((1.0 + pa[0]) / (1.0 - pa[0]));
        g.add (0.5 * (1.0 - p[0]));
    }

    for (int i = 0; i < L; ++i)
    {
        p.add ((1.0 + pa[i + r]) / (1.0 - pa[i + r]));
        z.add (za.size() == 0 ? -1.0 : (1.0 + za[i]) / (1.0 - za[i]));
        g.add ((1.0 - p[i + r]) / (1.0 - z[i]));
    }

    Array<IIR::Coefficients<FloatType>> theCascadedCoefficients;

    if (r == 1)
    {
        auto b0 = static_cast<FloatType> (H0 * std::real (g[0]));
        auto b1 = b0;
        auto a1 = static_cast<FloatType> (-std::real (p[0]));

        theCascadedCoefficients.add (IIR::Coefficients<FloatType> (b0, b1, 1.f, a1));
    }

    for (int i = 0; i < L; ++i)
    {
        auto gain = std::pow (std::abs (g[i + r]), 2.0);

        auto b0 = static_cast<FloatType> (gain);
        auto b1 = static_cast<FloatType> (std::real (-z[i] - std::conj (z[i])) * gain);
        auto b2 = static_cast<FloatType> (std::real ( z[i] * std::conj (z[i])) * gain);

        auto a1 = static_cast<FloatType> (std::real (-p[i+r] - std::conj (p[i + r])));
        auto a2 = static_cast<FloatType> (std::real ( p[i+r] * std::conj (p[i + r])));

        theCascadedCoefficients.add (IIR::Coefficients<FloatType> (b0, b1, b2, 1, a1, a2));
    }

    return theCascadedCoefficients;
}

template <typename FloatType>
typename FilterDesign<FloatType>::IIRPolyphaseAllpassStructure
    FilterDesign<FloatType>::designIIRLowpassHalfBandPolyphaseAllpassMethod (FloatType normalizedTransitionWidth,
                                                                             FloatType stopbandAttenuationdB)
{
    jassert (normalizedTransitionWidth > 0 && normalizedTransitionWidth <= 0.5);
    jassert (stopbandAttenuationdB > -300 && stopbandAttenuationdB < -10);

    const double wt = 2 * double_Pi * normalizedTransitionWidth;
    const double ds = Decibels::decibelsToGain (stopbandAttenuationdB, static_cast<FloatType> (-300.0));

    auto k = std::pow (std::tan ((double_Pi - wt) / 4), 2.0);
    auto kp = std::sqrt (1.0 - k * k);
    auto e = (1 - std::sqrt (kp)) / (1 + std::sqrt (kp)) * 0.5;
    auto q = e + 2 * std::pow (e, 5.0) + 15 * std::pow (e, 9.0) + 150 * std::pow (e, 13.0);

    auto k1 = ds * ds / (1 - ds * ds);
    int n = roundDoubleToInt (ceil (log (k1 * k1 / 16) / log (q)));

    if (n % 2 == 0)
        ++n;

    if (n == 1)
        n = 3;

    auto q1 = std::pow (q, (double) n);
    k1 = 4 * std::sqrt (q1);

    const int N = (n - 1) / 2;
    Array<double> ai;

    for (int i = 1; i <= N; ++i)
    {
        double num = 0.0;
        double delta = 1.0;
        int m = 0;

        while (std::abs (delta) > 1e-100)
        {
            delta = std::pow (-1, m) * std::pow (q, m * (m + 1))
                     * std::sin ((2 * m + 1) * double_Pi * i / (double) n);
            num += delta;
            m++;
        }

        num *= 2 * std::pow (q, 0.25);

        double den = 0.0;
        delta = 1.0;
        m = 1;

        while (std::abs (delta) > 1e-100)
        {
            delta = std::pow (-1, m) * std::pow (q, m * m)
                     * std::cos (2 * m * double_Pi * i / (double) n);
            den += delta;
            ++m;
        }

        den = 1 + 2 * den;

        auto wi = num / den;
        auto api = std::sqrt ((1 - wi * wi * k) * (1 - wi * wi / k)) / (1 + wi * wi);

        ai.add ((1 - api) / (1 + api));
    }

    IIRPolyphaseAllpassStructure structure;

    for (int i = 0; i < N; i += 2)
        structure.directPath.add (IIR::Coefficients<FloatType> (static_cast<FloatType> (ai[i]),
                                                                0, 1, 1, 0, static_cast<FloatType> (ai[i])));

    structure.delayedPath.add (IIR::Coefficients<FloatType> (0, 1, 1, 0));

    for (int i = 1; i < N; i += 2)
        structure.delayedPath.add (IIR::Coefficients<FloatType> (static_cast<FloatType> (ai[i]),
                                                                 0, 1, 1, 0, static_cast<FloatType> (ai[i])));

    return structure;
}


template struct FilterDesign<float>;
template struct FilterDesign<double>;

} // namespace dsp
} // namespace juce
