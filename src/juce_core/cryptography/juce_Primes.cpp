/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330, 
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#include "../basics/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE


#include "juce_Primes.h"


//==============================================================================
static void createSmallSieve (int numBits, BitArray& result)
{
    result.setBit (numBits);
    result.clearBit (numBits); // to enlarge the array

    result.setBit (0);
    int index = 1;

    do
    {
        const int step = (index << 1) + 1;

        for (int i = index + step; i < numBits; i += step)
        {
            jassert (i != 6);
            result.setBit (i);
        }

        index = result.findNextClearBit (index + 1);
    }
    while (index < numBits);
}

static void bigSieve (const BitArray& base,
                      int numBits,
                      BitArray& result,
                      const BitArray& smallSieve,
                      const int smallSieveSize)
{
    jassert (! base[0]); // must be even!

    result.setBit (numBits);
    result.clearBit (numBits);  // to enlarge the array

    int index = smallSieve.findNextClearBit (0);

    do
    {
        const int prime = (index << 1) + 1;

        BitArray r (base);
        BitArray remainder;
        r.divideBy (prime, remainder);

        int i = prime - remainder.getBitRangeAsInt (0, 32);

        if (r.isEmpty())
            i += prime;

        if ((i & 1) == 0)
            i += prime;

        i = (i - 1) >> 1;

        while (i < numBits)
        {
            result.setBit (i);
            i += prime;
        }

        index = smallSieve.findNextClearBit (index + 1);
    }
    while (index < smallSieveSize);
}

static bool findCandidate (const BitArray& base,
                           const BitArray& sieve,
                           int numBits,
                           BitArray& result,
                           int certainty)
{
    for (int i = 0; i < numBits; ++i)
    {
        if (! sieve[i])
        {
            result = base;
            result.add (BitArray ((unsigned int) ((i << 1) + 1)));

            if (Primes::isProbablyPrime (result, certainty))
                return true;
        }
    }

    return false;
}

//==============================================================================
const BitArray Primes::createProbablePrime (int bitLength, int certainty)
{
    BitArray smallSieve;
    const int smallSieveSize = 15000;
    createSmallSieve (smallSieveSize, smallSieve);

    BitArray p;
    p.fillBitsRandomly (0, bitLength);
    p.setBit (bitLength - 1);
    p.clearBit (0);

    const int searchLen = jmax (1024, (bitLength / 20) * 64);

    while (p.getHighestBit() < bitLength)
    {
        p.add (2 * searchLen);

        BitArray sieve;
        bigSieve (p, searchLen, sieve,
                  smallSieve, smallSieveSize);

        BitArray candidate;

        if (findCandidate (p, sieve, searchLen, candidate, certainty))
            return candidate;
    }

    jassertfalse
    return BitArray();
}

static bool passesMillerRabin (const BitArray& n, int iterations)
{
    const BitArray one (1);
    const BitArray two (2);

    BitArray nMinusOne (n);
    nMinusOne.subtract (one);

    BitArray d (nMinusOne);
    const int s = d.findNextSetBit (0);
    d.shiftBits (-s);

    while (--iterations >= 0)
    {
        BitArray r;
        r.createRandomNumber (nMinusOne);
        r.exponentModulo (d, n);

        if (! (r == one || r == nMinusOne))
        {
            for (int j = 0; j < s; ++j)
            {
                r.exponentModulo (two, n);

                if (r == nMinusOne)
                    break;
            }

            if (r != nMinusOne)
                return false;
        }
    }

    return true;
}

bool Primes::isProbablyPrime (const BitArray& number, int certainty)
{
    if (! number[0])
        return false;

    if (number.getHighestBit() <= 10)
    {
        const int num = number.getBitRangeAsInt (0, 10);

        for (int i = num / 2; --i > 1;)
            if (num % i == 0)
                return false;

        return true;
    }
    else
    {
        return passesMillerRabin (number, certainty);
    }
}

END_JUCE_NAMESPACE
