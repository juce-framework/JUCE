/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

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
#include "../Project/UI/jucer_ProjectContentComponent.h"
#include "../Project/UI/Sidebar/jucer_TreeItemTypes.h"
#include "Windows/jucer_UTF8WindowComponent.h"
#include "Windows/jucer_SVGPathDataWindowComponent.h"
#include "Windows/jucer_AboutWindowComponent.h"
#include "Windows/jucer_EditorColourSchemeWindowComponent.h"
#include "Windows/jucer_GlobalPathsWindowComponent.h"
#include "Windows/jucer_PIPCreatorWindowComponent.h"
#include "Windows/jucer_FloatingToolWindow.h"

#include "jucer_CommandLine.h"

#include "../Project/UI/jucer_ProjectContentComponent.cpp"
#include "jucer_Application.cpp"


START_JUCE_APPLICATION (ProjucerApplication)
