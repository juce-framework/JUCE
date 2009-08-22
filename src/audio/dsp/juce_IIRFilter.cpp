/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

#include "../../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_IIRFilter.h"
#include "../../threads/juce_ScopedLock.h"


//==============================================================================
IIRFilter::IIRFilter() throw()
    : active (false)
{
    reset();
}

IIRFilter::IIRFilter (const IIRFilter& other) throw()
    : active (other.active)
{
    const ScopedLock sl (other.processLock);
    memcpy (coefficients, other.coefficients, sizeof (coefficients));
    reset();
}

IIRFilter::~IIRFilter() throw()
{
}

//==============================================================================
void IIRFilter::reset() throw()
{
    const ScopedLock sl (processLock);

    x1 = 0;
    x2 = 0;
    y1 = 0;
    y2 = 0;
}

float IIRFilter::processSingleSampleRaw (const float in) throw()
{
    float out = coefficients[0] * in
                 + coefficients[1] * x1
                 + coefficients[2] * x2
                 - coefficients[4] * y1
                 - coefficients[5] * y2;

#if JUCE_INTEL
    if (! (out < -1.0e-8 || out > 1.0e-8))
        out = 0;
#endif

    x2 = x1;
    x1 = in;
    y2 = y1;
    y1 = out;

    return out;
}

void IIRFilter::processSamples (float* const samples,
                                const int numSamples) throw()
{
    const ScopedLock sl (processLock);

    if (active)
    {
        for (int i = 0; i < numSamples; ++i)
        {
            const float in = samples[i];

            float out = coefficients[0] * in
                         + coefficients[1] * x1
                         + coefficients[2] * x2
                         - coefficients[4] * y1
                         - coefficients[5] * y2;

#if JUCE_INTEL
            if (! (out < -1.0e-8 || out > 1.0e-8))
                out = 0;
#endif

            x2 = x1;
            x1 = in;
            y2 = y1;
            y1 = out;

            samples[i] = out;
        }
    }
}

//==============================================================================
void IIRFilter::makeLowPass (const double sampleRate,
                             const double frequency) throw()
{
    jassert (sampleRate > 0);

    const double n = 1.0 / tan (double_Pi * frequency / sampleRate);
    const double nSquared = n * n;
    const double c1 = 1.0 / (1.0 + sqrt (2.0) * n + nSquared);

    setCoefficients (c1,
                     c1 * 2.0f,
                     c1,
                     1.0,
                     c1 * 2.0 * (1.0 - nSquared),
                     c1 * (1.0 - sqrt (2.0) * n + nSquared));
}

void IIRFilter::makeHighPass (const double sampleRate,
                              const double frequency) throw()
{
    const double n = tan (double_Pi * frequency / sampleRate);
    const double nSquared = n * n;
    const double c1 = 1.0 / (1.0 + sqrt (2.0) * n + nSquared);

    setCoefficients (c1,
                     c1 * -2.0f,
                     c1,
                     1.0,
                     c1 * 2.0 * (nSquared - 1.0),
                     c1 * (1.0 - sqrt (2.0) * n + nSquared));
}

void IIRFilter::makeLowShelf (const double sampleRate,
                              const double cutOffFrequency,
                              const double Q,
                              const float gainFactor) throw()
{
    jassert (sampleRate > 0);
    jassert (Q > 0);

    const double A = jmax (0.0f, gainFactor);
    const double aminus1 = A - 1.0;
    const double aplus1 = A + 1.0;
    const double omega = (double_Pi * 2.0 * jmax (cutOffFrequency, 2.0)) / sampleRate;
    const double coso = cos (omega);
    const double beta = sin (omega) * sqrt (A) / Q;
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
                               const float gainFactor) throw()
{
    jassert (sampleRate > 0);
    jassert (Q > 0);

    const double A = jmax (0.0f, gainFactor);
    const double aminus1 = A - 1.0;
    const double aplus1 = A + 1.0;
    const double omega = (double_Pi * 2.0 * jmax (cutOffFrequency, 2.0)) / sampleRate;
    const double coso = cos (omega);
    const double beta = sin (omega) * sqrt (A) / Q;
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
                              const float gainFactor) throw()
{
    jassert (sampleRate > 0);
    jassert (Q > 0);

    const double A = jmax (0.0f, gainFactor);
    const double omega = (double_Pi * 2.0 * jmax (centreFrequency, 2.0)) / sampleRate;
    const double alpha = 0.5 * sin (omega) / Q;
    const double c2 = -2.0 * cos (omega);
    const double alphaTimesA = alpha * A;
    const double alphaOverA = alpha / A;

    setCoefficients (1.0 + alphaTimesA,
                     c2,
                     1.0 - alphaTimesA,
                     1.0 + alphaOverA,
                     c2,
                     1.0 - alphaOverA);
}

void IIRFilter::makeInactive() throw()
{
    const ScopedLock sl (processLock);
    active = false;
}

//==============================================================================
void IIRFilter::copyCoefficientsFrom (const IIRFilter& other) throw()
{
    const ScopedLock sl (processLock);

    memcpy (coefficients, other.coefficients, sizeof (coefficients));
    active = other.active;
}

//==============================================================================
void IIRFilter::setCoefficients (double c1,
                                 double c2,
                                 double c3,
                                 double c4,
                                 double c5,
                                 double c6) throw()
{
    const double a = 1.0 / c4;

    c1 *= a;
    c2 *= a;
    c3 *= a;
    c5 *= a;
    c6 *= a;

    const ScopedLock sl (processLock);

    coefficients[0] = (float) c1;
    coefficients[1] = (float) c2;
    coefficients[2] = (float) c3;
    coefficients[3] = (float) c4;
    coefficients[4] = (float) c5;
    coefficients[5] = (float) c6;

    active = true;
}



END_JUCE_NAMESPACE
