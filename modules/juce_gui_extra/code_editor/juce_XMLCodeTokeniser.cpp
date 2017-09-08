/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

XmlTokeniser::XmlTokeniser() {}
XmlTokeniser::~XmlTokeniser() {}

CodeEditorComponent::ColourScheme XmlTokeniser::getDefaultColourScheme()
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
        { "String",             0xff990099 },
        { "Bracket",            0xff000055 },
        { "Punctuation",        0xff004400 },
        { "Preprocessor Text",  0xff660000 }
    };

    CodeEditorComponent::ColourScheme cs;

    for (unsigned int i = 0; i < sizeof (types) / sizeof (types[0]); ++i)  // (NB: numElementsInArray doesn't work here in GCC4.2)
        cs.set (types[i].name, Colour (types[i].colour));

    return cs;
}

template <typename Iterator>
static void skipToEndOfXmlDTD (Iterator& source) noexcept
{
    bool lastWasQuestionMark = false;

    for (;;)
    {
        const juce_wchar c = source.nextChar();

        if (c == 0 || (c == '>' && lastWasQuestionMark))
            break;

        lastWasQuestionMark = (c == '?');
    }
}

template <typename Iterator>
static void skipToEndOfXmlComment (Iterator& source) noexcept
{
    juce_wchar last[2] = { 0 };

    for (;;)
    {
        const juce_wchar c = source.nextChar();

        if (c == 0 || (c == '>' && last[0] == '-' && last[1] == '-'))
            break;

        last[1] = last[0];
        last[0] = c;
    }
}

int XmlTokeniser::readNextToken (CodeDocument::Iterator& source)
{
    source.skipWhitespace();
    const juce_wchar firstChar = source.peekNextChar();

    switch (firstChar)
    {
        case 0:  break;

        case '"':
        case '\'':
            CppTokeniserFunctions::skipQuotedString (source);
            return tokenType_string;

        case '<':
        {
            source.skip();
            source.skipWhitespace();
            const juce_wchar nextChar = source.peekNextChar();

            if (nextChar == '?')
            {
                source.skip();
                skipToEndOfXmlDTD (source);
                return tokenType_preprocessor;
            }

            if (nextChar == '!')
            {
                source.skip();

                if (source.peekNextChar() == '-')
                {
                    source.skip();

                    if (source.peekNextChar() == '-')
                    {
                        skipToEndOfXmlComment (source);
                        return tokenType_comment;
                    }
                }
            }

            CppTokeniserFunctions::skipIfNextCharMatches (source, '/');
            CppTokeniserFunctions::parseIdentifier (source);
            source.skipWhitespace();
            CppTokeniserFunctions::skipIfNextCharMatches (source, '/');
            source.skipWhitespace();
            CppTokeniserFunctions::skipIfNextCharMatches (source, '>');
            return tokenType_keyword;
        }

        case '>':
            source.skip();
            return tokenType_keyword;

        case '/':
            source.skip();
            source.skipWhitespace();
            CppTokeniserFunctions::skipIfNextCharMatches (source, '>');
            return tokenType_keyword;

        case '=':
        case ':':
            source.skip();
            return tokenType_operator;

        default:
            if (CppTokeniserFunctions::isIdentifierStart (firstChar))
                CppTokeniserFunctions::parseIdentifier (source);

            source.skip();
            break;
    };

    return tokenType_identifier;
}

} // namespace juce
