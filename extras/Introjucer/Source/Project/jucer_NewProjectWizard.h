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

#ifndef __JUCER_NEWPROJECTWIZARD_JUCEHEADER__
#define __JUCER_NEWPROJECTWIZARD_JUCEHEADER__

#include "../jucer_Headers.h"
#include "jucer_Project.h"


//==============================================================================
class NewProjectWizard
{
public:
    virtual ~NewProjectWizard();

    //==============================================================================
    static StringArray getWizards();
    static int getNumWizards();
    static NewProjectWizard* createWizard (int index);

    static Component* createComponent();

    //==============================================================================
    virtual String getName() = 0;
    virtual String getDescription() = 0;

    virtual void addSetupItems (Component& setupComp, OwnedArray<Component>& itemsCreated) = 0;
    virtual Result processResultsFromSetupItems (Component& setupComp) = 0;
    virtual bool initialiseProject (Project& project) = 0;

protected:
    String appTitle;
    File targetFolder, projectFile;
    Component* ownerWindow;
    StringArray failedFiles;

    //==============================================================================
    NewProjectWizard();
    Project* runWizard (Component* ownerWindow,
                        const String& projectName,
                        const File& targetFolder);

    class WizardComp;
    friend class WizardComp;

    File getSourceFilesFolder() const         { return projectFile.getSiblingFile ("Source"); }
    static File& getLastWizardFolder();
};


#endif   // __JUCER_NEWPROJECTWIZARD_JUCEHEADER__
