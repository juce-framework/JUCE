/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

#ifndef __JUCE_BITARRAY_JUCEHEADER__
#define __JUCE_BITARRAY_JUCEHEADER__

#include "../text/juce_String.h"
#include "juce_Array.h"
#include "juce_HeapBlock.h"
class MemoryBlock;


//==============================================================================
/**
    An array of on/off bits, also usable to store large binary integers.

    A BitArray acts like an arbitrarily large integer whose bits can be set or
    cleared, and some basic mathematical operations can be done on the number as
    a whole.
*/
class JUCE_API  BitArray
{
public:
    //==============================================================================
    /** Creates an empty BitArray */
    BitArray() throw();

    /** Creates a BitArray containing an integer value in its low bits.

        The low 32 bits of the array are initialised with this value.
    */
    BitArray (const unsigned int value) throw();

    /** Creates a BitArray containing an integer value in its low bits.

        The low 32 bits of the array are initialised with the absolute value
        passed in, and its sign is set to reflect the sign of the number.
    */
    BitArray (const int value) throw();

    /** Creates a BitArray containing an integer value in its low bits.

        The low 64 bits of the array are initialised with the absolute value
        passed in, and its sign is set to reflect the sign of the number.
    */
    BitArray (int64 value) throw();

    /** Creates a copy of another BitArray. */
    BitArray (const BitArray& other) throw();

    /** Destructor. */
    ~BitArray() throw();

    //==============================================================================
    /** Copies another BitArray onto this one. */
    const BitArray& operator= (const BitArray& other) throw();

    /** Two arrays are the same if the same bits are set. */
    bool operator== (const BitArray& other) const throw();
    /** Two arrays are the same if the same bits are set. */
    bool operator!= (const BitArray& other) const throw();

    //==============================================================================
    /** Clears all bits in the BitArray to 0. */
    void clear() throw();

    /** Clears a particular bit in the array. */
    void clearBit (const int bitNumber) throw();

    /** Sets a specified bit to 1.

        If the bit number is high, this will grow the array to accomodate it.
    */
    void setBit (const int bitNumber) throw();

    /** Sets or clears a specified bit. */
    void setBit (const int bitNumber,
                 const bool shouldBeSet) throw();

    /** Sets a range of bits to be either on or off.

        @param startBit     the first bit to change
        @param numBits      the number of bits to change
        @param shouldBeSet  whether to turn these bits on or off
    */
    void setRange (int startBit,
                   int numBits,
                   const bool shouldBeSet) throw();

    /** Inserts a bit an a given position, shifting up any bits above it. */
    void insertBit (const int bitNumber,
                    const bool shouldBeSet) throw();

    /** Returns the value of a specified bit in the array.

        If the index is out-of-range, the result will be false.
    */
    bool operator[] (const int bit) const throw();

    /** Returns true if no bits are set. */
    bool isEmpty() const throw();

    //==============================================================================
    /** Returns a range of bits in the array as a new BitArray.

        e.g. getBitRangeAsInt (0, 64) would return the lowest 64 bits.
        @see getBitRangeAsInt
    */
    const BitArray getBitRange (int startBit, int numBits) const throw();

    /** Returns a range of bits in the array as an integer value.

        e.g. getBitRangeAsInt (0, 32) would return the lowest 32 bits.

        Asking for more than 32 bits isn't allowed (obviously) - for that, use
        getBitRange().
    */
    int getBitRangeAsInt (int startBit, int numBits) const throw();

    /** Sets a range of bits in the array based on an integer value.

        Copies the given integer into the array, starting at startBit,
        and only using up to numBits of the available bits.
    */
    void setBitRangeAsInt (int startBit, int numBits,
                           unsigned int valueToSet) throw();

    //==============================================================================
    /** Performs a bitwise OR with another BitArray.

        The result ends up in this array.
    */
    void orWith (const BitArray& other) throw();

    /** Performs a bitwise AND with another BitArray.

        The result ends up in this array.
    */
    void andWith (const BitArray& other) throw();

    /** Performs a bitwise XOR with another BitArray.

        The result ends up in this array.
    */
    void xorWith (const BitArray& other) throw();

    /** Adds another BitArray's value to this one.

        Treating the two arrays as large positive integers, this
        adds them up and puts the result in this array.
    */
    void add (const BitArray& other) throw();

    /** Subtracts another BitArray's value from this one.

        Treating the two arrays as large positive integers, this
        subtracts them and puts the result in this array.

        Note that if the result should be negative, this won't be
        handled correctly.
    */
    void subtract (const BitArray& other) throw();

    /** Multiplies another BitArray's value with this one.

        Treating the two arrays as large positive integers, this
        multiplies them and puts the result in this array.
    */
    void multiplyBy (const BitArray& other) throw();

    /** Divides another BitArray's value into this one and also produces a remainder.

        Treating the two arrays as large positive integers, this
        divides this value by the other, leaving the quotient in this
        array, and the remainder is copied into the other BitArray passed in.
    */
    void divideBy (const BitArray& divisor, BitArray& remainder) throw();

    /** Returns the largest value that will divide both this value and the one
        passed-in.
    */
    const BitArray findGreatestCommonDivisor (BitArray other) const throw();

    /** Performs a modulo operation on this value.

        The result is stored in this value.
    */
    void modulo (const BitArray& divisor) throw();

    /** Performs a combined exponent and modulo operation.

        This BitArray's value becomes (this ^ exponent) % modulus.
    */
    void exponentModulo (const BitArray& exponent, const BitArray& modulus) throw();

    /** Performs an inverse modulo on the value.

        i.e. the result is (this ^ -1) mod (modulus).
    */
    void inverseModulo (const BitArray& modulus) throw();

    /** Shifts a section of bits left or right.

        @param howManyBitsLeft  how far to move the bits (+ve numbers shift it left, -ve numbers shift it right).
        @param startBit         the first bit to affect - if this is > 0, only bits above that index will be affected.
    */
    void shiftBits (int howManyBitsLeft,
                    int startBit = 0) throw();

    /** Does a signed comparison of two BitArrays.

        Return values are:
            - 0 if the numbers are the same
            - < 0 if this number is smaller than the other
            - > 0 if this number is bigger than the other
    */
    int compare (const BitArray& other) const throw();

    /** Compares the magnitudes of two BitArrays, ignoring their signs.

        Return values are:
            - 0 if the numbers are the same
            - < 0 if this number is smaller than the other
            - > 0 if this number is bigger than the other
    */
    int compareAbsolute (const BitArray& other) const throw();

    //==============================================================================
    /** Returns true if the value is less than zero.

        @see setNegative, negate
    */
    bool isNegative() const throw();

    /** Changes the sign of the number to be positive or negative.

        @see isNegative, negate
    */
    void setNegative (const bool shouldBeNegative) throw();

    /** Inverts the sign of the number.

        @see isNegative, setNegative
    */
    void negate() throw();

    //==============================================================================
    /** Counts the total number of set bits in the array. */
    int countNumberOfSetBits() const throw();

    /** Looks for the index of the next set bit after a given starting point.

        searches from startIndex (inclusive) upwards for the first set bit,
        and returns its index.

        If no set bits are found, it returns -1.
    */
    int findNextSetBit (int startIndex = 0) const throw();

    /** Looks for the index of the next clear bit after a given starting point.

        searches from startIndex (inclusive) upwards for the first clear bit,
        and returns its index.
    */
    int findNextClearBit (int startIndex = 0) const throw();

    /** Returns the index of the highest set bit in the array.

        If the array is empty, this will return -1.
    */
    int getHighestBit() const throw();

    //==============================================================================
    /** Converts the array to a number string.

        Specify a base such as 2 (binary), 8 (octal), 10 (decimal), 16 (hex).

        If minuimumNumCharacters is greater than 0, the returned string will be
        padded with leading zeros to reach at least that length.
    */
    const String toString (const int base, const int minimumNumCharacters = 1) const throw();

    /** Converts a number string to an array.

        Any non-valid characters will be ignored.

        Specify a base such as 2 (binary), 8 (octal), 10 (decimal), 16 (hex).
    */
    void parseString (const String& text,
                      const int base) throw();

    //==============================================================================
    /** Turns the array into a block of binary data.

        The data is arranged as little-endian, so the first byte of data is the low 8 bits
        of the array, and so on.

        @see loadFromMemoryBlock
    */
    const MemoryBlock toMemoryBlock() const throw();

    /** Copies a block of raw data onto this array.

        The data is arranged as little-endian, so the first byte of data is the low 8 bits
        of the array, and so on.

        @see toMemoryBlock
    */
    void loadFromMemoryBlock (const MemoryBlock& data) throw();

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    void ensureSize (const int numVals) throw();
    HeapBlock <unsigned int> values;
    int numValues, highestBit;
    bool negative;
};


#endif   // __JUCE_BITARRAY_JUCEHEADER__
