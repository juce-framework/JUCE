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

#include "jucer_NewProjectWizard.h"
#include "jucer_ProjectType.h"
#include "jucer_Module.h"
#include "../Project Saving/jucer_ProjectExporter.h"
#include "../Application/jucer_Application.h"
#include "../Application/jucer_MainWindow.h"

static void createFileCreationOptionComboBox (Component& setupComp,
                                              OwnedArray<Component>& itemsCreated,
                                              const char** fileOptions)
{
    ComboBox* c = new ComboBox();
    itemsCreated.add (c);
    setupComp.addChildAndSetID (c, "filesToCreate");

    c->addItemList (StringArray (fileOptions), 1);
    c->setSelectedId (1, false);

    Label* l = new Label (String::empty, "Files to Auto-Generate:");
    l->attachToComponent (c, true);
    itemsCreated.add (l);

    c->setBounds ("parent.width / 2 + 160, 10, parent.width - 10, top + 22");
}

static void setExecutableNameForAllTargets (Project& project, const String& exeName)
{
    for (Project::ExporterIterator exporter (project); exporter.next();)
        for (ProjectExporter::ConfigIterator config (*exporter); config.next();)
            config->getTargetBinaryName() = exeName;
}

//==============================================================================
class GUIAppWizard   : public NewProjectWizard
{
public:
    GUIAppWizard()  {}

    String getName()          { return "GUI Application"; }
    String getDescription()   { return "Creates a standard application"; }

    void addSetupItems (Component& setupComp, OwnedArray<Component>& itemsCreated)
    {
        const char* fileOptions[] = { "Create a Main.cpp file",
                                      "Create a Main.cpp file and a basic window",
                                      "Don't create any files", 0 };

        createFileCreationOptionComboBox (setupComp, itemsCreated, fileOptions);
    }

    Result processResultsFromSetupItems (Component& setupComp)
    {
        ComboBox* cb = dynamic_cast<ComboBox*> (setupComp.findChildWithID ("filesToCreate"));
        jassert (cb != nullptr);
        createMainCpp = createWindow = false;

        switch (cb->getSelectedItemIndex())
        {
            case 0:     createMainCpp = true;  break;
            case 1:     createMainCpp = createWindow = true;  break;
            case 2:     break;
            default:    jassertfalse; break;
        }

        return Result::ok();
    }

    bool initialiseProject (Project& project)
    {
        if (! getSourceFilesFolder().createDirectory())
            failedFiles.add (getSourceFilesFolder().getFullPathName());

        File mainCppFile = getSourceFilesFolder().getChildFile ("Main.cpp");
        File mainWindowCpp = getSourceFilesFolder().getChildFile ("MainWindow.cpp");
        File mainWindowH = mainWindowCpp.withFileExtension (".h");
        String windowClassName = "MainAppWindow";

        project.getProjectTypeValue() = ProjectType::getGUIAppTypeName();

        Project::Item sourceGroup (project.getMainGroup().addNewSubGroup ("Source", 0));

        setExecutableNameForAllTargets (project, File::createLegalFileName (appTitle));

        String appHeaders (CodeHelpers::createIncludeStatement (project.getAppIncludeFile(), mainCppFile));
        String initCode, shutdownCode, anotherInstanceStartedCode, privateMembers, memberInitialisers;

        if (createWindow)
        {
            appHeaders << newLine << CodeHelpers::createIncludeStatement (mainWindowH, mainCppFile);
            initCode       = "mainWindow = new " + windowClassName + "();";
            shutdownCode   = "mainWindow = nullptr;";
            privateMembers = "ScopedPointer <" + windowClassName + "> mainWindow;";

            String windowH = project.getFileTemplate ("jucer_WindowTemplate_h")
                                .replace ("INCLUDE_JUCE", CodeHelpers::createIncludeStatement (project.getAppIncludeFile(), mainWindowH), false)
                                .replace ("WINDOWCLASS", windowClassName, false)
                                .replace ("HEADERGUARD", CodeHelpers::makeHeaderGuardName (mainWindowH), false);

            String windowCpp = project.getFileTemplate ("jucer_WindowTemplate_cpp")
                                .replace ("INCLUDE_JUCE", CodeHelpers::createIncludeStatement (project.getAppIncludeFile(), mainWindowCpp), false)
                                .replace ("INCLUDE_CORRESPONDING_HEADER", CodeHelpers::createIncludeStatement (mainWindowH, mainWindowCpp), false)
                                .replace ("WINDOWCLASS", windowClassName, false);

            if (! FileHelpers::overwriteFileWithNewDataIfDifferent (mainWindowH, windowH))
                failedFiles.add (mainWindowH.getFullPathName());

            if (! FileHelpers::overwriteFileWithNewDataIfDifferent (mainWindowCpp, windowCpp))
                failedFiles.add (mainWindowCpp.getFullPathName());

            sourceGroup.addFile (mainWindowCpp, -1, true);
            sourceGroup.addFile (mainWindowH, -1, false);
        }

        if (createMainCpp)
        {
            String mainCpp = project.getFileTemplate ("jucer_MainTemplate_cpp")
                                .replace ("APPHEADERS", appHeaders, false)
                                .replace ("APPCLASSNAME", CodeHelpers::makeValidIdentifier (appTitle + "Application", false, true, false), false)
                                .replace ("MEMBERINITIALISERS", memberInitialisers, false)
                                .replace ("APPINITCODE", initCode, false)
                                .replace ("APPSHUTDOWNCODE", shutdownCode, false)
                                .replace ("APPNAME", CodeHelpers::addEscapeChars (appTitle), false)
                                .replace ("APPVERSION", "1.0", false)
                                .replace ("ALLOWMORETHANONEINSTANCE", "true", false)
                                .replace ("ANOTHERINSTANCECODE", anotherInstanceStartedCode, false)
                                .replace ("PRIVATEMEMBERS", privateMembers, false);

            if (! FileHelpers::overwriteFileWithNewDataIfDifferent (mainCppFile, mainCpp))
                failedFiles.add (mainCppFile.getFullPathName());

            sourceGroup.addFile (mainCppFile, -1, true);
        }

        project.createExporterForCurrentPlatform();

        return true;
    }

private:
    bool createMainCpp, createWindow;
};

//==============================================================================
class ConsoleAppWizard   : public NewProjectWizard
{
public:
    ConsoleAppWizard()  {}

    String getName()          { return "Console Application"; }
    String getDescription()   { return "Creates a command-line application with no GUI features"; }

    void addSetupItems (Component& setupComp, OwnedArray<Component>& itemsCreated)
    {
        const char* fileOptions[] = { "Create a Main.cpp file",
                                      "Don't create any files", 0 };

        createFileCreationOptionComboBox (setupComp, itemsCreated, fileOptions);
    }

    Result processResultsFromSetupItems (Component& setupComp)
    {
        ComboBox* cb = dynamic_cast<ComboBox*> (setupComp.findChildWithID ("filesToCreate"));
        jassert (cb != nullptr);

        createMainCpp = false;

        switch (cb->getSelectedItemIndex())
        {
            case 0:     createMainCpp = true;  break;
            case 1:     break;
            default:    jassertfalse; break;
        }

        return Result::ok();
    }

    bool initialiseProject (Project& project)
    {
        if (! getSourceFilesFolder().createDirectory())
            failedFiles.add (getSourceFilesFolder().getFullPathName());

        File mainCppFile = getSourceFilesFolder().getChildFile ("Main.cpp");

        project.getProjectTypeValue() = ProjectType::getConsoleAppTypeName();

        Project::Item sourceGroup (project.getMainGroup().addNewSubGroup ("Source", 0));

        setExecutableNameForAllTargets (project, File::createLegalFileName (appTitle));

        if (createMainCpp)
        {
            String appHeaders (CodeHelpers::createIncludeStatement (project.getAppIncludeFile(), mainCppFile));

            String mainCpp = project.getFileTemplate ("jucer_MainConsoleAppTemplate_cpp")
                                .replace ("APPHEADERS", appHeaders, false);

            if (! FileHelpers::overwriteFileWithNewDataIfDifferent (mainCppFile, mainCpp))
                failedFiles.add (mainCppFile.getFullPathName());

            sourceGroup.addFile (mainCppFile, -1, true);
        }

        project.createExporterForCurrentPlatform();

        return true;
    }

private:
    bool createMainCpp;
};

//==============================================================================
class AudioPluginAppWizard   : public NewProjectWizard
{
public:
    AudioPluginAppWizard()  {}

    String getName()          { return "Audio Plug-In"; }
    String getDescription()   { return "Creates an audio plugin project"; }

    void addSetupItems (Component& setupComp, OwnedArray<Component>& itemsCreated)
    {
    }

    Result processResultsFromSetupItems (Component& setupComp)
    {
        return Result::ok();
    }

    bool initialiseProject (Project& project)
    {
        if (! getSourceFilesFolder().createDirectory())
            failedFiles.add (getSourceFilesFolder().getFullPathName());

        String filterClassName = CodeHelpers::makeValidIdentifier (appTitle, true, true, false) + "AudioProcessor";
        filterClassName = filterClassName.substring (0, 1).toUpperCase() + filterClassName.substring (1);
        String editorClassName = filterClassName + "Editor";

        File filterCppFile = getSourceFilesFolder().getChildFile ("PluginProcessor.cpp");
        File filterHFile   = filterCppFile.withFileExtension (".h");
        File editorCppFile = getSourceFilesFolder().getChildFile ("PluginEditor.cpp");
        File editorHFile   = editorCppFile.withFileExtension (".h");

        project.getProjectTypeValue() = ProjectType::getAudioPluginTypeName();
        project.addModule ("juce_audio_plugin_client", true);

        Project::Item sourceGroup (project.getMainGroup().addNewSubGroup ("Source", 0));
        project.getConfigFlag ("JUCE_QUICKTIME") = Project::configFlagDisabled; // disabled because it interferes with RTAS build on PC

        setExecutableNameForAllTargets (project, File::createLegalFileName (appTitle));

        String appHeaders (CodeHelpers::createIncludeStatement (project.getAppIncludeFile(), filterCppFile));

        String filterCpp = project.getFileTemplate ("jucer_AudioPluginFilterTemplate_cpp")
                            .replace ("FILTERHEADERS", CodeHelpers::createIncludeStatement (filterHFile, filterCppFile)
                                                            + newLine + CodeHelpers::createIncludeStatement (editorHFile, filterCppFile), false)
                            .replace ("FILTERCLASSNAME", filterClassName, false)
                            .replace ("EDITORCLASSNAME", editorClassName, false);

        String filterH = project.getFileTemplate ("jucer_AudioPluginFilterTemplate_h")
                            .replace ("APPHEADERS", appHeaders, false)
                            .replace ("FILTERCLASSNAME", filterClassName, false)
                            .replace ("HEADERGUARD", CodeHelpers::makeHeaderGuardName (filterHFile), false);

        String editorCpp = project.getFileTemplate ("jucer_AudioPluginEditorTemplate_cpp")
                            .replace ("EDITORCPPHEADERS", CodeHelpers::createIncludeStatement (filterHFile, filterCppFile)
                                                               + newLine + CodeHelpers::createIncludeStatement (editorHFile, filterCppFile), false)
                            .replace ("FILTERCLASSNAME", filterClassName, false)
                            .replace ("EDITORCLASSNAME", editorClassName, false);

        String editorH = project.getFileTemplate ("jucer_AudioPluginEditorTemplate_h")
                            .replace ("EDITORHEADERS", appHeaders + newLine + CodeHelpers::createIncludeStatement (filterHFile, filterCppFile), false)
                            .replace ("FILTERCLASSNAME", filterClassName, false)
                            .replace ("EDITORCLASSNAME", editorClassName, false)
                            .replace ("HEADERGUARD", CodeHelpers::makeHeaderGuardName (editorHFile), false);

        if (! FileHelpers::overwriteFileWithNewDataIfDifferent (filterCppFile, filterCpp))
            failedFiles.add (filterCppFile.getFullPathName());

        if (! FileHelpers::overwriteFileWithNewDataIfDifferent (filterHFile, filterH))
            failedFiles.add (filterHFile.getFullPathName());

        if (! FileHelpers::overwriteFileWithNewDataIfDifferent (editorCppFile, editorCpp))
            failedFiles.add (editorCppFile.getFullPathName());

        if (! FileHelpers::overwriteFileWithNewDataIfDifferent (editorHFile, editorH))
            failedFiles.add (editorHFile.getFullPathName());

        sourceGroup.addFile (filterCppFile, -1, true);
        sourceGroup.addFile (filterHFile, -1, false);
        sourceGroup.addFile (editorCppFile, -1, true);
        sourceGroup.addFile (editorHFile, -1, false);

        project.createExporterForCurrentPlatform();

        return true;
    }
};

//==============================================================================
//==============================================================================
NewProjectWizard::NewProjectWizard() {}
NewProjectWizard::~NewProjectWizard() {}

StringArray NewProjectWizard::getWizards()
{
    StringArray s;

    for (int i = 0; i < getNumWizards(); ++i)
    {
        ScopedPointer <NewProjectWizard> wiz (createWizard (i));
        s.add (wiz->getName());
    }

    return s;
}

int NewProjectWizard::getNumWizards()
{
    return 3;
}

NewProjectWizard* NewProjectWizard::createWizard (int index)
{
    switch (index)
    {
        case 0:     return new GUIAppWizard();
        case 1:     return new ConsoleAppWizard();
        case 2:     return new AudioPluginAppWizard();
        //case 3:     return new BrowserPluginAppWizard();
        default:    jassertfalse; break;
    }

    return 0;
}

File& NewProjectWizard::getLastWizardFolder()
{
   #if JUCE_WINDOWS
    static File lastFolder (File::getSpecialLocation (File::userDocumentsDirectory));
   #else
    static File lastFolder (File::getSpecialLocation (File::userHomeDirectory));
   #endif
    return lastFolder;
}

//==============================================================================
Project* NewProjectWizard::runWizard (Component* ownerWindow_,
                                      const String& projectName,
                                      const File& targetFolder_)
{
    ownerWindow = ownerWindow_;
    appTitle = projectName;
    targetFolder = targetFolder_;

    if (! targetFolder.exists())
    {
        if (! targetFolder.createDirectory())
            failedFiles.add (targetFolder.getFullPathName());
    }
    else if (FileHelpers::containsAnyNonHiddenFiles (targetFolder))
    {
        if (! AlertWindow::showOkCancelBox (AlertWindow::InfoIcon, "New Juce Project",
                                            "The folder you chose isn't empty - are you sure you want to create the project there?\n\nAny existing files with the same names may be overwritten by the new files."))
            return nullptr;
    }

    projectFile = targetFolder.getChildFile (File::createLegalFileName (appTitle))
                              .withFileExtension (Project::projectFileExtension);

    ScopedPointer<Project> project (new Project (projectFile));
    project->addDefaultModules (true);

    if (failedFiles.size() == 0)
    {
        project->setFile (projectFile);
        project->setTitle (appTitle);
        project->getBundleIdentifier() = project->getDefaultBundleIdentifier();

        if (! initialiseProject (*project))
            return nullptr;

        if (project->save (false, true) != FileBasedDocument::savedOk)
            return nullptr;

        project->setChangedFlag (false);
    }

    if (failedFiles.size() > 0)
    {
        AlertWindow::showMessageBox (AlertWindow::WarningIcon,
                                     "Errors in Creating Project!",
                                     "The following files couldn't be written:\n\n"
                                        + failedFiles.joinIntoString ("\n", 0, 10));
        return nullptr;
    }

    return project.release();
}

//==============================================================================
class NewProjectWizard::WizardComp  : public Component,
                                      private ButtonListener,
                                      private ComboBoxListener,
                                      private TextEditorListener
{
public:
    WizardComp()
        : projectName ("Project name"),
          nameLabel (String::empty, "Project Name:"),
          typeLabel (String::empty, "Project Type:"),
          fileBrowser (FileBrowserComponent::saveMode | FileBrowserComponent::canSelectDirectories,
                       getLastWizardFolder(), nullptr, nullptr),
          fileOutline (String::empty, "Project Folder:"),
          createButton ("Create..."),
          cancelButton ("Cancel")
    {
        setOpaque (true);
        setSize (600, 500);

        addChildAndSetID (&projectName, "projectName");
        projectName.setText ("NewProject");
        projectName.setBounds ("100, 14, parent.width / 2 - 10, top + 22");
        nameLabel.attachToComponent (&projectName, true);
        projectName.addListener (this);

        addChildAndSetID (&projectType, "projectType");
        projectType.addItemList (getWizards(), 1);
        projectType.setSelectedId (1, true);
        projectType.setBounds ("100, projectName.bottom + 4, projectName.right, top + 22");
        typeLabel.attachToComponent (&projectType, true);
        projectType.addListener (this);

        addChildAndSetID (&fileOutline, "fileOutline");
        fileOutline.setColour (GroupComponent::outlineColourId, Colours::black.withAlpha (0.2f));
        fileOutline.setTextLabelPosition (Justification::centred);
        fileOutline.setBounds ("10, projectType.bottom + 20, projectType.right, parent.height - 10");

        addChildAndSetID (&fileBrowser, "fileBrowser");
        fileBrowser.setBounds ("fileOutline.left + 10, fileOutline.top + 20, fileOutline.right - 10, fileOutline.bottom - 12");
        fileBrowser.setFilenameBoxLabel ("Folder:");

        addChildAndSetID (&createButton, "createButton");
        createButton.setBounds ("right - 140, bottom - 24, parent.width - 10, parent.height - 10");
        createButton.addListener (this);

        addChildAndSetID (&cancelButton, "cancelButton");
        cancelButton.setBounds ("right - 140, createButton.top, createButton.left - 10, createButton.bottom");
        cancelButton.addListener (this);

        updateCustomItems();
        updateCreateButton();
    }

    void paint (Graphics& g)
    {
        g.fillAll (Colour::greyLevel (0.93f));
    }

    void buttonClicked (Button* b)
    {
        if (b == &createButton)
        {
            createProject();
        }
        else
        {
            MainWindow* mw = dynamic_cast<MainWindow*> (getTopLevelComponent());
            jassert (mw != nullptr);

            IntrojucerApp::getApp().mainWindowList.closeWindow (mw);
        }
    }

    void createProject()
    {
        MainWindow* mw = Component::findParentComponentOfClass<MainWindow>();
        jassert (mw != nullptr);

        ScopedPointer <NewProjectWizard> wizard (createWizard());

        if (wizard != nullptr)
        {
            Result result (wizard->processResultsFromSetupItems (*this));

            if (result.failed())
            {
                AlertWindow::showMessageBox (AlertWindow::WarningIcon, "Create Project", result.getErrorMessage());
                return;
            }

            ScopedPointer<Project> project (wizard->runWizard (mw, projectName.getText(),
                                                               fileBrowser.getSelectedFile (0)));

            if (project != nullptr)
                mw->setProject (project.release());
        }
    }

    void updateCustomItems()
    {
        customItems.clear();

        ScopedPointer <NewProjectWizard> wizard (createWizard());

        if (wizard != nullptr)
            wizard->addSetupItems (*this, customItems);
    }

    void comboBoxChanged (ComboBox*)
    {
        updateCustomItems();
    }

    void textEditorTextChanged (TextEditor&)
    {
        updateCreateButton();

        fileBrowser.setFileName (File::createLegalFileName (projectName.getText()));
    }

private:
    ComboBox projectType;
    TextEditor projectName;
    Label nameLabel, typeLabel;
    FileBrowserComponent fileBrowser;
    GroupComponent fileOutline;
    TextButton createButton, cancelButton;
    OwnedArray<Component> customItems;

    NewProjectWizard* createWizard()
    {
        return NewProjectWizard::createWizard (projectType.getSelectedItemIndex());
    }

    void updateCreateButton()
    {
        createButton.setEnabled (projectName.getText().trim().isNotEmpty());
    }
};

Component* NewProjectWizard::createComponent()
{
    return new WizardComp();
}
