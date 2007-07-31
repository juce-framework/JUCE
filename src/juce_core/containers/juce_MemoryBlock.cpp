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

#include "juce_MemoryBlock.h"


//==============================================================================
MemoryBlock::MemoryBlock() throw()
    : data (0),
      size (0)
{
}

MemoryBlock::MemoryBlock (const int initialSize,
                          const bool initialiseToZero) throw()
{
    if (initialSize > 0)
    {
        size = initialSize;

        if (initialiseToZero)
            data = (char*) juce_calloc (initialSize);
        else
            data = (char*) juce_malloc (initialSize);
    }
    else
    {
        data = 0;
        size = 0;
    }
}

MemoryBlock::MemoryBlock (const MemoryBlock& other) throw()
    : data (0),
      size (other.size)
{
    if (size > 0)
    {
        jassert (other.data != 0);
        data = (char*) juce_malloc (size);
        memcpy (data, other.data, size);
    }
}

MemoryBlock::MemoryBlock (const void* const dataToInitialiseFrom,
                          const int sizeInBytes) throw()
    : data (0),
      size (jmax (0, sizeInBytes))
{
    jassert (sizeInBytes >= 0);

    if (size > 0)
    {
        jassert (dataToInitialiseFrom != 0); // non-zero size, but a zero pointer passed-in?

        data = (char*) juce_malloc (size);

        if (dataToInitialiseFrom != 0)
            memcpy (data, dataToInitialiseFrom, size);
    }
}

MemoryBlock::~MemoryBlock() throw()
{
    jassert (size >= 0);    // should never happen
    jassert (size == 0 || data != 0); // non-zero size but no data allocated?

    juce_free (data);
}

const MemoryBlock& MemoryBlock::operator= (const MemoryBlock& other) throw()
{
    if (this != &other)
    {
        setSize (other.size, false);
        memcpy (data, other.data, size);
    }

    return *this;
}

//==============================================================================
bool MemoryBlock::operator== (const MemoryBlock& other) const throw()
{
    return (size == other.size)
            && (memcmp (data, other.data, size) == 0);
}

bool MemoryBlock::operator!= (const MemoryBlock& other) const throw()
{
    return ! operator== (other);
}

//==============================================================================
// this will resize the block to this size
void MemoryBlock::setSize (const int newSize,
                           const bool initialiseToZero) throw()
{
    if (size != newSize)
    {
        if (newSize <= 0)
        {
            juce_free (data);
            data = 0;
            size = 0;
        }
        else
        {
            if (data != 0)
            {
                data = (char*) juce_realloc (data, newSize);

                if (initialiseToZero && (newSize > size))
                    zeromem (data + size, newSize - size);
            }
            else
            {
                if (initialiseToZero)
                    data = (char*) juce_calloc (newSize);
                else
                    data = (char*) juce_malloc (newSize);
            }

            size = newSize;
        }
    }
}

void MemoryBlock::ensureSize (const int minimumSize,
                              const bool initialiseToZero) throw()
{
    if (size < minimumSize)
        setSize (minimumSize, initialiseToZero);
}

//==============================================================================
void MemoryBlock::fillWith (const uint8 value) throw()
{
    memset (data, (int) value, size);
}

void MemoryBlock::append (const void* const srcData,
                          const int numBytes) throw()
{
    if (numBytes > 0)
    {
        const int oldSize = size;
        setSize (size + numBytes);
        memcpy (data + oldSize, srcData, numBytes);
    }
}

void MemoryBlock::copyFrom (const void* const src, int offset, int num) throw()
{
    const char* d = (const char*) src;

    if (offset < 0)
    {
        d -= offset;
        num -= offset;
        offset = 0;
    }

    if (offset + num > size)
        num = size - offset;

    if (num > 0)
        memcpy (data + offset, d, num);
}

void MemoryBlock::copyTo (void* const dst, int offset, int num) const throw()
{
    char* d = (char*) dst;

    if (offset < 0)
    {
        zeromem (d, -offset);
        d -= offset;

        num += offset;
        offset = 0;
    }

    if (offset + num > size)
    {
        const int newNum = size - offset;
        zeromem (d + newNum, num - newNum);
        num = newNum;
    }

    if (num > 0)
        memcpy (d, data + offset, num);
}

void MemoryBlock::removeSection (int startByte, int numBytesToRemove) throw()
{
    if (startByte < 0)
    {
        numBytesToRemove += startByte;
        startByte = 0;
    }

    if (startByte + numBytesToRemove >= size)
    {
        setSize (startByte);
    }
    else if (numBytesToRemove > 0)
    {
        memmove (data + startByte,
                 data + startByte + numBytesToRemove,
                 size - (startByte + numBytesToRemove));

        setSize (size - numBytesToRemove);
    }
}

const String MemoryBlock::toString() const throw()
{
    return String (data, size);
}

//==============================================================================
int MemoryBlock::getBitRange (const int bitRangeStart, int numBits) const throw()
{
    int res = 0;

    int byte = bitRangeStart >> 3;
    int offsetInByte = bitRangeStart & 7;
    int bitsSoFar = 0;

    while (numBits > 0 && byte < size)
    {
        const int bitsThisTime = jmin (numBits, 8 - offsetInByte);
        const int mask = (0xff >> (8 - bitsThisTime)) << offsetInByte;

        res |= (((data[byte] & mask) >> offsetInByte) << bitsSoFar);

        bitsSoFar += bitsThisTime;
        numBits -= bitsThisTime;
        ++byte;
        offsetInByte = 0;
    }

    return res;
}

void MemoryBlock::setBitRange (const int bitRangeStart, int numBits, int bitsToSet) throw()
{
    int byte = bitRangeStart >> 3;
    int offsetInByte = bitRangeStart & 7;
    unsigned int mask = ~((((unsigned int)0xffffffff) << (32 - numBits)) >> (32 - numBits));

    while (numBits > 0 && byte < size)
    {
        const int bitsThisTime = jmin (numBits, 8 - offsetInByte);

        const unsigned int tempMask = (mask << offsetInByte) | ~((((unsigned int)0xffffffff) >> offsetInByte) << offsetInByte);
        const unsigned int tempBits = bitsToSet << offsetInByte;

        data[byte] = (char)((data[byte] & tempMask) | tempBits);

        ++byte;
        numBits -= bitsThisTime;
        bitsToSet >>= bitsThisTime;
        mask >>= bitsThisTime;
        offsetInByte = 0;
    }
}

//==============================================================================
void MemoryBlock::loadFromHexString (const String& hex) throw()
{
    ensureSize (hex.length() >> 1);
    char* dest = data;
    int i = 0;

    for (;;)
    {
        int byte = 0;

        for (int loop = 2; --loop >= 0;)
        {
            byte <<= 4;

            for (;;)
            {
                const tchar c = hex [i++];

                if (c >= T('0') && c <= T('9'))
                {
                    byte |= c - T('0');
                    break;
                }
                else if (c >= T('a') && c <= T('z'))
                {
                    byte |= c - (T('a') - 10);
                    break;
                }
                else if (c >= T('A') && c <= T('Z'))
                {
                    byte |= c - (T('A') - 10);
                    break;
                }
                else if (c == 0)
                {
                    setSize ((int) (dest - data));
                    return;
                }
            }
        }

        *dest++ = (char) byte;
    }
}

//==============================================================================
static const char* const encodingTable
    = ".ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+";

const String MemoryBlock::toBase64Encoding() const throw()
{
    const int numChars = ((size << 3) + 5) / 6;

    String destString (size); // store the length, followed by a '.', and then the data.
    const int initialLen = destString.length();
    destString.preallocateStorage (initialLen + 2 + numChars);

    tchar* d = const_cast <tchar*> (((const tchar*) destString) + initialLen);
    *d++ = T('.');

    for (int i = 0; i < numChars; ++i)
        *d++ = encodingTable [getBitRange (i * 6, 6)];

    *d++ = 0;

    return destString;
}

bool MemoryBlock::fromBase64Encoding (const String& s) throw()
{
    const int startPos = s.indexOfChar (T('.')) + 1;

    if (startPos <= 0)
        return false;

    const int numBytesNeeded = s.substring (0, startPos - 1).getIntValue();

    setSize (numBytesNeeded, true);

    const int numChars = s.length() - startPos;
    const tchar* const srcChars = ((const tchar*) s) + startPos;

    for (int i = 0; i < numChars; ++i)
    {
        const char c = (char) srcChars[i];

        for (int j = 0; j < 64; ++j)
        {
            if (encodingTable[j] == c)
            {
                setBitRange (i * 6, 6, j);
                break;
            }
        }
    }

    return true;
}


END_JUCE_NAMESPACE
