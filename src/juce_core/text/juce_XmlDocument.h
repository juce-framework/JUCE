/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#ifndef __JUCE_XMLDOCUMENT_JUCEHEADER__
#define __JUCE_XMLDOCUMENT_JUCEHEADER__

#include "juce_XmlElement.h"
#include "juce_StringArray.h"
#include "../io/files/juce_File.h"
#include "../io/juce_InputStream.h"


//==============================================================================
/**
    Used by the XmlDocument class to find a document's associated files.

    Because an XML document might need to reference other files for its
    external DTDs, this class can be used to create input streams for
    these files.

    @see XmlDocument
*/
class JUCE_API  XmlInputSource
{
protected:
    /** Creates a default source that can read from files. */
    XmlInputSource() throw();

public:
    /** Destructor. */
    virtual ~XmlInputSource();

    /** Returns a new InputStream to read a required file.

        @param filename     the partial filename of a file that needs to be read,
                            or an empty string to open the root document that the
                            source refers to
        @returns            an inputstream that the caller will delete, or 0 if
                            the filename isn't found.
    */
    virtual InputStream* createInputStreamFor (const String& filename) = 0;
};


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

        @see XmlInputSource
    */
    void setInputSource (XmlInputSource* const newSource) throw();


    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    String originalText;
    const tchar* input;
    bool outOfData, errorOccurred;

    bool identifierLookupTable [128];
    String lastError, dtdText;
    StringArray tokenisedDTD;
    bool needToLoadDTD;
    XmlInputSource* inputSource;

    void setLastError (const String& desc, const bool carryOn) throw();
    void skipHeader() throw();
    void skipNextWhiteSpace() throw();
    tchar readNextChar() throw();
    XmlElement* readNextElement (const bool alsoParseSubElements) throw();
    void readChildElements (XmlElement* parent) throw();
    int findNextTokenLength() throw();
    void readQuotedString (String& result) throw();
    void readEntity (String& result) throw();

    const String getFileContents (const String& filename) const;
    const String expandEntity (const String& entity);
    const String expandExternalEntity (const String& entity);
    const String getParameterEntity (const String& entity);
};


#endif   // __JUCE_XMLDOCUMENT_JUCEHEADER__
