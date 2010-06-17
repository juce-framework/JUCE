/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

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

#ifndef __JUCER_PROJECTWIZARD_JUCEHEADER__
#define __JUCER_PROJECTWIZARD_JUCEHEADER__

#include "../jucer_Headers.h"
#include "jucer_Project.h"


//==============================================================================
class NewProjectWizard
{
public:
    virtual ~NewProjectWizard();

    //==============================================================================
    static const StringArray getWizards();
    static int getNumWizards();
    static NewProjectWizard* createWizard (int index);

    static Project* runNewProjectWizard (Component* ownerWindow);

    //==============================================================================
    virtual const String getName() = 0;
    virtual const String getDescription() = 0;

    virtual void addItemsToAlertWindow (AlertWindow& aw) = 0;
    virtual const String processResultsFromAlertWindow (AlertWindow& aw) = 0;
    virtual bool initialiseProject (Project& project) = 0;

protected:
    String appTitle;
    File targetFolder, projectFile;
    Component* ownerWindow;
    StringArray failedFiles;

    //==============================================================================
    NewProjectWizard();
    Project* runWizard (Component* ownerWindow);

    const File getSourceFilesFolder() const         { return projectFile.getSiblingFile ("Source"); }
};


#endif   // __JUCER_PROJECTWIZARD_JUCEHEADER__
