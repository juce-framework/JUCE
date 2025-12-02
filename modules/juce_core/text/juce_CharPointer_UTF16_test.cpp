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

namespace juce
{

class CharPointer_UTF16Test final : public UnitTest
{
public:
    CharPointer_UTF16Test() : UnitTest { "CharPointer_UTF16Test", UnitTestCategories::text } {}

    void runTest() final
    {
        const auto toCharType = [] (const std::vector<char16_t>& str)
        {
            return reinterpret_cast<const CharPointer_UTF16::CharType*> (str.data());
        };

        const auto getNumBytes = [] (const auto& str)
        {
            return (int) (sizeof (CharPointer_UTF16::CharType) * str.size());
        };

        beginTest ("String validation - empty string / null-terminator");
        {
            const std::vector<CharPointer_UTF16::CharType> string { 0x0 };
            expect (CharPointer_UTF16::isValidString (string.data(), getNumBytes (string)));
        }

        beginTest ("String validation - ascii");
        {
            const std::vector<char16_t> string { 0x54, 0x65, 0x73, 0x74, 0x21, 0x0 }; // Test!
            expect (CharPointer_UTF16::isValidString (toCharType (string), getNumBytes (string)));
        }

        beginTest ("String validation - 2-byte code points");
        {
            const std::vector<char16_t> string { 0x54, 0x65, 0x73, 0x74, 0x20ac, 0x0 }; // Test€
            expect (CharPointer_UTF16::isValidString (toCharType (string), getNumBytes (string)));
        }

        beginTest ("String validation - surrogate pairs");
        {
            const std::vector<char16_t> string { 0x54, 0x65, 0x73, 0x74, 0xd83d, 0xde03, 0x0 }; // Test😃
            expect (CharPointer_UTF16::isValidString (toCharType (string), getNumBytes (string)));
        }

        beginTest ("String validation - high-surrogate without a low-surrogate");
        {
            const std::vector<char16_t> string { 0x54, 0x65, 0x73, 0x74, 0xd83d, 0x0 };
            expect (! CharPointer_UTF16::isValidString (toCharType (string), getNumBytes (string)));
        }

        beginTest ("String validation - low-surrogate without a high-surrogate");
        {
            const std::vector<char16_t> string { 0x54, 0x65, 0x73, 0x74, 0xde03, 0x0 };
            expect (! CharPointer_UTF16::isValidString (toCharType (string), getNumBytes (string)));
        }

        beginTest ("String validation - characters after a null terminator are ignored");
        {
            const std::vector<char16_t> string { 0x54, 0x65, 0x73, 0x74, 0x0, 0xde03 };
            expect (CharPointer_UTF16::isValidString (toCharType (string), getNumBytes (string)));
        }

        beginTest ("String validation - characters exceeding max bytes are ignored");
        {
            const std::vector<char16_t> string { 0x54, 0x65, 0x73, 0x74, 0xde03 };
            expect (CharPointer_UTF16::isValidString (toCharType (string), 8));
        }

        beginTest ("String validation - all unicode characters");
        {
            for (uint32_t c = 0; c < 0x110000; ++c)
            {
                std::array<CharPointer_UTF16::CharType, 2> string = {};
                CharPointer_UTF16 utf16 { string.data() };
                utf16.write ((juce_wchar) c);
                expect (CharPointer_UTF16::isValidString (string.data(), 4) == CharPointer_UTF32::canRepresent ((juce_wchar) c));
            }
        }

        beginTest ("Iterating string starting with unpaired high surrogate produces a wide character with the surrogate value");
        {
            const std::vector<char16_t> stringA { 0xd800, 0xa, 0xb };
            expect (rangesEqual (Span (stringA), Span (stringA)));

            const std::vector<char16_t> stringB { 0xd800, 0xe000, 0xb };
            expect (rangesEqual (Span (stringB), Span (stringB)));
        }

        beginTest ("Iterating string ending with unpaired high surrogate produces a wide character with the surrogate value");
        {
            const std::vector<char16_t> string { 0xa, 0xb, 0xd800, 0x0 };
            expect (rangesEqual (Span (string), Span (string)));
        }

        beginTest ("Iterating string starting with unpaired low surrogate produces a wide character with the surrogate value");
        {
            const std::vector<char16_t> stringA { 0xdc00, 0xa, 0xb };
            expect (rangesEqual (Span (stringA), Span (stringA)));

            const std::vector<char16_t> stringB { 0xdc00, 0xe000, 0xb };
            expect (rangesEqual (Span (stringB), Span (stringB)));
        }

        beginTest ("Iterating string ending with unpaired low surrogate produces a wide character with the surrogate value");
        {
            const std::vector<char16_t> string { 0xa, 0xb, 0xdc00 };
            expect (rangesEqual (Span (string), Span (string)));
        }

        beginTest ("Iterating string with multiple unpaired surrogates produces produces wide characters with those surrogate values");
        {
            const std::vector<char16_t> string { 0xd800, 0xd800, 0xdc00, 0xdc00, 0xa, 0xb };
            const std::vector<juce_wchar> expected { 0xd800, 0x10000, 0xdc00, 0xa, 0xb };
            expect (rangesEqual (Span (expected), Span (string)));
        }

        beginTest ("Can decrement to unpaired low surrogate");
        {
            const CharPointer_UTF16::CharType chars[] { 0xa, (CharPointer_UTF16::CharType) 0xdc00, 0xb};
            CharPointer_UTF16 ptr { chars + 2 };

            expect (*ptr == 0xb);
            --ptr;
            expect (ptr == CharPointer_UTF16 { chars + 1 });
            expect (*ptr == 0xdc00);
        }

        beginTest ("Can decrement to unpaired high surrogate");
        {
            const CharPointer_UTF16::CharType chars[] { 0xa, (CharPointer_UTF16::CharType) 0xd800, 0xb };
            CharPointer_UTF16 ptr { chars + 2 };

            expect (*ptr == 0xb);
            --ptr;
            expect (ptr == CharPointer_UTF16 { chars + 1 });
            expect (*ptr == 0xd800);
        }

        beginTest ("Can decrement through surrogate pair");
        {
            const CharPointer_UTF16::CharType chars[] { 0xa,
                                                        (CharPointer_UTF16::CharType) 0xd800,
                                                        (CharPointer_UTF16::CharType) 0xdc00,
                                                        0xb };
            CharPointer_UTF16 ptr { chars + 3 };

            expect (*ptr == 0xb);

            --ptr;
            expect (ptr == CharPointer_UTF16 { chars + 1 });
            expect (*ptr == 0x10000);

            --ptr;
            expect (ptr == CharPointer_UTF16 { chars });
            expect (*ptr == (CharPointer_UTF16::CharType) 0xa);
        }
    }

private:
    template <typename Expected>
    static bool rangesEqual (Span<const Expected> expected, Span<const char16_t> units)
    {
        const auto dataPtr = reinterpret_cast<const CharPointer_UTF16::CharType*> (units.data());
        std::vector<juce_wchar> converted;

        for (const auto it : makeRange (CharPointer_UTF16 { dataPtr },
                                        CharPointer_UTF16 { dataPtr + units.size() }))
        {
            converted.push_back (it);
        }

        // Some stdlibs require the arguments to std::equal to have the full complement of iterator
        // member type aliases, so compare using std::vector iterators instead of CharPointer_UTF16
        // directly.
        return std::equal (expected.begin(), expected.end(), converted.begin(), converted.end());
    }
};

static CharPointer_UTF16Test charPointer_UTF16Test;

} // namespace juce
