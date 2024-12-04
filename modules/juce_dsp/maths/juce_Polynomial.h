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
    A class representing a polynomial

    @tags{DSP}
*/
template <typename FloatingType>
class Polynomial
{
public:
    //==============================================================================
    /** Creates a new polynomial which will always evaluate to zero. */
    Polynomial()
    {
        coeffs.add (0);
    }

    /** Creates a new polynomial with given coefficients.

        @param numCoefficients The number of coefficients stored in coefficients.
                               This is also the order of the returned polynomial.
        @param coefficients    The coefficients which will be used by the newly
                               created polynomial. The Polynomial class will keep
                               a private copy of the coefficients.
    */
    Polynomial (const FloatingType* coefficients, int numCoefficients)
        : coeffs (coefficients, numCoefficients)
    {
        jassert (! coeffs.isEmpty());
    }

    /** Creates a copy of another polynomial. */
    Polynomial (const Polynomial&) = default;

    /** Creates a copy of another polynomial. */
    Polynomial (Polynomial&&) = default;

    /** Creates a copy of another polynomial. */
    Polynomial& operator= (const Polynomial&) = default;

    /** Creates a copy of another polynomial. */
    Polynomial& operator= (Polynomial&&) = default;

    /** Creates a new polynomial with coefficients by a C++11 initializer list.
        This function can be used in the following way:
        Polynomial<float> p ({0.5f, -0.3f, 0.2f});
    */
    template <typename... Values>
    Polynomial (Values... items)  : coeffs (items...)
    {
        jassert (! coeffs.isEmpty());
    }

    //==============================================================================
    /** Returns a single coefficient of the receiver for reading */
    FloatingType operator[] (int index) const noexcept              { return coeffs.getUnchecked (index); }

    /** Returns a single coefficient of the receiver for modifying. */
    FloatingType& operator[] (int index) noexcept                   { return coeffs.getReference (index); }

    /** Evaluates the value of the polynomial at a single point x. */
    FloatingType operator() (FloatingType x) const noexcept
    {
        // Horner's method
        FloatingType y (0);

        for (int i = coeffs.size(); --i >= 0;)
            y = (x * y) + coeffs.getUnchecked (i);

        return y;
    }

    /** Returns the order of the polynomial. */
    int getOrder() noexcept
    {
        return coeffs.size() - 1;
    }

    //==============================================================================
    /** Returns the polynomial with all its coefficients multiplied with a gain factor */
    Polynomial<FloatingType> withGain (double gain) const
    {
        auto result = *this;

        for (auto& c : result.coeffs)
            c *= gain;

        return result;
    }

    /** Returns the sum of this polynomial with another */
    Polynomial<FloatingType> getSumWith (const Polynomial<FloatingType>& other) const
    {
        if (coeffs.size() < other.coeffs.size())
            return other.getSumWith (*this);

        auto result = *this;

        for (int i = 0; i < other.coeffs.size(); ++i)
            result[i] += other[i];

        return result;
    }

    /** computes the product of two polynomials and return the result */
    Polynomial<FloatingType> getProductWith (const Polynomial<FloatingType>& other) const
    {
        Polynomial<FloatingType> result;
        result.coeffs.clearQuick();

        auto N1 = coeffs.size();
        auto N2 = other.coeffs.size();
        auto Nmax = jmax (N1, N2);

        auto N = N1 + N2 - 1;

        for (int i = 0; i < N; ++i)
        {
            FloatingType value (0);

            for (int j = 0; j < Nmax; ++j)
                if (j >= 0 && j < N1 && i - j >= 0 && i - j < N2)
                    value = value + (*this)[j] * other[i - j];

            result.coeffs.add (value);
        }

        return result;
    }

private:
    //==============================================================================
    Array<FloatingType> coeffs;

    JUCE_LEAK_DETECTOR (Polynomial)
};

} // namespace juce::dsp
