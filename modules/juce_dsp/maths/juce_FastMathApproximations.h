/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce::dsp
{

/**
    This class contains various fast mathematical function approximations.

    @tags{DSP}
*/
struct FastMathApproximations
{
    /** Provides a fast approximation of the function cosh(x) using a Pade approximant
        continued fraction, calculated sample by sample.

        Note: This is an approximation which works on a limited range. You are
        advised to use input values only between -5 and +5 for limiting the error.
    */
    template <typename FloatType>
    static FloatType cosh (FloatType x) noexcept
    {
        auto x2 = x * x;
        auto numerator = -(39251520 + x2 * (18471600 + x2 * (1075032 + 14615 * x2)));
        auto denominator = -39251520 + x2 * (1154160 + x2 * (-16632 + 127 * x2));
        return numerator / denominator;
    }

    /** Provides a fast approximation of the function cosh(x) using a Pade approximant
        continued fraction, calculated on a whole buffer.

        Note: This is an approximation which works on a limited range. You are
        advised to use input values only between -5 and +5 for limiting the error.
    */
    template <typename FloatType>
    static void cosh (FloatType* values, size_t numValues) noexcept
    {
        for (size_t i = 0; i < numValues; ++i)
            values[i] = FastMathApproximations::cosh (values[i]);
    }

    /** Provides a fast approximation of the function sinh(x) using a Pade approximant
        continued fraction, calculated sample by sample.

        Note: This is an approximation which works on a limited range. You are
        advised to use input values only between -5 and +5 for limiting the error.
    */
    template <typename FloatType>
    static FloatType sinh (FloatType x) noexcept
    {
        auto x2 = x * x;
        auto numerator = -x * (11511339840 + x2 * (1640635920 + x2 * (52785432 + x2 * 479249)));
        auto denominator = -11511339840 + x2 * (277920720 + x2 * (-3177720 + x2 * 18361));
        return numerator / denominator;
    }

    /** Provides a fast approximation of the function sinh(x) using a Pade approximant
        continued fraction, calculated on a whole buffer.

        Note: This is an approximation which works on a limited range. You are
        advised to use input values only between -5 and +5 for limiting the error.
    */
    template <typename FloatType>
    static void sinh (FloatType* values, size_t numValues) noexcept
    {
        for (size_t i = 0; i < numValues; ++i)
            values[i] = FastMathApproximations::sinh (values[i]);
    }

    /** Provides a fast approximation of the function tanh(x) using a Pade approximant
        continued fraction, calculated sample by sample.

        Note: This is an approximation which works on a limited range. You are
        advised to use input values only between -5 and +5 for limiting the error.
    */
    template <typename FloatType>
    static FloatType tanh (FloatType x) noexcept
    {
        auto x2 = x * x;
        auto numerator = x * (135135 + x2 * (17325 + x2 * (378 + x2)));
        auto denominator = 135135 + x2 * (62370 + x2 * (3150 + 28 * x2));
        return numerator / denominator;
    }

    /** Provides a fast approximation of the function tanh(x) using a Pade approximant
        continued fraction, calculated on a whole buffer.

        Note: This is an approximation which works on a limited range. You are
        advised to use input values only between -5 and +5 for limiting the error.
    */
    template <typename FloatType>
    static void tanh (FloatType* values, size_t numValues) noexcept
    {
        for (size_t i = 0; i < numValues; ++i)
            values[i] = FastMathApproximations::tanh (values[i]);
    }

    //==============================================================================
    /** Provides a fast approximation of the function cos(x) using a Pade approximant
        continued fraction, calculated sample by sample.

        Note: This is an approximation which works on a limited range. You are
        advised to use input values only between -pi and +pi for limiting the error.
    */
    template <typename FloatType>
    static FloatType cos (FloatType x) noexcept
    {
        auto x2 = x * x;
        auto numerator = -(-39251520 + x2 * (18471600 + x2 * (-1075032 + 14615 * x2)));
        auto denominator = 39251520 + x2 * (1154160 + x2 * (16632 + x2 * 127));
        return numerator / denominator;
    }

    /** Provides a fast approximation of the function cos(x) using a Pade approximant
        continued fraction, calculated on a whole buffer.

        Note: This is an approximation which works on a limited range. You are
        advised to use input values only between -pi and +pi for limiting the error.
    */
    template <typename FloatType>
    static void cos (FloatType* values, size_t numValues) noexcept
    {
        for (size_t i = 0; i < numValues; ++i)
            values[i] = FastMathApproximations::cos (values[i]);
    }

    /** Provides a fast approximation of the function sin(x) using a Pade approximant
        continued fraction, calculated sample by sample.

        Note: This is an approximation which works on a limited range. You are
        advised to use input values only between -pi and +pi for limiting the error.
    */
    template <typename FloatType>
    static FloatType sin (FloatType x) noexcept
    {
        auto x2 = x * x;
        auto numerator = -x * (-11511339840 + x2 * (1640635920 + x2 * (-52785432 + x2 * 479249)));
        auto denominator = 11511339840 + x2 * (277920720 + x2 * (3177720 + x2 * 18361));
        return numerator / denominator;
    }

    /** Provides a fast approximation of the function sin(x) using a Pade approximant
        continued fraction, calculated on a whole buffer.

        Note: This is an approximation which works on a limited range. You are
        advised to use input values only between -pi and +pi for limiting the error.
    */
    template <typename FloatType>
    static void sin (FloatType* values, size_t numValues) noexcept
    {
        for (size_t i = 0; i < numValues; ++i)
            values[i] = FastMathApproximations::sin (values[i]);
    }

    /** Provides a fast approximation of the function tan(x) using a Pade approximant
        continued fraction, calculated sample by sample.

        Note: This is an approximation which works on a limited range. You are
        advised to use input values only between -pi/2 and +pi/2 for limiting the error.
    */
    template <typename FloatType>
    static FloatType tan (FloatType x) noexcept
    {
        auto x2 = x * x;
        auto numerator = x * (-135135 + x2 * (17325 + x2 * (-378 + x2)));
        auto denominator = -135135 + x2 * (62370 + x2 * (-3150 + 28 * x2));
        return numerator / denominator;
    }

    /** Provides a fast approximation of the function tan(x) using a Pade approximant
        continued fraction, calculated on a whole buffer.

        Note: This is an approximation which works on a limited range. You are
        advised to use input values only between -pi/2 and +pi/2 for limiting the error.
    */
    template <typename FloatType>
    static void tan (FloatType* values, size_t numValues) noexcept
    {
        for (size_t i = 0; i < numValues; ++i)
            values[i] = FastMathApproximations::tan (values[i]);
    }

    //==============================================================================
    /** Provides a fast approximation of the function exp(x) using a Pade approximant
        continued fraction, calculated sample by sample.

        Note: This is an approximation which works on a limited range. You are
        advised to use input values only between -6 and +4 for limiting the error.
    */
    template <typename FloatType>
    static FloatType exp (FloatType x) noexcept
    {
        auto numerator = 1680 + x * (840 + x * (180 + x * (20 + x)));
        auto denominator = 1680 + x *(-840 + x * (180 + x * (-20 + x)));
        return numerator / denominator;
    }

    /** Provides a fast approximation of the function exp(x) using a Pade approximant
        continued fraction, calculated on a whole buffer.

        Note: This is an approximation which works on a limited range. You are
        advised to use input values only between -6 and +4 for limiting the error.
    */
    template <typename FloatType>
    static void exp (FloatType* values, size_t numValues) noexcept
    {
        for (size_t i = 0; i < numValues; ++i)
            values[i] = FastMathApproximations::exp (values[i]);
    }

    /** Provides a fast approximation of the function log(x+1) using a Pade approximant
        continued fraction, calculated sample by sample.

        Note: This is an approximation which works on a limited range. You are
        advised to use input values only between -0.8 and +5 for limiting the error.
    */
    template <typename FloatType>
    static FloatType logNPlusOne (FloatType x) noexcept
    {
        auto numerator = x * (7560 + x * (15120 + x * (9870 + x * (2310 + x * 137))));
        auto denominator = 7560 + x * (18900 + x * (16800 + x * (6300 + x * (900 + 30 * x))));
        return numerator / denominator;
    }

    /** Provides a fast approximation of the function log(x+1) using a Pade approximant
        continued fraction, calculated on a whole buffer.

        Note: This is an approximation which works on a limited range. You are
        advised to use input values only between -0.8 and +5 for limiting the error.
    */
    template <typename FloatType>
    static void logNPlusOne (FloatType* values, size_t numValues) noexcept
    {
        for (size_t i = 0; i < numValues; ++i)
            values[i] = FastMathApproximations::logNPlusOne (values[i]);
    }
};

} // namespace juce::dsp
