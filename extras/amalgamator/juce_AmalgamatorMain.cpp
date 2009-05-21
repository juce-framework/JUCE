/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

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
    printf ("reading: " + file.getFileName() + "\n");

    if (! file.exists())
    {
        printf ("!! ERROR - file doesn't exist!");
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
                    if (! alreadyIncludedFiles.contains (targetFile.getFullPathName()))
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

        dest.add (line.trimEnd());
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
        printf (" The template file doesn't exist!\n\n");
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

    //lines.trim();
    //lines.removeEmptyStrings();
    printf ("\nwriting: " + targetFile.getFullPathName() + "...\n\n");

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
        printf ("(No need to write - new file is identical)\n\n");
        return true;
    }

    if (! targetFile.replaceWithData (newData.getData(), newData.getSize()))
    {
        printf ("\n!! ERROR - couldn't write to the target file: " + targetFile.getFullPathName() + "\n\n");
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
        printf (" The folder supplied must be the root of your Juce directory!\n\n");
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

    printf ("\n The C++ Amalgamator! Copyright 2008 by Julian Storer - www.rawmaterialsoftware.com\n\n");

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
        printf (" Usage: amalgamator TemplateFile TargetFile \"FileToReplaceWildcard\"\n\n");
        printf (" amalgamator will run through a C++ file and replace any\n"
                " #include statements with the contents of the file they refer to.\n"
                " It'll only do this for files that are within the same parent\n"
                " directory as the target file, and will ignore include statements\n"
                " that use '<>' instead of quotes. It'll also only include a file once,\n"
                " ignoring any repeated instances of it.\n\n"
                " The wildcard lets you specify what kind of files will be replaced, so\n"
                " \"*.cpp;*.h\" would replace only includes that reference a .cpp or .h file.\n\n"
                " Or: just run 'amalgamator YourJuceDirectory' to rebuild the juce files." 
                );
    }

    return 0;
}

