/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

#include "../JuceLibraryCode/JuceHeader.h"


//==============================================================================
static bool matchesWildcard (const String& filename, const StringArray& wildcards)
{
    for (int i = wildcards.size(); --i >= 0;)
        if (filename.matchesWildcard (wildcards[i], true))
            return true;

    return false;
}

static bool canFileBeReincluded (const File& f)
{
    String content (f.loadFileAsString());

    for (;;)
    {
        content = content.trimStart();

        if (content.startsWith ("//"))
            content = content.fromFirstOccurrenceOf ("\n", false, false);
        else if (content.startsWith ("/*"))
            content = content.fromFirstOccurrenceOf ("*/", false, false);
        else
            break;
    }

    StringArray lines;
    lines.addLines (content);
    lines.trim();
    lines.removeEmptyStrings();

    const String l1 (lines[0].removeCharacters (" \t").trim());
    const String l2 (lines[1].removeCharacters (" \t").trim());

    if (l1.replace ("#ifndef", "#define") == l2)
        return false;

    return true;
}

static int64 calculateStreamHashCode (InputStream& in)
{
    int64 t = 0;

    const int bufferSize = 4096;
    HeapBlock <uint8> buffer;
    buffer.malloc (bufferSize);

    for (;;)
    {
        const int num = in.read (buffer, bufferSize);

        if (num <= 0)
            break;

        for (int i = 0; i < num; ++i)
            t = t * 65599 + buffer[i];
    }

    return t;
}

static int64 calculateFileHashCode (const File& file)
{
    ScopedPointer <FileInputStream> stream (file.createInputStream());
    return stream != 0 ? calculateStreamHashCode (*stream) : 0;
}


//==============================================================================
static bool parseFile (const File& rootFolder,
                       const File& newTargetFile,
                       OutputStream& dest,
                       const File& file,
                       StringArray& alreadyIncludedFiles,
                       const StringArray& includesToIgnore,
                       const StringArray& wildcards,
                       bool isOuterFile,
                       bool stripCommentBlocks)
{
    if (! file.exists())
    {
        std::cout << "!! ERROR - file doesn't exist!";
        return false;
    }

    StringArray lines;
    lines.addLines (file.loadFileAsString());

    if (lines.size() == 0)
    {
        std::cout << "!! ERROR - input file was empty: " << file.getFullPathName();
        return false;
    }

    bool lastLineWasBlank = true;

    for (int i = 0; i < lines.size(); ++i)
    {
        String line (lines[i]);
        String trimmed (line.trimStart());

        if ((! isOuterFile) && trimmed.startsWith ("//================================================================"))
            line = String::empty;

        if (trimmed.startsWithChar ('#')
             && trimmed.removeCharacters (" \t").startsWithIgnoreCase ("#include\""))
        {
            const int endOfInclude = line.indexOfChar (line.indexOfChar ('\"') + 1, '\"') + 1;
            const String lineUpToEndOfInclude (line.substring (0, endOfInclude));
            const String lineAfterInclude (line.substring (endOfInclude));

            const String filename (line.fromFirstOccurrenceOf ("\"", false, false)
                                       .upToLastOccurrenceOf ("\"", false, false));
            const File targetFile (file.getSiblingFile (filename));

            if (targetFile.exists() && targetFile.isAChildOf (rootFolder))
            {
                if (matchesWildcard (filename.replaceCharacter ('\\', '/'), wildcards)
                     && ! includesToIgnore.contains (targetFile.getFileName()))
                {
                    if (line.containsIgnoreCase ("FORCE_AMALGAMATOR_INCLUDE")
                        || ! alreadyIncludedFiles.contains (targetFile.getFullPathName()))
                    {
                        if (! canFileBeReincluded (targetFile))
                            alreadyIncludedFiles.add (targetFile.getFullPathName());

                        dest << newLine << "/*** Start of inlined file: " << targetFile.getFileName() << " ***/" << newLine;

                        if (! parseFile (rootFolder, newTargetFile,
                                         dest, targetFile, alreadyIncludedFiles, includesToIgnore,
                                         wildcards, false, stripCommentBlocks))
                        {
                            return false;
                        }

                        dest << "/*** End of inlined file: " << targetFile.getFileName() << " ***/" << newLine << newLine;

                        line = lineAfterInclude;
                    }
                    else
                    {
                        line = String::empty;
                    }
                }
                else
                {
                    line = lineUpToEndOfInclude.upToFirstOccurrenceOf ("\"", true, false)
                            + targetFile.getRelativePathFrom (newTargetFile.getParentDirectory())
                                        .replaceCharacter ('\\', '/')
                            + "\""
                            + lineAfterInclude;
                }
            }
        }

        if ((stripCommentBlocks || i == 0) && trimmed.startsWith ("/*") && (i > 10 || ! isOuterFile))
        {
            int originalI = i;
            String originalLine = line;

            for (;;)
            {
                int end = line.indexOf ("*/");

                if (end >= 0)
                {
                    line = line.substring (end + 2);

                    // If our comment appeared just before an assertion, leave it in, as it
                    // might be useful..
                    if (lines [i + 1].contains ("assert")
                         || lines [i + 2].contains ("assert"))
                    {
                        i = originalI;
                        line = originalLine;
                    }

                    break;
                }

                line = lines [++i];

                if (i >= lines.size())
                    break;
            }

            line = line.trimEnd();
            if (line.isEmpty())
                continue;
        }

        line = line.trimEnd();

        {
            // Turn initial spaces into tabs..
            int numIntialSpaces = 0;
            int len = line.length();
            while (numIntialSpaces < len && line [numIntialSpaces] == ' ')
                ++numIntialSpaces;

            if (numIntialSpaces > 0)
            {
                int tabSize = 4;
                int numTabs = numIntialSpaces / tabSize;
                line = String::repeatedString ("\t", numTabs) + line.substring (numTabs * tabSize);
            }

            if (! line.containsChar ('"'))
            {
                // turn large areas of spaces into tabs - this will mess up alignment a bit, but
                // it's only the amalgamated file, so doesn't matter...
                line = line.replace ("        ", "\t", false);
                line = line.replace ("    ", "\t", false);
            }
        }

        if (line.isNotEmpty() || ! lastLineWasBlank)
            dest << line << newLine;

        lastLineWasBlank = line.isEmpty();
    }

    return true;
}

//==============================================================================
class NullOutputStream  : public OutputStream
{
public:
    NullOutputStream() {}
    void flush() {}
    int64 getPosition() { return 0; }
    bool setPosition (int64) { return false; }
    bool write (const void*, int) { return true; }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NullOutputStream);
};

static bool munge (const File& templateFile, const File& targetFile, const String& wildcard,
                   StringArray& alreadyIncludedFiles, const StringArray& includesToIgnore,
                   bool produceOutputFile = true)
{
    if (! templateFile.existsAsFile())
    {
        std::cout << " The template file doesn't exist!\n\n";
        return false;
    }

    StringArray wildcards;
    wildcards.addTokens (wildcard, ";,", "'\"");
    wildcards.trim();
    wildcards.removeEmptyStrings();

    std::cout << "Building: " << targetFile.getFullPathName() << "...\n";

    if (produceOutputFile)
    {
        TemporaryFile temp (targetFile);
        ScopedPointer <FileOutputStream> out (temp.getFile().createOutputStream (1024 * 128));

        if (out == 0)
        {
            std::cout << "\n!! ERROR - couldn't write to the target file: "
                      << temp.getFile().getFullPathName() << "\n\n";
            return false;
        }

        out->setNewLineString ("\n");

        if (! parseFile (targetFile.getParentDirectory(),
                         targetFile,
                         *out, templateFile,
                         alreadyIncludedFiles,
                         includesToIgnore,
                         wildcards,
                         true, false))
        {
            return false;
        }

        out = 0;

        if (calculateFileHashCode (targetFile) == calculateFileHashCode (temp.getFile()))
        {
            std::cout << " -- No need to write - new file is identical\n";
            return true;
        }

        if (! temp.overwriteTargetFileWithTemporary())
        {
            std::cout << "\n!! ERROR - couldn't write to the target file: "
                      << targetFile.getFullPathName() << "\n\n";
            return false;
        }
    }
    else
    {
        NullOutputStream out;
        if (! parseFile (targetFile.getParentDirectory(),
                         targetFile,
                         out, templateFile,
                         alreadyIncludedFiles,
                         includesToIgnore,
                         wildcards,
                         true, false))
        {
            return false;
        }

    }

    return true;
}

static void findAllFilesIncludedIn (const File& hppTemplate, StringArray& alreadyIncludedFiles)
{
    StringArray lines;
    lines.addLines (hppTemplate.loadFileAsString());

    for (int i = 0; i < lines.size(); ++i)
    {
        String line (lines[i]);

        if (line.removeCharacters (" \t").startsWithIgnoreCase ("#include\""))
        {
            const String filename (line.fromFirstOccurrenceOf ("\"", false, false)
                                       .upToLastOccurrenceOf ("\"", false, false));
            const File targetFile (hppTemplate.getSiblingFile (filename));

            if (! alreadyIncludedFiles.contains (targetFile.getFullPathName()))
            {
                alreadyIncludedFiles.add (targetFile.getFullPathName());

                if (targetFile.getFileName().containsIgnoreCase ("juce_") && targetFile.exists())
                    findAllFilesIncludedIn (targetFile, alreadyIncludedFiles);
            }
        }
    }
}

//==============================================================================
static void mungeJuce (const File& juceFolder)
{
    if (! juceFolder.isDirectory())
    {
        std::cout << " The folder supplied must be the root of your Juce directory!\n\n";
        return;
    }

    const File hppTemplate (juceFolder.getChildFile ("amalgamation/juce_amalgamated_template.h"));
    const File cppTemplate (juceFolder.getChildFile ("amalgamation/juce_amalgamated_template.cpp"));

    const File hppTarget (juceFolder.getChildFile ("juce_amalgamated.h"));
    const File cppTarget (juceFolder.getChildFile ("juce_amalgamated.cpp"));

    StringArray alreadyIncludedFiles, includesToIgnore;

    if (! munge (hppTemplate, hppTarget, "*.h", alreadyIncludedFiles, includesToIgnore))
    {
        return;
    }

    findAllFilesIncludedIn (hppTemplate, alreadyIncludedFiles);
    includesToIgnore.add (hppTarget.getFileName());

    std::cout << alreadyIncludedFiles.joinIntoString (";") << "\n";
    munge (cppTemplate, cppTarget, "*.cpp;*.c;*.h;*.mm;*.m", alreadyIncludedFiles, includesToIgnore);
}

//==============================================================================
const String defaultWildcard ("*.cpp;*.c;*.h;*.mm;*.m");

int main (int argc, char* argv[])
{
    std::cout << "\n*** The C++ Amalgamator! Written for Juce - www.rawmaterialsoftware.com\n\n";

    if (argc == 5)
    {
        const File templateHeader (File::getCurrentWorkingDirectory().getChildFile (String (argv[1]).unquoted()));
        const File templateFile (File::getCurrentWorkingDirectory().getChildFile (String (argv[2]).unquoted()));
        const File targetFile (File::getCurrentWorkingDirectory().getChildFile (String (argv[3]).unquoted()));

        std::cout << "using " << templateHeader.getFileName() << "\n";

        String wildcard (String (argv[4]).unquoted());
        if (wildcard == "-d")
            wildcard = defaultWildcard;

        StringArray alreadyIncludedFiles, includesToIgnore;

        if (! munge (templateHeader, String::empty, "*.h", alreadyIncludedFiles, includesToIgnore, false))
        {
            return 1;
        }

        findAllFilesIncludedIn (templateHeader, alreadyIncludedFiles);
        includesToIgnore.add ("juce_amalgamated.h");

        std::cout << alreadyIncludedFiles.joinIntoString (";") << "\n";
        munge (templateFile, targetFile, wildcard, alreadyIncludedFiles, includesToIgnore);
    }
    else if (argc == 4)
    {
        const File templateFile (File::getCurrentWorkingDirectory().getChildFile (String (argv[1]).unquoted()));
        const File targetFile (File::getCurrentWorkingDirectory().getChildFile (String (argv[2]).unquoted()));
        StringArray alreadyIncludedFiles, includesToIgnore;

        String wildcard (String (argv[3]).unquoted());
        if (wildcard == "-d")
            wildcard = defaultWildcard;

        munge (templateFile, targetFile, wildcard, alreadyIncludedFiles, includesToIgnore);
    }
    else if (argc == 2)
    {
        const File juceFolder (File::getCurrentWorkingDirectory().getChildFile (String (argv[1]).unquoted()));
        mungeJuce (juceFolder);
    }
    else
    {
        String usage;

        usage << "Usage: " << argv[0] << " juce_directory\n";
        usage << "       " << argv[0] << " template_file target_file ( -d | {wildcard_list} )\n";
        usage << "       " << argv[0] << " template_header template_file target_file ( -d | {wildcard_list} )\n";
        usage << "\n";
        usage << "In the first form, this command will recreate the single-file amalgamation "
              << "inside the root of the juce source tree specified by juce_directory. The output files "
                 "are called juce_amalgamated.h and juce_amalgamated.cpp\n";
        usage << "\n";
        usage << "In the second form, the file specified by template_file will be processed, and "
                 "any #include statements will be replaced by inserting the contents of the file "
                 "they refer to. This replacement will only occur for files that are within the "
                 "same parent directory as the target file, and will ignore include statements in "
                 "angle brackets ('<' and '>') instead of double quotes. This replacement will only "
                 "happen once - if the same include file is found again it will be replaced with an "
                 "empty line.\n";
        usage << "\n";
        usage << "In the third form, the header file specified by template_header is processed "
                 "internally without creating an output file, and then target_file is produced from "
                 "template_file. However, #includes which appear in template_header are not replaced in "
                 "the template_file when creating the target_file. This form is used to create "
                 "amalgamations split into multiple source files sharing a common amalgamated header, "
                 "for compilers which have trouble compiling a large single amalgamation. For creating "
                 "a split amalgamation for juce, template_header is usually the path to "
                 "juce_amalgamated_template.h. The resulting amalgamation will typically use "
                 "a previously generated juce_amalgamated.h for the header, and multiple .cpp for the sources\n";
        usage << "\n";
        usage << "{wildcard_list} is a semicolon delimited list of expressions used to match #include "
                 "filenames to determine if they are a candidate for replacement. For example, a "
                 "wildcard_list of \"*.cpp;*.h\" would replace only those #include lines which referenced "
                 "files ending in .cpp or .h\n";
        usage << "\n";
        usage << "The -d option can be used in place of wildcard_list to use the default list of wildcards, "
                 "which is equal to \"*.cpp;*.c;*.h;*.mm;*.m\".\n";
        usage << "\n";

        std::cout << usage;
    }

    std::cout << "\n";
    return 0;
}
