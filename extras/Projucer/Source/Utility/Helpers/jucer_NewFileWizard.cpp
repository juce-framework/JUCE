/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#include "../../Application/jucer_Headers.h"
#include "jucer_NewFileWizard.h"

//==============================================================================
namespace
{
    static String fillInBasicTemplateFields (const File& file, const Project::Item& item, const char* templateName)
    {
        int dataSize;

        if (auto* data = BinaryData::getNamedResource (templateName, dataSize))
        {
            auto fileTemplate = String::fromUTF8 (data, dataSize);

            return replaceLineFeeds (fileTemplate.replace ("%%filename%%", file.getFileName(), false)
                                                 .replace ("%%date%%", Time::getCurrentTime().toString (true, true, true), false)
                                                 .replace ("%%author%%", SystemStats::getFullUserName(), false)
                                                 .replace ("%%include_corresponding_header%%", CodeHelpers::createIncludeStatement (file.withFileExtension (".h"), file)),
                                     item.project.getProjectLineFeed());
        }

        jassertfalse;
        return {};
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
    String getName() override  { return "CPP File"; }

    void createNewFile (Project&, Project::Item parent) override
    {
        askUserToChooseNewFile ("SourceCode.cpp", "*.cpp", parent, [parent] (File newFile)
        {
            if (newFile != File())
                create (parent, newFile, "jucer_NewCppFileTemplate_cpp");
        });
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
    String getName() override  { return "Header File"; }

    void createNewFile (Project&, Project::Item parent) override
    {
        askUserToChooseNewFile ("SourceCode.h", "*.h", parent, [parent] (File newFile)
        {
            if (newFile != File())
                create (parent, newFile, "jucer_NewCppFileTemplate_h");
        });
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
    String getName() override  { return "CPP & Header File"; }

    void createNewFile (Project&, Project::Item parent) override
    {
        askUserToChooseNewFile ("SourceCode.h", "*.h;*.cpp", parent, [parent] (File newFile)
        {
            if (NewCppFileWizard::create (parent, newFile.withFileExtension ("h"),   "jucer_NewCppFileTemplate_h"))
                NewCppFileWizard::create (parent, newFile.withFileExtension ("cpp"), "jucer_NewCppFileTemplate_cpp");
        });
    }
};

//==============================================================================
class NewComponentFileWizard  : public NewFileWizard::Type
{
public:
    String getName() override  { return "Component class (split between a CPP & header)"; }

    void createNewFile (Project&, Project::Item parent) override
    {
        createNewFileInternal (parent);
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

    void createNewFileInternal (Project::Item parent)
    {
        asyncAlertWindow = std::make_unique<AlertWindow> (TRANS ("Create new Component class"),
                                                          TRANS ("Please enter the name for the new class"),
                                                          MessageBoxIconType::NoIcon, nullptr);

        asyncAlertWindow->addTextEditor (getClassNameFieldName(), String(), String(), false);
        asyncAlertWindow->addButton (TRANS ("Create Files"),  1, KeyPress (KeyPress::returnKey));
        asyncAlertWindow->addButton (TRANS ("Cancel"),        0, KeyPress (KeyPress::escapeKey));

        auto resultCallback = [safeThis = WeakReference<NewComponentFileWizard> { this }, parent] (int result)
        {
            if (safeThis == nullptr)
                return;

            auto& aw = *(safeThis->asyncAlertWindow);

            aw.exitModalState (result);
            aw.setVisible (false);

            if (result == 0)
                return;

            const String className (aw.getTextEditorContents (getClassNameFieldName()).trim());

            if (className == build_tools::makeValidIdentifier (className, false, true, false))
            {
                safeThis->askUserToChooseNewFile (className + ".h", "*.h;*.cpp",
                                                  parent,
                                                  [safeThis, parent, className] (File newFile)
                                                  {
                                                      if (safeThis == nullptr)
                                                          return;

                                                      if (newFile != File())
                                                          safeThis->createFiles (parent, className, newFile);
                                                  });

                return;
            }

            safeThis->createNewFileInternal (parent);
        };

        asyncAlertWindow->enterModalState (true, ModalCallbackFunction::create (std::move (resultCallback)), false);
    }

    std::unique_ptr<AlertWindow> asyncAlertWindow;

    JUCE_DECLARE_WEAK_REFERENCEABLE (NewComponentFileWizard)
};

//==============================================================================
class NewSingleFileComponentFileWizard  : public NewComponentFileWizard
{
public:
    String getName() override  { return "Component class (in a single source file)"; }

    void createFiles (Project::Item parent, const String& className, const File& newFile) override
    {
        create (className, parent, newFile.withFileExtension ("h"), "jucer_NewInlineComponentTemplate_h");
    }
};


//==============================================================================
void NewFileWizard::Type::showFailedToWriteMessage (const File& file)
{
    AlertWindow::showMessageBoxAsync (MessageBoxIconType::WarningIcon,
                                      "Failed to Create File!",
                                      "Couldn't write to the file: " + file.getFullPathName());
}

void NewFileWizard::Type::askUserToChooseNewFile (const String& suggestedFilename, const String& wildcard,
                                                  const Project::Item& projectGroupToAddTo,
                                                  std::function<void (File)> callback)
{
    chooser = std::make_unique<FileChooser> ("Select File to Create",
                                             projectGroupToAddTo.determineGroupFolder()
                                                                .getChildFile (suggestedFilename)
                                                                .getNonexistentSibling(),
                                             wildcard);
    auto flags = FileBrowserComponent::saveMode
               | FileBrowserComponent::canSelectFiles
               | FileBrowserComponent::warnAboutOverwriting;

    chooser->launchAsync (flags, [callback] (const FileChooser& fc)
    {
        callback (fc.getResult());
    });
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
