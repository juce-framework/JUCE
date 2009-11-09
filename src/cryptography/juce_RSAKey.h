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

#ifndef __JUCE_RSAKEY_JUCEHEADER__
#define __JUCE_RSAKEY_JUCEHEADER__

#include "../containers/juce_BitArray.h"


//==============================================================================
/**
    RSA public/private key-pair encryption class.

    An object of this type makes up one half of a public/private RSA key pair. Use the
    createKeyPair() method to create a matching pair for encoding/decoding.
*/
class JUCE_API  RSAKey
{
public:
    //==============================================================================
    /** Creates a null key object.

        Initialise a pair of objects for use with the createKeyPair() method.
    */
    RSAKey() throw();

    /** Loads a key from an encoded string representation.

        This reloads a key from a string created by the toString() method.
    */
    RSAKey (const String& stringRepresentation) throw();

    /** Destructor. */
    ~RSAKey() throw();

    //==============================================================================
    /** Turns the key into a string representation.

        This can be reloaded using the constructor that takes a string.
    */
    const String toString() const throw();

    //==============================================================================
    /** Encodes or decodes a value.

        Call this on the public key object to encode some data, then use the matching
        private key object to decode it.

        Returns false if the operation couldn't be completed, e.g. if this key hasn't been
        initialised correctly.

        NOTE: This method dumbly applies this key to this data. If you encode some data
        and then try to decode it with a key that doesn't match, this method will still
        happily do its job and return true, but the result won't be what you were expecting.
        It's your responsibility to check that the result is what you wanted.
    */
    bool applyToValue (BitArray& value) const throw();

    //==============================================================================
    /** Creates a public/private key-pair.

        Each key will perform one-way encryption that can only be reversed by
        using the other key.

        The numBits parameter specifies the size of key, e.g. 128, 256, 512 bit. Bigger
        sizes are more secure, but this method will take longer to execute.

        The randomSeeds parameter lets you optionally pass it a set of values with
        which to seed the random number generation, improving the security of the
        keys generated.
    */
    static void createKeyPair (RSAKey& publicKey,
                               RSAKey& privateKey,
                               const int numBits,
                               const int* randomSeeds = 0,
                               const int numRandomSeeds = 0) throw();


    //==============================================================================
    juce_UseDebuggingNewOperator

protected:
    BitArray part1, part2;
};


#endif   // __JUCE_RSAKEY_JUCEHEADER__
