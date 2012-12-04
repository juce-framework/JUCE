/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

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

#ifndef __JUCE_MD5_JUCEHEADER__
#define __JUCE_MD5_JUCEHEADER__

//==============================================================================
/**
    MD5 checksum class.

    Create one of these with a block of source data or a stream, and it calculates
    the MD5 checksum of that data.

    You can then retrieve this checksum as a 16-byte block, or as a hex string.
    @see SHA256
*/
class JUCE_API  MD5
{
public:
    //==============================================================================
    /** Creates a null MD5 object. */
    MD5() noexcept;

    /** Creates a copy of another MD5. */
    MD5 (const MD5& other) noexcept;

    /** Copies another MD5. */
    MD5& operator= (const MD5& other) noexcept;

    //==============================================================================
    /** Creates a checksum for a block of binary data. */
    explicit MD5 (const MemoryBlock& data) noexcept;

    /** Creates a checksum for a block of binary data. */
    MD5 (const void* data, size_t numBytes) noexcept;

    /** Creates a checksum for the input from a stream.

        This will read up to the given number of bytes from the stream, and produce the
        checksum of that. If the number of bytes to read is negative, it'll read
        until the stream is exhausted.
    */
    MD5 (InputStream& input, int64 numBytesToRead = -1);

    /** Creates a checksum for a file. */
    explicit MD5 (const File& file);

    /** Creates a checksum from a UTF-8 buffer.
        E.g.
        @code MD5 checksum (myString.toUTF8());
        @endcode
    */
    explicit MD5 (const CharPointer_UTF8& utf8Text) noexcept;

    /** Destructor. */
    ~MD5() noexcept;

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
    static MD5 fromUTF32 (const String&);

    //==============================================================================
    bool operator== (const MD5&) const noexcept;
    bool operator!= (const MD5&) const noexcept;


private:
    //==============================================================================
    uint8 result [16];

    void processData (const void*, size_t) noexcept;
    void processStream (InputStream&, int64);

    JUCE_LEAK_DETECTOR (MD5)
};


#endif   // __JUCE_MD5_JUCEHEADER__
