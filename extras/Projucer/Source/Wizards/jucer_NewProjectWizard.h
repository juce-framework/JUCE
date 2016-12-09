/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

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

#ifndef JUCER_NEWPROJECTWIZARD_H_INCLUDED
#define JUCER_NEWPROJECTWIZARD_H_INCLUDED


//==============================================================================
static ComboBox& createFileCreationOptionComboBox (Component& setupComp,
                                              OwnedArray<Component>& itemsCreated,
                                              const StringArray& fileOptions)
{
    ComboBox* c = new ComboBox();
    itemsCreated.add (c);
    setupComp.addChildAndSetID (c, "filesToCreate");

    c->addItemList (fileOptions, 1);
    c->setSelectedId (1, dontSendNotification);

    Label* l = new Label (String(), TRANS("Files to Auto-Generate") + ":");
    l->attachToComponent (c, true);
    itemsCreated.add (l);

    c->setBounds ("parent.width / 2 + 160, 30, parent.width - 30, top + 22");

    return *c;
}

static int getFileCreationComboResult (WizardComp& setupComp)
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

//==============================================================================
struct NewProjectWizard
{
    NewProjectWizard() {}
    virtual ~NewProjectWizard() {}

    //==============================================================================
    virtual String getName() const = 0;
    virtual String getDescription() const = 0;
    virtual const char* getIcon() const = 0;

    virtual void addSetupItems (Component&, OwnedArray<Component>&)     {}
    virtual Result processResultsFromSetupItems (WizardComp&)           { return Result::ok(); }

    virtual bool initialiseProject (Project& project) = 0;

    virtual StringArray getDefaultModules()
    {
        static const char* mods[] =
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
    WizardComp* ownerWizardComp;
    StringArray failedFiles;

    bool selectJuceFolder()
    {
        return ModulesFolderPathBox::selectJuceFolder (modulesFolder);
    }

    //==============================================================================
    Project* runWizard (WizardComp& wc,
                        const String& projectName,
                        const File& target)
    {
        ownerWizardComp = &wc;
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
                                                TRANS("New JUCE Project"),
                                                TRANS("You chose the folder:\n\nXFLDRX\n\n").replace ("XFLDRX", targetFolder.getFullPathName())
                                                  + TRANS("This folder isn't empty - are you sure you want to create the project there?")
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

            addExporters (*project, wc);
            addDefaultModules (*project, false);

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

    void addDefaultModules (Project& project, bool areModulesCopiedLocally)
    {
        StringArray mods (getDefaultModules());

        ModuleList list;
        list.addAllModulesInFolder (modulesFolder);

        for (int i = 0; i < mods.size(); ++i)
            if (const ModuleDescription* info = list.getModuleWithID (mods[i]))
                project.getModules().addModule (info->moduleFolder, areModulesCopiedLocally);
    }

    void addExporters (Project& project, WizardComp& wizardComp)
    {
        StringArray types (wizardComp.platformTargets.getSelectedPlatforms());

        for (int i = 0; i < types.size(); ++i)
            project.addNewExporter (types[i]);

        if (project.getNumExporters() == 0)
            project.createExporterForCurrentPlatform();
    }
};


#endif   // JUCER_NEWPROJECTWIZARD_H_INCLUDED
