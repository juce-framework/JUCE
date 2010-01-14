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

#ifndef __JUCE_MD5_JUCEHEADER__
#define __JUCE_MD5_JUCEHEADER__

#include "../containers/juce_MemoryBlock.h"
#include "../io/streams/juce_InputStream.h"
#include "../io/files/juce_File.h"


//==============================================================================
/**
    MD5 checksum class.

    Create one of these with a block of source data or a string, and it calculates the
    MD5 checksum of that data.

    You can then retrieve this checksum as a 16-byte block, or as a hex string.
*/
class JUCE_API  MD5
{
public:
    //==============================================================================
    /** Creates a null MD5 object. */
    MD5();

    /** Creates a copy of another MD5. */
    MD5 (const MD5& other);

    /** Copies another MD5. */
    const MD5& operator= (const MD5& other);

    //==============================================================================
    /** Creates a checksum for a block of binary data. */
    MD5 (const MemoryBlock& data);

    /** Creates a checksum for a block of binary data. */
    MD5 (const char* data, const size_t numBytes);

    /** Creates a checksum for a string.

        Note that this operates on the string as a block of unicode characters, so the
        result you get will differ from the value you'd get if the string was treated
        as a block of utf8 or ascii. Bear this in mind if you're comparing the result
        of this method with a checksum created by a different framework, which may have
        used a different encoding.
    */
    MD5 (const String& text);

    /** Creates a checksum for the input from a stream.

        This will read up to the given number of bytes from the stream, and produce the
        checksum of that. If the number of bytes to read is negative, it'll read
        until the stream is exhausted.
    */
    MD5 (InputStream& input, int64 numBytesToRead = -1);

    /** Creates a checksum for a file. */
    MD5 (const File& file);

    /** Destructor. */
    ~MD5();

    //==============================================================================
    /** Returns the checksum as a 16-byte block of data. */
    const MemoryBlock getRawChecksumData() const;

    /** Returns the checksum as a 32-digit hex string. */
    const String toHexString() const;


    //==============================================================================
    /** Compares this to another MD5. */
    bool operator== (const MD5& other) const;

    /** Compares this to another MD5. */
    bool operator!= (const MD5& other) const;


    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    uint8 result [16];

    struct ProcessContext
    {
        uint8 buffer [64];
        uint32 state [4];
        uint32 count [2];

        ProcessContext();

        void processBlock (const uint8* const data, size_t dataSize);
        void transform (const uint8* const buffer);
        void finish (uint8* const result);
    };

    void processStream (InputStream& input, int64 numBytesToRead);
};


#endif   // __JUCE_MD5_JUCEHEADER__
