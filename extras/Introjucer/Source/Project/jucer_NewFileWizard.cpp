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

#include "jucer_NewFileWizard.h"


//==============================================================================
namespace
{
    bool fillInNewCppFileTemplate (const File& file, const Project::Item& item, const char* templateName)
    {
        String s = item.project.getFileTemplate (templateName)
                      .replace ("FILENAME", file.getFileName(), false)
                      .replace ("DATE", Time::getCurrentTime().toString (true, true, true), false)
                      .replace ("AUTHOR", SystemStats::getFullUserName(), false)
                      .replace ("HEADERGUARD", CodeHelpers::makeHeaderGuardName (file), false);

        return FileHelpers::overwriteFileWithNewDataIfDifferent (file, s);
    }

    const int menuBaseID = 0x12d83f0;
}


//==============================================================================
class NewCppFileWizard  : public NewFileWizard::Type
{
public:
    NewCppFileWizard() {}

    String getName()  { return "CPP File"; }

    void createNewFile (Project::Item parent)
    {
        const File newFile (askUserToChooseNewFile ("SourceCode.cpp", "*.cpp", parent));

        if (newFile != File::nonexistent)
            create (parent, newFile);
    }

    static bool create (Project::Item parent, const File& newFile)
    {
        if (fillInNewCppFileTemplate (newFile, parent, "jucer_NewCppFileTemplate_cpp"))
        {
            parent.addFile (newFile, 0, true);
            return true;
        }

        showFailedToWriteMessage (newFile);
        return false;
    }
};

//==============================================================================
class NewHeaderFileWizard  : public NewFileWizard::Type
{
public:
    NewHeaderFileWizard() {}

    String getName()  { return "Header File"; }

    void createNewFile (Project::Item parent)
    {
        const File newFile (askUserToChooseNewFile ("SourceCode.h", "*.h", parent));

        if (newFile != File::nonexistent)
            create (parent, newFile);
    }

    static bool create (Project::Item parent, const File& newFile)
    {
        if (fillInNewCppFileTemplate (newFile, parent, "jucer_NewCppFileTemplate_h"))
        {
            parent.addFile (newFile, 0, true);
            return true;
        }

        showFailedToWriteMessage (newFile);
        return false;
    }
};

//==============================================================================
class NewCppAndHeaderFileWizard  : public NewFileWizard::Type
{
public:
    NewCppAndHeaderFileWizard() {}

    String getName()  { return "CPP & Header File"; }

    void createNewFile (Project::Item parent)
    {
        const File newFile (askUserToChooseNewFile ("SourceCode.h", "*.h;*.cpp", parent));

        if (newFile != File::nonexistent)
        {
            if (NewHeaderFileWizard::create (parent, newFile.withFileExtension ("h")))
                NewCppFileWizard::create (parent, newFile.withFileExtension ("cpp"));
        }
    }
};

//==============================================================================
void NewFileWizard::Type::showFailedToWriteMessage (const File& file)
{
    AlertWindow::showMessageBox (AlertWindow::WarningIcon,
                                 "Failed to Create File!",
                                 "Couldn't write to the file: " + file.getFullPathName());
}

File NewFileWizard::Type::askUserToChooseNewFile (const String& suggestedFilename, const String& wildcard,
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

//==============================================================================
NewFileWizard::NewFileWizard()
{
    registerWizard (new NewCppFileWizard());
    registerWizard (new NewHeaderFileWizard());
    registerWizard (new NewCppAndHeaderFileWizard());
}

NewFileWizard::~NewFileWizard()
{
    clearSingletonInstance();
}

juce_ImplementSingleton_SingleThreaded (NewFileWizard)

void NewFileWizard::addWizardsToMenu (PopupMenu& m) const
{
    for (int i = 0; i < wizards.size(); ++i)
        m.addItem (menuBaseID + i, "Add New " + wizards.getUnchecked(i)->getName() + "...");
}

bool NewFileWizard::runWizardFromMenu (int chosenMenuItemID, const Project::Item& projectGroupToAddTo) const
{
    Type* wiz = wizards [chosenMenuItemID - menuBaseID];

    if (wiz != nullptr)
    {
        wiz->createNewFile (projectGroupToAddTo);
        return true;
    }

    return false;
}

void NewFileWizard::registerWizard (Type* newWizard)
{
    wizards.add (newWizard);
}
