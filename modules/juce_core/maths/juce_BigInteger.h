/*
  ==============================================================================

   This file is part of the juce_core module of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission to use, copy, modify, and/or distribute this software for any purpose with
   or without fee is hereby granted, provided that the above copyright notice and this
   permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD
   TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN
   NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
   DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
   IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
   CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

   ------------------------------------------------------------------------------

   NOTE! This permissive ISC license applies ONLY to files within the juce_core module!
   All other JUCE modules are covered by a dual GPL/commercial license, so if you are
   using any other modules, be sure to check that you also comply with their license.

   For more details, visit www.juce.com

  ==============================================================================
*/

#ifndef JUCE_BIGINTEGER_H_INCLUDED
#define JUCE_BIGINTEGER_H_INCLUDED


//==============================================================================
/**
    An arbitrarily large integer class.

    A BigInteger can be used in a similar way to a normal integer, but has no size
    limit (except for memory and performance constraints).

    Negative values are possible, but the value isn't stored as 2s-complement, so
    be careful if you use negative values and look at the values of individual bits.
*/
class JUCE_API  BigInteger
{
public:
    //==============================================================================
    /** Creates an empty BigInteger */
    BigInteger();

    /** Creates a BigInteger containing an integer value in its low bits.
        The low 32 bits of the number are initialised with this value.
    */
    BigInteger (uint32 value);

    /** Creates a BigInteger containing an integer value in its low bits.
        The low 32 bits of the number are initialised with the absolute value
        passed in, and its sign is set to reflect the sign of the number.
    */
    BigInteger (int32 value);

    /** Creates a BigInteger containing an integer value in its low bits.
        The low 64 bits of the number are initialised with the absolute value
        passed in, and its sign is set to reflect the sign of the number.
    */
    BigInteger (int64 value);

    /** Creates a copy of another BigInteger. */
    BigInteger (const BigInteger&);

   #if JUCE_COMPILER_SUPPORTS_MOVE_SEMANTICS
    BigInteger (BigInteger&&) noexcept;
    BigInteger& operator= (BigInteger&&) noexcept;
   #endif

    /** Destructor. */
    ~BigInteger();

    //==============================================================================
    /** Copies another BigInteger onto this one. */
    BigInteger& operator= (const BigInteger&);

    /** Swaps the internal contents of this with another object. */
    void swapWith (BigInteger&) noexcept;

    //==============================================================================
    /** Returns the value of a specified bit in the number.
        If the index is out-of-range, the result will be false.
    */
    bool operator[] (int bit) const noexcept;

    /** Returns true if no bits are set. */
    bool isZero() const noexcept;

    /** Returns true if the value is 1. */
    bool isOne() const noexcept;

    /** Attempts to get the lowest 32 bits of the value as an integer.
        If the value is bigger than the integer limits, this will return only the lower bits.
    */
    int toInteger() const noexcept;

    /** Attempts to get the lowest 64 bits of the value as an integer.
        If the value is bigger than the integer limits, this will return only the lower bits.
    */
    int64 toInt64() const noexcept;

    //==============================================================================
    /** Resets the value to 0. */
    void clear();

    /** Clears a particular bit in the number. */
    void clearBit (int bitNumber) noexcept;

    /** Sets a specified bit to 1. */
    void setBit (int bitNumber);

    /** Sets or clears a specified bit. */
    void setBit (int bitNumber, bool shouldBeSet);

    /** Sets a range of bits to be either on or off.

        @param startBit     the first bit to change
        @param numBits      the number of bits to change
        @param shouldBeSet  whether to turn these bits on or off
    */
    void setRange (int startBit, int numBits, bool shouldBeSet);

    /** Inserts a bit an a given position, shifting up any bits above it. */
    void insertBit (int bitNumber, bool shouldBeSet);

    /** Returns a range of bits as a new BigInteger.

        e.g. getBitRangeAsInt (0, 64) would return the lowest 64 bits.
        @see getBitRangeAsInt
    */
    BigInteger getBitRange (int startBit, int numBits) const;

    /** Returns a range of bits as an integer value.

        e.g. getBitRangeAsInt (0, 32) would return the lowest 32 bits.

        Asking for more than 32 bits isn't allowed (obviously) - for that, use
        getBitRange().
    */
    uint32 getBitRangeAsInt (int startBit, int numBits) const noexcept;

    /** Sets a range of bits to an integer value.

        Copies the given integer onto a range of bits, starting at startBit,
        and using up to numBits of the available bits.
    */
    void setBitRangeAsInt (int startBit, int numBits, uint32 valueToSet);

    /** Shifts a section of bits left or right.

        @param howManyBitsLeft  how far to move the bits (+ve numbers shift it left, -ve numbers shift it right).
        @param startBit         the first bit to affect - if this is > 0, only bits above that index will be affected.
    */
    void shiftBits (int howManyBitsLeft, int startBit);

    /** Returns the total number of set bits in the value. */
    int countNumberOfSetBits() const noexcept;

    /** Looks for the index of the next set bit after a given starting point.

        This searches from startIndex (inclusive) upwards for the first set bit,
        and returns its index. If no set bits are found, it returns -1.
    */
    int findNextSetBit (int startIndex) const noexcept;

    /** Looks for the index of the next clear bit after a given starting point.

        This searches from startIndex (inclusive) upwards for the first clear bit,
        and returns its index.
    */
    int findNextClearBit (int startIndex) const noexcept;

    /** Returns the index of the highest set bit in the number.
        If the value is zero, this will return -1.
    */
    int getHighestBit() const noexcept;

    //==============================================================================
    // All the standard arithmetic ops...

    BigInteger& operator+= (const BigInteger&);
    BigInteger& operator-= (const BigInteger&);
    BigInteger& operator*= (const BigInteger&);
    BigInteger& operator/= (const BigInteger&);
    BigInteger& operator|= (const BigInteger&);
    BigInteger& operator&= (const BigInteger&);
    BigInteger& operator^= (const BigInteger&);
    BigInteger& operator%= (const BigInteger&);
    BigInteger& operator<<= (int numBitsToShift);
    BigInteger& operator>>= (int numBitsToShift);
    BigInteger& operator++();
    BigInteger& operator--();
    BigInteger operator++ (int);
    BigInteger operator-- (int);

    BigInteger operator-() const;
    BigInteger operator+ (const BigInteger&) const;
    BigInteger operator- (const BigInteger&) const;
    BigInteger operator* (const BigInteger&) const;
    BigInteger operator/ (const BigInteger&) const;
    BigInteger operator| (const BigInteger&) const;
    BigInteger operator& (const BigInteger&) const;
    BigInteger operator^ (const BigInteger&) const;
    BigInteger operator% (const BigInteger&) const;
    BigInteger operator<< (int numBitsToShift) const;
    BigInteger operator>> (int numBitsToShift) const;

    bool operator== (const BigInteger&) const noexcept;
    bool operator!= (const BigInteger&) const noexcept;
    bool operator<  (const BigInteger&) const noexcept;
    bool operator<= (const BigInteger&) const noexcept;
    bool operator>  (const BigInteger&) const noexcept;
    bool operator>= (const BigInteger&) const noexcept;

    //==============================================================================
    /** Does a signed comparison of two BigIntegers.

        Return values are:
            - 0 if the numbers are the same
            - < 0 if this number is smaller than the other
            - > 0 if this number is bigger than the other
    */
    int compare (const BigInteger& other) const noexcept;

    /** Compares the magnitudes of two BigIntegers, ignoring their signs.

        Return values are:
            - 0 if the numbers are the same
            - < 0 if this number is smaller than the other
            - > 0 if this number is bigger than the other
    */
    int compareAbsolute (const BigInteger& other) const noexcept;

    /** Divides this value by another one and returns the remainder.

        This number is divided by other, leaving the quotient in this number,
        with the remainder being copied to the other BigInteger passed in.
    */
    void divideBy (const BigInteger& divisor, BigInteger& remainder);

    /** Returns the largest value that will divide both this value and the one passed-in. */
    BigInteger findGreatestCommonDivisor (BigInteger other) const;

    /** Performs a combined exponent and modulo operation.
        This BigInteger's value becomes (this ^ exponent) % modulus.
    */
    void exponentModulo (const BigInteger& exponent, const BigInteger& modulus);

    /** Performs an inverse modulo on the value.
        i.e. the result is (this ^ -1) mod (modulus).
    */
    void inverseModulo (const BigInteger& modulus);

    //==============================================================================
    /** Returns true if the value is less than zero.
        @see setNegative, negate
    */
    bool isNegative() const noexcept;

    /** Changes the sign of the number to be positive or negative.
        @see isNegative, negate
    */
    void setNegative (bool shouldBeNegative) noexcept;

    /** Inverts the sign of the number.
        @see isNegative, setNegative
    */
    void negate() noexcept;

    //==============================================================================
    /** Converts the number to a string.

        Specify a base such as 2 (binary), 8 (octal), 10 (decimal), 16 (hex).
        If minimumNumCharacters is greater than 0, the returned string will be
        padded with leading zeros to reach at least that length.
    */
    String toString (int base, int minimumNumCharacters = 1) const;

    /** Reads the numeric value from a string.

        Specify a base such as 2 (binary), 8 (octal), 10 (decimal), 16 (hex).
        Any invalid characters will be ignored.
    */
    void parseString (StringRef text, int base);

    //==============================================================================
    /** Turns the number into a block of binary data.

        The data is arranged as little-endian, so the first byte of data is the low 8 bits
        of the number, and so on.

        @see loadFromMemoryBlock
    */
    MemoryBlock toMemoryBlock() const;

    /** Converts a block of raw data into a number.

        The data is arranged as little-endian, so the first byte of data is the low 8 bits
        of the number, and so on.

        @see toMemoryBlock
    */
    void loadFromMemoryBlock (const MemoryBlock& data);

private:
    //==============================================================================
    HeapBlock<uint32> values;
    size_t numValues;
    int highestBit;
    bool negative;

    void ensureSize (size_t);
    void shiftLeft (int bits, int startBit);
    void shiftRight (int bits, int startBit);

    JUCE_LEAK_DETECTOR (BigInteger)
};

/** Writes a BigInteger to an OutputStream as a UTF8 decimal string. */
OutputStream& JUCE_CALLTYPE operator<< (OutputStream& stream, const BigInteger& value);

//==============================================================================
#ifndef DOXYGEN
 // For backwards compatibility, BitArray is defined as an alias for BigInteger.
 typedef BigInteger BitArray;
#endif


#endif   // JUCE_BIGINTEGER_H_INCLUDED
