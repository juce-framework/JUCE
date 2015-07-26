/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

namespace PrimesHelpers
{
    static void createSmallSieve (const int numBits, BigInteger& result)
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

    static void bigSieve (const BigInteger& base, const int numBits, BigInteger& result,
                          const BigInteger& smallSieve, const int smallSieveSize)
    {
        jassert (! base[0]); // must be even!

        result.setBit (numBits);
        result.clearBit (numBits);  // to enlarge the array

        int index = smallSieve.findNextClearBit (0);

        do
        {
            const unsigned int prime = ((unsigned int) index << 1) + 1;

            BigInteger r (base), remainder;
            r.divideBy (prime, remainder);

            unsigned int i = prime - remainder.getBitRangeAsInt (0, 32);

            if (r.isZero())
                i += prime;

            if ((i & 1) == 0)
                i += prime;

            i = (i - 1) >> 1;

            while (i < (unsigned int) numBits)
            {
                result.setBit ((int) i);
                i += prime;
            }

            index = smallSieve.findNextClearBit (index + 1);
        }
        while (index < smallSieveSize);
    }

    static bool findCandidate (const BigInteger& base, const BigInteger& sieve,
                               const int numBits, BigInteger& result, const int certainty)
    {
        for (int i = 0; i < numBits; ++i)
        {
            if (! sieve[i])
            {
                result = base + (unsigned int) ((i << 1) + 1);

                if (Primes::isProbablyPrime (result, certainty))
                    return true;
            }
        }

        return false;
    }

    static bool passesMillerRabin (const BigInteger& n, int iterations)
    {
        const BigInteger one (1), two (2);
        const BigInteger nMinusOne (n - one);

        BigInteger d (nMinusOne);
        const int s = d.findNextSetBit (0);
        d >>= s;

        BigInteger smallPrimes;
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

            BigInteger r (smallPrime);
            r.exponentModulo (d, n);

            if (r != one && r != nMinusOne)
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
}

//==============================================================================
BigInteger Primes::createProbablePrime (const int bitLength,
                                        const int certainty,
                                        const int* randomSeeds,
                                        int numRandomSeeds)
{
    using namespace PrimesHelpers;
    int defaultSeeds [16];

    if (numRandomSeeds <= 0)
    {
        randomSeeds = defaultSeeds;
        numRandomSeeds = numElementsInArray (defaultSeeds);
        Random r1, r2;

        for (int j = 10; --j >= 0;)
        {
            r1.setSeedRandomly();

            for (int i = numRandomSeeds; --i >= 0;)
                defaultSeeds[i] ^= r1.nextInt() ^ r2.nextInt();
        }
    }

    BigInteger smallSieve;
    const int smallSieveSize = 15000;
    createSmallSieve (smallSieveSize, smallSieve);

    BigInteger p;

    for (int i = numRandomSeeds; --i >= 0;)
    {
        BigInteger p2;

        Random r (randomSeeds[i]);
        r.fillBitsRandomly (p2, 0, bitLength);

        p ^= p2;
    }

    p.setBit (bitLength - 1);
    p.clearBit (0);

    const int searchLen = jmax (1024, (bitLength / 20) * 64);

    while (p.getHighestBit() < bitLength)
    {
        p += 2 * searchLen;

        BigInteger sieve;
        bigSieve (p, searchLen, sieve,
                  smallSieve, smallSieveSize);

        BigInteger candidate;

        if (findCandidate (p, sieve, searchLen, candidate, certainty))
            return candidate;
    }

    jassertfalse;
    return BigInteger();
}

bool Primes::isProbablyPrime (const BigInteger& number, const int certainty)
{
    using namespace PrimesHelpers;

    if (! number[0])
        return false;

    if (number.getHighestBit() <= 10)
    {
        const unsigned int num = number.getBitRangeAsInt (0, 10);

        for (unsigned int i = num / 2; --i > 1;)
            if (num % i == 0)
                return false;

        return true;
    }
    else
    {
        if (number.findGreatestCommonDivisor (2 * 3 * 5 * 7 * 11 * 13 * 17 * 19 * 23) != 1)
            return false;

        return passesMillerRabin (number, certainty);
    }
}
