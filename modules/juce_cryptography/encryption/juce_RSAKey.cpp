/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

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

RSAKey::RSAKey()
{
}

RSAKey::RSAKey (const String& s)
{
    if (s.containsChar (','))
    {
        part1.parseString (s.upToFirstOccurrenceOf (",", false, false), 16);
        part2.parseString (s.fromFirstOccurrenceOf (",", false, false), 16);
    }
    else
    {
        // the string needs to be two hex numbers, comma-separated..
        jassertfalse;
    }
}

RSAKey::~RSAKey()
{
}

bool RSAKey::operator== (const RSAKey& other) const noexcept
{
    return part1 == other.part1 && part2 == other.part2;
}

bool RSAKey::operator!= (const RSAKey& other) const noexcept
{
    return ! operator== (other);
}

bool RSAKey::isValid() const noexcept
{
    return operator!= (RSAKey());
}

String RSAKey::toString() const
{
    return part1.toString (16) + "," + part2.toString (16);
}

bool RSAKey::applyToValue (BigInteger& value) const
{
    if (part1.isZero() || part2.isZero() || value <= 0)
    {
        jassertfalse;   // using an uninitialised key
        value.clear();
        return false;
    }

    BigInteger result;

    while (! value.isZero())
    {
        result *= part2;

        BigInteger remainder;
        value.divideBy (part2, remainder);

        remainder.exponentModulo (part1, part2);

        result += remainder;
    }

    value.swapWith (result);
    return true;
}

BigInteger RSAKey::findBestCommonDivisor (const BigInteger& p, const BigInteger& q)
{
    // try 3, 5, 9, 17, etc first because these only contain 2 bits and so
    // are fast to divide + multiply
    for (int i = 2; i <= 65536; i *= 2)
    {
        const BigInteger e (1 + i);

        if (e.findGreatestCommonDivisor (p).isOne() && e.findGreatestCommonDivisor (q).isOne())
            return e;
    }

    BigInteger e (4);

    while (! (e.findGreatestCommonDivisor (p).isOne() && e.findGreatestCommonDivisor (q).isOne()))
        ++e;

    return e;
}

void RSAKey::createKeyPair (RSAKey& publicKey, RSAKey& privateKey,
                            const int numBits, const int* randomSeeds, const int numRandomSeeds)
{
    jassert (numBits > 16); // not much point using less than this..
    jassert (numRandomSeeds == 0 || numRandomSeeds >= 2); // you need to provide plenty of seeds here!

    BigInteger p (Primes::createProbablePrime (numBits / 2, 30, randomSeeds, numRandomSeeds / 2));
    BigInteger q (Primes::createProbablePrime (numBits - numBits / 2, 30, randomSeeds == nullptr ? 0 : (randomSeeds + numRandomSeeds / 2), numRandomSeeds - numRandomSeeds / 2));

    const BigInteger n (p * q);
    const BigInteger m (--p * --q);
    const BigInteger e (findBestCommonDivisor (p, q));

    BigInteger d (e);
    d.inverseModulo (m);

    publicKey.part1 = e;
    publicKey.part2 = n;

    privateKey.part1 = d;
    privateKey.part2 = n;
}
