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

#include "../../../ProjectSaving/jucer_ProjectExporter.h"
#include "../../../Application/Windows/jucer_TranslationToolWindowComponent.h"
#include "../../../Utility/UI/jucer_JucerTreeViewBase.h"
#include "../../../Wizards/jucer_NewFileWizard.h"
#include "../jucer_ContentViewComponents.h"
#include "../jucer_FileGroupInformationComponent.h"
#include "../jucer_ModulesInformationComponent.h"

struct TreeItemTypes
{
    #include "jucer_ProjectTreeItemBase.h"
    #include "jucer_ExporterTreeItems.h"
    #include "jucer_ModuleTreeItems.h"
    #include "jucer_FileTreeItems.h"
};
