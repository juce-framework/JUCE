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

MemoryBlock::MemoryBlock() noexcept
    : size (0)
{
}

MemoryBlock::MemoryBlock (const size_t initialSize, const bool initialiseToZero)
{
    if (initialSize > 0)
    {
        size = initialSize;
        data.allocate (initialSize, initialiseToZero);
    }
    else
    {
        size = 0;
    }
}

MemoryBlock::MemoryBlock (const MemoryBlock& other)
    : size (other.size)
{
    if (size > 0)
    {
        jassert (other.data != nullptr);
        data.malloc (size);
        memcpy (data, other.data, size);
    }
}

MemoryBlock::MemoryBlock (const void* const dataToInitialiseFrom, const size_t sizeInBytes)
    : size (sizeInBytes)
{
    jassert (((ssize_t) sizeInBytes) >= 0);

    if (size > 0)
    {
        jassert (dataToInitialiseFrom != nullptr); // non-zero size, but a zero pointer passed-in?

        data.malloc (size);

        if (dataToInitialiseFrom != nullptr)
            memcpy (data, dataToInitialiseFrom, size);
    }
}

MemoryBlock::~MemoryBlock() noexcept
{
}

MemoryBlock& MemoryBlock::operator= (const MemoryBlock& other)
{
    if (this != &other)
    {
        setSize (other.size, false);
        memcpy (data, other.data, size);
    }

    return *this;
}

#if JUCE_COMPILER_SUPPORTS_MOVE_SEMANTICS
MemoryBlock::MemoryBlock (MemoryBlock&& other) noexcept
    : data (static_cast <HeapBlock <char>&&> (other.data)),
      size (other.size)
{
}

MemoryBlock& MemoryBlock::operator= (MemoryBlock&& other) noexcept
{
    data = static_cast <HeapBlock <char>&&> (other.data);
    size = other.size;
    return *this;
}
#endif


//==============================================================================
bool MemoryBlock::operator== (const MemoryBlock& other) const noexcept
{
    return matches (other.data, other.size);
}

bool MemoryBlock::operator!= (const MemoryBlock& other) const noexcept
{
    return ! operator== (other);
}

bool MemoryBlock::matches (const void* dataToCompare, size_t dataSize) const noexcept
{
    return size == dataSize
            && memcmp (data, dataToCompare, size) == 0;
}

//==============================================================================
// this will resize the block to this size
void MemoryBlock::setSize (const size_t newSize, const bool initialiseToZero)
{
    if (size != newSize)
    {
        if (newSize <= 0)
        {
            data.free();
            size = 0;
        }
        else
        {
            if (data != nullptr)
            {
                data.realloc (newSize);

                if (initialiseToZero && (newSize > size))
                    zeromem (data + size, newSize - size);
            }
            else
            {
                data.allocate (newSize, initialiseToZero);
            }

            size = newSize;
        }
    }
}

void MemoryBlock::ensureSize (const size_t minimumSize, const bool initialiseToZero)
{
    if (size < minimumSize)
        setSize (minimumSize, initialiseToZero);
}

void MemoryBlock::swapWith (MemoryBlock& other) noexcept
{
    std::swap (size, other.size);
    data.swapWith (other.data);
}

//==============================================================================
void MemoryBlock::fillWith (const uint8 value) noexcept
{
    memset (data, (int) value, size);
}

void MemoryBlock::append (const void* const srcData, const size_t numBytes)
{
    if (numBytes > 0)
    {
        jassert (srcData != nullptr); // this must not be null!
        const size_t oldSize = size;
        setSize (size + numBytes);
        memcpy (data + oldSize, srcData, numBytes);
    }
}

void MemoryBlock::replaceWith (const void* const srcData, const size_t numBytes)
{
    if (numBytes > 0)
    {
        jassert (srcData != nullptr); // this must not be null!
        setSize (numBytes);
        memcpy (data, srcData, numBytes);
    }
}

void MemoryBlock::insert (const void* const srcData, const size_t numBytes, size_t insertPosition)
{
    if (numBytes > 0)
    {
        jassert (srcData != nullptr); // this must not be null!
        insertPosition = jmin (size, insertPosition);
        const size_t trailingDataSize = size - insertPosition;
        setSize (size + numBytes, false);

        if (trailingDataSize > 0)
            memmove (data + insertPosition + numBytes,
                     data + insertPosition,
                     trailingDataSize);

        memcpy (data + insertPosition, srcData, numBytes);
    }
}

void MemoryBlock::removeSection (const size_t startByte, const size_t numBytesToRemove)
{
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

void MemoryBlock::copyFrom (const void* const src, int offset, size_t num) noexcept
{
    const char* d = static_cast<const char*> (src);

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

void MemoryBlock::copyTo (void* const dst, int offset, size_t num) const noexcept
{
    char* d = static_cast<char*> (dst);

    if (offset < 0)
    {
        zeromem (d, (size_t) -offset);
        d -= offset;

        num += offset;
        offset = 0;
    }

    if (offset + num > size)
    {
        const size_t newNum = size - offset;
        zeromem (d + newNum, num - newNum);
        num = newNum;
    }

    if (num > 0)
        memcpy (d, data + offset, num);
}

String MemoryBlock::toString() const
{
    return String (CharPointer_UTF8 (data), size);
}

//==============================================================================
int MemoryBlock::getBitRange (const size_t bitRangeStart, size_t numBits) const noexcept
{
    int res = 0;

    size_t byte = bitRangeStart >> 3;
    int offsetInByte = (int) bitRangeStart & 7;
    size_t bitsSoFar = 0;

    while (numBits > 0 && (size_t) byte < size)
    {
        const int bitsThisTime = jmin ((int) numBits, 8 - offsetInByte);
        const int mask = (0xff >> (8 - bitsThisTime)) << offsetInByte;

        res |= (((data[byte] & mask) >> offsetInByte) << bitsSoFar);

        bitsSoFar += bitsThisTime;
        numBits -= bitsThisTime;
        ++byte;
        offsetInByte = 0;
    }

    return res;
}

void MemoryBlock::setBitRange (const size_t bitRangeStart, size_t numBits, int bitsToSet) noexcept
{
    size_t byte = bitRangeStart >> 3;
    int offsetInByte = (int) bitRangeStart & 7;
    unsigned int mask = ~((((unsigned int) 0xffffffff) << (32 - numBits)) >> (32 - numBits));

    while (numBits > 0 && (size_t) byte < size)
    {
        const int bitsThisTime = jmin ((int) numBits, 8 - offsetInByte);

        const unsigned int tempMask = (mask << offsetInByte) | ~((((unsigned int) 0xffffffff) >> offsetInByte) << offsetInByte);
        const unsigned int tempBits = (unsigned int) bitsToSet << offsetInByte;

        data[byte] = (char) ((data[byte] & tempMask) | tempBits);

        ++byte;
        numBits -= bitsThisTime;
        bitsToSet >>= bitsThisTime;
        mask >>= bitsThisTime;
        offsetInByte = 0;
    }
}

//==============================================================================
void MemoryBlock::loadFromHexString (const String& hex)
{
    ensureSize ((size_t) hex.length() >> 1);
    char* dest = data;
    String::CharPointerType t (hex.getCharPointer());

    for (;;)
    {
        int byte = 0;

        for (int loop = 2; --loop >= 0;)
        {
            byte <<= 4;

            for (;;)
            {
                const juce_wchar c = t.getAndAdvance();

                if (c >= '0' && c <= '9')
                {
                    byte |= c - '0';
                    break;
                }
                else if (c >= 'a' && c <= 'z')
                {
                    byte |= c - ('a' - 10);
                    break;
                }
                else if (c >= 'A' && c <= 'Z')
                {
                    byte |= c - ('A' - 10);
                    break;
                }
                else if (c == 0)
                {
                    setSize (static_cast <size_t> (dest - data));
                    return;
                }
            }
        }

        *dest++ = (char) byte;
    }
}

//==============================================================================
const char* const MemoryBlock::encodingTable = ".ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+";

String MemoryBlock::toBase64Encoding() const
{
    const size_t numChars = ((size << 3) + 5) / 6;

    String destString ((unsigned int) size); // store the length, followed by a '.', and then the data.
    const int initialLen = destString.length();
    destString.preallocateBytes (sizeof (String::CharPointerType::CharType) * (size_t) (initialLen + 2 + numChars));

    String::CharPointerType d (destString.getCharPointer());
    d += initialLen;
    d.write ('.');

    for (size_t i = 0; i < numChars; ++i)
        d.write ((juce_wchar) (uint8) encodingTable [getBitRange (i * 6, 6)]);

    d.writeNull();
    return destString;
}

bool MemoryBlock::fromBase64Encoding (const String& s)
{
    const int startPos = s.indexOfChar ('.') + 1;

    if (startPos <= 0)
        return false;

    const int numBytesNeeded = s.substring (0, startPos - 1).getIntValue();

    setSize ((size_t) numBytesNeeded, true);

    const int numChars = s.length() - startPos;

    String::CharPointerType srcChars (s.getCharPointer());
    srcChars += startPos;
    int pos = 0;

    for (int i = 0; i < numChars; ++i)
    {
        const char c = (char) srcChars.getAndAdvance();

        for (int j = 0; j < 64; ++j)
        {
            if (encodingTable[j] == c)
            {
                setBitRange ((size_t) pos, 6, j);
                pos += 6;
                break;
            }
        }
    }

    return true;
}
