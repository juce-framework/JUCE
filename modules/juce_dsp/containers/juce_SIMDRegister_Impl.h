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

namespace juce
{
namespace dsp
{


//==============================================================================
template <typename Type>
struct SIMDRegister<Type>::ElementAccess
{
    ElementAccess (const ElementAccess&) = default;
    operator Type() const                                       { return simd.get (idx); }
    ElementAccess& operator= (Type scalar) noexcept             { simd.set (idx, scalar); return *this; }
    ElementAccess& operator= (const ElementAccess& o) noexcept  { return operator= ((Type) o); }

private:
    friend struct SIMDRegister;
    ElementAccess (SIMDRegister& owner, size_t index) noexcept : simd (owner), idx (index) {}
    SIMDRegister& simd;
    size_t idx;
};

#ifndef DOXYGEN
//==============================================================================
/* This class is used internally by SIMDRegister to abstract away differences
   in operations which are different for complex and pure floating point types. */

// the pure floating-point version
template <typename Scalar>
struct CmplxSIMDOps
{
    using vSIMDType = typename SIMDNativeOps<Scalar>::vSIMDType;

    static vSIMDType JUCE_VECTOR_CALLTYPE load (const Scalar* a) noexcept
    {
        return SIMDNativeOps<Scalar>::load (a);
    }

    static void JUCE_VECTOR_CALLTYPE store (vSIMDType value, Scalar* dest) noexcept
    {
        SIMDNativeOps<Scalar>::store (value, dest);
    }

    static vSIMDType JUCE_VECTOR_CALLTYPE expand (Scalar s) noexcept
    {
        return SIMDNativeOps<Scalar>::expand (s);
    }

    static Scalar JUCE_VECTOR_CALLTYPE get (vSIMDType v, std::size_t i) noexcept
    {
        return SIMDNativeOps<Scalar>::get (v, i);
    }

    static vSIMDType JUCE_VECTOR_CALLTYPE set (vSIMDType v, std::size_t i, Scalar s) noexcept
    {
        return SIMDNativeOps<Scalar>::set (v, i, s);
    }

    static Scalar JUCE_VECTOR_CALLTYPE sum (vSIMDType a)  noexcept
    {
        return SIMDNativeOps<Scalar>::sum (a);
    }

    static vSIMDType JUCE_VECTOR_CALLTYPE mul (vSIMDType a, vSIMDType b) noexcept
    {
        return SIMDNativeOps<Scalar>::mul (a, b);
    }

    static vSIMDType JUCE_VECTOR_CALLTYPE muladd (vSIMDType a, vSIMDType b, vSIMDType c) noexcept
    {
        return SIMDNativeOps<Scalar>::multiplyAdd (a, b, c);
    }
};

// The pure complex version
template <typename Scalar>
struct CmplxSIMDOps<std::complex<Scalar>>
{
    using vSIMDType = typename SIMDNativeOps<Scalar>::vSIMDType;

    static vSIMDType JUCE_VECTOR_CALLTYPE load (const std::complex<Scalar>* a) noexcept
    {
        return SIMDNativeOps<Scalar>::load (reinterpret_cast<const Scalar*> (a));
    }

    static void JUCE_VECTOR_CALLTYPE store (vSIMDType value, std::complex<Scalar>* dest) noexcept
    {
        SIMDNativeOps<Scalar>::store (value, reinterpret_cast<Scalar*> (dest));
    }

    static vSIMDType JUCE_VECTOR_CALLTYPE expand (std::complex<Scalar> s) noexcept
    {
        const int n = sizeof (vSIMDType) / sizeof (Scalar);

        union
        {
            vSIMDType v;
            Scalar floats[(size_t) n];
        } u;

        for (int i = 0; i < n; ++i)
            u.floats[i] = (i & 1) == 0 ? s.real() : s.imag();

        return u.v;
    }

    static std::complex<Scalar> JUCE_VECTOR_CALLTYPE get (vSIMDType v, std::size_t i) noexcept
    {
        auto j = i << 1;
        return std::complex<Scalar> (SIMDNativeOps<Scalar>::get (v, j), SIMDNativeOps<Scalar>::get (v, j + 1));
    }

    static vSIMDType JUCE_VECTOR_CALLTYPE set (vSIMDType v, std::size_t i, std::complex<Scalar> s) noexcept
    {
        auto j = i << 1;
        return SIMDNativeOps<Scalar>::set (SIMDNativeOps<Scalar>::set (v, j, s.real()), j + 1, s.imag());
    }

    static std::complex<Scalar> JUCE_VECTOR_CALLTYPE sum (vSIMDType a)  noexcept
    {
        vSIMDType result = SIMDNativeOps<Scalar>::oddevensum (a);
        auto* ptr = reinterpret_cast<const Scalar*> (&result);
        return std::complex<Scalar> (ptr[0], ptr[1]);
    }

    static vSIMDType JUCE_VECTOR_CALLTYPE mul (vSIMDType a, vSIMDType b)  noexcept
    {
        return SIMDNativeOps<Scalar>::cmplxmul (a, b);
    }

    static vSIMDType JUCE_VECTOR_CALLTYPE muladd (vSIMDType a, vSIMDType b, vSIMDType c) noexcept
    {
        return SIMDNativeOps<Scalar>::add (a, SIMDNativeOps<Scalar>::cmplxmul (b, c));
    }
};
#endif

//==============================================================================
 namespace util
 {
     template <typename Type>
     inline void snapToZero (SIMDRegister<Type>&) noexcept      {}
 }

} // namespace dsp

// Extend some common used global functions to SIMDRegister types
template <typename Type>
inline dsp::SIMDRegister<Type> JUCE_VECTOR_CALLTYPE jmin (dsp::SIMDRegister<Type> a, dsp::SIMDRegister<Type> b) { return dsp::SIMDRegister<Type>::min (a, b); }
template <typename Type>
inline dsp::SIMDRegister<Type> JUCE_VECTOR_CALLTYPE jmax (dsp::SIMDRegister<Type> a, dsp::SIMDRegister<Type> b) { return dsp::SIMDRegister<Type>::max (a, b); }

} // namespace juce
