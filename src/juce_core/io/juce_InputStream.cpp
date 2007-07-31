/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#include "../basics/juce_StandardHeader.h"

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
        return (short) littleEndianShort (temp);
    else
        return 0;
}

short InputStream::readShortBigEndian()
{
    char temp [2];

    if (read (temp, 2) == 2)
        return (short) bigEndianShort (temp);
    else
        return 0;
}

int InputStream::readInt()
{
    char temp [4];

    if (read (temp, 4) == 4)
        return (int) littleEndianInt (temp);
    else
        return 0;
}

int InputStream::readIntBigEndian()
{
    char temp [4];

    if (read (temp, 4) == 4)
        return (int) bigEndianInt (temp);
    else
        return 0;
}

int InputStream::readCompressedInt()
{
    int num = 0;

    if (! isExhausted())
    {
        unsigned char numBytes = readByte();
        const bool negative = (numBytes & 0x80) != 0;
        numBytes &= 0x7f;

        if (numBytes <= 4)
        {
            if (read (&num, numBytes) != numBytes)
                return 0;

            if (negative)
                num = -num;
        }
    }

    return num;
}

int64 InputStream::readInt64()
{
    const int temp1 = readInt();
    int64 temp = readInt();
    temp <<= 32;
    temp |= temp1 & (int64) 0xffffffff;

    return temp;
}

int64 InputStream::readInt64BigEndian()
{
    int64 temp = readIntBigEndian();
    const int temp2 = readIntBigEndian();
    temp <<= 32;
    temp |= temp2 & (int64) 0xffffffff;

    return temp;
}

float InputStream::readFloat()
{
    union { int asInt; float asFloat; } n;
    n.asInt = readInt();
    return n.asFloat;
}

double InputStream::readDouble()
{
    union { int64 asInt; double asDouble; } n;
    n.asInt = readInt64();
    return n.asDouble;
}

const String InputStream::readString()
{
    const int tempBufferSize = 256;
    uint8 temp [tempBufferSize];
    int i = 0;

    while ((temp [i++] = readByte()) != 0)
    {
        if (i == tempBufferSize)
        {
            // too big for our quick buffer, so read it in blocks..
            String result (String::fromUTF8 (temp, i));
            i = 0;

            for (;;)
            {
                if ((temp [i++] = readByte()) == 0)
                {
                    result += String::fromUTF8 (temp, i - 1);
                    break;
                }
                else if (i == tempBufferSize)
                {
                    result += String::fromUTF8 (temp, i);
                    i = 0;
                }
            }

            return result;
        }
    }

    return String::fromUTF8 (temp, i - 1);
}

const String InputStream::readNextLine()
{
    String s;
    const int maxChars = 256;
    tchar buffer [maxChars];
    int charsInBuffer = 0;

    while (! isExhausted())
    {
        const uint8 c = readByte();
        const int64 lastPos = getPosition();

        if (c == '\n')
        {
            break;
        }
        else if (c == '\r')
        {
            if (readByte() != '\n')
                setPosition (lastPos);

            break;
        }

        buffer [charsInBuffer++] = c;

        if (charsInBuffer == maxChars)
        {
            s.append (buffer, maxChars);
            charsInBuffer = 0;
        }
    }

    if (charsInBuffer > 0)
        s.append (buffer, charsInBuffer);

    return s;
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

    const int originalBlockSize = block.getSize();
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
        MemoryBlock temp (skipBufferSize);

        while ((numBytesToSkip > 0) && ! isExhausted())
        {
            numBytesToSkip -= read (temp.getData(), (int) jmin (numBytesToSkip, (int64) skipBufferSize));
        }
    }
}

END_JUCE_NAMESPACE
