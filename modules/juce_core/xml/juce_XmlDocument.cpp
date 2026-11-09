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

   JUCE End User Licence Agreement: https://juce.com/legal/juce-9-licence/
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

XmlDocument::XmlDocument (const String& text)  : originalText (text) {}
XmlDocument::XmlDocument (const File& file)  : inputSource (new FileInputSource (file)) {}

XmlDocument::~XmlDocument() {}

std::unique_ptr<XmlElement> XmlDocument::parse (const File& file)
{
    return XmlDocument (file).getDocumentElement();
}

std::unique_ptr<XmlElement> XmlDocument::parse (const String& textToParse)
{
    return XmlDocument (textToParse).getDocumentElement();
}

std::unique_ptr<XmlElement> parseXML (const String& textToParse)
{
    return XmlDocument (textToParse).getDocumentElement();
}

std::unique_ptr<XmlElement> parseXML (const File& file)
{
    return XmlDocument (file).getDocumentElement();
}

std::unique_ptr<XmlElement> parseXMLIfTagMatches (const String& textToParse, StringRef requiredTag)
{
    return XmlDocument (textToParse).getDocumentElementIfTagMatches (requiredTag);
}

std::unique_ptr<XmlElement> parseXMLIfTagMatches (const File& file, StringRef requiredTag)
{
    return XmlDocument (file).getDocumentElementIfTagMatches (requiredTag);
}

void XmlDocument::setInputSource (InputSource* newSource) noexcept
{
    inputSource.reset (newSource);
    externalEntitiesAllowed = true;
}

void XmlDocument::setEmptyTextElementsIgnored (bool shouldBeIgnored) noexcept
{
    ignoreEmptyTextElements = shouldBeIgnored;
}

namespace XmlParsingLimits
{
    constexpr int maxElementNestingDepth = 256,
                  maxEntityNestingDepth = 40;

    constexpr int64 entityAmplificationThreshold = 8 * 1024 * 1024,
                    maxEntityAmplification = 100;
}

static bool entityTextIsMarkup (const String& text)
{
    return text[0] == '<' && text[1] != 0;
}

namespace XmlIdentifierChars
{
    static bool isIdentifierCharSlow (juce_wchar c) noexcept
    {
        return CharacterFunctions::isLetterOrDigit (c)
                 || c == '_' || c == '-' || c == ':' || c == '.';
    }

    static bool isIdentifierChar (juce_wchar c) noexcept
    {
        static const uint32 legalChars[] = { 0, 0x7ff6000, 0x87fffffe, 0x7fffffe, 0 };

        return ((int) c < (int) numElementsInArray (legalChars) * 32) ? ((legalChars [c >> 5] & (uint32) (1 << (c & 31))) != 0)
                                                                      : isIdentifierCharSlow (c);
    }

    /*static void generateIdentifierCharConstants()
    {
        uint32 n[8] = { 0 };
        for (int i = 0; i < 256; ++i)
            if (isIdentifierCharSlow (i))
                n[i >> 5] |= (1 << (i & 31));

        String s;
        for (int i = 0; i < 8; ++i)
            s << "0x" << String::toHexString ((int) n[i]) << ", ";

        DBG (s);
    }*/

    static String::CharPointerType findEndOfToken (String::CharPointerType p) noexcept
    {
        while (isIdentifierChar (*p))
            ++p;

        return p;
    }
}

std::unique_ptr<XmlElement> XmlDocument::getDocumentElement (const bool onlyReadOuterDocumentElement)
{
    if (originalText.isEmpty() && inputSource != nullptr)
    {
        std::unique_ptr<InputStream> in (inputSource->createInputStream());

        if (in != nullptr)
        {
            MemoryOutputStream data;
            data.writeFromInputStream (*in, onlyReadOuterDocumentElement ? 8192 : -1);

           #if JUCE_STRING_UTF_TYPE == 8
            if (data.getDataSize() > 2)
            {
                data.writeByte (0);
                auto* text = static_cast<const char*> (data.getData());

                if (CharPointer_UTF16::isByteOrderMarkBigEndian (text)
                      || CharPointer_UTF16::isByteOrderMarkLittleEndian (text))
                {
                    originalText = data.toString();
                }
                else
                {
                    if (CharPointer_UTF8::isByteOrderMark (text))
                        text += 3;

                    // parse the input buffer directly to avoid copying it all to a string
                    return parseDocumentElement (String::CharPointerType (text), onlyReadOuterDocumentElement);
                }
            }
           #else
            originalText = data.toString();
           #endif
        }
    }

    return parseDocumentElement (originalText.getCharPointer(), onlyReadOuterDocumentElement);
}

std::unique_ptr<XmlElement> XmlDocument::getDocumentElementIfTagMatches (StringRef requiredTag)
{
    if (auto xml = getDocumentElement (true))
        if (xml->hasTagName (requiredTag))
            return getDocumentElement (false);

    return {};
}

const String& XmlDocument::getLastParseError() const noexcept
{
    return lastError;
}

void XmlDocument::setLastError (const String& desc, const bool carryOn)
{
    if (errorOccurred)
        return;

    lastError = desc;
    errorOccurred = ! carryOn;
}

String XmlDocument::getFileContents (const String& filename) const
{
    // A document can name any file or URL as an external entity, so these are only fetched
    // through an input source that the caller has explicitly supplied
    if (externalEntitiesAllowed && inputSource != nullptr)
    {
        std::unique_ptr<InputStream> in (inputSource->createInputStreamFor (filename.trim().unquoted()));

        if (in != nullptr)
            return in->readEntireStreamAsString();
    }

    return {};
}

juce_wchar XmlDocument::readNextChar() noexcept
{
    auto c = input.getAndAdvance();

    if (c == 0)
    {
        outOfData = true;
        --input;
    }

    return c;
}

std::unique_ptr<XmlElement> XmlDocument::parseDocumentElement (String::CharPointerType textToParse,
                                                               bool onlyReadOuterDocumentElement)
{
    input = textToParse;
    errorOccurred = false;
    outOfData = false;
    needToLoadDTD = true;
    elementNestingDepth = 0;
    inputLength = (int64) textToParse.length();
    expandedEntityLength = 0;
    entitiesBeingExpanded.clear();

    if (textToParse.isEmpty())
    {
        lastError = "not enough input";
    }
    else if (! parseHeader())
    {
        lastError = "malformed header";
    }
    else if (! parseDTD())
    {
        lastError = "malformed DTD";
    }
    else
    {
        lastError.clear();
        std::unique_ptr<XmlElement> result (readNextElement (! onlyReadOuterDocumentElement));

        if (! errorOccurred)
            return result;
    }

    return {};
}

bool XmlDocument::parseHeader()
{
    skipNextWhiteSpace();

    if (CharacterFunctions::compareUpTo (input, CharPointer_ASCII ("<?xml"), 5) == 0)
    {
        auto headerEnd = CharacterFunctions::find (input, CharPointer_ASCII ("?>"));

        if (headerEnd.isEmpty())
            return false;

       #if JUCE_DEBUG
        auto encoding = String (input, headerEnd)
                          .fromFirstOccurrenceOf ("encoding", false, true)
                          .fromFirstOccurrenceOf ("=", false, false)
                          .fromFirstOccurrenceOf ("\"", false, false)
                          .upToFirstOccurrenceOf ("\"", false, false)
                          .trim();

        /* If you load an XML document with a non-UTF encoding type, it may have been
           loaded wrongly. Since all the files are read via the normal juce file streams,
           they're treated as UTF-8, so by the time it gets to the parser, the encoding will
           have been lost. Best plan is to stick to utf-8 or if you have specific files to
           read, use your own code to convert them to a unicode String, and pass that to the
           XML parser.
        */
        jassert (encoding.isEmpty() || encoding.startsWithIgnoreCase ("utf-"));
       #endif

        input = headerEnd + 2;
        skipNextWhiteSpace();
    }

    return true;
}

bool XmlDocument::parseDTD()
{
    if (CharacterFunctions::compareUpTo (input, CharPointer_ASCII ("<!DOCTYPE"), 9) == 0)
    {
        input += 9;
        auto dtdStart = input;

        for (int n = 1; n > 0;)
        {
            auto c = readNextChar();

            if (outOfData)
                return false;

            if (c == '<')
                ++n;
            else if (c == '>')
                --n;
        }

        dtdText = String (dtdStart, input - 1).trim();
    }

    return true;
}

template <typename CharPtr>
static auto skipIgnoredElement (CharPtr p)
{
    struct SkipIgnoredElementResult
    {
        CharPtr next;
        bool exhausted;
    };

    jassert (*p == '<');

    if (p[1] == '!' && p[2] == '-' && p[3] == '-')
    {
        p += 4;
        const auto close = p.indexOf (CharPointer_ASCII ("-->"));

        if (close < 0)
        {
            return SkipIgnoredElementResult { p.findTerminatingNull(), true };
        }

        return SkipIgnoredElementResult { p + close + 3, false };
    }

    if (p[1] == '?')
    {
        p += 2;
        const auto close = p.indexOf (CharPointer_ASCII ("?>"));

        if (close < 0)
        {
            return SkipIgnoredElementResult { p.findTerminatingNull(), true };
        }

        return SkipIgnoredElementResult { p + close + 2, false };
    }

    return SkipIgnoredElementResult { p, false };
}

void XmlDocument::skipNextWhiteSpace()
{
    for (;;)
    {
        input.incrementToEndOfWhitespace();

        if (input.isEmpty())
        {
            outOfData = true;
            break;
        }

        if (*input == '<')
        {
            const auto prevInput = input;
            const auto result = skipIgnoredElement (input);
            std::tie (input, outOfData) = std::tie (result.next, result.exhausted);

            if (outOfData || input == prevInput)
                break;

            continue;
        }

        break;
    }
}

void XmlDocument::readQuotedString (String& result)
{
    auto quote = readNextChar();

    while (! outOfData)
    {
        auto c = readNextChar();

        if (c == quote)
            break;

        --input;

        if (c == '&')
        {
            readEntity (result);
        }
        else
        {
            auto start = input;

            for (;;)
            {
                auto character = *input;

                if (character == quote)
                {
                    result.appendCharPointer (start, input);
                    ++input;
                    return;
                }

                if (character == '&')
                {
                    result.appendCharPointer (start, input);
                    break;
                }

                if (character == 0)
                {
                    setLastError ("unmatched quotes", false);
                    outOfData = true;
                    break;
                }

                ++input;
            }
        }
    }
}

XmlElement* XmlDocument::readNextElement (const bool alsoParseSubElements)
{
    XmlElement* node = nullptr;
    skipNextWhiteSpace();

    if (outOfData)
        return nullptr;

    if (*input == '<')
    {
        ++input;
        auto endOfToken = XmlIdentifierChars::findEndOfToken (input);

        if (endOfToken == input)
        {
            // no tag name - but allow for a gap after the '<' before giving an error
            skipNextWhiteSpace();
            endOfToken = XmlIdentifierChars::findEndOfToken (input);

            if (endOfToken == input)
            {
                setLastError ("tag name missing", false);
                return node;
            }
        }

        node = new XmlElement (input, endOfToken);
        input = endOfToken;
        LinkedListPointer<XmlElement::XmlAttributeNode>::Appender attributeAppender (node->attributes);

        // look for attributes
        for (;;)
        {
            skipNextWhiteSpace();
            auto c = *input;

            // empty tag
            if (c == '/' && input[1] == '>')
            {
                input += 2;
                break;
            }

            // parse the guts of the element
            if (c == '>')
            {
                ++input;

                if (alsoParseSubElements)
                {
                    if (XmlParsingLimits::maxElementNestingDepth <= elementNestingDepth)
                    {
                        setLastError ("elements nested too deeply", false);
                        outOfData = true;
                        return node;
                    }

                    ++elementNestingDepth;
                    readChildElements (*node);
                    --elementNestingDepth;
                }

                break;
            }

            // get an attribute
            if (XmlIdentifierChars::isIdentifierChar (c))
            {
                auto attNameEnd = XmlIdentifierChars::findEndOfToken (input);

                if (attNameEnd != input)
                {
                    auto attNameStart = input;
                    input = attNameEnd;
                    skipNextWhiteSpace();

                    if (readNextChar() == '=')
                    {
                        skipNextWhiteSpace();
                        auto nextChar = *input;

                        if (nextChar == '"' || nextChar == '\'')
                        {
                            auto* newAtt = new XmlElement::XmlAttributeNode (attNameStart, attNameEnd);
                            readQuotedString (newAtt->attribute.value);
                            attributeAppender.append (newAtt);
                            continue;
                        }
                    }
                    else
                    {
                        setLastError ("expected '=' after attribute '"
                                        + String (attNameStart, attNameEnd) + "'", false);
                        return node;
                    }
                }
            }
            else
            {
                if (! outOfData)
                    setLastError ("illegal character found in " + node->getTagName() + ": '" + c + "'", false);
            }

            break;
        }
    }

    return node;
}

void XmlDocument::readChildElements (XmlElement& parent)
{
    LinkedListPointer<XmlElement>::Appender childAppender (parent.firstChildElement);

    for (;;)
    {
        auto preWhitespaceInput = input;
        skipNextWhiteSpace();

        if (outOfData)
        {
            setLastError ("unmatched tags", false);
            break;
        }

        if (*input == '<')
        {
            auto c1 = input[1];

            if (c1 == '/')
            {
                // our close tag
                auto closeTag = input.indexOf ((juce_wchar) '>');

                if (closeTag >= 0)
                    input += closeTag + 1;

                break;
            }

            if (c1 == '!' && CharacterFunctions::compareUpTo (input + 2, CharPointer_ASCII ("[CDATA["), 7) == 0)
            {
                input += 9;
                auto inputStart = input;

                for (;;)
                {
                    auto c0 = *input;

                    if (c0 == 0)
                    {
                        setLastError ("unterminated CDATA section", false);
                        outOfData = true;
                        break;
                    }

                    if (c0 == ']' && input[1] == ']' && input[2] == '>')
                    {
                        childAppender.append (XmlElement::createTextElement (String (inputStart, input)));
                        input += 3;
                        break;
                    }

                    ++input;
                }
            }
            else
            {
                // this is some other element, so parse and add it
                if (auto* n = readNextElement (true))
                    childAppender.append (n);
                else
                    break;
            }
        }
        else  // must be a character block
        {
            input = preWhitespaceInput; // roll back to include the leading whitespace
            MemoryOutputStream textElementContent;
            bool contentShouldBeUsed = ! ignoreEmptyTextElements;

            for (;;)
            {
                auto c = *input;

                if (c == '<')
                {
                    const auto prevInput = input;
                    const auto result = skipIgnoredElement (input);
                    std::tie (input, outOfData) = std::tie (result.next, result.exhausted);

                    if (outOfData)
                    {
                        setLastError ("unexpected end of stream", false);
                        return;
                    }

                    if (prevInput == input)
                        break;

                    continue;
                }

                if (c == 0)
                {
                    setLastError ("unmatched tags", false);
                    outOfData = true;
                    return;
                }

                if (c == '&')
                {
                    String entity;
                    const auto entityName = readEntity (entity);

                    if (entityTextIsMarkup (entity))
                    {
                        auto oldInput = input;
                        auto oldOutOfData = outOfData;

                        input = entity.getCharPointer();
                        outOfData = false;

                        // References inside markup are only resolved while the text is parsed, so the
                        // entity stays marked as expanding until then
                        entitiesBeingExpanded.add (entityName);

                        while (auto* n = readNextElement (true))
                            childAppender.append (n);

                        entitiesBeingExpanded.removeString (entityName);

                        input = oldInput;
                        outOfData = oldOutOfData;
                    }
                    else
                    {
                        textElementContent << entity;
                        contentShouldBeUsed = contentShouldBeUsed || entity.containsNonWhitespaceChars();
                    }
                }
                else
                {
                    for (;; ++input)
                    {
                        auto nextChar = *input;

                        if (nextChar == '\r')
                        {
                            nextChar = '\n';

                            if (input[1] == '\n')
                                continue;
                        }

                        if (nextChar == '<' || nextChar == '&')
                            break;

                        if (nextChar == 0)
                        {
                            setLastError ("unmatched tags", false);
                            outOfData = true;
                            return;
                        }

                        textElementContent.appendUTF8Char (nextChar);
                        contentShouldBeUsed = contentShouldBeUsed || ! CharacterFunctions::isWhitespace (nextChar);
                    }
                }
            }

            if (contentShouldBeUsed)
                childAppender.append (XmlElement::createTextElement (textElementContent.toUTF8()));
        }
    }
}

String XmlDocument::readEntity (String& result)
{
    // skip over the ampersand
    ++input;

    if (input.compareIgnoreCaseUpTo (CharPointer_ASCII ("amp;"), 4) == 0)
    {
        input += 4;
        result += '&';
    }
    else if (input.compareIgnoreCaseUpTo (CharPointer_ASCII ("quot;"), 5) == 0)
    {
        input += 5;
        result += '"';
    }
    else if (input.compareIgnoreCaseUpTo (CharPointer_ASCII ("apos;"), 5) == 0)
    {
        input += 5;
        result += '\'';
    }
    else if (input.compareIgnoreCaseUpTo (CharPointer_ASCII ("lt;"), 3) == 0)
    {
        input += 3;
        result += '<';
    }
    else if (input.compareIgnoreCaseUpTo (CharPointer_ASCII ("gt;"), 3) == 0)
    {
        input += 3;
        result += '>';
    }
    else if (*input == '#')
    {
        int64_t charCode = 0;
        ++input;

        if (*input == 'x' || *input == 'X')
        {
            ++input;
            int numChars = 0;

            while (input[0] != ';')
            {
                auto hexValue = CharacterFunctions::getHexDigitValue (input[0]);

                if (hexValue < 0 || ++numChars > 8)
                {
                    setLastError ("illegal escape sequence", true);
                    break;
                }

                charCode = (charCode << 4) | hexValue;
                ++input;
            }

            ++input;
        }
        else if (input[0] >= '0' && input[0] <= '9')
        {
            int numChars = 0;

            for (;;)
            {
                const auto firstChar = input[0];

                if (firstChar == 0)
                {
                    setLastError ("unexpected end of input", true);
                    return {};
                }

                if (firstChar == ';')
                    break;

                if (++numChars > 12)
                {
                    setLastError ("illegal escape sequence", true);
                    break;
                }

                charCode = charCode * 10 + ((int) firstChar - '0');
                ++input;
            }

            ++input;
        }
        else
        {
            setLastError ("illegal escape sequence", true);
            result += '&';
            return {};
        }

        result << (juce_wchar) charCode;
    }
    else
    {
        auto entityNameStart = input;
        auto closingSemiColon = input.indexOf ((juce_wchar) ';');

        if (closingSemiColon < 0)
        {
            outOfData = true;
            result += '&';
        }
        else
        {
            input += closingSemiColon + 1;
            const String entityName (entityNameStart, (size_t) closingSemiColon);
            result += expandExternalEntity (entityName);
            return entityName;
        }
    }

    return {};
}

String XmlDocument::expandEntity (const String& ent)
{
    if (ent.equalsIgnoreCase ("amp"))   return String::charToString ('&');
    if (ent.equalsIgnoreCase ("quot"))  return String::charToString ('"');
    if (ent.equalsIgnoreCase ("apos"))  return String::charToString ('\'');
    if (ent.equalsIgnoreCase ("lt"))    return String::charToString ('<');
    if (ent.equalsIgnoreCase ("gt"))    return String::charToString ('>');

    if (ent[0] == '#')
    {
        auto char1 = ent[1];

        if (char1 == 'x' || char1 == 'X')
            return String::charToString (static_cast<juce_wchar> (ent.substring (2).getHexValue32()));

        if (char1 >= '0' && char1 <= '9')
            return String::charToString (static_cast<juce_wchar> (ent.substring (1).getIntValue()));

        setLastError ("illegal escape sequence", false);
        return String::charToString ('&');
    }

    return expandExternalEntity (ent);
}

String XmlDocument::expandExternalEntity (const String& entity)
{
    if (errorOccurred)
        return {};

    if (needToLoadDTD)
    {
        if (dtdText.isNotEmpty())
        {
            dtdText = dtdText.trimCharactersAtEnd (">");
            tokenisedDTD.addTokens (dtdText, true);

            if (tokenisedDTD[tokenisedDTD.size() - 2].equalsIgnoreCase ("system")
                 && tokenisedDTD[tokenisedDTD.size() - 1].isQuotedString())
            {
                auto fn = tokenisedDTD[tokenisedDTD.size() - 1];

                tokenisedDTD.clear();
                tokenisedDTD.addTokens (getFileContents (fn), true);
            }
            else
            {
                tokenisedDTD.clear();
                auto openBracket = dtdText.indexOfChar ('[');

                if (openBracket > 0)
                {
                    auto closeBracket = dtdText.lastIndexOfChar (']');

                    if (closeBracket > openBracket)
                        tokenisedDTD.addTokens (dtdText.substring (openBracket + 1,
                                                                   closeBracket), true);
                }
            }

            for (int i = tokenisedDTD.size(); --i >= 0;)
            {
                if (tokenisedDTD[i].startsWithChar ('%')
                     && tokenisedDTD[i].endsWithChar (';'))
                {
                    auto parsed = getParameterEntity (tokenisedDTD[i].substring (1, tokenisedDTD[i].length() - 1));
                    checkEntityExpansionLimit (parsed);

                    if (errorOccurred)
                        break;

                    StringArray newToks;
                    newToks.addTokens (parsed, true);

                    tokenisedDTD.remove (i);

                    for (int j = newToks.size(); --j >= 0;)
                        tokenisedDTD.insert (i, newToks[j]);
                }
            }
        }

        needToLoadDTD = false;
    }

    for (int i = 0; i < tokenisedDTD.size(); ++i)
    {
        if (tokenisedDTD[i] == entity)
        {
            if (tokenisedDTD[i - 1].equalsIgnoreCase ("<!entity"))
            {
                if (entitiesBeingExpanded.contains (entity))
                {
                    setLastError ("recursive entity reference", false);
                    outOfData = true;
                    return {};
                }

                if (XmlParsingLimits::maxEntityNestingDepth <= entitiesBeingExpanded.size())
                {
                    setLastError ("entities nested too deeply", false);
                    outOfData = true;
                    return {};
                }

                entitiesBeingExpanded.add (entity);
                auto ent = tokenisedDTD [i + 1].trimCharactersAtEnd (">").trim().unquoted();

                // check for sub-entities
                auto ampersand = ent.indexOfChar ('&');

                while (ampersand >= 0 && ! errorOccurred)
                {
                    auto semiColon = ent.indexOf (ampersand + 1, ";");

                    if (semiColon < 0)
                    {
                        setLastError ("entity without terminating semi-colon", false);
                        break;
                    }

                    const auto reference = ent.substring (ampersand + 1, semiColon);

                    // The content parser resolves general entity references in markup, so
                    // expanding them here too would turn an escaped "&amp;a;" into a live
                    // reference. Character references are part of the text and are always expanded.
                    if (reference[0] != '#' && entityTextIsMarkup (ent))
                    {
                        ampersand = ent.indexOfChar (semiColon + 1, '&');
                        continue;
                    }

                    auto resolved = expandEntity (reference);

                    ent = ent.substring (0, ampersand)
                           + resolved
                           + ent.substring (semiColon + 1);

                    ampersand = ent.indexOfChar (ampersand + resolved.length(), '&');
                }

                entitiesBeingExpanded.removeString (entity);

                if (! errorOccurred)
                    checkEntityExpansionLimit (ent);

                return errorOccurred ? String() : ent;
            }
        }
    }

    setLastError ("unknown entity", true);
    return entity;
}

void XmlDocument::checkEntityExpansionLimit (const String& expandedText)
{
    expandedEntityLength += (int64) expandedText.length();

    if (XmlParsingLimits::entityAmplificationThreshold < expandedEntityLength
         && inputLength * XmlParsingLimits::maxEntityAmplification < expandedEntityLength)
    {
        setLastError ("entity expansion too large", false);
        outOfData = true;
    }
}

String XmlDocument::getParameterEntity (const String& entity)
{
    for (int i = 0; i < tokenisedDTD.size(); ++i)
    {
        if (tokenisedDTD[i] == entity
             && tokenisedDTD [i - 1] == "%"
             && tokenisedDTD [i - 2].equalsIgnoreCase ("<!entity"))
        {
            auto ent = tokenisedDTD [i + 1].trimCharactersAtEnd (">");

            if (ent.equalsIgnoreCase ("system"))
                return getFileContents (tokenisedDTD [i + 2].trimCharactersAtEnd (">"));

            return ent.trim().unquoted();
        }
    }

    return entity;
}

//==============================================================================
#if JUCE_UNIT_TESTS

class XmlDocumentTests final : public UnitTest
{
public:
    XmlDocumentTests()
        : UnitTest ("XmlDocument", UnitTestCategories::xml)
    {}

    void runTest() override
    {
        testCase ("Entities referenced from other entities are expanded", [&]
        {
            const auto xml = parseXML ("<!DOCTYPE doc [<!ENTITY a \"1\"> <!ENTITY b \"&a;2&a;\">]><doc>&b;</doc>");
            expect (xml != nullptr);

            if (xml != nullptr)
                expectEquals (xml->getAllSubText(), String ("121"));
        });

        testCase ("Entities that refer to themselves are rejected", [&]
        {
            expectParseFailure ("<!DOCTYPE doc [<!ENTITY a \"x&a;\">]><doc>&a;</doc>", "recursive entity reference");
            expectParseFailure ("<!DOCTYPE doc [<!ENTITY a \"&b;\"> <!ENTITY b \"<e>&a;</e>\">]><doc>&a;</doc>", "recursive entity reference");
            expectParseFailure ("<!DOCTYPE doc [<!ENTITY a \"<e>&#38;a;</e>\">]><doc>&a;</doc>", "recursive entity reference");

            // A recoverable error before or after the fatal one, or an attribute value, must not hide it
            expectParseFailure ("<!DOCTYPE doc [<!ENTITY a \"x&a;\">]><doc>&a;&nope;</doc>", "recursive entity reference");
            expectParseFailure ("<!DOCTYPE doc [<!ENTITY a \"x&a;\">]><doc>&nope;&a;</doc>", "recursive entity reference");
            expectParseFailure ("<!DOCTYPE doc [<!ENTITY a \"x&a;\">]><doc y=\"&a;\"/>", "recursive entity reference");
        });

        testCase ("Escaped references inside entity markup are not expanded again", [&]
        {
            const auto markup = parseXML ("<!DOCTYPE doc [<!ENTITY a \"<e x='&amp;a;'>&amp;a; &lt;b&gt;</e>\">]><doc>&a;</doc>");
            expect (markup != nullptr);

            if (markup != nullptr)
            {
                expectEquals (markup->getAllSubText(), String ("&a; <b>"));

                auto* e = markup->getChildByName ("e");
                expect (e != nullptr);

                if (e != nullptr)
                    expectEquals (e->getStringAttribute ("x"), String ("&a;"));
            }

            const auto text = parseXML ("<!DOCTYPE doc [<!ENTITY a \"x &amp; y\">]><doc>&a;</doc>");
            expect (text != nullptr);

            if (text != nullptr)
                expectEquals (text->getAllSubText(), String ("x & y"));
        });

        testCase ("Deeply nested entity references are rejected", [&]
        {
            const auto chainedDocument = [] (int chainLength)
            {
                String dtd ("<!ENTITY e0 \"x\">");

                for (int i = 1; i < chainLength; ++i)
                    dtd << " <!ENTITY e" << i << " \"&e" << (i - 1) << ";\">";

                return "<!DOCTYPE doc [" + dtd + "]><doc>&e" + String (chainLength - 1) + ";</doc>";
            };

            expect (parseXML (chainedDocument (XmlParsingLimits::maxEntityNestingDepth)) != nullptr);
            expectParseFailure (chainedDocument (XmlParsingLimits::maxEntityNestingDepth + 1), "entities nested too deeply");
        });

        testCase ("Entity expansion that grows far beyond the document is rejected", [&]
        {
            const auto doublingDocument = [] (int numDoublings)
            {
                String dtd ("<!ENTITY e0 \"" + String::repeatedString ("x", 1024) + "\">");

                for (int i = 1; i <= numDoublings; ++i)
                    dtd << " <!ENTITY e" << i << " \"&e" << (i - 1) << ";&e" << (i - 1) << ";\">";

                return "<!DOCTYPE doc [" + dtd + "]><doc>&e" + String (numDoublings) + ";</doc>";
            };

            expect (parseXML (doublingDocument (9)) != nullptr);
            expectParseFailure (doublingDocument (10), "entity expansion too large");
        });

        testCase ("Deeply nested elements are rejected", [&]
        {
            const auto nestedDocument = [] (int depth)
            {
                return String::repeatedString ("<e>", depth) + String::repeatedString ("</e>", depth);
            };

            expect (parseXML (nestedDocument (XmlParsingLimits::maxElementNestingDepth)) != nullptr);
            expectParseFailure (nestedDocument (XmlParsingLimits::maxElementNestingDepth + 1), "elements nested too deeply");
        });
    }

private:
    void expectParseFailure (const String& documentText, const String& expectedError)
    {
        XmlDocument document (documentText);
        expect (document.getDocumentElement() == nullptr);
        expectEquals (document.getLastParseError(), expectedError);
    }
};

static XmlDocumentTests xmlDocumentTests;

#endif

}
