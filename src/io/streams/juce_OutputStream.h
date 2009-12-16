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

#ifndef __JUCE_OUTPUTSTREAM_JUCEHEADER__
#define __JUCE_OUTPUTSTREAM_JUCEHEADER__

#include "../../text/juce_String.h"
#include "juce_InputStream.h"


//==============================================================================
/**
    The base class for streams that write data to some kind of destination.

    Input and output streams are used throughout the library - subclasses can override
    some or all of the virtual functions to implement their behaviour.

    @see InputStream, MemoryOutputStream, FileOutputStream
*/
class JUCE_API  OutputStream
{
public:
    /** Destructor.

        Some subclasses might want to do things like call flush() during their
        destructors.
    */
    virtual ~OutputStream();

    //==============================================================================
    /** If the stream is using a buffer, this will ensure it gets written
        out to the destination. */
    virtual void flush() = 0;

    /** Tries to move the stream's output position.

        Not all streams will be able to seek to a new position - this will return
        false if it fails to work.

        @see getPosition
    */
    virtual bool setPosition (int64 newPosition) = 0;

    /** Returns the stream's current position.

        @see setPosition
    */
    virtual int64 getPosition() = 0;

    //==============================================================================
    /** Writes a block of data to the stream.

        When creating a subclass of OutputStream, this is the only write method
        that needs to be overloaded - the base class has methods for writing other
        types of data which use this to do the work.

        @returns false if the write operation fails for some reason
    */
    virtual bool write (const void* dataToWrite,
                        int howManyBytes) = 0;

    //==============================================================================
    /** Writes a single byte to the stream.

        @see InputStream::readByte
    */
    virtual void writeByte (char byte);

    /** Writes a boolean to the stream.

        This is encoded as a byte - either 1 or 0.

        @see InputStream::readBool
    */
    virtual void writeBool (bool boolValue);

    /** Writes a 16-bit integer to the stream in a little-endian byte order.

        This will write two bytes to the stream: (value & 0xff), then (value >> 8).

        @see InputStream::readShort
    */
    virtual void writeShort (short value);

    /** Writes a 16-bit integer to the stream in a big-endian byte order.

        This will write two bytes to the stream: (value >> 8), then (value & 0xff).

        @see InputStream::readShortBigEndian
    */
    virtual void writeShortBigEndian (short value);

    /** Writes a 32-bit integer to the stream in a little-endian byte order.

        @see InputStream::readInt
    */
    virtual void writeInt (int value);

    /** Writes a 32-bit integer to the stream in a big-endian byte order.

        @see InputStream::readIntBigEndian
    */
    virtual void writeIntBigEndian (int value);

    /** Writes a 64-bit integer to the stream in a little-endian byte order.

        @see InputStream::readInt64
    */
    virtual void writeInt64 (int64 value);

    /** Writes a 64-bit integer to the stream in a big-endian byte order.

        @see InputStream::readInt64BigEndian
    */
    virtual void writeInt64BigEndian (int64 value);

    /** Writes a 32-bit floating point value to the stream.

        The binary 32-bit encoding of the float is written as a little-endian int.

        @see InputStream::readFloat
    */
    virtual void writeFloat (float value);

    /** Writes a 32-bit floating point value to the stream.

        The binary 32-bit encoding of the float is written as a big-endian int.

        @see InputStream::readFloatBigEndian
    */
    virtual void writeFloatBigEndian (float value);

    /** Writes a 64-bit floating point value to the stream.

        The eight raw bytes of the double value are written out as a little-endian 64-bit int.

        @see InputStream::readDouble
    */
    virtual void writeDouble (double value);

    /** Writes a 64-bit floating point value to the stream.

        The eight raw bytes of the double value are written out as a big-endian 64-bit int.

        @see InputStream::readDoubleBigEndian
    */
    virtual void writeDoubleBigEndian (double value);

    /** Writes a condensed encoding of a 32-bit integer.

        If you're storing a lot of integers which are unlikely to have very large values,
        this can save a lot of space, because values under 0xff will only take up 2 bytes,
        under 0xffff only 3 bytes, etc.

        The format used is: number of significant bytes + up to 4 bytes in little-endian order.

        @see InputStream::readCompressedInt
    */
    virtual void writeCompressedInt (int value);

    /** Stores a string in the stream.

        This isn't the method to use if you're trying to append text to the end of a
        text-file! It's intended for storing a string for later retrieval
        by InputStream::readString.

        It writes the string to the stream as UTF8, with a null character terminating it.

        For appending text to a file, instead use writeText, printf, or operator<<

        @see InputStream::readString, writeText, printf, operator<<
    */
    virtual void writeString (const String& text);

    /** Writes a string of text to the stream.

        It can either write it as 8-bit system-encoded characters, or as unicode, and
        can also add unicode header bytes (0xff, 0xfe) to indicate the endianness (this
        should only be done at the start of a file).

        The method also replaces '\\n' characters in the text with '\\r\\n'.
    */
    virtual void writeText (const String& text,
                            const bool asUnicode,
                            const bool writeUnicodeHeaderBytes);

    /** Writes a string of text to the stream.

        @see writeText
    */
    virtual void printf (const char* format, ...);

    /** Reads data from an input stream and writes it to this stream.

        @param source               the stream to read from
        @param maxNumBytesToWrite   the number of bytes to read from the stream (if this is
                                    less than zero, it will keep reading until the input
                                    is exhausted)
    */
    virtual int writeFromInputStream (InputStream& source,
                                      int maxNumBytesToWrite);

    //==============================================================================
    /** Writes a number to the stream as 8-bit characters in the default system encoding. */
    virtual OutputStream& operator<< (const int number);

    /** Writes a number to the stream as 8-bit characters in the default system encoding. */
    virtual OutputStream& operator<< (const double number);

    /** Writes a character to the stream. */
    virtual OutputStream& operator<< (const char character);

    /** Writes a null-terminated string to the stream. */
    virtual OutputStream& operator<< (const char* const text);

    /** Writes a null-terminated unicode text string to the stream, converting it
        to 8-bit characters in the default system encoding. */
    virtual OutputStream& operator<< (const juce_wchar* const text);

    /** Writes a string to the stream as 8-bit characters in the default system encoding. */
    virtual OutputStream& operator<< (const String& text);


    //==============================================================================
    juce_UseDebuggingNewOperator


protected:
    //==============================================================================
    OutputStream() throw();
};

#endif   // __JUCE_OUTPUTSTREAM_JUCEHEADER__
