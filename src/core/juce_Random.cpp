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

#include "juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_Random.h"
#include "juce_Time.h"


//==============================================================================
Random::Random (const int64 seedValue) throw()
    : seed (seedValue)
{
}

Random::~Random() throw()
{
}

void Random::setSeed (const int64 newSeed) throw()
{
    seed = newSeed;
}

void Random::setSeedRandomly()
{
    seed ^= (int64) (pointer_sized_int) this;
    seed ^= nextInt64() ^ Time::getMillisecondCounter();
    seed ^= nextInt64() ^ Time::getHighResolutionTicks();
    seed ^= nextInt64() ^ Time::getHighResolutionTicksPerSecond();
    seed ^= nextInt64() ^ Time::currentTimeMillis();
}

//==============================================================================
int Random::nextInt() throw()
{
    seed = (seed * literal64bit (0x5deece66d) + 11) & literal64bit (0xffffffffffff);

    return (int) (seed >> 16);
}

int Random::nextInt (const int maxValue) throw()
{
    jassert (maxValue > 0);
    return (nextInt() & 0x7fffffff) % maxValue;
}

int64 Random::nextInt64() throw()
{
    return (((int64) nextInt()) << 32) | (int64) (uint64) (uint32) nextInt();
}

bool Random::nextBool() throw()
{
    return (nextInt() & 0x80000000) != 0;
}

float Random::nextFloat() throw()
{
    return ((uint32) nextInt()) / (float) 0xffffffff;
}

double Random::nextDouble() throw()
{
    return ((uint32) nextInt()) / (double) 0xffffffff;
}

const BitArray Random::nextLargeNumber (const BitArray& maximumValue) throw()
{
    BitArray n;

    do
    {
        fillBitsRandomly (n, 0, maximumValue.getHighestBit() + 1);
    }
    while (n.compare (maximumValue) >= 0);

    return n;
}

void Random::fillBitsRandomly (BitArray& arrayToChange, int startBit, int numBits) throw()
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
Random& Random::getSystemRandom() throw()
{
    static Random sysRand (1);
    return sysRand;
}

END_JUCE_NAMESPACE
