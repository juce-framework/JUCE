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

#ifndef __JUCE_CODETOKENISER_JUCEHEADER__
#define __JUCE_CODETOKENISER_JUCEHEADER__

#include "juce_CodeDocument.h"


//==============================================================================
/**
    A base class for tokenising code so that the syntax can be displayed in a
    code editor.

    @see CodeDocument, CodeEditorComponent
*/
class JUCE_API  CodeTokeniser
{
public:
    CodeTokeniser()                 {}
    virtual ~CodeTokeniser()        {}

    //==============================================================================
    /** Reads the next token from the source and returns its token type.

        This must leave the source pointing to the first character in the
        next token.
    */
    virtual int readNextToken (CodeDocument::Iterator& source) = 0;

    /** Returns a list of the names of the token types this analyser uses.

        The index in this list must match the token type numbers that are
        returned by readNextToken().
    */
    virtual const StringArray getTokenTypes() = 0;

    /** Returns a suggested syntax highlighting colour for a specified
        token type.
    */
    virtual const Colour getDefaultColour (const int tokenType) = 0;


    //==============================================================================
    juce_UseDebuggingNewOperator
};


#endif   // __JUCE_CODETOKENISER_JUCEHEADER__
