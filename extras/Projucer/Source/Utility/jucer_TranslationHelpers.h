/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

#pragma once

struct TranslationHelpers
{
    static void addString (StringArray& strings, const String& s)
    {
        if (s.isNotEmpty() && ! strings.contains (s))
            strings.add (s);
    }

    static void scanFileForTranslations (StringArray& strings, const File& file)
    {
        const String content (file.loadFileAsString());

        String::CharPointerType p (content.getCharPointer());

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
            String::CharPointerType start (p);

            for (;;)
            {
                juce_wchar c = *p;

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
        juce_wchar c = *p;

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
        project.getModules().createRequiredModules (modules);

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

    static StringArray trimEnd (const StringArray& array)
    {
        StringArray result;
        result.ensureStorageAllocated (array.size());

        for (int i = 0; i < array.size(); ++i)
        {
            result.add (array[i].trimEnd()
                                .replace ("\n", String())
                                .replace ("\r", String()));
        }
        return result;
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
