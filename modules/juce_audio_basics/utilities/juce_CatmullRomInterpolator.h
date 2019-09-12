/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

template <typename SampleType, typename CoefficientType>
struct CatmullRomAlgorithmInternal
{
    CatmullRomAlgorithmInternal() = delete;

    static forcedinline SampleType valueAtOffset (const SampleType* const inputs, const CoefficientType offset) noexcept
    {
        auto y0 = inputs[3];
        auto y1 = inputs[2];
        auto y2 = inputs[1];
        auto y3 = inputs[0];

        auto halfY0 = 0.5f * y0;
        auto halfY3 = 0.5f * y3;

        return y1 +  offset * ((0.5f * y2 - halfY0)
                  + (offset * (((y0 + 2.0f * y2) - (halfY3 + 2.5f * y1))
                  + (offset * ((halfY3 + 1.5f * y1) - (halfY0 + 1.5f * y2))))));
    }
};

/**
    Interpolator for resampling a stream of floating point values using Catmull-Rom
    interpolation. SampleType can be float, double, std::complex<float> and
    std::complex<double>. Note that you need to specify a real valued
    CoefficientType if working with complex data.

    Note that the resampler is stateful, so when there's a break in the continuity
    of the input stream you're feeding it, you should call reset() before feeding
    it any new data. And like with any other stateful filter, if you're resampling
    multiple channels, make sure each one uses its own CatmullRomInterpolator
    object.

    @see LagrangeResampler

    @tags{Audio}
*/
template <typename SampleType, typename CoefficientType = SampleType>
class JUCE_API  CatmullRomResampler : public ResamplerBase<SampleType, CoefficientType, CatmullRomAlgorithmInternal<SampleType, CoefficientType>>
{
public:
    CatmullRomResampler()  noexcept {};
    ~CatmullRomResampler() noexcept {};


private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CatmullRomResampler)
};

/** Alias to make the new templated class backwards compatible with the old float-only implementation */
using CatmullRomInterpolator = CatmullRomResampler<float>;

} // namespace juce
