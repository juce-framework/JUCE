/*
  ==============================================================================

   This file is part of the juce_core module of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission to use, copy, modify, and/or distribute this software for any purpose with
   or without fee is hereby granted, provided that the above copyright notice and this
   permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD
   TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN
   NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
   DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
   IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
   CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

   ------------------------------------------------------------------------------

   NOTE! This permissive ISC license applies ONLY to files within the juce_core module!
   All other JUCE modules are covered by a dual GPL/commercial license, so if you are
   using any other modules, be sure to check that you also comply with their license.

   For more details, visit www.juce.com

  ==============================================================================
*/

class JSONParser
{
public:
    static Result parseObjectOrArray (String::CharPointerType t, var& result)
    {
        t = t.findEndOfWhitespace();

        switch (t.getAndAdvance())
        {
            case 0:      result = var(); return Result::ok();
            case '{':    return parseObject (t, result);
            case '[':    return parseArray  (t, result);
        }

        return createFail ("Expected '{' or '['", &t);
    }

    static Result parseString (const juce_wchar quoteChar, String::CharPointerType& t, var& result)
    {
        MemoryOutputStream buffer (256);

        for (;;)
        {
            juce_wchar c = t.getAndAdvance();

            if (c == quoteChar)
                break;

            if (c == '\\')
            {
                c = t.getAndAdvance();

                switch (c)
                {
                    case '"':
                    case '\'':
                    case '\\':
                    case '/':  break;

                    case 'a':  c = '\a'; break;
                    case 'b':  c = '\b'; break;
                    case 'f':  c = '\f'; break;
                    case 'n':  c = '\n'; break;
                    case 'r':  c = '\r'; break;
                    case 't':  c = '\t'; break;

                    case 'u':
                    {
                        c = 0;

                        for (int i = 4; --i >= 0;)
                        {
                            const int digitValue = CharacterFunctions::getHexDigitValue (t.getAndAdvance());
                            if (digitValue < 0)
                                return createFail ("Syntax error in unicode escape sequence");

                            c = (juce_wchar) ((c << 4) + digitValue);
                        }

                        break;
                    }
                }
            }

            if (c == 0)
                return createFail ("Unexpected end-of-input in string constant");

            buffer.appendUTF8Char (c);
        }

        result = buffer.toUTF8();
        return Result::ok();
    }

    static Result parseAny (String::CharPointerType& t, var& result)
    {
        t = t.findEndOfWhitespace();
        String::CharPointerType t2 (t);

        switch (t2.getAndAdvance())
        {
            case '{':    t = t2; return parseObject (t, result);
            case '[':    t = t2; return parseArray  (t, result);
            case '"':    t = t2; return parseString ('"',  t, result);
            case '\'':   t = t2; return parseString ('\'', t, result);

            case '-':
                t2 = t2.findEndOfWhitespace();
                if (! CharacterFunctions::isDigit (*t2))
                    break;

                t = t2;
                return parseNumber (t, result, true);

            case '0': case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9':
                return parseNumber (t, result, false);

            case 't':   // "true"
                if (t2.getAndAdvance() == 'r' && t2.getAndAdvance() == 'u' && t2.getAndAdvance() == 'e')
                {
                    t = t2;
                    result = var (true);
                    return Result::ok();
                }
                break;

            case 'f':   // "false"
                if (t2.getAndAdvance() == 'a' && t2.getAndAdvance() == 'l'
                      && t2.getAndAdvance() == 's' && t2.getAndAdvance() == 'e')
                {
                    t = t2;
                    result = var (false);
                    return Result::ok();
                }
                break;

            case 'n':   // "null"
                if (t2.getAndAdvance() == 'u' && t2.getAndAdvance() == 'l' && t2.getAndAdvance() == 'l')
                {
                    t = t2;
                    result = var();
                    return Result::ok();
                }
                break;

            default:
                break;
        }

        return createFail ("Syntax error", &t);
    }

private:
    static Result createFail (const char* const message, const String::CharPointerType* location = nullptr)
    {
        String m (message);
        if (location != nullptr)
            m << ": \"" << String (*location, 20) << '"';

        return Result::fail (m);
    }

    static Result parseNumber (String::CharPointerType& t, var& result, const bool isNegative)
    {
        String::CharPointerType oldT (t);

        int64 intValue = t.getAndAdvance() - '0';
        jassert (intValue >= 0 && intValue < 10);

        for (;;)
        {
            String::CharPointerType previousChar (t);
            const juce_wchar c = t.getAndAdvance();
            const int digit = ((int) c) - '0';

            if (isPositiveAndBelow (digit, 10))
            {
                intValue = intValue * 10 + digit;
                continue;
            }

            if (c == 'e' || c == 'E' || c == '.')
            {
                t = oldT;
                const double asDouble = CharacterFunctions::readDoubleValue (t);
                result = isNegative ? -asDouble : asDouble;
                return Result::ok();
            }

            if (CharacterFunctions::isWhitespace (c)
                 || c == ',' || c == '}' || c == ']' || c == 0)
            {
                t = previousChar;
                break;
            }

            return createFail ("Syntax error in number", &oldT);
        }

        const int64 correctedValue = isNegative ? -intValue : intValue;

        if ((intValue >> 31) != 0)
            result = correctedValue;
        else
            result = (int) correctedValue;

        return Result::ok();
    }

    static Result parseObject (String::CharPointerType& t, var& result)
    {
        DynamicObject* const resultObject = new DynamicObject();
        result = resultObject;
        NamedValueSet& resultProperties = resultObject->getProperties();

        for (;;)
        {
            t = t.findEndOfWhitespace();

            String::CharPointerType oldT (t);
            const juce_wchar c = t.getAndAdvance();

            if (c == '}')
                break;

            if (c == 0)
                return createFail ("Unexpected end-of-input in object declaration");

            if (c == '"')
            {
                var propertyNameVar;
                Result r (parseString ('"', t, propertyNameVar));

                if (r.failed())
                    return r;

                const Identifier propertyName (propertyNameVar.toString());

                if (propertyName.isValid())
                {
                    t = t.findEndOfWhitespace();
                    oldT = t;

                    const juce_wchar c2 = t.getAndAdvance();
                    if (c2 != ':')
                        return createFail ("Expected ':', but found", &oldT);

                    resultProperties.set (propertyName, var());
                    var* propertyValue = resultProperties.getVarPointer (propertyName);

                    Result r2 (parseAny (t, *propertyValue));

                    if (r2.failed())
                        return r2;

                    t = t.findEndOfWhitespace();
                    oldT = t;

                    const juce_wchar nextChar = t.getAndAdvance();

                    if (nextChar == ',')
                        continue;

                    if (nextChar == '}')
                        break;
                }
            }

            return createFail ("Expected object member declaration, but found", &oldT);
        }

        return Result::ok();
    }

    static Result parseArray (String::CharPointerType& t, var& result)
    {
        result = var (Array<var>());
        Array<var>* const destArray = result.getArray();

        for (;;)
        {
            t = t.findEndOfWhitespace();

            String::CharPointerType oldT (t);
            const juce_wchar c = t.getAndAdvance();

            if (c == ']')
                break;

            if (c == 0)
                return createFail ("Unexpected end-of-input in array declaration");

            t = oldT;
            destArray->add (var());
            Result r (parseAny (t, destArray->getReference (destArray->size() - 1)));

            if (r.failed())
                return r;

            t = t.findEndOfWhitespace();
            oldT = t;

            const juce_wchar nextChar = t.getAndAdvance();

            if (nextChar == ',')
                continue;

            if (nextChar == ']')
                break;

            return createFail ("Expected object array item, but found", &oldT);
        }

        return Result::ok();
    }
};

//==============================================================================
class JSONFormatter
{
public:
    static void write (OutputStream& out, const var& v,
                       const int indentLevel, const bool allOnOneLine)
    {
        if (v.isString())
        {
            out << '"';
            writeString (out, v.toString().getCharPointer());
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
        else if (v.isArray())
        {
            writeArray (out, *v.getArray(), indentLevel, allOnOneLine);
        }
        else if (v.isObject())
        {
            if (DynamicObject* object = v.getDynamicObject())
                object->writeAsJSON (out, indentLevel, allOnOneLine);
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

    static void writeEscapedChar (OutputStream& out, const unsigned short value)
    {
        out << "\\u" << String::toHexString ((int) value).paddedLeft ('0', 4);
    }

    static void writeString (OutputStream& out, String::CharPointerType t)
    {
        for (;;)
        {
            const juce_wchar c (t.getAndAdvance());

            switch (c)
            {
                case 0:  return;

                case '\"':  out << "\\\""; break;
                case '\\':  out << "\\\\"; break;
                case '\a':  out << "\\a";  break;
                case '\b':  out << "\\b";  break;
                case '\f':  out << "\\f";  break;
                case '\t':  out << "\\t";  break;
                case '\r':  out << "\\r";  break;
                case '\n':  out << "\\n";  break;

                default:
                    if (c >= 32 && c < 127)
                    {
                        out << (char) c;
                    }
                    else
                    {
                        if (CharPointer_UTF16::getBytesRequiredFor (c) > 2)
                        {
                            CharPointer_UTF16::CharType chars[2];
                            CharPointer_UTF16 utf16 (chars);
                            utf16.write (c);

                            for (int i = 0; i < 2; ++i)
                                writeEscapedChar (out, (unsigned short) chars[i]);
                        }
                        else
                        {
                            writeEscapedChar (out, (unsigned short) c);
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

    static void writeArray (OutputStream& out, const Array<var>& array,
                            const int indentLevel, const bool allOnOneLine)
    {
        out << '[';

        if (array.size() > 0)
        {
            if (! allOnOneLine)
                out << newLine;

            for (int i = 0; i < array.size(); ++i)
            {
                if (! allOnOneLine)
                    writeSpaces (out, indentLevel + indentSize);

                write (out, array.getReference(i), indentLevel + indentSize, allOnOneLine);

                if (i < array.size() - 1)
                {
                    if (allOnOneLine)
                        out << ", ";
                    else
                        out << ',' << newLine;
                }
                else if (! allOnOneLine)
                    out << newLine;
            }

            if (! allOnOneLine)
                writeSpaces (out, indentLevel);
        }

        out << ']';
    }

    enum { indentSize = 2 };
};

//==============================================================================
var JSON::parse (const String& text)
{
    var result;

    if (! parse (text, result))
        result = var();

    return result;
}

var JSON::fromString (StringRef text)
{
    var result;

    if (! JSONParser::parseAny (text.text, result))
        result = var();

    return result;
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
    return JSONParser::parseObjectOrArray (text.getCharPointer(), result);
}

String JSON::toString (const var& data, const bool allOnOneLine)
{
    MemoryOutputStream mo (1024);
    JSONFormatter::write (mo, data, 0, allOnOneLine);
    return mo.toUTF8();
}

void JSON::writeToStream (OutputStream& output, const var& data, const bool allOnOneLine)
{
    JSONFormatter::write (output, data, 0, allOnOneLine);
}

String JSON::escapeString (StringRef s)
{
    MemoryOutputStream mo;
    JSONFormatter::writeString (mo, s.text);
    return mo.toString();
}

Result JSON::parseQuotedString (String::CharPointerType& t, var& result)
{
    const juce_wchar quote = t.getAndAdvance();

    if (quote == '"' || quote == '\'')
        return JSONParser::parseString (quote, t, result);

    return Result::fail ("Not a quoted string!");
}

//==============================================================================
//==============================================================================
#if JUCE_UNIT_TESTS

class JSONTests  : public UnitTest
{
public:
    JSONTests() : UnitTest ("JSON") {}

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

    // (creates a random double that can be easily stringified, to avoid
    // false failures when decimal places are rounded or truncated slightly)
    static var createRandomDouble (Random& r)
    {
        for (;;)
        {
            var v (String (r.nextDouble() * 1000.0, 20).getDoubleValue());

            if (v.toString() == String (static_cast<double> (v), 20))
                return v;
        }
    }

    static var createRandomVar (Random& r, int depth)
    {
        switch (r.nextInt (depth > 3 ? 6 : 8))
        {
            case 0:     return var();
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
                DynamicObject* o = new DynamicObject();

                for (int i = r.nextInt (30); --i >= 0;)
                    o->setProperty (createRandomIdentifier (r), createRandomVar (r, depth + 1));

                return o;
            }

            default:
                return var();
        }
    }

    void runTest() override
    {
        beginTest ("JSON");
        Random r = getRandom();

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

            const bool oneLine = r.nextBool();
            String asString (JSON::toString (v, oneLine));
            var parsed = JSON::parse ("[" + asString + "]")[0];
            String parsedString (JSON::toString (parsed, oneLine));
            expect (asString.isNotEmpty() && parsedString == asString);
        }
    }
};

static JSONTests JSONUnitTests;

#endif
