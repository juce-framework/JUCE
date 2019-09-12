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
class LagrangeAlgorithmInternal
{
public:
    LagrangeAlgorithmInternal() = delete;

    static forcedinline SampleType valueAtOffset (const SampleType* inputs, CoefficientType offset) noexcept
    {
        return calcCoefficient<0> (inputs[4], offset)
                + calcCoefficient<1> (inputs[3], offset)
                + calcCoefficient<2> (inputs[2], offset)
                + calcCoefficient<3> (inputs[1], offset)
                + calcCoefficient<4> (inputs[0], offset);
    }

private:
    template<int k>
    struct LagrangeResampleHelper
    {
        static forcedinline void calc (SampleType& a, CoefficientType b) noexcept
        {
            if (k != 0) a *= b * (1.0f / k);
        }
    };

    template<int k>
    static forcedinline SampleType calcCoefficient (SampleType input, CoefficientType offset) noexcept
    {
        LagrangeResampleHelper<0 - k>::calc (input, CoefficientType (-2.0) - offset);
        LagrangeResampleHelper<1 - k>::calc (input, CoefficientType (-1.0) - offset);
        LagrangeResampleHelper<2 - k>::calc (input, CoefficientType (0.0f) - offset);
        LagrangeResampleHelper<3 - k>::calc (input, CoefficientType (1.0f) - offset);
        LagrangeResampleHelper<4 - k>::calc (input, CoefficientType (2.0f) - offset);
        return input;
    }
};

/**
    Interpolator for resampling a stream of floating point values using 4-point
    lagrange interpolation. SampleType can be float, double, std::complex<float>
    and std::complex<double>. Note that you need to specify a real valued
    CoefficientType if working with complex data.

    Note that the resampler is stateful, so when there's a break in the continuity
    of the input stream you're feeding it, you should call reset() before feeding
    it any new data. And like with any other stateful filter, if you're resampling
    multiple channels, make sure each one uses its own LagrangeInterpolator
    object.

    @see CatmullRomInterpolator

    @tags{Audio}
*/
template <typename SampleType = float, typename CoefficientType = SampleType>
class JUCE_API  LagrangeResampler : public ResamplerBase<SampleType, CoefficientType, LagrangeAlgorithmInternal<SampleType, CoefficientType>>
{
public:
    LagrangeResampler()  noexcept {};
    ~LagrangeResampler() noexcept {};

private:

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LagrangeResampler)
};

/** Alias to make the new templated class backwards compatible with the old float-only implementation */
using LagrangeInterpolator = LagrangeResampler<float>;
    
} // namespace juce
