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
namespace build_tools
{
    static bool isReservedKeyword (String::CharPointerType token, const int tokenLength) noexcept
    {
        static const char* const keywords2Char[] =
                { "do", "if", "or", nullptr };

        static const char* const keywords3Char[] =
                { "and", "asm", "for", "int", "new", "not", "try", "xor", nullptr };

        static const char* const keywords4Char[] =
                { "auto", "bool", "case", "char", "else", "enum", "goto",
                  "long", "this", "true", "void", nullptr };

        static const char* const keywords5Char[] =
                { "bitor", "break", "catch", "class", "compl", "const", "false", "final",
                  "float", "or_eq", "short", "throw", "union", "using", "while", nullptr };

        static const char* const keywords6Char[] =
                { "and_eq", "bitand", "delete", "double", "export", "extern", "friend",
                  "import", "inline", "module", "not_eq", "public", "return", "signed",
                  "sizeof", "static", "struct", "switch", "typeid", "xor_eq", nullptr };

        static const char* const keywords7Char[] =
                { "__cdecl", "_Pragma", "alignas", "alignof", "concept", "default",
                  "mutable", "nullptr", "private", "typedef", "uint8_t", "virtual",
                  "wchar_t", nullptr };

        static const char* const keywordsOther[] =
                { "@class", "@dynamic", "@end", "@implementation", "@interface", "@public",
                  "@private", "@protected", "@property", "@synthesize", "__fastcall", "__stdcall",
                  "atomic_cancel", "atomic_commit", "atomic_noexcept", "char16_t", "char32_t",
                  "co_await", "co_return", "co_yield", "const_cast", "constexpr", "continue",
                  "decltype", "dynamic_cast", "explicit", "namespace", "noexcept", "operator", "override",
                  "protected", "register", "reinterpret_cast", "requires", "static_assert",
                  "static_cast", "synchronized", "template", "thread_local", "typename", "unsigned",
                  "volatile", nullptr };

        const char* const* k;

        switch (tokenLength)
        {
            case 2:     k = keywords2Char; break;
            case 3:     k = keywords3Char; break;
            case 4:     k = keywords4Char; break;
            case 5:     k = keywords5Char; break;
            case 6:     k = keywords6Char; break;
            case 7:     k = keywords7Char; break;

            default:
                if (tokenLength < 2 || tokenLength > 16)
                    return false;

                k = keywordsOther;
                break;
        }

        for (int i = 0; k[i] != nullptr; ++i)
            if (token.compare (CharPointer_ASCII (k[i])) == 0)
                return true;

        return false;
    }

    static bool isReservedKeyword (const String& token) noexcept
    {
        return isReservedKeyword (token.getCharPointer(), token.length());
    }

    //==============================================================================
    /** Takes a UTF8 string and writes it to a stream using standard C++ escape sequences for any
        non-ascii bytes.

        Although not strictly a tokenising function, this is still a function that often comes in
        handy when working with C++ code!

        Note that addEscapeChars() is easier to use than this function if you're working with Strings.

        @see addEscapeChars
    */
    static void writeEscapeChars (OutputStream& out, const char* utf8, const int numBytesToRead,
                                  const int maxCharsOnLine, const bool breakAtNewLines,
                                  const bool replaceSingleQuotes, const bool allowStringBreaks)
    {
        int charsOnLine = 0;
        bool lastWasHexEscapeCode = false;
        bool trigraphDetected = false;

        for (int i = 0; i < numBytesToRead || numBytesToRead < 0; ++i)
        {
            auto c = (unsigned char) utf8[i];
            bool startNewLine = false;

            switch (c)
            {

                case '\t':  out << "\\t";  trigraphDetected = false; lastWasHexEscapeCode = false; charsOnLine += 2; break;
                case '\r':  out << "\\r";  trigraphDetected = false; lastWasHexEscapeCode = false; charsOnLine += 2; break;
                case '\n':  out << "\\n";  trigraphDetected = false; lastWasHexEscapeCode = false; charsOnLine += 2; startNewLine = breakAtNewLines; break;
                case '\\':  out << "\\\\"; trigraphDetected = false; lastWasHexEscapeCode = false; charsOnLine += 2; break;
                case '\"':  out << "\\\""; trigraphDetected = false; lastWasHexEscapeCode = false; charsOnLine += 2; break;

                case '?':
                    if (trigraphDetected)
                    {
                        out << "\\?";
                        charsOnLine++;
                        trigraphDetected = false;
                    }
                    else
                    {
                        out << "?";
                        trigraphDetected = true;
                    }

                    lastWasHexEscapeCode = false;
                    charsOnLine++;
                    break;

                case 0:
                    if (numBytesToRead < 0)
                        return;

                    out << "\\0";
                    lastWasHexEscapeCode = true;
                    trigraphDetected = false;
                    charsOnLine += 2;
                    break;

                case '\'':
                    if (replaceSingleQuotes)
                    {
                        out << "\\\'";
                        lastWasHexEscapeCode = false;
                        trigraphDetected = false;
                        charsOnLine += 2;
                        break;
                    }

                    // deliberate fall-through...

                default:
                    if (c >= 32 && c < 127 && ! (lastWasHexEscapeCode  // (have to avoid following a hex escape sequence with a valid hex digit)
                                                 && CharacterFunctions::getHexDigitValue (c) >= 0))
                    {
                        out << (char) c;
                        lastWasHexEscapeCode = false;
                        trigraphDetected = false;
                        ++charsOnLine;
                    }
                    else if (allowStringBreaks && lastWasHexEscapeCode && c >= 32 && c < 127)
                    {
                        out << "\"\"" << (char) c;
                        lastWasHexEscapeCode = false;
                        trigraphDetected = false;
                        charsOnLine += 3;
                    }
                    else
                    {
                        out << (c < 16 ? "\\x0" : "\\x") << String::toHexString ((int) c);
                        lastWasHexEscapeCode = true;
                        trigraphDetected = false;
                        charsOnLine += 4;
                    }

                    break;
            }

            if ((startNewLine || (maxCharsOnLine > 0 && charsOnLine >= maxCharsOnLine))
                && (numBytesToRead < 0 || i < numBytesToRead - 1))
            {
                charsOnLine = 0;
                out << "\"" << newLine << "\"";
                lastWasHexEscapeCode = false;
            }
        }
    }
}
}
