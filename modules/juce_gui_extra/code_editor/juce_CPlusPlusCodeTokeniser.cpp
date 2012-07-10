/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

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


namespace CppTokeniser
{
    enum TokenType
    {
        tokenType_error = 0,
        tokenType_comment,
        tokenType_keyword,
        tokenType_operator,
        tokenType_identifier,
        tokenType_integer,
        tokenType_float,
        tokenType_string,
        tokenType_bracket,
        tokenType_punctuation,
        tokenType_preprocessor
    };

    static bool isIdentifierStart (const juce_wchar c) noexcept
    {
        return CharacterFunctions::isLetter (c)
                || c == '_' || c == '@';
    }

    static bool isIdentifierBody (const juce_wchar c) noexcept
    {
        return CharacterFunctions::isLetterOrDigit (c)
                || c == '_' || c == '@';
    }

    static bool isReservedKeyword (String::CharPointerType token, const int tokenLength) noexcept
    {
        static const char* const keywords2Char[] =
            { "if", "do", "or", "id", 0 };

        static const char* const keywords3Char[] =
            { "for", "int", "new", "try", "xor", "and", "asm", "not", 0 };

        static const char* const keywords4Char[] =
            { "bool", "void", "this", "true", "long", "else", "char",
              "enum", "case", "goto", "auto", 0 };

        static const char* const keywords5Char[] =
            {  "while", "bitor", "break", "catch", "class", "compl", "const", "false",
                "float", "short", "throw", "union", "using", "or_eq", 0 };

        static const char* const keywords6Char[] =
            { "return", "struct", "and_eq", "bitand", "delete", "double", "extern",
              "friend", "inline", "not_eq", "public", "sizeof", "static", "signed",
              "switch", "typeid", "wchar_t", "xor_eq", 0};

        static const char* const keywords7Char[] =
            { "default", "mutable", "private", "typedef", "nullptr", "virtual", 0 };

        static const char* const keywordsOther[] =
            { "noexcept", "const_cast", "continue", "explicit", "namespace",
              "operator", "protected", "register", "reinterpret_cast", "static_cast",
              "template", "typename", "unsigned", "volatile", "constexpr",
              "@implementation", "@interface", "@end", "@synthesize", "@dynamic", "@public",
              "@private", "@property", "@protected", "@class", 0 };

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

        for (int i = 0; k[i] != 0; ++i)
            if (token.compare (CharPointer_ASCII (k[i])) == 0)
                return true;

        return false;
    }

    static int parseIdentifier (CodeDocument::Iterator& source) noexcept
    {
        int tokenLength = 0;
        String::CharPointerType::CharType possibleIdentifier [100];
        String::CharPointerType possible (possibleIdentifier);

        while (isIdentifierBody (source.peekNextChar()))
        {
            const juce_wchar c = source.nextChar();

            if (tokenLength < 20)
                possible.write (c);

            ++tokenLength;
        }

        if (tokenLength > 1 && tokenLength <= 16)
        {
            possible.writeNull();

            if (isReservedKeyword (String::CharPointerType (possibleIdentifier), tokenLength))
                return tokenType_keyword;
        }

        return tokenType_identifier;
    }

    static bool skipNumberSuffix (CodeDocument::Iterator& source)
    {
        const juce_wchar c = source.peekNextChar();
        if (c == 'l' || c == 'L' || c == 'u' || c == 'U')
            source.skip();

        if (CharacterFunctions::isLetterOrDigit (source.peekNextChar()))
            return false;

        return true;
    }

    static bool isHexDigit (const juce_wchar c) noexcept
    {
        return (c >= '0' && c <= '9')
                || (c >= 'a' && c <= 'f')
                || (c >= 'A' && c <= 'F');
    }

    static bool parseHexLiteral (CodeDocument::Iterator& source) noexcept
    {
        if (source.nextChar() != '0')
            return false;

        juce_wchar c = source.nextChar();
        if (c != 'x' && c != 'X')
            return false;

        int numDigits = 0;
        while (isHexDigit (source.peekNextChar()))
        {
            ++numDigits;
            source.skip();
        }

        if (numDigits == 0)
            return false;

        return skipNumberSuffix (source);
    }

    static bool isOctalDigit (const juce_wchar c) noexcept
    {
        return c >= '0' && c <= '7';
    }

    static bool parseOctalLiteral (CodeDocument::Iterator& source) noexcept
    {
        if (source.nextChar() != '0')
            return false;

        if (! isOctalDigit (source.nextChar()))
             return false;

        while (isOctalDigit (source.peekNextChar()))
            source.skip();

        return skipNumberSuffix (source);
    }

    static bool isDecimalDigit (const juce_wchar c) noexcept
    {
        return c >= '0' && c <= '9';
    }

    static bool parseDecimalLiteral (CodeDocument::Iterator& source) noexcept
    {
        int numChars = 0;
        while (isDecimalDigit (source.peekNextChar()))
        {
            ++numChars;
            source.skip();
        }

        if (numChars == 0)
            return false;

        return skipNumberSuffix (source);
    }

    static bool parseFloatLiteral (CodeDocument::Iterator& source) noexcept
    {
        int numDigits = 0;

        while (isDecimalDigit (source.peekNextChar()))
        {
            source.skip();
            ++numDigits;
        }

        const bool hasPoint = (source.peekNextChar() == '.');

        if (hasPoint)
        {
            source.skip();

            while (isDecimalDigit (source.peekNextChar()))
            {
                source.skip();
                ++numDigits;
            }
        }

        if (numDigits == 0)
            return false;

        juce_wchar c = source.peekNextChar();
        const bool hasExponent = (c == 'e' || c == 'E');

        if (hasExponent)
        {
            source.skip();

            c = source.peekNextChar();
            if (c == '+' || c == '-')
                source.skip();

            int numExpDigits = 0;
            while (isDecimalDigit (source.peekNextChar()))
            {
                source.skip();
                ++numExpDigits;
            }

            if (numExpDigits == 0)
                return false;
        }

        c = source.peekNextChar();
        if (c == 'f' || c == 'F')
            source.skip();
        else if (! (hasExponent || hasPoint))
            return false;

        return true;
    }

    static int parseNumber (CodeDocument::Iterator& source)
    {
        const CodeDocument::Iterator original (source);

        if (parseFloatLiteral (source))
            return tokenType_float;

        source = original;

        if (parseHexLiteral (source))
            return tokenType_integer;

        source = original;

        if (parseOctalLiteral (source))
            return tokenType_integer;

        source = original;

        if (parseDecimalLiteral (source))
            return tokenType_integer;

        source = original;
        source.skip();

        return tokenType_error;
    }

    static void skipQuotedString (CodeDocument::Iterator& source) noexcept
    {
        const juce_wchar quote = source.nextChar();

        for (;;)
        {
            const juce_wchar c = source.nextChar();

            if (c == quote || c == 0)
                break;

            if (c == '\\')
                source.skip();
        }
    }

    static void skipComment (CodeDocument::Iterator& source) noexcept
    {
        bool lastWasStar = false;

        for (;;)
        {
            const juce_wchar c = source.nextChar();

            if (c == 0 || (c == '/' && lastWasStar))
                break;

            lastWasStar = (c == '*');
        }
    }

    static void skipPreprocessorLine (CodeDocument::Iterator& source) noexcept
    {
        bool lastWasBackslash = false;

        for (;;)
        {
            const juce_wchar c = source.peekNextChar();

            if (c == '"')
            {
                skipQuotedString (source);
                continue;
            }

            if (c == '/')
            {
                CodeDocument::Iterator next (source);
                next.skip();
                const juce_wchar c2 = next.peekNextChar();

                if (c2 == '/' || c2 == '*')
                    return;
            }

            if (c == 0)
                break;

            if (c == '\n' || c == '\r')
            {
                source.skipToEndOfLine();

                if (lastWasBackslash)
                    skipPreprocessorLine (source);

                break;
            }

            lastWasBackslash = (c == '\\');
            source.skip();
        }
    }

    static void skipIfNextCharMatches (CodeDocument::Iterator& source, const juce_wchar c) noexcept
    {
        if (source.peekNextChar() == c)
            source.skip();
    }

    static void skipIfNextCharMatches (CodeDocument::Iterator& source,
                                       const juce_wchar c1, const juce_wchar c2) noexcept
    {
        const juce_wchar c = source.peekNextChar();

        if (c == c1 || c == c2)
            source.skip();
    }
}

//==============================================================================
CPlusPlusCodeTokeniser::CPlusPlusCodeTokeniser() {}
CPlusPlusCodeTokeniser::~CPlusPlusCodeTokeniser() {}

int CPlusPlusCodeTokeniser::readNextToken (CodeDocument::Iterator& source)
{
    using namespace CppTokeniser;
    int result = tokenType_error;
    source.skipWhitespace();

    const juce_wchar firstChar = source.peekNextChar();

    switch (firstChar)
    {
    case 0:
        source.skip();
        break;

    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
        result = parseNumber (source);
        break;

    case '.':
        result = parseNumber (source);

        if (result == tokenType_error)
            result = tokenType_punctuation;

        break;

    case ',':
    case ';':
    case ':':
        source.skip();
        result = tokenType_punctuation;
        break;

    case '(':
    case ')':
    case '{':
    case '}':
    case '[':
    case ']':
        source.skip();
        result = tokenType_bracket;
        break;

    case '"':
    case '\'':
        skipQuotedString (source);
        result = tokenType_string;
        break;

    case '+':
        result = tokenType_operator;
        source.skip();
        skipIfNextCharMatches (source, '+', '=');
        break;

    case '-':
        source.skip();
        result = parseNumber (source);

        if (result == tokenType_error)
        {
            result = tokenType_operator;
            skipIfNextCharMatches (source, '-', '=');
        }
        break;

    case '*':
    case '%':
    case '=':
    case '!':
        result = tokenType_operator;
        source.skip();
        skipIfNextCharMatches (source, '=');
        break;

    case '/':
        result = tokenType_operator;
        source.skip();

        if (source.peekNextChar() == '=')
        {
            source.skip();
        }
        else if (source.peekNextChar() == '/')
        {
            result = tokenType_comment;
            source.skipToEndOfLine();
        }
        else if (source.peekNextChar() == '*')
        {
            source.skip();
            result = tokenType_comment;
            skipComment (source);
        }

        break;

    case '?':
    case '~':
        source.skip();
        result = tokenType_operator;
        break;

    case '<':
    case '>':
    case '|':
    case '&':
    case '^':
        source.skip();
        result = tokenType_operator;
        skipIfNextCharMatches (source, firstChar);
        skipIfNextCharMatches (source, '=');
        break;

    case '#':
        result = tokenType_preprocessor;
        skipPreprocessorLine (source);
        break;

    default:
        if (isIdentifierStart (firstChar))
            result = parseIdentifier (source);
        else
            source.skip();

        break;
    }

    return result;
}

CodeEditorComponent::ColourScheme CPlusPlusCodeTokeniser::getDefaultColourScheme()
{
    struct Type
    {
        const char* name;
        uint32 colour;
    };

    const Type types[] =
    {
        { "Error",              0xffcc0000 },
        { "Comment",            0xff00aa00 },
        { "Keyword",            0xff0000cc },
        { "Operator",           0xff225500 },
        { "Identifier",         0xff000000 },
        { "Integer",            0xff880000 },
        { "Float",              0xff885500 },
        { "String",             0xff990099 },
        { "Bracket",            0xff000055 },
        { "Punctuation",        0xff004400 },
        { "Preprocessor Text",  0xff660000 }
    };

    CodeEditorComponent::ColourScheme cs;

    for (int i = 0; i < sizeof (types) / sizeof (types[0]); ++i)  // (NB: numElementsInArray doesn't work here in GCC4.2)
        cs.set (types[i].name, Colour (types[i].colour));

    return cs;
}

bool CPlusPlusCodeTokeniser::isReservedKeyword (const String& token) noexcept
{
    return CppTokeniser::isReservedKeyword (token.getCharPointer(), token.length());
}
