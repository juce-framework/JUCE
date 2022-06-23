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

namespace juce
{
namespace build_tools
{
    void overwriteFileIfDifferentOrThrow (const File& file, const MemoryOutputStream& newData)
    {
        if (! overwriteFileWithNewDataIfDifferent (file, newData))
            throw SaveError (file);
    }

    void overwriteFileIfDifferentOrThrow (const File& file, const String& newData)
    {
        if (! overwriteFileWithNewDataIfDifferent (file, newData))
            throw SaveError (file);
    }

    String replacePreprocessorDefs (const StringPairArray& definitions, String sourceString)
    {
        for (int i = 0; i < definitions.size(); ++i)
        {
            const String key (definitions.getAllKeys()[i]);
            const String value (definitions.getAllValues()[i]);

            sourceString = sourceString.replace ("${" + key + "}", value);
        }

        return sourceString;
    }

    String getXcodePackageType (ProjectType::Target::Type type)
    {
        using Type = ProjectType::Target::Type;

        switch (type)
        {
            case Type::GUIApp:
            case Type::StandalonePlugIn:
                return "APPL";

            case Type::VSTPlugIn:
            case Type::VST3PlugIn:
            case Type::AudioUnitPlugIn:
            case Type::UnityPlugIn:
                return "BNDL";

            case Type::AudioUnitv3PlugIn:
                return "XPC!";

            case Type::AAXPlugIn:
                return "TDMw";

            case Type::ConsoleApp:
            case Type::StaticLibrary:
            case Type::DynamicLibrary:
            case Type::LV2PlugIn:
            case Type::LV2TurtleProgram:
            case Type::SharedCodeTarget:
            case Type::AggregateTarget:
            case Type::unspecified:
            default:
                return {};
        }
    }

    String getXcodeBundleSignature (ProjectType::Target::Type type)
    {
        using Type = ProjectType::Target::Type;

        switch (type)
        {
            case Type::GUIApp:
            case Type::VSTPlugIn:
            case Type::VST3PlugIn:
            case Type::AudioUnitPlugIn:
            case Type::StandalonePlugIn:
            case Type::AudioUnitv3PlugIn:
            case Type::UnityPlugIn:
                return "????";

            case Type::AAXPlugIn:
                return "PTul";

            case Type::ConsoleApp:
            case Type::StaticLibrary:
            case Type::DynamicLibrary:
            case Type::LV2PlugIn:
            case Type::LV2TurtleProgram:
            case Type::SharedCodeTarget:
            case Type::AggregateTarget:
            case Type::unspecified:
            default:
                return {};
        }
    }

    static unsigned int calculateHash (const String& s, const unsigned int hashMultiplier)
    {
        auto t = s.toUTF8();
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
            SortedSet<unsigned int> hashes;
            bool collision = false;

            for (int i = strings.size(); --i >= 0;)
            {
                auto hash = calculateHash (strings[i], v);

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

    String makeValidIdentifier (String s, bool makeCamelCase, bool removeColons, bool allowTemplates, bool allowAsterisks)
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

        auto n = words[0];

        if (makeCamelCase)
            n = n.toLowerCase();

        for (int i = 1; i < words.size(); ++i)
        {
            if (makeCamelCase && words[i].length() > 1)
                n << words[i].substring (0, 1).toUpperCase()
                  << words[i].substring (1).toLowerCase();
            else
                n << words[i];
        }

        if (CharacterFunctions::isDigit (n[0]))
            n = "_" + n;

        if (isReservedKeyword (n))
            n << '_';

        return n;
    }

    String makeBinaryDataIdentifierName (const File& file)
    {
        return makeValidIdentifier (file.getFileName()
                                            .replaceCharacters (" .", "__")
                                            .retainCharacters ("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_0123456789"),
                                    false, true, false);
    }

    void writeDataAsCppLiteral (const MemoryBlock& mb, OutputStream& out,
                                bool breakAtNewLines, bool allowStringBreaks)
    {
        const int maxCharsOnLine = 250;

        auto data = (const unsigned char*) mb.getData();
        int charsOnLine = 0;

        bool canUseStringLiteral = mb.getSize() < 32768; // MS compilers can't handle big string literals..

        if (canUseStringLiteral)
        {
            unsigned int numEscaped = 0;

            for (size_t i = 0; i < mb.getSize(); ++i)
            {
                auto num = (unsigned int) data[i];

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
                auto num = (int) (unsigned int) data[i];
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
            writeEscapeChars (out, (const char*) data, (int) mb.getSize(),
                              maxCharsOnLine, breakAtNewLines, false, allowStringBreaks);
            out << "\";";
        }
    }

    void createStringMatcher (OutputStream& out, const String& utf8PointerVariable,
                              const StringArray& strings, const StringArray& codeToExecute, const int indentLevel)
    {
        jassert (strings.size() == codeToExecute.size());
        auto indent = String::repeatedString (" ", indentLevel);
        auto hashMultiplier = findBestHashMultiplier (strings);

        out << indent << "unsigned int hash = 0;" << newLine
            << newLine
            << indent << "if (" << utf8PointerVariable << " != nullptr)" << newLine
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

    String unixStylePath (const String& path)       { return path.replaceCharacter ('\\', '/'); }
    String windowsStylePath (const String& path)    { return path.replaceCharacter ('/', '\\'); }

    String currentOSStylePath (const String& path)
    {
       #if JUCE_WINDOWS
        return windowsStylePath (path);
       #else
        return unixStylePath (path);
       #endif
    }

    bool isAbsolutePath (const String& path)
    {
        return File::isAbsolutePath (path)
                || path.startsWithChar ('/') // (needed because File::isAbsolutePath will ignore forward-slashes on Windows)
                || path.startsWithChar ('$')
                || path.startsWithChar ('~')
                || (CharacterFunctions::isLetter (path[0]) && path[1] == ':')
                || path.startsWithIgnoreCase ("smb:");
    }

    String getRelativePathFrom (const File& file, const File& sourceFolder)
    {
       #if ! JUCE_WINDOWS
        // On a non-windows machine, we can't know if a drive-letter path may be relative or not.
        if (CharacterFunctions::isLetter (file.getFullPathName()[0]) && file.getFullPathName()[1] == ':')
            return file.getFullPathName();
       #endif

        return file.getRelativePathFrom (sourceFolder);
    }

    void writeStreamToFile (const File& file, const std::function<void (MemoryOutputStream&)>& writer)
    {
        MemoryOutputStream mo;
        writer (mo);
        overwriteFileIfDifferentOrThrow (file, mo);
    }
}
}
