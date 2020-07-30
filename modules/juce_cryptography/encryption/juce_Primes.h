/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

//==============================================================================
/**
    Prime number creation class.

    This class contains static methods for generating and testing prime numbers.

    @see BigInteger

    @tags{Cryptography}
*/
class JUCE_API  Primes
{
public:
    //==============================================================================
    /** Creates a random prime number with a given bit-length.

        The certainty parameter specifies how many iterations to use when testing
        for primality. A safe value might be anything over about 20-30.

        The randomSeeds parameter lets you optionally pass it a set of values with
        which to seed the random number generation, improving the security of the
        keys generated.
    */
    static BigInteger createProbablePrime (int bitLength,
                                           int certainty,
                                        const int* randomSeeds = nullptr,
                                           int numRandomSeeds = 0);

    /** Tests a number to see if it's prime.

        This isn't a bulletproof test, it uses a Miller-Rabin test to determine
        whether the number is prime.

        The certainty parameter specifies how many iterations to use when testing - a
        safe value might be anything over about 20-30.
    */
    static bool isProbablyPrime (const BigInteger& number, int certainty);


private:
    Primes();

    JUCE_DECLARE_NON_COPYABLE (Primes)
};

} // namespace juce
