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

class CharPointer_UTF8Test final : public UnitTest
{
public:
    CharPointer_UTF8Test() : UnitTest { "CharPointer_UTF8", UnitTestCategories::text } {}

    void runTest() final
    {
        beginTest ("String validation - empty string / null-terminator");
        {
            const std::vector<CharPointer_UTF8::CharType> string { '\0' };
            expect (CharPointer_UTF8::isValidString (string.data(), (int) string.size()));
        }

        beginTest ("String validation - ascii");
        {
            const std::vector<CharPointer_UTF8::CharType> string { 'T', 'e', 's', 'T', '!', '\0' }; // Test!
            expect (CharPointer_UTF8::isValidString (string.data(), (int) string.size()));
        }

        constexpr auto continuationCharacter = static_cast<char> (0x80);

        beginTest ("String validation - continuation characters are invalid when not proceeded by the correct bytes");
        {
            const std::vector<CharPointer_UTF8::CharType> string { continuationCharacter };
            expect (! CharPointer_UTF8::isValidString (string.data(), (int) string.size()));
        }

        beginTest ("String validation - characters after a null terminator are ignored");
        {
            const std::vector<CharPointer_UTF8::CharType> string { 'T', 'e', 's', 'T', '\0', continuationCharacter };
            expect (CharPointer_UTF8::isValidString (string.data(), (int) string.size()));
        }

        beginTest ("String validation - characters exceeding max bytes are ignored");
        {
            const std::vector<CharPointer_UTF8::CharType> string { 'T', 'e', 's', 'T', continuationCharacter };
            expect (CharPointer_UTF8::isValidString (string.data(), 4));
        }

        beginTest ("String validation - all unicode characters");
        {
            for (uint32_t c = 0; c < 0x110000; ++c)
            {
                std::array<CharPointer_UTF8::CharType, 4> string = {};
                CharPointer_UTF8 utf8 { string.data() };
                utf8.write ((juce_wchar) c);
                expect (CharPointer_UTF8::isValidString (string.data(), (int) string.size()) == CharPointer_UTF32::canRepresent ((juce_wchar) c));
            }
        }
    }
};


static CharPointer_UTF8Test charPointer_UTF8Test;

} // namespace juce
