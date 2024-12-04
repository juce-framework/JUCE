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

} // namespace juce::dsp
