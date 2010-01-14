/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

#include "../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_XmlDocument.h"
#include "../io/streams/juce_FileInputSource.h"


//==============================================================================
XmlDocument::XmlDocument (const String& documentText) throw()
    : originalText (documentText),
      ignoreEmptyTextElements (true)
{
}

XmlDocument::XmlDocument (const File& file)
{
    inputSource = new FileInputSource (file);
}

XmlDocument::~XmlDocument() throw()
{
}

void XmlDocument::setInputSource (InputSource* const newSource) throw()
{
    inputSource = newSource;
}

void XmlDocument::setEmptyTextElementsIgnored (const bool shouldBeIgnored) throw()
{
    ignoreEmptyTextElements = shouldBeIgnored;
}

bool XmlDocument::isXmlIdentifierCharSlow (const tchar c) throw()
{
    return CharacterFunctions::isLetterOrDigit (c)
            || c == T('_')
            || c == T('-')
            || c == T(':')
            || c == T('.');
}

inline bool XmlDocument::isXmlIdentifierChar (const tchar c) const throw()
{
    return (c > 0 && c <= 127) ? identifierLookupTable [(int) c]
                               : isXmlIdentifierCharSlow (c);
}

XmlElement* XmlDocument::getDocumentElement (const bool onlyReadOuterDocumentElement)
{
    String textToParse (originalText);

    if (textToParse.isEmpty() && inputSource != 0)
    {
        ScopedPointer <InputStream> in (inputSource->createInputStream());

        if (in != 0)
        {
            MemoryBlock data;

            in->readIntoMemoryBlock (data, onlyReadOuterDocumentElement ? 8192 : -1);

            if (data.getSize() >= 2
                 && ((data[0] == (char)-2 && data[1] == (char)-1)
                      || (data[0] == (char)-1 && data[1] == (char)-2)))
            {
                textToParse = String::createStringFromData ((const char*) data.getData(), (int) data.getSize());
            }
            else
            {
                textToParse = String::fromUTF8 ((const uint8*) data.getData(), (int) data.getSize());
            }

            if (! onlyReadOuterDocumentElement)
                originalText = textToParse;
        }
    }

    input = textToParse;
    lastError = String::empty;
    errorOccurred = false;
    outOfData = false;
    needToLoadDTD = true;

    for (int i = 0; i < 128; ++i)
        identifierLookupTable[i] = isXmlIdentifierCharSlow ((tchar) i);

    if (textToParse.isEmpty())
    {
        lastError = "not enough input";
    }
    else
    {
        skipHeader();

        if (input != 0)
        {
            ScopedPointer <XmlElement> result (readNextElement (! onlyReadOuterDocumentElement));

            if (! errorOccurred)
                return result.release();
        }
        else
        {
            lastError = "incorrect xml header";
        }
    }

    return 0;
}

const String& XmlDocument::getLastParseError() const throw()
{
    return lastError;
}

void XmlDocument::setLastError (const String& desc, const bool carryOn) throw()
{
    lastError = desc;
    errorOccurred = ! carryOn;
}

const String XmlDocument::getFileContents (const String& filename) const
{
    if (inputSource != 0)
    {
        const ScopedPointer <InputStream> in (inputSource->createInputStreamFor (filename.trim().unquoted()));

        if (in != 0)
            return in->readEntireStreamAsString();
    }

    return String::empty;
}

tchar XmlDocument::readNextChar() throw()
{
    if (*input != 0)
    {
        return *input++;
    }
    else
    {
        outOfData = true;
        return 0;
    }
}

int XmlDocument::findNextTokenLength() throw()
{
    int len = 0;
    tchar c = *input;

    while (isXmlIdentifierChar (c))
        c = input [++len];

    return len;
}

void XmlDocument::skipHeader() throw()
{
    const tchar* const found = CharacterFunctions::find (input, T("<?xml"));

    if (found != 0)
    {
        input = found;
        input = CharacterFunctions::find (input, T("?>"));

        if (input == 0)
            return;

        input += 2;
    }

    skipNextWhiteSpace();
    const tchar* docType = CharacterFunctions::find (input, T("<!DOCTYPE"));

    if (docType == 0)
        return;

    input = docType + 9;

    int n = 1;

    while (n > 0)
    {
        const tchar c = readNextChar();

        if (outOfData)
            return;

        if (c == T('<'))
            ++n;
        else if (c == T('>'))
            --n;
    }

    docType += 9;
    dtdText = String (docType, (int) (input - (docType + 1))).trim();
}

void XmlDocument::skipNextWhiteSpace() throw()
{
    for (;;)
    {
        tchar c = *input;

        while (CharacterFunctions::isWhitespace (c))
            c = *++input;

        if (c == 0)
        {
            outOfData = true;
            break;
        }
        else if (c == T('<'))
        {
            if (input[1] == T('!')
                 && input[2] == T('-')
                 && input[3] == T('-'))
            {
                const tchar* const closeComment = CharacterFunctions::find (input, T("-->"));

                if (closeComment == 0)
                {
                    outOfData = true;
                    break;
                }

                input = closeComment + 3;
                continue;
            }
            else if (input[1] == T('?'))
            {
                const tchar* const closeBracket = CharacterFunctions::find (input, T("?>"));

                if (closeBracket == 0)
                {
                    outOfData = true;
                    break;
                }

                input = closeBracket + 2;
                continue;
            }
        }

        break;
    }
}

void XmlDocument::readQuotedString (String& result) throw()
{
    const tchar quote = readNextChar();

    while (! outOfData)
    {
        const tchar c = readNextChar();

        if (c == quote)
            break;

        if (c == T('&'))
        {
            --input;
            readEntity (result);
        }
        else
        {
            --input;
            const tchar* const start = input;

            for (;;)
            {
                const tchar character = *input;

                if (character == quote)
                {
                    result.append (start, (int) (input - start));
                    ++input;

                    return;
                }
                else if (character == T('&'))
                {
                    result.append (start, (int) (input - start));
                    break;
                }
                else if (character == 0)
                {
                    outOfData = true;
                    setLastError ("unmatched quotes", false);
                    break;
                }

                ++input;
            }
        }
    }
}

XmlElement* XmlDocument::readNextElement (const bool alsoParseSubElements) throw()
{
    XmlElement* node = 0;

    skipNextWhiteSpace();
    if (outOfData)
        return 0;

    input = CharacterFunctions::find (input, T("<"));

    if (input != 0)
    {
        ++input;
        int tagLen = findNextTokenLength();

        if (tagLen == 0)
        {
            // no tag name - but allow for a gap after the '<' before giving an error
            skipNextWhiteSpace();
            tagLen = findNextTokenLength();

            if (tagLen == 0)
            {
                setLastError ("tag name missing", false);
                return node;
            }
        }

        node = new XmlElement (input, tagLen);
        input += tagLen;
        XmlElement::XmlAttributeNode* lastAttribute = 0;

        // look for attributes
        for (;;)
        {
            skipNextWhiteSpace();

            const tchar c = *input;

            // empty tag..
            if (c == T('/') && input[1] == T('>'))
            {
                input += 2;
                break;
            }

            // parse the guts of the element..
            if (c == T('>'))
            {
                ++input;
                skipNextWhiteSpace();

                if (alsoParseSubElements)
                    readChildElements (node);

                break;
            }

            // get an attribute..
            if (isXmlIdentifierChar (c))
            {
                const int attNameLen = findNextTokenLength();

                if (attNameLen > 0)
                {
                    const tchar* attNameStart = input;
                    input += attNameLen;

                    skipNextWhiteSpace();

                    if (readNextChar() == T('='))
                    {
                        skipNextWhiteSpace();

                        const tchar nextChar = *input;

                        if (nextChar == T('"') || nextChar == T('\''))
                        {
                            XmlElement::XmlAttributeNode* const newAtt
                                = new XmlElement::XmlAttributeNode (String (attNameStart, attNameLen),
                                                                    String::empty);

                            readQuotedString (newAtt->value);

                            if (lastAttribute == 0)
                                node->attributes = newAtt;
                            else
                                lastAttribute->next = newAtt;

                            lastAttribute = newAtt;

                            continue;
                        }
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

void XmlDocument::readChildElements (XmlElement* parent) throw()
{
    XmlElement* lastChildNode = 0;

    for (;;)
    {
        skipNextWhiteSpace();

        if (outOfData)
        {
            setLastError ("unmatched tags", false);
            break;
        }

        if (*input == T('<'))
        {
            if (input[1] == T('/'))
            {
                // our close tag..
                input = CharacterFunctions::find (input, T(">"));
                ++input;
                break;
            }
            else if (input[1] == T('!')
                  && input[2] == T('[')
                  && input[3] == T('C')
                  && input[4] == T('D')
                  && input[5] == T('A')
                  && input[6] == T('T')
                  && input[7] == T('A')
                  && input[8] == T('['))
            {
                input += 9;
                const tchar* const inputStart = input;

                int len = 0;

                for (;;)
                {
                    if (*input == 0)
                    {
                        setLastError ("unterminated CDATA section", false);
                        outOfData = true;
                        break;
                    }
                    else if (input[0] == T(']')
                             && input[1] == T(']')
                             && input[2] == T('>'))
                    {
                        input += 3;
                        break;
                    }

                    ++input;
                    ++len;
                }

                XmlElement* const e = new XmlElement ((int) 0);
                e->setText (String (inputStart, len));

                if (lastChildNode != 0)
                    lastChildNode->nextElement = e;
                else
                    parent->addChildElement (e);

                lastChildNode = e;
            }
            else
            {
                // this is some other element, so parse and add it..
                XmlElement* const n = readNextElement (true);

                if (n != 0)
                {
                    if (lastChildNode == 0)
                        parent->addChildElement (n);
                    else
                        lastChildNode->nextElement = n;

                    lastChildNode = n;
                }
                else
                {
                    return;
                }
            }
        }
        else
        {
            // read character block..
            XmlElement* const e = new XmlElement ((int)0);

            if (lastChildNode != 0)
                lastChildNode->nextElement = e;
            else
                parent->addChildElement (e);

            lastChildNode = e;

            String textElementContent;

            for (;;)
            {
                const tchar c = *input;

                if (c == T('<'))
                    break;

                if (c == 0)
                {
                    setLastError ("unmatched tags", false);
                    outOfData = true;
                    return;
                }

                if (c == T('&'))
                {
                    String entity;
                    readEntity (entity);

                    if (entity.startsWithChar (T('<')) && entity [1] != 0)
                    {
                        const tchar* const oldInput = input;
                        const bool oldOutOfData = outOfData;

                        input = (const tchar*) entity;
                        outOfData = false;

                        for (;;)
                        {
                            XmlElement* const n = readNextElement (true);

                            if (n == 0)
                                break;

                            if (lastChildNode == 0)
                                parent->addChildElement (n);
                            else
                                lastChildNode->nextElement = n;

                            lastChildNode = n;
                        }

                        input = oldInput;
                        outOfData = oldOutOfData;
                    }
                    else
                    {
                        textElementContent += entity;
                    }
                }
                else
                {
                    const tchar* start = input;
                    int len = 0;

                    for (;;)
                    {
                        const tchar nextChar = *input;

                        if (nextChar == T('<') || nextChar == T('&'))
                        {
                            break;
                        }
                        else if (nextChar == 0)
                        {
                            setLastError ("unmatched tags", false);
                            outOfData = true;
                            return;
                        }

                        ++input;
                        ++len;
                    }

                    textElementContent.append (start, len);
                }
            }

            if (ignoreEmptyTextElements ? textElementContent.containsNonWhitespaceChars()
                                        : textElementContent.isNotEmpty())
                e->setText (textElementContent);
        }
    }
}

void XmlDocument::readEntity (String& result) throw()
{
    // skip over the ampersand
    ++input;

    if (CharacterFunctions::compareIgnoreCase (input, T("amp;"), 4) == 0)
    {
        input += 4;
        result += T("&");
    }
    else if (CharacterFunctions::compareIgnoreCase (input, T("quot;"), 5) == 0)
    {
        input += 5;
        result += T("\"");
    }
    else if (CharacterFunctions::compareIgnoreCase (input, T("apos;"), 5) == 0)
    {
        input += 5;
        result += T("\'");
    }
    else if (CharacterFunctions::compareIgnoreCase (input, T("lt;"), 3) == 0)
    {
        input += 3;
        result += T("<");
    }
    else if (CharacterFunctions::compareIgnoreCase (input, T("gt;"), 3) == 0)
    {
        input += 3;
        result += T(">");
    }
    else if (*input == T('#'))
    {
        int charCode = 0;
        ++input;

        if (*input == T('x') || *input == T('X'))
        {
            ++input;
            int numChars = 0;

            while (input[0] != T(';'))
            {
                const int hexValue = CharacterFunctions::getHexDigitValue (input[0]);

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
        else if (input[0] >= T('0') && input[0] <= T('9'))
        {
            int numChars = 0;

            while (input[0] != T(';'))
            {
                if (++numChars > 12)
                {
                    setLastError ("illegal escape sequence", true);
                    break;
                }

                charCode = charCode * 10 + (input[0] - T('0'));
                ++input;
            }

            ++input;
        }
        else
        {
            setLastError ("illegal escape sequence", true);
            result += T("&");
            return;
        }

        result << (tchar) charCode;
    }
    else
    {
        const tchar* const entityNameStart = input;
        const tchar* const closingSemiColon = CharacterFunctions::find (input, T(";"));

        if (closingSemiColon == 0)
        {
            outOfData = true;
            result += T("&");
        }
        else
        {
            input = closingSemiColon + 1;

            result += expandExternalEntity (String (entityNameStart,
                                                    (int) (closingSemiColon - entityNameStart)));
        }
    }
}

const String XmlDocument::expandEntity (const String& ent)
{
    if (ent.equalsIgnoreCase (T("amp")))
    {
        return T("&");
    }
    else if (ent.equalsIgnoreCase (T("quot")))
    {
        return T("\"");
    }
    else if (ent.equalsIgnoreCase (T("apos")))
    {
        return T("\'");
    }
    else if (ent.equalsIgnoreCase (T("lt")))
    {
        return T("<");
    }
    else if (ent.equalsIgnoreCase (T("gt")))
    {
        return T(">");
    }
    else if (ent[0] == T('#'))
    {
        if (ent[1] == T('x') || ent[1] == T('X'))
        {
            return String::charToString ((tchar) ent.substring (2).getHexValue32());
        }
        else if (ent[1] >= T('0') && ent[1] <= T('9'))
        {
            return String::charToString ((tchar) ent.substring (1).getIntValue());
        }

        setLastError ("illegal escape sequence", false);
        return T("&");
    }
    else
    {
        return expandExternalEntity (ent);
    }
}

const String XmlDocument::expandExternalEntity (const String& entity)
{
    if (needToLoadDTD)
    {
        if (dtdText.isNotEmpty())
        {
            while (dtdText.endsWithChar (T('>')))
                dtdText = dtdText.dropLastCharacters (1);

            tokenisedDTD.addTokens (dtdText, true);

            if (tokenisedDTD [tokenisedDTD.size() - 2].equalsIgnoreCase (T("system"))
                 && tokenisedDTD [tokenisedDTD.size() - 1].isQuotedString())
            {
                const String fn (tokenisedDTD [tokenisedDTD.size() - 1]);

                tokenisedDTD.clear();
                tokenisedDTD.addTokens (getFileContents (fn), true);
            }
            else
            {
                tokenisedDTD.clear();
                const int openBracket = dtdText.indexOfChar (T('['));

                if (openBracket > 0)
                {
                    const int closeBracket = dtdText.lastIndexOfChar (T(']'));

                    if (closeBracket > openBracket)
                        tokenisedDTD.addTokens (dtdText.substring (openBracket + 1,
                                                                   closeBracket), true);
                }
            }

            for (int i = tokenisedDTD.size(); --i >= 0;)
            {
                if (tokenisedDTD[i].startsWithChar (T('%'))
                     && tokenisedDTD[i].endsWithChar (T(';')))
                {
                    const String parsed (getParameterEntity (tokenisedDTD[i].substring (1, tokenisedDTD[i].length() - 1)));
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
            if (tokenisedDTD[i - 1].equalsIgnoreCase (T("<!entity")))
            {
                String ent (tokenisedDTD [i + 1]);

                while (ent.endsWithChar (T('>')))
                    ent = ent.dropLastCharacters (1);

                ent = ent.trim().unquoted();

                // check for sub-entities..
                int ampersand = ent.indexOfChar (T('&'));

                while (ampersand >= 0)
                {
                    const int semiColon = ent.indexOf (i + 1, T(";"));

                    if (semiColon < 0)
                    {
                        setLastError ("entity without terminating semi-colon", false);
                        break;
                    }

                    const String resolved (expandEntity (ent.substring (i + 1, semiColon)));

                    ent = ent.substring (0, ampersand)
                           + resolved
                           + ent.substring (semiColon + 1);

                    ampersand = ent.indexOfChar (semiColon + 1, T('&'));
                }

                return ent;
            }
        }
    }

    setLastError ("unknown entity", true);

    return entity;
}

const String XmlDocument::getParameterEntity (const String& entity)
{
    for (int i = 0; i < tokenisedDTD.size(); ++i)
    {
        if (tokenisedDTD[i] == entity)
        {
            if (tokenisedDTD [i - 1] == T("%")
                && tokenisedDTD [i - 2].equalsIgnoreCase (T("<!entity")))
            {
                String ent (tokenisedDTD [i + 1]);

                while (ent.endsWithChar (T('>')))
                    ent = ent.dropLastCharacters (1);

                if (ent.equalsIgnoreCase (T("system")))
                {
                    String filename (tokenisedDTD [i + 2]);

                    while (filename.endsWithChar (T('>')))
                        filename = filename.dropLastCharacters (1);

                    return getFileContents (filename);
                }
                else
                {
                    return ent.trim().unquoted();
                }
            }
        }
    }

    return entity;
}


END_JUCE_NAMESPACE
