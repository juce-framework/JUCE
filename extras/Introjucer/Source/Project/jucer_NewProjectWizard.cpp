/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

#include "jucer_NewProjectWizard.h"
#include "jucer_ProjectType.h"
#include "jucer_Module.h"
#include "../Project Saving/jucer_ProjectExporter.h"
#include "../Application/jucer_Application.h"
#include "../Application/jucer_MainWindow.h"


struct NewProjectWizardClasses
{
    //==============================================================================
    static void createFileCreationOptionComboBox (Component& setupComp,
                                                  OwnedArray<Component>& itemsCreated,
                                                  const StringArray& fileOptions)
    {
        ComboBox* c = new ComboBox();
        itemsCreated.add (c);
        setupComp.addChildAndSetID (c, "filesToCreate");

        c->addItemList (fileOptions, 1);
        c->setSelectedId (1, dontSendNotification);

        Label* l = new Label (String::empty, TRANS("Files to Auto-Generate") + ":");
        l->attachToComponent (c, true);
        itemsCreated.add (l);

        c->setBounds ("parent.width / 2 + 160, 10, parent.width - 10, top + 22");
    }

    static int getFileCreationComboResult (Component& setupComp)
    {
        if (ComboBox* cb = dynamic_cast<ComboBox*> (setupComp.findChildWithID ("filesToCreate")))
            return cb->getSelectedItemIndex();

        jassertfalse;
        return 0;
    }

    static void setExecutableNameForAllTargets (Project& project, const String& exeName)
    {
        for (Project::ExporterIterator exporter (project); exporter.next();)
            for (ProjectExporter::ConfigIterator config (*exporter); config.next();)
                config->getTargetBinaryName() = exeName;
    }

    static Project::Item createSourceGroup (Project& project)
    {
        return project.getMainGroup().addNewSubGroup ("Source", 0);
    }

    static File& getLastWizardFolder()
    {
       #if JUCE_WINDOWS
        static File lastFolder (File::getSpecialLocation (File::userDocumentsDirectory));
       #else
        static File lastFolder (File::getSpecialLocation (File::userHomeDirectory));
       #endif

        return lastFolder;
    }

    static bool isJuceModulesFolder (const File& f)
    {
        return f.isDirectory()
                && f.getChildFile ("juce_core").isDirectory();
    }

    static File findDefaultModulesFolder (bool mustContainJuceCoreModule = true)
    {
        const MainWindowList& windows = IntrojucerApp::getApp().mainWindowList;

        for (int i = windows.windows.size(); --i >= 0;)
        {
            if (Project* p = windows.windows.getUnchecked (i)->getProject())
            {
                const File f (EnabledModuleList::findDefaultModulesFolder (*p));

                if (isJuceModulesFolder (f) || (f.isDirectory() && ! mustContainJuceCoreModule))
                    return f;
            }
        }

        if (mustContainJuceCoreModule)
            return findDefaultModulesFolder (false);

        return File::nonexistent;
    }

    //==============================================================================
    struct NewProjectWizard
    {
        NewProjectWizard() {}
        virtual ~NewProjectWizard() {}

        //==============================================================================
        virtual String getName() = 0;
        virtual String getDescription() = 0;

        virtual void addSetupItems (Component&, OwnedArray<Component>&)     {}
        virtual Result processResultsFromSetupItems (Component&)            { return Result::ok(); }

        virtual bool initialiseProject (Project& project) = 0;

        virtual StringArray getDefaultModules()
        {
            const char* mods[] =
            {
                "juce_core",
                "juce_events",
                "juce_graphics",
                "juce_data_structures",
                "juce_gui_basics",
                "juce_gui_extra",
                "juce_cryptography",
                "juce_video",
                "juce_opengl",
                "juce_audio_basics",
                "juce_audio_devices",
                "juce_audio_formats",
                "juce_audio_processors",
                nullptr
            };

            return StringArray (mods);
        }

        String appTitle;
        File targetFolder, projectFile, modulesFolder;
        Component* ownerWindow;
        StringArray failedFiles;

        //==============================================================================
        Project* runWizard (Component* window,
                            const String& projectName,
                            const File& target)
        {
            ownerWindow = window;
            appTitle = projectName;
            targetFolder = target;

            if (! targetFolder.exists())
            {
                if (! targetFolder.createDirectory())
                    failedFiles.add (targetFolder.getFullPathName());
            }
            else if (FileHelpers::containsAnyNonHiddenFiles (targetFolder))
            {
                if (! AlertWindow::showOkCancelBox (AlertWindow::InfoIcon,
                                                    TRANS("New Juce Project"),
                                                    TRANS("The folder you chose isn't empty - are you sure you want to create the project there?")
                                                      + "\n\n"
                                                      + TRANS("Any existing files with the same names may be overwritten by the new files.")))
                    return nullptr;
            }

            projectFile = targetFolder.getChildFile (File::createLegalFileName (appTitle))
                                      .withFileExtension (Project::projectFileExtension);

            ScopedPointer<Project> project (new Project (projectFile));

            if (failedFiles.size() == 0)
            {
                project->setFile (projectFile);
                project->setTitle (appTitle);
                project->getBundleIdentifier() = project->getDefaultBundleIdentifier();

                if (! initialiseProject (*project))
                    return nullptr;

                addDefaultModules (*project);

                if (project->save (false, true) != FileBasedDocument::savedOk)
                    return nullptr;

                project->setChangedFlag (false);
            }

            if (failedFiles.size() > 0)
            {
                AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon,
                                                  TRANS("Errors in Creating Project!"),
                                                  TRANS("The following files couldn't be written:")
                                                    + "\n\n"
                                                    + failedFiles.joinIntoString ("\n", 0, 10));
                return nullptr;
            }

            return project.release();
        }

        bool selectJuceFolder()
        {
            for (;;)
            {
                FileChooser fc ("Select your JUCE modules folder...",
                                findDefaultModulesFolder(),
                                "*");

                if (! fc.browseForDirectory())
                    return false;

                if (isJuceModulesFolder (fc.getResult()))
                {
                    modulesFolder = fc.getResult();
                    return true;
                }

                AlertWindow::showMessageBox (AlertWindow::WarningIcon,
                                             "Not a valid JUCE modules folder!",
                                             "Please select the folder containing your juce_* modules!\n\n"
                                             "This is required so that the new project can be given some essential core modules.");
            }
        }

        //==============================================================================
        File getSourceFilesFolder() const
        {
            return projectFile.getSiblingFile ("Source");
        }

        void createSourceFolder()
        {
            if (! getSourceFilesFolder().createDirectory())
                failedFiles.add (getSourceFilesFolder().getFullPathName());
        }

        void addDefaultModules (Project& project)
        {
            StringArray mods (getDefaultModules());

            ModuleList list;
            list.addAllModulesInFolder (modulesFolder);

            for (int i = 0; i < mods.size(); ++i)
                if (const ModuleDescription* info = list.getModuleWithID (mods[i]))
                    project.getModules().addModule (info->manifestFile, true);
        }
    };

    //==============================================================================
    struct GUIAppWizard   : public NewProjectWizard
    {
        GUIAppWizard()  {}

        String getName()          { return TRANS("GUI Application"); }
        String getDescription()   { return TRANS("Creates a standard application"); }

        void addSetupItems (Component& setupComp, OwnedArray<Component>& itemsCreated)
        {
            const String fileOptions[] = { TRANS("Create a Main.cpp file"),
                                           TRANS("Create a Main.cpp file and a basic window"),
                                           TRANS("Don't create any files") };

            createFileCreationOptionComboBox (setupComp, itemsCreated,
                                              StringArray (fileOptions, numElementsInArray (fileOptions)));
        }

        Result processResultsFromSetupItems (Component& setupComp)
        {
            createMainCpp = createWindow = false;

            switch (getFileCreationComboResult (setupComp))
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
            createSourceFolder();

            File mainCppFile    = getSourceFilesFolder().getChildFile ("Main.cpp");
            File contentCompCpp = getSourceFilesFolder().getChildFile ("MainComponent.cpp");
            File contentCompH   = contentCompCpp.withFileExtension (".h");
            String contentCompName = "MainContentComponent";

            project.getProjectTypeValue() = ProjectType::getGUIAppTypeName();

            Project::Item sourceGroup (createSourceGroup (project));

            setExecutableNameForAllTargets (project, File::createLegalFileName (appTitle));

            String appHeaders (CodeHelpers::createIncludeStatement (project.getAppIncludeFile(), mainCppFile));

            if (createWindow)
            {
                appHeaders << newLine << CodeHelpers::createIncludeStatement (contentCompH, mainCppFile);

                String windowH = project.getFileTemplate ("jucer_ContentCompTemplate_h")
                                    .replace ("INCLUDE_JUCE", CodeHelpers::createIncludeStatement (project.getAppIncludeFile(), contentCompH), false)
                                    .replace ("CONTENTCOMPCLASS", contentCompName, false)
                                    .replace ("HEADERGUARD", CodeHelpers::makeHeaderGuardName (contentCompH), false);

                String windowCpp = project.getFileTemplate ("jucer_ContentCompTemplate_cpp")
                                    .replace ("INCLUDE_JUCE", CodeHelpers::createIncludeStatement (project.getAppIncludeFile(), contentCompCpp), false)
                                    .replace ("INCLUDE_CORRESPONDING_HEADER", CodeHelpers::createIncludeStatement (contentCompH, contentCompCpp), false)
                                    .replace ("CONTENTCOMPCLASS", contentCompName, false);

                if (! FileHelpers::overwriteFileWithNewDataIfDifferent (contentCompH, windowH))
                    failedFiles.add (contentCompH.getFullPathName());

                if (! FileHelpers::overwriteFileWithNewDataIfDifferent (contentCompCpp, windowCpp))
                    failedFiles.add (contentCompCpp.getFullPathName());

                sourceGroup.addFile (contentCompCpp, -1, true);
                sourceGroup.addFile (contentCompH, -1, false);
            }

            if (createMainCpp)
            {
                String mainCpp = project.getFileTemplate (createWindow ? "jucer_MainTemplate_Window_cpp"
                                                                       : "jucer_MainTemplate_NoWindow_cpp")
                                    .replace ("APPHEADERS", appHeaders, false)
                                    .replace ("APPCLASSNAME", CodeHelpers::makeValidIdentifier (appTitle + "Application", false, true, false), false)
                                    .replace ("APPNAME", CodeHelpers::addEscapeChars (appTitle), false)
                                    .replace ("CONTENTCOMPCLASS", contentCompName, false)
                                    .replace ("ALLOWMORETHANONEINSTANCE", "true", false);

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
    struct ConsoleAppWizard   : public NewProjectWizard
    {
        ConsoleAppWizard()  {}

        String getName()          { return TRANS("Console Application"); }
        String getDescription()   { return TRANS("Creates a command-line application with no GUI features"); }

        void addSetupItems (Component& setupComp, OwnedArray<Component>& itemsCreated)
        {
            const String fileOptions[] = { TRANS("Create a Main.cpp file"),
                                           TRANS("Don't create any files") };

            createFileCreationOptionComboBox (setupComp, itemsCreated,
                                              StringArray (fileOptions, numElementsInArray (fileOptions)));
        }

        Result processResultsFromSetupItems (Component& setupComp)
        {
            createMainCpp = false;

            switch (getFileCreationComboResult (setupComp))
            {
                case 0:     createMainCpp = true;  break;
                case 1:     break;
                default:    jassertfalse; break;
            }

            return Result::ok();
        }

        bool initialiseProject (Project& project)
        {
            createSourceFolder();

            project.getProjectTypeValue() = ProjectType::getConsoleAppTypeName();

            Project::Item sourceGroup (createSourceGroup (project));

            setExecutableNameForAllTargets (project, File::createLegalFileName (appTitle));

            if (createMainCpp)
            {
                File mainCppFile = getSourceFilesFolder().getChildFile ("Main.cpp");
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
    struct AudioPluginAppWizard   : public NewProjectWizard
    {
        AudioPluginAppWizard()  {}

        String getName() override          { return TRANS("Audio Plug-In"); }
        String getDescription() override   { return TRANS("Creates an audio plugin project"); }

        StringArray getDefaultModules() override
        {
            StringArray s (NewProjectWizard::getDefaultModules());
            s.add ("juce_audio_plugin_client");
            return s;
        }

        bool initialiseProject (Project& project) override
        {
            createSourceFolder();

            String filterClassName = CodeHelpers::makeValidIdentifier (appTitle, true, true, false) + "AudioProcessor";
            filterClassName = filterClassName.substring (0, 1).toUpperCase() + filterClassName.substring (1);
            String editorClassName = filterClassName + "Editor";

            File filterCppFile = getSourceFilesFolder().getChildFile ("PluginProcessor.cpp");
            File filterHFile   = filterCppFile.withFileExtension (".h");
            File editorCppFile = getSourceFilesFolder().getChildFile ("PluginEditor.cpp");
            File editorHFile   = editorCppFile.withFileExtension (".h");

            project.getProjectTypeValue() = ProjectType::getAudioPluginTypeName();

            Project::Item sourceGroup (createSourceGroup (project));
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
            sourceGroup.addFile (filterHFile,   -1, false);
            sourceGroup.addFile (editorCppFile, -1, true);
            sourceGroup.addFile (editorHFile,   -1, false);

            project.createExporterForCurrentPlatform();

            return true;
        }
    };

    //==============================================================================
    struct StaticLibraryWizard   : public NewProjectWizard
    {
        StaticLibraryWizard()  {}

        String getName() override          { return TRANS("Static Library"); }
        String getDescription() override   { return TRANS("Creates a static library"); }

        bool initialiseProject (Project& project) override
        {
            createSourceFolder();
            project.getProjectTypeValue() = ProjectType::getStaticLibTypeName();
            createSourceGroup (project);
            setExecutableNameForAllTargets (project, File::createLegalFileName (appTitle));
            project.createExporterForCurrentPlatform();

            return true;
        }
    };

    //==============================================================================
    struct DynamicLibraryWizard   : public NewProjectWizard
    {
        DynamicLibraryWizard()  {}

        String getName() override          { return TRANS("Dynamic Library"); }
        String getDescription() override   { return TRANS("Creates a dynamic library"); }

        bool initialiseProject (Project& project) override
        {
            createSourceFolder();
            project.getProjectTypeValue() = ProjectType::getDynamicLibTypeName();
            createSourceGroup (project);
            setExecutableNameForAllTargets (project, File::createLegalFileName (appTitle));
            project.createExporterForCurrentPlatform();

            return true;
        }
    };

    //==============================================================================
    class WizardComp  : public Component,
                        private ButtonListener,
                        private ComboBoxListener,
                        private TextEditorListener
    {
    public:
        WizardComp()
            : projectName (TRANS("Project name")),
              nameLabel (String::empty, TRANS("Project Name") + ":"),
              typeLabel (String::empty, TRANS("Project Type") + ":"),
              fileBrowser (FileBrowserComponent::saveMode | FileBrowserComponent::canSelectDirectories,
                           getLastWizardFolder(), nullptr, nullptr),
              fileOutline (String::empty, TRANS("Project Folder") + ":"),
              createButton (TRANS("Create") + "..."),
              cancelButton (TRANS("Cancel"))
        {
            setOpaque (true);
            setSize (600, 500);

            addChildAndSetID (&projectName, "projectName");
            projectName.setText ("NewProject");
            projectName.setBounds ("100, 14, parent.width / 2 - 10, top + 22");
            nameLabel.attachToComponent (&projectName, true);
            projectName.addListener (this);

            addChildAndSetID (&projectType, "projectType");
            projectType.addItemList (getWizardNames(), 1);
            projectType.setSelectedId (1, dontSendNotification);
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
            cancelButton.addShortcut (KeyPress (KeyPress::escapeKey));
            cancelButton.setBounds ("right - 140, createButton.top, createButton.left - 10, createButton.bottom");
            cancelButton.addListener (this);

            updateCustomItems();
            updateCreateButton();
        }

        void paint (Graphics& g) override
        {
            g.fillAll (Colour::greyLevel (0.93f));
        }

        void buttonClicked (Button* b) override
        {
            if (b == &createButton)
            {
                createProject();
            }
            else
            {
                if (MainWindow* mw = dynamic_cast<MainWindow*> (getTopLevelComponent()))
                    IntrojucerApp::getApp().mainWindowList.closeWindow (mw);
            }
        }

        void createProject()
        {
            MainWindow* mw = Component::findParentComponentOfClass<MainWindow>();
            jassert (mw != nullptr);

            ScopedPointer<NewProjectWizard> wizard (createWizard());

            if (wizard != nullptr)
            {
                Result result (wizard->processResultsFromSetupItems (*this));

                if (result.failed())
                {
                    AlertWindow::showMessageBox (AlertWindow::WarningIcon,
                                                 TRANS("Create Project"),
                                                 result.getErrorMessage());
                    return;
                }

                if (! wizard->selectJuceFolder())
                    return;

                ScopedPointer<Project> project (wizard->runWizard (mw, projectName.getText(),
                                                                   fileBrowser.getSelectedFile (0)));

                if (project != nullptr)
                    mw->setProject (project.release());
            }
        }

        void updateCustomItems()
        {
            customItems.clear();

            ScopedPointer<NewProjectWizard> wizard (createWizard());

            if (wizard != nullptr)
                wizard->addSetupItems (*this, customItems);
        }

        void comboBoxChanged (ComboBox*) override
        {
            updateCustomItems();
        }

        void textEditorTextChanged (TextEditor&) override
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
            return createWizardType (projectType.getSelectedItemIndex());
        }

        void updateCreateButton()
        {
            createButton.setEnabled (projectName.getText().trim().isNotEmpty());
        }
    };

    //==============================================================================
    static int getNumWizards()
    {
        return 5;
    }

    static NewProjectWizard* createWizardType (int index)
    {
        switch (index)
        {
            case 0:     return new GUIAppWizard();
            case 1:     return new ConsoleAppWizard();
            case 2:     return new AudioPluginAppWizard();
            case 3:     return new StaticLibraryWizard();
            case 4:     return new DynamicLibraryWizard();
            default:    jassertfalse; break;
        }

        return nullptr;
    }

    static StringArray getWizardNames()
    {
        StringArray s;

        for (int i = 0; i < getNumWizards(); ++i)
        {
            ScopedPointer<NewProjectWizard> wiz (createWizardType (i));
            s.add (wiz->getName());
        }

        return s;
    }
};

Component* createNewProjectWizardComponent()
{
    return new NewProjectWizardClasses::WizardComp();
}
