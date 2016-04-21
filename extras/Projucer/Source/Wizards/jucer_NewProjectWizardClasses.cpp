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

#include "../jucer_Headers.h"
#include "jucer_NewProjectWizardClasses.h"
#include "../Project/jucer_ProjectType.h"
#include "../Project/jucer_Module.h"
#include "../Project Saving/jucer_ProjectExporter.h"
#include "../Application/jucer_MainWindow.h"
#include "../Utility/jucer_SlidingPanelComponent.h"

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

    static NewProjectWizard* createWizardType (int index)
    {
        switch (index)
        {
            case 0:     return new NewProjectWizardClasses::GUIAppWizard();
            case 1:     return new NewProjectWizardClasses::AnimatedAppWizard();
            case 2:     return new NewProjectWizardClasses::OpenGLAppWizard();
            case 3:     return new NewProjectWizardClasses::ConsoleAppWizard();
            case 4:     return new NewProjectWizardClasses::AudioAppWizard();
            case 5:     return new NewProjectWizardClasses::AudioPluginAppWizard();
            case 6:     return new NewProjectWizardClasses::StaticLibraryWizard();
            case 7:     return new NewProjectWizardClasses::DynamicLibraryWizard();
            case 8:     return new NewProjectWizardClasses::BlankAppWizard();
            default:    jassertfalse; break;
        }

        return nullptr;
    }

    static StringArray getWizardNames()
    {
        StringArray s;

        for (int i = 0; i < getNumWizards(); ++i)
        {
            ScopedPointer<NewProjectWizard> wiz (createWizardType (i));
            s.add (wiz->getName());
        }

        return s;
    }
};

Component* createNewProjectWizardComponent()
{
    return new NewProjectWizardClasses::StartPageComponent();
}
