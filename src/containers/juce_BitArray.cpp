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

#include "../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE


#include "juce_BitArray.h"
#include "juce_MemoryBlock.h"
#include "../core/juce_Random.h"


//==============================================================================
BitArray::BitArray() throw()
    : numValues (4),
      highestBit (-1),
      negative (false)
{
    values.calloc (numValues + 1);
}

BitArray::BitArray (const int value) throw()
    : numValues (4),
      highestBit (31),
      negative (value < 0)
{
    values.calloc (numValues + 1);
    values[0] = abs (value);
    highestBit = getHighestBit();
}

BitArray::BitArray (int64 value) throw()
    : numValues (4),
      highestBit (63),
      negative (value < 0)
{
    values.calloc (numValues + 1);

    if (value < 0)
        value = -value;

    values[0] = (unsigned int) value;
    values[1] = (unsigned int) (value >> 32);
    highestBit = getHighestBit();
}

BitArray::BitArray (const unsigned int value) throw()
    : numValues (4),
      highestBit (31),
      negative (false)
{
    values.calloc (numValues + 1);
    values[0] = value;
    highestBit = getHighestBit();
}

BitArray::BitArray (const BitArray& other) throw()
    : numValues (jmax (4, (other.highestBit >> 5) + 1)),
      highestBit (other.getHighestBit()),
      negative (other.negative)
{
    values.malloc (numValues + 1);
    memcpy (values, other.values, sizeof (unsigned int) * (numValues + 1));
}

BitArray::~BitArray() throw()
{
}

const BitArray& BitArray::operator= (const BitArray& other) throw()
{
    if (this != &other)
    {
        highestBit = other.getHighestBit();
        numValues = jmax (4, (highestBit >> 5) + 1);
        negative = other.negative;
        values.malloc (numValues + 1);
        memcpy (values, other.values, sizeof (unsigned int) * (numValues + 1));
    }

    return *this;
}

// result == 0 = the same
// result < 0  = this number is smaller
// result > 0  = this number is bigger
int BitArray::compare (const BitArray& other) const throw()
{
    if (isNegative() == other.isNegative())
    {
        const int absComp = compareAbsolute (other);
        return isNegative() ? -absComp : absComp;
    }
    else
    {
        return isNegative() ? -1 : 1;
    }
}

int BitArray::compareAbsolute (const BitArray& other) const throw()
{
    const int h1 = getHighestBit();
    const int h2 = other.getHighestBit();

    if (h1 > h2)
        return 1;
    else if (h1 < h2)
        return -1;

    for (int i = (h1 >> 5) + 1; --i >= 0;)
        if (values[i] != other.values[i])
            return (values[i] > other.values[i]) ? 1 : -1;

    return 0;
}

bool BitArray::operator== (const BitArray& other) const throw()
{
    return compare (other) == 0;
}

bool BitArray::operator!= (const BitArray& other) const throw()
{
    return compare (other) != 0;
}

bool BitArray::operator[] (const int bit) const throw()
{
    return bit >= 0 && bit <= highestBit
             && ((values [bit >> 5] & (1 << (bit & 31))) != 0);
}

bool BitArray::isEmpty() const throw()
{
    return getHighestBit() < 0;
}

void BitArray::clear() throw()
{
    if (numValues > 16)
    {
        numValues = 4;
        values.calloc (numValues + 1);
    }
    else
    {
        zeromem (values, sizeof (unsigned int) * (numValues + 1));
    }

    highestBit = -1;
    negative = false;
}

void BitArray::setBit (const int bit) throw()
{
    if (bit >= 0)
    {
        if (bit > highestBit)
        {
            ensureSize (bit >> 5);
            highestBit = bit;
        }

        values [bit >> 5] |= (1 << (bit & 31));
    }
}

void BitArray::setBit (const int bit,
                       const bool shouldBeSet) throw()
{
    if (shouldBeSet)
        setBit (bit);
    else
        clearBit (bit);
}

void BitArray::clearBit (const int bit) throw()
{
    if (bit >= 0 && bit <= highestBit)
        values [bit >> 5] &= ~(1 << (bit & 31));
}

void BitArray::setRange (int startBit,
                         int numBits,
                         const bool shouldBeSet) throw()
{
    while (--numBits >= 0)
        setBit (startBit++, shouldBeSet);
}

void BitArray::insertBit (const int bit,
                          const bool shouldBeSet) throw()
{
    if (bit >= 0)
        shiftBits (1, bit);

    setBit (bit, shouldBeSet);
}

//==============================================================================
void BitArray::andWith (const BitArray& other) throw()
{
    // this operation will only work with the absolute values
    jassert (isNegative() == other.isNegative());

    int n = numValues;

    while (n > other.numValues)
        values[--n] = 0;

    while (--n >= 0)
        values[n] &= other.values[n];

    if (other.highestBit < highestBit)
        highestBit = other.highestBit;

    highestBit = getHighestBit();
}

void BitArray::orWith (const BitArray& other) throw()
{
    if (other.highestBit < 0)
        return;

    // this operation will only work with the absolute values
    jassert (isNegative() == other.isNegative());

    ensureSize (other.highestBit >> 5);

    int n = (other.highestBit >> 5) + 1;

    while (--n >= 0)
        values[n] |= other.values[n];

    if (other.highestBit > highestBit)
        highestBit = other.highestBit;

    highestBit = getHighestBit();
}

void BitArray::xorWith (const BitArray& other) throw()
{
    if (other.highestBit < 0)
        return;

    // this operation will only work with the absolute values
    jassert (isNegative() == other.isNegative());

    ensureSize (other.highestBit >> 5);

    int n = (other.highestBit >> 5) + 1;

    while (--n >= 0)
        values[n] ^= other.values[n];

    if (other.highestBit > highestBit)
        highestBit = other.highestBit;

    highestBit = getHighestBit();
}

//==============================================================================
void BitArray::add (const BitArray& other) throw()
{
    if (other.isNegative())
    {
        BitArray o (other);
        o.negate();
        subtract (o);
        return;
    }

    if (isNegative())
    {
        if (compareAbsolute (other) < 0)
        {
            BitArray temp (*this);
            temp.negate();
            *this = other;
            subtract (temp);
        }
        else
        {
            negate();
            subtract (other);
            negate();
        }

        return;
    }

    if (other.highestBit > highestBit)
        highestBit = other.highestBit;

    ++highestBit;

    const int numInts = (highestBit >> 5) + 1;
    ensureSize (numInts);

    int64 remainder = 0;

    for (int i = 0; i <= numInts; ++i)
    {
        if (i < numValues)
            remainder += values[i];

        if (i < other.numValues)
            remainder += other.values[i];

        values[i] = (unsigned int) remainder;
        remainder >>= 32;
    }

    jassert (remainder == 0);
    highestBit = getHighestBit();
}

void BitArray::subtract (const BitArray& other) throw()
{
    if (other.isNegative())
    {
        BitArray o (other);
        o.negate();
        add (o);
        return;
    }

    if (! isNegative())
    {
        if (compareAbsolute (other) < 0)
        {
            BitArray temp (*this);
            *this = other;
            subtract (temp);
            negate();
            return;
        }
    }
    else
    {
        negate();
        add (other);
        negate();
        return;
    }

    const int numInts = (highestBit >> 5) + 1;
    const int maxOtherInts = (other.highestBit >> 5) + 1;
    int64 amountToSubtract = 0;

    for (int i = 0; i <= numInts; ++i)
    {
        if (i <= maxOtherInts)
            amountToSubtract += (int64)other.values[i];

        if (values[i] >= amountToSubtract)
        {
            values[i] = (unsigned int) (values[i] - amountToSubtract);
            amountToSubtract = 0;
        }
        else
        {
            const int64 n = ((int64) values[i] + (((int64) 1) << 32)) - amountToSubtract;
            values[i] = (unsigned int) n;
            amountToSubtract = 1;
        }
    }
}

void BitArray::multiplyBy (const BitArray& other) throw()
{
    BitArray total;
    highestBit = getHighestBit();
    const bool wasNegative = isNegative();
    setNegative (false);

    for (int i = 0; i <= highestBit; ++i)
    {
        if (operator[](i))
        {
            BitArray n (other);
            n.setNegative (false);
            n.shiftBits (i);
            total.add (n);
        }
    }

    *this = total;
    negative = wasNegative ^ other.isNegative();
}

void BitArray::divideBy (const BitArray& divisor, BitArray& remainder) throw()
{
    jassert (this != &remainder); // (can't handle passing itself in to get the remainder)

    const int divHB = divisor.getHighestBit();
    const int ourHB = getHighestBit();

    if (divHB < 0 || ourHB < 0)
    {
        // division by zero
        remainder.clear();
        clear();
    }
    else
    {
        remainder = *this;
        remainder.setNegative (false);
        const bool wasNegative = isNegative();
        clear();

        BitArray temp (divisor);
        temp.setNegative (false);

        int leftShift = ourHB - divHB;
        temp.shiftBits (leftShift);

        while (leftShift >= 0)
        {
            if (remainder.compareAbsolute (temp) >= 0)
            {
                remainder.subtract (temp);
                setBit (leftShift);
            }

            if (--leftShift >= 0)
                temp.shiftBits (-1);
        }

        negative = wasNegative ^ divisor.isNegative();
        remainder.setNegative (wasNegative);
    }
}

void BitArray::modulo (const BitArray& divisor) throw()
{
    BitArray remainder;
    divideBy (divisor, remainder);
    *this = remainder;
}

static const BitArray simpleGCD (BitArray* m, BitArray* n) throw()
{
    while (! m->isEmpty())
    {
        if (n->compareAbsolute (*m) > 0)
            swapVariables (m, n);

        m->subtract (*n);
    }

    return *n;
}

const BitArray BitArray::findGreatestCommonDivisor (BitArray n) const throw()
{
    BitArray m (*this);

    while (! n.isEmpty())
    {
        if (abs (m.getHighestBit() - n.getHighestBit()) <= 16)
            return simpleGCD (&m, &n);

        BitArray temp1 (m), temp2;
        temp1.divideBy (n, temp2);

        m = n;
        n = temp2;
    }

    return m;
}

void BitArray::exponentModulo (const BitArray& exponent,
                               const BitArray& modulus) throw()
{
    BitArray exp (exponent);
    exp.modulo (modulus);

    BitArray value (*this);
    value.modulo (modulus);

    clear();
    setBit (0);

    while (! exp.isEmpty())
    {
        if (exp [0])
        {
            multiplyBy (value);
            this->modulo (modulus);
        }

        value.multiplyBy (value);
        value.modulo (modulus);

        exp.shiftBits (-1);
    }
}

void BitArray::inverseModulo (const BitArray& modulus) throw()
{
    const BitArray one (1);

    if (modulus == one || modulus.isNegative())
    {
        clear();
        return;
    }

    if (isNegative() || compareAbsolute (modulus) >= 0)
        this->modulo (modulus);

    if (*this == one)
        return;

    if (! (*this)[0])
    {
        // not invertible
        clear();
        return;
    }

    BitArray a1 (modulus);
    BitArray a2 (*this);
    BitArray b1 (modulus);
    BitArray b2 (1);

    while (a2 != one)
    {
        BitArray temp1, temp2, multiplier (a1);
        multiplier.divideBy (a2, temp1);

        temp1 = a2;
        temp1.multiplyBy (multiplier);
        temp2 = a1;
        temp2.subtract (temp1);
        a1 = a2;
        a2 = temp2;

        temp1 = b2;
        temp1.multiplyBy (multiplier);
        temp2 = b1;
        temp2.subtract (temp1);
        b1 = b2;
        b2 = temp2;
    }

    while (b2.isNegative())
        b2.add (modulus);

    b2.modulo (modulus);
    *this = b2;
}

//==============================================================================
void BitArray::shiftBits (int bits, const int startBit) throw()
{
    if (highestBit < 0)
        return;

    if (startBit > 0)
    {
        if (bits < 0)
        {
            // right shift
            for (int i = startBit; i <= highestBit; ++i)
                setBit (i, operator[] (i - bits));

            highestBit = getHighestBit();
        }
        else if (bits > 0)
        {
            // left shift
            for (int i = highestBit + 1; --i >= startBit;)
                setBit (i + bits, operator[] (i));

            while (--bits >= 0)
                clearBit (bits + startBit);
        }
    }
    else
    {
        if (bits < 0)
        {
            // right shift
            bits = -bits;

            if (bits > highestBit)
            {
                clear();
            }
            else
            {
                const int wordsToMove = bits >> 5;
                int top = 1 + (highestBit >> 5) - wordsToMove;
                highestBit -= bits;

                if (wordsToMove > 0)
                {
                    int i;
                    for (i = 0; i < top; ++i)
                        values [i] = values [i + wordsToMove];

                    for (i = 0; i < wordsToMove; ++i)
                        values [top + i] = 0;

                    bits &= 31;
                }

                if (bits != 0)
                {
                    const int invBits = 32 - bits;

                    --top;
                    for (int i = 0; i < top; ++i)
                        values[i] = (values[i] >> bits) | (values [i + 1] << invBits);

                    values[top] = (values[top] >> bits);
                }

                highestBit = getHighestBit();
            }
        }
        else if (bits > 0)
        {
            // left shift
            ensureSize (((highestBit + bits) >> 5) + 1);

            const int wordsToMove = bits >> 5;
            int top = 1 + (highestBit >> 5);
            highestBit += bits;

            if (wordsToMove > 0)
            {
                int i;
                for (i = top; --i >= 0;)
                    values [i + wordsToMove] = values [i];

                for (i = 0; i < wordsToMove; ++i)
                    values [i] = 0;

                bits &= 31;
            }

            if (bits != 0)
            {
                const int invBits = 32 - bits;

                for (int i = top + 1 + wordsToMove; --i > wordsToMove;)
                    values[i] = (values[i] << bits) | (values [i - 1] >> invBits);

                values [wordsToMove] = values [wordsToMove] << bits;
            }

            highestBit = getHighestBit();
        }
    }
}

const BitArray BitArray::getBitRange (int startBit, int numBits) const throw()
{
    BitArray r;
    numBits = jmin (numBits, getHighestBit() + 1 - startBit);
    r.ensureSize (numBits >> 5);
    r.highestBit = numBits;

    int i = 0;
    while (numBits > 0)
    {
        r.values[i++] = getBitRangeAsInt (startBit, jmin (32, numBits));
        numBits -= 32;
        startBit += 32;
    }

    r.highestBit = r.getHighestBit();

    return r;
}

int BitArray::getBitRangeAsInt (const int startBit, int numBits) const throw()
{
    if (numBits > 32)
    {
        jassertfalse  // use getBitRange() if you need more than 32 bits..
        numBits = 32;
    }

    numBits = jmin (numBits, highestBit + 1 - startBit);

    if (numBits <= 0)
        return 0;

    const int pos = startBit >> 5;
    const int offset = startBit & 31;
    const int endSpace = 32 - numBits;

    uint32 n = ((uint32) values [pos]) >> offset;

    if (offset > endSpace)
        n |= ((uint32) values [pos + 1]) << (32 - offset);

    return (int) (n & (((uint32) 0xffffffff) >> endSpace));
}

void BitArray::setBitRangeAsInt (const int startBit, int numBits, unsigned int valueToSet) throw()
{
    if (numBits > 32)
    {
        jassertfalse
        numBits = 32;
    }

    for (int i = 0; i < numBits; ++i)
    {
        setBit (startBit + i, (valueToSet & 1) != 0);
        valueToSet >>= 1;
    }
}

//==============================================================================
bool BitArray::isNegative() const throw()
{
    return negative && ! isEmpty();
}

void BitArray::setNegative (const bool neg) throw()
{
    negative = neg;
}

void BitArray::negate() throw()
{
    negative = (! negative) && ! isEmpty();
}

int BitArray::countNumberOfSetBits() const throw()
{
    int total = 0;

    for (int i = (highestBit >> 5) + 1; --i >= 0;)
    {
        unsigned int n = values[i];

        if (n == 0xffffffff)
        {
            total += 32;
        }
        else
        {
            while (n != 0)
            {
                total += (n & 1);
                n >>= 1;
            }
        }
    }

    return total;
}

int BitArray::getHighestBit() const throw()
{
    for (int i = highestBit + 1; --i >= 0;)
        if ((values [i >> 5] & (1 << (i & 31))) != 0)
            return i;

    return -1;
}

int BitArray::findNextSetBit (int i) const throw()
{
    for (; i <= highestBit; ++i)
        if ((values [i >> 5] & (1 << (i & 31))) != 0)
            return i;

    return -1;
}

int BitArray::findNextClearBit (int i) const throw()
{
    for (; i <= highestBit; ++i)
        if ((values [i >> 5] & (1 << (i & 31))) == 0)
            break;

    return i;
}

void BitArray::ensureSize (const int numVals) throw()
{
    if (numVals + 2 >= numValues)
    {
        int oldSize = numValues;
        numValues = ((numVals + 2) * 3) / 2;
        values.realloc (numValues + 1);

        while (oldSize < numValues)
            values [oldSize++] = 0;
    }
}

//==============================================================================
const String BitArray::toString (const int base, const int minimumNumCharacters) const throw()
{
    String s;
    BitArray v (*this);

    if (base == 2 || base == 8 || base == 16)
    {
        const int bits = (base == 2) ? 1 : (base == 8 ? 3 : 4);
        static const tchar* const hexDigits = T("0123456789abcdef");

        for (;;)
        {
            const int remainder = v.getBitRangeAsInt (0, bits);

            v.shiftBits (-bits);

            if (remainder == 0 && v.isEmpty())
                break;

            s = String::charToString (hexDigits [remainder]) + s;
        }
    }
    else if (base == 10)
    {
        const BitArray ten (10);
        BitArray remainder;

        for (;;)
        {
            v.divideBy (ten, remainder);

            if (remainder.isEmpty() && v.isEmpty())
                break;

            s = String (remainder.getBitRangeAsInt (0, 8)) + s;
        }
    }
    else
    {
        jassertfalse // can't do the specified base
        return String::empty;
    }

    const int length = s.length();

    if (length < minimumNumCharacters)
        s = String::repeatedString (T("0"), minimumNumCharacters - length) + s;

    return isNegative() ? T("-") + s : s;
}

void BitArray::parseString (const String& text,
                            const int base) throw()
{
    clear();
    const tchar* t = (const tchar*) text;

    if (base == 2 || base == 8 || base == 16)
    {
        const int bits = (base == 2) ? 1 : (base == 8 ? 3 : 4);

        for (;;)
        {
            const tchar c = *t++;
            const int digit = CharacterFunctions::getHexDigitValue (c);

            if (((unsigned int) digit) < (unsigned int) base)
            {
                shiftBits (bits);
                add (digit);
            }
            else if (c == 0)
            {
                break;
            }
        }
    }
    else if (base == 10)
    {
        const BitArray ten ((unsigned int) 10);

        for (;;)
        {
            const tchar c = *t++;

            if (c >= T('0') && c <= T('9'))
            {
                multiplyBy (ten);
                add ((int) (c - T('0')));
            }
            else if (c == 0)
            {
                break;
            }
        }
    }

    setNegative (text.trimStart().startsWithChar (T('-')));
}

const MemoryBlock BitArray::toMemoryBlock() const throw()
{
    const int numBytes = (getHighestBit() + 8) >> 3;
    MemoryBlock mb ((size_t) numBytes);

    for (int i = 0; i < numBytes; ++i)
        mb[i] = (uint8) getBitRangeAsInt (i << 3, 8);

    return mb;
}

void BitArray::loadFromMemoryBlock (const MemoryBlock& data) throw()
{
    clear();

    for (size_t i = data.getSize(); --i >= 0;)
        this->setBitRangeAsInt ((int) (i << 3), 8, data [i]);
}

END_JUCE_NAMESPACE
