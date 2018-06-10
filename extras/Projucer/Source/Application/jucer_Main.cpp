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

#include "jucer_Headers.h"

#include "jucer_Application.h"
#include "../CodeEditor/jucer_OpenDocumentManager.h"
#include "../CodeEditor/jucer_SourceCodeEditor.h"
#include "../Utility/UI/PropertyComponents/jucer_FilePathPropertyComponent.h"
#include "../Project/UI/Sidebar/jucer_TreeItemTypes.h"
#include "Windows/jucer_UTF8WindowComponent.h"
#include "Windows/jucer_SVGPathDataWindowComponent.h"
#include "Windows/jucer_AboutWindowComponent.h"
#include "Windows/jucer_ApplicationUsageDataWindowComponent.h"
#include "Windows/jucer_EditorColourSchemeWindowComponent.h"
#include "Windows/jucer_GlobalPathsWindowComponent.h"
#include "Windows/jucer_PIPCreatorWindowComponent.h"
#include "Windows/jucer_FloatingToolWindow.h"

#include "../LiveBuildEngine/jucer_MessageIDs.h"
#include "../LiveBuildEngine/jucer_CppHelpers.h"
#include "../LiveBuildEngine/jucer_SourceCodeRange.h"
#include "../LiveBuildEngine/jucer_ClassDatabase.h"
#include "../LiveBuildEngine/jucer_DiagnosticMessage.h"

#include "../LiveBuildEngine/jucer_CompileEngineDLL.h"
#include "../LiveBuildEngine/jucer_CompileEngineClient.h"
#include "../LiveBuildEngine/UI/jucer_ActivityListComponent.h"
#include "../LiveBuildEngine/UI/jucer_BuildTabStatusComponent.h"
#include "../LiveBuildEngine/UI/jucer_ComponentListComponent.h"
#include "../LiveBuildEngine/jucer_CompileEngineServer.h"

JUCE_IMPLEMENT_SINGLETON (CompileEngineDLL)

struct ProjucerAppClasses
{
    #include "../CodeEditor/jucer_LiveBuildCodeEditor.h"
    #include "../LiveBuildEngine/UI/jucer_ErrorListComponent.h"
};

#include "jucer_CommandLine.h"

#include "../Project/UI/jucer_ProjectContentComponent.cpp"
#include "jucer_Application.cpp"


START_JUCE_APPLICATION (ProjucerApplication)
