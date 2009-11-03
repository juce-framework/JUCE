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

#ifndef __JUCE_CPLUSPLUSCODETOKENISER_JUCEHEADER__
#define __JUCE_CPLUSPLUSCODETOKENISER_JUCEHEADER__

#include "juce_CodeTokeniser.h"


//==============================================================================
/**
    A simple lexical analyser for syntax colouring of C++ code.

    @see SyntaxAnalyser, CodeEditorComponent, CodeDocument
*/
class JUCE_API  CPlusPlusCodeTokeniser    : public CodeTokeniser
{
public:
    //==============================================================================
    CPlusPlusCodeTokeniser();
    ~CPlusPlusCodeTokeniser();

    //==============================================================================
    enum TokenType
    {
        tokenType_error = 0,
        tokenType_comment,
        tokenType_builtInKeyword,
        tokenType_identifier,
        tokenType_integerLiteral,
        tokenType_floatLiteral,
        tokenType_stringLiteral,
        tokenType_operator,
        tokenType_bracket,
        tokenType_punctuation,
        tokenType_preprocessor
    };

    //==============================================================================
    int readNextToken (CodeDocument::Iterator& source);
    const StringArray getTokenTypes();
    const Colour getDefaultColour (const int tokenType);

    //==============================================================================
    juce_UseDebuggingNewOperator
};


#endif   // __JUCE_CPLUSPLUSCODETOKENISER_JUCEHEADER__
