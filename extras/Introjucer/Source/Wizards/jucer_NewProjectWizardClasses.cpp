/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

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

#include "jucer_NewProjectWizardClasses.h"
#include "jucer_ProjectType.h"
#include "jucer_Module.h"
#include "../Project Saving/jucer_ProjectExporter.h"
#include "../Application/jucer_Application.h"
#include "../Application/jucer_MainWindow.h"
#include "../Utility/jucer_SlidingPanelComponent.h"

struct NewProjectWizardClasses
{
    #include "jucer_NewProjectWizard.h"

    #include "jucer_GUIAppWizard.h"
    #include "jucer_ConsoleAppWizard.h"
    #include "jucer_AudioPluginAppWizard.h"
    #include "jucer_StaticLibraryWizard.h"
    #include "jucer_DynamicLibraryWizard.h"
};

struct NewProjectWizardComponents
{
    #include "jucer_NewProjectWizardComponent.h"
    #include "jucer_TemplateThumbnailsComponent.h"
    #include "jucer_StartPageComponent.h"
};

Component* createNewProjectWizardComponent()
{
    return new NewProjectWizardComponents::StartPageComponent();
}
