/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{
namespace dsp
{

/**
    Contains miscellaneous filter design and windowing functions.
*/
struct SpecialFunctions
{
    /** Computes the modified Bessel function of the first kind I0 for a
        given double value x. Modified Bessel functions are useful to solve
        various mathematical problems involving differential equations.
    */
    static double besselI0 (double x) noexcept;

    /** Computes the complete elliptic integral of the first kind K for a
        given double value k, and the associated complete elliptic integral
        of the first kind Kp for the complementary modulus of k.
    */
    static void ellipticIntegralK (double k, double& K, double& Kp) noexcept;

    /** Computes the Jacobian elliptic function cd for the elliptic
        modulus k and the quarter-period units u.
    */
    static Complex<double> cde (Complex<double> u, double k) noexcept;

    /** Computes the Jacobian elliptic function sn for the elliptic
        modulus k and the quarter-period units u.
    */
    static Complex<double> sne (Complex<double> u, double k) noexcept;

    /** Computes the inverse of the Jacobian elliptic function sn
        for the elliptic modulus k and the quarter-period units u.
    */
    static Complex<double> asne (Complex<double> w, double k) noexcept;
};

} // namespace dsp
} // namespace juce
