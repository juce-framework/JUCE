/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

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
class NewCppFileWizard final : public NewFileWizard::Type
{
public:
    String getName() override  { return "CPP File"; }

    void createNewFile (Project&, Project::Item parent) override
    {
        askUserToChooseNewFile ("SourceCode.cpp", "*.cpp", parent, [this, parent] (File newFile)
        {
            if (newFile != File())
                create (*this, parent, newFile, "jucer_NewCppFileTemplate_cpp");
        });
    }

    static bool create (NewFileWizard::Type& wizard, Project::Item parent, const File& newFile, const char* templateName)
    {
        if (fillInNewCppFileTemplate (newFile, parent, templateName))
        {
            parent.addFileRetainingSortOrder (newFile, true);
            return true;
        }

        wizard.showFailedToWriteMessage (newFile);
        return false;
    }
};

//==============================================================================
class NewHeaderFileWizard final : public NewFileWizard::Type
{
public:
    String getName() override  { return "Header File"; }

    void createNewFile (Project&, Project::Item parent) override
    {
        askUserToChooseNewFile ("SourceCode.h", "*.h", parent, [this, parent] (File newFile)
        {
            if (newFile != File())
                create (*this, parent, newFile, "jucer_NewCppFileTemplate_h");
        });
    }

    static bool create (NewFileWizard::Type& wizard, Project::Item parent, const File& newFile, const char* templateName)
    {
        if (fillInNewCppFileTemplate (newFile, parent, templateName))
        {
            parent.addFileRetainingSortOrder (newFile, true);
            return true;
        }

        wizard.showFailedToWriteMessage (newFile);
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
        askUserToChooseNewFile ("SourceCode.h", "*.h;*.cpp", parent, [this, parent] (File newFile)
        {
            if (newFile == File{})
                return;

            if (NewCppFileWizard::create (*this, parent, newFile.withFileExtension ("h"),   "jucer_NewCppFileTemplate_h"))
                NewCppFileWizard::create (*this, parent, newFile.withFileExtension ("cpp"), "jucer_NewCppFileTemplate_cpp");
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

    static bool create (NewFileWizard::Type& wizard, const String& className, Project::Item parent,
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

        wizard.showFailedToWriteMessage (newFile);
        return false;
    }

private:
    virtual void createFiles (Project::Item parent, const String& className, const File& newFile)
    {
        if (create (*this, className, parent, newFile.withFileExtension ("h"),   "jucer_NewComponentTemplate_h"))
            create (*this, className, parent, newFile.withFileExtension ("cpp"), "jucer_NewComponentTemplate_cpp");
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
class NewSingleFileComponentFileWizard final : public NewComponentFileWizard
{
public:
    String getName() override  { return "Component class (in a single source file)"; }

    void createFiles (Project::Item parent, const String& className, const File& newFile) override
    {
        create (*this, className, parent, newFile.withFileExtension ("h"), "jucer_NewInlineComponentTemplate_h");
    }
};

//==============================================================================
void NewFileWizard::Type::showFailedToWriteMessage (const File& file)
{
    auto options = MessageBoxOptions::makeOptionsOk (MessageBoxIconType::WarningIcon,
                                                     "Failed to Create File!",
                                                     "Couldn't write to the file: " + file.getFullPathName());
    messageBox = AlertWindow::showScopedAsync (options, nullptr);
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
        m.addItem (menuBaseID + i, "Add New " + wizards.getUnchecked (i)->getName() + "...");
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
