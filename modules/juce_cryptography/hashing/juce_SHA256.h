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
    SHA-256 secure hash generator.

    Create one of these objects from a block of source data or a stream, and it
    calculates the SHA-256 hash of that data.

    You can retrieve the hash as a raw 32-byte block, or as a 64-digit hex string.
    @see MD5

    @tags{Cryptography}
*/
class JUCE_API  SHA256
{
public:
    //==============================================================================
    /** Creates an empty SHA256 object.
        The default constructor just creates a hash filled with zeros. (This is not
        equal to the hash of an empty block of data).
    */
    SHA256();

    /** Destructor. */
    ~SHA256();

    /** Creates a copy of another SHA256. */
    SHA256 (const SHA256&);

    /** Copies another SHA256. */
    SHA256& operator= (const SHA256&);

    //==============================================================================
    /** Creates a hash from a block of raw data. */
    explicit SHA256 (const MemoryBlock& data);

    /** Creates a hash from a block of raw data. */
    SHA256 (const void* data, size_t numBytes);

    /** Creates a hash from the contents of a stream.

        This will read from the stream until the stream is exhausted, or until
        maxBytesToRead bytes have been read. If maxBytesToRead is negative, the entire
        stream will be read.
    */
    SHA256 (InputStream& input, int64 maxBytesToRead = -1);

    /** Reads a file and generates the hash of its contents.
        If the file can't be opened, the hash will be left uninitialised (i.e. full
        of zeros).
    */
    explicit SHA256 (const File& file);

    /** Creates a checksum from a UTF-8 buffer.
        E.g.
        @code SHA256 checksum (myString.toUTF8());
        @endcode
    */
    explicit SHA256 (CharPointer_UTF8 utf8Text) noexcept;

    //==============================================================================
    /** Returns the hash as a 32-byte block of data. */
    MemoryBlock getRawData() const;

    /** Returns the checksum as a 64-digit hex string. */
    String toHexString() const;

    //==============================================================================
    bool operator== (const SHA256&) const noexcept;
    bool operator!= (const SHA256&) const noexcept;


private:
    //==============================================================================
    uint8 result[32] = {};
    void process (const void*, size_t);

    JUCE_LEAK_DETECTOR (SHA256)
};

} // namespace juce
