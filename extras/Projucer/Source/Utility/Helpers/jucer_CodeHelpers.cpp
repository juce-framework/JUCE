/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#include "../../Application/jucer_Headers.h"
#include "jucer_CodeHelpers.h"

//==============================================================================
namespace CodeHelpers
{
    String indent (const String& code, const int numSpaces, bool indentFirstLine)
    {
        if (numSpaces == 0)
            return code;

        auto space = String::repeatedString (" ", numSpaces);
        auto lines = StringArray::fromLines (code);

        for (auto& line : lines)
        {
            if (! indentFirstLine)
            {
                indentFirstLine = true;
                continue;
            }

            if (line.trimEnd().isNotEmpty())
                line = space + line;
        }

        return lines.joinIntoString (newLine);
    }

    String unindent (const String& code, const int numSpaces)
    {
        if (numSpaces == 0)
            return code;

        auto space = String::repeatedString (" ", numSpaces);
        auto lines = StringArray::fromLines (code);

        for (auto& line : lines)
            if (line.startsWith (space))
                line = line.substring (numSpaces);

        return lines.joinIntoString (newLine);
    }


    String createIncludeStatement (const File& includeFile, const File& targetFile)
    {
        return createIncludeStatement (build_tools::unixStylePath (build_tools::getRelativePathFrom (includeFile, targetFile.getParentDirectory())));
    }

    String createIncludeStatement (const String& includePath)
    {
        if (includePath.startsWithChar ('<') || includePath.startsWithChar ('"'))
            return "#include " + includePath;

        return "#include \"" + includePath + "\"";
    }

    String createIncludePathIncludeStatement (const String& includedFilename)
    {
        return "#include <" + includedFilename + ">";
    }

    String stringLiteral (const String& text, int maxLineLength)
    {
        if (text.isEmpty())
            return "juce::String()";

        StringArray lines;

        {
            auto t = text.getCharPointer();
            bool finished = t.isEmpty();

            while (! finished)
            {
                for (auto startOfLine = t;;)
                {
                    switch (t.getAndAdvance())
                    {
                        case 0:     finished = true; break;
                        case '\n':  break;
                        case '\r':  if (*t == '\n') ++t; break;
                        default:    continue;
                    }

                    lines.add (String (startOfLine, t));
                    break;
                }
            }
        }

        if (maxLineLength > 0)
        {
            for (int i = 0; i < lines.size(); ++i)
            {
                auto& line = lines.getReference (i);

                if (line.length() > maxLineLength)
                {
                    const String start (line.substring (0, maxLineLength));
                    const String end (line.substring (maxLineLength));
                    line = start;
                    lines.insert (i + 1, end);
                }
            }
        }

        for (int i = 0; i < lines.size(); ++i)
            lines.getReference(i) = CppTokeniserFunctions::addEscapeChars (lines.getReference(i));

        lines.removeEmptyStrings();

        for (int i = 0; i < lines.size(); ++i)
            lines.getReference(i) = "\"" + lines.getReference(i) + "\"";

        String result (lines.joinIntoString (newLine));

        if (! CharPointer_ASCII::isValidString (text.toUTF8(), std::numeric_limits<int>::max()))
            result = "juce::CharPointer_UTF8 (" + result + ")";

        return result;
    }

    String alignFunctionCallParams (const String& call, const StringArray& parameters, const int maxLineLength)
    {
        String result, currentLine (call);

        for (int i = 0; i < parameters.size(); ++i)
        {
            if (currentLine.length() >= maxLineLength)
            {
                result += currentLine.trimEnd() + newLine;
                currentLine = String::repeatedString (" ", call.length()) + parameters[i];
            }
            else
            {
                currentLine += parameters[i];
            }

            if (i < parameters.size() - 1)
                currentLine << ", ";
        }

        return result + currentLine.trimEnd() + ")";
    }

    String floatLiteral (double value, int numDecPlaces)
    {
        String s (value, numDecPlaces);

        if (s.containsChar ('.'))
            s << 'f';
        else
            s << ".0f";

        return s;
    }

    String boolLiteral (bool value)
    {
        return value ? "true" : "false";
    }

    String colourToCode (Colour col)
    {
        const Colour colours[] =
        {
            #define COL(col)  Colours::col,
            #include "jucer_Colours.h"
            #undef COL
            Colours::transparentBlack
        };

        static const char* colourNames[] =
        {
            #define COL(col)  #col,
            #include "jucer_Colours.h"
            #undef COL
            nullptr
        };

        for (int i = 0; i < numElementsInArray (colourNames) - 1; ++i)
            if (col == colours[i])
                return "juce::Colours::" + String (colourNames[i]);

        return "juce::Colour (0x" + build_tools::hexString8Digits ((int) col.getARGB()) + ')';
    }

    String justificationToCode (Justification justification)
    {
        switch (justification.getFlags())
        {
            case Justification::centred:                return "juce::Justification::centred";
            case Justification::centredLeft:            return "juce::Justification::centredLeft";
            case Justification::centredRight:           return "juce::Justification::centredRight";
            case Justification::centredTop:             return "juce::Justification::centredTop";
            case Justification::centredBottom:          return "juce::Justification::centredBottom";
            case Justification::topLeft:                return "juce::Justification::topLeft";
            case Justification::topRight:               return "juce::Justification::topRight";
            case Justification::bottomLeft:             return "juce::Justification::bottomLeft";
            case Justification::bottomRight:            return "juce::Justification::bottomRight";
            case Justification::left:                   return "juce::Justification::left";
            case Justification::right:                  return "juce::Justification::right";
            case Justification::horizontallyCentred:    return "juce::Justification::horizontallyCentred";
            case Justification::top:                    return "juce::Justification::top";
            case Justification::bottom:                 return "juce::Justification::bottom";
            case Justification::verticallyCentred:      return "juce::Justification::verticallyCentred";
            case Justification::horizontallyJustified:  return "juce::Justification::horizontallyJustified";
            default:                                    break;
        }

        jassertfalse;
        return "Justification (" + String (justification.getFlags()) + ")";
    }

    //==============================================================================
    String getLeadingWhitespace (String line)
    {
        line = line.removeCharacters (line.endsWith ("\r\n") ? "\r\n" : "\n");
        auto endOfLeadingWS = line.getCharPointer().findEndOfWhitespace();
        return String (line.getCharPointer(), endOfLeadingWS);
    }

    int getBraceCount (String::CharPointerType line)
    {
        int braces = 0;

        for (;;)
        {
            const juce_wchar c = line.getAndAdvance();

            if (c == 0)                         break;
            else if (c == '{')                  ++braces;
            else if (c == '}')                  --braces;
            else if (c == '/')                  { if (*line == '/') break; }
            else if (c == '"' || c == '\'')     { while (! (line.isEmpty() || line.getAndAdvance() == c)) {} }
        }

        return braces;
    }

    bool getIndentForCurrentBlock (CodeDocument::Position pos, const String& tab,
                                   String& blockIndent, String& lastLineIndent)
    {
        int braceCount = 0;
        bool indentFound = false;

        while (pos.getLineNumber() > 0)
        {
            pos = pos.movedByLines (-1);

            auto line = pos.getLineText();
            auto trimmedLine = line.trimStart();

            braceCount += getBraceCount (trimmedLine.getCharPointer());

            if (braceCount > 0)
            {
                blockIndent = getLeadingWhitespace (line);
                if (! indentFound)
                    lastLineIndent = blockIndent + tab;

                return true;
            }

            if ((! indentFound) && trimmedLine.isNotEmpty())
            {
                indentFound = true;
                lastLineIndent = getLeadingWhitespace (line);
            }
        }

        return false;
    }
}
