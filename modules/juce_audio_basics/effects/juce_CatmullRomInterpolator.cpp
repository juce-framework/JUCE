/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

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

struct CatmullRomAlgorithm
{
    static forcedinline float valueAtOffset (const float* const inputs, const float offset) noexcept
    {
        const float y0 = inputs[3];
        const float y1 = inputs[2];
        const float y2 = inputs[1];
        const float y3 = inputs[0];

        const float halfY0 = 0.5f * y0;
        const float halfY3 = 0.5f * y3;

        return y1 + offset * ((0.5f * y2 - halfY0)
                                + (offset * (((y0 + 2.0f * y2) - (halfY3 + 2.5f * y1))
                                              + (offset * ((halfY3 + 1.5f * y1) - (halfY0 + 1.5f * y2))))));
    }
};

CatmullRomInterpolator::CatmullRomInterpolator() noexcept  { reset(); }
CatmullRomInterpolator::~CatmullRomInterpolator() noexcept {}

void CatmullRomInterpolator::reset() noexcept
{
    subSamplePos = 1.0;

    for (int i = 0; i < numElementsInArray (lastInputSamples); ++i)
        lastInputSamples[i] = 0;
}

int CatmullRomInterpolator::process (double actualRatio, const float* in, float* out, int numOut) noexcept
{
    return interpolate<CatmullRomAlgorithm> (lastInputSamples, subSamplePos, actualRatio, in, out, numOut);
}

int CatmullRomInterpolator::processAdding (double actualRatio, const float* in, float* out, int numOut, float gain) noexcept
{
    return interpolateAdding<CatmullRomAlgorithm> (lastInputSamples, subSamplePos, actualRatio, in, out, numOut, gain);
}
