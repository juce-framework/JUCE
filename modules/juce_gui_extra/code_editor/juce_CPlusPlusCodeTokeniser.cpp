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

CPlusPlusCodeTokeniser::CPlusPlusCodeTokeniser() {}
CPlusPlusCodeTokeniser::~CPlusPlusCodeTokeniser() {}

//==============================================================================
namespace CppTokeniser
{
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

        int i = 0;
        while (k[i] != 0)
        {
            if (token.compare (CharPointer_ASCII (k[i])) == 0)
                return true;

            ++i;
        }

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
                return CPlusPlusCodeTokeniser::tokenType_builtInKeyword;
        }

        return CPlusPlusCodeTokeniser::tokenType_identifier;
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
            return CPlusPlusCodeTokeniser::tokenType_floatLiteral;

        source = original;

        if (parseHexLiteral (source))
            return CPlusPlusCodeTokeniser::tokenType_integerLiteral;

        source = original;

        if (parseOctalLiteral (source))
            return CPlusPlusCodeTokeniser::tokenType_integerLiteral;

        source = original;

        if (parseDecimalLiteral (source))
            return CPlusPlusCodeTokeniser::tokenType_integerLiteral;

        source = original;
        source.skip();

        return CPlusPlusCodeTokeniser::tokenType_error;
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
}

//==============================================================================
int CPlusPlusCodeTokeniser::readNextToken (CodeDocument::Iterator& source)
{
    int result = tokenType_error;
    source.skipWhitespace();

    juce_wchar firstChar = source.peekNextChar();

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
        result = CppTokeniser::parseNumber (source);
        break;

    case '.':
        result = CppTokeniser::parseNumber (source);

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
        CppTokeniser::skipQuotedString (source);
        result = tokenType_stringLiteral;
        break;

    case '+':
        result = tokenType_operator;
        source.skip();

        if (source.peekNextChar() == '+')
            source.skip();
        else if (source.peekNextChar() == '=')
            source.skip();

        break;

    case '-':
        source.skip();
        result = CppTokeniser::parseNumber (source);

        if (result == tokenType_error)
        {
            result = tokenType_operator;

            if (source.peekNextChar() == '-')
                source.skip();
            else if (source.peekNextChar() == '=')
                source.skip();
        }
        break;

    case '*':
    case '%':
    case '=':
    case '!':
        result = tokenType_operator;
        source.skip();

        if (source.peekNextChar() == '=')
            source.skip();

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
            CppTokeniser::skipComment (source);
        }

        break;

    case '?':
    case '~':
        source.skip();
        result = tokenType_operator;
        break;

    case '<':
        source.skip();
        result = tokenType_operator;

        if (source.peekNextChar() == '=')
        {
            source.skip();
        }
        else if (source.peekNextChar() == '<')
        {
            source.skip();

            if (source.peekNextChar() == '=')
                source.skip();
        }

        break;

    case '>':
        source.skip();
        result = tokenType_operator;

        if (source.peekNextChar() == '=')
        {
            source.skip();
        }
        else if (source.peekNextChar() == '<')
        {
            source.skip();

            if (source.peekNextChar() == '=')
                source.skip();
        }

        break;

    case '|':
        source.skip();
        result = tokenType_operator;

        if (source.peekNextChar() == '=')
        {
            source.skip();
        }
        else if (source.peekNextChar() == '|')
        {
            source.skip();

            if (source.peekNextChar() == '=')
                source.skip();
        }

        break;

    case '&':
        source.skip();
        result = tokenType_operator;

        if (source.peekNextChar() == '=')
        {
            source.skip();
        }
        else if (source.peekNextChar() == '&')
        {
            source.skip();

            if (source.peekNextChar() == '=')
                source.skip();
        }

        break;

    case '^':
        source.skip();
        result = tokenType_operator;

        if (source.peekNextChar() == '=')
        {
            source.skip();
        }
        else if (source.peekNextChar() == '^')
        {
            source.skip();

            if (source.peekNextChar() == '=')
                source.skip();
        }

        break;

    case '#':
        result = tokenType_preprocessor;
        source.skipToEndOfLine();
        break;

    default:
        if (CppTokeniser::isIdentifierStart (firstChar))
            result = CppTokeniser::parseIdentifier (source);
        else
            source.skip();

        break;
    }

    return result;
}

StringArray CPlusPlusCodeTokeniser::getTokenTypes()
{
    const char* const types[] =
    {
        "Error",
        "Comment",
        "C++ keyword",
        "Identifier",
        "Integer literal",
        "Float literal",
        "String literal",
        "Operator",
        "Bracket",
        "Punctuation",
        "Preprocessor line",
        0
    };

    return StringArray (types);
}

Colour CPlusPlusCodeTokeniser::getDefaultColour (const int tokenType)
{
    const uint32 colours[] =
    {
        0xffcc0000,  // error
        0xff00aa00,  // comment
        0xff0000cc,  // keyword
        0xff000000,  // identifier
        0xff880000,  // int literal
        0xff885500,  // float literal
        0xff990099,  // string literal
        0xff225500,  // operator
        0xff000055,  // bracket
        0xff004400,  // punctuation
        0xff660000   // preprocessor
    };

    if (tokenType >= 0 && tokenType < numElementsInArray (colours))
        return Colour (colours [tokenType]);

    return Colours::black;
}

bool CPlusPlusCodeTokeniser::isReservedKeyword (const String& token) noexcept
{
    return CppTokeniser::isReservedKeyword (token.getCharPointer(), token.length());
}
