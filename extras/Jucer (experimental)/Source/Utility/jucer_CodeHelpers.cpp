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

#include "../jucer_Headers.h"
#include "jucer_CodeHelpers.h"


//==============================================================================
namespace CodeHelpers
{
    const String indent (const String& code, const int numSpaces, bool indentFirstLine)
    {
        if (numSpaces == 0)
            return code;

        const String space (String::repeatedString (" ", numSpaces));

        StringArray lines;
        lines.addLines (code);

        for (int i = (indentFirstLine ? 0 : 1); i < lines.size(); ++i)
        {
            String s (lines[i].trimEnd());
            if (s.isNotEmpty())
                s = space + s;

            lines.set (i, s);
        }

        return lines.joinIntoString (newLine);
    }

    const String makeValidIdentifier (String s, bool capitalise, bool removeColons, bool allowTemplates)
    {
        if (s.isEmpty())
            return "unknown";

        if (removeColons)
            s = s.replaceCharacters (".,;:/@", "______");
        else
            s = s.replaceCharacters (".,;/@", "_____");

        int i;
        for (i = s.length(); --i > 0;)
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

        StringArray words;
        words.addTokens (s.retainCharacters (allowedChars), false);
        words.trim();

        String n (words[0]);

        if (capitalise)
            n = n.toLowerCase();

        for (i = 1; i < words.size(); ++i)
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

    template <class CharType>
    static void writeEscapeChars (OutputStream& out, const CharType* data, const int numChars,
                                  const int maxCharsOnLine, const bool breakAtNewLines, const bool replaceSingleQuotes)
    {
        int charsOnLine = 0;
        bool lastWasHexEscapeCode = false;

        for (int i = 0; i < numChars || numChars < 0; ++i)
        {
            const CharType c = data[i];
            bool startNewLine = false;

            switch (c)
            {
                case '\t':  out << "\\t";  lastWasHexEscapeCode = false; break;
                case '\r':  out << "\\r";  lastWasHexEscapeCode = false; break;
                case '\n':  out << "\\n";  lastWasHexEscapeCode = false; startNewLine = breakAtNewLines; break;
                case '\\':  out << "\\\\"; lastWasHexEscapeCode = false; break;
                case '\"':  out << "\\\""; lastWasHexEscapeCode = false; break;

                case 0:
                    if (numChars < 0)
                        return;

                    out << "\\0";
                    lastWasHexEscapeCode = true;
                    break;

                case '\'':
                    if (replaceSingleQuotes)
                    {
                        out << "\\\'";
                        lastWasHexEscapeCode = false;
                        break;
                    }

                    // deliberate fall-through...

                default:
                    if (c >= 32 && c < 127 && ! (lastWasHexEscapeCode  // (have to avoid following a hex escape sequence with a valid hex digit)
                                                  && ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))))
                    {
                        out << (char) c;
                        lastWasHexEscapeCode = false;
                    }
                    else
                    {
                        out << (c < 16 ? "\\x0" : "\\x") << String::toHexString ((int) (unsigned int) c);
                        lastWasHexEscapeCode = true;
                    }

                    break;
            }

            if ((startNewLine || (maxCharsOnLine > 0 && ++charsOnLine >= maxCharsOnLine))
                 && (numChars < 0 || i < numChars - 1))
            {
                charsOnLine = 0;
                out << "\"" << newLine << "\"";
            }
        }
    }

    const String addEscapeChars (const String& s)
    {
        MemoryOutputStream out;
        writeEscapeChars (out, (const juce_wchar*) s, -1, -1, false, true);
        return out.toUTF8();
    }

    const String createIncludeStatement (const File& includeFile, const File& targetFile)
    {
        return "#include \"" + FileHelpers::unixStylePath (includeFile.getRelativePathFrom (targetFile.getParentDirectory())) + "\"";
    }

    const String makeHeaderGuardName (const File& file)
    {
        return "__" + file.getFileName().toUpperCase()
                                        .replaceCharacters (" .", "__")
                                        .retainCharacters ("_ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789")
                + "_" + String::toHexString (file.hashCode()).toUpperCase() + "__";
    }

    const String stringLiteral (const String& text)
    {
        if (text.isEmpty())
            return "String::empty";

        return CodeHelpers::addEscapeChars (text).quoted();
    }

    const String boolLiteral (const bool b)
    {
        return b ? "true" : "false";
    }

    const String floatLiteral (float v)
    {
        String s ((double) v, 4);

        if (s.containsChar ('.'))
        {
            s = s.trimCharactersAtEnd ("0");
            if (s.endsWithChar ('.'))
                s << '0';

            s << 'f';
        }
        else
        {
            s << ".0f";
        }

        return s;
    }

    const String doubleLiteral (double v)
    {
        String s (v, 7);

        if (s.containsChar ('.'))
        {
            s = s.trimCharactersAtEnd ("0");
            if (s.endsWithChar ('.'))
                s << '0';
        }
        else
        {
            s << ".0";
        }

        return s;
    }

    const String alignFunctionCallParams (const String& call, const StringArray& parameters, const int maxLineLength)
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

    const String colourToCode (const Colour& col)
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

    const String justificationToCode (const Justification& justification)
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
            default:                                    jassertfalse; break;
        }

        return "Justification (" + String (justification.getFlags()) + ")";
    }

    const String fontToCode (const Font& font)
    {
        String s ("Font (");
        String name (font.getTypefaceName());

        if (name != Font::getDefaultSansSerifFontName())
        {
            if (name == Font::getDefaultSerifFontName())
                name = "Font::getDefaultSerifFontName()";
            else if (name == Font::getDefaultMonospacedFontName())
                name = "Font::getDefaultMonospacedFontName()";
            else
                name = stringLiteral (font.getTypefaceName());

            s << name << ", ";
        }

        s << floatLiteral (font.getHeight());

        if (font.isBold() && font.isItalic())
            s << ", Font::bold | Font::italic";
        else if (font.isBold())
            s << ", Font::bold";
        else if (font.isItalic())
            s << ", Font::italic";

        return s + ")";
    }

    const String castToFloat (const String& expression)
    {
        if (expression.containsOnly ("0123456789.f"))
        {
            String s (expression.getFloatValue());

            if (s.containsChar (T('.')))
                return s + "f";

            return s + ".0f";
        }

        return "(float) (" + expression + ")";
    }

    const String castToInt (const String& expression)
    {
        if (expression.containsOnly ("0123456789."))
            return String ((int) expression.getFloatValue());

        return "(int) (" + expression + ")";
    }

    void writeDataAsCppLiteral (const MemoryBlock& mb, OutputStream& out)
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
                    ++charsOnLine;
                if (num >= 100)
                    ++charsOnLine;

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
            writeEscapeChars (out, data, (int) mb.getSize(), maxCharsOnLine, true, false);
            out << "\";";
        }
    }

    static int calculateHash (const String& s, const int hashMultiplier)
    {
        const char* t = s.toUTF8();
        int hash = 0;
        while (*t != 0)
            hash = hashMultiplier * hash + *t++;

        return hash;
    }

    static int findBestHashMultiplier (const StringArray& strings)
    {
        int v = 31;

        for (;;)
        {
            SortedSet <int> hashes;
            bool collision = false;
            for (int i = strings.size(); --i >= 0;)
            {
                const int hash = calculateHash (strings[i], v);
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
        const int hashMultiplier = findBestHashMultiplier (strings);

        out << indent << "int hash = 0;" << newLine
            << indent << "if (" << utf8PointerVariable << " != 0)" << newLine
            << indent << "    while (*" << utf8PointerVariable << " != 0)" << newLine
            << indent << "        hash = " << hashMultiplier << " * hash + *" << utf8PointerVariable << "++;" << newLine
            << newLine
            << indent << "switch (hash)" << newLine
            << indent << "{" << newLine;

        for (int i = 0; i < strings.size(); ++i)
            out << indent << "    case 0x" << hexString8Digits (calculateHash (strings[i], hashMultiplier))
                << ":  " << codeToExecute[i] << newLine;

        out << indent << "    default: break;" << newLine
            << indent << "}" << newLine << newLine;
    }
}
