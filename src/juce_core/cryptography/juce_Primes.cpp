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
static void createSmallSieve (const int numBits, BitArray& result) throw()
{
    result.setBit (numBits);
    result.clearBit (numBits); // to enlarge the array

    result.setBit (0);
    int n = 2;

    do
    {
        for (int i = n + n; i < numBits; i += n)
            result.setBit (i);

        n = result.findNextClearBit (n + 1);
    }
    while (n <= (numBits >> 1));
}

static void bigSieve (const BitArray& base,
                      const int numBits,
                      BitArray& result,
                      const BitArray& smallSieve,
                      const int smallSieveSize) throw()
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
                           const int numBits,
                           BitArray& result,
                           const int certainty) throw()
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
const BitArray Primes::createProbablePrime (const int bitLength,
                                            const int certainty) throw()
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

static bool passesMillerRabin (const BitArray& n, int iterations) throw()
{
    const BitArray one (1);
    const BitArray two (2);

    BitArray nMinusOne (n);
    nMinusOne.subtract (one);

    BitArray d (nMinusOne);
    const int s = d.findNextSetBit (0);
    d.shiftBits (-s);

    BitArray smallPrimes;
    int numBitsInSmallPrimes = 0;

    for (;;)
    {
        numBitsInSmallPrimes += 256;
        createSmallSieve (numBitsInSmallPrimes, smallPrimes);

        const int numPrimesFound = numBitsInSmallPrimes - smallPrimes.countNumberOfSetBits();

        if (numPrimesFound > iterations + 1)
            break;
    }

    int smallPrime = 2;

    while (--iterations >= 0)
    {
        smallPrime = smallPrimes.findNextClearBit (smallPrime + 1);

        BitArray r (smallPrime);
        //r.createRandomNumber (nMinusOne);
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

bool Primes::isProbablyPrime (const BitArray& number,
                              const int certainty) throw()
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
        const BitArray screen (2 * 3 * 5 * 7 * 11 * 13 * 17 * 19 * 23);

        if (number.findGreatestCommonDivisor (screen) != BitArray (1))
            return false;

        return passesMillerRabin (number, certainty);
    }
}

END_JUCE_NAMESPACE
