/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

struct GUIAppWizard   : public NewProjectWizard
{
    GUIAppWizard()  {}

    String getName() const override         { return TRANS("GUI Application"); }
    String getDescription() const override  { return TRANS("Creates a blank JUCE application with a single window component."); }
    const char* getIcon() const override    { return BinaryData::wizard_GUI_svg; }

    StringArray getFileCreationOptions() override
    {
        return { "Create a Main.cpp file and a basic window",
                 "Create a Main.cpp file",
                 "Don't create any files" };
    }

    Result processResultsFromSetupItems (WizardComp& setupComp) override
    {
        createMainCpp = createWindow = false;

        switch (setupComp.getFileCreationComboID())
        {
            case 0:     createMainCpp = createWindow = true;  break;
            case 1:     createMainCpp = true;  break;
            case 2:     break;
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
        String contentCompName = "MainContentComponent";

        project.getProjectTypeValue() = ProjectType_GUIApp::getTypeName();

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

            sourceGroup.addFileAtIndex (contentCompCpp, -1, true);
            sourceGroup.addFileAtIndex (contentCompH, -1, false);
        }

        if (createMainCpp)
        {
            String mainCpp = project.getFileTemplate (createWindow ? "jucer_MainTemplate_Window_cpp"
                                                                   : "jucer_MainTemplate_NoWindow_cpp")
                                .replace ("APPHEADERS", appHeaders, false)
                                .replace ("APPCLASSNAME", CodeHelpers::makeValidIdentifier (appTitle + "Application", false, true, false), false)
                                .replace ("APPNAME", CppTokeniserFunctions::addEscapeChars (appTitle), false)
                                .replace ("CONTENTCOMPCLASS", contentCompName, false)
                                .replace ("ALLOWMORETHANONEINSTANCE", "true", false);

            if (! FileHelpers::overwriteFileWithNewDataIfDifferent (mainCppFile, mainCpp))
                failedFiles.add (mainCppFile.getFullPathName());

            sourceGroup.addFileAtIndex (mainCppFile, -1, true);
        }

        return true;
    }

private:
    bool createMainCpp, createWindow;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GUIAppWizard)
};
