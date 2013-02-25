/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#if JUCE_INTEL
 #define JUCE_SNAP_TO_ZERO(n)    if (! (n < -1.0e-8 || n > 1.0e-8)) n = 0;
#else
 #define JUCE_SNAP_TO_ZERO(n)
#endif

//==============================================================================
IIRFilter::IIRFilter()
    : active (false), v1 (0), v2 (0)
{
}

IIRFilter::IIRFilter (const IIRFilter& other)
    : active (other.active), v1 (0), v2 (0)
{
    const SpinLock::ScopedLockType sl (other.processLock);
    memcpy (coefficients, other.coefficients, sizeof (coefficients));
}

IIRFilter::~IIRFilter()
{
}

//==============================================================================
void IIRFilter::reset() noexcept
{
    const SpinLock::ScopedLockType sl (processLock);
    v1 = v2 = 0;
}

float IIRFilter::processSingleSampleRaw (const float in) noexcept
{
    float out = coefficients[0] * in + v1;

    JUCE_SNAP_TO_ZERO (out);

    v1 = coefficients[1] * in - coefficients[3] * out + v2;
    v2 = coefficients[2] * in - coefficients[4] * out;

    return out;
}

void IIRFilter::processSamples (float* const samples,
                                const int numSamples) noexcept
{
    const SpinLock::ScopedLockType sl (processLock);

    if (active)
    {
        const float c0 = coefficients[0];
        const float c1 = coefficients[1];
        const float c2 = coefficients[2];
        const float c3 = coefficients[3];
        const float c4 = coefficients[4];
        float lv1 = v1, lv2 = v2;

        for (int i = 0; i < numSamples; ++i)
        {
            const float in = samples[i];
            const float out = c0 * in + lv1;
            samples[i] = out;

            lv1 = c1 * in - c3 * out + lv2;
            lv2 = c2 * in - c4 * out;
        }

        JUCE_SNAP_TO_ZERO (lv1);  v1 = lv1;
        JUCE_SNAP_TO_ZERO (lv2);  v2 = lv2;
    }
}

//==============================================================================
void IIRFilter::makeLowPass (const double sampleRate,
                             const double frequency) noexcept
{
    jassert (sampleRate > 0);

    const double n = 1.0 / tan (double_Pi * frequency / sampleRate);
    const double nSquared = n * n;
    const double c1 = 1.0 / (1.0 + std::sqrt (2.0) * n + nSquared);

    setCoefficients (c1,
                     c1 * 2.0f,
                     c1,
                     1.0,
                     c1 * 2.0 * (1.0 - nSquared),
                     c1 * (1.0 - std::sqrt (2.0) * n + nSquared));
}

void IIRFilter::makeHighPass (const double sampleRate,
                              const double frequency) noexcept
{
    const double n = tan (double_Pi * frequency / sampleRate);
    const double nSquared = n * n;
    const double c1 = 1.0 / (1.0 + std::sqrt (2.0) * n + nSquared);

    setCoefficients (c1,
                     c1 * -2.0f,
                     c1,
                     1.0,
                     c1 * 2.0 * (nSquared - 1.0),
                     c1 * (1.0 - std::sqrt (2.0) * n + nSquared));
}

void IIRFilter::makeLowShelf (const double sampleRate,
                              const double cutOffFrequency,
                              const double Q,
                              const float gainFactor) noexcept
{
    jassert (sampleRate > 0);
    jassert (Q > 0);

    const double A = jmax (0.0f, gainFactor);
    const double aminus1 = A - 1.0;
    const double aplus1 = A + 1.0;
    const double omega = (double_Pi * 2.0 * jmax (cutOffFrequency, 2.0)) / sampleRate;
    const double coso = std::cos (omega);
    const double beta = std::sin (omega) * std::sqrt (A) / Q;
    const double aminus1TimesCoso = aminus1 * coso;

    setCoefficients (A * (aplus1 - aminus1TimesCoso + beta),
                     A * 2.0 * (aminus1 - aplus1 * coso),
                     A * (aplus1 - aminus1TimesCoso - beta),
                     aplus1 + aminus1TimesCoso + beta,
                     -2.0 * (aminus1 + aplus1 * coso),
                     aplus1 + aminus1TimesCoso - beta);
}

void IIRFilter::makeHighShelf (const double sampleRate,
                               const double cutOffFrequency,
                               const double Q,
                               const float gainFactor) noexcept
{
    jassert (sampleRate > 0);
    jassert (Q > 0);

    const double A = jmax (0.0f, gainFactor);
    const double aminus1 = A - 1.0;
    const double aplus1 = A + 1.0;
    const double omega = (double_Pi * 2.0 * jmax (cutOffFrequency, 2.0)) / sampleRate;
    const double coso = std::cos (omega);
    const double beta = std::sin (omega) * std::sqrt (A) / Q;
    const double aminus1TimesCoso = aminus1 * coso;

    setCoefficients (A * (aplus1 + aminus1TimesCoso + beta),
                     A * -2.0 * (aminus1 + aplus1 * coso),
                     A * (aplus1 + aminus1TimesCoso - beta),
                     aplus1 - aminus1TimesCoso + beta,
                     2.0 * (aminus1 - aplus1 * coso),
                     aplus1 - aminus1TimesCoso - beta);
}

void IIRFilter::makeBandPass (const double sampleRate,
                              const double centreFrequency,
                              const double Q,
                              const float gainFactor) noexcept
{
    jassert (sampleRate > 0);
    jassert (Q > 0);

    const double A = jmax (0.0f, gainFactor);
    const double omega = (double_Pi * 2.0 * jmax (centreFrequency, 2.0)) / sampleRate;
    const double alpha = 0.5 * std::sin (omega) / Q;
    const double c2 = -2.0 * std::cos (omega);
    const double alphaTimesA = alpha * A;
    const double alphaOverA = alpha / A;

    setCoefficients (1.0 + alphaTimesA,
                     c2,
                     1.0 - alphaTimesA,
                     1.0 + alphaOverA,
                     c2,
                     1.0 - alphaOverA);
}

void IIRFilter::makeInactive() noexcept
{
    const SpinLock::ScopedLockType sl (processLock);
    active = false;
}

//==============================================================================
void IIRFilter::copyCoefficientsFrom (const IIRFilter& other) noexcept
{
    const SpinLock::ScopedLockType sl (processLock);

    memcpy (coefficients, other.coefficients, sizeof (coefficients));
    active = other.active;
}

//==============================================================================
void IIRFilter::setCoefficients (double c1, double c2, double c3,
                                 double c4, double c5, double c6) noexcept
{
    const double a = 1.0 / c4;

    c1 *= a;
    c2 *= a;
    c3 *= a;
    c5 *= a;
    c6 *= a;

    const SpinLock::ScopedLockType sl (processLock);

    coefficients[0] = (float) c1;
    coefficients[1] = (float) c2;
    coefficients[2] = (float) c3;
    coefficients[3] = (float) c5;
    coefficients[4] = (float) c6;

    active = true;
}

#undef JUCE_SNAP_TO_ZERO
