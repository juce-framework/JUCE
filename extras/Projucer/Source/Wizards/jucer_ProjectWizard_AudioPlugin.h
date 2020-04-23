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
struct AudioPluginAppWizard   : public NewProjectWizard
{
    AudioPluginAppWizard()  {}

    String getName() const override         { return TRANS("Audio Plug-In"); }
    String getDescription() const override  { return TRANS("Creates a VST/AU/RTAS/AAX audio plug-in. This template features a single window GUI and Audio/MIDI IO functions."); }
    const char* getIcon() const override    { return BinaryData::wizard_AudioPlugin_svg; }

    StringArray getDefaultModules() override
    {
        StringArray s (NewProjectWizard::getDefaultModules());
        s.add ("juce_audio_plugin_client");
        s.add ("juce_audio_utils");

        return s;
    }

    bool initialiseProject (Project& project) override
    {
        createSourceFolder();

        String filterClassName = build_tools::makeValidIdentifier (appTitle, true, true, false) + "AudioProcessor";
        filterClassName = filterClassName.substring (0, 1).toUpperCase() + filterClassName.substring (1);
        String editorClassName = filterClassName + "Editor";

        File filterCppFile = getSourceFilesFolder().getChildFile ("PluginProcessor.cpp");
        File filterHFile   = filterCppFile.withFileExtension (".h");
        File editorCppFile = getSourceFilesFolder().getChildFile ("PluginEditor.cpp");
        File editorHFile   = editorCppFile.withFileExtension (".h");

        project.setProjectType (build_tools::ProjectType_AudioPlugin::getTypeName());

        setExecutableNameForAllTargets (project, File::createLegalFileName (appTitle));

        auto juceHeaderInclude = CodeHelpers::createIncludePathIncludeStatement (Project::getJuceSourceHFilename());

        String filterCpp = project.getFileTemplate ("jucer_AudioPluginFilterTemplate_cpp")
                            .replace ("%%filter_headers%%", CodeHelpers::createIncludeStatement (filterHFile, filterCppFile)
                                                            + newLine + CodeHelpers::createIncludeStatement (editorHFile, filterCppFile), false)
                            .replace ("%%filter_class_name%%", filterClassName, false)
                            .replace ("%%editor_class_name%%", editorClassName, false);

        String filterH = project.getFileTemplate ("jucer_AudioPluginFilterTemplate_h")
                            .replace ("%%app_headers%%", juceHeaderInclude, false)
                            .replace ("%%filter_class_name%%", filterClassName, false);

        String editorCpp = project.getFileTemplate ("jucer_AudioPluginEditorTemplate_cpp")
                            .replace ("%%editor_cpp_headers%%", CodeHelpers::createIncludeStatement (filterHFile, filterCppFile)
                                                               + newLine + CodeHelpers::createIncludeStatement (editorHFile, filterCppFile), false)
                            .replace ("%%filter_class_name%%", filterClassName, false)
                            .replace ("%%editor_class_name%%", editorClassName, false);

        String editorH = project.getFileTemplate ("jucer_AudioPluginEditorTemplate_h")
                            .replace ("%%editor_headers%%", juceHeaderInclude + newLine + CodeHelpers::createIncludeStatement (filterHFile, filterCppFile), false)
                            .replace ("%%filter_class_name%%", filterClassName, false)
                            .replace ("%%editor_class_name%%", editorClassName, false);

        if (!build_tools::overwriteFileWithNewDataIfDifferent (filterCppFile, filterCpp))
            failedFiles.add (filterCppFile.getFullPathName());

        if (!build_tools::overwriteFileWithNewDataIfDifferent (filterHFile, filterH))
            failedFiles.add (filterHFile.getFullPathName());

        if (!build_tools::overwriteFileWithNewDataIfDifferent (editorCppFile, editorCpp))
            failedFiles.add (editorCppFile.getFullPathName());

        if (!build_tools::overwriteFileWithNewDataIfDifferent (editorHFile, editorH))
            failedFiles.add (editorHFile.getFullPathName());

        Project::Item sourceGroup (createSourceGroup (project));

        sourceGroup.addFileAtIndex (filterCppFile, -1, true);
        sourceGroup.addFileAtIndex (filterHFile,   -1, false);
        sourceGroup.addFileAtIndex (editorCppFile, -1, true);
        sourceGroup.addFileAtIndex (editorHFile,   -1, false);

        project.getConfigFlag ("JUCE_VST3_CAN_REPLACE_VST2") = 0;

        return true;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAppWizard)
};
