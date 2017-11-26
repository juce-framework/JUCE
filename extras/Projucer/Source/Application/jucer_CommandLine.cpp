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

#include "jucer_Headers.h"
#include "../Project/jucer_Module.h"
#include "../Utility/Helpers/jucer_TranslationHelpers.h"

#include "jucer_CommandLine.h"


//==============================================================================
namespace
{
    static const char* getLineEnding()  { return "\r\n"; }

    struct CommandLineError
    {
        CommandLineError (const String& s) : message (s) {}

        String message;
    };

    static void hideDockIcon()
    {
       #if JUCE_MAC
        Process::setDockIconVisible (false);
       #endif
    }

    static bool matchArgument (const String& arg, const String& possible)
    {
        return arg == possible
            || arg == "-" + possible
            || arg == "--" + possible;
    }

    static void checkArgumentCount (const StringArray& args, int minNumArgs)
    {
        if (args.size() < minNumArgs)
            throw CommandLineError ("Not enough arguments!");
    }

    static File getFile (const String& filename)
    {
        return File::getCurrentWorkingDirectory().getChildFile (filename.unquoted());
    }

    static File getDirectoryCheckingForExistence (const String& filename)
    {
        File f = getFile (filename);

        if (! f.isDirectory())
            throw CommandLineError ("Could not find folder: " + f.getFullPathName());

        return f;
    }

    static File getFileCheckingForExistence (const String& filename)
    {
        File f = getFile (filename);

        if (! f.exists())
            throw CommandLineError ("Could not find file: " + f.getFullPathName());

        return f;
    }

    static Array<File> findAllSourceFiles (const File& folder)
    {
        Array<File> files;

        for (DirectoryIterator di (folder, true, "*.cpp;*.cxx;*.cc;*.c;*.h;*.hpp;*.hxx;*.hpp;*.mm;*.m;*.java;", File::findFiles); di.next();)
            if (! di.getFile().isSymbolicLink())
                files.add (di.getFile());

        return files;
    }

    static String joinLinesIntoSourceFile (StringArray& lines)
    {
        while (lines.size() > 10 && lines [lines.size() - 1].isEmpty())
            lines.remove (lines.size() - 1);

        return lines.joinIntoString (getLineEnding()) + getLineEnding();
    }

    static void replaceFile (const File& file, const String& newText, const String& message)
    {
        std::cout << message << file.getFullPathName() << std::endl;

        TemporaryFile temp (file);

        if (! temp.getFile().replaceWithText (newText, false, false))
            throw CommandLineError ("!!! ERROR Couldn't write to temp file!");

        if (! temp.overwriteTargetFileWithTemporary())
            throw CommandLineError ("!!! ERROR Couldn't write to file!");
    }

    //==============================================================================
    struct LoadedProject
    {
        LoadedProject (const String& fileToLoad)
        {
            hideDockIcon();

            File projectFile = getFileCheckingForExistence (fileToLoad);

            if (! projectFile.hasFileExtension (Project::projectFileExtension))
                throw CommandLineError (projectFile.getFullPathName() + " isn't a valid jucer project file!");

            project = new Project (projectFile);

            if (! project->loadFrom (projectFile, true))
            {
                project = nullptr;
                throw CommandLineError ("Failed to load the project file: " + projectFile.getFullPathName());
            }
        }

        void save (bool justSaveResources)
        {
            if (project != nullptr)
            {
                Result error (justSaveResources ? project->saveResourcesOnly (project->getFile())
                                                : project->saveProject (project->getFile(), true));

                project = nullptr;

                if (error.failed())
                    throw CommandLineError ("Error when saving: " + error.getErrorMessage());
            }
        }

        ScopedPointer<Project> project;
    };

    //==============================================================================
    /* Running a command-line of the form "projucer --resave foobar.jucer" will try to load
       that project and re-export all of its targets.
    */
    static void resaveProject (const StringArray& args, bool justSaveResources)
    {
        checkArgumentCount (args, 2);
        LoadedProject proj (args[1]);

        std::cout << (justSaveResources ? "Re-saving project resources: "
                                        : "Re-saving file: ")
                  << proj.project->getFile().getFullPathName() << std::endl;

        proj.save (justSaveResources);
    }

    //==============================================================================
    static void getVersion (const StringArray& args)
    {
        checkArgumentCount (args, 2);
        LoadedProject proj (args[1]);

        std::cout << proj.project->getVersionString() << std::endl;
    }

    //==============================================================================
    static void setVersion (const StringArray& args)
    {
        checkArgumentCount (args, 3);
        LoadedProject proj (args[2]);

        String version (args[1].trim());

        std::cout << "Setting project version: " << version << std::endl;

        proj.project->getVersionValue() = version;
        proj.save (false);
    }

    //==============================================================================
    static void bumpVersion (const StringArray& args)
    {
        checkArgumentCount (args, 2);
        LoadedProject proj (args[1]);

        String version = proj.project->getVersionString();

        version = version.upToLastOccurrenceOf (".", true, false)
                    + String (version.getTrailingIntValue() + 1);

        std::cout << "Bumping project version to: " << version << std::endl;

        proj.project->getVersionValue() = version;
        proj.save (false);
    }

    static void gitTag (const StringArray& args)
    {
        checkArgumentCount (args, 2);
        LoadedProject proj (args[1]);

        String version (proj.project->getVersionValue().toString());

        if (version.trim().isEmpty())
            throw CommandLineError ("Cannot read version number from project!");

        StringArray command;
        command.add ("git");
        command.add ("tag");
        command.add ("-a");
        command.add (version);
        command.add ("-m");
        command.add (version.quoted());

        std::cout << "Performing command: " << command.joinIntoString(" ") << std::endl;

        ChildProcess c;

        if (! c.start (command, 0))
            throw CommandLineError ("Cannot run git!");

        c.waitForProcessToFinish (10000);

        if (c.getExitCode() != 0)
            throw CommandLineError ("git command failed!");
    }

    //==============================================================================
    static void showStatus (const StringArray& args)
    {
        hideDockIcon();
        checkArgumentCount (args, 2);

        LoadedProject proj (args[1]);

        std::cout << "Project file: " << proj.project->getFile().getFullPathName() << std::endl
                  << "Name: " << proj.project->getTitle() << std::endl
                  << "UID: " << proj.project->getProjectUID() << std::endl;

        EnabledModuleList& modules = proj.project->getModules();

        if (int numModules = modules.getNumModules())
        {
            std::cout << "Modules:" << std::endl;

            for (int i = 0; i < numModules; ++i)
                std::cout << "  " << modules.getModuleID (i) << std::endl;
        }
    }

    //==============================================================================
    static String getModulePackageName (const LibraryModule& module)
    {
        return module.getID() + ".jucemodule";
    }

    static void zipModule (const File& targetFolder, const File& moduleFolder)
    {
        jassert (targetFolder.isDirectory());

        const File moduleFolderParent (moduleFolder.getParentDirectory());
        LibraryModule module (moduleFolder);

        if (! module.isValid())
            throw CommandLineError (moduleFolder.getFullPathName() + " is not a valid module folder!");

        const File targetFile (targetFolder.getChildFile (getModulePackageName (module)));

        ZipFile::Builder zip;

        {
            DirectoryIterator i (moduleFolder, true, "*", File::findFiles);

            while (i.next())
                if (! i.getFile().isHidden())
                    zip.addFile (i.getFile(), 9, i.getFile().getRelativePathFrom (moduleFolderParent));
        }

        std::cout << "Writing: " << targetFile.getFullPathName() << std::endl;

        TemporaryFile temp (targetFile);
        ScopedPointer<FileOutputStream> out (temp.getFile().createOutputStream());

        bool ok = out != nullptr && zip.writeToStream (*out, nullptr);
        out = nullptr;
        ok = ok && temp.overwriteTargetFileWithTemporary();

        if (! ok)
            throw CommandLineError ("Failed to write to the target file: " + targetFile.getFullPathName());
    }

    static void buildModules (const StringArray& args, const bool buildAllWithIndex)
    {
        hideDockIcon();
        checkArgumentCount (args, 3);

        const File targetFolder (getFile (args[1]));

        if (! targetFolder.isDirectory())
            throw CommandLineError ("The first argument must be the directory to put the result.");

        if (buildAllWithIndex)
        {
            const File folderToSearch (getFile (args[2]));
            DirectoryIterator i (folderToSearch, false, "*", File::findDirectories);
            var infoList;

            while (i.next())
            {
                LibraryModule module (i.getFile());

                if (module.isValid())
                {
                    zipModule (targetFolder, i.getFile());

                    var moduleInfo (new DynamicObject());
                    moduleInfo.getDynamicObject()->setProperty ("file", getModulePackageName (module));
                    moduleInfo.getDynamicObject()->setProperty ("info", module.moduleInfo.moduleInfo);
                    infoList.append (moduleInfo);
                }
            }

            const File indexFile (targetFolder.getChildFile ("modulelist"));
            std::cout << "Writing: " << indexFile.getFullPathName() << std::endl;
            indexFile.replaceWithText (JSON::toString (infoList), false, false);
        }
        else
        {
            for (int i = 2; i < args.size(); ++i)
                zipModule (targetFolder, getFile (args[i]));
        }
    }

    //==============================================================================
    struct CleanupOptions
    {
        bool removeTabs;
        bool fixDividerComments;
    };

    static void cleanWhitespace (const File& file, CleanupOptions options)
    {
        const String content (file.loadFileAsString());

        if (content.contains ("%""%") && content.contains ("//["))
            return; // ignore projucer GUI template files

        StringArray lines;
        lines.addLines (content);
        bool anyTabsRemoved = false;

        for (int i = 0; i < lines.size(); ++i)
        {
            String& line = lines.getReference(i);

            if (options.removeTabs && line.containsChar ('\t'))
            {
                anyTabsRemoved = true;

                for (;;)
                {
                    const int tabPos = line.indexOfChar ('\t');
                    if (tabPos < 0)
                        break;

                    const int spacesPerTab = 4;
                    const int spacesNeeded = spacesPerTab - (tabPos % spacesPerTab);
                    line = line.replaceSection (tabPos, 1, String::repeatedString (" ", spacesNeeded));
                }
            }

            if (options.fixDividerComments)
            {
                String afterIndent (line.trim());

                if (afterIndent.startsWith ("//") && afterIndent.length() > 20)
                {
                    afterIndent = afterIndent.substring (2);

                    if (afterIndent.containsOnly ("=")
                          || afterIndent.containsOnly ("/")
                          || afterIndent.containsOnly ("-"))
                    {
                        line = line.substring (0, line.indexOfChar ('/'))
                                  + "//" + String::repeatedString ("=", 78);
                    }
                }
            }

            line = line.trimEnd();
        }

        if (options.removeTabs && ! anyTabsRemoved)
            return;

        const String newText = joinLinesIntoSourceFile (lines);

        if (newText != content && newText != content + getLineEnding())
            replaceFile (file, newText, options.removeTabs ? "Removing tabs in: "
                                                           : "Cleaning file: ");
    }

    static void scanFilesForCleanup (const StringArray& args, CleanupOptions options)
    {
        checkArgumentCount (args, 2);

        for (auto it = args.begin() + 1; it < args.end(); ++it)
        {
            const File target (getFileCheckingForExistence (*it));

            Array<File> files;

            if (target.isDirectory())
                files = findAllSourceFiles (target);
            else
                files.add (target);

            for (int i = 0; i < files.size(); ++i)
                cleanWhitespace (files.getReference(i), options);
        }
    }

    static void cleanWhitespace (const StringArray& args, bool replaceTabs)
    {
        CleanupOptions options = { replaceTabs, false };
        scanFilesForCleanup (args, options);
    }

    static void tidyDividerComments (const StringArray& args)
    {
        CleanupOptions options = { false, true };
        scanFilesForCleanup (args, options);
    }

    //==============================================================================
    static File findSimilarlyNamedHeader (const Array<File>& allFiles, const String& name, const File& sourceFile)
    {
        File result;

        for (auto& f : allFiles)
        {
            if (f.getFileName().equalsIgnoreCase (name) && f != sourceFile)
            {
                if (result.exists())
                    return {}; // multiple possible results, so don't change it!

                result = f;
            }
        }

        return result;
    }

    static void fixIncludes (const File& file, const Array<File>& allFiles)
    {
        const String content (file.loadFileAsString());

        StringArray lines;
        lines.addLines (content);
        bool hasChanged = false;

        for (int i = 0; i < lines.size(); ++i)
        {
            String line = lines[i];

            if (line.trimStart().startsWith ("#include \""))
            {
                const String includedFile (line.fromFirstOccurrenceOf ("\"", true, false)
                                               .upToLastOccurrenceOf ("\"", true, false)
                                               .trim()
                                               .unquoted());

                const File target (file.getSiblingFile (includedFile));

                if (! target.exists())
                {
                    File header = findSimilarlyNamedHeader (allFiles, target.getFileName(), file);

                    if (header.exists())
                    {
                        lines.set (i, line.upToFirstOccurrenceOf ("#include \"", true, false)
                                        + header.getRelativePathFrom (file.getParentDirectory())
                                            .replaceCharacter ('\\', '/')
                                        + "\"");
                        hasChanged = true;
                    }
                }
            }
        }

        if (hasChanged)
        {
            const String newText = joinLinesIntoSourceFile (lines);

            if (newText != content && newText != content + getLineEnding())
                replaceFile (file, newText, "Fixing includes in: ");
        }
    }

    static void fixRelativeIncludePaths (const StringArray& args)
    {
        checkArgumentCount (args, 2);
        const File target (getDirectoryCheckingForExistence (args[1]));

        Array<File> files = findAllSourceFiles (target);

        for (int i = 0; i < files.size(); ++i)
            fixIncludes (files.getReference(i), files);
    }

    //==============================================================================
    static String getStringConcatenationExpression (Random& rng, int start, int length)
    {
        jassert (length > 0);

        if (length == 1)
            return "s" + String (start);

        int breakPos = jlimit (1, length - 1, (length / 3) + rng.nextInt (length / 3));

        return "(" + getStringConcatenationExpression (rng, start, breakPos)
                + " + " + getStringConcatenationExpression (rng, start + breakPos, length - breakPos) + ")";
    }

    static void generateObfuscatedStringCode (const StringArray& args)
    {
        checkArgumentCount (args, 2);
        const String originalText (args[1]);

        struct Section
        {
            String text;
            int position, index;

            void writeGenerator (MemoryOutputStream& out) const
            {
                String name ("s" + String (index));

                out << "    String " << name << ";  " << name;

                for (int i = 0; i < text.length(); ++i)
                    out << " << '" << String::charToString (text[i]) << "'";

                out << ";" << newLine;
            }
        };

        Array<Section> sections;
        String text = originalText;
        Random rng;

        while (text.isNotEmpty())
        {
            int pos = jmax (0, text.length() - (1 + rng.nextInt (6)));
            Section s = { text.substring (pos), pos, 0 };
            sections.insert (0, s);
            text = text.substring (0, pos);
        }

        for (int i = 0; i < sections.size(); ++i)
            sections.getReference(i).index = i;

        for (int i = 0; i < sections.size(); ++i)
            sections.swap (i, rng.nextInt (sections.size()));

        MemoryOutputStream out;

        out << "String createString()" << newLine
            << "{" << newLine;

        for (int i = 0; i < sections.size(); ++i)
            sections.getReference(i).writeGenerator (out);

        out << newLine
            << "    String result = " << getStringConcatenationExpression (rng, 0, sections.size()) << ";" << newLine
            << newLine
            << "    jassert (result == " << originalText.quoted() << ");" << newLine
            << "    return result;" << newLine
            << "}" << newLine;

        std::cout << out.toString() << std::endl;
    }

    static void scanFoldersForTranslationFiles (const StringArray& args)
    {
        checkArgumentCount (args, 2);

        StringArray translations;

        for (auto it = args.begin() + 1; it != args.end(); ++it)
        {
            const File directoryToSearch (getDirectoryCheckingForExistence (*it));
            TranslationHelpers::scanFolderForTranslations (translations, directoryToSearch);
        }

        std::cout << TranslationHelpers::mungeStrings (translations) << std::endl;
    }

    static void createFinishedTranslationFile (const StringArray& args)
    {
        checkArgumentCount (args, 3);

        auto preTranslated  = getFileCheckingForExistence (args[1]).loadFileAsString();
        auto postTranslated = getFileCheckingForExistence (args[2]).loadFileAsString();

        auto localisedContent = (args.size() > 3 ? getFileCheckingForExistence (args[3]).loadFileAsString() : String());
        auto localised        = LocalisedStrings (localisedContent, false);

        using TH = TranslationHelpers;
        std::cout << TH::createFinishedTranslationFile (TH::withTrimmedEnds (TH::breakApart (preTranslated)),
                                                        TH::withTrimmedEnds (TH::breakApart (postTranslated)),
                                                        localised) << std::endl;
    }

    //==============================================================================
    static void encodeBinary (const StringArray& args)
    {
        checkArgumentCount (args, 3);
        const File source (getFileCheckingForExistence (args[1]));
        const File target (getFile (args[2]));

        MemoryOutputStream literal;
        size_t dataSize = 0;

        {
            MemoryBlock data;
            FileInputStream input (source);
            input.readIntoMemoryBlock (data);
            CodeHelpers::writeDataAsCppLiteral (data, literal, true, true);
            dataSize = data.getSize();
        }

        auto variableName = CodeHelpers::makeBinaryDataIdentifierName (source);

        MemoryOutputStream header, cpp;

        header << "// Auto-generated binary data by the Projucer" << newLine
               << "// Source file: " << source.getRelativePathFrom (target.getParentDirectory()) << newLine
               << newLine;

        cpp << header.toString();

        if (target.hasFileExtension (headerFileExtensions))
        {
            header << "static constexpr unsigned char " << variableName << "[] =" << newLine
                   << literal.toString() << newLine
                   << newLine;

            replaceFile (target, header.toString(), "Writing: ");
        }
        else if (target.hasFileExtension (cppFileExtensions))
        {
            header << "extern const char*  " << variableName << ";" << newLine
                   << "const unsigned int  " << variableName << "Size = " << (int) dataSize << ";" << newLine
                   << newLine;

            cpp << CodeHelpers::createIncludeStatement (target.withFileExtension (".h").getFileName()) << newLine
                << newLine
                << "static constexpr unsigned char " << variableName << "_local[] =" << newLine
                << literal.toString() << newLine
                << newLine
                << "const char* " << variableName << " = (const char*) " << variableName << "_local;" << newLine;

            replaceFile (target, cpp.toString(), "Writing: ");
            replaceFile (target.withFileExtension (".h"), header.toString(), "Writing: ");
        }
        else
        {
            throw CommandLineError ("You need to specify a .h or .cpp file as the target");
        }
    }

    //==============================================================================
    static void showHelp()
    {
        hideDockIcon();

        const String appName (JUCEApplication::getInstance()->getApplicationName());

        std::cout << appName << std::endl
                  << std::endl
                  << "Usage: " << std::endl
                  << std::endl
                  << " " << appName << " --resave project_file" << std::endl
                  << "    Resaves all files and resources in a project." << std::endl
                  << std::endl
                  << " " << appName << " --resave-resources project_file" << std::endl
                  << "    Resaves just the binary resources for a project." << std::endl
                  << std::endl
                  << " " << appName << " --get-version project_file" << std::endl
                  << "    Returns the version number of a project." << std::endl
                  << std::endl
                  << " " << appName << " --set-version version_number project_file" << std::endl
                  << "    Updates the version number in a project." << std::endl
                  << std::endl
                  << " " << appName << " --bump-version project_file" << std::endl
                  << "    Updates the minor version number in a project by 1." << std::endl
                  << std::endl
                  << " " << appName << " --git-tag-version project_file" << std::endl
                  << "    Invokes 'git tag' to attach the project's version number to the current git repository." << std::endl
                  << std::endl
                  << " " << appName << " --status project_file" << std::endl
                  << "    Displays information about a project." << std::endl
                  << std::endl
                  << " " << appName << " --buildmodule target_folder module_folder" << std::endl
                  << "    Zips a module into a downloadable file format." << std::endl
                  << std::endl
                  << " " << appName << " --buildallmodules target_folder module_folder" << std::endl
                  << "    Zips all modules in a given folder and creates an index for them." << std::endl
                  << std::endl
                  << " " << appName << " --trim-whitespace target_folder" << std::endl
                  << "    Scans the given folder for C/C++ source files (recursively), and trims any trailing whitespace from their lines, as well as normalising their line-endings to CR-LF." << std::endl
                  << std::endl
                  << " " << appName << " --remove-tabs target_folder" << std::endl
                  << "    Scans the given folder for C/C++ source files (recursively), and replaces any tab characters with 4 spaces." << std::endl
                  << std::endl
                  << " " << appName << " --tidy-divider-comments target_folder" << std::endl
                  << "    Scans the given folder for C/C++ source files (recursively), and normalises any juce-style comment division lines (i.e. any lines that look like //===== or //------- or /////////// will be replaced)." << std::endl
                  << std::endl
                  << " " << appName << " --fix-broken-include-paths target_folder" << std::endl
                  << "    Scans the given folder for C/C++ source files (recursively). Where a file contains an #include of one of the other filenames, it changes it to use the optimum relative path. Helpful for auto-fixing includes when re-arranging files and folders in a project." << std::endl
                  << std::endl
                  << " " << appName << " --obfuscated-string-code string_to_obfuscate" << std::endl
                  << "    Generates a C++ function which returns the given string, but in an obfuscated way." << std::endl
                  << std::endl
                  << " " << appName << " --encode-binary source_binary_file target_cpp_file" << std::endl
                  << "    Converts a binary file to a C++ file containing its contents as a block of data. Provide a .h file as the target if you want a single output file, or a .cpp file if you want a pair of .h/.cpp files." << std::endl
                  << std::endl
                  << " " << appName << " --trans target_folders..." << std::endl
                  << "    Scans each of the given folders (recursively) for any NEEDS_TRANS macros, and generates a translation file that can be used with Projucer's translation file builder" << std::endl
                  << std::endl
                  << " " << appName << " --trans-finish pre_translated_file post_translated_file optional_existing_translation_file" << std::endl
                  << "    Creates a completed translations mapping file, that can be used to initialise a LocalisedStrings object. This allows you to localise the strings in your project" << std::endl
                  << std::endl;
    }
}

//==============================================================================
int performCommandLine (const String& commandLine)
{
    StringArray args;
    args.addTokens (commandLine, true);
    args.trim();

    String command (args[0]);

    try
    {
        if (matchArgument (command, "help"))                     { showHelp(); return 0; }
        if (matchArgument (command, "h"))                        { showHelp(); return 0; }
        if (matchArgument (command, "resave"))                   { resaveProject (args, false); return 0; }
        if (matchArgument (command, "resave-resources"))         { resaveProject (args, true); return 0; }
        if (matchArgument (command, "get-version"))              { getVersion (args); return 0; }
        if (matchArgument (command, "set-version"))              { setVersion (args); return 0; }
        if (matchArgument (command, "bump-version"))             { bumpVersion (args); return 0; }
        if (matchArgument (command, "git-tag-version"))          { gitTag (args); return 0; }
        if (matchArgument (command, "buildmodule"))              { buildModules (args, false); return 0; }
        if (matchArgument (command, "buildallmodules"))          { buildModules (args, true); return 0; }
        if (matchArgument (command, "status"))                   { showStatus (args); return 0; }
        if (matchArgument (command, "trim-whitespace"))          { cleanWhitespace (args, false); return 0; }
        if (matchArgument (command, "remove-tabs"))              { cleanWhitespace (args, true); return 0; }
        if (matchArgument (command, "tidy-divider-comments"))    { tidyDividerComments (args); return 0; }
        if (matchArgument (command, "fix-broken-include-paths")) { fixRelativeIncludePaths (args); return 0; }
        if (matchArgument (command, "obfuscated-string-code"))   { generateObfuscatedStringCode (args); return 0; }
        if (matchArgument (command, "encode-binary"))            { encodeBinary (args); return 0; }
        if (matchArgument (command, "trans"))                    { scanFoldersForTranslationFiles (args); return 0; }
        if (matchArgument (command, "trans-finish"))             { createFinishedTranslationFile (args); return 0; }
    }
    catch (const CommandLineError& error)
    {
        std::cout << error.message << std::endl << std::endl;
        return 1;
    }

    return commandLineNotPerformed;
}
