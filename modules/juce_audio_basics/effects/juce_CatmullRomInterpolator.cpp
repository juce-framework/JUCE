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
