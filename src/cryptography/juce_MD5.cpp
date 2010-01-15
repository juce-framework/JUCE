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

#include "../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE


#include "juce_MD5.h"
#include "../io/files/juce_FileInputStream.h"
#include "../containers/juce_ScopedPointer.h"


//==============================================================================
MD5::MD5()
{
    zeromem (result, sizeof (result));
}

MD5::MD5 (const MD5& other)
{
    memcpy (result, other.result, sizeof (result));
}

const MD5& MD5::operator= (const MD5& other)
{
    memcpy (result, other.result, sizeof (result));
    return *this;
}

//==============================================================================
MD5::MD5 (const MemoryBlock& data)
{
    ProcessContext context;
    context.processBlock ((const uint8*) data.getData(), data.getSize());
    context.finish (result);
}

MD5::MD5 (const char* data, const size_t numBytes)
{
    ProcessContext context;
    context.processBlock ((const uint8*) data, numBytes);
    context.finish (result);
}

MD5::MD5 (const String& text)
{
    ProcessContext context;

    const int len = text.length();
    const juce_wchar* const t = text;

    for (int i = 0; i < len; ++i)
    {
        // force the string into integer-sized unicode characters, to try to make it
        // get the same results on all platforms + compilers.
        uint32 unicodeChar = (uint32) t[i];
        ByteOrder::swapIfBigEndian (unicodeChar);

        context.processBlock ((const uint8*) &unicodeChar,
                              sizeof (unicodeChar));
    }

    context.finish (result);
}

void MD5::processStream (InputStream& input, int64 numBytesToRead)
{
    ProcessContext context;

    if (numBytesToRead < 0)
        numBytesToRead = INT_MAX;

    while (numBytesToRead > 0)
    {
        char tempBuffer [512];
        const int bytesRead = input.read (tempBuffer, (int) jmin (numBytesToRead, (int64) sizeof (tempBuffer)));

        if (bytesRead <= 0)
            break;

        numBytesToRead -= bytesRead;

        context.processBlock ((const uint8*) tempBuffer, bytesRead);
    }

    context.finish (result);
}

MD5::MD5 (InputStream& input, int64 numBytesToRead)
{
    processStream (input, numBytesToRead);
}

MD5::MD5 (const File& file)
{
    const ScopedPointer <FileInputStream> fin (file.createInputStream());

    if (fin != 0)
        processStream (*fin, -1);
    else
        zeromem (result, sizeof (result));
}

MD5::~MD5()
{
}

//==============================================================================
MD5::ProcessContext::ProcessContext()
{
    state[0] = 0x67452301;
    state[1] = 0xefcdab89;
    state[2] = 0x98badcfe;
    state[3] = 0x10325476;

    count[0] = 0;
    count[1] = 0;
}

void MD5::ProcessContext::processBlock (const uint8* const data, size_t dataSize)
{
    int bufferPos = ((count[0] >> 3) & 0x3F);

    count[0] += (uint32) (dataSize << 3);

    if (count[0] < ((uint32) dataSize << 3))
        count[1]++;

    count[1] += (uint32) (dataSize >> 29);

    const size_t spaceLeft = 64 - bufferPos;

    size_t i = 0;

    if (dataSize >= spaceLeft)
    {
        memcpy (buffer + bufferPos, data, spaceLeft);

        transform (buffer);

        i = spaceLeft;

        while (i < dataSize - 63)
        {
            transform (data + i);
            i += 64;
        }

        bufferPos = 0;
    }

    memcpy (buffer + bufferPos, data + i, dataSize - i);
}

//==============================================================================
static void encode (uint8* const output,
                    const uint32* const input,
                    const int numBytes)
{
    uint32* const o = (uint32*) output;

    for (int i = 0; i < (numBytes >> 2); ++i)
        o[i] = ByteOrder::swapIfBigEndian (input [i]);
}

static void decode (uint32* const output,
                    const uint8* const input,
                    const int numBytes)
{
    for (int i = 0; i < (numBytes >> 2); ++i)
        output[i] = ByteOrder::littleEndianInt ((const char*) input + (i << 2));
}

//==============================================================================
void MD5::ProcessContext::finish (uint8* const result)
{
    unsigned char encodedLength[8];
    encode (encodedLength, count, 8);

    // Pad out to 56 mod 64.
    const int index = (uint32) ((count[0] >> 3) & 0x3f);

    const int paddingLength = (index < 56) ? (56 - index)
                                           : (120 - index);

    uint8 paddingBuffer [64];
    zeromem (paddingBuffer, paddingLength);
    paddingBuffer [0] = 0x80;
    processBlock (paddingBuffer, paddingLength);

    processBlock (encodedLength, 8);

    encode (result, state, 16);

    zeromem (buffer, sizeof (buffer));
}

//==============================================================================
#define S11 7
#define S12 12
#define S13 17
#define S14 22
#define S21 5
#define S22 9
#define S23 14
#define S24 20
#define S31 4
#define S32 11
#define S33 16
#define S34 23
#define S41 6
#define S42 10
#define S43 15
#define S44 21

static inline uint32 F (const uint32 x, const uint32 y, const uint32 z)   { return (x & y) | (~x & z); }
static inline uint32 G (const uint32 x, const uint32 y, const uint32 z)   { return (x & z) | (y & ~z); }
static inline uint32 H (const uint32 x, const uint32 y, const uint32 z)   { return x ^ y ^ z; }
static inline uint32 I (const uint32 x, const uint32 y, const uint32 z)   { return y ^ (x | ~z); }

static inline uint32 rotateLeft (const uint32 x, const uint32 n)          { return (x << n) | (x >> (32 - n)); }

static inline void FF (uint32& a, const uint32 b, const uint32 c, const uint32 d, const uint32 x, const uint32 s, const uint32 ac)
{
    a += F (b, c, d) + x + ac;
    a = rotateLeft (a, s) + b;
}

static inline void GG (uint32& a, const uint32 b, const uint32 c, const uint32 d, const uint32 x, const uint32 s, const uint32 ac)
{
    a += G (b, c, d) + x + ac;
    a = rotateLeft (a, s) + b;
}

static inline void HH (uint32& a, const uint32 b, const uint32 c, const uint32 d, const uint32 x, const uint32 s, const uint32 ac)
{
    a += H (b, c, d) + x + ac;
    a = rotateLeft (a, s) + b;
}

static inline void II (uint32& a, const uint32 b, const uint32 c, const uint32 d, const uint32 x, const uint32 s, const uint32 ac)
{
    a += I (b, c, d) + x + ac;
    a = rotateLeft (a, s) + b;
}

void MD5::ProcessContext::transform (const uint8* const bufferToTransform)
{
    uint32 a = state[0];
    uint32 b = state[1];
    uint32 c = state[2];
    uint32 d = state[3];
    uint32 x[16];

    decode (x, bufferToTransform, 64);

    FF (a, b, c, d, x[ 0], S11, 0xd76aa478); /* 1 */
    FF (d, a, b, c, x[ 1], S12, 0xe8c7b756); /* 2 */
    FF (c, d, a, b, x[ 2], S13, 0x242070db); /* 3 */
    FF (b, c, d, a, x[ 3], S14, 0xc1bdceee); /* 4 */
    FF (a, b, c, d, x[ 4], S11, 0xf57c0faf); /* 5 */
    FF (d, a, b, c, x[ 5], S12, 0x4787c62a); /* 6 */
    FF (c, d, a, b, x[ 6], S13, 0xa8304613); /* 7 */
    FF (b, c, d, a, x[ 7], S14, 0xfd469501); /* 8 */
    FF (a, b, c, d, x[ 8], S11, 0x698098d8); /* 9 */
    FF (d, a, b, c, x[ 9], S12, 0x8b44f7af); /* 10 */
    FF (c, d, a, b, x[10], S13, 0xffff5bb1); /* 11 */
    FF (b, c, d, a, x[11], S14, 0x895cd7be); /* 12 */
    FF (a, b, c, d, x[12], S11, 0x6b901122); /* 13 */
    FF (d, a, b, c, x[13], S12, 0xfd987193); /* 14 */
    FF (c, d, a, b, x[14], S13, 0xa679438e); /* 15 */
    FF (b, c, d, a, x[15], S14, 0x49b40821); /* 16 */

    GG (a, b, c, d, x[ 1], S21, 0xf61e2562); /* 17 */
    GG (d, a, b, c, x[ 6], S22, 0xc040b340); /* 18 */
    GG (c, d, a, b, x[11], S23, 0x265e5a51); /* 19 */
    GG (b, c, d, a, x[ 0], S24, 0xe9b6c7aa); /* 20 */
    GG (a, b, c, d, x[ 5], S21, 0xd62f105d); /* 21 */
    GG (d, a, b, c, x[10], S22,  0x2441453); /* 22 */
    GG (c, d, a, b, x[15], S23, 0xd8a1e681); /* 23 */
    GG (b, c, d, a, x[ 4], S24, 0xe7d3fbc8); /* 24 */
    GG (a, b, c, d, x[ 9], S21, 0x21e1cde6); /* 25 */
    GG (d, a, b, c, x[14], S22, 0xc33707d6); /* 26 */
    GG (c, d, a, b, x[ 3], S23, 0xf4d50d87); /* 27 */
    GG (b, c, d, a, x[ 8], S24, 0x455a14ed); /* 28 */
    GG (a, b, c, d, x[13], S21, 0xa9e3e905); /* 29 */
    GG (d, a, b, c, x[ 2], S22, 0xfcefa3f8); /* 30 */
    GG (c, d, a, b, x[ 7], S23, 0x676f02d9); /* 31 */
    GG (b, c, d, a, x[12], S24, 0x8d2a4c8a); /* 32 */

    HH (a, b, c, d, x[ 5], S31, 0xfffa3942); /* 33 */
    HH (d, a, b, c, x[ 8], S32, 0x8771f681); /* 34 */
    HH (c, d, a, b, x[11], S33, 0x6d9d6122); /* 35 */
    HH (b, c, d, a, x[14], S34, 0xfde5380c); /* 36 */
    HH (a, b, c, d, x[ 1], S31, 0xa4beea44); /* 37 */
    HH (d, a, b, c, x[ 4], S32, 0x4bdecfa9); /* 38 */
    HH (c, d, a, b, x[ 7], S33, 0xf6bb4b60); /* 39 */
    HH (b, c, d, a, x[10], S34, 0xbebfbc70); /* 40 */
    HH (a, b, c, d, x[13], S31, 0x289b7ec6); /* 41 */
    HH (d, a, b, c, x[ 0], S32, 0xeaa127fa); /* 42 */
    HH (c, d, a, b, x[ 3], S33, 0xd4ef3085); /* 43 */
    HH (b, c, d, a, x[ 6], S34,  0x4881d05); /* 44 */
    HH (a, b, c, d, x[ 9], S31, 0xd9d4d039); /* 45 */
    HH (d, a, b, c, x[12], S32, 0xe6db99e5); /* 46 */
    HH (c, d, a, b, x[15], S33, 0x1fa27cf8); /* 47 */
    HH (b, c, d, a, x[ 2], S34, 0xc4ac5665); /* 48 */

    II (a, b, c, d, x[ 0], S41, 0xf4292244); /* 49 */
    II (d, a, b, c, x[ 7], S42, 0x432aff97); /* 50 */
    II (c, d, a, b, x[14], S43, 0xab9423a7); /* 51 */
    II (b, c, d, a, x[ 5], S44, 0xfc93a039); /* 52 */
    II (a, b, c, d, x[12], S41, 0x655b59c3); /* 53 */
    II (d, a, b, c, x[ 3], S42, 0x8f0ccc92); /* 54 */
    II (c, d, a, b, x[10], S43, 0xffeff47d); /* 55 */
    II (b, c, d, a, x[ 1], S44, 0x85845dd1); /* 56 */
    II (a, b, c, d, x[ 8], S41, 0x6fa87e4f); /* 57 */
    II (d, a, b, c, x[15], S42, 0xfe2ce6e0); /* 58 */
    II (c, d, a, b, x[ 6], S43, 0xa3014314); /* 59 */
    II (b, c, d, a, x[13], S44, 0x4e0811a1); /* 60 */
    II (a, b, c, d, x[ 4], S41, 0xf7537e82); /* 61 */
    II (d, a, b, c, x[11], S42, 0xbd3af235); /* 62 */
    II (c, d, a, b, x[ 2], S43, 0x2ad7d2bb); /* 63 */
    II (b, c, d, a, x[ 9], S44, 0xeb86d391); /* 64 */

    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;

    zeromem (x, sizeof (x));
}

//==============================================================================
const MemoryBlock MD5::getRawChecksumData() const
{
    return MemoryBlock (result, 16);
}

const String MD5::toHexString() const
{
    return String::toHexString (result, 16, 0);
}

//==============================================================================
bool MD5::operator== (const MD5& other) const
{
    return memcmp (result, other.result, 16) == 0;
}

bool MD5::operator!= (const MD5& other) const
{
    return ! operator== (other);
}

END_JUCE_NAMESPACE
