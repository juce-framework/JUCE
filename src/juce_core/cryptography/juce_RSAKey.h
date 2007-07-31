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

        Returns false if the operation failed, e.g. if this object isn't a valid key.
    */
    bool applyToValue (BitArray& value) const throw();

    //==============================================================================
    /** Creates a public/private key-pair.

        Each key will perform one-way encryption that can only be reversed by
        using the other key.

        The numBits parameter specifies the size of key, e.g. 128, 256, 512 bit. Bigger
        sizes are more secure, but this method will take longer to execute.
    */
    static void createKeyPair (RSAKey& publicKey,
                               RSAKey& privateKey,
                               const int numBits) throw();


    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    BitArray part1, part2;
};


#endif   // __JUCE_RSAKEY_JUCEHEADER__
