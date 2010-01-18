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

#include "juce_AppConfig.h"
#include "../../juce_amalgamated.h"


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

//==============================================================================
static bool parseFile (const File& rootFolder,
                       const File& newTargetFile,
                       StringArray& dest,
                       const File& file,
                       StringArray& alreadyIncludedFiles,
                       const StringArray& includesToIgnore,
                       const StringArray& wildcards,
                       const bool isOuterFile,
                       const bool stripUnnecessaryStuff)
{
    if (! file.exists())
    {
        std::cout << "!! ERROR - file doesn't exist!";
        return false;
    }

    String content (file.loadFileAsString());

    if (stripUnnecessaryStuff && ! isOuterFile)
    {
        if (content.startsWith (T("/*")))
            content = content.fromFirstOccurrenceOf (T("*/"), false, false).trimStart();

        content = content.replace (T("\r\n\r\n\r\n"), T("\r\n\r\n"));
    }

    StringArray lines;
    lines.addLines (content);
    while (lines.size() > 0 && lines[0].trim().isEmpty())
        lines.remove (0);

    for (int i = 0; i < lines.size(); ++i)
    {
        String line (lines[i]);

        if ((! isOuterFile) && line.contains (T("//================================================================")))
            line = String::empty;

        if (line.trimStart().startsWithChar (T('#'))
             && line.removeCharacters (T(" \t")).startsWithIgnoreCase (T("#include\"")))
        {
            const int endOfInclude = line.indexOfChar (line.indexOfChar (T('\"')) + 1, T('\"')) + 1;
            const String lineUpToEndOfInclude (line.substring (0, endOfInclude));
            const String lineAfterInclude (line.substring (endOfInclude));

            const String filename (line.fromFirstOccurrenceOf (T("\""), false, false)
                                       .upToLastOccurrenceOf (T("\""), false, false));
            const File targetFile (file.getSiblingFile (filename));

            if (targetFile.exists()
                 && targetFile.isAChildOf (rootFolder))
            {
                if (matchesWildcard (filename.replaceCharacter (T('\\'), T('/')), wildcards)
                     && ! includesToIgnore.contains (targetFile.getFileName()))
                {
                    if (line.containsIgnoreCase (T("FORCE_AMALGAMATOR_INCLUDE"))
                        || ! alreadyIncludedFiles.contains (targetFile.getFullPathName()))
                    {
                        if (! canFileBeReincluded (targetFile))
                            alreadyIncludedFiles.add (targetFile.getFullPathName());

                        dest.add (String::empty);
                        dest.add (T("/********* Start of inlined file: ")
                                    + targetFile.getFileName()
                                    + T(" *********/"));

                        if (! parseFile (rootFolder, newTargetFile,
                                         dest, targetFile, alreadyIncludedFiles, includesToIgnore,
                                         wildcards, false, stripUnnecessaryStuff))
                        {
                            return false;
                        }

                        dest.add (T("/********* End of inlined file: ")
                                    + targetFile.getFileName()
                                    + T(" *********/"));
                        dest.add (String::empty);

                        line = lineAfterInclude;
                    }
                    else
                    {
                        if (stripUnnecessaryStuff)
                            line = String::empty;
                        else
                            line = T("/* ") + lineUpToEndOfInclude + T(" */") + lineAfterInclude;
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

        if (line.trimStart().startsWith (T("/*")))
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

        dest.add (line);
    }

    return true;
}

//==============================================================================
static bool munge (const File& templateFile, const File& targetFile, const String& wildcard,
                   const bool stripUnnecessaryStuff, StringArray& alreadyIncludedFiles,
                   const StringArray& includesToIgnore)
{
    if (! templateFile.existsAsFile())
    {
        std::cout << " The template file doesn't exist!\n\n";
        return false;
    }

    StringArray lines, wildcards;
    wildcards.addTokens (wildcard, T(";,"), T("'\""));
    wildcards.trim();
    wildcards.removeEmptyStrings();

    if (! parseFile (targetFile.getParentDirectory(),
                     targetFile,
                     lines, templateFile,
                     alreadyIncludedFiles,
                     includesToIgnore,
                     wildcards,
                     true, stripUnnecessaryStuff))
    {
        return false;
    }

    std::cout << "Building: " << (const char*) targetFile.getFullPathName() << "...\n";

    for (int i = 0; i < lines.size() - 2; ++i)
    {
        if (lines[i].isEmpty() && lines[i + 1].isEmpty())
        {
            lines.remove (i + 1);
            --i;
        }
    }

    MemoryBlock newData, oldData;
    const String newText (lines.joinIntoString (T("\n")) + T("\n"));
    newData.append ((const char*) newText, (int) strlen ((const char*) newText));
    targetFile.loadFileAsData (oldData);

    if (oldData == newData)
    {
        std::cout << "(No need to write - new file is identical)\n";
        return true;
    }

    if (! targetFile.replaceWithData (newData.getData(), newData.getSize()))
    {
        std::cout << "\n!! ERROR - couldn't write to the target file: "
                  << (const char*) targetFile.getFullPathName() << "\n\n";
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

    const File hppTemplate (juceFolder.getChildFile (T("src/juce_amalgamated_template.h")));
    const File cppTemplate (juceFolder.getChildFile (T("src/juce_amalgamated_template.cpp")));

    const File hppTarget (juceFolder.getChildFile (T("juce_amalgamated.h")));
    const File cppTarget (juceFolder.getChildFile (T("juce_amalgamated.cpp")));

    StringArray alreadyIncludedFiles, includesToIgnore;

    if (! munge (hppTemplate, hppTarget,
                 "*.h", true, alreadyIncludedFiles, includesToIgnore))
    {
        return;
    }

    findAllFilesIncludedIn (hppTemplate, alreadyIncludedFiles);
    includesToIgnore.add (hppTarget.getFileName());

    munge (cppTemplate, cppTarget,
           "*.cpp;*.c;*.h;*.mm;*.m", true, alreadyIncludedFiles,
           includesToIgnore);
}

//==============================================================================
int main (int argc, char* argv[])
{
    // If you're running a command-line app, you need to initialise juce manually
    // before calling any Juce functionality..
    initialiseJuce_NonGUI();

    std::cout << "\n*** The C++ Amalgamator! Written for Juce - www.rawmaterialsoftware.com\n";

    if (argc == 4)
    {
        const File templateFile (File::getCurrentWorkingDirectory().getChildFile (String (argv[1]).unquoted()));
        const File targetFile (File::getCurrentWorkingDirectory().getChildFile (String (argv[2]).unquoted()));
        const String wildcard (String (argv[3]).unquoted());
        StringArray alreadyIncludedFiles, includesToIgnore;

        munge (templateFile, targetFile, wildcard, false, alreadyIncludedFiles, includesToIgnore);
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
