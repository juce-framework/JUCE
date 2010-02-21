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
static const char* newLine = "\n";

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

        if (content.startsWith (T("//")))
            content = content.fromFirstOccurrenceOf (T("\n"), false, false);
        else if (content.startsWith (T("/*")))
            content = content.fromFirstOccurrenceOf (T("*/"), false, false);
        else
            break;
    }

    StringArray lines;
    lines.addLines (content);
    lines.trim();
    lines.removeEmptyStrings();

    const String l1 (lines[0].removeCharacters (T(" \t")).trim());
    const String l2 (lines[1].removeCharacters (T(" \t")).trim());

    if (l1.replace (T("#ifndef"), T("#define")) == l2)
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
                       const bool isOuterFile)
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

        if ((! isOuterFile) && trimmed.startsWith (T("//================================================================")))
            line = String::empty;

        if (trimmed.startsWithChar (T('#'))
             && trimmed.removeCharacters (T(" \t")).startsWithIgnoreCase (T("#include\"")))
        {
            const int endOfInclude = line.indexOfChar (line.indexOfChar (T('\"')) + 1, T('\"')) + 1;
            const String lineUpToEndOfInclude (line.substring (0, endOfInclude));
            const String lineAfterInclude (line.substring (endOfInclude));

            const String filename (line.fromFirstOccurrenceOf (T("\""), false, false)
                                       .upToLastOccurrenceOf (T("\""), false, false));
            const File targetFile (file.getSiblingFile (filename));

            if (targetFile.exists() && targetFile.isAChildOf (rootFolder))
            {
                if (matchesWildcard (filename.replaceCharacter (T('\\'), T('/')), wildcards)
                     && ! includesToIgnore.contains (targetFile.getFileName()))
                {
                    if (line.containsIgnoreCase (T("FORCE_AMALGAMATOR_INCLUDE"))
                        || ! alreadyIncludedFiles.contains (targetFile.getFullPathName()))
                    {
                        if (! canFileBeReincluded (targetFile))
                            alreadyIncludedFiles.add (targetFile.getFullPathName());

                        dest << newLine << "/*** Start of inlined file: " << targetFile.getFileName() << " ***/" << newLine;

                        if (! parseFile (rootFolder, newTargetFile,
                                         dest, targetFile, alreadyIncludedFiles, includesToIgnore,
                                         wildcards, false))
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
                    line = lineUpToEndOfInclude.upToFirstOccurrenceOf (T("\""), true, false)
                            + targetFile.getRelativePathFrom (newTargetFile.getParentDirectory())
                                        .replaceCharacter (T('\\'), T('/'))
                            + T("\"")
                            + lineAfterInclude;
                }
            }
        }

        if (trimmed.startsWith (T("/*")) && (i > 10 || ! isOuterFile))
        {
            int originalI = i;
            String originalLine = line;

            for (;;)
            {
                int end = line.indexOf (T("*/"));

                if (end >= 0)
                {
                    line = line.substring (end + 2);

                    // If our comment appeared just before an assertion, leave it in, as it
                    // might be useful..
                    if (lines [i + 1].contains (T("assert"))
                         || lines [i + 2].contains (T("assert")))
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
                line = String::repeatedString (T("\t"), numTabs) + line.substring (numTabs * tabSize);
            }

            if (! line.containsChar (T('"')))
            {
                // turn large areas of spaces into tabs - this will mess up alignment a bit, but
                // it's only the amalgamated file, so doesn't matter...
                line = line.replace (T("        "), T("\t"), false);
                line = line.replace (T("    "), T("\t"), false);
            }
        }

        if (line.isNotEmpty() || ! lastLineWasBlank)
            dest << line << newLine;

        lastLineWasBlank = line.isEmpty();
    }

    return true;
}

//==============================================================================
static bool munge (const File& templateFile, const File& targetFile, const String& wildcard,
                   StringArray& alreadyIncludedFiles, const StringArray& includesToIgnore)
{
    if (! templateFile.existsAsFile())
    {
        std::cout << " The template file doesn't exist!\n\n";
        return false;
    }

    StringArray wildcards;
    wildcards.addTokens (wildcard, T(";,"), T("'\""));
    wildcards.trim();
    wildcards.removeEmptyStrings();

    std::cout << "Building: " << targetFile.getFullPathName() << "...\n";

    TemporaryFile temp (targetFile);
    ScopedPointer <FileOutputStream> out (temp.getFile().createOutputStream (1024 * 128));

    if (out == 0)
    {
        std::cout << "\n!! ERROR - couldn't write to the target file: "
                  << temp.getFile().getFullPathName() << "\n\n";
        return false;
    }

    if (! parseFile (targetFile.getParentDirectory(),
                     targetFile,
                     *out, templateFile,
                     alreadyIncludedFiles,
                     includesToIgnore,
                     wildcards,
                     true))
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

    return true;
}

static void findAllFilesIncludedIn (const File& hppTemplate, StringArray& alreadyIncludedFiles)
{
    StringArray lines;
    lines.addLines (hppTemplate.loadFileAsString());

    for (int i = 0; i < lines.size(); ++i)
    {
        String line (lines[i]);

        if (line.removeCharacters (T(" \t")).startsWithIgnoreCase (T("#include\"")))
        {
            const String filename (line.fromFirstOccurrenceOf (T("\""), false, false)
                                       .upToLastOccurrenceOf (T("\""), false, false));
            const File targetFile (hppTemplate.getSiblingFile (filename));

            if (! alreadyIncludedFiles.contains (targetFile.getFullPathName()))
            {
                alreadyIncludedFiles.add (targetFile.getFullPathName());

                if (targetFile.getFileName().containsIgnoreCase (T("juce_")) && targetFile.exists())
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

    const File hppTemplate (juceFolder.getChildFile (T("amalgamation/juce_amalgamated_template.h")));
    const File cppTemplate (juceFolder.getChildFile (T("amalgamation/juce_amalgamated_template.cpp")));

    const File hppTarget (juceFolder.getChildFile (T("juce_amalgamated.h")));
    const File cppTarget (juceFolder.getChildFile (T("juce_amalgamated.cpp")));

    StringArray alreadyIncludedFiles, includesToIgnore;

    if (! munge (hppTemplate, hppTarget, "*.h", alreadyIncludedFiles, includesToIgnore))
    {
        return;
    }

    findAllFilesIncludedIn (hppTemplate, alreadyIncludedFiles);
    includesToIgnore.add (hppTarget.getFileName());

    munge (cppTemplate, cppTarget, "*.cpp;*.c;*.h;*.mm;*.m", alreadyIncludedFiles, includesToIgnore);
}

//==============================================================================
int main (int argc, char* argv[])
{
    // This object makes sure that Juce is initialised and shut down correctly
    // for the scope of this function call. Make sure this declaration is the
    // first statement of this function.
    const ScopedJuceInitialiser_NonGUI juceSystemInitialiser;

    std::cout << "\n*** The C++ Amalgamator! Written for Juce - www.rawmaterialsoftware.com\n";

    if (argc == 4)
    {
        const File templateFile (File::getCurrentWorkingDirectory().getChildFile (String (argv[1]).unquoted()));
        const File targetFile (File::getCurrentWorkingDirectory().getChildFile (String (argv[2]).unquoted()));
        const String wildcard (String (argv[3]).unquoted());
        StringArray alreadyIncludedFiles, includesToIgnore;

        munge (templateFile, targetFile, wildcard, alreadyIncludedFiles, includesToIgnore);
    }
    else if (argc == 2)
    {
        const File juceFolder (File::getCurrentWorkingDirectory().getChildFile (String (argv[1]).unquoted()));
        mungeJuce (juceFolder);
    }
    else
    {
        std::cout << " Usage: amalgamator TemplateFile TargetFile \"FileToReplaceWildcard\"\n\n";
                     " amalgamator will run through a C++ file and replace any\n"
                     " #include statements with the contents of the file they refer to.\n"
                     " It'll only do this for files that are within the same parent\n"
                     " directory as the target file, and will ignore include statements\n"
                     " that use '<>' instead of quotes. It'll also only include a file once,\n"
                     " ignoring any repeated instances of it.\n\n"
                     " The wildcard lets you specify what kind of files will be replaced, so\n"
                     " \"*.cpp;*.h\" would replace only includes that reference a .cpp or .h file.\n\n"
                     " Or: just run 'amalgamator YourJuceDirectory' to rebuild the juce files.";
    }

    std::cout << "\n";
    return 0;
}
