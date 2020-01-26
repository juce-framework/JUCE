/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#pragma once


//==============================================================================
namespace CodeHelpers
{
    String indent (const String& code, int numSpaces, bool indentFirstLine);
    String unindent (const String& code, int numSpaces);
    String makeValidIdentifier (String s, bool capitalise, bool removeColons,
                                bool allowTemplates, bool allowAsterisks = false);
    String makeBinaryDataIdentifierName (const File& file);

    String createIncludeStatement (const File& includedFile, const File& targetFile);
    String createIncludeStatement (const String& includePath);
    String createIncludePathIncludeStatement (const String& includedFilename);

    String stringLiteral (const String& text, int maxLineLength = -1);
    String floatLiteral (double value, int numDecPlaces);
    String boolLiteral (bool value);

    String colourToCode (Colour);
    String justificationToCode (Justification);

    String alignFunctionCallParams (const String& call, const StringArray& parameters, int maxLineLength);

    void writeDataAsCppLiteral (const MemoryBlock& data, OutputStream& out,
                                bool breakAtNewLines, bool allowStringBreaks);

    void createStringMatcher (OutputStream& out, const String& utf8PointerVariable,
                              const StringArray& strings, const StringArray& codeToExecute, int indentLevel);

    String getLeadingWhitespace (String line);
    int getBraceCount (String::CharPointerType line);
    bool getIndentForCurrentBlock (CodeDocument::Position pos, const String& tab,
                                   String& blockIndent, String& lastLineIndent);
}
