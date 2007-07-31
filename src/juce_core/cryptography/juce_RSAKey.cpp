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


#include "juce_RSAKey.h"
#include "juce_Primes.h"


//==============================================================================
RSAKey::RSAKey() throw()
{
}

RSAKey::RSAKey (const String& s) throw()
{
    if (s.containsChar (T(',')))
    {
        part1.parseString (s.upToFirstOccurrenceOf (T(","), false, false), 16);
        part2.parseString (s.fromFirstOccurrenceOf (T(","), false, false), 16);
    }
    else
    {
        // the string needs to be two hex numbers, comma-separated..
        jassertfalse;
    }
}

RSAKey::~RSAKey() throw()
{
}

const String RSAKey::toString() const throw()
{
    return part1.toString (16) + T(",") + part2.toString (16);
}

bool RSAKey::applyToValue (BitArray& value) const throw()
{
    if (part1.isEmpty() || part2.isEmpty()
         || value.compare (0) <= 0)
    {
        jassertfalse   // using an uninitialised key
        value.clear();
        return false;
    }

    BitArray result;

    while (! value.isEmpty())
    {
        result.multiplyBy (part2);

        BitArray remainder;
        value.divideBy (part2, remainder);

        remainder.exponentModulo (part1, part2);

        result.add (remainder);
    }

    value = result;

    return true;
}

static const BitArray findBestCommonDivisor (const BitArray& p,
                                             const BitArray& q) throw()
{
    const BitArray one (1);

    // try 3, 5, 9, 17, etc first because these only contain 2 bits and so
    // are fast to divide + multiply
    for (int i = 2; i <= 65536; i *= 2)
    {
        const BitArray e (1 + i);

        if (e.findGreatestCommonDivisor (p) == one
             && e.findGreatestCommonDivisor (q) == one)
        {
            return e;
        }
    }

    BitArray e (4);

    while (! (e.findGreatestCommonDivisor (p) == one
                && e.findGreatestCommonDivisor (q) == one))
    {
        e.add (one);
    }

    return e;
}

void RSAKey::createKeyPair (RSAKey& publicKey,
                            RSAKey& privateKey,
                            const int numBits) throw()
{
    jassert (numBits > 16); // not much point using less than this..

    BitArray p (Primes::createProbablePrime (numBits / 2, 30));
    BitArray q (Primes::createProbablePrime (numBits - numBits / 2, 30));

    BitArray n (p);
    n.multiplyBy (q);   // n = pq

    const BitArray one (1);
    p.subtract (one);
    q.subtract (one);

    BitArray m (p);
    m.multiplyBy (q);   // m = (p - 1)(q - 1)

    const BitArray e (findBestCommonDivisor (p, q));

    BitArray d (e);
    d.inverseModulo (m);

    publicKey.part1 = e;
    publicKey.part2 = n;

    privateKey.part1 = d;
    privateKey.part2 = n;
}


END_JUCE_NAMESPACE
