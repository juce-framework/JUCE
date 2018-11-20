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

    String makeValidIdentifier (String s, bool capitalise, bool removeColons, bool allowTemplates, bool allowAsterisks)
    {
        if (s.isEmpty())
            return "unknown";

        if (removeColons)
            s = s.replaceCharacters (".,;:/@", "______");
        else
            s = s.replaceCharacters (".,;/@", "_____");

        for (int i = s.length(); --i > 0;)
            if (CharacterFunctions::isLetter (s[i])
                 && CharacterFunctions::isLetter (s[i - 1])
                 && CharacterFunctions::isUpperCase (s[i])
                 && ! CharacterFunctions::isUpperCase (s[i - 1]))
                s = s.substring (0, i) + " " + s.substring (i);

        String allowedChars ("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_ 0123456789");
        if (allowTemplates)
            allowedChars += "<>";

        if (! removeColons)
            allowedChars += ":";

        if (allowAsterisks)
            allowedChars += "*";

        StringArray words;
        words.addTokens (s.retainCharacters (allowedChars), false);
        words.trim();

        String n (words[0]);

        if (capitalise)
            n = n.toLowerCase();

        for (int i = 1; i < words.size(); ++i)
        {
            if (capitalise && words[i].length() > 1)
                n << words[i].substring (0, 1).toUpperCase()
                  << words[i].substring (1).toLowerCase();
            else
                n << words[i];
        }

        if (CharacterFunctions::isDigit (n[0]))
            n = "_" + n;

        if (CPlusPlusCodeTokeniser::isReservedKeyword (n))
            n << '_';

        return n;
    }

    String createIncludeStatement (const File& includeFile, const File& targetFile)
    {
        return createIncludeStatement (FileHelpers::unixStylePath (FileHelpers::getRelativePathFrom (includeFile, targetFile.getParentDirectory())));
    }

    String createIncludeStatement (const String& includePath)
    {
        if (includePath.startsWithChar ('<') || includePath.startsWithChar ('"'))
            return "#include " + includePath;

        return "#include \"" + includePath + "\"";
    }

    String makeBinaryDataIdentifierName (const File& file)
    {
        return makeValidIdentifier (file.getFileName()
                                        .replaceCharacters (" .", "__")
                                        .retainCharacters ("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_0123456789"),
                                    false, true, false);
    }

    String stringLiteral (const String& text, int maxLineLength)
    {
        if (text.isEmpty())
            return "String()";

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
                String& line = lines.getReference (i);

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
            result = "CharPointer_UTF8 (" + result + ")";

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
            0
        };

        for (int i = 0; i < numElementsInArray (colourNames) - 1; ++i)
            if (col == colours[i])
                return "Colours::" + String (colourNames[i]);

        return "Colour (0x" + hexString8Digits ((int) col.getARGB()) + ')';
    }

    String justificationToCode (Justification justification)
    {
        switch (justification.getFlags())
        {
            case Justification::centred:                return "Justification::centred";
            case Justification::centredLeft:            return "Justification::centredLeft";
            case Justification::centredRight:           return "Justification::centredRight";
            case Justification::centredTop:             return "Justification::centredTop";
            case Justification::centredBottom:          return "Justification::centredBottom";
            case Justification::topLeft:                return "Justification::topLeft";
            case Justification::topRight:               return "Justification::topRight";
            case Justification::bottomLeft:             return "Justification::bottomLeft";
            case Justification::bottomRight:            return "Justification::bottomRight";
            case Justification::left:                   return "Justification::left";
            case Justification::right:                  return "Justification::right";
            case Justification::horizontallyCentred:    return "Justification::horizontallyCentred";
            case Justification::top:                    return "Justification::top";
            case Justification::bottom:                 return "Justification::bottom";
            case Justification::verticallyCentred:      return "Justification::verticallyCentred";
            case Justification::horizontallyJustified:  return "Justification::horizontallyJustified";
            default:                                    break;
        }

        jassertfalse;
        return "Justification (" + String (justification.getFlags()) + ")";
    }

    void writeDataAsCppLiteral (const MemoryBlock& mb, OutputStream& out,
                                bool breakAtNewLines, bool allowStringBreaks)
    {
        const int maxCharsOnLine = 250;

        const unsigned char* data = (const unsigned char*) mb.getData();
        int charsOnLine = 0;

        bool canUseStringLiteral = mb.getSize() < 32768; // MS compilers can't handle big string literals..

        if (canUseStringLiteral)
        {
            unsigned int numEscaped = 0;

            for (size_t i = 0; i < mb.getSize(); ++i)
            {
                const unsigned int num = (unsigned int) data[i];
                if (! ((num >= 32 && num < 127) || num == '\t' || num == '\r' || num == '\n'))
                {
                    if (++numEscaped > mb.getSize() / 4)
                    {
                        canUseStringLiteral = false;
                        break;
                    }
                }
            }
        }

        if (! canUseStringLiteral)
        {
            out << "{ ";

            for (size_t i = 0; i < mb.getSize(); ++i)
            {
                const int num = (int) (unsigned int) data[i];
                out << num << ',';

                charsOnLine += 2;

                if (num >= 10)
                {
                    ++charsOnLine;

                    if (num >= 100)
                        ++charsOnLine;
                }

                if (charsOnLine >= maxCharsOnLine)
                {
                    charsOnLine = 0;
                    out << newLine;
                }
            }

            out << "0,0 };";
        }
        else
        {
            out << "\"";
            CppTokeniserFunctions::writeEscapeChars (out, (const char*) data, (int) mb.getSize(),
                                                     maxCharsOnLine, breakAtNewLines, false, allowStringBreaks);
            out << "\";";
        }
    }

    //==============================================================================
    static unsigned int calculateHash (const String& s, const unsigned int hashMultiplier)
    {
        const char* t = s.toUTF8();
        unsigned int hash = 0;
        while (*t != 0)
            hash = hashMultiplier * hash + (unsigned int) *t++;

        return hash;
    }

    static unsigned int findBestHashMultiplier (const StringArray& strings)
    {
        unsigned int v = 31;

        for (;;)
        {
            SortedSet <unsigned int> hashes;
            bool collision = false;
            for (int i = strings.size(); --i >= 0;)
            {
                const unsigned int hash = calculateHash (strings[i], v);
                if (hashes.contains (hash))
                {
                    collision = true;
                    break;
                }

                hashes.add (hash);
            }

            if (! collision)
                break;

            v += 2;
        }

        return v;
    }

    void createStringMatcher (OutputStream& out, const String& utf8PointerVariable,
                              const StringArray& strings, const StringArray& codeToExecute, const int indentLevel)
    {
        jassert (strings.size() == codeToExecute.size());
        const String indent (String::repeatedString (" ", indentLevel));
        const unsigned int hashMultiplier = findBestHashMultiplier (strings);

        out << indent << "unsigned int hash = 0;" << newLine
            << indent << "if (" << utf8PointerVariable << " != 0)" << newLine
            << indent << "    while (*" << utf8PointerVariable << " != 0)" << newLine
            << indent << "        hash = " << (int) hashMultiplier << " * hash + (unsigned int) *" << utf8PointerVariable << "++;" << newLine
            << newLine
            << indent << "switch (hash)" << newLine
            << indent << "{" << newLine;

        for (int i = 0; i < strings.size(); ++i)
        {
            out << indent << "    case 0x" << hexString8Digits ((int) calculateHash (strings[i], hashMultiplier))
                << ":  " << codeToExecute[i] << newLine;
        }

        out << indent << "    default: break;" << newLine
            << indent << "}" << newLine << newLine;
    }

    String getLeadingWhitespace (String line)
    {
        line = line.removeCharacters ("\r\n");
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

            const String line (pos.getLineText());
            const String trimmedLine (line.trimStart());

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
