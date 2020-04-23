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
struct StaticLibraryWizard   : public NewProjectWizard
{
    StaticLibraryWizard()  {}

    String getName() const override         { return TRANS("Static Library"); }
    String getDescription() const override  { return TRANS("Creates a static library."); }
    const char* getIcon() const override    { return BinaryData::wizard_StaticLibrary_svg; }

    bool initialiseProject (Project& project) override
    {
        createSourceFolder();
        project.setProjectType (build_tools::ProjectType_StaticLibrary::getTypeName());
        createSourceGroup (project);
        setExecutableNameForAllTargets (project, File::createLegalFileName (appTitle));

        return true;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StaticLibraryWizard)
};
