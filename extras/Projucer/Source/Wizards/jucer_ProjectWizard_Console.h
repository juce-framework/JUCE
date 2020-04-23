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


//==============================================================================
struct ConsoleAppWizard   : public NewProjectWizard
{
    ConsoleAppWizard()  {}

    String getName() const override         { return TRANS("Console Application"); }
    String getDescription() const override  { return TRANS("Creates a command-line application without GUI support."); }
    const char* getIcon() const override    { return BinaryData::wizard_ConsoleApp_svg; }

    StringArray getFileCreationOptions() override
    {
        return { "Create a Main.cpp file",
                 "Don't create any files" };
    }

    Result processResultsFromSetupItems (WizardComp& setupComp) override
    {
        createMainCpp = false;

        switch (setupComp.getFileCreationComboID())
        {
            case 0:     createMainCpp = true;  break;
            case 1:     break;
            default:    jassertfalse; break;
        }

        return Result::ok();
    }

    bool initialiseProject (Project& project) override
    {
        createSourceFolder();

        project.setProjectType (build_tools::ProjectType_ConsoleApp::getTypeName());

        Project::Item sourceGroup (createSourceGroup (project));

        setExecutableNameForAllTargets (project, File::createLegalFileName (appTitle));

        if (createMainCpp)
        {
            File mainCppFile = getSourceFilesFolder().getChildFile ("Main.cpp");

            String mainCpp = project.getFileTemplate ("jucer_MainConsoleAppTemplate_cpp")
                                .replace ("%%app_headers%%", CodeHelpers::createIncludePathIncludeStatement (Project::getJuceSourceHFilename()), false);

            if (!build_tools::overwriteFileWithNewDataIfDifferent (mainCppFile, mainCpp))
                failedFiles.add (mainCppFile.getFullPathName());

            sourceGroup.addFileAtIndex (mainCppFile, -1, true);
        }

        return true;
    }

private:
    bool createMainCpp;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ConsoleAppWizard)
};
