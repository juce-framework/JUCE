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
    MD5 checksum class.

    Create one of these with a block of source data or a stream, and it calculates
    the MD5 checksum of that data.

    You can then retrieve this checksum as a 16-byte block, or as a hex string.
    @see SHA256

    @tags{Cryptography}
*/
class JUCE_API  MD5
{
public:
    //==============================================================================
    /** Creates a null MD5 object. */
    MD5();

    /** Creates a copy of another MD5. */
    MD5 (const MD5&);

    /** Copies another MD5. */
    MD5& operator= (const MD5&);

    //==============================================================================
    /** Creates a checksum for a block of binary data. */
    explicit MD5 (const MemoryBlock&) noexcept;

    /** Creates a checksum for a block of binary data. */
    MD5 (const void* data, size_t numBytes) noexcept;

    /** Creates a checksum for the input from a stream.

        This will read up to the given number of bytes from the stream, and produce the
        checksum of that. If the number of bytes to read is negative, it'll read
        until the stream is exhausted.
    */
    MD5 (InputStream& input, int64 numBytesToRead = -1);

    /** Creates a checksum for the contents of a file. */
    explicit MD5 (const File&);

    /** Creates a checksum of the characters in a UTF-8 buffer.
        E.g.
        @code MD5 checksum (myString.toUTF8());
        @endcode
    */
    explicit MD5 (CharPointer_UTF8 utf8Text) noexcept;

    /** Destructor. */
    ~MD5();

    //==============================================================================
    /** Returns the checksum as a 16-byte block of data. */
    MemoryBlock getRawChecksumData() const;

    /** Returns a pointer to the 16-byte array of result data. */
    const uint8* getChecksumDataArray() const noexcept          { return result; }

    /** Returns the checksum as a 32-digit hex string. */
    String toHexString() const;

    /** Creates an MD5 from a little-endian UTF-32 encoded string.

        Note that this method is provided for backwards-compatibility with the old
        version of this class, which had a constructor that took a string and performed
        this operation on it. In new code, you shouldn't use this, and are recommended to
        use the constructor that takes a CharPointer_UTF8 instead.
    */
    static MD5 fromUTF32 (StringRef);

    //==============================================================================
    bool operator== (const MD5&) const noexcept;
    bool operator!= (const MD5&) const noexcept;


private:
    //==============================================================================
    uint8 result[16] = {};

    void processStream (InputStream&, int64);

    // This private constructor is declared here to prevent you accidentally passing a
    // String and having it unexpectedly call the constructor that takes a File.
    explicit MD5 (const String&) = delete;

    JUCE_LEAK_DETECTOR (MD5)
};

} // namespace juce
