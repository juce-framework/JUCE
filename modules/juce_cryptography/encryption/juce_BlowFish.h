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

//==============================================================================
/**
    BlowFish encryption class.


    @tags{Cryptography}
*/
class JUCE_API  BlowFish
{
public:
    //==============================================================================
    /** Creates an object that can encode/decode based on the specified key.

        The key data can be up to 72 bytes long.
    */
    BlowFish (const void* keyData, int keyBytes);

    /** Creates a copy of another blowfish object. */
    BlowFish (const BlowFish&);

    /** Copies another blowfish object. */
    BlowFish& operator= (const BlowFish&) noexcept;

    /** Destructor. */
    ~BlowFish() noexcept;

    //==============================================================================
    /** Encrypts a pair of 32-bit integers. */
    void encrypt (uint32& data1, uint32& data2) const noexcept;

    /** Decrypts a pair of 32-bit integers. */
    void decrypt (uint32& data1, uint32& data2) const noexcept;

    //==============================================================================
    /** Encrypts a memory block */
    void encrypt (MemoryBlock& data) const;

    /** Decrypts a memory block */
    void decrypt (MemoryBlock& data) const;

    //==============================================================================
    /** Encrypts data in-place

        @param buffer       The message that should be encrypted. See bufferSize on size
                            requirements!
        @param sizeOfMsg    The size of the message that should be encrypted in bytes
        @param bufferSize   The size of the buffer in bytes. To accommodate the encrypted
                            data, the buffer must be larger than the message: the size of
                            the buffer needs to be equal or greater than the size of the
                            message in bytes rounded to the next integer which is divisible
                            by eight. If the message size in bytes is already divisible by eight
                            then you need to add eight bytes to the buffer size. If in doubt
                            simply use bufferSize = sizeOfMsg + 8.

        @returns            The size of the decrypted data in bytes or -1 if the decryption failed.
     */
    int encrypt (void* buffer, size_t sizeOfMsg, size_t bufferSize) const noexcept;

    /** Decrypts data in-place

        @param buffer  The encrypted data that should be decrypted
        @param bytes   The size of the encrypted data in bytes

        @returns       The size of the decrypted data in bytes or -1 if the decryption failed.
    */
    int decrypt (void* buffer, size_t bytes) const noexcept;

private:
    //==============================================================================
    static int pad   (void*, size_t, size_t) noexcept;
    static int unpad (const void*, size_t) noexcept;

    bool apply (void*, size_t, void (BlowFish::*op) (uint32&, uint32&) const) const;

    //==============================================================================
    uint32 p[18];
    HeapBlock<uint32> s[4];

    uint32 F (uint32) const noexcept;

    JUCE_LEAK_DETECTOR (BlowFish)
};

} // namespace juce
