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
#include "jucer_CommandLine.h"


//==============================================================================
namespace
{
    File getFile (const String& filename)
    {
        return File::getCurrentWorkingDirectory().getChildFile (filename.unquoted());
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
    int buildModule (const StringArray& tokens)
    {

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
        return buildModule (tokens);

    return commandLineNotPerformed;
}
