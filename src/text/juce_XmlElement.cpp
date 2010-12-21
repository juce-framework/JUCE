/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

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
#include "../memory/juce_ScopedPointer.h"


//==============================================================================
XmlElement::XmlAttributeNode::XmlAttributeNode (const XmlAttributeNode& other) throw()
    : name (other.name),
      value (other.value),
      next (0)
{
}

XmlElement::XmlAttributeNode::XmlAttributeNode (const String& name_, const String& value_) throw()
    : name (name_),
      value (value_),
      next (0)
{
  #if JUCE_DEBUG
    // this checks whether the attribute name string contains any illegals characters..
    for (const juce_wchar* t = name; *t != 0; ++t)
        jassert (CharacterFunctions::isLetterOrDigit (*t) || *t == '_' || *t == '-' || *t == ':');
  #endif
}

inline bool XmlElement::XmlAttributeNode::hasName (const String& nameToMatch) const throw()
{
    return name.equalsIgnoreCase (nameToMatch);
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

    // The tag can't contain spaces or other characters that would create invalid XML!
    jassert (! tagName_.containsAnyOf (" <>/&"));
}

XmlElement::XmlElement (int /*dummy*/) throw()
    : firstChildElement (0),
      nextElement (0),
      attributes (0)
{
}

XmlElement::XmlElement (const XmlElement& other)
    : tagName (other.tagName),
      firstChildElement (0),
      nextElement (0),
      attributes (0)
{
    copyChildrenAndAttributesFrom (other);
}

XmlElement& XmlElement::operator= (const XmlElement& other)
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

void XmlElement::copyChildrenAndAttributesFrom (const XmlElement& other)
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
namespace XmlOutputFunctions
{
    /*bool isLegalXmlCharSlow (const juce_wchar character) throw()
    {
        if ((character >= 'a' && character <= 'z')
             || (character >= 'A' && character <= 'Z')
                || (character >= '0' && character <= '9'))
            return true;

        const char* t = " .,;:-()_+=?!'#@[]/\\*%~{}$|";

        do
        {
            if (((juce_wchar) (uint8) *t) == character)
                return true;
        }
        while (*++t != 0);

        return false;
    }

    void generateLegalCharConstants()
    {
        uint8 n[32];
        zerostruct (n);
        for (int i = 0; i < 256; ++i)
            if (isLegalXmlCharSlow (i))
                n[i >> 3] |= (1 << (i & 7));

        String s;
        for (int i = 0; i < 32; ++i)
            s << (int) n[i] << ", ";

        DBG (s);
    }*/

    bool isLegalXmlChar (const uint32 c) throw()
    {
        static const unsigned char legalChars[] = { 0, 0, 0, 0, 187, 255, 255, 175, 255, 255, 255, 191, 254, 255, 255, 127 };

        return c < sizeof (legalChars) * 8
                 && (legalChars [c >> 3] & (1 << (c & 7))) != 0;
    }

    void escapeIllegalXmlChars (OutputStream& outputStream, const String& text, const bool changeNewLines)
    {
        const juce_wchar* t = text;

        for (;;)
        {
            const juce_wchar character = *t++;

            if (character == 0)
                break;

            if (isLegalXmlChar ((uint32) character))
            {
                outputStream << (char) character;
            }
            else
            {
                switch (character)
                {
                case '&':   outputStream << "&amp;"; break;
                case '"':   outputStream << "&quot;"; break;
                case '>':   outputStream << "&gt;"; break;
                case '<':   outputStream << "&lt;"; break;

                case '\n':
                case '\r':
                    if (! changeNewLines)
                    {
                        outputStream << (char) character;
                        break;
                    }
                    // Note: deliberate fall-through here!
                default:
                    outputStream << "&#" << ((int) (unsigned int) character) << ';';
                    break;
                }
            }
        }
    }

    void writeSpaces (OutputStream& out, int numSpaces)
    {
        if (numSpaces > 0)
        {
            const char blanks[] = "                        ";
            const int blankSize = (int) numElementsInArray (blanks) - 1;

            while (numSpaces > blankSize)
            {
                out.write (blanks, blankSize);
                numSpaces -= blankSize;
            }

            out.write (blanks, numSpaces);
        }
    }
}

void XmlElement::writeElementAsText (OutputStream& outputStream,
                                     const int indentationLevel,
                                     const int lineWrapLength) const
{
    using namespace XmlOutputFunctions;
    writeSpaces (outputStream, indentationLevel);

    if (! isTextElement())
    {
        outputStream.writeByte ('<');
        outputStream << tagName;

        {
            const int attIndent = indentationLevel + tagName.length() + 1;
            int lineLen = 0;

            for (const XmlAttributeNode* att = attributes; att != 0; att = att->next)
            {
                if (lineLen > lineWrapLength && indentationLevel >= 0)
                {
                    outputStream << newLine;
                    writeSpaces (outputStream, attIndent);
                    lineLen = 0;
                }

                const int64 startPos = outputStream.getPosition();
                outputStream.writeByte (' ');
                outputStream << att->name;
                outputStream.write ("=\"", 2);
                escapeIllegalXmlChars (outputStream, att->value, true);
                outputStream.writeByte ('"');
                lineLen += (int) (outputStream.getPosition() - startPos);
            }
        }

        if (firstChildElement != 0)
        {
            outputStream.writeByte ('>');

            XmlElement* child = firstChildElement;
            bool lastWasTextNode = false;

            while (child != 0)
            {
                if (child->isTextElement())
                {
                    escapeIllegalXmlChars (outputStream, child->getText(), false);
                    lastWasTextNode = true;
                }
                else
                {
                    if (indentationLevel >= 0 && ! lastWasTextNode)
                        outputStream << newLine;

                    child->writeElementAsText (outputStream,
                                               lastWasTextNode ? 0 : (indentationLevel + (indentationLevel >= 0 ? 2 : 0)), lineWrapLength);
                    lastWasTextNode = false;
                }

                child = child->nextElement;
            }

            if (indentationLevel >= 0 && ! lastWasTextNode)
            {
                outputStream << newLine;
                writeSpaces (outputStream, indentationLevel);
            }

            outputStream.write ("</", 2);
            outputStream << tagName;
            outputStream.writeByte ('>');
        }
        else
        {
            outputStream.write ("/>", 2);
        }
    }
    else
    {
        escapeIllegalXmlChars (outputStream, getText(), false);
    }
}

const String XmlElement::createDocument (const String& dtdToUse,
                                         const bool allOnOneLine,
                                         const bool includeXmlHeader,
                                         const String& encodingType,
                                         const int lineWrapLength) const
{
    MemoryOutputStream mem (2048);
    writeToStream (mem, dtdToUse, allOnOneLine, includeXmlHeader, encodingType, lineWrapLength);

    return mem.toUTF8();
}

void XmlElement::writeToStream (OutputStream& output,
                                const String& dtdToUse,
                                const bool allOnOneLine,
                                const bool includeXmlHeader,
                                const String& encodingType,
                                const int lineWrapLength) const
{
    using namespace XmlOutputFunctions;

    if (includeXmlHeader)
    {
        output << "<?xml version=\"1.0\" encoding=\"" << encodingType << "\"?>";

        if (allOnOneLine)
            output.writeByte (' ');
        else
            output << newLine << newLine;
    }

    if (dtdToUse.isNotEmpty())
    {
        output << dtdToUse;

        if (allOnOneLine)
            output.writeByte (' ');
        else
            output << newLine;
    }

    writeElementAsText (output, allOnOneLine ? -1 : 0, lineWrapLength);

    if (! allOnOneLine)
        output << newLine;
}

bool XmlElement::writeToFile (const File& file,
                              const String& dtdToUse,
                              const String& encodingType,
                              const int lineWrapLength) const
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
bool XmlElement::hasTagName (const String& tagNameWanted) const throw()
{
#if JUCE_DEBUG
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

XmlElement* XmlElement::getNextElementWithTagName (const String& requiredTagName) const
{
    XmlElement* e = nextElement;

    while (e != 0 && ! e->hasTagName (requiredTagName))
        e = e->nextElement;

    return e;
}

//==============================================================================
int XmlElement::getNumAttributes() const throw()
{
    int count = 0;

    for (const XmlAttributeNode* att = attributes; att != 0; att = att->next)
        ++count;

    return count;
}

const String& XmlElement::getAttributeName (const int index) const throw()
{
    int count = 0;

    for (const XmlAttributeNode* att = attributes; att != 0; att = att->next)
    {
        if (count == index)
            return att->name;

        ++count;
    }

    return String::empty;
}

const String& XmlElement::getAttributeValue (const int index) const throw()
{
    int count = 0;

    for (const XmlAttributeNode* att = attributes; att != 0; att = att->next)
    {
        if (count == index)
            return att->value;

        ++count;
    }

    return String::empty;
}

bool XmlElement::hasAttribute (const String& attributeName) const throw()
{
    for (const XmlAttributeNode* att = attributes; att != 0; att = att->next)
        if (att->hasName (attributeName))
            return true;

    return false;
}

//==============================================================================
const String& XmlElement::getStringAttribute (const String& attributeName) const throw()
{
    for (const XmlAttributeNode* att = attributes; att != 0; att = att->next)
        if (att->hasName (attributeName))
            return att->value;

    return String::empty;
}

const String XmlElement::getStringAttribute (const String& attributeName, const String& defaultReturnValue) const
{
    for (const XmlAttributeNode* att = attributes; att != 0; att = att->next)
        if (att->hasName (attributeName))
            return att->value;

    return defaultReturnValue;
}

int XmlElement::getIntAttribute (const String& attributeName, const int defaultReturnValue) const
{
    for (const XmlAttributeNode* att = attributes; att != 0; att = att->next)
        if (att->hasName (attributeName))
            return att->value.getIntValue();

    return defaultReturnValue;
}

double XmlElement::getDoubleAttribute (const String& attributeName, const double defaultReturnValue) const
{
    for (const XmlAttributeNode* att = attributes; att != 0; att = att->next)
        if (att->hasName (attributeName))
            return att->value.getDoubleValue();

    return defaultReturnValue;
}

bool XmlElement::getBoolAttribute (const String& attributeName, const bool defaultReturnValue) const
{
    for (const XmlAttributeNode* att = attributes; att != 0; att = att->next)
    {
        if (att->hasName (attributeName))
        {
            juce_wchar firstChar = att->value[0];

            if (CharacterFunctions::isWhitespace (firstChar))
                firstChar = att->value.trimStart() [0];

            return firstChar == '1'
                || firstChar == 't'
                || firstChar == 'y'
                || firstChar == 'T'
                || firstChar == 'Y';
        }
    }

    return defaultReturnValue;
}

bool XmlElement::compareAttribute (const String& attributeName,
                                   const String& stringToCompareAgainst,
                                   const bool ignoreCase) const throw()
{
    for (const XmlAttributeNode* att = attributes; att != 0; att = att->next)
        if (att->hasName (attributeName))
            return ignoreCase ? att->value.equalsIgnoreCase (stringToCompareAgainst)
                              : att->value == stringToCompareAgainst;

    return false;
}

//==============================================================================
void XmlElement::setAttribute (const String& attributeName, const String& value)
{
    if (attributes == 0)
    {
        attributes = new XmlAttributeNode (attributeName, value);
    }
    else
    {
        XmlAttributeNode* att = attributes;

        for (;;)
        {
            if (att->hasName (attributeName))
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

void XmlElement::setAttribute (const String& attributeName, const int number)
{
    setAttribute (attributeName, String (number));
}

void XmlElement::setAttribute (const String& attributeName, const double number)
{
    setAttribute (attributeName, String (number));
}

void XmlElement::removeAttribute (const String& attributeName) throw()
{
    XmlAttributeNode* lastAtt = 0;

    for (XmlAttributeNode* att = attributes; att != 0; att = att->next)
    {
        if (att->hasName (attributeName))
        {
            if (lastAtt == 0)
                attributes = att->next;
            else
                lastAtt->next = att->next;

            delete att;
            break;
        }

        lastAtt = att;
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

XmlElement* XmlElement::getChildByName (const String& childName) const throw()
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
                    indexToInsertAt = std::numeric_limits<int>::max();

                XmlElement* child = firstChildElement;

                while (child->nextElement != 0 && --indexToInsertAt > 0)
                    child = child->nextElement;

                newNode->nextElement = child->nextElement;
                child->nextElement = newNode;
            }
        }
    }
}

XmlElement* XmlElement::createNewChildElement (const String& childTagName)
{
    XmlElement* const newElement = new XmlElement (childTagName);
    addChildElement (newElement);
    return newElement;
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
            return false;

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

void XmlElement::deleteAllChildElementsWithTagName (const String& name) throw()
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

static const juce_wchar* const juce_xmltextContentAttributeName = L"text";

const String& XmlElement::getText() const throw()
{
    jassert (isTextElement());  // you're trying to get the text from an element that
                                // isn't actually a text element.. If this contains text sub-nodes, you
                                // probably want to use getAllSubText instead.

    return getStringAttribute (juce_xmltextContentAttributeName);
}

void XmlElement::setText (const String& newText)
{
    if (isTextElement())
        setAttribute (juce_xmltextContentAttributeName, newText);
    else
        jassertfalse; // you can only change the text in a text element, not a normal one.
}

const String XmlElement::getAllSubText() const
{
    if (isTextElement())
        return getText();

    String result;
    String::Concatenator concatenator (result);
    const XmlElement* child = firstChildElement;

    while (child != 0)
    {
        concatenator.append (child->getAllSubText());
        child = child->nextElement;
    }

    return result;
}

const String XmlElement::getChildElementAllSubText (const String& childTagName,
                                                    const String& defaultReturnValue) const
{
    const XmlElement* const child = getChildByName (childTagName);

    if (child != 0)
        return child->getAllSubText();

    return defaultReturnValue;
}

XmlElement* XmlElement::createTextElement (const String& text)
{
    XmlElement* const e = new XmlElement ((int) 0);
    e->setAttribute (juce_xmltextContentAttributeName, text);
    return e;
}

void XmlElement::addTextElement (const String& text)
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
