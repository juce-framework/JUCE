/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

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
    void hideDockIcon()
    {
       #if JUCE_MAC
        Process::setDockIconVisible (false);
       #endif
    }

    File getFile (const String& filename)
    {
        return File::getCurrentWorkingDirectory().getChildFile (filename.unquoted());
    }

    bool checkArgumentCount (const StringArray& args, int minNumArgs)
    {
        if (args.size() < minNumArgs)
        {
            std::cout << "Not enough arguments!" << std::endl;
            return false;
        }

        return true;
    }

    //==============================================================================
    /* Running a command-line of the form "introjucer --resave foobar.jucer" will try to load
       that project and re-export all of its targets.
    */
    int resaveProject (const StringArray& args, bool justSaveResources)
    {
        hideDockIcon();

        if (! checkArgumentCount (args, 2))
            return 1;

        const File projectFile (getFile (args[1]));

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

        Project proj (projectFile);

        if (! proj.loadFrom (projectFile, true))
        {
            std::cout << "Failed to load the project file: " << projectFile.getFullPathName() << std::endl;
            return 1;
        }

        std::cout << (justSaveResources ? "The Introjucer - Re-saving project resources: "
                                        : "The Introjucer - Re-saving file: ")
                  << projectFile.getFullPathName() << std::endl;

        Result error (justSaveResources ? proj.saveResourcesOnly (projectFile)
                                        : proj.saveProject (projectFile, true));

        if (error.failed())
        {
            std::cout << "Error when saving: " << error.getErrorMessage() << std::endl;
            return 1;
        }

        return 0;
    }

    //==============================================================================
    String getModulePackageName (const LibraryModule& module)
    {
        return module.getID() + ".jucemodule";
    }

    int zipModule (const File& targetFolder, const File& moduleFolder)
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

    int buildModules (const StringArray& args, const bool buildAllWithIndex)
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

    int showStatus (const StringArray& args)
    {
        hideDockIcon();

        if (! checkArgumentCount (args, 2))
            return 1;

        const File projectFile (getFile (args[1]));

        Project proj (projectFile);
        const Result result (proj.loadDocument (projectFile));

        if (result.failed())
        {
            std::cout << "Failed to load project: " << projectFile.getFullPathName() << std::endl;
            return 1;
        }

        std::cout << "Project file: " << projectFile.getFullPathName() << std::endl
                  << "Name: " << proj.getTitle() << std::endl
                  << "UID: " << proj.getProjectUID() << std::endl;

        EnabledModuleList& modules = proj.getModules();

        const int numModules = modules.getNumModules();
        if (numModules > 0)
        {
            std::cout << "Modules:" << std::endl;

            for (int i = 0; i < numModules; ++i)
                std::cout << "  " << modules.getModuleID (i) << std::endl;
        }

        return 0;
    }

    bool matchArgument (const String& arg, const String& possible)
    {
        return arg == possible
            || arg == "-" + possible
            || arg == "--" + possible;
    }

    //==============================================================================
    int showHelp()
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

    if (matchArgument (args[0], "help"))                return showHelp();
    if (matchArgument (args[0], "resave"))              return resaveProject (args, false);
    if (matchArgument (args[0], "resave-resources"))    return resaveProject (args, true);
    if (matchArgument (args[0], "buildmodule"))         return buildModules (args, false);
    if (matchArgument (args[0], "buildallmodules"))     return buildModules (args, true);
    if (matchArgument (args[0], "status"))              return showStatus (args);

    return commandLineNotPerformed;
}
