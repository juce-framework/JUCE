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

#include "jucer_Application.h"
#include "jucer_OpenDocumentManager.h"
#include "../Code Editor/jucer_SourceCodeEditor.h"
#include "../Utility/jucer_FilePathPropertyComponent.h"
#include "../Project/jucer_TreeItemTypes.h"
#include "../Utility/jucer_UTF8Component.h"
#include "../Utility/jucer_SVGPathDataComponent.h"
#include "../Utility/jucer_FloatingToolWindow.h"
#include "../Utility/jucer_DialogLookAndFeel.h"

#include "../LiveBuildEngine/projucer_MessageIDs.h"
#include "../LiveBuildEngine/projucer_CppHelpers.h"
#include "../LiveBuildEngine/projucer_SourceCodeRange.h"
#include "../LiveBuildEngine/projucer_ClassDatabase.h"
#include "../LiveBuildEngine/projucer_DiagnosticMessage.h"

#include "../LiveBuildEngine/projucer_CompileEngineDLL.h"
#include "../LiveBuildEngine/projucer_CompileEngineClient.h"
#include "../LiveBuildEngine/projucer_ActivityListComponent.h"
#include "../LiveBuildEngine/projucer_BuildTabStatusComp.h"
#include "../LiveBuildEngine/projucer_ComponentListComp.h"
#include "../LiveBuildEngine/projucer_CompileEngineServer.h"

#include "jucer_ProjucerLicenses.h"
juce_ImplementSingleton (ProjucerLicenses);

struct ProjucerAppClasses
{
    #include "../Code Editor/jucer_LiveBuildCodeEditor.h"
    #include "../LiveBuildEngine/projucer_ErrorListComponent.h"
};

#include "jucer_LoginForm.h"
#include "jucer_EulaDialogue.h"
#include "jucer_CommandLine.h"

#include "../Project/jucer_ProjectContentComponent.cpp"
#include "jucer_Application.cpp"


START_JUCE_APPLICATION (ProjucerApplication)
