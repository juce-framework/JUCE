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

#ifndef JUCE_RSAKEY_H_INCLUDED
#define JUCE_RSAKEY_H_INCLUDED


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
    RSAKey();

    /** Loads a key from an encoded string representation.

        This reloads a key from a string created by the toString() method.
    */
    explicit RSAKey (const String& stringRepresentation);

    /** Destructor. */
    ~RSAKey();

    bool operator== (const RSAKey& other) const noexcept;
    bool operator!= (const RSAKey& other) const noexcept;

    //==============================================================================
    /** Turns the key into a string representation.

        This can be reloaded using the constructor that takes a string.
    */
    String toString() const;

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
    bool applyToValue (BigInteger& value) const;

    //==============================================================================
    /** Creates a public/private key-pair.

        Each key will perform one-way encryption that can only be reversed by
        using the other key.

        The numBits parameter specifies the size of key, e.g. 128, 256, 512 bit. Bigger
        sizes are more secure, but this method will take longer to execute.

        The randomSeeds parameter lets you optionally pass it a set of values with
        which to seed the random number generation, improving the security of the
        keys generated. If you supply these, make sure you provide more than 2 values,
        and the more your provide, the better the security.
    */
    static void createKeyPair (RSAKey& publicKey,
                               RSAKey& privateKey,
                               int numBits,
                               const int* randomSeeds = nullptr,
                               int numRandomSeeds = 0);


protected:
    //==============================================================================
    BigInteger part1, part2;

private:
    //==============================================================================
    static BigInteger findBestCommonDivisor (const BigInteger& p, const BigInteger& q);

    JUCE_LEAK_DETECTOR (RSAKey)
};


#endif   // JUCE_RSAKEY_H_INCLUDED
