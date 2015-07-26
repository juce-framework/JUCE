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

#ifndef JUCE_PRIMES_H_INCLUDED
#define JUCE_PRIMES_H_INCLUDED


//==============================================================================
/**
    Prime number creation class.

    This class contains static methods for generating and testing prime numbers.

    @see BigInteger
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
                                           const int* randomSeeds = 0,
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


#endif   // JUCE_PRIMES_H_INCLUDED
