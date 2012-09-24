/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

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

#include "juce_CPlusPlusCodeTokeniserFunctions.h"


//==============================================================================
CPlusPlusCodeTokeniser::CPlusPlusCodeTokeniser() {}
CPlusPlusCodeTokeniser::~CPlusPlusCodeTokeniser() {}

int CPlusPlusCodeTokeniser::readNextToken (CodeDocument::Iterator& source)
{
    return CppTokeniserFunctions::readNextToken (source);
}

CodeEditorComponent::ColourScheme CPlusPlusCodeTokeniser::getDefaultColourScheme()
{
    struct Type
    {
        const char* name;
        uint32 colour;
    };

    const Type types[] =
    {
        { "Error",              0xffcc0000 },
        { "Comment",            0xff00aa00 },
        { "Keyword",            0xff0000cc },
        { "Operator",           0xff225500 },
        { "Identifier",         0xff000000 },
        { "Integer",            0xff880000 },
        { "Float",              0xff885500 },
        { "String",             0xff990099 },
        { "Bracket",            0xff000055 },
        { "Punctuation",        0xff004400 },
        { "Preprocessor Text",  0xff660000 }
    };

    CodeEditorComponent::ColourScheme cs;

    for (unsigned int i = 0; i < sizeof (types) / sizeof (types[0]); ++i)  // (NB: numElementsInArray doesn't work here in GCC4.2)
        cs.set (types[i].name, Colour (types[i].colour));

    return cs;
}

bool CPlusPlusCodeTokeniser::isReservedKeyword (const String& token) noexcept
{
    return CppTokeniserFunctions::isReservedKeyword (token.getCharPointer(), token.length());
}
