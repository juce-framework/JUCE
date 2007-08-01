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

#include "juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_Random.h"

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

//==============================================================================
static Random sysRand (1);

Random& Random::getSystemRandom() throw()
{
    return sysRand;
}

END_JUCE_NAMESPACE
