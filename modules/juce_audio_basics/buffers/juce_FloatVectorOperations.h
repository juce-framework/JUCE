/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#ifndef __JUCE_FLOATVECTOROPERATIONS_JUCEHEADER__
#define __JUCE_FLOATVECTOROPERATIONS_JUCEHEADER__


//==============================================================================
/**
    A collection of simple vector operations on arrays of floats, accelerated with
    SIMD instructions where possible.
*/
class JUCE_API  FloatVectorOperations
{
public:
    //==============================================================================
    /** Clears a vector of floats. */
    static void JUCE_CALLTYPE clear (float* dest, int numValues) noexcept;

    /** Copies a repeated value into a vector of floats. */
    static void JUCE_CALLTYPE fill (float* dest, float valueToFill, int numValues) noexcept;

    /** Copies a vector of floats. */
    static void JUCE_CALLTYPE copy (float* dest, const float* src, int numValues) noexcept;

    /** Copies a vector of floats, multiplying each value by a given multiplier */
    static void JUCE_CALLTYPE copyWithMultiply (float* dest, const float* src, float multiplier, int numValues) noexcept;

    /** Adds the source values to the destination values. */
    static void JUCE_CALLTYPE add (float* dest, const float* src, int numValues) noexcept;

    /** Adds a fixed value to the destination values. */
    static void JUCE_CALLTYPE add (float* dest, float amount, int numValues) noexcept;

    /** Multiplies each source value by the given multiplier, then adds it to the destination value. */
    static void JUCE_CALLTYPE addWithMultiply (float* dest, const float* src, float multiplier, int numValues) noexcept;

    /** Multiplies the destination values by the source values. */
    static void JUCE_CALLTYPE multiply (float* dest, const float* src, int numValues) noexcept;

    /** Multiplies each of the destination values by a fixed multiplier. */
    static void JUCE_CALLTYPE multiply (float* dest, float multiplier, int numValues) noexcept;

    /** Converts a stream of integers to floats, multiplying each one by the given multiplier. */
    static void JUCE_CALLTYPE convertFixedToFloat (float* dest, const int* src, float multiplier, int numValues) noexcept;

    /** Finds the miniumum and maximum values in the given array. */
    static void JUCE_CALLTYPE findMinAndMax (const float* src, int numValues, float& minResult, float& maxResult) noexcept;

    /** Finds the miniumum value in the given array. */
    static float JUCE_CALLTYPE findMinimum (const float* src, int numValues) noexcept;

    /** Finds the maximum value in the given array. */
    static float JUCE_CALLTYPE findMaximum (const float* src, int numValues) noexcept;
};


#endif   // __JUCE_FLOATVECTOROPERATIONS_JUCEHEADER__
