/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

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

//==============================================================================
/**
    A collection of simple vector operations on arrays of floating point numbers,
    accelerated with SIMD instructions where possible, usually accessed from
    the FloatVectorOperations class.

    @code
    float data[64];

    // The following two function calls are equivalent:
    FloatVectorOperationsBase<float, int>::clear (data, 64);
    FloatVectorOperations::clear (data, 64);
    @endcode

    @see FloatVectorOperations

    @tags{Audio}
*/
template <typename FloatType, typename CountType>
struct FloatVectorOperationsBase
{
    /** Clears a vector of floating point numbers. */
    static void JUCE_CALLTYPE clear (FloatType* dest, CountType numValues) noexcept;

    /** Copies a repeated value into a vector of floating point numbers. */
    static void JUCE_CALLTYPE fill (FloatType* dest, FloatType valueToFill, CountType numValues) noexcept;

    /** Copies a vector of floating point numbers. */
    static void JUCE_CALLTYPE copy (FloatType* dest, const FloatType* src, CountType numValues) noexcept;

    /** Copies a vector of floating point numbers, multiplying each value by a given multiplier */
    static void JUCE_CALLTYPE copyWithMultiply (FloatType* dest, const FloatType* src, FloatType multiplier, CountType numValues) noexcept;

    /** Adds a fixed value to the destination values. */
    static void JUCE_CALLTYPE add (FloatType* dest, FloatType amountToAdd, CountType numValues) noexcept;

    /** Adds a fixed value to each source value and stores it in the destination array. */
    static void JUCE_CALLTYPE add (FloatType* dest, const FloatType* src, FloatType amount, CountType numValues) noexcept;

    /** Adds the source values to the destination values. */
    static void JUCE_CALLTYPE add (FloatType* dest, const FloatType* src, CountType numValues) noexcept;

    /** Adds each source1 value to the corresponding source2 value and stores the result in the destination array. */
    static void JUCE_CALLTYPE add (FloatType* dest, const FloatType* src1, const FloatType* src2, CountType num) noexcept;

    /** Subtracts the source values from the destination values. */
    static void JUCE_CALLTYPE subtract (FloatType* dest, const FloatType* src, CountType numValues) noexcept;

    /** Subtracts each source2 value from the corresponding source1 value and stores the result in the destination array. */
    static void JUCE_CALLTYPE subtract (FloatType* dest, const FloatType* src1, const FloatType* src2, CountType num) noexcept;

    /** Multiplies each source value by the given multiplier, then adds it to the destination value. */
    static void JUCE_CALLTYPE addWithMultiply (FloatType* dest, const FloatType* src, FloatType multiplier, CountType numValues) noexcept;

    /** Multiplies each source1 value by the corresponding source2 value, then adds it to the destination value. */
    static void JUCE_CALLTYPE addWithMultiply (FloatType* dest, const FloatType* src1, const FloatType* src2, CountType num) noexcept;

    /** Multiplies each source value by the given multiplier, then subtracts it to the destination value. */
    static void JUCE_CALLTYPE subtractWithMultiply (FloatType* dest, const FloatType* src, FloatType multiplier, CountType numValues) noexcept;

    /** Multiplies each source1 value by the corresponding source2 value, then subtracts it to the destination value. */
    static void JUCE_CALLTYPE subtractWithMultiply (FloatType* dest, const FloatType* src1, const FloatType* src2, CountType num) noexcept;

    /** Multiplies the destination values by the source values. */
    static void JUCE_CALLTYPE multiply (FloatType* dest, const FloatType* src, CountType numValues) noexcept;

    /** Multiplies each source1 value by the corresponding source2 value, then stores it in the destination array. */
    static void JUCE_CALLTYPE multiply (FloatType* dest, const FloatType* src1, const FloatType* src2, CountType numValues) noexcept;

    /** Multiplies each of the destination values by a fixed multiplier. */
    static void JUCE_CALLTYPE multiply (FloatType* dest, FloatType multiplier, CountType numValues) noexcept;

    /** Multiplies each of the source values by a fixed multiplier and stores the result in the destination array. */
    static void JUCE_CALLTYPE multiply (FloatType* dest, const FloatType* src, FloatType multiplier, CountType num) noexcept;

    /** Copies a source vector to a destination, negating each value. */
    static void JUCE_CALLTYPE negate (FloatType* dest, const FloatType* src, CountType numValues) noexcept;

    /** Copies a source vector to a destination, taking the absolute of each value. */
    static void JUCE_CALLTYPE abs (FloatType* dest, const FloatType* src, CountType numValues) noexcept;

    /** Each element of dest will be the minimum of the corresponding element of the source array and the given comp value. */
    static void JUCE_CALLTYPE min (FloatType* dest, const FloatType* src, FloatType comp, CountType num) noexcept;

    /** Each element of dest will be the minimum of the corresponding source1 and source2 values. */
    static void JUCE_CALLTYPE min (FloatType* dest, const FloatType* src1, const FloatType* src2, CountType num) noexcept;

    /** Each element of dest will be the maximum of the corresponding element of the source array and the given comp value. */
    static void JUCE_CALLTYPE max (FloatType* dest, const FloatType* src, FloatType comp, CountType num) noexcept;

    /** Each element of dest will be the maximum of the corresponding source1 and source2 values. */
    static void JUCE_CALLTYPE max (FloatType* dest, const FloatType* src1, const FloatType* src2, CountType num) noexcept;

    /** Each element of dest is calculated by hard clipping the corresponding src element so that it is in the range specified by the arguments low and high. */
    static void JUCE_CALLTYPE clip (FloatType* dest, const FloatType* src, FloatType low, FloatType high, CountType num) noexcept;

    /** Finds the minimum and maximum values in the given array. */
    static Range<FloatType> JUCE_CALLTYPE findMinAndMax (const FloatType* src, CountType numValues) noexcept;

    /** Finds the minimum value in the given array. */
    static FloatType JUCE_CALLTYPE findMinimum (const FloatType* src, CountType numValues) noexcept;

    /** Finds the maximum value in the given array. */
    static FloatType JUCE_CALLTYPE findMaximum (const FloatType* src, CountType numValues) noexcept;
};

#if ! DOXYGEN
namespace detail
{

template <typename... Bases>
struct NameForwarder : public Bases...
{
    using Bases::clear...,
          Bases::fill...,
          Bases::copy...,
          Bases::copyWithMultiply...,
          Bases::add...,
          Bases::subtract...,
          Bases::addWithMultiply...,
          Bases::subtractWithMultiply...,
          Bases::multiply...,
          Bases::negate...,
          Bases::abs...,
          Bases::min...,
          Bases::max...,
          Bases::clip...,
          Bases::findMinAndMax...,
          Bases::findMinimum...,
          Bases::findMaximum...;
};

} // namespace detail
#endif

//==============================================================================
/**
    A collection of simple vector operations on arrays of floating point numbers,
    accelerated with SIMD instructions where possible and providing all methods
    from FloatVectorOperationsBase.

    @see FloatVectorOperationsBase

    @tags{Audio}
*/
class JUCE_API  FloatVectorOperations : public detail::NameForwarder<FloatVectorOperationsBase<float, int>,
                                                                     FloatVectorOperationsBase<float, size_t>,
                                                                     FloatVectorOperationsBase<double, int>,
                                                                     FloatVectorOperationsBase<double, size_t>>
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
  #if JUCE_USE_SSE_INTRINSICS || (JUCE_USE_ARM_NEON || (JUCE_64BIT && JUCE_ARM))
    intptr_t fpsr;
  #endif
};

} // namespace juce
