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

struct OpenGLAppWizard   : public NewProjectWizard
{
    OpenGLAppWizard()  {}

    String getName() const override         { return TRANS("OpenGL Application"); }
    String getDescription() const override  { return TRANS("Creates a blank JUCE application with a single window component. This component supports openGL drawing features including 3D model import and GLSL shaders."); }
    const char* getIcon() const override    { return BinaryData::wizard_OpenGL_svg; }

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

        // create main window
        String windowCpp = project.getFileTemplate ("jucer_OpenGLComponentTemplate_cpp")
                            .replace ("INCLUDE_JUCE", CodeHelpers::createIncludeStatement (project.getAppIncludeFile(), contentCompCpp), false);

        if (! FileHelpers::overwriteFileWithNewDataIfDifferent (contentCompCpp, windowCpp))
            failedFiles.add (contentCompCpp.getFullPathName());

        sourceGroup.addFileAtIndex (contentCompCpp, -1, true);


        // create main cpp
        String mainCpp = project.getFileTemplate ("jucer_MainTemplate_SimpleWindow_cpp")
                            .replace ("APPHEADERS", appHeaders, false)
                            .replace ("APPCLASSNAME", CodeHelpers::makeValidIdentifier (appTitle + "Application", false, true, false), false)
                            .replace ("APPNAME", CppTokeniserFunctions::addEscapeChars (appTitle), false)
                            .replace ("CONTENTCOMPCLASS", contentCompName, false)
                            .replace ("ALLOWMORETHANONEINSTANCE", "true", false);

        if (! FileHelpers::overwriteFileWithNewDataIfDifferent (mainCppFile, mainCpp))
            failedFiles.add (mainCppFile.getFullPathName());

        sourceGroup.addFileAtIndex (mainCppFile, -1, true);

        return true;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OpenGLAppWizard)
};
