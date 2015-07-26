/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

class MD5Generator
{
public:
    MD5Generator() noexcept
    {
        state[0] = 0x67452301;
        state[1] = 0xefcdab89;
        state[2] = 0x98badcfe;
        state[3] = 0x10325476;

        count[0] = 0;
        count[1] = 0;
    }

    void processBlock (const void* data, size_t dataSize) noexcept
    {
        int bufferPos = ((count[0] >> 3) & 0x3F);

        count[0] += (uint32) (dataSize << 3);

        if (count[0] < ((uint32) dataSize << 3))
            count[1]++;

        count[1] += (uint32) (dataSize >> 29);

        const size_t spaceLeft = 64 - (size_t) bufferPos;
        size_t i = 0;

        if (dataSize >= spaceLeft)
        {
            memcpy (buffer + bufferPos, data, spaceLeft);
            transform (buffer);

            for (i = spaceLeft; i + 64 <= dataSize; i += 64)
                transform (static_cast <const char*> (data) + i);

            bufferPos = 0;
        }

        memcpy (buffer + bufferPos, static_cast <const char*> (data) + i, dataSize - i);
    }

    void transform (const void* bufferToTransform) noexcept
    {
        uint32 a = state[0];
        uint32 b = state[1];
        uint32 c = state[2];
        uint32 d = state[3];
        uint32 x[16];

        encode (x, bufferToTransform, 64);

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

        zerostruct (x);
    }

    void finish (void* result) noexcept
    {
        unsigned char encodedLength[8];
        encode (encodedLength, count, 8);

        // Pad out to 56 mod 64.
        const int index = (count[0] >> 3) & 0x3f;

        const int paddingLength = (index < 56) ? (56 - index)
                                               : (120 - index);

        uint8 paddingBuffer[64] = { 0x80 }; // first byte is 0x80, remaining bytes are zero.
        processBlock (paddingBuffer, (size_t) paddingLength);

        processBlock (encodedLength, 8);

        encode (result, state, 16);
        zerostruct (buffer);
    }

private:
    uint8 buffer [64];
    uint32 state [4];
    uint32 count [2];

    static void encode (void* const output, const void* const input, const int numBytes) noexcept
    {
        for (int i = 0; i < (numBytes >> 2); ++i)
            static_cast<uint32*> (output)[i] = ByteOrder::swapIfBigEndian (static_cast<const uint32*> (input) [i]);
    }

    static inline uint32 rotateLeft (const uint32 x, const uint32 n) noexcept          { return (x << n) | (x >> (32 - n)); }

    static inline uint32 F (const uint32 x, const uint32 y, const uint32 z) noexcept   { return (x & y) | (~x & z); }
    static inline uint32 G (const uint32 x, const uint32 y, const uint32 z) noexcept   { return (x & z) | (y & ~z); }
    static inline uint32 H (const uint32 x, const uint32 y, const uint32 z) noexcept   { return x ^ y ^ z; }
    static inline uint32 I (const uint32 x, const uint32 y, const uint32 z) noexcept   { return y ^ (x | ~z); }

    static void FF (uint32& a, const uint32 b, const uint32 c, const uint32 d, const uint32 x, const uint32 s, const uint32 ac) noexcept
    {
        a += F (b, c, d) + x + ac;
        a = rotateLeft (a, s) + b;
    }

    static void GG (uint32& a, const uint32 b, const uint32 c, const uint32 d, const uint32 x, const uint32 s, const uint32 ac) noexcept
    {
        a += G (b, c, d) + x + ac;
        a = rotateLeft (a, s) + b;
    }

    static void HH (uint32& a, const uint32 b, const uint32 c, const uint32 d, const uint32 x, const uint32 s, const uint32 ac) noexcept
    {
        a += H (b, c, d) + x + ac;
        a = rotateLeft (a, s) + b;
    }

    static void II (uint32& a, const uint32 b, const uint32 c, const uint32 d, const uint32 x, const uint32 s, const uint32 ac) noexcept
    {
        a += I (b, c, d) + x + ac;
        a = rotateLeft (a, s) + b;
    }
};

//==============================================================================
MD5::MD5() noexcept
{
    zerostruct (result);
}

MD5::MD5 (const MD5& other) noexcept
{
    memcpy (result, other.result, sizeof (result));
}

MD5& MD5::operator= (const MD5& other) noexcept
{
    memcpy (result, other.result, sizeof (result));
    return *this;
}

//==============================================================================
MD5::MD5 (const MemoryBlock& data) noexcept
{
    processData (data.getData(), data.getSize());
}

MD5::MD5 (const void* data, const size_t numBytes) noexcept
{
    processData (data, numBytes);
}

MD5::MD5 (CharPointer_UTF8 utf8) noexcept
{
    jassert (utf8.getAddress() != nullptr);
    processData (utf8.getAddress(), utf8.sizeInBytes() - 1);
}

MD5 MD5::fromUTF32 (StringRef text)
{
    MD5Generator generator;
    String::CharPointerType t (text.text);

    while (! t.isEmpty())
    {
        uint32 unicodeChar = ByteOrder::swapIfBigEndian ((uint32) t.getAndAdvance());
        generator.processBlock (&unicodeChar, sizeof (unicodeChar));
    }

    MD5 m;
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

    if (fin.getStatus().wasOk())
        processStream (fin, -1);
    else
        zerostruct (result);
}

MD5::~MD5() noexcept {}

void MD5::processData (const void* data, size_t numBytes) noexcept
{
    MD5Generator generator;
    generator.processBlock (data, numBytes);
    generator.finish (result);
}

void MD5::processStream (InputStream& input, int64 numBytesToRead)
{
    MD5Generator generator;

    if (numBytesToRead < 0)
        numBytesToRead = std::numeric_limits<int64>::max();

    while (numBytesToRead > 0)
    {
        uint8 tempBuffer [512];
        const int bytesRead = input.read (tempBuffer, (int) jmin (numBytesToRead, (int64) sizeof (tempBuffer)));

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
#if JUCE_UNIT_TESTS

class MD5Tests  : public UnitTest
{
public:
    MD5Tests() : UnitTest ("MD5") {}

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

    void runTest()
    {
        beginTest ("MD5");

        test ("", "d41d8cd98f00b204e9800998ecf8427e");
        test ("The quick brown fox jumps over the lazy dog",  "9e107d9d372bb6826bd81d3542a419d6");
        test ("The quick brown fox jumps over the lazy dog.", "e4d909c290d0fb1ca068ffaddf22cbd0");
    }
};

static MD5Tests MD5UnitTests;

#endif
