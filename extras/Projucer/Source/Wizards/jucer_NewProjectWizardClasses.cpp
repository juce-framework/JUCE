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

#include "../Application/jucer_Headers.h"
#include "jucer_NewProjectWizardClasses.h"
#include "../ProjectSaving/jucer_ProjectExporter.h"
#include "../Utility/UI/jucer_SlidingPanelComponent.h"

struct NewProjectWizardClasses
{
    class WizardComp;
    #include "jucer_NewProjectWizard.h"

    #include "jucer_ProjectWizard_GUIApp.h"
    #include "jucer_ProjectWizard_Console.h"
    #include "jucer_ProjectWizard_AudioPlugin.h"
    #include "jucer_ProjectWizard_StaticLibrary.h"
    #include "jucer_ProjectWizard_DLL.h"
    #include "jucer_ProjectWizard_openGL.h"
    #include "jucer_ProjectWizard_Animated.h"
    #include "jucer_ProjectWizard_AudioApp.h"
    #include "jucer_ProjectWizard_Blank.h"

    #include "jucer_NewProjectWizardComponent.h"
    #include "jucer_TemplateThumbnailsComponent.h"
    #include "jucer_StartPageComponent.h"

    //==============================================================================
    static int getNumWizards() noexcept
    {
        return 9;
    }

    static std::unique_ptr<NewProjectWizard> createWizardType (int index)
    {
        switch (index)
        {
            case 0:     return std::unique_ptr<NewProjectWizard> (new NewProjectWizardClasses::GUIAppWizard());
            case 1:     return std::unique_ptr<NewProjectWizard> (new NewProjectWizardClasses::AnimatedAppWizard());
            case 2:     return std::unique_ptr<NewProjectWizard> (new NewProjectWizardClasses::OpenGLAppWizard());
            case 3:     return std::unique_ptr<NewProjectWizard> (new NewProjectWizardClasses::ConsoleAppWizard());
            case 4:     return std::unique_ptr<NewProjectWizard> (new NewProjectWizardClasses::AudioAppWizard());
            case 5:     return std::unique_ptr<NewProjectWizard> (new NewProjectWizardClasses::AudioPluginAppWizard());
            case 6:     return std::unique_ptr<NewProjectWizard> (new NewProjectWizardClasses::StaticLibraryWizard());
            case 7:     return std::unique_ptr<NewProjectWizard> (new NewProjectWizardClasses::DynamicLibraryWizard());
            case 8:     return std::unique_ptr<NewProjectWizard> (new NewProjectWizardClasses::BlankAppWizard());
            default:    jassertfalse; break;
        }

        return {};
    }

    static StringArray getWizardNames()
    {
        StringArray s;

        for (int i = 0; i < getNumWizards(); ++i)
            s.add (createWizardType (i)->getName());

        return s;
    }
};

Component* createNewProjectWizardComponent()
{
    return new NewProjectWizardClasses::StartPageComponent();
}
