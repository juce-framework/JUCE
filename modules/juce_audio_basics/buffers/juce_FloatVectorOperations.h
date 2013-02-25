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
*/
class JUCE_API  FloatVectorOperations
{
public:
    //==============================================================================
    /** Clears a vector of floats. */
    static void clear (float* dest, int numValues) noexcept;

    /** Copies a vector of floats. */
    static void copy (float* dest, const float* src, int numValues) noexcept;

    /** Copies a vector of floats, multiplying each value by a given multiplier */
    static void copyWithMultiply (float* dest, const float* src, float multiplier, int numValues) noexcept;

    /** Adds the source values to the destination values. */
    static void add (float* dest, const float* src, int numValues) noexcept;

    /** Adds a fixed value to the destination values. */
    static void add (float* dest, float amount, int numValues) noexcept;

    /** Multiplies each source value by the given multiplier, then adds it to the destination value. */
    static void addWithMultiply (float* dest, const float* src, float multiplier, int numValues) noexcept;

    /** Multiplies the destination values by the source values. */
    static void multiply (float* dest, const float* src, int numValues) noexcept;

    /** Multiplies each of the destination values by a fixed multiplier. */
    static void multiply (float* dest, float multiplier, int numValues) noexcept;

    /** Converts a stream of integers to floats, multiplying each one by the given multiplier. */
    static void convertFixedToFloat (float* dest, const int* src, float multiplier, int numValues) noexcept;

    /** Finds the miniumum and maximum values in the given array. */
    static void findMinAndMax (const float* src, int numValues, float& minResult, float& maxResult) noexcept;
};


#endif   // __JUCE_FLOATVECTOROPERATIONS_JUCEHEADER__
