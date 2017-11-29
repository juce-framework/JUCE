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

#pragma once


//==============================================================================
struct CppParserHelpers
{
    static bool parseHexInt (const String& text, int64& result)
    {
        CppTokeniserFunctions::StringIterator i (text);

        if (CppTokeniserFunctions::parseHexLiteral (i))
        {
            result = text.fromFirstOccurrenceOf ("x", false, true).getHexValue64();
            return true;
        }

        return false;
    }

    static bool parseOctalInt (const String& text, int64& result)
    {
        CppTokeniserFunctions::StringIterator it (text);

        if (CppTokeniserFunctions::parseOctalLiteral (it))
        {
            result = 0;

            for (int i = 0; i < text.length(); ++i)
            {
                const int digit = text[i] - '0';

                if (digit < 0 || digit > 7)
                    break;

                result = result * 8 + digit;
            }

            return true;
        }

        return false;
    }

    static bool parseDecimalInt (const String& text, int64& result)
    {
        CppTokeniserFunctions::StringIterator i (text);

        if (CppTokeniserFunctions::parseDecimalLiteral (i))
        {
            result = text.getLargeIntValue();
            return true;
        }

        return false;
    }

    static bool parseInt (const String& text, int64& result)
    {
        return parseHexInt (text, result)
            || parseOctalInt (text, result)
            || parseDecimalInt (text, result);
    }

    static bool parseFloat (const String& text, double& result)
    {
        CppTokeniserFunctions::StringIterator i (text);

        if (CppTokeniserFunctions::parseFloatLiteral (i))
        {
            result = text.getDoubleValue();
            return true;
        }

        return false;
    }

    static int parseSingleToken (const String& text)
    {
        if (text.isEmpty())
            return CPlusPlusCodeTokeniser::tokenType_error;

        CppTokeniserFunctions::StringIterator i (text);
        i.skipWhitespace();
        const int tok = CppTokeniserFunctions::readNextToken (i);
        i.skipWhitespace();
        i.skip();
        return i.isEOF() ? tok : CPlusPlusCodeTokeniser::tokenType_error;
    }

    static String getIntegerSuffix (const String& s)    { return s.retainCharacters ("lLuU"); }
    static String getFloatSuffix (const String& s)      { return s.retainCharacters ("fF"); }

    static String getReplacementStringInSameFormat (const String& old, double newValue)
    {
        {
            CppTokeniserFunctions::StringIterator i (old);

            if (CppTokeniserFunctions::parseFloatLiteral (i))
            {
                String s (newValue);

                if (! s.containsChar ('.'))
                    s += ".0";

                return s + getFloatSuffix (old);
            }
        }

        return getReplacementStringInSameFormat (old, (int64) newValue);
    }

    static String getReplacementStringInSameFormat (const String& old, int64 newValue)
    {
        {
            CppTokeniserFunctions::StringIterator i (old);

            if (CppTokeniserFunctions::parseHexLiteral (i))
            {
                String s ("0x" + String::toHexString (newValue) + getIntegerSuffix (old));

                if (old.toUpperCase() == old)
                    s = s.toUpperCase();

                return s;
            }
        }

        {
            CppTokeniserFunctions::StringIterator i (old);

            if (CppTokeniserFunctions::parseDecimalLiteral (i))
                return String (newValue) + getIntegerSuffix (old);
        }

        return old;
    }

    // Given a type name which could be a smart pointer or other pointer/ref, this extracts
    // the essential class name of the thing that it points to.
    static String getSignificantClass (String cls)
    {
        int firstAngleBracket = cls.indexOfChar ('<');

        if (firstAngleBracket > 0)
            cls = cls.substring (firstAngleBracket + 1).upToLastOccurrenceOf (">", false, false).trim();

        while (cls.endsWithChar ('*') || cls.endsWithChar ('&'))
            cls = cls.dropLastCharacters (1).trim();

        return cls;
    }

    //==============================================================================
    struct ValidCppIdentifierRestriction  : public TextEditor::InputFilter
    {
        ValidCppIdentifierRestriction (bool allowTemplatesAndNamespaces)
            : className (allowTemplatesAndNamespaces) {}

        String filterNewText (TextEditor& ed, const String& text)
        {
            String allowedChars ("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_");
            if (className)
                allowedChars += "<>:";

            if (ed.getHighlightedRegion().getStart() > 0)
                allowedChars += "0123456789";

            String s = text.retainCharacters (allowedChars);

            if (CPlusPlusCodeTokeniser::isReservedKeyword (ed.getText().replaceSection (ed.getHighlightedRegion().getStart(),
                                                                                        ed.getHighlightedRegion().getLength(),
                                                                                        s)))
                return String();

            return s;
        }

        bool className;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ValidCppIdentifierRestriction)
    };
};

//==============================================================================
struct CodeChange
{
    CodeChange (Range<int> r, const String& t)  : range (r), text (t)
    {
    }

    bool mergeWith (const CodeChange& next)
    {
        if (text.isEmpty())
        {
            if (next.text.isNotEmpty()
                && next.range.isEmpty()
                && next.range.getStart() == range.getStart())
            {
                text = next.text;
                return true;
            }

            if (next.text.isEmpty())
            {
                Range<int> nextRange (next.range);

                if (nextRange.getStart() >= range.getStart())
                    nextRange += range.getLength();
                else if (nextRange.getEnd() > range.getStart())
                    nextRange.setEnd (nextRange.getEnd() + range.getLength());

                if (range.intersects (nextRange)
                      || range.getEnd() == nextRange.getStart()
                      || range.getStart() == nextRange.getEnd())
                {
                    range = range.getUnionWith (nextRange);
                    return true;
                }
            }
        }
        else if (next.text.isEmpty())
        {
            if (next.range.getEnd() == range.getStart())
            {
                range.setStart (next.range.getStart());
                return true;
            }

            if (next.range.getStart() == range.getStart() + text.length())
            {
                range.setLength (range.getLength() + next.range.getLength());
                return true;
            }
        }

        return false;
    }

    void addToList (Array<CodeChange>& list) const
    {
        if (list.size() == 0 || ! list.getReference (list.size() - 1).mergeWith (*this))
            list.add (*this);
    }

    Range<int> range;
    String text;
};

//==============================================================================
static inline String concatenateListOfStrings (const StringArray& s)
{
    return s.joinIntoString ("\x01");
}

static inline StringArray separateJoinedStrings (const String& s)
{
    return StringArray::fromTokens (s, "\x01", juce::StringRef());
}
