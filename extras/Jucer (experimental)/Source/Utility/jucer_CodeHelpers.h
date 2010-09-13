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
    const String indent (const String& code, const int numSpaces, bool indentFirstLine);
    const String makeValidIdentifier (String s, bool capitalise, bool removeColons, bool allowTemplates);
    const String addEscapeChars (const String& text);
    const String createIncludeStatement (const File& includeFile, const File& targetFile);
    const String makeHeaderGuardName (const File& file);

    const String stringLiteral (const String& text);
    const String boolLiteral (bool b);
    const String floatLiteral (float v);
    const String doubleLiteral (double v);

    const String colourToCode (const Colour& col);
    const String justificationToCode (const Justification& justification);
    const String castToFloat (const String& expression);
    const String castToInt (const String& expression);
    const String fontToCode (const Font& font);
    const String alignFunctionCallParams (const String& call, const StringArray& parameters, int maxLineLength);

    void writeDataAsCppLiteral (const MemoryBlock& data, OutputStream& out);

    void createStringMatcher (OutputStream& out, const String& utf8PointerVariable,
                              const StringArray& strings, const StringArray& codeToExecute, const int indentLevel);

}


#endif  // __JUCER_CODEUTILITIES_H_B86AA5D2__
