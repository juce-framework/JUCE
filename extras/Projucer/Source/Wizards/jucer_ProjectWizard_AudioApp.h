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
struct AudioAppWizard   : public NewProjectWizard
{
    AudioAppWizard()  {}

    String getName() const override         { return TRANS("Audio Application"); }
    String getDescription() const override  { return TRANS("Creates a JUCE application with a single window component and audio and MIDI in/out functions."); }
    const char* getIcon() const override    { return BinaryData::wizard_AudioApp_svg; }

    StringArray getFileCreationOptions() override
    {
        return { "Create header and implementation files",
                 "Create header file only" };
    }

    Result processResultsFromSetupItems (WizardComp& setupComp) override
    {
        createCppFile = false;

        switch (setupComp.getFileCreationComboID())
        {
            case 0:     createCppFile = true;  break;
            case 1:     break;
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
        auto appHeaders = juceHeaderInclude + newLine + CodeHelpers::createIncludeStatement (contentCompH, mainCppFile);

        // create main window
        String windowH = project.getFileTemplate (createCppFile ? "jucer_AudioComponentTemplate_h"
                                                                : "jucer_AudioComponentSimpleTemplate_h")
                            .replace ("%%include_juce%%", juceHeaderInclude)
                            .replace ("%%content_component_class%%", contentCompName, false);

        if (!build_tools::overwriteFileWithNewDataIfDifferent (contentCompH, windowH))
            failedFiles.add (contentCompH.getFullPathName());

        sourceGroup.addFileAtIndex (contentCompH, -1, false);

        if (createCppFile)
        {
            String windowCpp = project.getFileTemplate ("jucer_AudioComponentTemplate_cpp")
                                  .replace ("%%include_juce%%", juceHeaderInclude)
                                  .replace ("%%include_corresponding_header%%", CodeHelpers::createIncludeStatement (contentCompH, contentCompCpp), false)
                                  .replace ("%%content_component_class%%", contentCompName, false);



            if (!build_tools::overwriteFileWithNewDataIfDifferent (contentCompCpp, windowCpp))
                failedFiles.add (contentCompCpp.getFullPathName());

            sourceGroup.addFileAtIndex (contentCompCpp, -1, true);
        }

        // create main cpp
        String mainCpp = project.getFileTemplate ("jucer_MainTemplate_SimpleWindow_cpp")
                            .replace ("%%app_headers%%", appHeaders, false)
                            .replace ("%%app_class_name%%", build_tools::makeValidIdentifier (appTitle + "Application", false, true, false), false)
                            .replace ("%%content_component_class%%", contentCompName, false)
                            .replace ("%%allow_more_than_one_instance%%", "true", false);

        if (!build_tools::overwriteFileWithNewDataIfDifferent (mainCppFile, mainCpp))
            failedFiles.add (mainCppFile.getFullPathName());

        sourceGroup.addFileAtIndex (mainCppFile, -1, true);

        return true;
    }

    StringArray getDefaultModules() override
    {
        StringArray s (NewProjectWizard::getDefaultModules());
        s.addIfNotAlreadyThere ("juce_audio_utils");
        return s;
    }

private:
    bool createCppFile;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioAppWizard)
};
