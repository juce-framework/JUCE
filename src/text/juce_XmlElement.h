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

#ifndef __JUCE_XMLELEMENT_JUCEHEADER__
#define __JUCE_XMLELEMENT_JUCEHEADER__

#include "juce_String.h"
#include "../io/streams/juce_OutputStream.h"
#include "../io/files/juce_File.h"


//==============================================================================
/** A handy macro to make it easy to iterate all the child elements in an XmlElement.

    The parentXmlElement should be a reference to the parent XML, and the childElementVariableName
    will be the name of a pointer to each child element.

    E.g. @code
    XmlElement* myParentXml = createSomeKindOfXmlDocument();

    forEachXmlChildElement (*myParentXml, child)
    {
        if (child->hasTagName (T("FOO")))
            doSomethingWithXmlElement (child);
    }

    @endcode

    @see forEachXmlChildElementWithTagName
*/
#define forEachXmlChildElement(parentXmlElement, childElementVariableName) \
\
    for (XmlElement* childElementVariableName = (parentXmlElement).getFirstChildElement(); \
         childElementVariableName != 0; \
         childElementVariableName = childElementVariableName->getNextElement())

/** A macro that makes it easy to iterate all the child elements of an XmlElement
    which have a specified tag.

    This does the same job as the forEachXmlChildElement macro, but only for those
    elements that have a particular tag name.

    The parentXmlElement should be a reference to the parent XML, and the childElementVariableName
    will be the name of a pointer to each child element. The requiredTagName is the
    tag name to match.

    E.g. @code
    XmlElement* myParentXml = createSomeKindOfXmlDocument();

    forEachXmlChildElementWithTagName (*myParentXml, child, T("MYTAG"))
    {
        // the child object is now guaranteed to be a <MYTAG> element..
        doSomethingWithMYTAGElement (child);
    }

    @endcode

    @see forEachXmlChildElement
*/
#define forEachXmlChildElementWithTagName(parentXmlElement, childElementVariableName, requiredTagName) \
\
    for (XmlElement* childElementVariableName = (parentXmlElement).getChildByName (requiredTagName); \
         childElementVariableName != 0; \
         childElementVariableName = childElementVariableName->getNextElementWithTagName (requiredTagName))


//==============================================================================
/** Used to build a tree of elements representing an XML document.

    An XML document can be parsed into a tree of XmlElements, each of which
    represents an XML tag structure, and which may itself contain other
    nested elements.

    An XmlElement can also be converted back into a text document, and has
    lots of useful methods for manipulating its attributes and sub-elements,
    so XmlElements can actually be used as a handy general-purpose data
    structure.

    Here's an example of parsing some elements: @code
    // check we're looking at the right kind of document..
    if (myElement->hasTagName (T("ANIMALS")))
    {
        // now we'll iterate its sub-elements looking for 'giraffe' elements..
        forEachXmlChildElement (*myElement, e)
        {
            if (e->hasTagName (T("GIRAFFE")))
            {
                // found a giraffe, so use some of its attributes..

                String giraffeName  = e->getStringAttribute ("name");
                int giraffeAge      = e->getIntAttribute ("age");
                bool isFriendly     = e->getBoolAttribute ("friendly");
            }
        }
    }
    @endcode

    And here's an example of how to create an XML document from scratch: @code
    // create an outer node called "ANIMALS"
    XmlElement animalsList ("ANIMALS");

    for (int i = 0; i < numAnimals; ++i)
    {
        // create an inner element..
        XmlElement* giraffe = new XmlElement ("GIRAFFE");

        giraffe->setAttribute ("name", "nigel");
        giraffe->setAttribute ("age", 10);
        giraffe->setAttribute ("friendly", true);

        // ..and add our new element to the parent node
        animalsList.addChildElement (giraffe);
    }

    // now we can turn the whole thing into a text document..
    String myXmlDoc = animalsList.createDocument (String::empty);
    @endcode

    @see XmlDocument
*/
class JUCE_API  XmlElement
{
public:
    //==============================================================================
    /** Creates an XmlElement with this tag name. */
    XmlElement (const String& tagName) throw();

    /** Creates a (deep) copy of another element. */
    XmlElement (const XmlElement& other) throw();

    /** Creates a (deep) copy of another element. */
    const XmlElement& operator= (const XmlElement& other) throw();

    /** Deleting an XmlElement will also delete all its child elements. */
    ~XmlElement() throw();

    //==============================================================================
    /** Compares two XmlElements to see if they contain the same text and attiributes.

        The elements are only considered equivalent if they contain the same attiributes
        with the same values, and have the same sub-nodes.

        @param other                    the other element to compare to
        @param ignoreOrderOfAttributes  if true, this means that two elements with the
                                        same attributes in a different order will be
                                        considered the same; if false, the attributes must
                                        be in the same order as well
    */
    bool isEquivalentTo (const XmlElement* const other,
                         const bool ignoreOrderOfAttributes) const throw();

    //==============================================================================
    /** Returns an XML text document that represents this element.

        The string returned can be parsed to recreate the same XmlElement that
        was used to create it.

        @param dtdToUse         the DTD to add to the document
        @param allOnOneLine     if true, this means that the document will not contain any
                                linefeeds, so it'll be smaller but not very easy to read.
        @param includeXmlHeader whether to add the "<?xml version..etc" line at the start of the
                                document
        @param encodingType     the character encoding format string to put into the xml
                                header
        @param lineWrapLength   the line length that will be used before items get placed on
                                a new line. This isn't an absolute maximum length, it just
                                determines how lists of attributes get broken up
        @see writeToStream, writeToFile
    */
    const String createDocument (const String& dtdToUse,
                                 const bool allOnOneLine = false,
                                 const bool includeXmlHeader = true,
                                 const tchar* const encodingType = JUCE_T("UTF-8"),
                                 const int lineWrapLength = 60) const throw();

    /** Writes the document to a stream as UTF-8.

        @param output           the stream to write to
        @param dtdToUse         the DTD to add to the document
        @param allOnOneLine     if true, this means that the document will not contain any
                                linefeeds, so it'll be smaller but not very easy to read.
        @param includeXmlHeader whether to add the "<?xml version..etc" line at the start of the
                                document
        @param encodingType     the character encoding format string to put into the xml
                                header
        @param lineWrapLength   the line length that will be used before items get placed on
                                a new line. This isn't an absolute maximum length, it just
                                determines how lists of attributes get broken up
        @see writeToFile, createDocument
    */
    void writeToStream (OutputStream& output,
                        const String& dtdToUse,
                        const bool allOnOneLine = false,
                        const bool includeXmlHeader = true,
                        const tchar* const encodingType = JUCE_T("UTF-8"),
                        const int lineWrapLength = 60) const throw();

    /** Writes the element to a file as an XML document.

        To improve safety in case something goes wrong while writing the file, this
        will actually write the document to a new temporary file in the same
        directory as the destination file, and if this succeeds, it will rename this
        new file as the destination file (overwriting any existing file that was there).

        @param destinationFile  the file to write to. If this already exists, it will be
                                overwritten.
        @param dtdToUse         the DTD to add to the document
        @param encodingType     the character encoding format string to put into the xml
                                header
        @param lineWrapLength   the line length that will be used before items get placed on
                                a new line. This isn't an absolute maximum length, it just
                                determines how lists of attributes get broken up
        @returns    true if the file is written successfully; false if something goes wrong
                    in the process
        @see createDocument
    */
    bool writeToFile (const File& destinationFile,
                      const String& dtdToUse,
                      const tchar* const encodingType = JUCE_T("UTF-8"),
                      const int lineWrapLength = 60) const throw();

    //==============================================================================
    /** Returns this element's tag type name.

        E.g. for an element such as \<MOOSE legs="4" antlers="2">, this would return
        "MOOSE".

        @see hasTagName
    */
    inline const String& getTagName() const throw()  { return tagName; }

    /** Tests whether this element has a particular tag name.

        @param possibleTagName  the tag name you're comparing it with

        @see getTagName
    */
    bool hasTagName (const tchar* const possibleTagName) const throw();

    //==============================================================================
    /** Returns the number of XML attributes this element contains.

        E.g. for an element such as \<MOOSE legs="4" antlers="2">, this would
        return 2.
    */
    int getNumAttributes() const throw();

    /** Returns the name of one of the elements attributes.

        E.g. for an element such as \<MOOSE legs="4" antlers="2">, then
        getAttributeName(1) would return "antlers".

        @see getAttributeValue, getStringAttribute
    */
    const String& getAttributeName (const int attributeIndex) const throw();

    /** Returns the value of one of the elements attributes.

        E.g. for an element such as \<MOOSE legs="4" antlers="2">, then
        getAttributeName(1) would return "2".

        @see getAttributeName, getStringAttribute
    */
    const String& getAttributeValue (const int attributeIndex) const throw();

    //==============================================================================
    // Attribute-handling methods..

    /** Checks whether the element contains an attribute with a certain name. */
    bool hasAttribute (const tchar* const attributeName) const throw();

    /** Returns the value of a named attribute.

        @param attributeName        the name of the attribute to look up
        @param defaultReturnValue   a value to return if the element doesn't have an attribute
                                    with this name
    */
    const String getStringAttribute (const tchar* const attributeName,
                                     const tchar* const defaultReturnValue = 0) const throw();

    /** Compares the value of a named attribute with a value passed-in.

        @param attributeName            the name of the attribute to look up
        @param stringToCompareAgainst   the value to compare it with
        @param ignoreCase               whether the comparison should be case-insensitive
        @returns    true if the value of the attribute is the same as the string passed-in;
                    false if it's different (or if no such attribute exists)
    */
    bool compareAttribute (const tchar* const attributeName,
                           const tchar* const stringToCompareAgainst,
                           const bool ignoreCase = false) const throw();

    /** Returns the value of a named attribute as an integer.

        This will try to find the attribute and convert it to an integer (using
        the String::getIntValue() method).

        @param attributeName        the name of the attribute to look up
        @param defaultReturnValue   a value to return if the element doesn't have an attribute
                                    with this name
        @see setAttribute (const tchar* const, int)
    */
    int getIntAttribute (const tchar* const attributeName,
                         const int defaultReturnValue = 0) const throw();

    /** Returns the value of a named attribute as floating-point.

        This will try to find the attribute and convert it to an integer (using
        the String::getDoubleValue() method).

        @param attributeName        the name of the attribute to look up
        @param defaultReturnValue   a value to return if the element doesn't have an attribute
                                    with this name
        @see setAttribute (const tchar* const, double)
    */
    double getDoubleAttribute (const tchar* const attributeName,
                               const double defaultReturnValue = 0.0) const throw();

    /** Returns the value of a named attribute as a boolean.

        This will try to find the attribute and interpret it as a boolean. To do this,
        it'll return true if the value is "1", "true", "y", etc, or false for other
        values.

        @param attributeName        the name of the attribute to look up
        @param defaultReturnValue   a value to return if the element doesn't have an attribute
                                    with this name
    */
    bool getBoolAttribute (const tchar* const attributeName,
                           const bool defaultReturnValue = false) const throw();

    /** Adds a named attribute to the element.

        If the element already contains an attribute with this name, it's value will
        be updated to the new value. If there's no such attribute yet, a new one will
        be added.

        Note that there are other setAttribute() methods that take integers,
        doubles, etc. to make it easy to store numbers.

        @param attributeName        the name of the attribute to set
        @param newValue             the value to set it to
        @see removeAttribute
    */
    void setAttribute (const tchar* const attributeName,
                       const String& newValue) throw();

    /** Adds a named attribute to the element.

        If the element already contains an attribute with this name, it's value will
        be updated to the new value. If there's no such attribute yet, a new one will
        be added.

        Note that there are other setAttribute() methods that take integers,
        doubles, etc. to make it easy to store numbers.

        @param attributeName        the name of the attribute to set
        @param newValue             the value to set it to
    */
    void setAttribute (const tchar* const attributeName,
                       const tchar* const newValue) throw();

    /** Adds a named attribute to the element, setting it to an integer value.

        If the element already contains an attribute with this name, it's value will
        be updated to the new value. If there's no such attribute yet, a new one will
        be added.

        Note that there are other setAttribute() methods that take integers,
        doubles, etc. to make it easy to store numbers.

        @param attributeName        the name of the attribute to set
        @param newValue             the value to set it to
    */
    void setAttribute (const tchar* const attributeName,
                       const int newValue) throw();

    /** Adds a named attribute to the element, setting it to a floating-point value.

        If the element already contains an attribute with this name, it's value will
        be updated to the new value. If there's no such attribute yet, a new one will
        be added.

        Note that there are other setAttribute() methods that take integers,
        doubles, etc. to make it easy to store numbers.

        @param attributeName        the name of the attribute to set
        @param newValue             the value to set it to
    */
    void setAttribute (const tchar* const attributeName,
                       const double newValue) throw();

    /** Removes a named attribute from the element.

        @param attributeName    the name of the attribute to remove
        @see removeAllAttributes
    */
    void removeAttribute (const tchar* const attributeName) throw();

    /** Removes all attributes from this element.
    */
    void removeAllAttributes() throw();

    //==============================================================================
    // Child element methods..

    /** Returns the first of this element's sub-elements.

        see getNextElement() for an example of how to iterate the sub-elements.

        @see forEachXmlChildElement
    */
    XmlElement* getFirstChildElement() const throw()    { return firstChildElement; }

    /** Returns the next of this element's siblings.

        This can be used for iterating an element's sub-elements, e.g.
        @code
        XmlElement* child = myXmlDocument->getFirstChildElement();

        while (child != 0)
        {
            ...do stuff with this child..

            child = child->getNextElement();
        }
        @endcode

        Note that when iterating the child elements, some of them might be
        text elements as well as XML tags - use isTextElement() to work this
        out.

        Also, it's much easier and neater to use this method indirectly via the
        forEachXmlChildElement macro.

        @returns    the sibling element that follows this one, or zero if this is the last
                    element in its parent

        @see getNextElement, isTextElement, forEachXmlChildElement
    */
    inline XmlElement* getNextElement() const throw()   { return nextElement; }

    /** Returns the next of this element's siblings which has the specified tag
        name.

        This is like getNextElement(), but will scan through the list until it
        finds an element with the given tag name.

        @see getNextElement, forEachXmlChildElementWithTagName
    */
    XmlElement* getNextElementWithTagName (const tchar* const requiredTagName) const;

    /** Returns the number of sub-elements in this element.

        @see getChildElement
    */
    int getNumChildElements() const throw();

    /** Returns the sub-element at a certain index.

        It's not very efficient to iterate the sub-elements by index - see
        getNextElement() for an example of how best to iterate.

        @returns the n'th child of this element, or 0 if the index is out-of-range
        @see getNextElement, isTextElement, getChildByName
    */
    XmlElement* getChildElement (const int index) const throw();

    /** Returns the first sub-element with a given tag-name.

        @param tagNameToLookFor     the tag name of the element you want to find
        @returns the first element with this tag name, or 0 if none is found
        @see getNextElement, isTextElement, getChildElement
    */
    XmlElement* getChildByName (const tchar* const tagNameToLookFor) const throw();

    //==============================================================================
    /** Appends an element to this element's list of children.

        Child elements are deleted automatically when their parent is deleted, so
        make sure the object that you pass in will not be deleted by anything else,
        and make sure it's not already the child of another element.

        @see getFirstChildElement, getNextElement, getNumChildElements,
             getChildElement, removeChildElement
    */
    void addChildElement (XmlElement* const newChildElement) throw();

    /** Inserts an element into this element's list of children.

        Child elements are deleted automatically when their parent is deleted, so
        make sure the object that you pass in will not be deleted by anything else,
        and make sure it's not already the child of another element.

        @param newChildNode     the element to add
        @param indexToInsertAt  the index at which to insert the new element - if this is
                                below zero, it will be added to the end of the list
        @see addChildElement, insertChildElement
    */
    void insertChildElement (XmlElement* const newChildNode,
                             int indexToInsertAt) throw();

    /** Replaces one of this element's children with another node.

        If the current element passed-in isn't actually a child of this element,
        this will return false and the new one won't be added. Otherwise, the
        existing element will be deleted, replaced with the new one, and it
        will return true.
    */
    bool replaceChildElement (XmlElement* const currentChildElement,
                              XmlElement* const newChildNode) throw();

    /** Removes a child element.

        @param childToRemove            the child to look for and remove
        @param shouldDeleteTheChild     if true, the child will be deleted, if false it'll
                                        just remove it
    */
    void removeChildElement (XmlElement* const childToRemove,
                             const bool shouldDeleteTheChild) throw();

    /** Deletes all the child elements in the element.

        @see removeChildElement, deleteAllChildElementsWithTagName
    */
    void deleteAllChildElements() throw();

    /** Deletes all the child elements with a given tag name.

        @see removeChildElement
    */
    void deleteAllChildElementsWithTagName (const tchar* const tagName) throw();

    /** Returns true if the given element is a child of this one. */
    bool containsChildElement (const XmlElement* const possibleChild) const throw();

    /** Recursively searches all sub-elements to find one that contains the specified
        child element.
    */
    XmlElement* findParentElementOf (const XmlElement* const elementToLookFor) throw();

    //==============================================================================
    /** Sorts the child elements using a comparator.

        This will use a comparator object to sort the elements into order. The object
        passed must have a method of the form:
        @code
        int compareElements (const XmlElement* first, const XmlElement* second);
        @endcode

        ..and this method must return:
          - a value of < 0 if the first comes before the second
          - a value of 0 if the two objects are equivalent
          - a value of > 0 if the second comes before the first

        To improve performance, the compareElements() method can be declared as static or const.

        @param comparator   the comparator to use for comparing elements.
        @param retainOrderOfEquivalentItems     if this is true, then items
                            which the comparator says are equivalent will be
                            kept in the order in which they currently appear
                            in the array. This is slower to perform, but may
                            be important in some cases. If it's false, a faster
                            algorithm is used, but equivalent elements may be
                            rearranged.
    */
    template <class ElementComparator>
    void sortChildElements (ElementComparator& comparator,
                            const bool retainOrderOfEquivalentItems = false) throw()
    {
        const int num = getNumChildElements();

        if (num > 1)
        {
            HeapBlock <XmlElement*> elems (num);
            getChildElementsAsArray (elems);
            sortArray (comparator, (XmlElement**) elems, 0, num - 1, retainOrderOfEquivalentItems);
            reorderChildElements (elems, num);
        }
    }

    //==============================================================================
    /** Returns true if this element is a section of text.

        Elements can either be an XML tag element or a secton of text, so this
        is used to find out what kind of element this one is.

        @see getAllText, addTextElement, deleteAllTextElements
    */
    bool isTextElement() const throw();

    /** Returns the text for a text element.

        Note that if you have an element like this:

        @code<xyz>hello</xyz>@endcode

        then calling getText on the "xyz" element won't return "hello", because that is
        actually stored in a special text sub-element inside the xyz element. To get the
        "hello" string, you could either call getText on the (unnamed) sub-element, or
        use getAllSubText() to do this automatically.

        @see isTextElement, getAllSubText, getChildElementAllSubText
    */
    const String getText() const throw();

    /** Sets the text in a text element.

        Note that this is only a valid call if this element is a text element. If it's
        not, then no action will be performed.
    */
    void setText (const String& newText) throw();

    /** Returns all the text from this element's child nodes.

        This iterates all the child elements and when it finds text elements,
        it concatenates their text into a big string which it returns.

        E.g. @code<xyz> hello <x></x> there </xyz>@endcode
        if you called getAllSubText on the "xyz" element, it'd return "hello there".

        @see isTextElement, getChildElementAllSubText, getText, addTextElement
    */
    const String getAllSubText() const throw();

    /** Returns all the sub-text of a named child element.

        If there is a child element with the given tag name, this will return
        all of its sub-text (by calling getAllSubText() on it). If there is
        no such child element, this will return the default string passed-in.

        @see getAllSubText
    */
    const String getChildElementAllSubText (const tchar* const childTagName,
                                            const String& defaultReturnValue) const throw();

    /** Appends a section of text to this element.

        @see isTextElement, getText, getAllSubText
    */
    void addTextElement (const String& text) throw();

    /** Removes all the text elements from this element.

        @see isTextElement, getText, getAllSubText, addTextElement
    */
    void deleteAllTextElements() throw();

    /** Creates a text element that can be added to a parent element.
    */
    static XmlElement* createTextElement (const String& text) throw();


    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    friend class XmlDocument;

    String tagName;
    XmlElement* firstChildElement;
    XmlElement* nextElement;

    struct XmlAttributeNode
    {
        XmlAttributeNode (const XmlAttributeNode& other) throw();
        XmlAttributeNode (const String& name, const String& value) throw();

        String name, value;
        XmlAttributeNode* next;

    private:
        const XmlAttributeNode& operator= (const XmlAttributeNode&);
    };

    XmlAttributeNode* attributes;

    XmlElement (int) throw(); // for internal use
    XmlElement (const tchar* const tagNameText, const int nameLen) throw();

    void copyChildrenAndAttributesFrom (const XmlElement& other) throw();

    void writeElementAsText (OutputStream& out,
                             const int indentationLevel,
                             const int lineWrapLength) const throw();

    void getChildElementsAsArray (XmlElement**) const throw();
    void reorderChildElements (XmlElement** const, const int) throw();
};


#endif   // __JUCE_XMLELEMENT_JUCEHEADER__
