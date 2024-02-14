/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
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

struct MD5Generator
{
    void processBlock (const void* data, size_t dataSize) noexcept
    {
        auto bufferPos = ((count[0] >> 3) & 0x3f);

        count[0] += (uint32_t) (dataSize << 3);

        if (count[0] < ((uint32_t) dataSize << 3))
            count[1]++;

        count[1] += (uint32_t) (dataSize >> 29);

        auto spaceLeft = (size_t) 64 - (size_t) bufferPos;
        size_t i = 0;

        if (dataSize >= spaceLeft)
        {
            memcpy (buffer + bufferPos, data, spaceLeft);
            transform (buffer);

            for (i = spaceLeft; i + 64 <= dataSize; i += 64)
                transform (static_cast<const char*> (data) + i);

            bufferPos = 0;
        }

        memcpy (buffer + bufferPos, static_cast<const char*> (data) + i, dataSize - i);
    }

    void transform (const void* bufferToTransform) noexcept
    {
        auto a = state[0];
        auto b = state[1];
        auto c = state[2];
        auto d = state[3];

        uint32_t x[16];
        copyWithEndiannessConversion (x, bufferToTransform, 64);

        enum Constants
        {
            S11 = 7, S12 = 12, S13 = 17, S14 = 22, S21 = 5, S22 = 9,  S23 = 14, S24 = 20,
            S31 = 4, S32 = 11, S33 = 16, S34 = 23, S41 = 6, S42 = 10, S43 = 15, S44 = 21
        };

        FF (a, b, c, d, x[ 0], S11, 0xd76aa478);     FF (d, a, b, c, x[ 1], S12, 0xe8c7b756);
        FF (c, d, a, b, x[ 2], S13, 0x242070db);     FF (b, c, d, a, x[ 3], S14, 0xc1bdceee);
        FF (a, b, c, d, x[ 4], S11, 0xf57c0faf);     FF (d, a, b, c, x[ 5], S12, 0x4787c62a);
        FF (c, d, a, b, x[ 6], S13, 0xa8304613);     FF (b, c, d, a, x[ 7], S14, 0xfd469501);
        FF (a, b, c, d, x[ 8], S11, 0x698098d8);     FF (d, a, b, c, x[ 9], S12, 0x8b44f7af);
        FF (c, d, a, b, x[10], S13, 0xffff5bb1);     FF (b, c, d, a, x[11], S14, 0x895cd7be);
        FF (a, b, c, d, x[12], S11, 0x6b901122);     FF (d, a, b, c, x[13], S12, 0xfd987193);
        FF (c, d, a, b, x[14], S13, 0xa679438e);     FF (b, c, d, a, x[15], S14, 0x49b40821);

        GG (a, b, c, d, x[ 1], S21, 0xf61e2562);     GG (d, a, b, c, x[ 6], S22, 0xc040b340);
        GG (c, d, a, b, x[11], S23, 0x265e5a51);     GG (b, c, d, a, x[ 0], S24, 0xe9b6c7aa);
        GG (a, b, c, d, x[ 5], S21, 0xd62f105d);     GG (d, a, b, c, x[10], S22, 0x02441453);
        GG (c, d, a, b, x[15], S23, 0xd8a1e681);     GG (b, c, d, a, x[ 4], S24, 0xe7d3fbc8);
        GG (a, b, c, d, x[ 9], S21, 0x21e1cde6);     GG (d, a, b, c, x[14], S22, 0xc33707d6);
        GG (c, d, a, b, x[ 3], S23, 0xf4d50d87);     GG (b, c, d, a, x[ 8], S24, 0x455a14ed);
        GG (a, b, c, d, x[13], S21, 0xa9e3e905);     GG (d, a, b, c, x[ 2], S22, 0xfcefa3f8);
        GG (c, d, a, b, x[ 7], S23, 0x676f02d9);     GG (b, c, d, a, x[12], S24, 0x8d2a4c8a);

        HH (a, b, c, d, x[ 5], S31, 0xfffa3942);     HH (d, a, b, c, x[ 8], S32, 0x8771f681);
        HH (c, d, a, b, x[11], S33, 0x6d9d6122);     HH (b, c, d, a, x[14], S34, 0xfde5380c);
        HH (a, b, c, d, x[ 1], S31, 0xa4beea44);     HH (d, a, b, c, x[ 4], S32, 0x4bdecfa9);
        HH (c, d, a, b, x[ 7], S33, 0xf6bb4b60);     HH (b, c, d, a, x[10], S34, 0xbebfbc70);
        HH (a, b, c, d, x[13], S31, 0x289b7ec6);     HH (d, a, b, c, x[ 0], S32, 0xeaa127fa);
        HH (c, d, a, b, x[ 3], S33, 0xd4ef3085);     HH (b, c, d, a, x[ 6], S34, 0x04881d05);
        HH (a, b, c, d, x[ 9], S31, 0xd9d4d039);     HH (d, a, b, c, x[12], S32, 0xe6db99e5);
        HH (c, d, a, b, x[15], S33, 0x1fa27cf8);     HH (b, c, d, a, x[ 2], S34, 0xc4ac5665);

        II (a, b, c, d, x[ 0], S41, 0xf4292244);     II (d, a, b, c, x[ 7], S42, 0x432aff97);
        II (c, d, a, b, x[14], S43, 0xab9423a7);     II (b, c, d, a, x[ 5], S44, 0xfc93a039);
        II (a, b, c, d, x[12], S41, 0x655b59c3);     II (d, a, b, c, x[ 3], S42, 0x8f0ccc92);
        II (c, d, a, b, x[10], S43, 0xffeff47d);     II (b, c, d, a, x[ 1], S44, 0x85845dd1);
        II (a, b, c, d, x[ 8], S41, 0x6fa87e4f);     II (d, a, b, c, x[15], S42, 0xfe2ce6e0);
        II (c, d, a, b, x[ 6], S43, 0xa3014314);     II (b, c, d, a, x[13], S44, 0x4e0811a1);
        II (a, b, c, d, x[ 4], S41, 0xf7537e82);     II (d, a, b, c, x[11], S42, 0xbd3af235);
        II (c, d, a, b, x[ 2], S43, 0x2ad7d2bb);     II (b, c, d, a, x[ 9], S44, 0xeb86d391);

        state[0] += a;
        state[1] += b;
        state[2] += c;
        state[3] += d;
    }

    void finish (uint8_t* result) noexcept
    {
        uint8_t encodedLength[8];
        copyWithEndiannessConversion (encodedLength, count, 8);

        // Pad out to 56 mod 64.
        auto index = (count[0] >> 3) & 0x3f;
        auto paddingLength = (index < 56 ? 56 : 120) - index;

        uint8_t paddingBuffer[64] = { 0x80 }; // first byte is 0x80, remaining bytes are zero.

        processBlock (paddingBuffer, (size_t) paddingLength);
        processBlock (encodedLength, 8);

        copyWithEndiannessConversion (result, state, 16);
    }

private:
    uint8_t buffer[64] = {};
    uint32_t state[4] = { 0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476 };
    uint32_t count[2] = {};

    static void copyWithEndiannessConversion (void* output, const void* input, size_t numBytes) noexcept
    {
       #if JUCE_LITTLE_ENDIAN
        memcpy (output, input, numBytes);
       #else
        auto dst = static_cast<uint8_t*> (output);
        auto src = static_cast<const uint8_t*> (input);

        for (size_t i = 0; i < numBytes; i += 4)
        {
            dst[i + 0] = src[i + 3];
            dst[i + 1] = src[i + 2];
            dst[i + 2] = src[i + 1];
            dst[i + 3] = src[i + 0];
        }
       #endif
    }

    static uint32_t rotateLeft (uint32_t x, uint32_t n) noexcept     { return (x << n) | (x >> (32 - n)); }

    static uint32_t F (uint32_t x, uint32_t y, uint32_t z) noexcept  { return (x & y) | (~x & z); }
    static uint32_t G (uint32_t x, uint32_t y, uint32_t z) noexcept  { return (x & z) | (y & ~z); }
    static uint32_t H (uint32_t x, uint32_t y, uint32_t z) noexcept  { return x ^ y ^ z; }
    static uint32_t I (uint32_t x, uint32_t y, uint32_t z) noexcept  { return y ^ (x | ~z); }

    static void FF (uint32_t& a, uint32_t b, uint32_t c, uint32_t d, uint32_t x, uint32_t s, uint32_t ac) noexcept
    {
        a = rotateLeft (a + F (b, c, d) + x + ac, s) + b;
    }

    static void GG (uint32_t& a, uint32_t b, uint32_t c, uint32_t d, uint32_t x, uint32_t s, uint32_t ac) noexcept
    {
        a = rotateLeft (a + G (b, c, d) + x + ac, s) + b;
    }

    static void HH (uint32_t& a, uint32_t b, uint32_t c, uint32_t d, uint32_t x, uint32_t s, uint32_t ac) noexcept
    {
        a = rotateLeft (a + H (b, c, d) + x + ac, s) + b;
    }

    static void II (uint32_t& a, uint32_t b, uint32_t c, uint32_t d, uint32_t x, uint32_t s, uint32_t ac) noexcept
    {
        a = rotateLeft (a + I (b, c, d) + x + ac, s) + b;
    }
};

//==============================================================================
MD5::MD5() = default;
MD5::~MD5() = default;
MD5::MD5 (const MD5&) = default;
MD5& MD5::operator= (const MD5&) = default;

MD5::MD5 (const void* data, size_t numBytes) noexcept
{
    MD5Generator generator;
    generator.processBlock (data, numBytes);
    generator.finish (result);
}

MD5::MD5 (const MemoryBlock& data) noexcept : MD5 (data.getData(), data.getSize()) {}
MD5::MD5 (CharPointer_UTF8 utf8) noexcept : MD5 (utf8.getAddress(), utf8.getAddress() != nullptr ? utf8.sizeInBytes() - 1 : 0) {}

MD5 MD5::fromUTF32 (StringRef text)
{
    MD5 m;
    MD5Generator generator;

    for (auto t = text.text; t.isNotEmpty();)
    {
        auto unicodeChar = ByteOrder::swapIfBigEndian ((uint32_t) t.getAndAdvance());
        generator.processBlock (&unicodeChar, sizeof (unicodeChar));
    }

    generator.finish (m.result);
    return m;
}

MD5::MD5 (InputStream& input, int64 numBytesToRead)
{
    processStream (input, numBytesToRead);
}

MD5::MD5 (const File& file)
{
    FileInputStream fin (file);

    if (fin.openedOk())
        processStream (fin, -1);
}

void MD5::processStream (InputStream& input, int64 numBytesToRead)
{
    MD5Generator generator;

    if (numBytesToRead < 0)
        numBytesToRead = std::numeric_limits<int64>::max();

    while (numBytesToRead > 0)
    {
        uint8_t tempBuffer[512];
        auto bytesRead = input.read (tempBuffer, (int) jmin (numBytesToRead, (int64) sizeof (tempBuffer)));

        if (bytesRead <= 0)
            break;

        numBytesToRead -= bytesRead;
        generator.processBlock (tempBuffer, (size_t) bytesRead);
    }

    generator.finish (result);
}

//==============================================================================
MemoryBlock MD5::getRawChecksumData() const
{
    return MemoryBlock (result, sizeof (result));
}

String MD5::toHexString() const
{
    return String::toHexString (result, sizeof (result), 0);
}

//==============================================================================
bool MD5::operator== (const MD5& other) const noexcept   { return memcmp (result, other.result, sizeof (result)) == 0; }
bool MD5::operator!= (const MD5& other) const noexcept   { return ! operator== (other); }


//==============================================================================
//==============================================================================
#if JUCE_UNIT_TESTS

class MD5Tests final : public UnitTest
{
public:
    MD5Tests()
        : UnitTest ("MD5", UnitTestCategories::cryptography)
    {}

    void test (const char* input, const char* expected)
    {
        {
            MD5 hash (input, strlen (input));
            expectEquals (hash.toHexString(), String (expected));
        }

        {
            MemoryInputStream m (input, strlen (input), false);
            MD5 hash (m);
            expectEquals (hash.toHexString(), String (expected));
        }
    }

    void runTest() override
    {
        beginTest ("MD5");

        test ("", "d41d8cd98f00b204e9800998ecf8427e");
        test ("The quick brown fox jumps over the lazy dog",  "9e107d9d372bb6826bd81d3542a419d6");
        test ("The quick brown fox jumps over the lazy dog.", "e4d909c290d0fb1ca068ffaddf22cbd0");

        expectEquals (MD5 (CharPointer_UTF8 (nullptr)).toHexString(), String ("d41d8cd98f00b204e9800998ecf8427e"));
    }
};

static MD5Tests MD5UnitTests;

#endif

} // namespace juce
