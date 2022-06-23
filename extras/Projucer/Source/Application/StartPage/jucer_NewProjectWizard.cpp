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

#include "../jucer_Headers.h"
#include "../jucer_Application.h"
#include "../../ProjectSaving/jucer_ProjectExporter.h"

#include "jucer_NewProjectWizard.h"

//==============================================================================
static String getFileTemplate (const String& templateName)
{
    int dataSize = 0;

    if (auto* data = BinaryData::getNamedResource (templateName.toUTF8(), dataSize))
        return String::fromUTF8 (data, dataSize);

    jassertfalse;
    return {};
}

static String getJuceHeaderInclude()
{
    return CodeHelpers::createIncludePathIncludeStatement (Project::getJuceSourceHFilename());
}

static String getContentComponentName()
{
    return "MainComponent";
}

using Opts = NewProjectTemplates::FileCreationOptions;

static bool shouldCreateHeaderFile (Opts opts) noexcept  { return opts == Opts::header || opts == Opts::headerAndCpp; }
static bool shouldCreateCppFile    (Opts opts) noexcept  { return opts == Opts::headerAndCpp; }

static void doBasicProjectSetup (Project& project, const NewProjectTemplates::ProjectTemplate& projectTemplate, const String& name)
{
    project.setTitle (name);
    project.setProjectType (projectTemplate.projectTypeString);
    project.getMainGroup().addNewSubGroup ("Source", 0);

    project.getConfigFlag ("JUCE_STRICT_REFCOUNTEDPOINTER") = true;
    project.getProjectValue (Ids::useAppConfig) = false;
    project.getProjectValue (Ids::addUsingNamespaceToJuceHeader) = false;

    if (! ProjucerApplication::getApp().getLicenseController().getCurrentState().canUnlockFullFeatures())
        project.getProjectValue (Ids::displaySplashScreen) = true;

    if (NewProjectTemplates::isPlugin (projectTemplate))
        project.getConfigFlag ("JUCE_VST3_CAN_REPLACE_VST2") = 0;
}

static std::map<String, String> getSharedFileTokenReplacements()
{
    return { { "%%app_headers%%", getJuceHeaderInclude() } };
}

static std::map<String, String> getApplicationFileTokenReplacements (const String& name,
                                                                     NewProjectTemplates::FileCreationOptions fileOptions,
                                                                     const File& sourceFolder)
{
    auto tokenReplacements = getSharedFileTokenReplacements();

    tokenReplacements.insert ({ "%%app_class_name%%",
                                build_tools::makeValidIdentifier (name + "Application", false, true, false) });
    tokenReplacements.insert ({ "%%content_component_class%%",
                                getContentComponentName() });
    tokenReplacements.insert ({ "%%include_juce%%",
                                getJuceHeaderInclude() });

    if (shouldCreateHeaderFile (fileOptions))
        tokenReplacements["%%app_headers%%"] << newLine
                                             << CodeHelpers::createIncludeStatement (sourceFolder.getChildFile ("MainComponent.h"),
                                                                                     sourceFolder.getChildFile ("Main.cpp"));

    if (shouldCreateCppFile (fileOptions))
        tokenReplacements.insert ({ "%%include_corresponding_header%%",
                                    CodeHelpers::createIncludeStatement (sourceFolder.getChildFile ("MainComponent.h"),
                                                                         sourceFolder.getChildFile ("MainComponent.cpp")) });

    return tokenReplacements;
}

static std::map<String, String> getPluginFileTokenReplacements (const String& name,
                                                                const File& sourceFolder)
{
    auto tokenReplacements = getSharedFileTokenReplacements();

    auto processorCppFile = sourceFolder.getChildFile ("PluginProcessor.cpp");
    auto processorHFile   = processorCppFile.withFileExtension (".h");
    auto editorCppFile    = sourceFolder.getChildFile ("PluginEditor.cpp");
    auto editorHFile      = editorCppFile.withFileExtension (".h");

    auto processorHInclude = CodeHelpers::createIncludeStatement (processorHFile, processorCppFile);
    auto editorHInclude    = CodeHelpers::createIncludeStatement (editorHFile, processorCppFile);

    auto processorClassName = build_tools::makeValidIdentifier (name, false, true, false) + "AudioProcessor";
    processorClassName = processorClassName.substring (0, 1).toUpperCase() + processorClassName.substring (1);
    auto editorClassName = processorClassName + "Editor";

    tokenReplacements.insert ({"%%filter_headers%%",     processorHInclude + newLine + editorHInclude });
    tokenReplacements.insert ({"%%filter_class_name%%",  processorClassName });
    tokenReplacements.insert ({"%%editor_class_name%%",  editorClassName });
    tokenReplacements.insert ({"%%editor_cpp_headers%%", processorHInclude + newLine + editorHInclude });
    tokenReplacements.insert ({"%%editor_headers%%",     getJuceHeaderInclude() + newLine + processorHInclude });

    return tokenReplacements;
}

static bool addFiles (Project& project, const NewProjectTemplates::ProjectTemplate& projectTemplate,
                      const String& name, var fileOptionsVar, StringArray& failedFiles)
{
    auto sourceFolder = project.getFile().getSiblingFile ("Source");

    if (! sourceFolder.createDirectory())
    {
        failedFiles.add (sourceFolder.getFullPathName());
        return false;
    }

    auto fileOptions = NewProjectTemplates::getFileOptionForVar (fileOptionsVar);

    if (fileOptions == Opts::noFiles)
        return true;

    auto tokenReplacements = [&]() -> std::map<String, String>
    {
        if (NewProjectTemplates::isApplication (projectTemplate))
            return getApplicationFileTokenReplacements (name, fileOptions, sourceFolder);

        if (NewProjectTemplates::isPlugin (projectTemplate))
            return getPluginFileTokenReplacements (name, sourceFolder);

        jassertfalse;
        return {};
    }();

    auto sourceGroup = project.getMainGroup().getOrCreateSubGroup ("Source");

    for (auto& files : projectTemplate.getFilesForOption (fileOptions))
    {
        auto file = sourceFolder.getChildFile (files.first);
        auto fileContent = getFileTemplate (files.second);

        for (auto& tokenReplacement : tokenReplacements)
            fileContent = fileContent.replace (tokenReplacement.first, tokenReplacement.second, false);

        if (! build_tools::overwriteFileWithNewDataIfDifferent (file, fileContent))
        {
            failedFiles.add (file.getFullPathName());
            return false;
        }

        sourceGroup.addFileAtIndex (file, -1, (file.hasFileExtension (sourceFileExtensions)));
    }

    return true;
}

static void addModules (Project& project, Array<var> modules, const String& modulePath, bool useGlobalPath)
{
    AvailableModulesList list;
    list.scanPaths ({ modulePath });

    auto& projectModules = project.getEnabledModules();

    for (auto& mod : list.getAllModules())
        if (modules.contains (mod.first))
            projectModules.addModule (mod.second, false, useGlobalPath);

    for (auto& mod : projectModules.getModulesWithMissingDependencies())
        projectModules.tryToFixMissingDependencies (mod);
}

static void addExporters (Project& project, Array<var> exporters)
{
    for (auto exporter : exporters)
        project.addNewExporter (exporter.toString());

    for (Project::ExporterIterator exporter (project); exporter.next();)
        for (ProjectExporter::ConfigIterator config (*exporter); config.next();)
            config->getValue (Ids::targetName) = project.getProjectFilenameRootString();
}

//==============================================================================
File NewProjectWizard::getLastWizardFolder()
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

static void displayFailedFilesMessage (const StringArray& failedFiles)
{
    AlertWindow::showMessageBoxAsync (MessageBoxIconType::WarningIcon,
                                      TRANS("Errors in Creating Project!"),
                                      TRANS("The following files couldn't be written:")
                                        + "\n\n"
                                        + failedFiles.joinIntoString ("\n", 0, 10));
}

template <typename Callback>
static void prepareDirectory (const File& targetFolder, Callback&& callback)
{
    StringArray failedFiles;

    if (! targetFolder.exists())
    {
        if (! targetFolder.createDirectory())
        {
            displayFailedFilesMessage ({ targetFolder.getFullPathName() });
            return;
        }
    }
    else if (FileHelpers::containsAnyNonHiddenFiles (targetFolder))
    {
        AlertWindow::showOkCancelBox (MessageBoxIconType::InfoIcon,
                                      TRANS("New JUCE Project"),
                                      TRANS("You chose the folder:\n\nXFLDRX\n\n").replace ("XFLDRX", targetFolder.getFullPathName())
                                        + TRANS("This folder isn't empty - are you sure you want to create the project there?")
                                        + "\n\n"
                                        + TRANS("Any existing files with the same names may be overwritten by the new files."),
                                      {},
                                      {},
                                      nullptr,
                                      ModalCallbackFunction::create ([callback] (int result)
                                      {
                                          if (result != 0)
                                              callback();
                                      }));

        return;
    }

    callback();
}

void NewProjectWizard::createNewProject (const NewProjectTemplates::ProjectTemplate& projectTemplate,
                                         const File& targetFolder, const String& name, var modules, var exporters, var fileOptions,
                                         const String& modulePath, bool useGlobalModulePath,
                                         std::function<void (std::unique_ptr<Project>)> callback)
{
    prepareDirectory (targetFolder, [=]
    {
        auto project = std::make_unique<Project> (targetFolder.getChildFile (File::createLegalFileName (name))
                                                              .withFileExtension (Project::projectFileExtension));

        doBasicProjectSetup (*project, projectTemplate, name);

        StringArray failedFiles;

        if (addFiles (*project, projectTemplate, name, fileOptions, failedFiles))
        {
            addExporters (*project, *exporters.getArray());
            addModules   (*project, *modules.getArray(), modulePath, useGlobalModulePath);

            auto sharedProject = std::make_shared<std::unique_ptr<Project>> (std::move (project));
            (*sharedProject)->saveAsync (false, true, [sharedProject, failedFiles, callback] (FileBasedDocument::SaveResult r)
            {
                auto uniqueProject = std::move (*sharedProject.get());

                if (r == FileBasedDocument::savedOk)
                {
                    uniqueProject->setChangedFlag (false);
                    uniqueProject->loadFrom (uniqueProject->getFile(), true);
                    callback (std::move (uniqueProject));
                    return;
                }

                auto failedFilesCopy = failedFiles;
                failedFilesCopy.add (uniqueProject->getFile().getFullPathName());
                displayFailedFilesMessage (failedFilesCopy);
            });

            return;
        }

        displayFailedFilesMessage (failedFiles);
    });
}
