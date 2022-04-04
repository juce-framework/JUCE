/*
  ==============================================================================

   This file is part of the JUCE 7 technical preview.
   Copyright (c) 2022 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For the technical preview this file cannot be licensed commercially.

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

    @tags{DSP}
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
