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

#include "../jucer_Headers.h"

#include "jucer_Application.h"
#include "jucer_OpenDocumentManager.h"
#include "../Code Editor/jucer_SourceCodeEditor.h"
#include "../Utility/jucer_FilePathPropertyComponent.h"
#include "../Project/jucer_TreeItemTypes.h"
#include "../Utility/jucer_UTF8Component.h"
#include "../Utility/jucer_SVGPathDataComponent.h"
#include "../Utility/jucer_AboutWindowComponent.h"
#include "../Utility/jucer_ApplicationUsageDataWindowComponent.h"
#include "../Utility/jucer_EditorColourSchemeWindowComponent.h"
#include "../Utility/jucer_GlobalSearchPathsWindowComponent.h"
#include "../Utility/jucer_FloatingToolWindow.h"

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

juce_ImplementSingleton (CompileEngineDLL);

struct ProjucerAppClasses
{
    #include "../Code Editor/jucer_LiveBuildCodeEditor.h"
    #include "../LiveBuildEngine/projucer_ErrorListComponent.h"
};

#include "jucer_CommandLine.h"

#include "../Project/jucer_ProjectContentComponent.cpp"
#include "jucer_Application.cpp"


START_JUCE_APPLICATION (ProjucerApplication)
