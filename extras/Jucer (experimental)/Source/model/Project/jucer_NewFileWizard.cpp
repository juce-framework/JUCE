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

#include "jucer_NewFileWizard.h"
#include "../Drawable/jucer_DrawableDocument.h"
#include "../Component/jucer_ComponentDocument.h"


//==============================================================================
static bool fillInNewCppFileTemplate (const File& file, const Project::Item& item, const char* templateName)
{
    String s = item.getProject().getFileTemplate (templateName)
                  .replace ("FILENAME", file.getFileName(), false)
                  .replace ("DATE", Time::getCurrentTime().toString (true, true, true), false)
                  .replace ("AUTHOR", SystemStats::getFullUserName(), false)
                  .replace ("HEADERGUARD", makeHeaderGuardName (file), false);

    return overwriteFileWithNewDataIfDifferent (file, s);
}

//==============================================================================
class NewCppFileWizard  : public NewFileWizard
{
public:
    NewCppFileWizard() {}
    ~NewCppFileWizard() {}

    const String getName()  { return "CPP File"; }

    void createNewFile (Project::Item parent)
    {
        File newFile = askUserToChooseNewFile ("SourceCode.cpp", "*.cpp", parent);

        if (newFile != File::nonexistent)
        {
            if (fillInNewCppFileTemplate (newFile, parent, "jucer_NewCppFileTemplate_cpp"))
                parent.addFile (newFile, 0);
            else
                showFailedToWriteMessage (newFile);
        }
    }
};

//==============================================================================
class NewHeaderFileWizard  : public NewFileWizard
{
public:
    NewHeaderFileWizard() {}
    ~NewHeaderFileWizard() {}

    const String getName()  { return "Header File"; }

    void createNewFile (Project::Item parent)
    {
        File newFile = askUserToChooseNewFile ("SourceCode.h", "*.h", parent);

        if (newFile != File::nonexistent)
        {
            if (fillInNewCppFileTemplate (newFile, parent, "jucer_NewCppFileTemplate_h"))
                parent.addFile (newFile, 0);
            else
                showFailedToWriteMessage (newFile);
        }
    }
};

//==============================================================================
class NewComponentWizard  : public NewFileWizard
{
public:
    NewComponentWizard() {}
    ~NewComponentWizard() {}

    const String getName()  { return "Component"; }

    void createNewFile (Project::Item parent)
    {
        File cppFile = askUserToChooseNewFile ("Component.cpp", "*.cpp", parent);

        if (cppFile != File::nonexistent)
        {
            File header (cppFile.withFileExtension (".h"));

            if (header.exists())
            {
                if (! AlertWindow::showOkCancelBox (AlertWindow::WarningIcon, "Create New Component",
                                                    "The file " + header.getFileName() + " already exists...\n\nDo you want to overwrite it?",
                                                    "Overwrite", "Cancel"))
                    return;
            }

            ComponentDocument doc (&parent.getProject(), cppFile);

            if (doc.save())
            {
                parent.addFile (header, 0);
                parent.addFile (cppFile, 0);
            }
            else
            {
                showFailedToWriteMessage (cppFile);
            }
        }
    }
};

//==============================================================================
class NewDrawableWizard  : public NewFileWizard
{
public:
    NewDrawableWizard() {}
    ~NewDrawableWizard() {}

    const String getName()  { return "Drawable"; }

    void createNewFile (Project::Item parent)
    {
        File newFile = askUserToChooseNewFile ("New Drawable.drawable", "*.drawable", parent);

        if (newFile != File::nonexistent)
        {
            DrawableDocument newDrawable (&(parent.getProject()), newFile);

            if (newDrawable.save())
                parent.addFile (newFile, 0);
            else
                showFailedToWriteMessage (newFile);
        }
    }
};

//==============================================================================
const StringArray NewFileWizard::getWizards()
{
    StringArray s;

    for (int i = 0; i < getNumWizards(); ++i)
    {
        ScopedPointer <NewFileWizard> wiz (createWizard (i));
        s.add (wiz->getName());
    }

    return s;
}

int NewFileWizard::getNumWizards()
{
    return 2;
}

NewFileWizard* NewFileWizard::createWizard (int index)
{
    switch (index)
    {
        case 0:     return new NewCppFileWizard();
        case 1:     return new NewHeaderFileWizard();
        case 2:     return new NewComponentWizard();
        case 3:     return new NewDrawableWizard();
        default:    jassertfalse; break;
    }

    return 0;
}

static const int menuBaseID = 0x12d83f0;

void NewFileWizard::addWizardsToMenu (PopupMenu& m)
{
    for (int i = 0; i < getNumWizards(); ++i)
    {
        ScopedPointer <NewFileWizard> wiz (createWizard (i));
        m.addItem (menuBaseID + i, "Add New " + wiz->getName() + "...");
    }
}

bool NewFileWizard::runWizardFromMenu (int chosenMenuItemID, const Project::Item& projectGroupToAddTo)
{
    int index = chosenMenuItemID - menuBaseID;

    if (index >= 0 && index < getNumWizards())
    {
        ScopedPointer <NewFileWizard> wiz (createWizard (index));

        wiz->createNewFile (projectGroupToAddTo);
        return true;
    }

    return false;
}

void NewFileWizard::showFailedToWriteMessage (const File& file)
{
    AlertWindow::showMessageBox (AlertWindow::WarningIcon,
                                 "Failed to Create File!",
                                 "Couldn't write to the file: " + file.getFullPathName());
}

const File NewFileWizard::askUserToChooseNewFile (const String& suggestedFilename, const String& wildcard,
                                                  const Project::Item& projectGroupToAddTo)
{
    FileChooser fc ("Select File to Create",
                    projectGroupToAddTo.determineGroupFolder()
                                       .getChildFile (suggestedFilename)
                                       .getNonexistentSibling(),
                    wildcard);

    if (fc.browseForFileToSave (true))
        return fc.getResult();

    return File::nonexistent;
}
