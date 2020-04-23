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
struct GUIAppWizard   : public NewProjectWizard
{
    GUIAppWizard()  {}

    String getName() const override         { return TRANS("GUI Application"); }
    String getDescription() const override  { return TRANS("Creates a blank JUCE application with a single window component."); }
    const char* getIcon() const override    { return BinaryData::wizard_GUI_svg; }

    StringArray getFileCreationOptions() override
    {
        return { "Create a Main.cpp file with header and implementation files",
                 "Create a Main.cpp file with header file only",
                 "Create a Main.cpp file only",
                 "Don't create any files" };
    }

    Result processResultsFromSetupItems (WizardComp& setupComp) override
    {
        createMainCpp = createWindow = createCppFile = false;

        switch (setupComp.getFileCreationComboID())
        {
            case 0:     createMainCpp = createWindow = createCppFile = true;  break;
            case 1:     createMainCpp = createWindow = true;  break;
            case 2:     createMainCpp = true;  break;
            case 3:     break;
            default:    jassertfalse; break;
        }

        return Result::ok();
    }

    bool initialiseProject (Project& project) override
    {
        createSourceFolder();

        File mainCppFile    = getSourceFilesFolder().getChildFile ("Main.cpp");
        File contentCompCpp = getSourceFilesFolder().getChildFile ("MainComponent.cpp");
        File contentCompH   = contentCompCpp.withFileExtension (".h");
        String contentCompName = "MainComponent";

        project.setProjectType (build_tools::ProjectType_GUIApp::getTypeName());

        Project::Item sourceGroup (createSourceGroup (project));

        setExecutableNameForAllTargets (project, File::createLegalFileName (appTitle));

        auto juceHeaderInclude = CodeHelpers::createIncludePathIncludeStatement (Project::getJuceSourceHFilename());
        auto appHeaders = juceHeaderInclude;

        if (createWindow)
        {
            appHeaders << newLine << CodeHelpers::createIncludeStatement (contentCompH, mainCppFile);

            String windowH = project.getFileTemplate (createCppFile ? "jucer_ContentCompTemplate_h"
                                                                    : "jucer_ContentCompSimpleTemplate_h")
                                .replace ("%%include_juce%%", juceHeaderInclude)
                                .replace ("%%content_component_class%%", contentCompName, false);

            if (!build_tools::overwriteFileWithNewDataIfDifferent (contentCompH, windowH))
                failedFiles.add (contentCompH.getFullPathName());

            sourceGroup.addFileAtIndex (contentCompH, -1, false);

            if (createCppFile)
            {
                String windowCpp = project.getFileTemplate ("jucer_ContentCompTemplate_cpp")
                                    .replace ("%%include_juce%%", juceHeaderInclude)
                                    .replace ("%%include_corresponding_header%%", CodeHelpers::createIncludeStatement (contentCompH, contentCompCpp), false)
                                    .replace ("%%content_component_class%%", contentCompName, false);

                if (!build_tools::overwriteFileWithNewDataIfDifferent (contentCompCpp, windowCpp))
                    failedFiles.add (contentCompCpp.getFullPathName());

                sourceGroup.addFileAtIndex (contentCompCpp, -1, true);
            }
        }

        if (createMainCpp)
        {
            String mainCpp = project.getFileTemplate (createWindow ? "jucer_MainTemplate_Window_cpp"
                                                                   : "jucer_MainTemplate_NoWindow_cpp")
                                .replace ("%%app_headers%%", appHeaders, false)
                                .replace ("%%app_class_name%%", build_tools::makeValidIdentifier (appTitle + "Application", false, true, false), false)
                                .replace ("%%content_component_class%%", contentCompName, false)
                                .replace ("%%allow_more_than_one_instance%%", "true", false);

            if (!build_tools::overwriteFileWithNewDataIfDifferent (mainCppFile, mainCpp))
                failedFiles.add (mainCppFile.getFullPathName());

            sourceGroup.addFileAtIndex (mainCppFile, -1, true);
        }

        return true;
    }

private:
    bool createMainCpp, createWindow, createCppFile;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GUIAppWizard)
};
