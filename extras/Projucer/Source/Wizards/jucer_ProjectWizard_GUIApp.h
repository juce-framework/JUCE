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

struct GUIAppWizard   : public NewProjectWizard
{
    GUIAppWizard()  {}

    String getName() const override         { return TRANS("GUI Application"); }
    String getDescription() const override  { return TRANS("Creates a blank JUCE application with a single window component."); }
    const char* getIcon() const override    { return BinaryData::wizard_GUI_svg; }

    void addSetupItems (Component& setupComp, OwnedArray<Component>& itemsCreated) override
    {
        const String fileOptions[] = { TRANS("Create a Main.cpp file"),
                                       TRANS("Create a Main.cpp file and a basic window"),
                                       TRANS("Don't create any files") };

        createFileCreationOptionComboBox (setupComp, itemsCreated,
                                          StringArray (fileOptions, numElementsInArray (fileOptions)))
            .setSelectedId (2);
    }

    Result processResultsFromSetupItems (WizardComp& setupComp) override
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
