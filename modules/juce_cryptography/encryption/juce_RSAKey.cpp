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
    BigInteger q (Primes::createProbablePrime (numBits - numBits / 2, 30, randomSeeds == nullptr ? nullptr : (randomSeeds + numRandomSeeds / 2), numRandomSeeds - numRandomSeeds / 2));

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

} // namespace juce
