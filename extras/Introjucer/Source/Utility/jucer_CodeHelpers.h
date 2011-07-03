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

#ifndef __JUCER_CODEUTILITIES_H_B86AA5D2__
#define __JUCER_CODEUTILITIES_H_B86AA5D2__


//==============================================================================
namespace CodeHelpers
{
    String indent (const String& code, const int numSpaces, bool indentFirstLine);
    String makeValidIdentifier (String s, bool capitalise, bool removeColons, bool allowTemplates);
    String addEscapeChars (const String& text);
    String createIncludeStatement (const File& includeFile, const File& targetFile);
    String makeHeaderGuardName (const File& file);
    String makeBinaryDataIdentifierName (const File& file);

    String stringLiteral (const String& text);
    String stringLiteralIfNotEmpty (const String& text); // if the string's empty, this returns an empty string
    String boolLiteral (bool b);
    String floatLiteral (float v);
    String doubleLiteral (double v);

    String colourToCode (const Colour& col);
    String justificationToCode (const Justification& justification);
    String castToFloat (const String& expression);
    String castToInt (const String& expression);
    String fontToCode (const Font& font);
    String alignFunctionCallParams (const String& call, const StringArray& parameters, int maxLineLength);

    void writeDataAsCppLiteral (const MemoryBlock& data, OutputStream& out);

    void createStringMatcher (OutputStream& out, const String& utf8PointerVariable,
                              const StringArray& strings, const StringArray& codeToExecute, const int indentLevel);

}


#endif  // __JUCER_CODEUTILITIES_H_B86AA5D2__
