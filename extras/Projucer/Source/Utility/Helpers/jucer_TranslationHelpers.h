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
struct TranslationHelpers
{
    static void addString (StringArray& strings, const String& s)
    {
        if (s.isNotEmpty() && ! strings.contains (s))
            strings.add (s);
    }

    static void scanFileForTranslations (StringArray& strings, const File& file)
    {
        auto content = file.loadFileAsString();
        auto p = content.getCharPointer();

        for (;;)
        {
            p = CharacterFunctions::find (p, CharPointer_ASCII ("TRANS"));

            if (p.isEmpty())
                break;

            p += 5;
            p = p.findEndOfWhitespace();

            if (*p == '(')
            {
                ++p;
                MemoryOutputStream text;
                parseStringLiteral (p, text);

                addString (strings, text.toString());
            }
        }
    }

    static void parseStringLiteral (String::CharPointerType& p, MemoryOutputStream& out) noexcept
    {
        p = p.findEndOfWhitespace();

        if (p.getAndAdvance() == '"')
        {
            auto start = p;

            for (;;)
            {
                auto c = *p;

                if (c == '"')
                {
                    out << String (start, p);
                    ++p;
                    parseStringLiteral (p, out);
                    return;
                }

                if (c == 0)
                    break;

                if (c == '\\')
                {
                    out << String (start, p);
                    ++p;
                    out << String::charToString (readEscapedChar (p));
                    start = p + 1;
                }

                ++p;
            }
        }
    }

    static juce_wchar readEscapedChar (String::CharPointerType& p)
    {
        auto c = *p;

        switch (c)
        {
            case '"':
            case '\\':
            case '/':  break;

            case 'b':  c = '\b'; break;
            case 'f':  c = '\f'; break;
            case 'n':  c = '\n'; break;
            case 'r':  c = '\r'; break;
            case 't':  c = '\t'; break;

            case 'x':
                ++p;
                c = 0;

                for (int i = 4; --i >= 0;)
                {
                    const int digitValue = CharacterFunctions::getHexDigitValue (*p);
                    if (digitValue < 0)
                        break;

                    ++p;
                    c = (juce_wchar) ((c << 4) + digitValue);
                }

                break;

            case '0': case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9':
                c = 0;

                for (int i = 4; --i >= 0;)
                {
                    const int digitValue = *p - '0';
                    if (digitValue < 0 || digitValue > 7)
                        break;

                    ++p;
                    c = (juce_wchar) ((c << 3) + digitValue);
                }

                break;

            default:
                break;
        }

        return c;
    }

    static void scanFilesForTranslations (StringArray& strings, const Project::Item& p)
    {
        if (p.isFile())
        {
            const File file (p.getFile());

            if (file.hasFileExtension (sourceOrHeaderFileExtensions))
                scanFileForTranslations (strings, file);
        }

        for (int i = 0; i < p.getNumChildren(); ++i)
            scanFilesForTranslations (strings, p.getChild (i));
    }

    static void scanFolderForTranslations (StringArray& strings, const File& root)
    {
        for (DirectoryIterator i (root, true); i.next();)
        {
            const auto file (i.getFile());

            if (file.hasFileExtension (sourceOrHeaderFileExtensions))
                scanFileForTranslations(strings, file);
         }
    }

    static void scanProject (StringArray& strings, Project& project)
    {
        scanFilesForTranslations (strings, project.getMainGroup());

        OwnedArray<LibraryModule> modules;
        project.getEnabledModules().createRequiredModules (modules);

        for (int j = 0; j < modules.size(); ++j)
        {
            const File localFolder (modules.getUnchecked(j)->getFolder());

            Array<File> files;
            modules.getUnchecked(j)->findBrowseableFiles (localFolder, files);

            for (int i = 0; i < files.size(); ++i)
                scanFileForTranslations (strings, files.getReference(i));
        }
    }

    static const char* getMungingSeparator()  { return "JCTRIDX"; }

    static StringArray breakApart (const String& munged)
    {
        StringArray lines, result;
        lines.addLines (munged);

        String currentItem;

        for (int i = 0; i < lines.size(); ++i)
        {
            if (lines[i].contains (getMungingSeparator()))
            {
                if (currentItem.isNotEmpty())
                    result.add (currentItem);

                currentItem = String();
            }
            else
            {
                if (currentItem.isNotEmpty())
                    currentItem << newLine;

                currentItem << lines[i];
            }
        }

        if (currentItem.isNotEmpty())
            result.add (currentItem);

        return result;
    }

    static StringArray withTrimmedEnds (StringArray array)
    {
        for (auto& s : array)
            s = s.trimEnd().removeCharacters ("\r\n");

        return array;
    }

    static String escapeString (const String& s)
    {
        return s.replace ("\"", "\\\"")
                .replace ("\'", "\\\'")
                .replace ("\t", "\\t")
                .replace ("\r", "\\r")
                .replace ("\n", "\\n");
    }

    static String getPreTranslationText (Project& project)
    {
        StringArray strings;
        scanProject (strings, project);
        return mungeStrings (strings);
    }

    static String getPreTranslationText (const LocalisedStrings& strings)
    {
        return mungeStrings (strings.getMappings().getAllKeys());
    }

    static String mungeStrings (const StringArray& strings)
    {
        MemoryOutputStream s;

        for (int i = 0; i < strings.size(); ++i)
        {
            s << getMungingSeparator() << i << "." << newLine << strings[i];

            if (i < strings.size() - 1)
                s << newLine;
        }

        return s.toString();
    }

    static String createLine (const String& preString, const String& postString)
    {
        return "\"" + escapeString (preString)
                + "\" = \""
                + escapeString (postString) + "\"";
    }

    static String createFinishedTranslationFile (StringArray preStrings,
                                                 StringArray postStrings,
                                                 const LocalisedStrings& original)
    {
        const StringPairArray& originalStrings (original.getMappings());

        StringArray lines;

        if (originalStrings.size() > 0)
        {
            lines.add ("language: " + original.getLanguageName());
            lines.add ("countries: " + original.getCountryCodes().joinIntoString (" "));
            lines.add (String());

            const StringArray& originalKeys (originalStrings.getAllKeys());
            const StringArray& originalValues (originalStrings.getAllValues());
            int numRemoved = 0;

            for (int i = preStrings.size(); --i >= 0;)
            {
                if (originalKeys.contains (preStrings[i]))
                {
                    preStrings.remove (i);
                    postStrings.remove (i);
                    ++numRemoved;
                }
            }

            for (int i = 0; i < originalStrings.size(); ++i)
                lines.add (createLine (originalKeys[i], originalValues[i]));
        }
        else
        {
            lines.add ("language: [enter full name of the language here!]");
            lines.add ("countries: [enter list of 2-character country codes here!]");
            lines.add (String());
        }

        for (int i = 0; i < preStrings.size(); ++i)
            lines.add (createLine (preStrings[i], postStrings[i]));

        return lines.joinIntoString (newLine);
    }
};
