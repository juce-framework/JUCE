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

#ifndef __JUCE_XMLDOCUMENT_JUCEHEADER__
#define __JUCE_XMLDOCUMENT_JUCEHEADER__

#include "juce_XmlElement.h"
#include "juce_StringArray.h"
#include "../io/files/juce_File.h"
#include "../io/streams/juce_InputSource.h"


//==============================================================================
/**
    Parses a text-based XML document and creates an XmlElement object from it.

    The parser will parse DTDs to load external entities but won't
    check the document for validity against the DTD.

    e.g.
    @code

    XmlDocument myDocument (File ("myfile.xml"));
    XmlElement* mainElement = myDocument.getDocumentElement();

    if (mainElement == 0)
    {
        String error = myDocument.getLastParseError();
    }
    else
    {
        ..use the element
    }

    @endcode

    @see XmlElement
*/
class JUCE_API  XmlDocument
{
public:
    //==============================================================================
    /** Creates an XmlDocument from the xml text.

        The text doesn't actually get parsed until the getDocumentElement() method is
        called.
    */
    XmlDocument (const String& documentText) throw();

    /** Creates an XmlDocument from a file.

        The text doesn't actually get parsed until the getDocumentElement() method is
        called.
    */
    XmlDocument (const File& file);

    /** Destructor. */
    ~XmlDocument() throw();

    /** Creates an XmlElement object to represent the main document node.

        This method will do the actual parsing of the text, and if there's a
        parse error, it may returns 0 (and you can find out the error using
        the getLastParseError() method).

        @param onlyReadOuterDocumentElement     if true, the parser will only read the
                                                first section of the file, and will only
                                                return the outer document element - this
                                                allows quick checking of large files to
                                                see if they contain the correct type of
                                                tag, without having to parse the entire file
        @returns    a new XmlElement which the caller will need to delete, or null if
                    there was an error.
        @see getLastParseError
    */
    XmlElement* getDocumentElement (const bool onlyReadOuterDocumentElement = false);

    /** Returns the parsing error that occurred the last time getDocumentElement was called.

        @returns the error, or an empty string if there was no error.
    */
    const String& getLastParseError() const throw();

    /** Sets an input source object to use for parsing documents that reference external entities.

        If the document has been created from a file, this probably won't be needed, but
        if you're parsing some text and there might be a DTD that references external
        files, you may need to create a custom input source that can retrieve the
        other files it needs.

        The object that is passed-in will be deleted automatically when no longer needed.

        @see InputSource
    */
    void setInputSource (InputSource* const newSource) throw();

    /** Sets a flag to change the treatment of empty text elements.

        If this is true (the default state), then any text elements that contain only
        whitespace characters will be ingored during parsing. If you need to catch
        whitespace-only text, then you should set this to false before calling the
        getDocumentElement() method.
    */
    void setEmptyTextElementsIgnored (const bool shouldBeIgnored) throw();

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    String originalText;
    const tchar* input;
    bool outOfData, errorOccurred;

    bool identifierLookupTable [128];
    String lastError, dtdText;
    StringArray tokenisedDTD;
    bool needToLoadDTD, ignoreEmptyTextElements;
    ScopedPointer <InputSource> inputSource;

    void setLastError (const String& desc, const bool carryOn) throw();
    void skipHeader() throw();
    void skipNextWhiteSpace() throw();
    tchar readNextChar() throw();
    XmlElement* readNextElement (const bool alsoParseSubElements) throw();
    void readChildElements (XmlElement* parent) throw();
    int findNextTokenLength() throw();
    void readQuotedString (String& result) throw();
    void readEntity (String& result) throw();
    static bool isXmlIdentifierCharSlow (const tchar c) throw();
    bool isXmlIdentifierChar (const tchar c) const throw();

    const String getFileContents (const String& filename) const;
    const String expandEntity (const String& entity);
    const String expandExternalEntity (const String& entity);
    const String getParameterEntity (const String& entity);
};


#endif   // __JUCE_XMLDOCUMENT_JUCEHEADER__
