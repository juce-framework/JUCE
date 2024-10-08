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

namespace
{
    inline uint32 bitToMask  (const int bit) noexcept           { return (uint32) 1 << (bit & 31); }
    inline size_t bitToIndex (const int bit) noexcept           { return (size_t) (bit >> 5); }
    inline size_t sizeNeededToHold (int highestBit) noexcept    { return (size_t) (highestBit >> 5) + 1; }
}

int findHighestSetBit (uint32 n) noexcept
{
    jassert (n != 0); // (the built-in functions may not work for n = 0)

  #if JUCE_GCC || JUCE_CLANG
    return 31 - __builtin_clz (n);
  #elif JUCE_MSVC
    unsigned long highest;
    _BitScanReverse (&highest, n);
    return (int) highest;
  #else
    n |= (n >> 1);
    n |= (n >> 2);
    n |= (n >> 4);
    n |= (n >> 8);
    n |= (n >> 16);
    return countNumberOfBits (n >> 1);
  #endif
}

//==============================================================================
BigInteger::BigInteger()
    : allocatedSize (numPreallocatedInts)
{
    for (int i = 0; i < numPreallocatedInts; ++i)
        preallocated[i] = 0;
}

BigInteger::BigInteger (const int32 value)
    : allocatedSize (numPreallocatedInts),
      highestBit (31),
      negative (value < 0)
{
    preallocated[0] = (uint32) std::abs (value);

    for (int i = 1; i < numPreallocatedInts; ++i)
        preallocated[i] = 0;

    highestBit = getHighestBit();
}

BigInteger::BigInteger (const uint32 value)
    : allocatedSize (numPreallocatedInts),
      highestBit (31)
{
    preallocated[0] = value;

    for (int i = 1; i < numPreallocatedInts; ++i)
        preallocated[i] = 0;

    highestBit = getHighestBit();
}

BigInteger::BigInteger (int64 value)
    : allocatedSize (numPreallocatedInts),
      highestBit (63),
      negative (value < 0)
{
    if (value < 0)
        value = -value;

    preallocated[0] = (uint32) value;
    preallocated[1] = (uint32) (value >> 32);

    for (int i = 2; i < numPreallocatedInts; ++i)
        preallocated[i] = 0;

    highestBit = getHighestBit();
}

BigInteger::BigInteger (const BigInteger& other)
    : allocatedSize (other.allocatedSize),
      highestBit (other.getHighestBit()),
      negative (other.negative)
{
    if (allocatedSize > numPreallocatedInts)
        heapAllocation.malloc (allocatedSize);

    memcpy (getValues(), other.getValues(), sizeof (uint32) * allocatedSize);
}

BigInteger::BigInteger (BigInteger&& other) noexcept
    : heapAllocation (std::move (other.heapAllocation)),
      allocatedSize (other.allocatedSize),
      highestBit (other.highestBit),
      negative (other.negative)
{
    memcpy (preallocated, other.preallocated, sizeof (preallocated));
}

BigInteger& BigInteger::operator= (BigInteger&& other) noexcept
{
    heapAllocation = std::move (other.heapAllocation);
    memcpy (preallocated, other.preallocated, sizeof (preallocated));
    allocatedSize = other.allocatedSize;
    highestBit = other.highestBit;
    negative = other.negative;
    return *this;
}

void BigInteger::swapWith (BigInteger& other) noexcept
{
    for (int i = 0; i < numPreallocatedInts; ++i)
        std::swap (preallocated[i], other.preallocated[i]);

    heapAllocation.swapWith (other.heapAllocation);
    std::swap (allocatedSize, other.allocatedSize);
    std::swap (highestBit, other.highestBit);
    std::swap (negative, other.negative);
}

BigInteger& BigInteger::operator= (const BigInteger& other)
{
    if (this != &other)
    {
        highestBit = other.getHighestBit();
        auto newAllocatedSize = (size_t) jmax ((size_t) numPreallocatedInts, sizeNeededToHold (highestBit));

        if (newAllocatedSize <= numPreallocatedInts)
            heapAllocation.free();
        else if (newAllocatedSize != allocatedSize)
            heapAllocation.malloc (newAllocatedSize);

        allocatedSize = newAllocatedSize;

        memcpy (getValues(), other.getValues(), sizeof (uint32) * allocatedSize);
        negative = other.negative;
    }

    return *this;
}

BigInteger::~BigInteger() = default;

uint32* BigInteger::getValues() const noexcept
{
    jassert (heapAllocation != nullptr || allocatedSize <= numPreallocatedInts);

    return heapAllocation != nullptr ? heapAllocation
                                     : const_cast<uint32*> (preallocated);
}

uint32* BigInteger::ensureSize (const size_t numVals)
{
    if (numVals > allocatedSize)
    {
        auto oldSize = allocatedSize;
        allocatedSize = ((numVals + 2) * 3) / 2;

        if (heapAllocation == nullptr)
        {
            heapAllocation.calloc (allocatedSize);
            memcpy (heapAllocation, preallocated, sizeof (uint32) * numPreallocatedInts);
        }
        else
        {
            heapAllocation.realloc (allocatedSize);

            for (auto* values = getValues(); oldSize < allocatedSize; ++oldSize)
                values[oldSize] = 0;
        }
    }

    return getValues();
}

//==============================================================================
bool BigInteger::operator[] (const int bit) const noexcept
{
    return bit <= highestBit && bit >= 0
             && ((getValues() [bitToIndex (bit)] & bitToMask (bit)) != 0);
}

int BigInteger::toInteger() const noexcept
{
    auto n = (int) (getValues()[0] & 0x7fffffff);
    return negative ? -n : n;
}

int64 BigInteger::toInt64() const noexcept
{
    auto* values = getValues();
    auto n = (((int64) (values[1] & 0x7fffffff)) << 32) | values[0];
    return negative ? -n : n;
}

BigInteger BigInteger::getBitRange (int startBit, int numBits) const
{
    BigInteger r;
    numBits = jmax (0, jmin (numBits, getHighestBit() + 1 - startBit));
    auto* destValues = r.ensureSize (sizeNeededToHold (numBits));
    r.highestBit = numBits;

    for (int i = 0; numBits > 0;)
    {
        destValues[i++] = getBitRangeAsInt (startBit, (int) jmin (32, numBits));
        numBits -= 32;
        startBit += 32;
    }

    r.highestBit = r.getHighestBit();
    return r;
}

uint32 BigInteger::getBitRangeAsInt (const int startBit, int numBits) const noexcept
{
    if (numBits > 32)
    {
        jassertfalse;  // use getBitRange() if you need more than 32 bits..
        numBits = 32;
    }

    numBits = jmin (numBits, highestBit + 1 - startBit);

    if (numBits <= 0)
        return 0;

    auto pos = bitToIndex (startBit);
    auto offset = startBit & 31;
    auto endSpace = 32 - numBits;
    auto* values = getValues();

    auto n = ((uint32) values [pos]) >> offset;

    if (offset > endSpace)
        n |= ((uint32) values [pos + 1]) << (32 - offset);

    return n & (((uint32) 0xffffffff) >> endSpace);
}

BigInteger& BigInteger::setBitRangeAsInt (const int startBit, int numBits, uint32 valueToSet)
{
    if (numBits > 32)
    {
        jassertfalse;
        numBits = 32;
    }

    for (int i = 0; i < numBits; ++i)
    {
        setBit (startBit + i, (valueToSet & 1) != 0);
        valueToSet >>= 1;
    }

    return *this;
}

//==============================================================================
BigInteger& BigInteger::clear() noexcept
{
    heapAllocation.free();
    allocatedSize = numPreallocatedInts;
    highestBit = -1;
    negative = false;

    for (int i = 0; i < numPreallocatedInts; ++i)
        preallocated[i] = 0;

    return *this;
}

BigInteger& BigInteger::setBit (const int bit)
{
    if (bit >= 0)
    {
        if (bit > highestBit)
        {
            ensureSize (sizeNeededToHold (bit));
            highestBit = bit;
        }

        getValues() [bitToIndex (bit)] |= bitToMask (bit);
    }

    return *this;
}

BigInteger& BigInteger::setBit (const int bit, const bool shouldBeSet)
{
    if (shouldBeSet)
        setBit (bit);
    else
        clearBit (bit);

    return *this;
}

BigInteger& BigInteger::clearBit (const int bit) noexcept
{
    if (bit >= 0 && bit <= highestBit)
    {
        getValues() [bitToIndex (bit)] &= ~bitToMask (bit);

        if (bit == highestBit)
            highestBit = getHighestBit();
    }

    return *this;
}

BigInteger& BigInteger::setRange (int startBit, int numBits, const bool shouldBeSet)
{
    while (--numBits >= 0)
        setBit (startBit++, shouldBeSet);

    return *this;
}

BigInteger& BigInteger::insertBit (const int bit, const bool shouldBeSet)
{
    if (bit >= 0)
        shiftBits (1, bit);

    setBit (bit, shouldBeSet);
    return *this;
}

//==============================================================================
bool BigInteger::isZero() const noexcept
{
    return getHighestBit() < 0;
}

bool BigInteger::isOne() const noexcept
{
    return getHighestBit() == 0 && ! negative;
}

bool BigInteger::isNegative() const noexcept
{
    return negative && ! isZero();
}

void BigInteger::setNegative (const bool neg) noexcept
{
    negative = neg;
}

void BigInteger::negate() noexcept
{
    negative = (! negative) && ! isZero();
}

#if JUCE_MSVC && ! defined (__INTEL_COMPILER)
 #pragma intrinsic (_BitScanReverse)
#endif

int BigInteger::countNumberOfSetBits() const noexcept
{
    int total = 0;
    auto* values = getValues();

    for (int i = (int) sizeNeededToHold (highestBit); --i >= 0;)
        total += countNumberOfBits (values[i]);

    return total;
}

int BigInteger::getHighestBit() const noexcept
{
    auto* values = getValues();

    for (int i = (int) bitToIndex (highestBit); i >= 0; --i)
        if (uint32 n = values[i])
            return findHighestSetBit (n) + (i << 5);

    return -1;
}

int BigInteger::findNextSetBit (int i) const noexcept
{
    auto* values = getValues();

    for (; i <= highestBit; ++i)
        if ((values [bitToIndex (i)] & bitToMask (i)) != 0)
            return i;

    return -1;
}

int BigInteger::findNextClearBit (int i) const noexcept
{
    auto* values = getValues();

    for (; i <= highestBit; ++i)
        if ((values [bitToIndex (i)] & bitToMask (i)) == 0)
            break;

    return i;
}

//==============================================================================
BigInteger& BigInteger::operator+= (const BigInteger& other)
{
    if (this == &other)
        return operator+= (BigInteger (other));

    if (other.isNegative())
        return operator-= (-other);

    if (isNegative())
    {
        if (compareAbsolute (other) < 0)
        {
            auto temp = *this;
            temp.negate();
            *this = other;
            *this -= temp;
        }
        else
        {
            negate();
            *this -= other;
            negate();
        }
    }
    else
    {
        highestBit = jmax (highestBit, other.highestBit) + 1;

        auto numInts = sizeNeededToHold (highestBit);
        auto* values = ensureSize (numInts);
        auto* otherValues = other.getValues();
        int64 remainder = 0;

        for (size_t i = 0; i < numInts; ++i)
        {
            remainder += values[i];

            if (i < other.allocatedSize)
                remainder += otherValues[i];

            values[i] = (uint32) remainder;
            remainder >>= 32;
        }

        jassert (remainder == 0);
        highestBit = getHighestBit();
    }

    return *this;
}

BigInteger& BigInteger::operator-= (const BigInteger& other)
{
    if (this == &other)
    {
        clear();
        return *this;
    }

    if (other.isNegative())
        return operator+= (-other);

    if (isNegative())
    {
        negate();
        *this += other;
        negate();
        return *this;
    }

    if (compareAbsolute (other) < 0)
    {
        auto temp = other;
        swapWith (temp);
        *this -= temp;
        negate();
        return *this;
    }

    auto numInts = sizeNeededToHold (getHighestBit());
    auto maxOtherInts = sizeNeededToHold (other.getHighestBit());
    jassert (numInts >= maxOtherInts);
    auto* values = getValues();
    auto* otherValues = other.getValues();
    int64 amountToSubtract = 0;

    for (size_t i = 0; i < numInts; ++i)
    {
        if (i < maxOtherInts)
            amountToSubtract += (int64) otherValues[i];

        if (values[i] >= amountToSubtract)
        {
            values[i] = (uint32) (values[i] - amountToSubtract);
            amountToSubtract = 0;
        }
        else
        {
            const int64 n = ((int64) values[i] + (((int64) 1) << 32)) - amountToSubtract;
            values[i] = (uint32) n;
            amountToSubtract = 1;
        }
    }

    highestBit = getHighestBit();
    return *this;
}

BigInteger& BigInteger::operator*= (const BigInteger& other)
{
    if (this == &other)
        return operator*= (BigInteger (other));

    auto n = getHighestBit();
    auto t = other.getHighestBit();

    auto wasNegative = isNegative();
    setNegative (false);

    BigInteger total;
    total.highestBit = n + t + 1;
    auto* totalValues = total.ensureSize (sizeNeededToHold (total.highestBit) + 1);

    n >>= 5;
    t >>= 5;

    auto m = other;
    m.setNegative (false);

    auto* mValues = m.getValues();
    auto* values = getValues();

    for (int i = 0; i <= t; ++i)
    {
        uint32 c = 0;

        for (int j = 0; j <= n; ++j)
        {
            auto uv = (uint64) totalValues[i + j] + (uint64) values[j] * (uint64) mValues[i] + (uint64) c;
            totalValues[i + j] = (uint32) uv;
            c = static_cast<uint32> (uv >> 32);
        }

        totalValues[i + n + 1] = c;
    }

    total.highestBit = total.getHighestBit();
    total.setNegative (wasNegative ^ other.isNegative());
    swapWith (total);

    return *this;
}

void BigInteger::divideBy (const BigInteger& divisor, BigInteger& remainder)
{
    if (this == &divisor)
        return divideBy (BigInteger (divisor), remainder);

    jassert (this != &remainder); // (can't handle passing itself in to get the remainder)

    auto divHB = divisor.getHighestBit();
    auto ourHB = getHighestBit();

    if (divHB < 0 || ourHB < 0)
    {
        // division by zero
        remainder.clear();
        clear();
    }
    else
    {
        auto wasNegative = isNegative();

        swapWith (remainder);
        remainder.setNegative (false);
        clear();

        BigInteger temp (divisor);
        temp.setNegative (false);

        auto leftShift = ourHB - divHB;
        temp <<= leftShift;

        while (leftShift >= 0)
        {
            if (remainder.compareAbsolute (temp) >= 0)
            {
                remainder -= temp;
                setBit (leftShift);
            }

            if (--leftShift >= 0)
                temp >>= 1;
        }

        negative = wasNegative ^ divisor.isNegative();
        remainder.setNegative (wasNegative);
    }
}

BigInteger& BigInteger::operator/= (const BigInteger& other)
{
    BigInteger remainder;
    divideBy (other, remainder);
    return *this;
}

BigInteger& BigInteger::operator|= (const BigInteger& other)
{
    if (this == &other)
        return *this;

    // this operation doesn't take into account negative values..
    jassert (isNegative() == other.isNegative());

    if (other.highestBit >= 0)
    {
        auto* values = ensureSize (sizeNeededToHold (other.highestBit));
        auto* otherValues = other.getValues();

        auto n = (int) bitToIndex (other.highestBit) + 1;

        while (--n >= 0)
            values[n] |= otherValues[n];

        if (other.highestBit > highestBit)
            highestBit = other.highestBit;

        highestBit = getHighestBit();
    }

    return *this;
}

BigInteger& BigInteger::operator&= (const BigInteger& other)
{
    if (this == &other)
        return *this;

    // this operation doesn't take into account negative values..
    jassert (isNegative() == other.isNegative());

    auto* values = getValues();
    auto* otherValues = other.getValues();

    auto n = (int) allocatedSize;

    while (n > (int) other.allocatedSize)
        values[--n] = 0;

    while (--n >= 0)
        values[n] &= otherValues[n];

    if (other.highestBit < highestBit)
        highestBit = other.highestBit;

    highestBit = getHighestBit();
    return *this;
}

BigInteger& BigInteger::operator^= (const BigInteger& other)
{
    if (this == &other)
    {
        clear();
        return *this;
    }

    // this operation will only work with the absolute values
    jassert (isNegative() == other.isNegative());

    if (other.highestBit >= 0)
    {
        auto* values = ensureSize (sizeNeededToHold (other.highestBit));
        auto* otherValues = other.getValues();

        auto n = (int) bitToIndex (other.highestBit) + 1;

        while (--n >= 0)
            values[n] ^= otherValues[n];

        if (other.highestBit > highestBit)
            highestBit = other.highestBit;

        highestBit = getHighestBit();
    }

    return *this;
}

BigInteger& BigInteger::operator%= (const BigInteger& divisor)
{
    BigInteger remainder;
    divideBy (divisor, remainder);
    swapWith (remainder);
    return *this;
}

BigInteger& BigInteger::operator++()      { return operator+= (1); }
BigInteger& BigInteger::operator--()      { return operator-= (1); }
BigInteger  BigInteger::operator++ (int)  { const auto old (*this); operator+= (1); return old; }
BigInteger  BigInteger::operator-- (int)  { const auto old (*this); operator-= (1); return old; }

BigInteger  BigInteger::operator-() const                            { auto b (*this); b.negate(); return b; }
BigInteger  BigInteger::operator+   (const BigInteger& other) const  { auto b (*this); return b += other; }
BigInteger  BigInteger::operator-   (const BigInteger& other) const  { auto b (*this); return b -= other; }
BigInteger  BigInteger::operator*   (const BigInteger& other) const  { auto b (*this); return b *= other; }
BigInteger  BigInteger::operator/   (const BigInteger& other) const  { auto b (*this); return b /= other; }
BigInteger  BigInteger::operator|   (const BigInteger& other) const  { auto b (*this); return b |= other; }
BigInteger  BigInteger::operator&   (const BigInteger& other) const  { auto b (*this); return b &= other; }
BigInteger  BigInteger::operator^   (const BigInteger& other) const  { auto b (*this); return b ^= other; }
BigInteger  BigInteger::operator%   (const BigInteger& other) const  { auto b (*this); return b %= other; }
BigInteger  BigInteger::operator<<  (const int numBits) const        { auto b (*this); return b <<= numBits; }
BigInteger  BigInteger::operator>>  (const int numBits) const        { auto b (*this); return b >>= numBits; }
BigInteger& BigInteger::operator<<= (const int numBits)              { shiftBits (numBits, 0);  return *this; }
BigInteger& BigInteger::operator>>= (const int numBits)              { shiftBits (-numBits, 0); return *this; }

//==============================================================================
int BigInteger::compare (const BigInteger& other) const noexcept
{
    auto isNeg = isNegative();

    if (isNeg == other.isNegative())
    {
        auto absComp = compareAbsolute (other);
        return isNeg ? -absComp : absComp;
    }

    return isNeg ? -1 : 1;
}

int BigInteger::compareAbsolute (const BigInteger& other) const noexcept
{
    auto h1 = getHighestBit();
    auto h2 = other.getHighestBit();

    if (h1 > h2) return 1;
    if (h1 < h2) return -1;

    auto* values = getValues();
    auto* otherValues = other.getValues();

    for (int i = (int) bitToIndex (h1); i >= 0; --i)
        if (values[i] != otherValues[i])
            return values[i] > otherValues[i] ? 1 : -1;

    return 0;
}

bool BigInteger::operator== (const BigInteger& other) const noexcept    { return compare (other) == 0; }
bool BigInteger::operator!= (const BigInteger& other) const noexcept    { return compare (other) != 0; }
bool BigInteger::operator<  (const BigInteger& other) const noexcept    { return compare (other) <  0; }
bool BigInteger::operator<= (const BigInteger& other) const noexcept    { return compare (other) <= 0; }
bool BigInteger::operator>  (const BigInteger& other) const noexcept    { return compare (other) >  0; }
bool BigInteger::operator>= (const BigInteger& other) const noexcept    { return compare (other) >= 0; }

//==============================================================================
void BigInteger::shiftLeft (int bits, const int startBit)
{
    if (startBit > 0)
    {
        for (int i = highestBit; i >= startBit; --i)
            setBit (i + bits, (*this) [i]);

        while (--bits >= 0)
            clearBit (bits + startBit);
    }
    else
    {
        auto* values = ensureSize (sizeNeededToHold (highestBit + bits));
        auto wordsToMove = bitToIndex (bits);
        auto numOriginalInts = bitToIndex (highestBit);
        highestBit += bits;

        if (wordsToMove > 0)
        {
            for (int i = (int) numOriginalInts; i >= 0; --i)
                values[(size_t) i + wordsToMove] = values[i];

            for (size_t j = 0; j < wordsToMove; ++j)
                values[j] = 0;

            bits &= 31;
        }

        if (bits != 0)
        {
            auto invBits = 32 - bits;

            for (size_t i = bitToIndex (highestBit); i > wordsToMove; --i)
                values[i] = (values[i] << bits) | (values[i - 1] >> invBits);

            values[wordsToMove] = values[wordsToMove] << bits;
        }

        highestBit = getHighestBit();
    }
}

void BigInteger::shiftRight (int bits, const int startBit)
{
    if (startBit > 0)
    {
        for (int i = startBit; i <= highestBit; ++i)
            setBit (i, (*this) [i + bits]);

        highestBit = getHighestBit();
    }
    else
    {
        if (bits > highestBit)
        {
            clear();
        }
        else
        {
            auto wordsToMove = bitToIndex (bits);
            auto top = 1 + bitToIndex (highestBit) - wordsToMove;
            highestBit -= bits;
            auto* values = getValues();

            if (wordsToMove > 0)
            {
                for (size_t i = 0; i < top; ++i)
                    values[i] = values[i + wordsToMove];

                for (size_t i = 0; i < wordsToMove; ++i)
                    values[top + i] = 0;

                bits &= 31;
            }

            if (bits != 0)
            {
                auto invBits = 32 - bits;
                --top;

                for (size_t i = 0; i < top; ++i)
                    values[i] = (values[i] >> bits) | (values[i + 1] << invBits);

                values[top] = (values[top] >> bits);
            }

            highestBit = getHighestBit();
        }
    }
}

BigInteger& BigInteger::shiftBits (int bits, const int startBit)
{
    if (highestBit >= 0)
    {
        if (bits < 0)
            shiftRight (-bits, startBit);
        else if (bits > 0)
            shiftLeft (bits, startBit);
    }

    return *this;
}

//==============================================================================
static BigInteger simpleGCD (BigInteger* m, BigInteger* n)
{
    while (! m->isZero())
    {
        if (n->compareAbsolute (*m) > 0)
            std::swap (m, n);

        *m -= *n;
    }

    return *n;
}

BigInteger BigInteger::findGreatestCommonDivisor (BigInteger n) const
{
    auto m = *this;

    while (! n.isZero())
    {
        if (std::abs (m.getHighestBit() - n.getHighestBit()) <= 16)
            return simpleGCD (&m, &n);

        BigInteger temp2;
        m.divideBy (n, temp2);

        m.swapWith (n);
        n.swapWith (temp2);
    }

    return m;
}

void BigInteger::exponentModulo (const BigInteger& exponent, const BigInteger& modulus)
{
    if (modulus == 1)
    {
        *this = 0;
        return;
    }

    *this %= modulus;
    auto exp = exponent;

    if (modulus.getHighestBit() <= 32 || modulus % 2 == 0)
    {
        auto a = *this;
        auto n = exp.getHighestBit();

        for (int i = n; --i >= 0;)
        {
            *this *= *this;

            if (exp[i])
                *this *= a;

            if (compareAbsolute (modulus) >= 0)
                *this %= modulus;
        }
    }
    else
    {
        auto Rfactor = modulus.getHighestBit() + 1;
        BigInteger R (1);
        R.shiftLeft (Rfactor, 0);

        BigInteger R1, m1, g;
        g.extendedEuclidean (modulus, R, m1, R1);

        if (! g.isOne())
        {
            BigInteger a (*this);

            for (int i = exp.getHighestBit(); --i >= 0;)
            {
                *this *= *this;

                if (exp[i])
                    *this *= a;

                if (compareAbsolute (modulus) >= 0)
                    *this %= modulus;
            }
        }
        else
        {
            auto am  = (*this * R) % modulus;
            auto xm = am;
            auto um = R % modulus;

            for (int i = exp.getHighestBit(); --i >= 0;)
            {
                xm.montgomeryMultiplication (xm, modulus, m1, Rfactor);

                if (exp[i])
                    xm.montgomeryMultiplication (am, modulus, m1, Rfactor);
            }

            xm.montgomeryMultiplication (1, modulus, m1, Rfactor);
            swapWith (xm);
        }
    }
}

void BigInteger::montgomeryMultiplication (const BigInteger& other, const BigInteger& modulus,
                                           const BigInteger& modulusp, const int k)
{
    *this *= other;
    auto t = *this;

    setRange (k, highestBit - k + 1, false);
    *this *= modulusp;

    setRange (k, highestBit - k + 1, false);
    *this *= modulus;
    *this += t;
    shiftRight (k, 0);

    if (compare (modulus) >= 0)
        *this -= modulus;
    else if (isNegative())
        *this += modulus;
}

void BigInteger::extendedEuclidean (const BigInteger& a, const BigInteger& b,
                                    BigInteger& x, BigInteger& y)
{
    BigInteger p (a), q (b), gcd (1);
    Array<BigInteger> tempValues;

    while (! q.isZero())
    {
        tempValues.add (p / q);
        gcd = q;
        q = p % q;
        p = gcd;
    }

    x.clear();
    y = 1;

    for (int i = 1; i < tempValues.size(); ++i)
    {
        auto& v = tempValues.getReference (tempValues.size() - i - 1);

        if ((i & 1) != 0)
            x += y * v;
        else
            y += x * v;
    }

    if (gcd.compareAbsolute (y * b - x * a) != 0)
    {
        x.negate();
        x.swapWith (y);
        x.negate();
    }

    swapWith (gcd);
}

void BigInteger::inverseModulo (const BigInteger& modulus)
{
    if (modulus.isOne() || modulus.isNegative())
    {
        clear();
        return;
    }

    if (isNegative() || compareAbsolute (modulus) >= 0)
        *this %= modulus;

    if (isOne())
        return;

    if (findGreatestCommonDivisor (modulus) != 1)
    {
        clear();  // not invertible!
        return;
    }

    BigInteger a1 (modulus), a2 (*this),
               b1 (modulus), b2 (1);

    while (! a2.isOne())
    {
        BigInteger temp1, multiplier (a1);
        multiplier.divideBy (a2, temp1);

        temp1 = a2;
        temp1 *= multiplier;
        auto temp2 = a1;
        temp2 -= temp1;
        a1 = a2;
        a2 = temp2;

        temp1 = b2;
        temp1 *= multiplier;
        temp2 = b1;
        temp2 -= temp1;
        b1 = b2;
        b2 = temp2;
    }

    while (b2.isNegative())
        b2 += modulus;

    b2 %= modulus;
    swapWith (b2);
}

//==============================================================================
OutputStream& JUCE_CALLTYPE operator<< (OutputStream& stream, const BigInteger& value)
{
    return stream << value.toString (10);
}

String BigInteger::toString (const int base, const int minimumNumCharacters) const
{
    String s;
    auto v = *this;

    if (base == 2 || base == 8 || base == 16)
    {
        auto bits = (base == 2) ? 1 : (base == 8 ? 3 : 4);
        static const char hexDigits[] = "0123456789abcdef";

        for (;;)
        {
            auto remainder = v.getBitRangeAsInt (0, bits);
            v >>= bits;

            if (remainder == 0 && v.isZero())
                break;

            s = String::charToString ((juce_wchar) (uint8) hexDigits [remainder]) + s;
        }
    }
    else if (base == 10)
    {
        const BigInteger ten (10);
        BigInteger remainder;

        for (;;)
        {
            v.divideBy (ten, remainder);

            if (remainder.isZero() && v.isZero())
                break;

            s = String (remainder.getBitRangeAsInt (0, 8)) + s;
        }
    }
    else
    {
        jassertfalse; // can't do the specified base!
        return {};
    }

    s = s.paddedLeft ('0', minimumNumCharacters);

    return isNegative() ? "-" + s : s;
}

void BigInteger::parseString (StringRef text, const int base)
{
    clear();
    auto t = text.text.findEndOfWhitespace();

    setNegative (*t == (juce_wchar) '-');

    if (base == 2 || base == 8 || base == 16)
    {
        auto bits = (base == 2) ? 1 : (base == 8 ? 3 : 4);

        for (;;)
        {
            auto c = t.getAndAdvance();
            auto digit = CharacterFunctions::getHexDigitValue (c);

            if (((uint32) digit) < (uint32) base)
            {
                *this <<= bits;
                *this += digit;
            }
            else if (c == 0)
            {
                break;
            }
        }
    }
    else if (base == 10)
    {
        const BigInteger ten ((uint32) 10);

        for (;;)
        {
            auto c = t.getAndAdvance();

            if (c >= '0' && c <= '9')
            {
                *this *= ten;
                *this += (int) (c - '0');
            }
            else if (c == 0)
            {
                break;
            }
        }
    }
}

MemoryBlock BigInteger::toMemoryBlock() const
{
    auto numBytes = (getHighestBit() + 8) >> 3;
    MemoryBlock mb ((size_t) numBytes);
    auto* values = getValues();

    for (int i = 0; i < numBytes; ++i)
        mb[i] = (char) ((values[i / 4] >> ((i & 3) * 8)) & 0xff);

    return mb;
}

void BigInteger::loadFromMemoryBlock (const MemoryBlock& data)
{
    auto numBytes = data.getSize();
    auto numInts = 1 + (numBytes / sizeof (uint32));
    auto* values = ensureSize (numInts);

    for (int i = 0; i < (int) numInts - 1; ++i)
        values[i] = (uint32) ByteOrder::littleEndianInt (addBytesToPointer (data.getData(), (size_t) i * sizeof (uint32)));

    values[numInts - 1] = 0;

    for (int i = (int) (numBytes & ~3u); i < (int) numBytes; ++i)
        this->setBitRangeAsInt (i << 3, 8, (uint32) data [i]);

    highestBit = (int) numBytes * 8;
    highestBit = getHighestBit();
}

//==============================================================================
void writeLittleEndianBitsInBuffer (void* buffer, uint32 startBit, uint32 numBits, uint32 value) noexcept
{
    jassert (buffer != nullptr);
    jassert (numBits > 0 && numBits <= 32);
    jassert (numBits == 32 || (value >> numBits) == 0);

    uint8* data = static_cast<uint8*> (buffer) + startBit / 8;

    if (const uint32 offset = (startBit & 7))
    {
        const uint32 bitsInByte = 8 - offset;
        const uint8 current = *data;

        if (bitsInByte >= numBits)
        {
            *data = (uint8) ((current & ~(((1u << numBits) - 1u) << offset)) | (value << offset));
            return;
        }

        *data++ = current ^ (uint8) (((value << offset) ^ current) & (((1u << bitsInByte) - 1u) << offset));
        numBits -= bitsInByte;
        value >>= bitsInByte;
    }

    while (numBits >= 8)
    {
        *data++ = (uint8) value;
        value >>= 8;
        numBits -= 8;
    }

    if (numBits > 0)
        *data = (uint8) ((*data & (uint32) (0xff << numBits)) | value);
}

uint32 readLittleEndianBitsInBuffer (const void* buffer, uint32 startBit, uint32 numBits) noexcept
{
    jassert (buffer != nullptr);
    jassert (numBits > 0 && numBits <= 32);

    uint32 result = 0;
    uint32 bitsRead = 0;
    const uint8* data = static_cast<const uint8*> (buffer) + startBit / 8;

    if (const uint32 offset = (startBit & 7))
    {
        const uint32 bitsInByte = 8 - offset;
        result = (uint32) (*data >> offset);

        if (bitsInByte >= numBits)
            return result & ((1u << numBits) - 1u);

        numBits -= bitsInByte;
        bitsRead += bitsInByte;
        ++data;
    }

    while (numBits >= 8)
    {
        result |= (((uint32) *data++) << bitsRead);
        bitsRead += 8;
        numBits -= 8;
    }

    if (numBits > 0)
        result |= ((*data & ((1u << numBits) - 1u)) << bitsRead);

    return result;
}


//==============================================================================
//==============================================================================
#if JUCE_UNIT_TESTS

class BigIntegerTests final : public UnitTest
{
public:
    BigIntegerTests()
        : UnitTest ("BigInteger", UnitTestCategories::maths)
    {}

    static BigInteger getBigRandom (Random& r)
    {
        BigInteger b;

        while (b < 2)
            r.fillBitsRandomly (b, 0, r.nextInt (150) + 1);

        return b;
    }

    static constexpr int64_t intPow (int64_t base, int64_t exponent)
    {
        return exponent == 0 ? 1 : base * intPow (base, exponent - 1);
    }

    void runTest() override
    {
        {
            beginTest ("BigInteger");

            Random r = getRandom();

            expect (BigInteger().isZero());
            expect (BigInteger (1).isOne());

            for (int j = 10000; --j >= 0;)
            {
                BigInteger b1 (getBigRandom (r)),
                           b2 (getBigRandom (r));

                BigInteger b3 = b1 + b2;
                expect (b3 > b1 && b3 > b2);
                expect (b3 - b1 == b2);
                expect (b3 - b2 == b1);

                BigInteger b4 = b1 * b2;
                expect (b4 > b1 && b4 > b2);
                expect (b4 / b1 == b2);
                expect (b4 / b2 == b1);
                expect (((b4 << 1) >> 1) == b4);
                expect (((b4 << 10) >> 10) == b4);
                expect (((b4 << 100) >> 100) == b4);

                // TODO: should add tests for other ops (although they also get pretty well tested in the RSA unit test)

                BigInteger b5;
                b5.loadFromMemoryBlock (b3.toMemoryBlock());
                expect (b3 == b5);
            }
        }

        {
            beginTest ("Bit setting");

            Random r = getRandom();
            static uint8 test[2048];

            for (int j = 100000; --j >= 0;)
            {
                uint32 offset = static_cast<uint32> (r.nextInt (200) + 10);
                uint32 num = static_cast<uint32> (r.nextInt (32) + 1);
                uint32 value = static_cast<uint32> (r.nextInt());

                if (num < 32)
                    value &= ((1u << num) - 1);

                auto old1 = readLittleEndianBitsInBuffer (test, offset - 6, 6);
                auto old2 = readLittleEndianBitsInBuffer (test, offset + num, 6);
                writeLittleEndianBitsInBuffer (test, offset, num, value);
                auto result = readLittleEndianBitsInBuffer (test, offset, num);

                expect (result == value);
                expect (old1 == readLittleEndianBitsInBuffer (test, offset - 6, 6));
                expect (old2 == readLittleEndianBitsInBuffer (test, offset + num, 6));
            }
        }

        {
            beginTest ("exponent modulo");

            {
                constexpr auto base = 3;
                constexpr auto exponent = 8;
                constexpr auto modulus = 5;
                constexpr auto expected = (intPow (base, exponent) % modulus);

                BigInteger result { base };
                result.exponentModulo (exponent, modulus);
                expect (result == (int64) expected);
            }

            {
                BigInteger large { (int64) 85899345927 };
                large.exponentModulo ((int64) 85899345926, (int64) 85899345925);
                expect (large == 0x4000000);
            }

            {
                struct TestCase
                {
                    const char* base;
                    const char* exponent;
                    const char* modulus;
                    const char* result;
                };

                // The following cases generated with python3:
                // import random
                // for i in range(100):
                //     b = random.getrandbits(256)
                //     e = random.getrandbits(256)
                //     m = random.getrandbits(256)
                //     print(f'{{ "{b:x}", "{e:x}", "{m:x}", "{pow(b, e, m):x}" }},')

                constexpr TestCase cases[]
                {
                    { "3578b135981b1cdf5df245363f5426f40b827f4551245c93cb89183cb8959703", "ac9617afbcc424f8aee92b39c4af0dfbfc6f335c78a00ddf37ba51da1eb49d38", "c5619dd6e05e07fdb9461de10f67bef3a830fb4255adf3d5246685e88541a575", "a5c1a0768b0ffe01fe37545279a23584a9bcd5f74598f9e133afdc53c934811d" },
                    { "3e5a849cc72aafac0360ae15a7b8c2c4ebf366d7cff60a9b14b47f54246ff535", "5d52d4fe7c9a2e7276d9275d4fd462d95314fcb61868c28f693de618e6aeea93", "ba8d7780816526be0b9a166b5abd08efa24e36090dd1fd9e184779ec0e58a0d8", "99524e9461c401e8760dcc0c2f9a83e0c02c3ca6f42f28650299bee99aac0445" },
                    { "7d731fb068d15173cd3b9343bc19772596decc6a241a0ae392fc6c01e6f6b4b8", "836d6d7b72a66bcba56c0bdb7d3330551f85a9347730eb72509f6623eaa8cfff", "96dd6304db8f2a482633a7a7d23bae5943b75b279c8450a328b977afa197c1b1", "831632b26efb943a025ee9cc5acdee0e36769508027822f72d530a66b9c5baa1" },
                    { "3d23320717a819331b44630e607a5604e6029a7a1137c4d41fec35878946ca45", "b1228d353b608ed946d1092bf31f30117f398378e6996fbd13a1c57e6c7276b9", "f6b7fe6ca8bb74c5d8087e75e1f50fa9ef56c63546dbdc46e136d45c75263039", "a2ee5889810d92fa205404d52b461fefd140ab43813b381841fe582a2e3808c0" },
                    { "80549342019ccfe0c8c347effc9b118e5f7d33986e63fd84a927e3aab1799962", "e8b9573a143c45c742e8598e9ddaa3dfe00ce856fefad84e1f5657e120b6bd", "b0032cd5d990b7e2d61871a56eca1400cdc467535e0817920930a345abfcbae0", "43a1b1bd9ba3b2656d55a32b409cbec55a2ae69a62e9978b83de36affaf2d940" },
                    { "74b534a01615f36be4ef90d7b338e76272732d8306e75de9ab327564c1adf144", "fb8d058ff343d2a13ae988b9be7d2f2cdd8e7f2e2280a119d0925ab55b639340", "a49d444db8e27cec7a56cdf95b2de3fc8f7997aff5e84efbaf9abaa7f42bd4f2", "8be7809b214b0af605e685485b993535ab384cb27d0694258a810fcd9d744160" },
                    { "91ab685e898c7d3727f9e229b0c62bb91e43b0e504d179af587012bc9983e4f4", "dc5e8b4d5e8ca22d161466c62c30e2a1cab1e2e18e68f22dae172a739c12ee48", "9cb82b323037db8c8832121eac01a83380a347f8a7e119bc3d6910e01aa4a0a5", "9159e8e3fec6fdcfcec9bbfcf9ef7c7b9fe8365495164dbcf9ff766a92223750" },
                    { "379d06b3aa878a18c14b7d1e1a9005640f57800ad27218b5263c2195056daec8", "84f7cd56ee658a7e57fefa6e2ccd3012f75d455dc135f95ecb57387842bf3223", "78b1f0cabc86034d069b5ceb2592cb108b3c417b24ada5adb957c7280445cb40", "1ea26f57aea790cdca3eefac229ea8cb1170efac8ee0a619268af0c6a6dcfd00" },
                    { "4e75de8fa551ac23ee4c51008df8a6358e36d69455a51018769ee739d477af99", "ec7177ec9246a5b0322582650ffa5275524f48e05e7a258ddbe3db1c3fe807d9", "60562bffd764b05e356e6f0a6bec65047dd9b63ca3e4542621788896ad553990", "4c4cd0478560216427337cae5be070b72510ebee96456702164fdf421f098969" },
                    { "a5b154a26903abd2936fa68cf3330b6165661dba8b7048d06c50cc9c1ede8f81", "18843eea7886c27f56df17ead5d598846fae650b0a391b95871b87cf59a6aee1", "2397b538dc83c5369034fac295e087389d8f7e343a04a65ea6e740cde71f0e50", "ca01bacca57cba8d579d974f6664e4b14b507e932a431c44517fc054b697281" },
                    { "fa26195f0f7ab449f69fa3fa03d259a135cfe27f8502b7cbc64df2e37b02f240", "bd6dc25a3d8cf31e06f43601d92d4b544f7698c0833a1143b6484f50ea3b2e6f", "e93a33cf550afab5e31c5c8c8addad99c7037091d6e1a153beab768dfbb4e271", "8a5a71c1f496844458cac6d0491e073f50be0e4df5781d1a5ddb58b2fc717594" },
                    { "18bca82f57974583bb0f6d1a98a112cdc6769d89b9bf6a1c1c40a043f86e432f", "53554117903b6f3dd388e13cb402c0f0ae960812a77cb03e65ee8c279321a35e", "6eea7e44ce5650b5d0f120adfaea175e3774c5c6e8c7b5f18e6a4b3a8b3b4d6b", "1efa31b046724024eef26dced334c95d6c2ab46df1da13510f32673c487d9252" },
                    { "8891b06093a0d40f049f8a517c6b006d2c8e13533372444b948b7a733480d678", "849044280d7203d48814ef4fa3b696b4365edb3c9719f8e24884cf562ebdb96b", "520e6050f6158df71dad0ca4687de19233d99bb5049eca5a182df83c29bec23b", "c2cc1a1e99f15142674a4b669ca3291a41bb7078c51b7d3424c2cac5931bcc4" },
                    { "9ef621d6ac704c305c212ac154a223cdd88b0314b8d4909994bae8132b74891", "2fe9bfbd5a39261ad3a53dd652f20bf25bbfa5c27583f6f3146225bf69df5130", "cfd49f637be563efa3aaf235352a995f0d2ddfaf4a81340919730f297f46e88d", "467337179bdb41bdcc80ae2a96bad5a357ca4a45d1ccd117833645ca5f46f15c" },
                    { "61c8ee2b3b01d010274cafb38e4838f149c1820a088a70e92c392438d0a448bb", "120040d9b30347bf1fba305386b0d96df45999c1e24c4ab104e7fc6d80bdb3c9", "931b3f332b6641734aad399a4cdcfda64322aaa37064af96593612c81b318bed", "61f16daa4498ba16cc81c80312bd8bd473175f17fda76c6d7df6c7742394ba88" },
                    { "578d2945ecd7b708ff5405c1d9443586e09d4a38f8fe22f2057ee6816e3cac0a", "668fa8648eb6804c96d6a21130113aba8d824cec18e9899abe357571c4868c41", "9236cc38adad644a8dda71ed90a637abb8dcbff0cdb96ee2608932983704cc20", "3c0770900018c66bea07b8c040e8216226911922fe69697bd0c7d6fef74aeae0" },
                    { "6349a03e04f3193f4c50248e384098ea1f32ed87e52ebb19c5478b065981e040", "6feea33c861e6a5d685f9fd37ae99c025bc0f2626871ff2a0c4332884038b663", "505dba96a03c583e5070f1313e495a0e808b4319ba070235455786551d2aef3a", "3f9afbb8b63e311eb3c029f5e15a4487ec916f58387a864d0488dbd5875a5d44" },
                    { "fcdcc28ee83ee8ed7eae4705bc8c149d1dca351041a2b98a5d8dca53f06d2e23", "9b663cbd7779f518abf89c0120620424d40414455014ab6272f3fbbd98d482df", "183e6ba5498bd9b7c35a1b5e6de0389e0a47ccf9b8d5993691d84f1558c37c61", "27732e7f36c3eb0b487e29572e22741f3363a82836097d56220b74004fb28b1" },
                    { "5a4d45d0cab77710fc5c3d6967285c28def8b10a49385186e5325df32b3b7064", "a41f47a361a0cd287cca7e290114bcdc5d0da54f201d530f125899a1f76426b2", "f711c0102d0ec25a551aa89575bf2fb27ba768f0b0f4db24d2a6548c45ababc7", "65bc2b0dd0f97d358a6b36675a5d06061e680bbd316d94e493f25b306ddb6d83" },
                    { "6a4e5f440d31ebf9acc060840181cb2847bf73139557f64314fc3cb1201a9aad", "14ed91b7e15a98f63b9811468888434cece501811880b99a29422c7eabcddd54", "cfc45e00d41dec58e7b397f8c0fdab7a60f8c8072c3d09ddcc42016fe3b75f70", "b5a07d828eb83de3ee415bd9ef65086b3544d71bc05407fe98579c4bb32c8b61" },
                    { "fdce2b68868577548ab019fb437b637d860fb5276dbe6aad88615c180d400289", "457b61210aa91dcb75da3e34da09c330f8a31a51fbff9acb4e13bd264d16439e", "c98d2422cd6aa5de14b88af202a9f321e80c1f08c4ab17f84966b723af782980", "b5f21a45a6ca40a36426f6fa7c37a6f9e0a56af2ef8a465c4f3332efa16dadb1" },
                    { "1ba35659ef4e3764fcee71336878ad5d70faead7e5d7f037f3cf958235f846bb", "45138f50346b8e6096d9fef8d02ca78bd4f363947228de27f91d05d9ceada55a", "92f998c6da18cce60da45e1d545a2011e6b9be4ebe8dfd6219c0723721c07245", "6a134c7acffe3e107286ac14c6ba08d9c9f688eec2e1d447bdb2d80d0618dda0" },
                    { "79bd8c96976ad2fddae2feeafdd84ffecb292a499f5202d531d53de2d2d526cb", "8babb5491de3c6d8f95d31d8bcb582e5ebb6e7a77ac7ba2ae7b0bc9873a5ce7f", "ea1851940f2bb94c0e86f7bb85c2c07626b0b3f5473ffc8ef0cc9bb86b9ac761", "399d6126d127263dbb5b62ca17b8d5422c74dd70659a5d8e41a22cc1432a1346" },
                    { "5fefb3732928b3c29192411ae1f716c387bb2b2c76658d45bbc8937971946caa", "974252acb6a517cfb6815e2c79428590985a2238c823ba2d9c826ddb12f3ddb6", "837e9d78d0f179e5ea18b9c031410c771527132f35d02030adf3881c5d8bb796", "fc0e9e2e1bd178bd95e2f86d8b6e07b81f44372687186f43eb0504ae3dc7614" },
                    { "4f87dac49c30ca4fd674e60c470be29f678943d2e719ff6425f5cfcd5c5b512c", "1ce75d258b49785d3362ad7863a7e6c2af13a27d789690030c3ed63755a204f7", "17fc7ea6e999d091283da1805f98402c12bf7fd21f000e2d99c6f33dcbd5d898", "72ecb66d8a47e2e14a3bea0a49c3213569f666876035d0dda0c98429bc37940" },
                    { "699a02d436d39d6c06388eae354a7b2991a4c4b94d268ca980ce5600e580dfce", "8b2bce31d9a97be587598464cd0b5bc22d4d1773ee587bc10214835cc0c95590", "345fcff5c12e26ef07b732eea36ffb5cfa837d2d7eee01e00d05a4e9f61aa2ec", "7fe77a7f81821c80b99e20aa185a4743c507e07d992af889b5b69e51991a134" },
                    { "9b8bee3df6d2ed33de3b475f5ee96c56020d416af903245ce5ea624d5de43252", "e81deaeb107b08b976e2e1fe32068479848c99734a2edea066013d8aa4a53a57", "f9a6c5c2047002d6bd5e220c85e4b3a066852b5e53b5d073363ad4fa64131b92", "9a1f02cf470f03d26ff3c1a0157e61be685e06354a27c5daa3deefc6c7b06a72" },
                    { "6e0b1df86380d38d2a343e745063395d0dfa817394ca6ccaa04185e5f6ac8a28", "e9f2aaf043999f0755f81b1c1834e7079a57f8e0e5da1c25eb3d4359e5abd909", "f8437002f2014addd15920a4023c96be28a75b2b760bb045e28a6fa307c0e130", "afd0f54734612b6245e754d14903cb5479af4034b37d7ef5af8fe222c54b24c0" },
                    { "c51eb6a4ea7dcc6d348c2e1451d556d48ed2c475eef58b14b049f33d84c8b9e", "33c54653c8b44b485dd4a752097a0a90741d7c609043cea76344525e24cc0ce2", "c29c20c34a6052f0583afa68b4a1f4d3a388d2a72277982f1284621cef315bb1", "ba9261bbf849d896417dbffb5c967c62fee195ebb164d2ca966c2f213d2eacf8" },
                    { "497ff688d8ba0970c750fc638d0aa35b975461c5aca747916b555e79dafcf868", "a3f1c4ae01bc6c7ce436288faf66dc562c38b93c650f97690a82817b76b9e3b8", "410fa726aa68bf435fc7d139b958990f5cd1a9058470e6bee0f12178a48d403c", "17947afc362f5b122a3ed0b8b45d9c148310e38ed92c13cb1c603465fcd147cc" },
                    { "95a15990195b0f361ca9d2b2626d878789879f6bbd94fcf988b21e0a710c5679", "2ff3d1707822185ed5720037a2fcad4af41a1e081f734815386143a62484855c", "fa0cf11dd25ff715502622ae3cceeb35a0a900cb4686b6c8044609ae6fd9b073", "b8d9d5f2e87c3f4413037632e1391f838e47a19102e7dc704de40ed38a05180d" },
                    { "9e77b4569f57a0fa8b51e7abb0bb20d6585760f681ff4324ff8e06b127e29562", "6143e0242f7302a099c21d407ed1cde2f41876a8a7bd80f1cdc904c790c49f04", "c66bfd2f235dabb3a1bb677c8bd52a41dc997f66a3d6e0e2b3151225b083339", "10ba597146fabe8cad2e1ecd2979189b8f60bcbbbbdd8a9655cd14336585c99" },
                    { "def0292036dc4d18779637cd66732395741ab93b63f1cd643fd69d25355310a", "648c659bfe727ff35fed6929bf1d0664770085d06efe7c82c2dd7a635ee7cb91", "97b22d252c9ed94eb7a00e4d545f43659a18879cc38586c8d24ed17091cf7719", "8e91128599d82e604176bdaf25120283a61fcf7ef0cf6b5e2cfe823dc644c9ab" },
                    { "34306824dc7124993c1ffe5f27c55d37566411c480b31636ff91ee9f964e93db", "b590da189c8f66af0bf59025b29cd99f8eb8450ddf2dc3e08605ec1108100d54", "b598ea469400624a5fc943ab73f816205bde7b529ba3665e90353e8d55ef9a5d", "1af083b978b4af7e861b65dc47990de7c6c3443e772718d27149b6ba77b0e1ce" },
                    { "6ba9b4b853fbcd8a71ddaae1369ee9627659c59e5dc786e8f6104cbac3e2c997", "125e3f87842d8eea37bdbadf42c798f42d1a50a40c8a3fcb9870f122ccd0c200", "5483343bfd964230da3ca092d92fb0fa4448168f611484d2f65e86e05d94e821", "2e223c52b9d43b01b36ecbdd5a7de7710e1bddc57d822938bf18aa6dd22e22f0" },
                    { "1426aacace38bf64482c6550dd476412b22a617648ef313890025d890679afe8", "168ba045e3b2a0e9a2209c725218fbdfd3009b53c522f4da6664e770351d1849", "860dff3b5e57f1227c922f84dbdfb91300ea872daf8b3a56020044e129d7931e", "25d6d88ca28328d2fc1084315cffa7c4ed7a2565936615f533fe6809581e9350" },
                    { "6a511a7d96fca462c45a3eca06c222aa72ea928f1d4961384532566e406f84ac", "acbe4d2403f9c17a9f232d4e0d06d86aa1754491933d2ae3bc44755d701a10bc", "c89ca0b7e7f776419617ffa4366ca3a115a66b1dc99935bdfb27f25886aad000", "604c220b2435ae293b47c1b25c85c04e0f62779be1d6463a992c2640eb820000" },
                    { "476ef4e8544cc658c4192d72d7b997431fb9641cd46ef69dba1dead6a9a40362", "d3ac5b1a1a27e294e53cbf1c331a4b12e7c38e8552e506b00bb56000e79ce7f2", "d193d04f308d223b626f5ef531871daf0295bdcf6e12667eecc196f6374032fa", "20c2d68dfa900771e6c8dd66a860805b59a9dfc77c5f4b20d67f74e2c6440fec" },
                    { "879e6ce29e9281d2bf6fbfc80bb0be1964c62dc17938a3746f24952681f71c8", "7bfd3a85ac051c976fa657152cb2acec4a99b71db1f48ac50c4031a3021fff9a", "a2a8703b82cbbc2999696cc945a893dffb9c7c7768e3458ea5520ef4d7292aa6", "2e69a647d708ac1aade1a6634103242c1dfed3b79be6782fe657d4d79fed9d9a" },
                    { "1debb109a760eb136bdd4eeccba55743a17ede3e9bd62887cf108aa2655dd87c", "c7ec55004a0244693422b697803ff57622b61cb078f6ccc785e0d0e1e1803272", "42c08f8ce738bc428ade96751df4e725023336f2d93c9dc563338ab89310ddef", "750841077ba6eb4e064d167bd918e7d200d21f586b5597ffe21a0d79ebbfe3e" },
                    { "3d568e695769725716087983fc5fa9d1293801f3aae0f1c0b997019612b63e94", "da1194acb4c498f76cb1b00889f64db1193723be7af2c3c0d6be12f5b6e2b0b5", "f0c6f4e5baad60f67f732ede4289decf6eb4d390bf14d9fbb7b3678d3263cae5", "476e603cf3757c33e933f7263cae871857fd9375c5a06b432143a99757d39476" },
                    { "74cbae40ec7c1c487776574b0c9440c12258ac0f31dfbd859ea731f32bb037b3", "22f537add5cf021382e418502b0f593caab76ce4a604c38ce5c4a5de6a263206", "fe5d94a081b36458c2a8be7ed5b89461fdb67d64657dc1645921aa8ad4bd02d", "94206dfa8b10575ffb822b534f00580514ddb4c01dc2d727ed2866ca9b97c17" },
                    { "cbe5dbb4173894eba8b1e58d1d1a9de842bb34397ff726a045010486972e66bb", "471e8aba15861a8ea5a5de124276b902826511da41677058a4c9aa8583dc966", "2431ac612894bb866ca82e10851cf701e411f99a352e60779dcd8fac93838b71", "100e6e06c9443ce5a874ba3bcbf3aec60694eca5fad43805e7cf05e60f2fad49" },
                    { "b9ca4e5998ae7912dae3f2b66c890475b6d6d2ccd722164c0b846f1c6b9affef", "67e6a91da769e5e0cfafb2d89836c4c54754e3a43e938926dc795627ce18f41f", "5db80afe4cef168cbb4777db29f87dcd1c599e51e1386928f4e95f0fe2ec1615", "2cd8726f5595ae943a8e05023bef60aef4b648aa3983bdef9cf159f8a4ef003d" },
                    { "a5500fd9802efef764f26a35e8a248a9fcf74f41bf3f7a857506360d88d9aa99", "1f8e55ff2c8ed88d820f392047ab3c2e6562386bb22cd59626154c1dd8f05633", "d5d59bb9098726186203dddec5b810e759678f790feee63d3a669dbf4061ab58", "124014359f52f000379d68ce775cb9c36a30fe092262cd0178d1fd829be986f1" },
                    { "cbffda41b6a708dcda9b0b411fd0de06efb2c26a0966fe893273eddbfbba221a", "5da4c2fd7c857a52a57437676fcc3b01b57ad0de41d6aad8e841effa4d65ec27", "da1d6c49f6a13bdc465c86d43aa553a656c070a15f0953772701e8bdf5a6164a", "bd6fba91b99cd5f70ec95ba66a2429deede3f46c8cee350cbe574f1f214ae64a" },
                    { "dc1ba5913d9d4fde41e6a198db2892302778dc2c06bf51bf2e5157381009b184", "27d76abe78c9f83ffa200afec8a7621052737e870fc6ea543b324b236acc3f43", "96dd16c1b08af57ef0c9447939fe5e7c4b9875f623af4ad4d87297f72ad1cdb4", "12f22e4f56e2159da49021ab9d27552faf04216529d5ca99ebbf3a1a3bcb7d80" },
                    { "bd5c4034df80a0c1b5a70b79124ae8bb70c398802ded93bc03e603310faffdc", "a1c3c6accbf56af108ddc59025ededb28f8eb4a5ae4a4ced4880542ee59e8746", "a50acf7417cb6275f5552b94ca12a6c4d4e3b166f9a2b401fbb7e5aa6157e2b7", "714b635a2bbf15f12e289427369037eaca5ff486612d1fe22fdd3a62cebf68c4" },
                    { "87a0badfdb46a17d49b75f8883d577fcb4e52a11319c71d55b34539b64be5d91", "612c87ab126b71ea23a78e4b94d55a54ce15544ad61e867acffbaca1029c558e", "3a8cf27d147bb543e6b817971e5788817107f219ce8decbc725d58bc436c828c", "2afd9a8b557bc93ad6f67dbcdb07bddbbddfada018d5490f89d117f9de58fd31" },
                    { "327740c0a75cededf1baab20de27cac2d48ba39f9bcbf7e83925a5ce018ca5f9", "3e98a153ff9b87ac0b6e885973613ea43b12dc5421b3498ec09258faf2f6648b", "1a9013bc36433f70ecb414fd89d3150d38133adafe0c2c0c38822e0f0cc623c9", "b15f039f737a721f9e5c55585fb6f7cf3a718636c1112dece1db9da25839c37" },
                    { "5db5940ffbdc7b18046ef144970297605cdc88edc65ce7ee7997b53beb4f5eaf", "94ca7fe929b406fa60000575a98e04e9c789db6829569f54be92fa230dd814e8", "e811e665b8d8f4daeb00ed3ff00ee1e2921d8224881b518a2348a77a6bbbbe97", "77158396a1d24ca7b59f399c3e2d8eaffc037dc877b0b77f250b448616439c96" },
                    { "31576414bee06f4150a225c7dd3be65bb62ad10893b48adc8fce23638b470de1", "726a44487f6704ba8bcf29e02cf9a0a17a01c9b0cf416f73737a3d76332cb18a", "f48d25efafc9a917e9af93701cd8930502c4a292131d0d0c09067dc0eebc9a93", "c70f3e72421646182929dd33f5e7ed2cd1d0b817c7ffaa6d9e1e75305092f78" },
                    { "3d030d399f94c39d2201d976b06c28ec744ba33ced4f2a5067c10d4943ffe29a", "b6ea436ff24ec7d51ef4abaa77061f9f2f0f96b69ace1ebfd861c96e12e8a305", "9a28df436cc2091d55dbe2914597d7787a7951629ce5c3ad89ae6fd482de004a", "757565fed2e50061e535ef1c55a4795f1752c1af28b936885623f227d13bdae" },
                    { "755a775da4982fdcead37724b3d748fcc1d92c7372aacc08a9356bc2be88252b", "7f422d318f973adf196e19e8c2b80453f8b38686a0a3328b4ad6716be4b6b96c", "428622e31a95841fe0be9a1582cdd3d22645db8f06dbcf3a7677910b5ba0fb68", "3a6d78eb9c1dbfa8316999d64034f4023a99aaae5f7c341546cb10283ca599b9" },
                    { "806d82739b30565313567f641ba6c216f2b72d782ef7db9fa02b1ca19fd3042e", "c7dab033daad97b599379f6fc5e8a1ee0f6bff80a00d9af51c79dbfc46daf06f", "deed65e29b35ca7f15615288af3044dcc7f4abf7cb109618e048f9be5fe41400", "d994a72164324c11c73a272a7d8d60acc75bb8098682f7a6f3e77f4f607b5400" },
                    { "20197acb5d3540be6b04c112f1fef6198808b824ef851d346778e6f106ff94d3", "702169bd35fe3a249a2764501525e4ed83f47a35c4653e4e7f53037c3cd18d5a", "5de8bb972bda1756cb69109a8859b49dc1543a49d3da8070cded7893ca9d5ee4", "c9780c01e449212ae9dc05e6d929dfed9e0888fd33509bfc71a75da7168ad45" },
                    { "668b32dfc766d4b59280ba2f8bb5d883ecc876bcb669059d7c9920ac892dd4e7", "89cb3c32cce13e7cf40af225db57049c50162514871942b747813f1822016edd", "c55585817c790f8b0adca15f9726ac18e973b4eb03cb3803694ea6a1c9e78d9", "769b21f44e791788446d1c459dc10f5cfacde579c669bd9c401a608bd658265" },
                    { "86d050674f8fdbd180f80000577251e467bc5971668781856bc68790ef4b6115", "9d353e79c3fbd6ecdaa292a4ac533fdaad3446113f6da5bad7e51638010b1550", "e1855566cfe41a93327941f879ef945528b2ef8009557020a368057674448483", "20542a5bcf33d5132fed48f6536d362ce802f5d4ab77e4e8e6fa028ce459dca" },
                    { "b09e452761cffbd673dcf424a581e49acbe74b7ea7618c90d57f798ad3c5b2cc", "c28fe9942feb8f1b6b33d221fa2acbad5cffbbfd3ebd6e300981ece361667343", "5679a98fd2f6274057cbd02da16318142b9a3020921f7cd73431ef2bd7da5dc0", "11ba74b0e14078cce4e347c52a621e0d66f60c4d57e6ddcd18996c643d839280" },
                    { "dc4286889d13690ecc823d649039a88d84459a0a8bb7bb76f2061b467555d6e3", "4d1e2ff47129a52e1025c6fff4796b2cca35606d55bc3be6bd221a34c9b89f76", "960bdb366d7dda459a06c0a59e7a6c7582c4d2890ce1ffc88c7f3e621e465a8e", "50f6236a81903628577fd6bc389490ae450377a86c539ced2ab129ffa3727da5" },
                    { "c2c40d831c38b74d4c51d12250fb455d3765ca822fd4bd2a1d4c52016455717f", "3b55242bc183956adb3fc207b93cb947cc003c1b117e8d4f16a00c393ad22a1d", "fca25e0c450be776050d160a62c6821e4c376154ccc1c8b660bac61565eddda6", "25c1f36dd47328f8ba22db8d38ff165c2db483b743dd9c9eb05198ad377ed907" },
                    { "621f47b3828250eb68536d2a21cbedd76bdf62aa05d8025aa4c0c31293eb8624", "e7e25c330fcf36ee1b2d7a8adf1a90cb1addf73e58d4c8b9110b55a78f87edce", "2c631b946d07456852a70777800f3a6ce90bf646cd831752867bf575879e3f28", "3d7ecf8e7d3cedd7731572f6b9cde11123a6af5c4ddb9d34c029bed19bd15c0" },
                    { "2b6ce61b7aee1c3a41093431fda3d57000eca08226bfd8698725a4734abdd5a9", "25d5a86073d9c67f055f5037335fe99b09e10da46126b3af1ec84ec080eaebfb", "ff307f74317e0cf41868032e5177b2d3e4e6e2e5158190187930cf9beb414001", "871a986b0bc786e9d8b77fae8963336e0c76ba9781df1aa13f5306e3dfc4f320" },
                    { "2324b1e2ee16f7c2449dc6cea7ebe31f6d752ecb6c6d31ca96613ce1a3d33466", "78448dfb43c8bfd6c7dee3d2e15eae5c506cb46207da954ee1f71383a35d22db", "6c8bbad886ddd2718fd2d7479f0894de9e77ea6a5adae7a24ad72057bef22627", "a958c905acbdda69390974711491fcca2e0999977e87fc6eede69c4b30d418f" },
                    { "9198d1b817cc59910bbd0ec09a13ffe893c5366662b5cb0fd435d273d83a4f92", "cef6fa241d2dc6993a65034e5904f2109aa1456dc4edfd7f4fd3801ae7b2aeec", "7e36809af018c4903cec5296699a487c800fc74666cbb57603458ec131da4c4", "47e13a1d9c4cdfe9ad5a81f8d6855a9e0b7dddb0e1e42bf1123afca91506740" },
                    { "4140521bdf55da30a3a53883fe4ffab67bb87db167362f08e0db14d4625b8a35", "3e57e1ae27d5bf8f5ad00d87e0d0caf80f09b6dbb2b17a4689656a044a1ad903", "dd92347183591359fd0baabc873e65d6d458e22e6457b4d790a2f40169b450d8", "6b1de9344c7d1c86db7338c005985e014381397de6b6aa782ab402a959aec12d" },
                    { "6e27b59b5e06d7bd9e269fc96092e1abab60afc5d1efb7ec33af301366b0c8fb", "47ee8bbd8194b9ef1bb4b5aa0c22d7b2e48cf04b712a893e4001b6b77b3ebfdc", "5ddc68d6754ff42cbf3417c22c032589bc6ce47fa0a510a48a709c31c0ab8d64", "3aa97e822391925ed98942d8a774efba28ab7efc11c833b2f4f0e0c3cf4cd239" },
                    { "6f88510258aca3f70a3e1691f4a3c50623b81cc73f8a95933ac04cdb1d353cec", "295c929be69b9ba94499a29711274ca82ebf79ad96c10e9a85cd5c5f01d00a85", "8a21aa1789536f0fc0f72e0aa5885f9e33f06fa709740604b236692969a6865c", "1acde79964917c749ef8f56b6b4751bf6dc720776e58668b00ac0cdf22ed338c" },
                    { "725fbb2f91d44e1c052dad803de497878b8c2e02bf3761715b10ae82c95dd490", "98e9ec783f2a0dcadfe32255a5ee40c7edfa45c4cc6a32c86cf8f28a350c6d08", "848bb09d41833c3f7cbe21b7263a6aef964ff228939885f8c284d03c4723de23", "79726ad0c54838eb62961f673e8ccf6b614db1900ea2d0ba86458056bc58f1e7" },
                    { "dc6a5b7d53e9fe173138b3625a603d2b0355a7a14f4cf9d5189e1ac84e4f6f92", "5261ae65c10f966a7e1e43c5e7f015be1259f8d17685a66d587be18cdb360c35", "391354db11ea61a92a7399de68d1ffa06cc59dc88f0476d996fde4cffc206164", "11cefa9b20e1f820bbb60556ba6a1fcb1829d250076fbee525ab88590ae9148" },
                    { "c7f7ba95a9219f5e4151b9acbd43a8afdf0cca1ebd2475c026cc569c7aa87b8d", "72d174b1b28578f28bc9f4c029baf114c38575dd72cf01ce233ae7348ebc2763", "bb78baa9cfbbf148a61c8c3e5ff7f137d27baa35fa0ee527f6bc4e873a1eace3", "328e1e202293d2c981a1c0a49d38dbf4d924b3d6500b0d2edac9e965a7132bc" },
                    { "85ca9922e9a4fafb4dda0df76642df01cdafe8947bf06674a16d8267e6f5c49f", "69aa8d23b5a457535326de3f42d99691ba27c576f5cbe67740671a269bc7a75a", "bd95a4236d8c8858d27b0142009e72346efb1f386e08a18ad35f994cdd3b4b8a", "552ff85533baa6bd187e395b306e61076ad42d12b061c829819b8a73d12c1823" },
                    { "725c9bf9c26305e4d7b2a3a02dc04996b16dba00e25b4e523127e4538c30d4f", "2c24b7255a5ee2641f28ff5d6934048fef93766322500ca8c8060f382e087581", "a07e6e2c94d0943fdfb74fb2674e6b813e620d557afcd01cd39ba13731fb06eb", "890eefb174503b92bb73587081ee23ad5012b0e8af006602e67fb3322d80404b" },
                    { "a37dec9302d013b80160a17581f211eeaa3a56f49505590749bb65c5f75ccaec", "80e5e9edaf3c2e7f40546eb15f958fc1a24b666346a5aa70b6359034bac3f884", "57212b0f56d908e84f6ef8d540cecdb4481bdf269e9bd09ea5af087dcc85a36d", "46e7fa466ef3e365ec513af8f070b54cbb4f2ac6aa9a1268112490248d2202ad" },
                    { "af40016ca8afc4046b569acf54cb7f2834ff66f89c13f040ad85b8eaacaa51d6", "c08489eff53c69e0740818369c47356b76a49db3c090bbf23083af17a05382e8", "e6dd287fd100cab6ef39804ab4275e08379644ead4ea41144f7c74759ecbbd0d", "2c90467b499ed312cb6570639b5e9369e4f89a0fbbfa990ec101a6f5dafa73c1" },
                    { "78927ba67c7b8e01cb1224d2419eda69f54df983c59f1942b753e2ea04965b5c", "e645ec62ddb42d0314579f6ece55bfe5b092a4047e088496c3f70e4a9fadb3fa", "85878fac8cc8a4d62abb3eaa806de544484b54fd1d44d3e555699d6137215916", "351ca2845010426454653c396eac20d91d0d6e1fc974d9b620eda2b9e975717a" },
                    { "499a52a0bdd67d20dba5a01a8aaad1e86d2d021c38497a70ea96f453fa07d8a0", "b855ed0eb723004c83a86c5921df39e6120808e6b4528770165cc628c2ac23a7", "ec02a879839d8495ee3191991f184cd5addac27e7bce8f82cf978bb68d1fddc4", "5288834ac5c7cd9f097b7ffeb10a682771a14aaebab267f0afcc3a4f7926b94" },
                    { "dc246f155546272371aff147eb863e72cf01f776ff8bf9efa37ff18802ca974b", "609f9d4753c0d5b146c97f3690b6a86b33667a11851d0e40ff89d42890fa42dd", "3b8a9204e8fd07f6d1017edb1d1758deb1c3c55e84de47ad9a75d6cd18b3260", "a9a393f18ec2f97a6d905254df1a7adc089710f9d55e2f5602bf11b05b3d5b" },
                    { "7e94b792bbcd917d886eed394356b07aaee72ea07c2703dcf85dd45611b84296", "273dfa0b099f87399298e2837704a7acff09576cbbb30ca9d167050037f1915f", "d009c80d01c005da15759ec73ad88e5e95cf0c8ca1e64ba9029e3d0028870b5b", "61aae2fa59828c06de8a1f769895dc520d1744459ba16d0326183647ca7ba447" },
                    { "69a55ad4263d4ffab8a39184aaadfc5a2d43c7789901e6b6040532b08af59ff8", "f999af591e9a2e0267b656300a951e938ad140addd98992c6ec62ebab9fdf952", "41a70e7ee10e74de114e21badd89850528f28f006e3fa9214250b68d862abef0", "8badbef662be0690f4dd45a02a41e89f277aba26cab89e7f758a6ab391af4c0" },
                    { "f3912bcb38f28ceabeebc99181a922da3f10d21fb22f99245a91772f843465aa", "1b0348a4fe5c20833ac3e337fb83d584061491ed7a3b6054b07fdf0bc9f5947c", "8190e0bc1f557146f41f3cdbf2f3e96b09d6251f7552e881a5ae5bdec8ff2568", "77d1a93fb374e45679306197cf74a6b88462d04c84918cceb940389431bca300" },
                    { "7f00602dffce9353abee67caf4147cfea9372a3a545a1385114bf842706615f1", "75d89fb9793328b3220ed08220bdcb8263cf5717ddd2741f8f7697c497f7633c", "d9342f90c56c33ea6e98ac038525481911fb64b1b488e55abd1243d0e8380bd7", "ac640b04ca72fe573aeaaff1bf4eab4e91c5b86536b8601e0fbf723a9d469b64" },
                    { "5fdc3682ebb8d0ddc5690106f60f339c9bc51d9a92bc0a8de0aeac529d162097", "6a526979b75dce8dce7e987b8255596dc86f6c44db76d96fdbcf9f49e849e931", "f5cb19f5f1f57b23bbfe3fe2f01e7f21ad50072747439460d112fc06fe7bbc94", "d6ed988976bae51c96d05d007cb2767ff3caf8180f14b91a0fc96fe1098eadb" },
                    { "628f0b2d459865261535cfa5c409d8c29c3f4c1609c11b22a2433d7b379290f9", "b028bb1066fd66abcb85c7658d75e3d0357acc45cc33bb2c67ca3c953f2bf02d", "6598e5423e2d58d25028d3e980e47c3dcd4f5538ea07dea22f0061952c462194", "122ce77c28873d6fe9f7fed2bb074901a8e7f77e6ceccc803f7c60394401dc95" },
                    { "2b4208dc2fdce33127031035a6cc2eb0204d08d15f8c532e2933090ec600879e", "e5accafe64e55cb2e4751e41f6c6daf0fdf5a31e74ef3f5be22b33739c34be41", "ba27ffeb3a0fa9a207411b332b585607260ac1fcb8ac674a9175d823f5fc26f3", "98147a938497910afd5a3147dcb916269250f175fb7e493d5bea5100188e58ca" },
                    { "f2083e188bfe58dcb1809a38043c4a5f56e052ae6786a016573e52e51cbddf77", "5ad97bf2edd3e5f3596132c19156e311baf8ba4f9722cf3cc3ab754393a75753", "14af980f508aca1c1c0fdc2340eea996f56b53637c5bff85963760e9de9a87e2", "161412e8883082dd6b50480a2980f44cfc81feb60da243f32257e9fd571cc39" },
                    { "334e889db6fe96aecf351f14d755b4540faa2d18898376169e1391e698cdc1e0", "7bfa380a6943a350b6a279a0f4f7fb47ba7cb2e26854ea213b84aa1becd804e2", "1c966f8c73ae13b13b25d7d4ab88ef85645a3b62d02c343276fa66c6111caab6", "147504ac25b6c0405a029b4a8cc84f1a3c02faf097278647e260f01b98c5193e" },
                    { "e1f1af5250132545045302d1fcc47762c36058012c60d25f1eb25b325bd305e1", "e7ed52bad67187e9015de34ce1c95c53ab1b8bb10c51c7178f717b0f0b24ffd2", "e12b2024ae81ea48d8b2933dba5e36e83825265d633d82b395c80465ea14d25b", "89be9f3d896c4ff31647fb78eb5efc39304dc80afd829b241315837eb5abb077" },
                    { "c13f850768eca0ed61a6aff05ed951a1367728a293d85e674d96da115bc9abc3", "a3b7d57849458c730ff21cd13a512a10f74c8774059cbbe0788283bf14fdff0", "241d6ab1cefa88b2fb584d40ea6095405d97328f9cc6aa2bc3d9e03f2c6e899c", "10f720b6c688caad10fecc9b038dd944ffde3d08b477eaf993af280509295b45" },
                    { "df2401a8d41a846f45dff43a8e0e362e0c446059a25646e16882b2ac6c077169", "f83e7cb26f62f7766ee0b085fad71f6ca46b06787fad6d806777331cfed5276a", "f9398ce4391ae71a4c14e5bcd3d56431ed5c81d8807d8c0b05bd6b7ada707146", "20af0b0d13e7a979b21f0298af1da178861766a9992c5202ff467e10b8db907f" },
                    { "2782d7b79cc0da7c30e6cf5d612bc52883da575ea91b3de0d4d17e3820ac4b93", "c875566ebdda0f8dcd1f74f4a21607be3557979ca8f10a4637108c753a2fbf20", "e9c7ed041188a33eb35afc39725dbee38b81d2f1bca8ffb4cee1ff7e94c45030", "9c5cc49f58176b0c0b40f8e5fc196a1c9525665d336b3eaa91c83176469f1101" },
                    { "f924a6d2381b7f2c29316d866488e503f6b687a683d6331c1d093b9fe90e99fa", "f875cd51f797081a69ad26bdc40711683a7f412b0f13719c6bcb5eff1bb2f56e", "37ffbc257aa4f1750d24e1b5a608e20e81cd20552e53d83447de88ece2079dc0", "18ea5792c5bcf8a55e8c6074f01dda53d10cbf5fa0878f789df11e91ef31ed00" },
                    { "5ee2b59eaa32003ffcf5d793223c59a45e767cd9c8df661444f0a3244217191d", "fa208d26272b3e707e512a833277e45c88d02cbd4428dc7025c55995c3bbff68", "55ac102a5ae9fe02be4deef330b1462cf178e6f908a0b1e30763012980ebafd", "4cb35ecb67146fdfa542f38690cab6a9f29d8c8a37022821273ec9cd7889610" },
                    { "8d2a667633a177f30ed31d3028da6fecf07336471873661b348e034f19a5ea18", "10a06071076fa9365da7762a337b9a04beb6f0ca6b1f5f62cf0fb98f3dd53f00", "3fa6df42503d9caaa01f0d43ee1847ed754a5729d5fe721ef172e2f6a72eaea5", "3092d08ccc41515c84f71aee233323693dfa8344a89b7b0d77f1289ba346f8b2" },
                    { "9d01ae3db1b95bcc4dd38228be3ec01d9ace998902df385b3ed9c0637009a3b4", "73e03dd451e6a6ce3a93b653e7f16809cf7267136ea30c5ef0c6ef7930a5da59", "806cac56cd7822b0753c5454e64a47774aa5fdaab7645d17af5744f99adfe5e1", "5340d73cb021600190cc9603c64a74da77d72a9146ff5924d53185693452ecd" },
                    { "3ec39761a40a9d8145bc1dca1e17813e5accffee14c13387c4069b05345aeb71", "4bc1700e2095e6e95b2b6ca1c3516d5eba7fcfed66779e1358d364ec4ef657d4", "cb7d7abf1c9de69ceac279e4aec2c322a2bd80b137bf83d48168f5c791e06150", "b9043724f998c62aa2808590348c3946cad96c962cc98609ba9f7dfb59c4f0b1" },
                    { "8c7800563c0bb0c2acd1b406ccdb35fb58e7f3be49f4ca6e8e7c62d2b818e910", "606a187b4d943bb792f1b65be5bd5721016fe73a8d49a46589c90b9f887ef07f", "fac1ec802edadd3a55d2cceda67b6fd1f41703fb0e0e5338cb970776c51d4316", "ca82fe34326c6aa5cdfa23165b89c85eee10f51e1f2a6bbc7ef6b795ce1f3d32" },
                    { "6be25fdbad664b25ee343071b68f03b4b53677d216720babcf3448b5cacc09b6", "f1eff50e9585eef36ccb8577accababdeed46c57e3785dc60b509ab753c1cc77", "3619e7b103428a10ac7c00a34d08d575d0531f37f4b32ba72d5d39cce5cf1ce7", "1f585fc835cfd9c30946424bf9e8e14970bc0109645144953dd8840836728b9c" },
                    { "a42e9912f8ed3723c597175dd6e720c207d65dac3e86bcc327aff2bfafc63d03", "9754b072a9eacb6de03ae58a92de20f986e77430ccb062d0b0484d49b4cd2c79", "ad151dc1f3901c5f0ebd4981d96395767039fc31f723cec6f93ee0b7d29e4f42", "4c6fd173353989a9bac1fc7ee776ffaaa566a184c9fcff46b8672e420b4e4a9d" },
                    { "729e7f30c844aef14c8322fbfe3c6c4c767d260c876a9493c92ca40f61cd405b", "cd483716833013c77726b1fe524f9c84b1da7f9537aa047815d1b0cd929f95cb", "b0ec037e333bcded79dae523bb5c9b61aa9bb40e88aa87a42a39405e0bfc1490", "859726bbbbef922b6334ccd1623c0118a550e5482923147e822f1c45844439c3" },
                };

                for (const auto [base, exponent, modulus, result] : cases)
                {
                    const auto parseAsBigInt = [] (const char* str)
                    {
                        BigInteger b;
                        b.parseString (str, 16);
                        return b;
                    };

                    auto computed = parseAsBigInt (base);
                    computed.exponentModulo (parseAsBigInt (exponent), parseAsBigInt (modulus));
                    expect (computed == parseAsBigInt (result));
                }
            }
        }
    }
};

static BigIntegerTests bigIntegerTests;

#endif

} // namespace juce
