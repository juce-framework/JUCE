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

class CharPointer_UTF32Test final : public UnitTest
{
public:
    CharPointer_UTF32Test() : UnitTest { "CharPointer_UTF32", UnitTestCategories::text } {}

    void runTest() final
    {
        const auto toCharType = [] (const std::vector<char32_t>& str)
        {
            return reinterpret_cast<const CharPointer_UTF32::CharType*> (str.data());
        };

        const auto getNumBytes = [] (const auto& str)
        {
            return (int) (sizeof (CharPointer_UTF32::CharType) * str.size());
        };

        beginTest ("String validation - empty string / null-terminator");
        {
            const std::vector<CharPointer_UTF32::CharType> string { 0x0 };
            expect (CharPointer_UTF32::isValidString (string.data(), getNumBytes (string)));
        }

        beginTest ("String validation - ascii");
        {
            const std::vector<char32_t> string { 0x54, 0x65, 0x73, 0x74, 0x21, 0x0 }; // Test!
            expect (CharPointer_UTF32::isValidString (toCharType (string), getNumBytes (string)));
        }

        beginTest ("String validation - 2-byte code points");
        {
            const std::vector<char32_t> string { 0x54, 0x65, 0x73, 0x74, 0x20ac, 0x0 }; // Test€
            expect (CharPointer_UTF32::isValidString (toCharType (string), getNumBytes (string)));
        }

        beginTest ("String validation - maximum code point");
        {
            const std::vector<char32_t> string1 { 0x54, 0x65, 0x73, 0x74, 0x10ffff, 0x0 };
            expect (CharPointer_UTF32::isValidString (toCharType (string1), getNumBytes (string1)));

            const std::vector<char32_t> string2 { 0x54, 0x65, 0x73, 0x74, 0x110000, 0x0 };
            expect (! CharPointer_UTF32::isValidString (toCharType (string2), getNumBytes (string2)));
        }

        beginTest ("String validation - characters after a null terminator are ignored");
        {
            const std::vector<char32_t> string { 0x54, 0x65, 0x73, 0x74, 0x0, 0x110000 };
            expect (CharPointer_UTF32::isValidString (toCharType (string), getNumBytes (string)));
        }

        beginTest ("String validation - characters exceeding max bytes are ignored");
        {
            const std::vector<char32_t> string { 0x54, 0x65, 0x73, 0x74, 0x110000 };
            expect (CharPointer_UTF32::isValidString (toCharType (string), 8));
        }

        beginTest ("String validation - surrogate code points are invalid");
        {
            const std::vector<char32_t> highSurrogate { 0xd800 };
            expect (! CharPointer_UTF32::isValidString (toCharType (highSurrogate), getNumBytes (highSurrogate)));

            const std::vector<char32_t> lowSurrogate { 0xdfff };
            expect (! CharPointer_UTF32::isValidString (toCharType (lowSurrogate), getNumBytes (lowSurrogate)));
        }
    }
};


static CharPointer_UTF32Test charPointer_UTF32Test;

} // namespace juce
