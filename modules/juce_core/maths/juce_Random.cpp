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

Random::Random (const int64 seedValue) noexcept
    : seed (seedValue)
{
}

Random::Random()
    : seed (1)
{
    setSeedRandomly();
}

Random::~Random() noexcept
{
}

void Random::setSeed (const int64 newSeed) noexcept
{
    seed = newSeed;
}

void Random::combineSeed (const int64 seedValue) noexcept
{
    seed ^= nextInt64() ^ seedValue;
}

void Random::setSeedRandomly()
{
    static int64 globalSeed = 0;

    combineSeed (globalSeed ^ (int64) (pointer_sized_int) this);
    combineSeed (Time::getMillisecondCounter());
    combineSeed (Time::getHighResolutionTicks());
    combineSeed (Time::getHighResolutionTicksPerSecond());
    combineSeed (Time::currentTimeMillis());
    globalSeed ^= seed;
}

Random& Random::getSystemRandom() noexcept
{
    static Random sysRand;
    return sysRand;
}

//==============================================================================
int Random::nextInt() noexcept
{
    seed = (seed * literal64bit (0x5deece66d) + 11) & literal64bit (0xffffffffffff);

    return (int) (seed >> 16);
}

int Random::nextInt (const int maxValue) noexcept
{
    jassert (maxValue > 0);
    return (int) ((((unsigned int) nextInt()) * (uint64) maxValue) >> 32);
}

int64 Random::nextInt64() noexcept
{
    return (((int64) nextInt()) << 32) | (int64) (uint64) (uint32) nextInt();
}

bool Random::nextBool() noexcept
{
    return (nextInt() & 0x40000000) != 0;
}

float Random::nextFloat() noexcept
{
    return static_cast <uint32> (nextInt()) / (float) 0xffffffff;
}

double Random::nextDouble() noexcept
{
    return static_cast <uint32> (nextInt()) / (double) 0xffffffff;
}

BigInteger Random::nextLargeNumber (const BigInteger& maximumValue)
{
    BigInteger n;

    do
    {
        fillBitsRandomly (n, 0, maximumValue.getHighestBit() + 1);
    }
    while (n >= maximumValue);

    return n;
}

void Random::fillBitsRandomly (BigInteger& arrayToChange, int startBit, int numBits)
{
    arrayToChange.setBit (startBit + numBits - 1, true);  // to force the array to pre-allocate space

    while ((startBit & 31) != 0 && numBits > 0)
    {
        arrayToChange.setBit (startBit++, nextBool());
        --numBits;
    }

    while (numBits >= 32)
    {
        arrayToChange.setBitRangeAsInt (startBit, 32, (unsigned int) nextInt());
        startBit += 32;
        numBits -= 32;
    }

    while (--numBits >= 0)
        arrayToChange.setBit (startBit + numBits, nextBool());
}

//==============================================================================
#if JUCE_UNIT_TESTS

class RandomTests  : public UnitTest
{
public:
    RandomTests() : UnitTest ("Random") {}

    void runTest()
    {
        beginTest ("Random");

        for (int j = 10; --j >= 0;)
        {
            Random r;
            r.setSeedRandomly();

            for (int i = 20; --i >= 0;)
            {
                expect (r.nextDouble() >= 0.0 && r.nextDouble() < 1.0);
                expect (r.nextFloat() >= 0.0f && r.nextFloat() < 1.0f);
                expect (r.nextInt (5) >= 0 && r.nextInt (5) < 5);
                expect (r.nextInt (1) == 0);

                int n = r.nextInt (50) + 1;
                expect (r.nextInt (n) >= 0 && r.nextInt (n) < n);

                n = r.nextInt (0x7ffffffe) + 1;
                expect (r.nextInt (n) >= 0 && r.nextInt (n) < n);
            }
        }
    }
};

static RandomTests randomTests;

#endif
