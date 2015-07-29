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

#ifndef JUCER_TREEITEMTYPES_H_INCLUDED
#define JUCER_TREEITEMTYPES_H_INCLUDED

#include "../Project Saving/jucer_ProjectExporter.h"
#include "../Utility/jucer_TranslationTool.h"
#include "../Utility/jucer_JucerTreeViewBase.h"
#include "../Wizards/jucer_NewFileWizard.h"
#include "jucer_GroupInformationComponent.h"
#include "jucer_ModulesPanel.h"


struct FileTreeItemTypes
{
    #include "jucer_ProjectTree_Base.h"
    #include "jucer_ProjectTree_Group.h"
    #include "jucer_ProjectTree_File.h"
};

struct ConfigTreeItemTypes
{
    #include "jucer_ConfigTree_Base.h"
    #include "jucer_ConfigTree_Modules.h"
    #include "jucer_ConfigTree_Exporter.h"
};


#endif   // JUCER_TREEITEMTYPES_H_INCLUDED
