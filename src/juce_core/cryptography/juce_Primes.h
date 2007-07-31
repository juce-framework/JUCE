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

#ifndef __JUCE_PRIMES_JUCEHEADER__
#define __JUCE_PRIMES_JUCEHEADER__

#include "../containers/juce_BitArray.h"


//==============================================================================
/**
    Prime number creation class.

    This class contains static methods for generating and testing prime numbers.

    @see BitArray
*/
class JUCE_API  Primes
{
public:
    //==============================================================================
    /** Creates a random prime number with a given bit-length.

        The certainty parameter specifies how many iterations to use when testing
        for primality. A safe value might be anything over about 20-30.
    */
    static const BitArray createProbablePrime (int bitLength,
                                               int certainty) throw();

    /** Tests a number to see if it's prime.

        This isn't a bulletproof test, it uses a Miller-Rabin test to determine
        whether the number is prime.

        The certainty parameter specifies how many iterations to use when testing - a
        safe value might be anything over about 20-30.
    */
    static bool isProbablyPrime (const BitArray& number,
                                 int certainty) throw();
};


#endif   // __JUCE_PRIMES_JUCEHEADER__
