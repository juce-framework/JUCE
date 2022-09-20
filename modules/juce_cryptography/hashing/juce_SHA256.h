/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
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
