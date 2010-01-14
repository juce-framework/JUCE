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

#include "../../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE


#include "juce_InputStream.h"


//==============================================================================
char InputStream::readByte()
{
    char temp = 0;
    read (&temp, 1);
    return temp;
}

bool InputStream::readBool()
{
    return readByte() != 0;
}

short InputStream::readShort()
{
    char temp [2];

    if (read (temp, 2) == 2)
        return (short) ByteOrder::littleEndianShort (temp);
    else
        return 0;
}

short InputStream::readShortBigEndian()
{
    char temp [2];

    if (read (temp, 2) == 2)
        return (short) ByteOrder::bigEndianShort (temp);
    else
        return 0;
}

int InputStream::readInt()
{
    char temp [4];

    if (read (temp, 4) == 4)
        return (int) ByteOrder::littleEndianInt (temp);
    else
        return 0;
}

int InputStream::readIntBigEndian()
{
    char temp [4];

    if (read (temp, 4) == 4)
        return (int) ByteOrder::bigEndianInt (temp);
    else
        return 0;
}

int InputStream::readCompressedInt()
{
    const unsigned char sizeByte = readByte();
    if (sizeByte == 0)
        return 0;

    const int numBytes = (sizeByte & 0x7f);
    if (numBytes > 4)
    {
        jassertfalse    // trying to read corrupt data - this method must only be used
                        // to read data that was written by OutputStream::writeCompressedInt()
        return 0;
    }

    char bytes[4] = { 0, 0, 0, 0 };
    if (read (bytes, numBytes) != numBytes)
        return 0;

    const int num = (int) ByteOrder::littleEndianInt (bytes);
    return (sizeByte >> 7) ? -num : num;
}

int64 InputStream::readInt64()
{
    char temp [8];

    if (read (temp, 8) == 8)
        return (int64) ByteOrder::swapIfBigEndian (*(uint64*) temp);
    else
        return 0;
}

int64 InputStream::readInt64BigEndian()
{
    char temp [8];

    if (read (temp, 8) == 8)
        return (int64) ByteOrder::swapIfLittleEndian (*(uint64*) temp);
    else
        return 0;
}

float InputStream::readFloat()
{
    union { int asInt; float asFloat; } n;
    n.asInt = readInt();
    return n.asFloat;
}

float InputStream::readFloatBigEndian()
{
    union { int asInt; float asFloat; } n;
    n.asInt = readIntBigEndian();
    return n.asFloat;
}

double InputStream::readDouble()
{
    union { int64 asInt; double asDouble; } n;
    n.asInt = readInt64();
    return n.asDouble;
}

double InputStream::readDoubleBigEndian()
{
    union { int64 asInt; double asDouble; } n;
    n.asInt = readInt64BigEndian();
    return n.asDouble;
}

const String InputStream::readString()
{
    MemoryBlock buffer (256);
    uint8* data = (uint8*) buffer.getData();
    size_t i = 0;

    while ((data[i] = readByte()) != 0)
    {
        if (++i >= buffer.getSize())
        {
            buffer.setSize (buffer.getSize() + 512);
            data = (uint8*) buffer.getData();
        }
    }

    return String::fromUTF8 (data, (int) i);
}

const String InputStream::readNextLine()
{
    MemoryBlock buffer (256);
    uint8* data = (uint8*) buffer.getData();
    size_t i = 0;

    while ((data[i] = readByte()) != 0)
    {
        if (data[i] == '\n')
            break;

        if (data[i] == '\r')
        {
            const int64 lastPos = getPosition();

            if (readByte() != '\n')
                setPosition (lastPos);

            break;
        }

        if (++i >= buffer.getSize())
        {
            buffer.setSize (buffer.getSize() + 512);
            data = (uint8*) buffer.getData();
        }
    }

    return String::fromUTF8 (data, (int) i);
}

int InputStream::readIntoMemoryBlock (MemoryBlock& block,
                                      int numBytes)
{
    const int64 totalLength = getTotalLength();

    if (totalLength >= 0)
    {
        const int totalBytesRemaining = (int) jmin ((int64) 0x7fffffff,
                                                    totalLength - getPosition());

        if (numBytes < 0)
            numBytes = totalBytesRemaining;
        else if (numBytes > 0)
            numBytes = jmin (numBytes, totalBytesRemaining);
        else
            return 0;
    }

    const size_t originalBlockSize = block.getSize();
    int totalBytesRead = 0;

    if (numBytes > 0)
    {
        // know how many bytes we want, so we can resize the block first..
        block.setSize (originalBlockSize + numBytes, false);
        totalBytesRead = read (((char*) block.getData()) + originalBlockSize, numBytes);
    }
    else
    {
        // read until end of stram..
        const int chunkSize = 32768;

        for (;;)
        {
            block.ensureSize (originalBlockSize + totalBytesRead + chunkSize, false);

            const int bytesJustIn = read (((char*) block.getData())
                                            + originalBlockSize
                                            + totalBytesRead,
                                          chunkSize);

            if (bytesJustIn == 0)
                break;

            totalBytesRead += bytesJustIn;
        }
    }

    // trim off any excess left at the end
    block.setSize (originalBlockSize +  totalBytesRead, false);
    return totalBytesRead;
}

const String InputStream::readEntireStreamAsString()
{
    MemoryBlock mb;
    const int size = readIntoMemoryBlock (mb);

    return String::createStringFromData ((const char*) mb.getData(), size);
}

//==============================================================================
void InputStream::skipNextBytes (int64 numBytesToSkip)
{
    if (numBytesToSkip > 0)
    {
        const int skipBufferSize = (int) jmin (numBytesToSkip, (int64) 16384);
        HeapBlock <char> temp (skipBufferSize);

        while (numBytesToSkip > 0 && ! isExhausted())
            numBytesToSkip -= read (temp, (int) jmin (numBytesToSkip, (int64) skipBufferSize));
    }
}

END_JUCE_NAMESPACE
