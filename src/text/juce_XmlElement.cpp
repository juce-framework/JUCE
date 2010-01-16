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


#include "juce_XmlElement.h"
#include "../io/streams/juce_MemoryOutputStream.h"
#include "../io/files/juce_TemporaryFile.h"
#include "../threads/juce_Thread.h"
#include "../containers/juce_ScopedPointer.h"


//==============================================================================
XmlElement::XmlAttributeNode::XmlAttributeNode (const XmlAttributeNode& other) throw()
    : name (other.name),
      value (other.value),
      next (0)
{
}

XmlElement::XmlAttributeNode::XmlAttributeNode (const String& name_,
                                                const String& value_) throw()
    : name (name_),
      value (value_),
      next (0)
{
}

//==============================================================================
XmlElement::XmlElement (const String& tagName_) throw()
    : tagName (tagName_),
      firstChildElement (0),
      nextElement (0),
      attributes (0)
{
    // the tag name mustn't be empty, or it'll look like a text element!
    jassert (tagName_.containsNonWhitespaceChars())
}

XmlElement::XmlElement (int /*dummy*/) throw()
    : firstChildElement (0),
      nextElement (0),
      attributes (0)
{
}

XmlElement::XmlElement (const tchar* const tagName_,
                        const int nameLen) throw()
    : tagName (tagName_, nameLen),
      firstChildElement (0),
      nextElement (0),
      attributes (0)
{
}

XmlElement::XmlElement (const XmlElement& other) throw()
    : tagName (other.tagName),
      firstChildElement (0),
      nextElement (0),
      attributes (0)
{
    copyChildrenAndAttributesFrom (other);
}

const XmlElement& XmlElement::operator= (const XmlElement& other) throw()
{
    if (this != &other)
    {
        removeAllAttributes();
        deleteAllChildElements();

        tagName = other.tagName;

        copyChildrenAndAttributesFrom (other);
    }

    return *this;
}

void XmlElement::copyChildrenAndAttributesFrom (const XmlElement& other) throw()
{
    XmlElement* child = other.firstChildElement;
    XmlElement* lastChild = 0;

    while (child != 0)
    {
        XmlElement* const copiedChild = new XmlElement (*child);

        if (lastChild != 0)
            lastChild->nextElement = copiedChild;
        else
            firstChildElement = copiedChild;

        lastChild = copiedChild;
        child = child->nextElement;
    }

    const XmlAttributeNode* att = other.attributes;
    XmlAttributeNode* lastAtt = 0;

    while (att != 0)
    {
        XmlAttributeNode* const newAtt = new XmlAttributeNode (*att);

        if (lastAtt != 0)
            lastAtt->next = newAtt;
        else
            attributes = newAtt;

        lastAtt = newAtt;
        att = att->next;
    }
}

XmlElement::~XmlElement() throw()
{
    XmlElement* child = firstChildElement;

    while (child != 0)
    {
        XmlElement* const nextChild = child->nextElement;
        delete child;
        child = nextChild;
    }

    XmlAttributeNode* att = attributes;

    while (att != 0)
    {
        XmlAttributeNode* const nextAtt = att->next;
        delete att;
        att = nextAtt;
    }
}

//==============================================================================
static bool isLegalXmlChar (const juce_wchar character)
{
    if ((character >= 'a' && character <= 'z')
         || (character >= 'A' && character <= 'Z')
            || (character >= '0' && character <= '9'))
        return true;

    const char* t = " .,;:-()_+=?!'#@[]/\\*%~{}";

    do
    {
        if (((juce_wchar) (uint8) *t) == character)
            return true;
    }
    while (*++t != 0);

    return false;
}

static void escapeIllegalXmlChars (OutputStream& outputStream,
                                   const String& text,
                                   const bool changeNewLines) throw()
{
    const juce_wchar* t = (const juce_wchar*) text;

    for (;;)
    {
        const juce_wchar character = *t++;

        if (character == 0)
        {
            break;
        }
        else if (isLegalXmlChar (character))
        {
            outputStream.writeByte ((char) character);
        }
        else
        {
            switch (character)
            {
            case '&':
                outputStream.write ("&amp;", 5);
                break;

            case '"':
                outputStream.write ("&quot;", 6);
                break;

            case '>':
                outputStream.write ("&gt;", 4);
                break;

            case '<':
                outputStream.write ("&lt;", 4);
                break;

            case '\n':
                if (changeNewLines)
                    outputStream.write ("&#10;", 5);
                else
                    outputStream.writeByte ((char) character);

                break;

            case '\r':
                if (changeNewLines)
                    outputStream.write ("&#13;", 5);
                else
                    outputStream.writeByte ((char) character);

                break;

            default:
                {
                    String encoded (T("&#"));
                    encoded << String ((int) (unsigned int) character).trim()
                            << T(';');

                    outputStream.write ((const char*) encoded, encoded.length());
                }
            }
        }
    }
}

static void writeSpaces (OutputStream& out, int numSpaces) throw()
{
    if (numSpaces > 0)
    {
        const char* const blanks = "                        ";
        const int blankSize = (int) sizeof (blanks) - 1;

        while (numSpaces > blankSize)
        {
            out.write (blanks, blankSize);
            numSpaces -= blankSize;
        }

        out.write (blanks, numSpaces);
    }
}

void XmlElement::writeElementAsText (OutputStream& outputStream,
                                     const int indentationLevel,
                                     const int lineWrapLength) const throw()
{
    writeSpaces (outputStream, indentationLevel);

    if (! isTextElement())
    {
        outputStream.writeByte ('<');
        const int nameLen = tagName.length();
        outputStream.write ((const char*) tagName, nameLen);

        const int attIndent = indentationLevel + nameLen + 1;
        int lineLen = 0;

        const XmlAttributeNode* att = attributes;
        while (att != 0)
        {
            if (lineLen > lineWrapLength && indentationLevel >= 0)
            {
                outputStream.write ("\r\n", 2);
                writeSpaces (outputStream, attIndent);
                lineLen = 0;
            }

            const int attNameLen = att->name.length();
            outputStream.writeByte (' ');
            outputStream.write ((const char*) (att->name), attNameLen);
            outputStream.write ("=\"", 2);
            escapeIllegalXmlChars (outputStream, att->value, true);
            outputStream.writeByte ('"');
            lineLen += 4 + attNameLen + att->value.length();

            att = att->next;
        }

        if (firstChildElement != 0)
        {
            XmlElement* child = firstChildElement;

            if (child->nextElement == 0 && child->isTextElement())
            {
                outputStream.writeByte ('>');
                escapeIllegalXmlChars (outputStream, child->getText(), false);
            }
            else
            {
                if (indentationLevel >= 0)
                    outputStream.write (">\r\n", 3);
                else
                    outputStream.writeByte ('>');

                bool lastWasTextNode = false;

                while (child != 0)
                {
                    if (child->isTextElement())
                    {
                        if ((! lastWasTextNode) && (indentationLevel >= 0))
                            writeSpaces (outputStream, indentationLevel + 2);

                        escapeIllegalXmlChars (outputStream, child->getText(), false);
                        lastWasTextNode = true;
                    }
                    else
                    {
                        if (indentationLevel >= 0)
                        {
                            if (lastWasTextNode)
                                outputStream.write ("\r\n", 2);

                            child->writeElementAsText (outputStream, indentationLevel + 2, lineWrapLength);
                        }
                        else
                        {
                            child->writeElementAsText (outputStream, indentationLevel, lineWrapLength);
                        }

                        lastWasTextNode = false;
                    }

                    child = child->nextElement;
                }

                if (indentationLevel >= 0)
                {
                    if (lastWasTextNode)
                        outputStream.write ("\r\n", 2);

                    writeSpaces (outputStream, indentationLevel);
                }
            }

            outputStream.write ("</", 2);
            outputStream.write ((const char*) tagName, nameLen);

            if (indentationLevel >= 0)
                outputStream.write (">\r\n", 3);
            else
                outputStream.writeByte ('>');
        }
        else
        {
            if (indentationLevel >= 0)
                outputStream.write ("/>\r\n", 4);
            else
                outputStream.write ("/>", 2);
        }
    }
    else
    {
        if (indentationLevel >= 0)
            writeSpaces (outputStream, indentationLevel + 2);

        escapeIllegalXmlChars (outputStream, getText(), false);
    }
}

const String XmlElement::createDocument (const String& dtdToUse,
                                         const bool allOnOneLine,
                                         const bool includeXmlHeader,
                                         const tchar* const encodingType,
                                         const int lineWrapLength) const throw()
{
    MemoryOutputStream mem (2048, 4096);
    writeToStream (mem, dtdToUse, allOnOneLine, includeXmlHeader, encodingType, lineWrapLength);

    return String (mem.getData(), mem.getDataSize());
}

void XmlElement::writeToStream (OutputStream& output,
                                const String& dtdToUse,
                                const bool allOnOneLine,
                                const bool includeXmlHeader,
                                const tchar* const encodingType,
                                const int lineWrapLength) const throw()
{
    if (includeXmlHeader)
    {
        output << "<?xml version=\"1.0\" encoding=\"" << encodingType;

        if (allOnOneLine)
            output << "\"?> ";
        else
            output << "\"?>\r\n\r\n";
    }

    if (dtdToUse.isNotEmpty())
    {
        output << dtdToUse;

        if (allOnOneLine)
            output << " ";
        else
            output << "\r\n";
    }

    writeElementAsText (output, allOnOneLine ? -1 : 0, lineWrapLength);
}

bool XmlElement::writeToFile (const File& file,
                              const String& dtdToUse,
                              const tchar* const encodingType,
                              const int lineWrapLength) const throw()
{
    if (file.hasWriteAccess())
    {
        TemporaryFile tempFile (file);
        ScopedPointer <FileOutputStream> out (tempFile.getFile().createOutputStream());

        if (out != 0)
        {
            writeToStream (*out, dtdToUse, false, true, encodingType, lineWrapLength);
            out = 0;

            return tempFile.overwriteTargetFileWithTemporary();
        }
    }

    return false;
}

//==============================================================================
bool XmlElement::hasTagName (const tchar* const tagNameWanted) const throw()
{
#ifdef JUCE_DEBUG
    // if debugging, check that the case is actually the same, because
    // valid xml is case-sensitive, and although this lets it pass, it's
    // better not to..
    if (tagName.equalsIgnoreCase (tagNameWanted))
    {
        jassert (tagName == tagNameWanted);
        return true;
    }
    else
    {
        return false;
    }
#else
    return tagName.equalsIgnoreCase (tagNameWanted);
#endif
}

XmlElement* XmlElement::getNextElementWithTagName (const tchar* const requiredTagName) const
{
    XmlElement* e = nextElement;

    while (e != 0 && ! e->hasTagName (requiredTagName))
        e = e->nextElement;

    return e;
}


//==============================================================================
int XmlElement::getNumAttributes() const throw()
{
    const XmlAttributeNode* att = attributes;
    int count = 0;

    while (att != 0)
    {
        att = att->next;
        ++count;
    }

    return count;
}

const String& XmlElement::getAttributeName (const int index) const throw()
{
    const XmlAttributeNode* att = attributes;
    int count = 0;

    while (att != 0)
    {
        if (count == index)
            return att->name;

        att = att->next;
        ++count;
    }

    return String::empty;
}

const String& XmlElement::getAttributeValue (const int index) const throw()
{
    const XmlAttributeNode* att = attributes;
    int count = 0;

    while (att != 0)
    {
        if (count == index)
            return att->value;

        att = att->next;
        ++count;
    }

    return String::empty;
}

bool XmlElement::hasAttribute (const tchar* const attributeName) const throw()
{
    const XmlAttributeNode* att = attributes;

    while (att != 0)
    {
        if (att->name.equalsIgnoreCase (attributeName))
            return true;

        att = att->next;
    }

    return false;
}

//==============================================================================
const String XmlElement::getStringAttribute (const tchar* const attributeName,
                                             const tchar* const defaultReturnValue) const throw()
{
    const XmlAttributeNode* att = attributes;

    while (att != 0)
    {
        if (att->name.equalsIgnoreCase (attributeName))
            return att->value;

        att = att->next;
    }

    return defaultReturnValue;
}

int XmlElement::getIntAttribute (const tchar* const attributeName,
                                 const int defaultReturnValue) const throw()
{
    const XmlAttributeNode* att = attributes;

    while (att != 0)
    {
        if (att->name.equalsIgnoreCase (attributeName))
            return att->value.getIntValue();

        att = att->next;
    }

    return defaultReturnValue;
}

double XmlElement::getDoubleAttribute (const tchar* const attributeName,
                                       const double defaultReturnValue) const throw()
{
    const XmlAttributeNode* att = attributes;

    while (att != 0)
    {
        if (att->name.equalsIgnoreCase (attributeName))
            return att->value.getDoubleValue();

        att = att->next;
    }

    return defaultReturnValue;
}

bool XmlElement::getBoolAttribute (const tchar* const attributeName,
                                   const bool defaultReturnValue) const throw()
{
    const XmlAttributeNode* att = attributes;

    while (att != 0)
    {
        if (att->name.equalsIgnoreCase (attributeName))
        {
            tchar firstChar = att->value[0];

            if (CharacterFunctions::isWhitespace (firstChar))
                firstChar = att->value.trimStart() [0];

            return firstChar == T('1')
                || firstChar == T('t')
                || firstChar == T('y')
                || firstChar == T('T')
                || firstChar == T('Y');
        }

        att = att->next;
    }

    return defaultReturnValue;
}

bool XmlElement::compareAttribute (const tchar* const attributeName,
                                   const tchar* const stringToCompareAgainst,
                                   const bool ignoreCase) const throw()
{
    const XmlAttributeNode* att = attributes;

    while (att != 0)
    {
        if (att->name.equalsIgnoreCase (attributeName))
        {
            if (ignoreCase)
                return att->value.equalsIgnoreCase (stringToCompareAgainst);
            else
                return att->value == stringToCompareAgainst;
        }

        att = att->next;
    }

    return false;
}

//==============================================================================
void XmlElement::setAttribute (const tchar* const attributeName,
                               const String& value) throw()
{
#ifdef JUCE_DEBUG
    // check the identifier being passed in is legal..
    const tchar* t = attributeName;
    while (*t != 0)
    {
        jassert (CharacterFunctions::isLetterOrDigit (*t)
                 || *t == T('_')
                 || *t == T('-')
                 || *t == T(':'));
        ++t;
    }
#endif

    if (attributes == 0)
    {
        attributes = new XmlAttributeNode (attributeName, value);
    }
    else
    {
        XmlAttributeNode* att = attributes;

        for (;;)
        {
            if (att->name.equalsIgnoreCase (attributeName))
            {
                att->value = value;
                break;
            }
            else if (att->next == 0)
            {
                att->next = new XmlAttributeNode (attributeName, value);
                break;
            }

            att = att->next;
        }
    }
}

void XmlElement::setAttribute (const tchar* const attributeName,
                               const tchar* const text) throw()
{
    setAttribute (attributeName, String (text));
}

void XmlElement::setAttribute (const tchar* const attributeName,
                               const int number) throw()
{
    setAttribute (attributeName, String (number));
}

void XmlElement::setAttribute (const tchar* const attributeName,
                               const double number) throw()
{
    setAttribute (attributeName, String (number));
}

void XmlElement::removeAttribute (const tchar* const attributeName) throw()
{
    XmlAttributeNode* att = attributes;
    XmlAttributeNode* lastAtt = 0;

    while (att != 0)
    {
        if (att->name.equalsIgnoreCase (attributeName))
        {
            if (lastAtt == 0)
                attributes = att->next;
            else
                lastAtt->next = att->next;

            delete att;
            break;
        }

        lastAtt = att;
        att = att->next;
    }
}

void XmlElement::removeAllAttributes() throw()
{
    while (attributes != 0)
    {
        XmlAttributeNode* const nextAtt = attributes->next;
        delete attributes;
        attributes = nextAtt;
    }
}

//==============================================================================
int XmlElement::getNumChildElements() const throw()
{
    int count = 0;
    const XmlElement* child = firstChildElement;

    while (child != 0)
    {
        ++count;
        child = child->nextElement;
    }

    return count;
}

XmlElement* XmlElement::getChildElement (const int index) const throw()
{
    int count = 0;
    XmlElement* child = firstChildElement;

    while (child != 0 && count < index)
    {
        child = child->nextElement;
        ++count;
    }

    return child;
}

XmlElement* XmlElement::getChildByName (const tchar* const childName) const throw()
{
    XmlElement* child = firstChildElement;

    while (child != 0)
    {
        if (child->hasTagName (childName))
            break;

        child = child->nextElement;
    }

    return child;
}

void XmlElement::addChildElement (XmlElement* const newNode) throw()
{
    if (newNode != 0)
    {
        if (firstChildElement == 0)
        {
            firstChildElement = newNode;
        }
        else
        {
            XmlElement* child = firstChildElement;

            while (child->nextElement != 0)
                child = child->nextElement;

            child->nextElement = newNode;

            // if this is non-zero, then something's probably
            // gone wrong..
            jassert (newNode->nextElement == 0);
        }
    }
}

void XmlElement::insertChildElement (XmlElement* const newNode,
                                     int indexToInsertAt) throw()
{
    if (newNode != 0)
    {
        removeChildElement (newNode, false);

        if (indexToInsertAt == 0)
        {
            newNode->nextElement = firstChildElement;
            firstChildElement = newNode;
        }
        else
        {
            if (firstChildElement == 0)
            {
                firstChildElement = newNode;
            }
            else
            {
                if (indexToInsertAt < 0)
                    indexToInsertAt = INT_MAX;

                XmlElement* child = firstChildElement;

                while (child->nextElement != 0 && --indexToInsertAt > 0)
                    child = child->nextElement;

                newNode->nextElement = child->nextElement;
                child->nextElement = newNode;
            }
        }
    }
}

bool XmlElement::replaceChildElement (XmlElement* const currentChildElement,
                                      XmlElement* const newNode) throw()
{
    if (newNode != 0)
    {
        XmlElement* child = firstChildElement;
        XmlElement* previousNode = 0;

        while (child != 0)
        {
            if (child == currentChildElement)
            {
                if (child != newNode)
                {
                    if (previousNode == 0)
                        firstChildElement = newNode;
                    else
                        previousNode->nextElement = newNode;

                    newNode->nextElement = child->nextElement;

                    delete child;
                }

                return true;
            }

            previousNode = child;
            child = child->nextElement;
        }
    }

    return false;
}

void XmlElement::removeChildElement (XmlElement* const childToRemove,
                                     const bool shouldDeleteTheChild) throw()
{
    if (childToRemove != 0)
    {
        if (firstChildElement == childToRemove)
        {
            firstChildElement = childToRemove->nextElement;
            childToRemove->nextElement = 0;
        }
        else
        {
            XmlElement* child = firstChildElement;
            XmlElement* last = 0;

            while (child != 0)
            {
                if (child == childToRemove)
                {
                    if (last == 0)
                        firstChildElement = child->nextElement;
                    else
                        last->nextElement = child->nextElement;

                    childToRemove->nextElement = 0;
                    break;
                }

                last = child;
                child = child->nextElement;
            }
        }

        if (shouldDeleteTheChild)
            delete childToRemove;
    }
}

bool XmlElement::isEquivalentTo (const XmlElement* const other,
                                 const bool ignoreOrderOfAttributes) const throw()
{
    if (this != other)
    {
        if (other == 0 || tagName != other->tagName)
        {
            return false;
        }

        if (ignoreOrderOfAttributes)
        {
            int totalAtts = 0;
            const XmlAttributeNode* att = attributes;

            while (att != 0)
            {
                if (! other->compareAttribute (att->name, att->value))
                    return false;

                att = att->next;
                ++totalAtts;
            }

            if (totalAtts != other->getNumAttributes())
                return false;
        }
        else
        {
            const XmlAttributeNode* thisAtt = attributes;
            const XmlAttributeNode* otherAtt = other->attributes;

            for (;;)
            {
                if (thisAtt == 0 || otherAtt == 0)
                {
                    if (thisAtt == otherAtt) // both 0, so it's a match
                        break;

                    return false;
                }

                if (thisAtt->name != otherAtt->name
                     || thisAtt->value != otherAtt->value)
                {
                    return false;
                }

                thisAtt = thisAtt->next;
                otherAtt = otherAtt->next;
            }
        }

        const XmlElement* thisChild = firstChildElement;
        const XmlElement* otherChild = other->firstChildElement;

        for (;;)
        {
            if (thisChild == 0 || otherChild == 0)
            {
                if (thisChild == otherChild) // both 0, so it's a match
                    break;

                return false;
            }

            if (! thisChild->isEquivalentTo (otherChild, ignoreOrderOfAttributes))
                return false;

            thisChild = thisChild->nextElement;
            otherChild = otherChild->nextElement;
        }
    }

    return true;
}

void XmlElement::deleteAllChildElements() throw()
{
    while (firstChildElement != 0)
    {
        XmlElement* const nextChild = firstChildElement->nextElement;
        delete firstChildElement;
        firstChildElement = nextChild;
    }
}

void XmlElement::deleteAllChildElementsWithTagName (const tchar* const name) throw()
{
    XmlElement* child = firstChildElement;

    while (child != 0)
    {
        if (child->hasTagName (name))
        {
            XmlElement* const nextChild = child->nextElement;
            removeChildElement (child, true);
            child = nextChild;
        }
        else
        {
            child = child->nextElement;
        }
    }
}

bool XmlElement::containsChildElement (const XmlElement* const possibleChild) const throw()
{
    const XmlElement* child = firstChildElement;

    while (child != 0)
    {
        if (child == possibleChild)
            return true;

        child = child->nextElement;
    }

    return false;
}

XmlElement* XmlElement::findParentElementOf (const XmlElement* const elementToLookFor) throw()
{
    if (this == elementToLookFor || elementToLookFor == 0)
        return 0;

    XmlElement* child = firstChildElement;

    while (child != 0)
    {
        if (elementToLookFor == child)
            return this;

        XmlElement* const found = child->findParentElementOf (elementToLookFor);

        if (found != 0)
            return found;

        child = child->nextElement;
    }

    return 0;
}

void XmlElement::getChildElementsAsArray (XmlElement** elems) const throw()
{
    XmlElement* e = firstChildElement;

    while (e != 0)
    {
        *elems++ = e;
        e = e->nextElement;
    }
}

void XmlElement::reorderChildElements (XmlElement** const elems, const int num) throw()
{
    XmlElement* e = firstChildElement = elems[0];

    for (int i = 1; i < num; ++i)
    {
        e->nextElement = elems[i];
        e = e->nextElement;
    }

    e->nextElement = 0;
}

//==============================================================================
bool XmlElement::isTextElement() const throw()
{
    return tagName.isEmpty();
}

static const tchar* const juce_xmltextContentAttributeName = T("text");

const String XmlElement::getText() const throw()
{
    jassert (isTextElement());  // you're trying to get the text from an element that
                                // isn't actually a text element.. If this contains text sub-nodes, you
                                // probably want to use getAllSubText instead.

    return getStringAttribute (juce_xmltextContentAttributeName);
}

void XmlElement::setText (const String& newText) throw()
{
    if (isTextElement())
    {
        setAttribute (juce_xmltextContentAttributeName, newText);
    }
    else
    {
        jassertfalse // you can only change the text in a text element, not a normal one.
    }
}

const String XmlElement::getAllSubText() const throw()
{
    String result;
    String::Concatenator concatenator (result);
    const XmlElement* child = firstChildElement;

    while (child != 0)
    {
        if (child->isTextElement())
            concatenator.append (child->getText());

        child = child->nextElement;
    }

    return result;
}

const String XmlElement::getChildElementAllSubText (const tchar* const childTagName,
                                                    const String& defaultReturnValue) const throw()
{
    const XmlElement* const child = getChildByName (childTagName);

    if (child != 0)
        return child->getAllSubText();

    return defaultReturnValue;
}

XmlElement* XmlElement::createTextElement (const String& text) throw()
{
    XmlElement* const e = new XmlElement ((int) 0);
    e->setAttribute (juce_xmltextContentAttributeName, text);
    return e;
}

void XmlElement::addTextElement (const String& text) throw()
{
    addChildElement (createTextElement (text));
}

void XmlElement::deleteAllTextElements() throw()
{
    XmlElement* child = firstChildElement;

    while (child != 0)
    {
        XmlElement* const next = child->nextElement;

        if (child->isTextElement())
            removeChildElement (child, true);

        child = next;
    }
}

END_JUCE_NAMESPACE
