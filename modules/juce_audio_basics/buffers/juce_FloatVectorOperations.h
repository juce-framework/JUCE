/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

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

#ifndef JUCE_SNAP_TO_ZERO
 #if JUCE_INTEL
  #define JUCE_SNAP_TO_ZERO(n)    if (! (n < -1.0e-8f || n > 1.0e-8f)) n = 0;
 #else
  #define JUCE_SNAP_TO_ZERO(n)    ignoreUnused (n)
 #endif
#endif
class ScopedNoDenormals;

#if ! DOXYGEN
namespace detail
{

template <typename FloatType, typename CountType>
struct FloatVectorOperationsBase
{
    static void JUCE_CALLTYPE clear (FloatType* dest, CountType numValues) noexcept;
    static void JUCE_CALLTYPE fill (FloatType* dest, FloatType valueToFill, CountType numValues) noexcept;
    static void JUCE_CALLTYPE copy (FloatType* dest, const FloatType* src, CountType numValues) noexcept;
    static void JUCE_CALLTYPE copyWithMultiply (FloatType* dest, const FloatType* src, FloatType multiplier, CountType numValues) noexcept;
    static void JUCE_CALLTYPE add (FloatType* dest, FloatType amountToAdd, CountType numValues) noexcept;
    static void JUCE_CALLTYPE add (FloatType* dest, const FloatType* src, FloatType amount, CountType numValues) noexcept;
    static void JUCE_CALLTYPE add (FloatType* dest, const FloatType* src, CountType numValues) noexcept;
    static void JUCE_CALLTYPE add (FloatType* dest, const FloatType* src1, const FloatType* src2, CountType num) noexcept;
    static void JUCE_CALLTYPE subtract (FloatType* dest, const FloatType* src, CountType numValues) noexcept;
    static void JUCE_CALLTYPE subtract (FloatType* dest, const FloatType* src1, const FloatType* src2, CountType num) noexcept;
    static void JUCE_CALLTYPE addWithMultiply (FloatType* dest, const FloatType* src, FloatType multiplier, CountType numValues) noexcept;
    static void JUCE_CALLTYPE addWithMultiply (FloatType* dest, const FloatType* src1, const FloatType* src2, CountType num) noexcept;
    static void JUCE_CALLTYPE subtractWithMultiply (FloatType* dest, const FloatType* src, FloatType multiplier, CountType numValues) noexcept;
    static void JUCE_CALLTYPE subtractWithMultiply (FloatType* dest, const FloatType* src1, const FloatType* src2, CountType num) noexcept;
    static void JUCE_CALLTYPE multiply (FloatType* dest, const FloatType* src, CountType numValues) noexcept;
    static void JUCE_CALLTYPE multiply (FloatType* dest, const FloatType* src1, const FloatType* src2, CountType numValues) noexcept;
    static void JUCE_CALLTYPE multiply (FloatType* dest, FloatType multiplier, CountType numValues) noexcept;
    static void JUCE_CALLTYPE multiply (FloatType* dest, const FloatType* src, FloatType multiplier, CountType num) noexcept;
    static void JUCE_CALLTYPE negate (FloatType* dest, const FloatType* src, CountType numValues) noexcept;
    static void JUCE_CALLTYPE abs (FloatType* dest, const FloatType* src, CountType numValues) noexcept;
    static void JUCE_CALLTYPE min (FloatType* dest, const FloatType* src, FloatType comp, CountType num) noexcept;
    static void JUCE_CALLTYPE min (FloatType* dest, const FloatType* src1, const FloatType* src2, CountType num) noexcept;
    static void JUCE_CALLTYPE max (FloatType* dest, const FloatType* src, FloatType comp, CountType num) noexcept;
    static void JUCE_CALLTYPE max (FloatType* dest, const FloatType* src1, const FloatType* src2, CountType num) noexcept;
    static void JUCE_CALLTYPE clip (FloatType* dest, const FloatType* src, FloatType low, FloatType high, CountType num) noexcept;
    static Range<FloatType> JUCE_CALLTYPE findMinAndMax (const FloatType* src, CountType numValues) noexcept;
    static FloatType JUCE_CALLTYPE findMinimum (const FloatType* src, CountType numValues) noexcept;
    static FloatType JUCE_CALLTYPE findMaximum (const FloatType* src, CountType numValues) noexcept;
};

template <typename...>
struct NameForwarder;

template <typename Head>
struct NameForwarder<Head> : Head {};

template <typename Head, typename... Tail>
struct NameForwarder<Head, Tail...> : Head, NameForwarder<Tail...>
{
    using Head::clear;
    using NameForwarder<Tail...>::clear;

    using Head::fill;
    using NameForwarder<Tail...>::fill;

    using Head::copy;
    using NameForwarder<Tail...>::copy;

    using Head::copyWithMultiply;
    using NameForwarder<Tail...>::copyWithMultiply;

    using Head::add;
    using NameForwarder<Tail...>::add;

    using Head::subtract;
    using NameForwarder<Tail...>::subtract;

    using Head::addWithMultiply;
    using NameForwarder<Tail...>::addWithMultiply;

    using Head::subtractWithMultiply;
    using NameForwarder<Tail...>::subtractWithMultiply;

    using Head::multiply;
    using NameForwarder<Tail...>::multiply;

    using Head::negate;
    using NameForwarder<Tail...>::negate;

    using Head::abs;
    using NameForwarder<Tail...>::abs;

    using Head::min;
    using NameForwarder<Tail...>::min;

    using Head::max;
    using NameForwarder<Tail...>::max;

    using Head::clip;
    using NameForwarder<Tail...>::clip;

    using Head::findMinAndMax;
    using NameForwarder<Tail...>::findMinAndMax;

    using Head::findMinimum;
    using NameForwarder<Tail...>::findMinimum;

    using Head::findMaximum;
    using NameForwarder<Tail...>::findMaximum;
};

} // namespace detail
#endif

//==============================================================================
/**
    A collection of simple vector operations on arrays of floats, accelerated with
    SIMD instructions where possible.

    @tags{Audio}
*/
class JUCE_API  FloatVectorOperations : public detail::NameForwarder<detail::FloatVectorOperationsBase<float, int>,
                                                                     detail::FloatVectorOperationsBase<float, size_t>,
                                                                     detail::FloatVectorOperationsBase<double, int>,
                                                                     detail::FloatVectorOperationsBase<double, size_t>>
{
public:
    static void JUCE_CALLTYPE convertFixedToFloat (float* dest, const int* src, float multiplier, int num) noexcept;

    static void JUCE_CALLTYPE convertFixedToFloat (float* dest, const int* src, float multiplier, size_t num) noexcept;

    /** This method enables or disables the SSE/NEON flush-to-zero mode. */
    static void JUCE_CALLTYPE enableFlushToZeroMode (bool shouldEnable) noexcept;

    /** On Intel CPUs, this method enables the SSE flush-to-zero and denormalised-are-zero modes.
        This effectively sets the DAZ and FZ bits of the MXCSR register. On arm CPUs this will
        enable flush to zero mode.
        It's a convenient thing to call before audio processing code where you really want to
        avoid denormalisation performance hits.
    */
    static void JUCE_CALLTYPE disableDenormalisedNumberSupport (bool shouldDisable = true) noexcept;

    /** This method returns true if denormals are currently disabled. */
    static bool JUCE_CALLTYPE areDenormalsDisabled() noexcept;

private:
    friend ScopedNoDenormals;

    static intptr_t JUCE_CALLTYPE getFpStatusRegister() noexcept;
    static void JUCE_CALLTYPE setFpStatusRegister (intptr_t) noexcept;
};

//==============================================================================
/**
     Helper class providing an RAII-based mechanism for temporarily disabling
     denormals on your CPU.

    @tags{Audio}
*/
class ScopedNoDenormals
{
public:
    ScopedNoDenormals() noexcept;
    ~ScopedNoDenormals() noexcept;

private:
  #if JUCE_USE_SSE_INTRINSICS || (JUCE_USE_ARM_NEON || defined (__arm64__) || defined (__aarch64__))
    intptr_t fpsr;
  #endif
};

} // namespace juce
