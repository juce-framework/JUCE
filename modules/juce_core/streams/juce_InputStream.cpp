/*
  ==============================================================================

   This file is part of the juce_core module of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

   Permission to use, copy, modify, and/or distribute this software for any purpose with
   or without fee is hereby granted, provided that the above copyright notice and this
   permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD
   TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN
   NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
   DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
   IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
   CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

   ------------------------------------------------------------------------------

   NOTE! This permissive ISC license applies ONLY to files within the juce_core module!
   All other JUCE modules are covered by a dual GPL/commercial license, so if you are
   using any other modules, be sure to check that you also comply with their license.

   For more details, visit www.juce.com

  ==============================================================================
*/

int64 InputStream::getNumBytesRemaining()
{
    int64 len = getTotalLength();

    if (len >= 0)
        len -= getPosition();

    return len;
}

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
    char temp[2];

    if (read (temp, 2) == 2)
        return (short) ByteOrder::littleEndianShort (temp);

    return 0;
}

short InputStream::readShortBigEndian()
{
    char temp[2];

    if (read (temp, 2) == 2)
        return (short) ByteOrder::bigEndianShort (temp);

    return 0;
}

int InputStream::readInt()
{
    char temp[4];

    if (read (temp, 4) == 4)
        return (int) ByteOrder::littleEndianInt (temp);

    return 0;
}

int InputStream::readIntBigEndian()
{
    char temp[4];

    if (read (temp, 4) == 4)
        return (int) ByteOrder::bigEndianInt (temp);

    return 0;
}

int InputStream::readCompressedInt()
{
    const uint8 sizeByte = (uint8) readByte();
    if (sizeByte == 0)
        return 0;

    const int numBytes = (sizeByte & 0x7f);
    if (numBytes > 4)
    {
        jassertfalse;    // trying to read corrupt data - this method must only be used
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
    union { uint8 asBytes[8]; uint64 asInt64; } n;

    if (read (n.asBytes, 8) == 8)
        return (int64) ByteOrder::swapIfBigEndian (n.asInt64);

    return 0;
}

int64 InputStream::readInt64BigEndian()
{
    union { uint8 asBytes[8]; uint64 asInt64; } n;

    if (read (n.asBytes, 8) == 8)
        return (int64) ByteOrder::swapIfLittleEndian (n.asInt64);

    return 0;
}

float InputStream::readFloat()
{
    // the union below relies on these types being the same size...
    static_jassert (sizeof (int32) == sizeof (float));
    union { int32 asInt; float asFloat; } n;
    n.asInt = (int32) readInt();
    return n.asFloat;
}

float InputStream::readFloatBigEndian()
{
    union { int32 asInt; float asFloat; } n;
    n.asInt = (int32) readIntBigEndian();
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

String InputStream::readString()
{
    MemoryBlock buffer (256);
    char* data = static_cast<char*> (buffer.getData());
    size_t i = 0;

    while ((data[i] = readByte()) != 0)
    {
        if (++i >= buffer.getSize())
        {
            buffer.setSize (buffer.getSize() + 512);
            data = static_cast<char*> (buffer.getData());
        }
    }

    return String::fromUTF8 (data, (int) i);
}

String InputStream::readNextLine()
{
    MemoryBlock buffer (256);
    char* data = static_cast<char*> (buffer.getData());
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
            data = static_cast<char*> (buffer.getData());
        }
    }

    return String::fromUTF8 (data, (int) i);
}

int InputStream::readIntoMemoryBlock (MemoryBlock& block, ssize_t numBytes)
{
    MemoryOutputStream mo (block, true);
    return mo.writeFromInputStream (*this, numBytes);
}

String InputStream::readEntireStreamAsString()
{
    MemoryOutputStream mo;
    mo << *this;
    return mo.toString();
}

//==============================================================================
void InputStream::skipNextBytes (int64 numBytesToSkip)
{
    if (numBytesToSkip > 0)
    {
        const int skipBufferSize = (int) jmin (numBytesToSkip, (int64) 16384);
        HeapBlock<char> temp ((size_t) skipBufferSize);

        while (numBytesToSkip > 0 && ! isExhausted())
            numBytesToSkip -= read (temp, (int) jmin (numBytesToSkip, (int64) skipBufferSize));
    }
}
