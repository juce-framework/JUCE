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

struct ConsoleAppWizard   : public NewProjectWizard
{
    ConsoleAppWizard()  {}

    String getName() const override         { return TRANS("Console Application"); }
    String getDescription() const override  { return TRANS("Creates a command-line application without GUI support."); }
    const char* getIcon() const override    { return BinaryData::wizard_ConsoleApp_svg; }

    void addSetupItems (Component& setupComp, OwnedArray<Component>& itemsCreated) override
    {
        const String fileOptions[] = { TRANS("Create a Main.cpp file"),
                                       TRANS("Don't create any files") };

        createFileCreationOptionComboBox (setupComp, itemsCreated,
                                          StringArray (fileOptions, numElementsInArray (fileOptions)));
    }

    Result processResultsFromSetupItems (WizardComp& setupComp) override
    {
        createMainCpp = false;

        switch (getFileCreationComboResult (setupComp))
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

        project.getProjectTypeValue() = ProjectType_ConsoleApp::getTypeName();

        Project::Item sourceGroup (createSourceGroup (project));

        setExecutableNameForAllTargets (project, File::createLegalFileName (appTitle));

        if (createMainCpp)
        {
            File mainCppFile = getSourceFilesFolder().getChildFile ("Main.cpp");
            String appHeaders (CodeHelpers::createIncludeStatement (project.getAppIncludeFile(), mainCppFile));

            String mainCpp = project.getFileTemplate ("jucer_MainConsoleAppTemplate_cpp")
                                .replace ("APPHEADERS", appHeaders, false);

            if (! FileHelpers::overwriteFileWithNewDataIfDifferent (mainCppFile, mainCpp))
                failedFiles.add (mainCppFile.getFullPathName());

            sourceGroup.addFileAtIndex (mainCppFile, -1, true);
        }

        return true;
    }

private:
    bool createMainCpp;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ConsoleAppWizard)
};
