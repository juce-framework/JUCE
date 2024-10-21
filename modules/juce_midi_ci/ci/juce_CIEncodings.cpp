/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce::midi_ci
{

String Encodings::stringFrom7BitText (Span<const std::byte> bytes)
{
    std::vector<CharPointer_UTF16::CharType> chars;

    while (! bytes.empty())
    {
        const auto front = (uint8_t) bytes.front();

        if ((front < 0x20 || 0x80 <= front) && front != 0x0a)
        {
            jassertfalse;
            return {};
        }

        if (front == '\\')
        {
            bytes = Span (bytes.data() + 1, bytes.size() - 1);

            if (bytes.empty())
                return {};

            const auto kind = (uint8_t) bytes.front();

            switch (kind)
            {
                case '"':  chars.push_back ('"');  break;
                case '\\': chars.push_back ('\\'); break;
                case '/':  chars.push_back ('/');  break;
                case 'b':  chars.push_back ('\b'); break;
                case 'f':  chars.push_back ('\f'); break;
                case 'n':  chars.push_back ('\n'); break;
                case 'r':  chars.push_back ('\r'); break;
                case 't':  chars.push_back ('\t'); break;

                case 'u':
                {
                    bytes = Span (bytes.data() + 1, bytes.size() - 1);

                    if (bytes.size() < 4)
                        return {};

                    std::string byteStr (reinterpret_cast<const char*> (bytes.data()), 4);
                    const auto unit = [&]() -> std::optional<CharPointer_UTF16::CharType>
                    {
                        try
                        {
                            return (CharPointer_UTF16::CharType) std::stoi (byteStr, {}, 16);
                        }
                        catch (...) {}

                        jassertfalse;
                        return {};
                    }();

                    if (! unit.has_value())
                        return {};

                    chars.push_back (*unit);
                    bytes = Span (bytes.data() + 4, bytes.size() - 4);
                    continue;
                }

                default:
                    return {};
            }

            bytes = Span (bytes.data() + 1, bytes.size() - 1);
        }
        else
        {
            chars.push_back (front);
            bytes = Span (bytes.data() + 1, bytes.size() - 1);
        }
    }

    chars.push_back ({});
    return String { CharPointer_UTF16 { chars.data() } };
}

std::vector<std::byte> Encodings::stringTo7BitText (const String& text)
{
    std::vector<std::byte> result;

    for (const auto character : text)
    {
        if (character == 0x0a || (0x20 <= character && character < 0x80))
        {
            result.emplace_back (std::byte (character));
        }
        else
        {
            // Suspiciously low ASCII value encountered!
            jassert (character >= 0x80);

            CharPointer_UTF16::CharType points[2]{};
            CharPointer_UTF16 asUTF16 { points };
            asUTF16.write (character);

            std::for_each (points, asUTF16.getAddress(), [&] (CharPointer_UTF16::CharType unit)
            {
                const auto str = String::toHexString (unit);

                result.insert (result.end(), { std::byte { '\\' }, std::byte { 'u' } });

                for (const auto c : str)
                    result.push_back ((std::byte) c);
            });
        }
    }

    return result;
}

std::vector<std::byte> Encodings::toMcoded7 (Span<const std::byte> bytes)
{
    std::vector<std::byte> result;
    result.reserve ((bytes.size() * 8) + 6 / 7);

    for (size_t index = 0; index < bytes.size(); index += 7)
    {
        std::array<std::byte, 7> slice{};
        const auto sliceSize = std::min ((size_t) 7, bytes.size() - index);
        std::copy (bytes.begin() + index, bytes.begin() + index + sliceSize, slice.begin());

        result.push_back ((slice[0] & std::byte { 0x80 }) >> 1
                        | (slice[1] & std::byte { 0x80 }) >> 2
                        | (slice[2] & std::byte { 0x80 }) >> 3
                        | (slice[3] & std::byte { 0x80 }) >> 4
                        | (slice[4] & std::byte { 0x80 }) >> 5
                        | (slice[5] & std::byte { 0x80 }) >> 6
                        | (slice[6] & std::byte { 0x80 }) >> 7);
        std::transform (slice.begin(),
                        std::next (slice.begin(), (ptrdiff_t) sliceSize),
                        std::back_inserter (result),
                        [] (const std::byte b) { return b & std::byte { 0x7f }; });
    }

    return result;
}

std::vector<std::byte> Encodings::fromMcoded7 (Span<const std::byte> bytes)
{
    std::vector<std::byte> result;
    result.reserve ((bytes.size() * 7) + 7 / 8);

    for (size_t index = 0; index < bytes.size(); index += 8)
    {
        const auto sliceSize = std::min ((size_t) 7, bytes.size() - index - 1);

        for (size_t i = 0; i < sliceSize; ++i)
        {
            const auto highBit = (bytes[index] & std::byte { (uint8_t) (1 << (6 - i)) }) << (i + 1);
            result.push_back (highBit | bytes[index + 1 + i]);
        }
    }

    return result;
}

std::optional<std::vector<std::byte>> Encodings::tryEncode (Span<const std::byte> bytes, Encoding mutualEncoding)
{
    switch (mutualEncoding)
    {
        case Encoding::ascii:
        {
            if (std::all_of (bytes.begin(), bytes.end(), [] (auto b) { return (b & std::byte { 0x80 }) == std::byte{}; }))
                return std::vector<std::byte> (bytes.begin(), bytes.end());

            jassertfalse;
            return {};
        }

        case Encoding::mcoded7:
            return toMcoded7 (bytes);

        case Encoding::zlibAndMcoded7:
        {
            MemoryOutputStream memoryStream;
            GZIPCompressorOutputStream (memoryStream).write (bytes.data(), bytes.size());
            return toMcoded7 (Span (static_cast<const std::byte*> (memoryStream.getData()), memoryStream.getDataSize()));
        }
    }

    // Unknown encoding!
    jassertfalse;
    return {};
}

std::vector<std::byte> Encodings::decode (Span<const std::byte> bytes, Encoding mutualEncoding)
{
    if (mutualEncoding == Encoding::ascii)
    {
        // All values must be 7-bit!
        jassert (std::none_of (bytes.begin(), bytes.end(), [] (const auto& b) { return (b & std::byte { 0x80 }) != std::byte{}; }));
        return std::vector<std::byte> (bytes.begin(), bytes.end());
    }

    if (mutualEncoding == Encoding::mcoded7)
        return fromMcoded7 (bytes);

    if (mutualEncoding == Encoding::zlibAndMcoded7)
    {
        const auto mcoded = fromMcoded7 (bytes);
        MemoryInputStream memoryStream (mcoded.data(), mcoded.size(), false);

        GZIPDecompressorInputStream zipStream (memoryStream);

        const size_t chunkSize = 1 << 8;

        std::vector<std::byte> result;

        for (;;)
        {
            const auto previousSize = result.size();
            result.resize (previousSize + chunkSize);
            const auto read = zipStream.read (result.data() + previousSize, chunkSize);

            if (read < 0)
            {
                // Decompression failed!
                jassertfalse;
                return {};
            }

            result.resize ((size_t) read + previousSize);

            if (read == 0)
                return result;
        }
    }

    // Unknown encoding!
    jassertfalse;
    return {};
}

#if JUCE_UNIT_TESTS

class EncodingsTests : public UnitTest
{
public:
    EncodingsTests() : UnitTest ("Encodings", UnitTestCategories::midi) {}

    void runTest() override
    {
        beginTest ("7-bit text encoding");
        {
            {
                const auto converted = Encodings::stringTo7BitText (juce::CharPointer_UTF8 ("Accepted Beat \xe2\x99\xaa"));
                const auto expected = makeByteArray ('A', 'c', 'c', 'e', 'p', 't', 'e', 'd', ' ', 'B', 'e', 'a', 't', ' ', '\\', 'u', '2', '6', '6', 'a');
                expect (std::equal (converted.begin(), converted.end(), expected.begin(), expected.end()));
            }

            {
                const auto converted = Encodings::stringTo7BitText (juce::CharPointer_UTF8 ("\xe6\xae\x8b\xe3\x82\x8a\xe3\x82\x8f\xe3\x81\x9a\xe3\x81\x8b""5\xe3\x83\x90\xe3\x82\xa4\xe3\x83\x88"));
                const auto expected = makeByteArray ('\\', 'u', '6', 'b', '8', 'b',
                                                     '\\', 'u', '3', '0', '8', 'a',
                                                     '\\', 'u', '3', '0', '8', 'f',
                                                     '\\', 'u', '3', '0', '5', 'a',
                                                     '\\', 'u', '3', '0', '4', 'b',
                                                     '5',
                                                     '\\', 'u', '3', '0', 'd', '0',
                                                     '\\', 'u', '3', '0', 'a', '4',
                                                     '\\', 'u', '3', '0', 'c', '8');
                expect (std::equal (converted.begin(), converted.end(), expected.begin(), expected.end()));
            }
        }

        beginTest ("7-bit text decoding");
        {
            {
                const auto converted = Encodings::stringFrom7BitText (makeByteArray ('A', 'c', 'c', 'e', 'p', 't', 'e', 'd', ' ', 'B', 'e', 'a', 't', ' ', '\\', 'u', '2', '6', '6', 'a'));
                const String expected = juce::CharPointer_UTF8 ("Accepted Beat \xe2\x99\xaa");
                expect (converted == expected);
            }

            {
                const auto converted = Encodings::stringFrom7BitText (makeByteArray ('\\', 'u', '6', 'b', '8', 'b',
                                                                                     '\\', 'u', '3', '0', '8', 'a',
                                                                                     '\\', 'u', '3', '0', '8', 'f',
                                                                                     '\\', 'u', '3', '0', '5', 'a',
                                                                                     '\\', 'u', '3', '0', '4', 'b',
                                                                                     '5',
                                                                                     '\\', 'u', '3', '0', 'd', '0',
                                                                                     '\\', 'u', '3', '0', 'a', '4',
                                                                                     '\\', 'u', '3', '0', 'c', '8'));
                const String expected = juce::CharPointer_UTF8 ("\xe6\xae\x8b\xe3\x82\x8a\xe3\x82\x8f\xe3\x81\x9a\xe3\x81\x8b""5\xe3\x83\x90\xe3\x82\xa4\xe3\x83\x88");
                expect (converted == expected);
            }
        }

        beginTest ("Mcoded7 encoding");
        {
            {
                const auto converted = Encodings::toMcoded7 (makeByteArray (0x81, 0x82, 0x83));
                const auto expected = makeByteArray (0x70, 0x01, 0x02, 0x03);
                expect (rangesEqual (converted, expected));
            }

            {
                const auto converted = Encodings::toMcoded7 (makeByteArray (0x01, 0x82, 0x03, 0x04, 0x85, 0x06, 0x87, 0x08));
                const auto expected = makeByteArray (0x25, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x00, 0x08);
                expect (rangesEqual (converted, expected));
            }
        }

        beginTest ("Mcoded7 decoding");
        {
            {
                const auto converted = Encodings::fromMcoded7 (makeByteArray (0x70, 0x01, 0x02, 0x03));
                const auto expected = makeByteArray (0x81, 0x82, 0x83);
                expect (rangesEqual (converted, expected));
            }

            {
                const auto converted = Encodings::fromMcoded7 (makeByteArray (0x25, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x00, 0x08));
                const auto expected = makeByteArray (0x01, 0x82, 0x03, 0x04, 0x85, 0x06, 0x87, 0x08);
                expect (rangesEqual (converted, expected));
            }
        }
    }

private:
    static bool deepEqual (const std::optional<var>& a, const std::optional<var>& b)
    {
        if (a.has_value() && b.has_value())
            return JSONUtils::deepEqual (*a, *b);

        return a == b;
    }

    template <typename A, typename B>
    static bool rangesEqual (A&& a, B&& b)
    {
        using std::begin, std::end;
        return std::equal (begin (a), end (a), begin (b), end (b));
    }

    template <typename... Ts>
    static std::array<std::byte, sizeof... (Ts)> makeByteArray (Ts&&... ts)
    {
        jassert (((0 <= (int) ts && (int) ts <= std::numeric_limits<uint8_t>::max()) && ...));
        return { std::byte (ts)... };
    }
};

static EncodingsTests encodingsTests;

#endif

} // namespace juce::midi_ci
