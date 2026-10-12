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
static bool isValidXmlNameStartCharacter (juce_wchar character) noexcept
{
    return character == ':'
        || character == '_'
        || (character >= 'a'     && character <= 'z')
        || (character >= 'A'     && character <= 'Z')
        || (character >= 0xc0    && character <= 0xd6)
        || (character >= 0xd8    && character <= 0xf6)
        || (character >= 0xf8    && character <= 0x2ff)
        || (character >= 0x370   && character <= 0x37d)
        || (character >= 0x37f   && character <= 0x1fff)
        || (character >= 0x200c  && character <= 0x200d)
        || (character >= 0x2070  && character <= 0x218f)
        || (character >= 0x2c00  && character <= 0x2fef)
        || (character >= 0x3001  && character <= 0xd7ff)
        || (character >= 0xf900  && character <= 0xfdcf)
        || (character >= 0xfdf0  && character <= 0xfffd)
        || (character >= 0x10000 && character <= 0xeffff);
}

static bool isValidXmlNameBodyCharacter (juce_wchar character) noexcept
{
    return isValidXmlNameStartCharacter (character)
        || character == '-'
        || character == '.'
        || character == 0xb7
        || (character >= '0'    && character <= '9')
        || (character >= 0x300  && character <= 0x036f)
        || (character >= 0x203f && character <= 0x2040);
}

XmlElement::XmlAttributeNode::XmlAttributeNode (const XmlAttributeNode& other) noexcept
    : attribute (other.attribute)
{
}

XmlElement::XmlAttributeNode::XmlAttributeNode (const Identifier& n, const String& v) noexcept
    : attribute { n, v }
{
    jassert (isValidXmlName (attribute.name));
}

XmlElement::XmlAttributeNode::XmlAttributeNode (String::CharPointerType nameStart, String::CharPointerType nameEnd)
    : XmlAttributeNode ({ nameStart, nameEnd }, {})
{
}

//==============================================================================
XmlElement::XmlElement (const String& tag)
    : tagName (StringPool::getGlobalPool().getPooledString (tag))
{
    jassert (isValidXmlName (tagName));
}

XmlElement::XmlElement (const char* tag)
    : tagName (StringPool::getGlobalPool().getPooledString (tag))
{
    jassert (isValidXmlName (tagName));
}

XmlElement::XmlElement (StringRef tag)
    : tagName (StringPool::getGlobalPool().getPooledString (tag))
{
    jassert (isValidXmlName (tagName));
}

XmlElement::XmlElement (const Identifier& tag)
    : tagName (tag.toString())
{
    jassert (isValidXmlName (tagName));
}

XmlElement::XmlElement (String::CharPointerType tagNameStart, String::CharPointerType tagNameEnd)
    : tagName (StringPool::getGlobalPool().getPooledString (tagNameStart, tagNameEnd))
{
    jassert (isValidXmlName (tagName));
}

XmlElement::XmlElement (int /*dummy*/) noexcept
{
}

XmlElement::XmlElement (const XmlElement& other)
    : tagName (other.tagName)
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

XmlElement::XmlElement (XmlElement&& other) noexcept
    : nextListItem      (std::move (other.nextListItem)),
      firstChildElement (std::move (other.firstChildElement)),
      attributes        (std::move (other.attributes)),
      tagName           (std::move (other.tagName))
{
}

XmlElement& XmlElement::operator= (XmlElement&& other) noexcept
{
    jassert (this != &other); // hopefully the compiler should make this situation impossible!

    removeAllAttributes();
    deleteAllChildElements();

    nextListItem      = std::move (other.nextListItem);
    firstChildElement = std::move (other.firstChildElement);
    attributes        = std::move (other.attributes);
    tagName           = std::move (other.tagName);

    return *this;
}

void XmlElement::copyChildrenAndAttributesFrom (const XmlElement& other)
{
    jassert (firstChildElement.get() == nullptr);
    firstChildElement.addCopyOfList (other.firstChildElement);

    jassert (attributes.get() == nullptr);
    attributes.addCopyOfList (other.attributes);
}

XmlElement::~XmlElement() noexcept
{
    firstChildElement.deleteAll();
    attributes.deleteAll();
}

//==============================================================================
namespace XmlOutputFunctions
{
    namespace LegalCharLookupTable
    {
        template <int c>
        struct Bit
        {
            enum { v = ((c >= 'a' && c <= 'z')
                     || (c >= 'A' && c <= 'Z')
                     || (c >= '0' && c <= '9')
                     || c == ' ' || c == '.'  || c == ',' || c == ';'
                     || c == ':' || c == '-'  || c == '(' || c == ')'
                     || c == '_' || c == '+'  || c == '=' || c == '?'
                     || c == '!' || c == '$'  || c == '#' || c == '@'
                     || c == '[' || c == ']'  || c == '/' || c == '|'
                     || c == '*' || c == '%'  || c == '~' || c == '{'
                     || c == '}' || c == '\'' || c == '\\')
                        ? (1 << (c & 7)) : 0 };
        };

        template <int tableIndex>
        struct Byte
        {
            enum { v = (int) Bit<tableIndex * 8 + 0>::v | (int) Bit<tableIndex * 8 + 1>::v
                     | (int) Bit<tableIndex * 8 + 2>::v | (int) Bit<tableIndex * 8 + 3>::v
                     | (int) Bit<tableIndex * 8 + 4>::v | (int) Bit<tableIndex * 8 + 5>::v
                     | (int) Bit<tableIndex * 8 + 6>::v | (int) Bit<tableIndex * 8 + 7>::v };
        };

        static bool isLegal (uint32 c) noexcept
        {
            static const unsigned char legalChars[] = { Byte< 0>::v, Byte< 1>::v, Byte< 2>::v, Byte< 3>::v,
                                                        Byte< 4>::v, Byte< 5>::v, Byte< 6>::v, Byte< 7>::v,
                                                        Byte< 8>::v, Byte< 9>::v, Byte<10>::v, Byte<11>::v,
                                                        Byte<12>::v, Byte<13>::v, Byte<14>::v, Byte<15>::v };

            return c < sizeof (legalChars) * 8
                     && (legalChars[c >> 3] & (1 << (c & 7))) != 0;
        }
    }

    static void escapeIllegalXmlChars (OutputStream& outputStream, const String& text, bool changeNewLines)
    {
        auto t = text.getCharPointer();

        for (;;)
        {
            auto character = (uint32) t.getAndAdvance();

            if (character == 0)
                break;

            if (LegalCharLookupTable::isLegal (character))
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
                        JUCE_FALLTHROUGH
                    default:
                        outputStream << "&#" << ((int) character) << ';';
                        break;
                }
            }
        }
    }

    static void writeSpaces (OutputStream& out, const size_t numSpaces)
    {
        out.writeRepeatedByte (' ', numSpaces);
    }
}

void XmlElement::LineFormat::writeNewLineAndIndent (OutputStream& out) const
{
    out << newLineChars;
    XmlOutputFunctions::writeSpaces (out, (size_t) indentation);
}

void XmlElement::writeElementAsText (OutputStream& outputStream,
                                     std::optional<LineFormat> lineFormat,
                                     int lineWrapLength) const
{
    if (! isTextElement())
    {
        outputStream.writeByte ('<');
        outputStream << tagName;

        {
            int lineLen = 0;

            for (const auto& [name, value] : getAttributeIterator())
            {
                if (lineLen > lineWrapLength && lineFormat.has_value())
                {
                    lineFormat->writeNewLineAndIndent (outputStream);
                    XmlOutputFunctions::writeSpaces (outputStream, (size_t) tagName.length() + 1);
                    lineLen = 0;
                }

                auto startPos = outputStream.getPosition();
                outputStream.writeByte (' ');
                outputStream << name;
                outputStream.write ("=\"", 2);
                XmlOutputFunctions::escapeIllegalXmlChars (outputStream, value, true);
                outputStream.writeByte ('"');
                lineLen += (int) (outputStream.getPosition() - startPos);
            }
        }

        if (auto* child = firstChildElement.get())
        {
            outputStream.writeByte ('>');
            bool lastWasTextNode = false;

            const auto childLineFormat = lineFormat.has_value()
                                             ? std::optional (lineFormat->indented())
                                             : std::nullopt;

            for (; child != nullptr; child = child->nextListItem)
            {
                if (child->isTextElement())
                {
                    XmlOutputFunctions::escapeIllegalXmlChars (outputStream, child->getText(), false);
                    lastWasTextNode = true;
                }
                else
                {
                    if (childLineFormat.has_value() && ! lastWasTextNode)
                        childLineFormat->writeNewLineAndIndent (outputStream);

                    child->writeElementAsText (outputStream,
                                               childLineFormat,
                                               lineWrapLength);
                    lastWasTextNode = false;
                }
            }

            if (lineFormat.has_value() && ! lastWasTextNode)
                lineFormat->writeNewLineAndIndent (outputStream);

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
        XmlOutputFunctions::escapeIllegalXmlChars (outputStream, getText(), false);
    }
}

XmlElement::TextFormat::TextFormat() {}

XmlElement::TextFormat XmlElement::TextFormat::singleLine() const
{
    auto f = *this;
    f.newLineChars = nullptr;
    return f;
}

XmlElement::TextFormat XmlElement::TextFormat::withoutHeader() const
{
    auto f = *this;
    f.addDefaultHeader = false;
    return f;
}

String XmlElement::toString (const TextFormat& options) const
{
    MemoryOutputStream mem (2048);
    writeTo (mem, options);
    return mem.toUTF8();
}

void XmlElement::writeTo (OutputStream& output, const TextFormat& options) const
{
    if (options.customHeader.isNotEmpty())
    {
        output << options.customHeader;

        if (options.newLineChars == nullptr)
            output.writeByte (' ');
        else
            output << options.newLineChars
                   << options.newLineChars;
    }
    else if (options.addDefaultHeader)
    {
        output << "<?xml version=\"1.0\" encoding=\"";

        if (options.customEncoding.isNotEmpty())
            output << options.customEncoding;
        else
            output << "UTF-8";

        output << "\"?>";

        if (options.newLineChars == nullptr)
            output.writeByte (' ');
        else
            output << options.newLineChars
                   << options.newLineChars;
    }

    if (options.dtd.isNotEmpty())
    {
        output << options.dtd;

        if (options.newLineChars == nullptr)
            output.writeByte (' ');
        else
            output << options.newLineChars;
    }

    writeElementAsText (output,
                        options.newLineChars != nullptr
                            ? std::optional (LineFormat { options.newLineChars })
                            : std::nullopt,
                        options.lineWrapLength);

    if (options.newLineChars != nullptr)
        output << options.newLineChars;
}

bool XmlElement::writeTo (const File& destinationFile, const TextFormat& options) const
{
    TemporaryFile tempFile (destinationFile);

    {
        FileOutputStream out (tempFile.getFile());

        if (! out.openedOk())
            return false;

        writeTo (out, options);
        out.flush(); // (called explicitly to force an fsync on posix)

        if (out.getStatus().failed())
            return false;
    }

    return tempFile.overwriteTargetFileWithTemporary();
}

String XmlElement::createDocument (StringRef dtdToUse, bool allOnOneLine, bool includeXmlHeader,
                                   StringRef encodingType, int lineWrapLength) const
{
    TextFormat options;
    options.dtd = dtdToUse;
    options.customEncoding = encodingType;
    options.addDefaultHeader = includeXmlHeader;
    options.lineWrapLength = lineWrapLength;

    if (allOnOneLine)
        options.newLineChars = nullptr;

    return toString (options);
}

void XmlElement::writeToStream (OutputStream& output, StringRef dtdToUse,
                                bool allOnOneLine, bool includeXmlHeader,
                                StringRef encodingType, int lineWrapLength) const
{
    TextFormat options;
    options.dtd = dtdToUse;
    options.customEncoding = encodingType;
    options.addDefaultHeader = includeXmlHeader;
    options.lineWrapLength = lineWrapLength;

    if (allOnOneLine)
        options.newLineChars = nullptr;

    writeTo (output, options);
}

bool XmlElement::writeToFile (const File& file, StringRef dtdToUse,
                              StringRef encodingType, int lineWrapLength) const
{
    TextFormat options;
    options.dtd = dtdToUse;
    options.customEncoding = encodingType;
    options.lineWrapLength = lineWrapLength;

    return writeTo (file, options);
}

//==============================================================================
bool XmlElement::hasTagName (StringRef possibleTagName) const noexcept
{
    const bool matches = tagName.equalsIgnoreCase (possibleTagName);

    // XML tags should be case-sensitive, so although this method allows a
    // case-insensitive match to pass, you should try to avoid this.
    jassert ((! matches) || tagName == possibleTagName);

    return matches;
}

String XmlElement::getNamespace() const
{
    return tagName.upToFirstOccurrenceOf (":", false, false);
}

String XmlElement::getTagNameWithoutNamespace() const
{
    return tagName.fromLastOccurrenceOf (":", false, false);
}

bool XmlElement::hasTagNameIgnoringNamespace (StringRef possibleTagName) const
{
    return hasTagName (possibleTagName) || getTagNameWithoutNamespace() == possibleTagName;
}

XmlElement* XmlElement::getNextElementWithTagName (StringRef requiredTagName) const
{
    auto* e = nextListItem.get();

    while (e != nullptr && ! e->hasTagName (requiredTagName))
        e = e->nextListItem;

    return e;
}

void XmlElement::setTagName (StringRef newTagName)
{
    jassert (isValidXmlName (newTagName));
    tagName = StringPool::getGlobalPool().getPooledString (newTagName);
}

//==============================================================================
int XmlElement::getNumAttributes() const noexcept
{
    return attributes.size();
}

static const String& getEmptyStringRef() noexcept
{
    static String empty;
    return empty;
}

const String& XmlElement::getAttributeName (const int index) const noexcept
{
    if (auto* att = attributes[index].get())
        return att->attribute.name.toString();

    return getEmptyStringRef();
}

const String& XmlElement::getAttributeValue (const int index) const noexcept
{
    if (auto* att = attributes[index].get())
        return att->attribute.value;

    return getEmptyStringRef();
}

const XmlAttribute* XmlElement::getAttribute (StringRef attributeName) const noexcept
{
    for (const auto& att : getAttributeIterator())
        if (att.name == attributeName)
            return &att;

    return nullptr;
}

bool XmlElement::hasAttribute (StringRef attributeName) const noexcept
{
    return getAttribute (attributeName) != nullptr;
}

//==============================================================================
const String& XmlElement::getStringAttribute (StringRef attributeName) const noexcept
{
    if (auto* att = getAttribute (attributeName))
        return att->value;

    return getEmptyStringRef();
}

String XmlElement::getStringAttribute (StringRef attributeName, const String& defaultReturnValue) const
{
    if (auto* att = getAttribute (attributeName))
        return att->value;

    return defaultReturnValue;
}

int XmlElement::getIntAttribute (StringRef attributeName, const int defaultReturnValue) const
{
    if (auto* att = getAttribute (attributeName))
        return att->value.getIntValue();

    return defaultReturnValue;
}

double XmlElement::getDoubleAttribute (StringRef attributeName, const double defaultReturnValue) const
{
    if (auto* att = getAttribute (attributeName))
        return att->value.getDoubleValue();

    return defaultReturnValue;
}

bool XmlElement::getBoolAttribute (StringRef attributeName, const bool defaultReturnValue) const
{
    if (auto* att = getAttribute (attributeName))
    {
        auto firstChar = *(att->value.getCharPointer().findEndOfWhitespace());

        return firstChar == '1'
            || firstChar == 't'
            || firstChar == 'y'
            || firstChar == 'T'
            || firstChar == 'Y';
    }

    return defaultReturnValue;
}

bool XmlElement::compareAttribute (StringRef attributeName,
                                   StringRef stringToCompareAgainst,
                                   const bool ignoreCase) const noexcept
{
    if (auto* att = getAttribute (attributeName))
        return att->equals (attributeName, stringToCompareAgainst, ignoreCase);

    return false;
}

bool XmlElement::compareAttribute (const XmlAttribute& other, const bool ignoreCase) const noexcept
{
    return compareAttribute (other.name, other.value, ignoreCase);
}

//==============================================================================
void XmlElement::setAttribute (const Identifier& attributeName, const String& value)
{
    if (attributes == nullptr)
    {
        attributes = new XmlAttributeNode (attributeName, value);
    }
    else
    {
        for (auto* att = attributes.get(); ; att = att->nextListItem)
        {
            if (att->attribute.name == attributeName)
            {
                att->attribute.value = value;
                break;
            }

            if (att->nextListItem == nullptr)
            {
                att->nextListItem = new XmlAttributeNode (attributeName, value);
                break;
            }
        }
    }
}

void XmlElement::setAttribute (const Identifier& attributeName, const int number)
{
    setAttribute (attributeName, String (number));
}

void XmlElement::setAttribute (const Identifier& attributeName, const double number)
{
    setAttribute (attributeName, serialiseDouble (number));
}

void XmlElement::removeAttribute (const Identifier& attributeName) noexcept
{
    for (auto* att = &attributes; att->get() != nullptr; att = &(att->get()->nextListItem))
    {
        if (att->get()->attribute.name == attributeName)
        {
            delete att->removeNext();
            break;
        }
    }
}

void XmlElement::removeAllAttributes() noexcept
{
    attributes.deleteAll();
}

//==============================================================================
int XmlElement::getNumChildElements() const noexcept
{
    return firstChildElement.size();
}

XmlElement* XmlElement::getChildElement (const int index) const noexcept
{
    return firstChildElement[index].get();
}

XmlElement* XmlElement::getChildByName (StringRef childName) const noexcept
{
    jassert (! childName.isEmpty());

    for (auto* child = firstChildElement.get(); child != nullptr; child = child->nextListItem)
        if (child->hasTagName (childName))
            return child;

    return nullptr;
}

XmlElement* XmlElement::getChildByAttribute (StringRef attributeName, StringRef attributeValue) const noexcept
{
    jassert (! attributeName.isEmpty());

    for (auto* child = firstChildElement.get(); child != nullptr; child = child->nextListItem)
        if (child->compareAttribute (attributeName, attributeValue))
            return child;

    return nullptr;
}

void XmlElement::addChildElement (XmlElement* const newNode) noexcept
{
    if (newNode != nullptr)
    {
        // The element being added must not be a child of another node!
        jassert (newNode->nextListItem == nullptr);

        firstChildElement.append (newNode);
    }
}

void XmlElement::insertChildElement (XmlElement* const newNode, int indexToInsertAt) noexcept
{
    if (newNode != nullptr)
    {
        // The element being added must not be a child of another node!
        jassert (newNode->nextListItem == nullptr);

        firstChildElement.insertAtIndex (indexToInsertAt, newNode);
    }
}

void XmlElement::prependChildElement (XmlElement* newNode) noexcept
{
    if (newNode != nullptr)
    {
        // The element being added must not be a child of another node!
        jassert (newNode->nextListItem == nullptr);

        firstChildElement.insertNext (newNode);
    }
}

XmlElement* XmlElement::createNewChildElement (StringRef childTagName)
{
    auto newElement = new XmlElement (childTagName);
    addChildElement (newElement);
    return newElement;
}

bool XmlElement::replaceChildElement (XmlElement* const currentChildElement,
                                      XmlElement* const newNode) noexcept
{
    if (newNode != nullptr)
    {
        if (auto* p = firstChildElement.findPointerTo (currentChildElement))
        {
            if (currentChildElement != newNode)
                delete p->replaceNext (newNode);

            return true;
        }
    }

    return false;
}

void XmlElement::removeChildElement (XmlElement* const childToRemove,
                                     const bool shouldDeleteTheChild) noexcept
{
    if (childToRemove != nullptr)
    {
        jassert (containsChildElement (childToRemove));

        firstChildElement.remove (childToRemove);

        if (shouldDeleteTheChild)
            delete childToRemove;
    }
}

bool XmlElement::isEquivalentTo (const XmlElement* const other,
                                 const bool ignoreOrderOfAttributes) const noexcept
{
    if (this != other)
    {
        if (other == nullptr || tagName != other->tagName)
            return false;

        if (ignoreOrderOfAttributes)
        {
            int totalAtts = 0;

            for (auto* att = attributes.get(); att != nullptr; att = att->nextListItem)
            {
                if (! other->compareAttribute (att->attribute))
                    return false;

                ++totalAtts;
            }

            if (totalAtts != other->getNumAttributes())
                return false;
        }
        else
        {
            auto* thisAtt = attributes.get();
            auto* otherAtt = other->attributes.get();

            for (;;)
            {
                if (thisAtt == nullptr || otherAtt == nullptr)
                {
                    if (thisAtt == otherAtt) // both nullptr, so it's a match
                        break;

                    return false;
                }

                if (thisAtt->attribute != otherAtt->attribute)
                    return false;

                thisAtt = thisAtt->nextListItem;
                otherAtt = otherAtt->nextListItem;
            }
        }

        auto* thisChild = firstChildElement.get();
        auto* otherChild = other->firstChildElement.get();

        for (;;)
        {
            if (thisChild == nullptr || otherChild == nullptr)
            {
                if (thisChild == otherChild) // both 0, so it's a match
                    break;

                return false;
            }

            if (! thisChild->isEquivalentTo (otherChild, ignoreOrderOfAttributes))
                return false;

            thisChild = thisChild->nextListItem;
            otherChild = otherChild->nextListItem;
        }
    }

    return true;
}

void XmlElement::deleteAllChildElements() noexcept
{
    firstChildElement.deleteAll();
}

void XmlElement::deleteAllChildElementsWithTagName (StringRef name) noexcept
{
    for (auto* child = firstChildElement.get(); child != nullptr;)
    {
        auto* nextChild = child->nextListItem.get();

        if (child->hasTagName (name))
            removeChildElement (child, true);

        child = nextChild;
    }
}

bool XmlElement::containsChildElement (const XmlElement* const possibleChild) const noexcept
{
    return firstChildElement.contains (possibleChild);
}

XmlElement* XmlElement::findParentElementOf (const XmlElement* const elementToLookFor) noexcept
{
    if (this == elementToLookFor || elementToLookFor == nullptr)
        return nullptr;

    for (auto* child = firstChildElement.get(); child != nullptr; child = child->nextListItem)
    {
        if (elementToLookFor == child)
            return this;

        if (auto* found = child->findParentElementOf (elementToLookFor))
            return found;
    }

    return nullptr;
}

void XmlElement::getChildElementsAsArray (XmlElement** elems) const noexcept
{
    firstChildElement.copyToArray (elems);
}

void XmlElement::reorderChildElements (XmlElement** elems, int num) noexcept
{
    auto* e = elems[0];
    firstChildElement = e;

    for (int i = 1; i < num; ++i)
    {
        e->nextListItem = elems[i];
        e = e->nextListItem;
    }

    e->nextListItem = nullptr;
}

//==============================================================================
bool XmlElement::isTextElement() const noexcept
{
    return tagName.isEmpty();
}

static const String& getJuceXmlTextContentAttributeName()
{
    static String result { "text" };
    return result;
}

const String& XmlElement::getText() const noexcept
{
    jassert (isTextElement());  // you're trying to get the text from an element that
                                // isn't actually a text element. If this contains text sub-nodes, you
                                // probably want to use getAllSubText instead.

    return getStringAttribute (getJuceXmlTextContentAttributeName());
}

void XmlElement::setText (const String& newText)
{
    if (isTextElement())
        setAttribute (getJuceXmlTextContentAttributeName(), newText);
    else
        jassertfalse; // you can only change the text in a text element, not a normal one
}

String XmlElement::getAllSubText() const
{
    if (isTextElement())
        return getText();

    if (getNumChildElements() == 1)
        return firstChildElement.get()->getAllSubText();

    MemoryOutputStream mem (1024);

    for (auto* child = firstChildElement.get(); child != nullptr; child = child->nextListItem)
        mem << child->getAllSubText();

    return mem.toUTF8();
}

String XmlElement::getChildElementAllSubText (StringRef childTagName, const String& defaultReturnValue) const
{
    if (auto* child = getChildByName (childTagName))
        return child->getAllSubText();

    return defaultReturnValue;
}

XmlElement* XmlElement::createTextElement (const String& text)
{
    auto e = new XmlElement ((int) 0);
    e->setAttribute (getJuceXmlTextContentAttributeName(), text);
    return e;
}

bool XmlElement::isValidXmlName (StringRef text) noexcept
{
    if (text.isEmpty() || ! isValidXmlNameStartCharacter (text.text.getAndAdvance()))
        return false;

    for (;;)
    {
        if (text.isEmpty())
            return true;

        if (! isValidXmlNameBodyCharacter (text.text.getAndAdvance()))
            return false;
    }
}

void XmlElement::addTextElement (const String& text)
{
    addChildElement (createTextElement (text));
}

void XmlElement::deleteAllTextElements() noexcept
{
    for (auto* child = firstChildElement.get(); child != nullptr;)
    {
        auto* next = child->nextListItem.get();

        if (child->isTextElement())
            removeChildElement (child, true);

        child = next;
    }
}

//==============================================================================
//==============================================================================
#if JUCE_UNIT_TESTS

class XmlElementTests final : public UnitTest
{
public:
    XmlElementTests()
        : UnitTest ("XmlElement", UnitTestCategories::xml)
    {}

    void runTest() override
    {
        testCase ("Float formatting", [&]
        {
            auto element = std::make_unique<XmlElement> ("test");
            Identifier number ("number");

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
            {
                element->setAttribute (number, test.first);
                expectEquals (element->getStringAttribute (number), test.second);
            }
        });

        testCase ("Single-line output contains no line endings or indentation", [&]
        {
            expectSingleLineOutputIsUnchanged ("<a>t<b><c/></b></a>");
            expectSingleLineOutputIsUnchanged ("<a>t<b><c/></b><d/></a>");
            expectSingleLineOutputIsUnchanged ("<a><b>t<c><d/></c></b></a>");
            expectSingleLineOutputIsUnchanged ("<a>t<b/></a>");
            expectSingleLineOutputIsUnchanged ("<a>t<b>x</b></a>");
            expectSingleLineOutputIsUnchanged ("<a><b><c/></b></a>");
        });

        testCase ("Nested elements are indented by two spaces per level", [&]
        {
            XmlElement root { "a" };
            root.createNewChildElement ("b")->createNewChildElement ("c");

            expectEquals (root.toString (XmlElement::TextFormat{}.withoutHeader()),
                          String ("<a>\r\n"
                                  "  <b>\r\n"
                                  "    <c/>\r\n"
                                  "  </b>\r\n"
                                  "</a>\r\n"));
        });

        testCase ("An element following text stays on that line, and its children keep their indent", [&]
        {
            XmlElement root { "a" };
            auto* b = root.createNewChildElement ("b");
            b->addTextElement ("t");
            b->createNewChildElement ("c")->createNewChildElement ("d");

            expectEquals (root.toString (XmlElement::TextFormat{}.withoutHeader()),
                          String ("<a>\r\n"
                                  "  <b>t<c>\r\n"
                                  "      <d/>\r\n"
                                  "    </c>\r\n"
                                  "  </b>\r\n"
                                  "</a>\r\n"));
        });

        testCase ("Line breaks inside text are preserved, and the closing tag breaks only when the last child is an element", [&]
        {
            XmlElement p { "p" };
            p.addTextElement ("One, ");
            expectEquals (p.toString (XmlElement::TextFormat{}.withoutHeader()),
                          String ("<p>One, </p>\r\n"));

            // </p> starts a new line because the last child is an element
            auto* b = p.createNewChildElement ("b");
            b->addTextElement ("two");
            expectEquals (p.toString (XmlElement::TextFormat{}.withoutHeader()),
                          String ("<p>One, <b>two</b>\r\n</p>\r\n"));

            // the line break before </p> is the one stored in the text
            p.addTextElement (", three, \r\n");
            expectEquals (p.toString (XmlElement::TextFormat{}.withoutHeader()),
                          String ("<p>One, <b>two</b>, three, \r\n</p>\r\n"));

            // <i> follows that stored break immediately, and </p> starts a new
            // line because the last child is an element
            auto* i = p.createNewChildElement ("i");
            i->addTextElement ("four");
            expectEquals (p.toString (XmlElement::TextFormat{}.withoutHeader()),
                          String ("<p>One, <b>two</b>, three, \r\n<i>four</i>\r\n</p>\r\n"));

            // </p> stays on the same line because the last child is text
            p.addTextElement (", five!");
            expectEquals (p.toString (XmlElement::TextFormat{}.withoutHeader()),
                          String ("<p>One, <b>two</b>, three, \r\n<i>four</i>, five!</p>\r\n"));
        });

        testCase ("Wrapped attributes on a nested element keep the parent indent", [&]
        {
            XmlElement root { "a" };
            auto* child = root.createNewChildElement ("e");
            child->setAttribute ("a", "1234567890");
            child->setAttribute ("b", "x");

            auto format = XmlElement::TextFormat{}.withoutHeader();
            format.lineWrapLength = 10;
            expectEquals (root.toString (format),
                          String ("<a>\r\n"
                                  "  <e a=\"1234567890\"\r\n"
                                  "     b=\"x\"/>\r\n"
                                  "</a>\r\n"));
        });

        testCase ("A default header is written unless it is suppressed", [&]
        {
            const XmlElement root { "a" };

            expectEquals (root.toString(),
                          String ("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n\r\n<a/>\r\n"));
            expectEquals (root.toString (XmlElement::TextFormat{}.withoutHeader()),
                          String ("<a/>\r\n"));
            expectEquals (root.toString (XmlElement::TextFormat{}.singleLine()),
                          String ("<?xml version=\"1.0\" encoding=\"UTF-8\"?> <a/>"));
        });

        testCase ("A custom header, encoding and DTD can be supplied", [&]
        {
            const XmlElement root { "a" };

            auto encoded = XmlElement::TextFormat{};
            encoded.customEncoding = "UTF-16";
            expectEquals (root.toString (encoded),
                          String ("<?xml version=\"1.0\" encoding=\"UTF-16\"?>\r\n\r\n<a/>\r\n"));

            auto custom = XmlElement::TextFormat{};
            custom.customHeader = "<?header?>";
            custom.customEncoding = "UTF-16";
            custom.dtd = "<!DOCTYPE a>";
            expectEquals (root.toString (custom),
                          String ("<?header?>\r\n\r\n<!DOCTYPE a>\r\n<a/>\r\n"));
        });

        testCase ("Attributes are wrapped once the line wrap length is passed", [&]
        {
            XmlElement root { "e" };
            root.setAttribute ("a", "1234567890");
            root.setAttribute ("b", "x");

            // the wrapped attribute is indented to line up under the first one,
            // which is the tag name plus the angle bracket and separating space
            auto format = XmlElement::TextFormat{}.withoutHeader();
            format.lineWrapLength = 10;
            expectEquals (root.toString (format), String ("<e a=\"1234567890\"\r\n   b=\"x\"/>\r\n"));

            format.lineWrapLength = 100;
            expectEquals (root.toString (format), String ("<e a=\"1234567890\" b=\"x\"/>\r\n"));
        });

        testCase ("Illegal characters are escaped", [&]
        {
            XmlElement root { "a" };
            root.setAttribute ("v", "\"&<>");
            root.addTextElement ("\"&<>");

            // quotes are escaped in text content as well as in attribute values
            expectEquals (root.toString (XmlElement::TextFormat{}.singleLine().withoutHeader()),
                          String ("<a v=\"&quot;&amp;&lt;&gt;\">&quot;&amp;&lt;&gt;</a>"));
        });

        testCase ("Newlines are escaped in attributes but kept in text", [&]
        {
            XmlElement root { "a" };
            root.setAttribute ("v", "x\ny");
            root.addTextElement ("x\ny");

            expectEquals (root.toString (XmlElement::TextFormat{}.singleLine().withoutHeader()),
                          String ("<a v=\"x&#10;y\">x\ny</a>"));
        });

        testCase ("A document survives a round-trip through parseXML", [&]
        {
            XmlElement root { "a" };
            root.setAttribute ("one", 1);
            root.setAttribute ("two", "<&\">");
            auto* child = root.createNewChildElement ("b");
            child->setAttribute ("three", 2.5);
            child->addTextElement ("some text");
            root.createNewChildElement ("c");

            for (const auto& format : { XmlElement::TextFormat{},
                                        XmlElement::TextFormat{}.singleLine() })
            {
                const auto parsed = parseXML (root.toString (format));
                expect (parsed != nullptr);

                if (parsed != nullptr)
                    expect (parsed->isEquivalentTo (&root, false));
            }
        });

        testCase ("Attributes can be read back as strings, ints, doubles and bools", [&]
        {
            XmlElement root { "a" };
            root.setAttribute ("str", "hello");
            root.setAttribute ("int", 42);
            root.setAttribute ("dbl", 0.5);

            expect (root.hasAttribute ("str"));
            expect (! root.hasAttribute ("missing"));
            expectEquals (root.getNumAttributes(), 3);
            expectEquals (root.getAttributeName (0), String ("str"));
            expectEquals (root.getAttributeValue (0), String ("hello"));

            expectEquals (root.getStringAttribute ("str"), String ("hello"));
            expectEquals (root.getStringAttribute ("missing", "fallback"), String ("fallback"));
            expectEquals (root.getIntAttribute ("int"), 42);
            expectEquals (root.getIntAttribute ("missing", -1), -1);
            expectEquals (root.getDoubleAttribute ("dbl"), 0.5);
            expectEquals (root.getDoubleAttribute ("missing", -1.0), -1.0);

            expect (  root.compareAttribute ("str", "hello"));
            expect (! root.compareAttribute ("str", "HELLO"));
            expect (  root.compareAttribute ("str", "HELLO",  true));
            expect (! root.compareAttribute ("str", "HELLO!", true));

            // getBoolAttribute only looks at the first non-whitespace character
            for (const auto* trueValue : { "1", "t", "true", "y", "yes", "T", "Y" })
            {
                root.setAttribute ("bool", trueValue);
                expect (root.getBoolAttribute ("bool"));
            }

            for (const auto* falseValue : { "0", "f", "false", "n", "no", "F", "N", "" })
            {
                root.setAttribute ("bool", falseValue);
                expect (! root.getBoolAttribute ("bool"));
            }

            expect (root.getBoolAttribute ("missing", true));
        });

        testCase ("Attributes can be removed", [&]
        {
            XmlElement root { "a" };
            root.setAttribute ("one", 1);
            root.setAttribute ("two", 2);

            root.removeAttribute ("one");
            expect (! root.hasAttribute ("one"));
            expect (root.hasAttribute ("two"));
            expectEquals (root.getNumAttributes(), 1);

            root.removeAttribute ("missing");
            expectEquals (root.getNumAttributes(), 1);

            root.removeAllAttributes();
            expectEquals (root.getNumAttributes(), 0);
        });

        testCase ("Setting an attribute twice replaces the value and keeps its position", [&]
        {
            XmlElement root { "a" };
            root.setAttribute ("one", 1);
            root.setAttribute ("two", 2);
            root.setAttribute ("one", 3);

            expectEquals (root.getNumAttributes(), 2);
            expectEquals (root.getAttributeName (0), String ("one"));
            expectEquals (root.getIntAttribute ("one"), 3);
        });

        testCase ("Child elements can be added, inserted, replaced and removed", [&]
        {
            XmlElement root { "a" };
            root.addChildElement (new XmlElement ("second"));
            root.prependChildElement (new XmlElement ("first"));
            root.insertChildElement (new XmlElement ("third"), 2);
            root.insertChildElement (new XmlElement ("last"), -1);

            expectEquals (root.getNumChildElements(), 4);
            expectEquals (root.getChildElement (0)->getTagName(), String ("first"));
            expectEquals (root.getChildElement (1)->getTagName(), String ("second"));
            expectEquals (root.getChildElement (2)->getTagName(), String ("third"));
            expectEquals (root.getChildElement (3)->getTagName(), String ("last"));
            expect (root.getChildElement (4) == nullptr);

            auto* second = root.getChildElement (1);
            expect (root.containsChildElement (second));
            expect (root.findParentElementOf (second) == &root);
            expect (root.replaceChildElement (second, new XmlElement ("replaced")));
            expectEquals (root.getChildElement (1)->getTagName(), String ("replaced"));

            root.removeChildElement (root.getChildElement (0), true);
            expectEquals (root.getNumChildElements(), 3);
            expectEquals (root.getChildElement (0)->getTagName(), String ("replaced"));

            root.deleteAllChildElements();
            expectEquals (root.getNumChildElements(), 0);
            expect (root.getFirstChildElement() == nullptr);
        });

        testCase ("Child elements can be found by name and by attribute", [&]
        {
            XmlElement root { "a" };
            root.createNewChildElement ("b")->setAttribute ("id", "1");
            root.createNewChildElement ("c")->setAttribute ("id", "2");
            root.createNewChildElement ("b")->setAttribute ("id", "3");

            auto* firstB = root.getChildByName ("b");
            expect (firstB != nullptr);

            if (firstB != nullptr)
            {
                expectEquals (firstB->getStringAttribute ("id"), String ("1"));

                auto* nextB = firstB->getNextElementWithTagName ("b");
                expect (nextB != nullptr);

                if (nextB != nullptr)
                    expectEquals (nextB->getStringAttribute ("id"), String ("3"));
            }

            expect (root.getChildByName ("missing") == nullptr);

            auto* byAttribute = root.getChildByAttribute ("id", "2");
            expect (byAttribute != nullptr);

            if (byAttribute != nullptr)
                expectEquals (byAttribute->getTagName(), String ("c"));

            expect (root.getChildByAttribute ("id", "missing") == nullptr);

            root.deleteAllChildElementsWithTagName ("b");
            expectEquals (root.getNumChildElements(), 1);
            expectEquals (root.getChildElement (0)->getTagName(), String ("c"));
        });

        testCase ("Text elements can be read, replaced and removed", [&]
        {
            XmlElement root { "a" };
            root.addTextElement ("one ");
            root.createNewChildElement ("b")->addTextElement ("two");
            root.addTextElement (" three");

            expect (! root.isTextElement());
            expectEquals (root.getAllSubText(), String ("one two three"));
            expectEquals (root.getChildElementAllSubText ("b", "fallback"), String ("two"));
            expectEquals (root.getChildElementAllSubText ("missing", "fallback"), String ("fallback"));

            root.deleteAllTextElements();
            expectEquals (root.getNumChildElements(), 1);
            expectEquals (root.getAllSubText(), String ("two"));

            const std::unique_ptr<XmlElement> text { XmlElement::createTextElement ("content") };
            expect (text->isTextElement());
            expectEquals (text->getText(), String ("content"));

            // setText() is only valid on a text element
            text->setText ("replaced");
            expectEquals (text->getText(), String ("replaced"));
        });

        testCase ("isEquivalentTo compares tag names, attributes and children", [&]
        {
            const auto build = [] (const String& tag, const String& firstAttribute)
            {
                auto e = std::make_unique<XmlElement> (tag);
                e->setAttribute (Identifier (firstAttribute), 1);
                e->setAttribute ("other", 2);
                e->createNewChildElement ("child");
                return e;
            };

            const auto a = build ("tag", "one");
            const auto b = build ("tag", "one");
            const auto differentTag = build ("other", "one");

            expect (a->isEquivalentTo (a.get(), false));
            expect (a->isEquivalentTo (b.get(), false));
            expect (! a->isEquivalentTo (differentTag.get(), false));
            expect (! a->isEquivalentTo (nullptr, false));

            // the same attributes in a different order
            XmlElement reordered { "tag" };
            reordered.setAttribute ("other", 2);
            reordered.setAttribute ("one", 1);
            reordered.createNewChildElement ("child");

            expect (! a->isEquivalentTo (&reordered, false));
            expect (a->isEquivalentTo (&reordered, true));

            // an extra child makes them differ
            b->createNewChildElement ("extra");
            expect (! a->isEquivalentTo (b.get(), false));
        });

        testCase ("Child elements can be sorted", [&]
        {
            struct TagNameComparator
            {
                int compareElements (const XmlElement* first, const XmlElement* second) const
                {
                    return first->getTagName().compare (second->getTagName());
                }
            };

            XmlElement root { "a" };

            for (const auto* tag : { "c", "a", "b" })
                root.createNewChildElement (tag);

            TagNameComparator comparator;
            root.sortChildElements (comparator);

            expectEquals (root.getChildElement (0)->getTagName(), String ("a"));
            expectEquals (root.getChildElement (1)->getTagName(), String ("b"));
            expectEquals (root.getChildElement (2)->getTagName(), String ("c"));
        });

        testCase ("Tag names can be tested and changed", [&]
        {
            XmlElement root { "ns:tag" };

            expect (root.hasTagName ("ns:tag"));
            expect (! root.hasTagName ("tag"));
            expect (root.hasTagNameIgnoringNamespace ("tag"));
            expectEquals (root.getTagNameWithoutNamespace(), String ("tag"));

            root.setTagName ("other");
            expectEquals (root.getTagName(), String ("other"));
            expectEquals (root.getTagNameWithoutNamespace(), String ("other"));
        });

        testCase ("isValidXmlName rejects names XML does not allow", [&]
        {
            expect (XmlElement::isValidXmlName ("tag"));
            expect (XmlElement::isValidXmlName ("_tag-1.2"));
            expect (XmlElement::isValidXmlName ("ns:tag"));

            expect (! XmlElement::isValidXmlName (""));
            expect (! XmlElement::isValidXmlName ("1tag"));
            expect (! XmlElement::isValidXmlName ("-tag"));
            expect (! XmlElement::isValidXmlName ("has space"));
            expect (! XmlElement::isValidXmlName ("has\"quote"));
        });
    }

private:
    void expectSingleLineOutputIsUnchanged (const String& text)
    {
        const auto parsed = parseXML (text);
        expect (parsed != nullptr);

        if (parsed == nullptr)
            return;

        expectEquals (parsed->toString (XmlElement::TextFormat{}.singleLine().withoutHeader()), text);
    }
};

static XmlElementTests xmlElementTests;

#endif

} // namespace juce
