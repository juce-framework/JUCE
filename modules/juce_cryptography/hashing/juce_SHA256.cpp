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

class SHA256Processor
{
public:
    SHA256Processor() noexcept
        : length (0)
    {
        state[0] = 0x6a09e667;
        state[1] = 0xbb67ae85;
        state[2] = 0x3c6ef372;
        state[3] = 0xa54ff53a;
        state[4] = 0x510e527f;
        state[5] = 0x9b05688c;
        state[6] = 0x1f83d9ab;
        state[7] = 0x5be0cd19;
    }

    // expects 64 bytes of data
    void processFullBlock (const void* const data) noexcept
    {
        const uint32 constants[] =
        {
            0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
            0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
            0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
            0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
            0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
            0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
            0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
            0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
        };

        uint32 block[16], s[8];
        memcpy (s, state, sizeof (s));

        for (int i = 0; i < 16; ++i)
            block[i] = ByteOrder::bigEndianInt (addBytesToPointer (data, i * 4));

        for (uint32 j = 0; j < 64; j += 16)
        {
            #define JUCE_SHA256(i) \
                s[(7 - i) & 7] += S1 (s[(4 - i) & 7]) + ch (s[(4 - i) & 7], s[(5 - i) & 7], s[(6 - i) & 7]) + constants[i + j] \
                                     + (j != 0 ? (block[i & 15] += s1 (block[(i - 2) & 15]) + block[(i - 7) & 15] + s0 (block[(i - 15) & 15])) \
                                               : block[i]); \
                s[(3 - i) & 7] += s[(7 - i) & 7]; \
                s[(7 - i) & 7] += S0 (s[(0 - i) & 7]) + maj (s[(0 - i) & 7], s[(1 - i) & 7], s[(2 - i) & 7])

            JUCE_SHA256(0);  JUCE_SHA256(1);  JUCE_SHA256(2);  JUCE_SHA256(3);  JUCE_SHA256(4);  JUCE_SHA256(5);  JUCE_SHA256(6);  JUCE_SHA256(7);
            JUCE_SHA256(8);  JUCE_SHA256(9);  JUCE_SHA256(10); JUCE_SHA256(11); JUCE_SHA256(12); JUCE_SHA256(13); JUCE_SHA256(14); JUCE_SHA256(15);
            #undef JUCE_SHA256
        }

        for (int i = 0; i < 8; ++i)
            state[i] += s[i];

        length += 64;
    }

    void processFinalBlock (const void* const data, unsigned int numBytes) noexcept
    {
        jassert (numBytes < 64);

        length += numBytes;
        length *= 8; // (the length is stored as a count of bits, not bytes)

        uint8 finalBlocks[128];

        memcpy (finalBlocks, data, numBytes);
        finalBlocks [numBytes++] = 128; // append a '1' bit

        while (numBytes != 56 && numBytes < 64 + 56)
            finalBlocks [numBytes++] = 0; // pad with zeros..

        for (int i = 8; --i >= 0;)
            finalBlocks [numBytes++] = (uint8) (length >> (i * 8)); // append the length.

        jassert (numBytes == 64 || numBytes == 128);

        processFullBlock (finalBlocks);

        if (numBytes > 64)
            processFullBlock (finalBlocks + 64);
    }

    void copyResult (uint8* result) const noexcept
    {
        for (int i = 0; i < 8; ++i)
        {
            *result++ = (uint8) (state[i] >> 24);
            *result++ = (uint8) (state[i] >> 16);
            *result++ = (uint8) (state[i] >> 8);
            *result++ = (uint8) state[i];
        }
    }

    void processStream (InputStream& input, int64 numBytesToRead, uint8* const result)
    {
        if (numBytesToRead < 0)
            numBytesToRead = std::numeric_limits<int64>::max();

        for (;;)
        {
            uint8 buffer [64];
            const int bytesRead = input.read (buffer, (int) jmin (numBytesToRead, (int64) sizeof (buffer)));

            if (bytesRead < (int) sizeof (buffer))
            {
                processFinalBlock (buffer, (unsigned int) bytesRead);
                break;
            }

            numBytesToRead -= sizeof (buffer);
            processFullBlock (buffer);
        }

        copyResult (result);
    }

private:
    uint32 state[8];
    uint64 length;

    static inline uint32 rotate (const uint32 x, const uint32 y) noexcept                { return (x >> y) | (x << (32 - y)); }
    static inline uint32 ch  (const uint32 x, const uint32 y, const uint32 z) noexcept   { return z ^ ((y ^ z) & x); }
    static inline uint32 maj (const uint32 x, const uint32 y, const uint32 z) noexcept   { return y ^ ((y ^ z) & (x ^ y)); }

    static inline uint32 s0 (const uint32 x) noexcept     { return rotate (x, 7)  ^ rotate (x, 18) ^ (x >> 3); }
    static inline uint32 s1 (const uint32 x) noexcept     { return rotate (x, 17) ^ rotate (x, 19) ^ (x >> 10); }
    static inline uint32 S0 (const uint32 x) noexcept     { return rotate (x, 2)  ^ rotate (x, 13) ^ rotate (x, 22); }
    static inline uint32 S1 (const uint32 x) noexcept     { return rotate (x, 6)  ^ rotate (x, 11) ^ rotate (x, 25); }

    JUCE_DECLARE_NON_COPYABLE (SHA256Processor)
};

//==============================================================================
SHA256::SHA256() noexcept
{
    zerostruct (result);
}

SHA256::~SHA256() noexcept {}

SHA256::SHA256 (const SHA256& other) noexcept
{
    memcpy (result, other.result, sizeof (result));
}

SHA256& SHA256::operator= (const SHA256& other) noexcept
{
    memcpy (result, other.result, sizeof (result));
    return *this;
}

SHA256::SHA256 (const MemoryBlock& data)
{
    process (data.getData(), data.getSize());
}

SHA256::SHA256 (const void* const data, const size_t numBytes)
{
    process (data, numBytes);
}

SHA256::SHA256 (InputStream& input, const int64 numBytesToRead)
{
    SHA256Processor processor;
    processor.processStream (input, numBytesToRead, result);
}

SHA256::SHA256 (const File& file)
{
    FileInputStream fin (file);

    if (fin.getStatus().wasOk())
    {
        SHA256Processor processor;
        processor.processStream (fin, -1, result);
    }
    else
    {
        zerostruct (result);
    }
}

SHA256::SHA256 (CharPointer_UTF8 utf8) noexcept
{
    jassert (utf8.getAddress() != nullptr);
    process (utf8.getAddress(), utf8.sizeInBytes() - 1);
}

void SHA256::process (const void* const data, size_t numBytes)
{
    MemoryInputStream m (data, numBytes, false);
    SHA256Processor processor;
    processor.processStream (m, -1, result);
}

MemoryBlock SHA256::getRawData() const
{
    return MemoryBlock (result, sizeof (result));
}

String SHA256::toHexString() const
{
    return String::toHexString (result, sizeof (result), 0);
}

bool SHA256::operator== (const SHA256& other) const noexcept  { return memcmp (result, other.result, sizeof (result)) == 0; }
bool SHA256::operator!= (const SHA256& other) const noexcept  { return ! operator== (other); }


//==============================================================================
#if JUCE_UNIT_TESTS

class SHA256Tests  : public UnitTest
{
public:
    SHA256Tests() : UnitTest ("SHA-256") {}

    void test (const char* input, const char* expected)
    {
        {
            SHA256 hash (input, strlen (input));
            expectEquals (hash.toHexString(), String (expected));
        }

        {
            CharPointer_UTF8 utf8 (input);
            SHA256 hash (utf8);
            expectEquals (hash.toHexString(), String (expected));
        }

        {
            MemoryInputStream m (input, strlen (input), false);
            SHA256 hash (m);
            expectEquals (hash.toHexString(), String (expected));
        }
    }

    void runTest() override
    {
        beginTest ("SHA256");

        test ("", "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");
        test ("The quick brown fox jumps over the lazy dog",  "d7a8fbb307d7809469ca9abcb0082e4f8d5651e46d3cdb762d02d0bf37c9e592");
        test ("The quick brown fox jumps over the lazy dog.", "ef537f25c895bfa782526529a9b63d97aa631564d5d789c2b765448c8635fb6c");
    }
};

static SHA256Tests sha256UnitTests;

#endif
