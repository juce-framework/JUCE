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

#pragma once


#include "../Utility/Helpers/jucer_PresetIDs.h"

//==============================================================================
static void setExecutableNameForAllTargets (Project& project, const String& exeName)
{
    for (Project::ExporterIterator exporter (project); exporter.next();)
        for (ProjectExporter::ConfigIterator config (*exporter); config.next();)
            config->getValue (Ids::targetName) = exeName;
}

static Project::Item createSourceGroup (Project& project)
{
    return project.getMainGroup().addNewSubGroup ("Source", 0);
}

static File& getLastWizardFolder()
{
    if (getAppSettings().lastWizardFolder.isDirectory())
        return getAppSettings().lastWizardFolder;

   #if JUCE_WINDOWS
    static File lastFolderFallback (File::getSpecialLocation (File::userDocumentsDirectory));
   #else
    static File lastFolderFallback (File::getSpecialLocation (File::userHomeDirectory));
   #endif

    return lastFolderFallback;
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

    virtual StringArray getFileCreationOptions()                        { return {}; }
    virtual Result processResultsFromSetupItems (WizardComp&)           { return Result::ok(); }

    virtual bool initialiseProject (Project& project) = 0;

    virtual StringArray getDefaultModules()
    {
        return
        {
            "juce_audio_basics",
            "juce_audio_devices",
            "juce_audio_formats",
            "juce_audio_processors",
            "juce_core",
            "juce_cryptography",
            "juce_data_structures",
            "juce_events",
            "juce_graphics",
            "juce_gui_basics",
            "juce_gui_extra",
            "juce_opengl",
        };
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
                        const File& target,
                        bool useGlobalPath)
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

        auto project = std::make_unique<Project> (projectFile);

        if (failedFiles.size() == 0)
        {
            project->setTitle (appTitle);

            if (! initialiseProject (*project))
                return nullptr;

            project->getConfigFlag ("JUCE_STRICT_REFCOUNTEDPOINTER") = true;
            project->getProjectValue (Ids::useAppConfig) = false;
            project->getProjectValue (Ids::addUsingNamespaceToJuceHeader) = false;

            if (! ProjucerApplication::getApp().getLicenseController().getCurrentState().isPaidOrGPL())
                project->getProjectValue (Ids::displaySplashScreen) = true;

            addExporters (*project, wc);
            addDefaultModules (*project, useGlobalPath);

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

    void addDefaultModules (Project& project, bool useGlobalPath)
    {
        auto defaultModules = getDefaultModules();

        AvailableModulesList list;
        list.scanPaths ({ modulesFolder });

        for (auto& mod : list.getAllModules())
            if (defaultModules.contains (mod.first))
                project.getEnabledModules().addModule (mod.second, false, useGlobalPath);
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
