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

struct SHA256Processor
{
    // expects 64 bytes of data
    void processFullBlock (const void* data) noexcept
    {
        const uint32_t constants[] =
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

        uint32_t block[16], s[8];
        memcpy (s, state, sizeof (s));

        auto d = static_cast<const uint8_t*> (data);

        for (auto& b : block)
        {
            b = (uint32_t (d[0]) << 24) | (uint32_t (d[1]) << 16) | (uint32_t (d[2]) << 8) | d[3];
            d += 4;
        }

        auto convolve = [&] (uint32_t i, uint32_t j)
        {
            s[(7 - i) & 7] += S1 (s[(4 - i) & 7]) + ch (s[(4 - i) & 7], s[(5 - i) & 7], s[(6 - i) & 7]) + constants[i + j]
                                 + (j != 0 ? (block[i & 15] += s1 (block[(i - 2) & 15]) + block[(i - 7) & 15] + s0 (block[(i - 15) & 15]))
                                           : block[i]);
            s[(3 - i) & 7] += s[(7 - i) & 7];
            s[(7 - i) & 7] += S0 (s[(0 - i) & 7]) + maj (s[(0 - i) & 7], s[(1 - i) & 7], s[(2 - i) & 7]);
        };

        for (uint32_t j = 0; j < 64; j += 16)
            for (uint32_t i = 0; i < 16; ++i)
                convolve (i, j);

        for (int i = 0; i < 8; ++i)
            state[i] += s[i];

        length += 64;
    }

    void processFinalBlock (const void* data, uint32_t numBytes) noexcept
    {
        jassert (numBytes < 64);

        length += numBytes;
        length *= 8; // (the length is stored as a count of bits, not bytes)

        uint8_t finalBlocks[128];

        memcpy (finalBlocks, data, numBytes);
        finalBlocks[numBytes++] = 128; // append a '1' bit

        while (numBytes != 56 && numBytes < 64 + 56)
            finalBlocks[numBytes++] = 0; // pad with zeros..

        for (int i = 8; --i >= 0;)
            finalBlocks[numBytes++] = (uint8_t) (length >> (i * 8)); // append the length.

        jassert (numBytes == 64 || numBytes == 128);

        processFullBlock (finalBlocks);

        if (numBytes > 64)
            processFullBlock (finalBlocks + 64);
    }

    void copyResult (uint8_t* result) const noexcept
    {
        for (auto s : state)
        {
            *result++ = (uint8_t) (s >> 24);
            *result++ = (uint8_t) (s >> 16);
            *result++ = (uint8_t) (s >> 8);
            *result++ = (uint8_t) s;
        }
    }

    void processStream (InputStream& input, int64_t numBytesToRead, uint8_t* result)
    {
        if (numBytesToRead < 0)
            numBytesToRead = std::numeric_limits<int64_t>::max();

        for (;;)
        {
            uint8_t buffer[64];
            auto bytesRead = input.read (buffer, (int) jmin (numBytesToRead, (int64_t) sizeof (buffer)));

            if (bytesRead < (int) sizeof (buffer))
            {
                processFinalBlock (buffer, (unsigned int) bytesRead);
                break;
            }

            numBytesToRead -= (int64_t) sizeof (buffer);
            processFullBlock (buffer);
        }

        copyResult (result);
    }

private:
    uint32_t state[8] = { 0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
                          0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19 };
    uint64_t length = 0;

    static uint32_t rotate (uint32_t x, uint32_t y) noexcept            { return (x >> y) | (x << (32 - y)); }
    static uint32_t ch  (uint32_t x, uint32_t y, uint32_t z) noexcept   { return z ^ ((y ^ z) & x); }
    static uint32_t maj (uint32_t x, uint32_t y, uint32_t z) noexcept   { return y ^ ((y ^ z) & (x ^ y)); }

    static uint32_t s0 (uint32_t x) noexcept     { return rotate (x, 7)  ^ rotate (x, 18) ^ (x >> 3); }
    static uint32_t s1 (uint32_t x) noexcept     { return rotate (x, 17) ^ rotate (x, 19) ^ (x >> 10); }
    static uint32_t S0 (uint32_t x) noexcept     { return rotate (x, 2)  ^ rotate (x, 13) ^ rotate (x, 22); }
    static uint32_t S1 (uint32_t x) noexcept     { return rotate (x, 6)  ^ rotate (x, 11) ^ rotate (x, 25); }
};

//==============================================================================
SHA256::SHA256() = default;
SHA256::~SHA256() = default;
SHA256::SHA256 (const SHA256&) = default;
SHA256& SHA256::operator= (const SHA256&) = default;

SHA256::SHA256 (const MemoryBlock& data)
{
    process (data.getData(), data.getSize());
}

SHA256::SHA256 (const void* data, size_t numBytes)
{
    process (data, numBytes);
}

SHA256::SHA256 (InputStream& input, int64 numBytesToRead)
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

void SHA256::process (const void* data, size_t numBytes)
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

class SHA256Tests final : public UnitTest
{
public:
    SHA256Tests()
        : UnitTest ("SHA-256", UnitTestCategories::cryptography)
    {}

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

} // namespace juce
