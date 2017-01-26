/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2016 - ROLI Ltd.

   Permission is granted to use this software under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license/

   Permission to use, copy, modify, and/or distribute this software for any
   purpose with or without fee is hereby granted, provided that the above
   copyright notice and this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES WITH REGARD
   TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
   FITNESS. IN NO EVENT SHALL ISC BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT,
   OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
   USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
   TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
   OF THIS SOFTWARE.

   -----------------------------------------------------------------------------

   To release a closed-source product which uses other parts of JUCE not
   licensed under the ISC terms, commercial licenses are available: visit
   www.juce.com for more information.

  ==============================================================================
*/

namespace
{
    static forcedinline void pushInterpolationSample (float* lastInputSamples, const float newValue) noexcept
    {
        lastInputSamples[4] = lastInputSamples[3];
        lastInputSamples[3] = lastInputSamples[2];
        lastInputSamples[2] = lastInputSamples[1];
        lastInputSamples[1] = lastInputSamples[0];
        lastInputSamples[0] = newValue;
    }

    static forcedinline void pushInterpolationSamples (float* lastInputSamples, const float* input, int numOut) noexcept
    {
        if (numOut >= 5)
        {
            for (int i = 0; i < 5; ++i)
                lastInputSamples[i] = input[--numOut];
        }
        else
        {
            for (int i = 0; i < numOut; ++i)
                pushInterpolationSample (lastInputSamples, input[i]);
        }
    }

    template <typename InterpolatorType>
    static int interpolate (float* lastInputSamples, double& subSamplePos, const double actualRatio,
                            const float* in, float* out, const int numOut) noexcept
    {
        if (actualRatio == 1.0)
        {
            memcpy (out, in, (size_t) numOut * sizeof (float));
            pushInterpolationSamples (lastInputSamples, in, numOut);
            return numOut;
        }

        const float* const originalIn = in;
        double pos = subSamplePos;

        if (actualRatio < 1.0)
        {
            for (int i = numOut; --i >= 0;)
            {
                if (pos >= 1.0)
                {
                    pushInterpolationSample (lastInputSamples, *in++);
                    pos -= 1.0;
                }

                *out++ = InterpolatorType::valueAtOffset (lastInputSamples, (float) pos);
                pos += actualRatio;
            }
        }
        else
        {
            for (int i = numOut; --i >= 0;)
            {
                while (pos < actualRatio)
                {
                    pushInterpolationSample (lastInputSamples, *in++);
                    pos += 1.0;
                }

                pos -= actualRatio;
                *out++ = InterpolatorType::valueAtOffset (lastInputSamples, jmax (0.0f, 1.0f - (float) pos));
            }
        }

        subSamplePos = pos;
        return (int) (in - originalIn);
    }

    template <typename InterpolatorType>
    static int interpolateAdding (float* lastInputSamples, double& subSamplePos, const double actualRatio,
                                  const float* in, float* out, const int numOut, const float gain) noexcept
    {
        if (actualRatio == 1.0)
        {
            FloatVectorOperations::addWithMultiply (out, in, gain, numOut);
            pushInterpolationSamples (lastInputSamples, in, numOut);
            return numOut;
        }

        const float* const originalIn = in;
        double pos = subSamplePos;

        if (actualRatio < 1.0)
        {
            for (int i = numOut; --i >= 0;)
            {
                if (pos >= 1.0)
                {
                    pushInterpolationSample (lastInputSamples, *in++);
                    pos -= 1.0;
                }

                *out++ += gain * InterpolatorType::valueAtOffset (lastInputSamples, (float) pos);
                pos += actualRatio;
            }
        }
        else
        {
            for (int i = numOut; --i >= 0;)
            {
                while (pos < actualRatio)
                {
                    pushInterpolationSample (lastInputSamples, *in++);
                    pos += 1.0;
                }

                pos -= actualRatio;
                *out++ += gain * InterpolatorType::valueAtOffset (lastInputSamples, jmax (0.0f, 1.0f - (float) pos));
            }
        }

        subSamplePos = pos;
        return (int) (in - originalIn);
    }
}

//==============================================================================
template <int k>
struct LagrangeResampleHelper
{
    static forcedinline void calc (float& a, float b) noexcept   { a *= b * (1.0f / k); }
};

template<>
struct LagrangeResampleHelper<0>
{
    static forcedinline void calc (float&, float) noexcept {}
};

struct LagrangeAlgorithm
{
    static forcedinline float valueAtOffset (const float* const inputs, const float offset) noexcept
    {
        return calcCoefficient<0> (inputs[4], offset)
             + calcCoefficient<1> (inputs[3], offset)
             + calcCoefficient<2> (inputs[2], offset)
             + calcCoefficient<3> (inputs[1], offset)
             + calcCoefficient<4> (inputs[0], offset);
    }

    template <int k>
    static forcedinline float calcCoefficient (float input, const float offset) noexcept
    {
        LagrangeResampleHelper<0 - k>::calc (input, -2.0f - offset);
        LagrangeResampleHelper<1 - k>::calc (input, -1.0f - offset);
        LagrangeResampleHelper<2 - k>::calc (input,  0.0f - offset);
        LagrangeResampleHelper<3 - k>::calc (input,  1.0f - offset);
        LagrangeResampleHelper<4 - k>::calc (input,  2.0f - offset);
        return input;
    }
};

LagrangeInterpolator::LagrangeInterpolator() noexcept  { reset(); }
LagrangeInterpolator::~LagrangeInterpolator() noexcept {}

void LagrangeInterpolator::reset() noexcept
{
    subSamplePos = 1.0;

    for (int i = 0; i < numElementsInArray (lastInputSamples); ++i)
        lastInputSamples[i] = 0;
}

int LagrangeInterpolator::process (double actualRatio, const float* in, float* out, int numOut) noexcept
{
    return interpolate<LagrangeAlgorithm> (lastInputSamples, subSamplePos, actualRatio, in, out, numOut);
}

int LagrangeInterpolator::processAdding (double actualRatio, const float* in, float* out, int numOut, float gain) noexcept
{
    return interpolateAdding<LagrangeAlgorithm> (lastInputSamples, subSamplePos, actualRatio, in, out, numOut, gain);
}
