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

struct JSONParser
{
    JSONParser (String::CharPointerType text) : startLocation (text), currentLocation (text) {}

    String::CharPointerType startLocation, currentLocation;

    struct ErrorException
    {
        String message;
        int line = 1, column = 1;

        String getDescription() const   { return String (line) + ":" + String (column) + ": error: " + message; }
        Result getResult() const        { return Result::fail (getDescription()); }
    };

    [[noreturn]] void throwError (juce::String message, String::CharPointerType location)
    {
        ErrorException e;
        e.message = std::move (message);

        for (auto i = startLocation; i < location && ! i.isEmpty(); ++i)
        {
            ++e.column;
            if (*i == '\n')  { e.column = 1; e.line++; }
        }

        throw e;
    }

    void skipWhitespace()             { currentLocation = currentLocation.findEndOfWhitespace(); }
    juce_wchar readChar()             { return currentLocation.getAndAdvance(); }
    juce_wchar peekChar() const       { return *currentLocation; }
    bool matchIf (char c)             { if (peekChar() == (juce_wchar) c) { ++currentLocation; return true; } return false; }
    bool isEOF() const                { return peekChar() == 0; }

    bool matchString (const char* t)
    {
        while (*t != 0)
            if (! matchIf (*t++))
                return false;

        return true;
    }

    var parseObjectOrArray()
    {
        skipWhitespace();

        if (matchIf ('{')) return parseObject();
        if (matchIf ('[')) return parseArray();

        if (! isEOF())
            throwError ("Expected '{' or '['", currentLocation);

        return {};
    }

    int parseHexDigit()
    {
        const auto digitValue = CharacterFunctions::getHexDigitValue (readChar());

        if (digitValue < 0)
            throwError ("Invalid hex character", currentLocation - 1);

        return digitValue;
    }

    CharPointer_UTF16::CharType parseCodeUnit()
    {
        return (CharPointer_UTF16::CharType) (   parseHexDigit() << 12
                                              | (parseHexDigit() << 8)
                                              | (parseHexDigit() << 4)
                                              | (parseHexDigit()));
    }

    static constexpr juce_wchar asCodePoint (CharPointer_UTF16::CharType codeUnit)
    {
        return (juce_wchar) (uint32) (uint16) codeUnit;
    }

    CharPointer_UTF16::CharType parseLowSurrogateCodeUnit()
    {
        const auto errorLocation = currentLocation;

        const auto throwLowSurrogateError = [&]()
        {
            throwError ("Expected UTF-16 low surrogate", errorLocation);
        };

        if (readChar() != '\\' || readChar() != 'u')
            throwLowSurrogateError();

        const auto lowSurrogate = parseCodeUnit();

        if (! CharacterFunctions::isLowSurrogate (asCodePoint (lowSurrogate)))
            throwLowSurrogateError();

        return lowSurrogate;
    }

    juce_wchar parseEscapeSequence()
    {
        const auto errorLocation = currentLocation - 2;

        const auto codeUnits = [&]() -> std::array<CharPointer_UTF16::CharType, 2>
        {
            const auto firstCodeUnit = parseCodeUnit();

            if (CharacterFunctions::isNonSurrogateCodePoint (asCodePoint (firstCodeUnit)))
                return { firstCodeUnit, 0 };

            if (! CharacterFunctions::isHighSurrogate (asCodePoint (firstCodeUnit)))
                throwError ("Invalid UTF-16 escape sequence", errorLocation);

            return { firstCodeUnit, parseLowSurrogateCodeUnit() };
        }();

        return CharPointer_UTF16 (codeUnits.data()).getAndAdvance();
    }

    String parseString (const juce_wchar quoteChar)
    {
        MemoryOutputStream buffer (256);

        for (;;)
        {
            auto c = readChar();

            if (c == quoteChar)
                break;

            if (c == '\\')
            {
                c = readChar();

                switch (c)
                {
                    case '"':
                    case '\'':
                    case '\\':
                    case '/': break;

                    case 'a': c = '\a'; break;
                    case 'b': c = '\b'; break;
                    case 'f': c = '\f'; break;
                    case 'n': c = '\n'; break;
                    case 'r': c = '\r'; break;
                    case 't': c = '\t'; break;

                    case 'u': c = parseEscapeSequence(); break;

                    default: break;
                }
            }

            if (c == 0)
                throwError ("Unexpected EOF in string constant", currentLocation);

            buffer.appendUTF8Char (c);
        }

        return buffer.toUTF8();
    }

    var parseAny()
    {
        skipWhitespace();
        auto originalLocation = currentLocation;

        switch (readChar())
        {
            case '{':    return parseObject();
            case '[':    return parseArray();
            case '"':    return parseString ('"');
            case '\'':   return parseString ('\'');

            case '-':
                skipWhitespace();
                return parseNumber (true);

            case '0': case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9':
                currentLocation = originalLocation;
                return parseNumber (false);

            case 't':   // "true"
                if (matchString ("rue"))
                    return var (true);

                break;

            case 'f':   // "false"
                if (matchString ("alse"))
                    return var (false);

                break;

            case 'n':   // "null"
                if (matchString ("ull"))
                    return {};

                break;

            default:
                break;
        }

        throwError ("Syntax error", originalLocation);
    }

    var parseNumber (bool isNegative)
    {
        auto originalPos = currentLocation;

        int64 intValue = readChar() - '0';
        jassert (intValue >= 0 && intValue < 10);

        for (;;)
        {
            auto lastPos = currentLocation;
            auto c = readChar();
            auto digit = ((int) c) - '0';

            if (isPositiveAndBelow (digit, 10))
            {
                intValue = intValue * 10 + digit;
                continue;
            }

            if (c == 'e' || c == 'E' || c == '.')
            {
                currentLocation = originalPos;
                auto asDouble = CharacterFunctions::readDoubleValue (currentLocation);
                return var (isNegative ? -asDouble : asDouble);
            }

            if (CharacterFunctions::isWhitespace (c)
                 || c == ',' || c == '}' || c == ']' || c == 0)
            {
                currentLocation = lastPos;
                break;
            }

            throwError ("Syntax error in number", lastPos);
        }

        auto correctedValue = isNegative ? -intValue : intValue;

        return (intValue >> 31) != 0 ? var (correctedValue)
                                     : var ((int) correctedValue);
    }

    var parseObject()
    {
        auto resultObject = new DynamicObject();
        var result (resultObject);
        auto& resultProperties = resultObject->getProperties();
        auto startOfObjectDecl = currentLocation;

        for (;;)
        {
            skipWhitespace();
            auto errorLocation = currentLocation;
            auto c = readChar();

            if (c == '}')
                break;

            if (c == 0)
                throwError ("Unexpected EOF in object declaration", startOfObjectDecl);

            if (c != '"')
                throwError ("Expected a property name in double-quotes", errorLocation);

            errorLocation = currentLocation;
            Identifier propertyName (parseString ('"'));

            if (! propertyName.isValid())
                throwError ("Invalid property name", errorLocation);

            skipWhitespace();
            errorLocation = currentLocation;

            if (readChar() != ':')
                throwError ("Expected ':'", errorLocation);

            resultProperties.set (propertyName, parseAny());

            skipWhitespace();
            if (matchIf (',')) continue;
            if (matchIf ('}')) break;

            throwError ("Expected ',' or '}'", currentLocation);
        }

        return result;
    }

    var parseArray()
    {
        auto result = var (Array<var>());
        auto destArray = result.getArray();
        auto startOfArrayDecl = currentLocation;

        for (;;)
        {
            skipWhitespace();

            if (matchIf (']'))
                break;

            if (isEOF())
                throwError ("Unexpected EOF in array declaration", startOfArrayDecl);

            destArray->add (parseAny());
            skipWhitespace();

            if (matchIf (',')) continue;
            if (matchIf (']')) break;

            throwError ("Expected ',' or ']'", currentLocation);
        }

        return result;
    }
};

//==============================================================================
struct JSONFormatter
{
    static void writeEscapedChar (OutputStream& out, const unsigned short value)
    {
        out << "\\u" << String::toHexString ((int) value).paddedLeft ('0', 4);
    }

    static void writeString (OutputStream& out, String::CharPointerType t, JSON::Encoding encoding)
    {
        for (;;)
        {
            const auto c = t.getAndAdvance();

            switch (c)
            {
                case 0: return;

                case '\"': out << "\\\""; break;
                case '\\': out << "\\\\"; break;
                case '\b': out << "\\b";  break;
                case '\f': out << "\\f";  break;
                case '\t': out << "\\t";  break;
                case '\r': out << "\\r";  break;
                case '\n': out << "\\n";  break;

                default:
                    if (CharacterFunctions::isAsciiControlCharacter (c))
                    {
                        writeEscapedChar (out, (unsigned short) c);
                    }
                    else
                    {
                        switch (encoding)
                        {
                            case JSON::Encoding::utf8:
                                out << String::charToString (c);
                                break;

                            case JSON::Encoding::ascii:
                                if (CharacterFunctions::isAscii (c))
                                {
                                    out << String::charToString (c);
                                }
                                else if (CharacterFunctions::isPartOfBasicMultilingualPlane (c))
                                {
                                    if (CharacterFunctions::isNonSurrogateCodePoint (c))
                                        writeEscapedChar (out, (unsigned short) c);
                                    else
                                        jassertfalse; // Illegal unicode character
                                }
                                else
                                {
                                    CharPointer_UTF16::CharType codeUnits[2] = {};
                                    CharPointer_UTF16 utf16 (codeUnits);
                                    utf16.write (c);

                                    for (auto& codeUnit : codeUnits)
                                        writeEscapedChar (out, (unsigned short) codeUnit);
                                }
                                break;
                        }
                    }
                    break;
            }
        }
    }

    static void writeSpaces (OutputStream& out, int numSpaces)
    {
        out.writeRepeatedByte (' ', (size_t) numSpaces);
    }

    static void writeArray (OutputStream& out, const Array<var>& array, const JSON::FormatOptions& format)
    {
        out << '[';

        if (! array.isEmpty())
        {
            if (format.getSpacing() == JSON::Spacing::multiLine)
                out << newLine;

            for (int i = 0; i < array.size(); ++i)
            {
                if (format.getSpacing() == JSON::Spacing::multiLine)
                    writeSpaces (out, format.getIndentLevel() + indentSize);

                JSON::writeToStream (out, array.getReference (i), format.withIndentLevel (format.getIndentLevel() + indentSize));

                if (i < array.size() - 1)
                {
                    out << ",";

                    switch (format.getSpacing())
                    {
                        case JSON::Spacing::none: break;
                        case JSON::Spacing::singleLine: out << ' '; break;
                        case JSON::Spacing::multiLine: out << newLine; break;
                    }
                }
                else if (format.getSpacing() == JSON::Spacing::multiLine)
                    out << newLine;
            }

            if (format.getSpacing() == JSON::Spacing::multiLine)
                writeSpaces (out, format.getIndentLevel());
        }

        out << ']';
    }

    enum { indentSize = 2 };
};


void JSON::writeToStream (OutputStream& out, const var& v, const FormatOptions& opt)
{
    if (v.isString())
    {
        out << '"';
        JSONFormatter::writeString (out, v.toString().getCharPointer(), opt.getEncoding());
        out << '"';
    }
    else if (v.isVoid())
    {
        out << "null";
    }
    else if (v.isUndefined())
    {
        out << "undefined";
    }
    else if (v.isBool())
    {
        out << (static_cast<bool> (v) ? "true" : "false");
    }
    else if (v.isDouble())
    {
        auto d = static_cast<double> (v);

        if (juce_isfinite (d))
        {
            out << serialiseDouble (d, opt.getMaxDecimalPlaces());
        }
        else
        {
            out << "null";
        }
    }
    else if (v.isArray())
    {
        JSONFormatter::writeArray (out, *v.getArray(), opt);
    }
    else if (v.isObject())
    {
        if (auto* object = v.getDynamicObject())
            object->writeAsJSON (out, opt);
        else
            jassertfalse; // Only DynamicObjects can be converted to JSON!
    }
    else
    {
        // Can't convert these other types of object to JSON!
        jassert (! (v.isMethod() || v.isBinaryData()));

        out << v.toString();
    }
}

String JSON::toString (const var& v, const FormatOptions& opt)
{
    MemoryOutputStream mo { 1024 };
    writeToStream (mo, v, opt);
    return mo.toUTF8();
}

//==============================================================================
var JSON::parse (const String& text)
{
    var result;

    if (parse (text, result))
        return result;

    return {};
}

var JSON::fromString (StringRef text)
{
    try
    {
        return JSONParser (text.text).parseAny();
    }
    catch (const JSONParser::ErrorException&) {}

    return {};
}

var JSON::parse (InputStream& input)
{
    return parse (input.readEntireStreamAsString());
}

var JSON::parse (const File& file)
{
    return parse (file.loadFileAsString());
}

Result JSON::parse (const String& text, var& result)
{
    try
    {
        result = JSONParser (text.getCharPointer()).parseObjectOrArray();
    }
    catch (const JSONParser::ErrorException& error)
    {
        return error.getResult();
    }

    return Result::ok();
}

String JSON::toString (const var& data, const bool allOnOneLine, int maximumDecimalPlaces)
{
    return toString (data, FormatOptions{}.withSpacing (allOnOneLine ? Spacing::singleLine : Spacing::multiLine)
                                          .withMaxDecimalPlaces (maximumDecimalPlaces));
}

void JSON::writeToStream (OutputStream& output, const var& data, const bool allOnOneLine, int maximumDecimalPlaces)
{
    writeToStream (output, data, FormatOptions{}.withSpacing (allOnOneLine ? Spacing::singleLine : Spacing::multiLine)
                                                .withMaxDecimalPlaces (maximumDecimalPlaces));
}

String JSON::escapeString (StringRef s)
{
    MemoryOutputStream mo;
    JSONFormatter::writeString (mo, s.text, Encoding::ascii);
    return mo.toString();
}

Result JSON::parseQuotedString (String::CharPointerType& t, var& result)
{
    try
    {
        JSONParser parser (t);
        auto quote = parser.readChar();

        if (quote != '"' && quote != '\'')
            return Result::fail ("Not a quoted string!");

        result = parser.parseString (quote);
        t = parser.currentLocation;
    }
    catch (const JSONParser::ErrorException& error)
    {
        return error.getResult();
    }

    return Result::ok();
}


//==============================================================================
//==============================================================================
#if JUCE_UNIT_TESTS

class JSONTests final : public UnitTest
{
public:
    JSONTests()
        : UnitTest ("JSON", UnitTestCategories::json)
    {}

    static String createRandomWideCharString (Random& r)
    {
        juce_wchar buffer[40] = { 0 };

        for (int i = 0; i < numElementsInArray (buffer) - 1; ++i)
        {
            if (r.nextBool())
            {
                do
                {
                    buffer[i] = (juce_wchar) (1 + r.nextInt (0x10ffff - 1));
                }
                while (! CharPointer_UTF16::canRepresent (buffer[i]));
            }
            else
                buffer[i] = (juce_wchar) (1 + r.nextInt (0xff));
        }

        return CharPointer_UTF32 (buffer);
    }

    static String createRandomIdentifier (Random& r)
    {
        char buffer[30] = { 0 };

        for (int i = 0; i < numElementsInArray (buffer) - 1; ++i)
        {
            static const char chars[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-:";
            buffer[i] = chars [r.nextInt (sizeof (chars) - 1)];
        }

        return CharPointer_ASCII (buffer);
    }

    // Creates a random double that can be easily stringified, to avoid
    // false failures when decimal places are rounded or truncated slightly
    static var createRandomDouble (Random& r)
    {
        return var ((r.nextDouble() * 1000.0) + 0.1);
    }

    static var createRandomVar (Random& r, int depth)
    {
        switch (r.nextInt (depth > 3 ? 6 : 8))
        {
            case 0:     return {};
            case 1:     return r.nextInt();
            case 2:     return r.nextInt64();
            case 3:     return r.nextBool();
            case 4:     return createRandomDouble (r);
            case 5:     return createRandomWideCharString (r);

            case 6:
            {
                var v (createRandomVar (r, depth + 1));

                for (int i = 1 + r.nextInt (30); --i >= 0;)
                    v.append (createRandomVar (r, depth + 1));

                return v;
            }

            case 7:
            {
                auto o = new DynamicObject();

                for (int i = r.nextInt (30); --i >= 0;)
                    o->setProperty (createRandomIdentifier (r), createRandomVar (r, depth + 1));

                return o;
            }

            default:
                return {};
        }
    }

    void expectCharacterEncoding (juce_wchar character, const String& expectedOutput, JSON::Encoding encoding)
    {
        const auto input = String::charToString (character);
        const auto quotedOutput = '"' + expectedOutput + '"';
        expectEquals (JSON::toString (input, JSON::FormatOptions{}.withEncoding (encoding)), quotedOutput);
        expectEquals (JSON::fromString (quotedOutput).toString(), input);
    }

    void expectNoEscapeSequence (juce_wchar input)
    {
        const auto inputString = String::charToString (input);
        expectCharacterEncoding (input, inputString, JSON::Encoding::ascii);
        expectCharacterEncoding (input, inputString, JSON::Encoding::utf8);
    }

    void expectEscapeSequenceForAllEncodings (juce_wchar input, const String& escapeSequence)
    {
        expectCharacterEncoding (input, escapeSequence, JSON::Encoding::ascii);
        expectCharacterEncoding (input, escapeSequence, JSON::Encoding::utf8);
    }

    void expectEscapeSequenceForAsciiEncodingOnly (juce_wchar input, const String& escapeSequence)
    {
        expectCharacterEncoding (input, escapeSequence, JSON::Encoding::ascii);
        expectCharacterEncoding (input, String::charToString (input), JSON::Encoding::utf8);
    }

    void runTest() override
    {
        beginTest ("Float formatting");
        {
            std::map<double, String> tests;
            tests[1] = "1.0";
            tests[1.1] = "1.1";
            tests[1.01] = "1.01";
            tests[0.76378] = "0.76378";
            tests[-10] = "-10.0";
            tests[10.01] = "10.01";
            tests[0.0123] = "0.0123";
            tests[-3.7e-27] = "-3.7e-27";
            tests[1e+40] = "1.0e40";
            tests[-12345678901234567.0] = "-1.234567890123457e16";
            tests[192000] = "192000.0";
            tests[1234567] = "1.234567e6";
            tests[0.00006] = "0.00006";
            tests[0.000006] = "6.0e-6";

            for (auto& test : tests)
                expectEquals (JSON::toString (test.first), test.second);
        }

        beginTest ("ASCII control characters are always escaped");
        {
            expectEscapeSequenceForAllEncodings ('\x01', "\\u0001");
            expectEscapeSequenceForAllEncodings ('\x02', "\\u0002");
            expectEscapeSequenceForAllEncodings ('\x03', "\\u0003");
            expectEscapeSequenceForAllEncodings ('\x04', "\\u0004");
            expectEscapeSequenceForAllEncodings ('\x05', "\\u0005");
            expectEscapeSequenceForAllEncodings ('\x06', "\\u0006");
            expectEscapeSequenceForAllEncodings ('\x07', "\\u0007");
            expectEscapeSequenceForAllEncodings ('\x08', "\\b");
            expectEscapeSequenceForAllEncodings ('\x09', "\\t");
            expectEscapeSequenceForAllEncodings ('\x0a', "\\n");
            expectEscapeSequenceForAllEncodings ('\x0b', "\\u000b");
            expectEscapeSequenceForAllEncodings ('\x0c', "\\f");
            expectEscapeSequenceForAllEncodings ('\x0d', "\\r");
            expectEscapeSequenceForAllEncodings ('\x0e', "\\u000e");
            expectEscapeSequenceForAllEncodings ('\x0f', "\\u000f");
            expectEscapeSequenceForAllEncodings ('\x10', "\\u0010");
            expectEscapeSequenceForAllEncodings ('\x11', "\\u0011");
            expectEscapeSequenceForAllEncodings ('\x12', "\\u0012");
            expectEscapeSequenceForAllEncodings ('\x13', "\\u0013");
            expectEscapeSequenceForAllEncodings ('\x14', "\\u0014");
            expectEscapeSequenceForAllEncodings ('\x15', "\\u0015");
            expectEscapeSequenceForAllEncodings ('\x16', "\\u0016");
            expectEscapeSequenceForAllEncodings ('\x17', "\\u0017");
            expectEscapeSequenceForAllEncodings ('\x18', "\\u0018");
            expectEscapeSequenceForAllEncodings ('\x19', "\\u0019");
            expectEscapeSequenceForAllEncodings ('\x1a', "\\u001a");
            expectEscapeSequenceForAllEncodings ('\x1b', "\\u001b");
            expectEscapeSequenceForAllEncodings ('\x1c', "\\u001c");
            expectEscapeSequenceForAllEncodings ('\x1d', "\\u001d");
            expectEscapeSequenceForAllEncodings ('\x1e', "\\u001e");
            expectEscapeSequenceForAllEncodings ('\x1f', "\\u001f");
        }

        beginTest ("Only special ASCII characters are escaped");
        {
            for (juce_wchar c = 32; CharacterFunctions::isAscii (c); ++c)
            {
                if (c != '"')
                    expectEscapeSequenceForAllEncodings ('"',  R"(\")");
                else if (c != '\\')
                    expectEscapeSequenceForAllEncodings ('\\', R"(\\)");
                else
                    expectNoEscapeSequence (c);
            }
        }

        beginTest ("Unicode characters are escaped for ASCII encoding only");
        {
            // First and last 2 byte UTF-8 code points
            expectEscapeSequenceForAsciiEncodingOnly ((juce_wchar) 0x0080, "\\u0080");
            expectEscapeSequenceForAsciiEncodingOnly ((juce_wchar) 0x07FF, "\\u07ff");

            // First and last 3 byte UTF-8 code points
            expectEscapeSequenceForAsciiEncodingOnly ((juce_wchar) 0x0800, "\\u0800");
            expectEscapeSequenceForAsciiEncodingOnly ((juce_wchar) 0xffff, "\\uffff");

            // Code points at the UTF-16 surrogate boundaries
            expectEscapeSequenceForAsciiEncodingOnly ((juce_wchar) 0xd7ff, "\\ud7ff");
            expectEscapeSequenceForAsciiEncodingOnly ((juce_wchar) 0xe000, "\\ue000");

            // First and last 4 byte UTF-8 code points (also first and last UTF-16 surrogate pairs)
            expectEscapeSequenceForAsciiEncodingOnly ((juce_wchar) 0x010000, "\\ud800\\udc00");
            expectEscapeSequenceForAsciiEncodingOnly ((juce_wchar) 0x10ffff, "\\udbff\\udfff");
        }

        beginTest ("Fuzz tests");
        {
            auto r = getRandom();

            expect (JSON::parse (String()) == var());
            expect (JSON::parse ("{}").isObject());
            expect (JSON::parse ("[]").isArray());
            expect (JSON::parse ("[ 1234 ]")[0].isInt());
            expect (JSON::parse ("[ 12345678901234 ]")[0].isInt64());
            expect (JSON::parse ("[ 1.123e3 ]")[0].isDouble());
            expect (JSON::parse ("[ -1234]")[0].isInt());
            expect (JSON::parse ("[-12345678901234]")[0].isInt64());
            expect (JSON::parse ("[-1.123e3]")[0].isDouble());

            for (int i = 100; --i >= 0;)
            {
                var v;

                if (i > 0)
                    v = createRandomVar (r, 0);

                const auto oneLine = r.nextBool();
                const auto asString = JSON::toString (v, oneLine);
                const auto parsed = JSON::parse ("[" + asString + "]")[0];
                const auto parsedString = JSON::toString (parsed, oneLine);
                expect (asString.isNotEmpty() && parsedString == asString);
            }
        }
    }
};

static JSONTests JSONUnitTests;

#endif

} // namespace juce
