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

#include "../Project/jucer_Project.h"
#include "../Project/jucer_Module.h"
#include "jucer_CommandLine.h"


//==============================================================================
namespace
{
    static void hideDockIcon()
    {
       #if JUCE_MAC
        Process::setDockIconVisible (false);
       #endif
    }

    static File getFile (const String& filename)
    {
        return File::getCurrentWorkingDirectory().getChildFile (filename.unquoted());
    }

    static bool matchArgument (const String& arg, const String& possible)
    {
        return arg == possible
            || arg == "-" + possible
            || arg == "--" + possible;
    }

    static bool checkArgumentCount (const StringArray& args, int minNumArgs)
    {
        if (args.size() < minNumArgs)
        {
            std::cout << "Not enough arguments!" << std::endl;
            return false;
        }

        return true;
    }

    //==============================================================================
    struct LoadedProject
    {
        LoadedProject()
        {
            hideDockIcon();
        }

        int load (const File& projectFile)
        {
            hideDockIcon();

            if (! projectFile.exists())
            {
                std::cout << "The file " << projectFile.getFullPathName() << " doesn't exist!" << std::endl;
                return 1;
            }

            if (! projectFile.hasFileExtension (Project::projectFileExtension))
            {
                std::cout << projectFile.getFullPathName() << " isn't a valid jucer project file!" << std::endl;
                return 1;
            }

            project = new Project (projectFile);

            if (! project->loadFrom (projectFile, true))
            {
                project = nullptr;
                std::cout << "Failed to load the project file: " << projectFile.getFullPathName() << std::endl;
                return 1;
            }

            return 0;
        }

        int save (bool justSaveResources)
        {
            if (project != nullptr)
            {
                Result error (justSaveResources ? project->saveResourcesOnly (project->getFile())
                                                : project->saveProject (project->getFile(), true));

                if (error.failed())
                {
                    std::cout << "Error when saving: " << error.getErrorMessage() << std::endl;
                    return 1;
                }

                project = nullptr;
            }

            return 0;
        }

        ScopedPointer<Project> project;
    };

    //==============================================================================
    /* Running a command-line of the form "introjucer --resave foobar.jucer" will try to load
       that project and re-export all of its targets.
    */
    static int resaveProject (const StringArray& args, bool justSaveResources)
    {
        if (! checkArgumentCount (args, 2))
            return 1;

        LoadedProject proj;

        int res = proj.load (getFile (args[1]));

        if (res != 0)
            return res;

        std::cout << (justSaveResources ? "Re-saving project resources: "
                                        : "Re-saving file: ")
                  << proj.project->getFile().getFullPathName() << std::endl;

        return proj.save (justSaveResources);
    }

    //==============================================================================
    static int setVersion (const StringArray& args)
    {
        if (! checkArgumentCount (args, 3))
            return 1;

        LoadedProject proj;

        int res = proj.load (getFile (args[2]));

        if (res != 0)
            return res;

        String version (args[1].trim());

        std::cout << "Setting project version: " << version << std::endl;

        proj.project->getVersionValue() = version;

        return proj.save (false);
    }

    //==============================================================================
    static int bumpVersion (const StringArray& args)
    {
        if (! checkArgumentCount (args, 2))
            return 1;

        LoadedProject proj;

        int res = proj.load (getFile (args[1]));

        if (res != 0)
            return res;

        String version = proj.project->getVersionString();

        version = version.upToLastOccurrenceOf (".", true, false)
                    + String (version.getTrailingIntValue() + 1);

        std::cout << "Bumping project version to: " << version << std::endl;

        proj.project->getVersionValue() = version;

        return proj.save (false);
    }

    static int gitTag (const StringArray& args)
    {
        if (! checkArgumentCount (args, 2))
            return 1;

        LoadedProject proj;

        int res = proj.load (getFile (args[1]));

        if (res != 0)
            return res;

        String version (proj.project->getVersionValue().toString());

        if (version.trim().isEmpty())
        {
            std::cout << "Cannot read version number from project!" << std::endl;
            return 1;
        }

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
        {
            std::cout << "Cannot run git!" << std::endl;
            return 1;
        }

        c.waitForProcessToFinish (10000);
        return (int) c.getExitCode();
    }

    //==============================================================================
    int showStatus (const StringArray& args)
    {
        hideDockIcon();

        if (! checkArgumentCount (args, 2))
            return 1;

        LoadedProject proj;

        int res = proj.load (getFile (args[1]));

        if (res != 0)
            return res;

        std::cout << "Project file: " << proj.project->getFile().getFullPathName() << std::endl
                  << "Name: " << proj.project->getTitle() << std::endl
                  << "UID: " << proj.project->getProjectUID() << std::endl;

        EnabledModuleList& modules = proj.project->getModules();

        const int numModules = modules.getNumModules();
        if (numModules > 0)
        {
            std::cout << "Modules:" << std::endl;

            for (int i = 0; i < numModules; ++i)
                std::cout << "  " << modules.getModuleID (i) << std::endl;
        }

        return 0;
    }

    //==============================================================================
    static String getModulePackageName (const LibraryModule& module)
    {
        return module.getID() + ".jucemodule";
    }

    static int zipModule (const File& targetFolder, const File& moduleFolder)
    {
        jassert (targetFolder.isDirectory());

        const File moduleFolderParent (moduleFolder.getParentDirectory());
        LibraryModule module (moduleFolder.getChildFile (ModuleDescription::getManifestFileName()));

        if (! module.isValid())
        {
            std::cout << moduleFolder.getFullPathName() << " is not a valid module folder!" << std::endl;
            return 1;
        }

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
        {
            std::cout << "Failed to write to the target file: " << targetFile.getFullPathName() << std::endl;
            return 1;
        }

        return 0;
    }

    static int buildModules (const StringArray& args, const bool buildAllWithIndex)
    {
        hideDockIcon();

        if (! checkArgumentCount (args, 3))
            return 1;

        const File targetFolder (getFile (args[1]));

        if (! targetFolder.isDirectory())
        {
            std::cout << "The first argument must be the directory to put the result." << std::endl;
            return 1;
        }

        if (buildAllWithIndex)
        {
            const File folderToSearch (getFile (args[2]));
            DirectoryIterator i (folderToSearch, false, "*", File::findDirectories);
            var infoList;

            while (i.next())
            {
                LibraryModule module (i.getFile().getChildFile (ModuleDescription::getManifestFileName()));

                if (module.isValid())
                {
                    const int result = zipModule (targetFolder, i.getFile());

                    if (result != 0)
                        return result;

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
            {
                const int result = zipModule (targetFolder, getFile (args[i]));

                if (result != 0)
                    return result;
            }
        }

        return 0;
    }

    //==============================================================================
    static int showHelp()
    {
        hideDockIcon();

        std::cout << "The Introjucer!" << std::endl
                  << std::endl
                  << "Usage: " << std::endl
                  << std::endl
                  << " introjucer --resave project_file" << std::endl
                  << "    Resaves all files and resources in a project." << std::endl
                  << std::endl
                  << " introjucer --resave-resources project_file" << std::endl
                  << "    Resaves just the binary resources for a project." << std::endl
                  << std::endl
                  << " introjucer --set-version version_number project_file" << std::endl
                  << "    Updates the version number in a project." << std::endl
                  << std::endl
                  << " introjucer --bump-version project_file" << std::endl
                  << "    Updates the minor version number in a project by 1." << std::endl
                  << std::endl
                  << " introjucer --git-tag-version project_file" << std::endl
                  << "    Invokes 'git tag' to attach the project's version number to the current git repository." << std::endl
                  << std::endl
                  << " introjucer --status project_file" << std::endl
                  << "    Displays information about a project." << std::endl
                  << std::endl
                  << " introjucer --buildmodule target_folder module_folder" << std::endl
                  << "    Zips a module into a downloadable file format." << std::endl
                  << std::endl
                  << " introjucer --buildallmodules target_folder module_folder" << std::endl
                  << "    Zips all modules in a given folder and creates an index for them." << std::endl
                  << std::endl;

        return 0;
    }
}

//==============================================================================
int performCommandLine (const String& commandLine)
{
    StringArray args;
    args.addTokens (commandLine, true);
    args.trim();

    String command (args[0]);

    if (matchArgument (command, "help"))                return showHelp();
    if (matchArgument (command, "h"))                   return showHelp();
    if (matchArgument (command, "resave"))              return resaveProject (args, false);
    if (matchArgument (command, "resave-resources"))    return resaveProject (args, true);
    if (matchArgument (command, "set-version"))         return setVersion (args);
    if (matchArgument (command, "bump-version"))        return bumpVersion (args);
    if (matchArgument (command, "git-tag-version"))     return gitTag (args);
    if (matchArgument (command, "buildmodule"))         return buildModules (args, false);
    if (matchArgument (command, "buildallmodules"))     return buildModules (args, true);
    if (matchArgument (command, "status"))              return showStatus (args);

    return commandLineNotPerformed;
}
