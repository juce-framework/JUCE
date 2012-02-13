/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

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

#include "../Project/jucer_Project.h"
#include "../Project/jucer_Module.h"
#include "jucer_CommandLine.h"


//==============================================================================
namespace
{
    File getFile (const String& filename)
    {
        return File::getCurrentWorkingDirectory().getChildFile (filename.unquoted());
    }

    bool checkArgumentCount (const StringArray& tokens, int minNumArgs)
    {
        if (tokens.size() < minNumArgs)
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
    int resaveProject (const File& file)
    {
        if (! file.exists())
        {
            std::cout << "The file " << file.getFullPathName() << " doesn't exist!" << std::endl;
            return 1;
        }

        if (! file.hasFileExtension (Project::projectFileExtension))
        {
            std::cout << file.getFullPathName() << " isn't a valid jucer project file!" << std::endl;
            return 1;
        }

        Project newDoc (file);

        if (! newDoc.loadFrom (file, true))
        {
            std::cout << "Failed to load the project file: " << file.getFullPathName() << std::endl;
            return 1;
        }

        std::cout << "The Introjucer - Re-saving file: " << file.getFullPathName() << std::endl;
        String error (newDoc.saveDocument (file));

        if (error.isNotEmpty())
        {
            std::cout << "Error when writing project: " << error << std::endl;
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
        LibraryModule module (moduleFolder.getChildFile (LibraryModule::getInfoFileName()));

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

        bool ok = out != nullptr && zip.writeToStream (*out);
        out = nullptr;
        ok = ok && temp.overwriteTargetFileWithTemporary();

        if (! ok)
        {
            std::cout << "Failed to write to the target file: " << targetFile.getFullPathName() << std::endl;
            return 1;
        }

        return 0;
    }

    int buildModules (const StringArray& tokens, const bool buildAllWithIndex)
    {
        if (! checkArgumentCount (tokens, 3))
            return 1;

        const File targetFolder (getFile (tokens[1]));

        if (! targetFolder.isDirectory())
        {
            std::cout << "The first argument must be the directory to put the result." << std::endl;
            return 1;
        }

        if (buildAllWithIndex)
        {
            const File folderToSearch (getFile (tokens[2]));
            DirectoryIterator i (folderToSearch, false, "*", File::findDirectories);
            var infoList;

            while (i.next())
            {
                LibraryModule module (i.getFile().getChildFile (LibraryModule::getInfoFileName()));

                if (module.isValid())
                {
                    const int result = zipModule (targetFolder, i.getFile());

                    if (result != 0)
                        return result;

                    var moduleInfo (new DynamicObject());
                    moduleInfo.getDynamicObject()->setProperty ("file", getModulePackageName (module));
                    moduleInfo.getDynamicObject()->setProperty ("info", module.moduleInfo);
                    infoList.append (moduleInfo);
                }
            }

            const File indexFile (targetFolder.getChildFile ("modulelist"));
            std::cout << "Writing: " << indexFile.getFullPathName() << std::endl;
            indexFile.replaceWithText (JSON::toString (infoList), false, false);
        }
        else
        {
            for (int i = 2; i < tokens.size(); ++i)
            {
                const int result = zipModule (targetFolder, getFile (tokens[i]));

                if (result != 0)
                    return result;
            }
        }

        return 0;
    }

    int listModules()
    {
        std::cout << "Downloading list of available modules..." << std::endl;
        ModuleList list;
        list.loadFromWebsite();

        for (int i = 0; i < list.modules.size(); ++i)
        {
            ModuleList::Module* m = list.modules.getUnchecked(i);

            std::cout << m->uid << ": " << m->version << std::endl;
        }

        return 0;
    }

    int showStatus (const StringArray& tokens)
    {
        if (! checkArgumentCount (tokens, 2))
            return 1;

        const File projectFile (getFile (tokens[1]));

        Project proj (projectFile);

        if (proj.loadDocument (projectFile).isNotEmpty())
        {
            std::cout << "Failed to load project: " << projectFile.getFullPathName() << std::endl;
            return 1;
        }

        std::cout << "Project file: " << projectFile.getFullPathName() << std::endl
                  << "Name: " << proj.getProjectName().toString() << std::endl
                  << "UID: " << proj.getProjectUID() << std::endl;

        const int numModules = proj.getNumModules();
        if (numModules > 0)
        {
            std::cout << "Modules:" << std::endl;

            for (int i = 0; i < numModules; ++i)
                std::cout << "  " << proj.getModuleID (i) << std::endl;
        }

        return 0;
    }
}

//==============================================================================
int performCommandLine (const String& commandLine)
{
    StringArray tokens;
    tokens.addTokens (commandLine, true);
    tokens.trim();

    if (tokens[0] == "-resave" || tokens[0] == "--resave" || tokens[0] == "resave")
        return resaveProject (getFile (tokens[1]));

    if (tokens[0] == "buildmodule")
        return buildModules (tokens, false);

    if (tokens[0] == "buildallmodules")
        return buildModules (tokens, true);

    if (tokens[0] == "listmodules")
        return listModules();

    if (tokens[0] == "status")
        return showStatus (tokens);

    return commandLineNotPerformed;
}
