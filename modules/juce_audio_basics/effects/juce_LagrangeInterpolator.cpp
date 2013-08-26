/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

namespace LagrangeHelpers
{
    template <int k>
    struct ResampleHelper
    {
        static forcedinline void calc (float& a, float b) { a *= b * (1.0f / k); }
    };

    template<>
    struct ResampleHelper <0>
    {
        static forcedinline void calc (float&, float) {}
    };

    template <int k>
    static forcedinline float calcCoefficient (float input, const float offset) noexcept
    {
        ResampleHelper <0 - k>::calc (input, -2.0f - offset);
        ResampleHelper <1 - k>::calc (input, -1.0f - offset);
        ResampleHelper <2 - k>::calc (input,  0.0f - offset);
        ResampleHelper <3 - k>::calc (input,  1.0f - offset);
        ResampleHelper <4 - k>::calc (input,  2.0f - offset);
        return input;
    }

    static forcedinline float valueAtOffset (const float* const inputs, const float offset) noexcept
    {
        return calcCoefficient<0> (inputs[4], offset)
             + calcCoefficient<1> (inputs[3], offset)
             + calcCoefficient<2> (inputs[2], offset)
             + calcCoefficient<3> (inputs[1], offset)
             + calcCoefficient<4> (inputs[0], offset);
    }

    static forcedinline void push (float* inputs, const float newValue) noexcept
    {
        inputs[4] = inputs[3];
        inputs[3] = inputs[2];
        inputs[2] = inputs[1];
        inputs[1] = inputs[0];
        inputs[0] = newValue;
    }
}

//==============================================================================
LagrangeInterpolator::LagrangeInterpolator()  { reset(); }
LagrangeInterpolator::~LagrangeInterpolator() {}

void LagrangeInterpolator::reset() noexcept
{
    subSamplePos = 1.0;

    for (int i = 0; i < numElementsInArray (lastInputSamples); ++i)
        lastInputSamples[i] = 0;
}

int LagrangeInterpolator::process (const double actualRatio, const float* in,
                                   float* out, const int numOut) noexcept
{
    if (actualRatio == 1.0)
    {
        memcpy (out, in, (size_t) numOut * sizeof (float));

        if (numOut >= 4)
        {
            memcpy (lastInputSamples, in + (numOut - 4), 4 * sizeof (float));
        }
        else
        {
            for (int i = 0; i < numOut; ++i)
                LagrangeHelpers::push (lastInputSamples, in[i]);
        }

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
                LagrangeHelpers::push (lastInputSamples, *in++);
                pos -= 1.0;
            }

            *out++ = LagrangeHelpers::valueAtOffset (lastInputSamples, (float) pos);
            pos += actualRatio;
        }
    }
    else
    {
        for (int i = numOut; --i >= 0;)
        {
            while (pos < actualRatio)
            {
                LagrangeHelpers::push (lastInputSamples, *in++);
                pos += 1.0;
            }

            pos -= actualRatio;
            *out++ = LagrangeHelpers::valueAtOffset (lastInputSamples, 1.0f - (float) pos);
        }
    }

    subSamplePos = pos;
    return (int) (in - originalIn);
}

int LagrangeInterpolator::processAdding (const double actualRatio, const float* in,
                                         float* out, const int numOut, const float gain) noexcept
{
    if (actualRatio == 1.0)
    {
        if (gain != 1.0f)
        {
            for (int i = 0; i < numOut; ++i)
                out[i] += in[i] * gain;
        }
        else
        {
            for (int i = 0; i < numOut; ++i)
                out[i] += in[i];
        }

        if (numOut >= 4)
        {
            memcpy (lastInputSamples, in + (numOut - 4), 4 * sizeof (float));
        }
        else
        {
            for (int i = 0; i < numOut; ++i)
                LagrangeHelpers::push (lastInputSamples, in[i]);
        }

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
                LagrangeHelpers::push (lastInputSamples, *in++);
                pos -= 1.0;
            }

            *out++ += gain * LagrangeHelpers::valueAtOffset (lastInputSamples, (float) pos);
            pos += actualRatio;
        }
    }
    else
    {
        for (int i = numOut; --i >= 0;)
        {
            while (pos < actualRatio)
            {
                LagrangeHelpers::push (lastInputSamples, *in++);
                pos += 1.0;
            }

            pos -= actualRatio;
            *out++ += gain * LagrangeHelpers::valueAtOffset (lastInputSamples, jmax (0.0f, 1.0f - (float) pos));
        }
    }

    subSamplePos = pos;
    return (int) (in - originalIn);
}
