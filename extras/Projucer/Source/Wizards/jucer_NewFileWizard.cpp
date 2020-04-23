/*
  ==============================================================================

   This file is part of the JUCE 6 technical preview.
   Copyright (c) 2020 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For this technical preview, this file is not subject to commercial licensing.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#include "../Application/jucer_Headers.h"
#include "jucer_NewFileWizard.h"

//==============================================================================
namespace
{
    static String fillInBasicTemplateFields (const File& file, const Project::Item& item, const char* templateName)
    {
        return replaceLineFeeds (item.project.getFileTemplate (templateName)
                                             .replace ("%%filename%%", file.getFileName(), false)
                                             .replace ("%%date%%", Time::getCurrentTime().toString (true, true, true), false)
                                             .replace ("%%author%%", SystemStats::getFullUserName(), false)
                                             .replace ("%%include_corresponding_header%%", CodeHelpers::createIncludeStatement (file.withFileExtension (".h"), file)),
                                 item.project.getProjectLineFeed());
    }

    static bool fillInNewCppFileTemplate (const File& file, const Project::Item& item, const char* templateName)
    {
        return build_tools::overwriteFileWithNewDataIfDifferent (file, fillInBasicTemplateFields (file, item, templateName));
    }

    const int menuBaseID = 0x12d83f0;
}

//==============================================================================
class NewCppFileWizard  : public NewFileWizard::Type
{
public:
    NewCppFileWizard() {}

    String getName() override  { return "CPP File"; }

    void createNewFile (Project&, Project::Item parent) override
    {
        const File newFile (askUserToChooseNewFile ("SourceCode.cpp", "*.cpp", parent));

        if (newFile != File())
            create (parent, newFile, "jucer_NewCppFileTemplate_cpp");
    }

    static bool create (Project::Item parent, const File& newFile, const char* templateName)
    {
        if (fillInNewCppFileTemplate (newFile, parent, templateName))
        {
            parent.addFileRetainingSortOrder (newFile, true);
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

    String getName() override  { return "Header File"; }

    void createNewFile (Project&, Project::Item parent) override
    {
        const File newFile (askUserToChooseNewFile ("SourceCode.h", "*.h", parent));

        if (newFile != File())
            create (parent, newFile, "jucer_NewCppFileTemplate_h");
    }

    static bool create (Project::Item parent, const File& newFile, const char* templateName)
    {
        if (fillInNewCppFileTemplate (newFile, parent, templateName))
        {
            parent.addFileRetainingSortOrder (newFile, true);
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

    String getName() override  { return "CPP & Header File"; }

    void createNewFile (Project&, Project::Item parent) override
    {
        const File newFile (askUserToChooseNewFile ("SourceCode.h", "*.h;*.cpp", parent));

        if (newFile != File())
        {
            if (NewCppFileWizard::create (parent, newFile.withFileExtension ("h"),   "jucer_NewCppFileTemplate_h"))
                NewCppFileWizard::create (parent, newFile.withFileExtension ("cpp"), "jucer_NewCppFileTemplate_cpp");
        }
    }
};

//==============================================================================
class NewComponentFileWizard  : public NewFileWizard::Type
{
public:
    NewComponentFileWizard() {}

    String getName() override  { return "Component class (split between a CPP & header)"; }

    void createNewFile (Project&, Project::Item parent) override
    {
        for (;;)
        {
            AlertWindow aw (TRANS ("Create new Component class"),
                            TRANS ("Please enter the name for the new class"),
                            AlertWindow::NoIcon, nullptr);

            aw.addTextEditor (getClassNameFieldName(), String(), String(), false);
            aw.addButton (TRANS ("Create Files"),  1, KeyPress (KeyPress::returnKey));
            aw.addButton (TRANS ("Cancel"),        0, KeyPress (KeyPress::escapeKey));

            if (aw.runModalLoop() == 0)
                break;

            const String className (aw.getTextEditorContents (getClassNameFieldName()).trim());

            if (className == build_tools::makeValidIdentifier (className, false, true, false))
            {
                const File newFile (askUserToChooseNewFile (className + ".h", "*.h;*.cpp", parent));

                if (newFile != File())
                    createFiles (parent, className, newFile);

                break;
            }
        }
    }

    static bool create (const String& className, Project::Item parent,
                        const File& newFile, const char* templateName)
    {
        auto content = fillInBasicTemplateFields (newFile, parent, templateName)
                           .replace ("%%component_class%%", className)
                           .replace ("%%include_juce%%", CodeHelpers::createIncludePathIncludeStatement (Project::getJuceSourceHFilename()));

        content = replaceLineFeeds (content, parent.project.getProjectLineFeed());

        if (build_tools::overwriteFileWithNewDataIfDifferent (newFile, content))
        {
            parent.addFileRetainingSortOrder (newFile, true);
            return true;
        }

        showFailedToWriteMessage (newFile);
        return false;
    }

private:
    virtual void createFiles (Project::Item parent, const String& className, const File& newFile)
    {
        if (create (className, parent, newFile.withFileExtension ("h"),   "jucer_NewComponentTemplate_h"))
            create (className, parent, newFile.withFileExtension ("cpp"), "jucer_NewComponentTemplate_cpp");
    }

    static String getClassNameFieldName()  { return "Class Name"; }
};

//==============================================================================
class NewSingleFileComponentFileWizard  : public NewComponentFileWizard
{
public:
    NewSingleFileComponentFileWizard() {}

    String getName() override  { return "Component class (in a single source file)"; }

    void createFiles (Project::Item parent, const String& className, const File& newFile) override
    {
        create (className, parent, newFile.withFileExtension ("h"), "jucer_NewInlineComponentTemplate_h");
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

    return {};
}

//==============================================================================
NewFileWizard::NewFileWizard()
{
    registerWizard (new NewCppFileWizard());
    registerWizard (new NewHeaderFileWizard());
    registerWizard (new NewCppAndHeaderFileWizard());
    registerWizard (new NewComponentFileWizard());
    registerWizard (new NewSingleFileComponentFileWizard());
}

NewFileWizard::~NewFileWizard()
{
}

void NewFileWizard::addWizardsToMenu (PopupMenu& m) const
{
    for (int i = 0; i < wizards.size(); ++i)
        m.addItem (menuBaseID + i, "Add New " + wizards.getUnchecked(i)->getName() + "...");
}

bool NewFileWizard::runWizardFromMenu (int chosenMenuItemID, Project& project, const Project::Item& projectGroupToAddTo) const
{
    if (Type* wiz = wizards [chosenMenuItemID - menuBaseID])
    {
        wiz->createNewFile (project, projectGroupToAddTo);
        return true;
    }

    return false;
}

void NewFileWizard::registerWizard (Type* newWizard)
{
    wizards.add (newWizard);
}
