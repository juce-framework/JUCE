/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#ifndef __JUCER_COMMONHEADERS_JUCEHEADER__
#define __JUCER_COMMONHEADERS_JUCEHEADER__


#include "../Utility/jucer_StoredSettings.h"
#include "../Utility/jucer_MiscUtilities.h"
#include "../Utility/jucer_CodeHelpers.h"
#include "../Utility/jucer_FileHelpers.h"
#include "../Utility/jucer_RelativePath.h"
#include "../Utility/jucer_ValueSourceHelpers.h"
#include "../Utility/jucer_PresetIDs.h"
#include "jucer_CommandIDs.h"

//==============================================================================
extern ScopedPointer<ApplicationCommandManager> commandManager;

//==============================================================================
const char* const projectItemDragType   = "Project Items";
const char* const drawableItemDragType  = "Drawable Items";
const char* const componentItemDragType = "Components";

const char* const sourceFileExtensions          = "cpp;mm;m;c;cc;cxx";
const char* const headerFileExtensions          = "h;hpp;hxx";
const char* const sourceOrHeaderFileExtensions  = "cpp;mm;m;c;cc;cxx;h;hpp;hxx";

enum ColourIds
{
    mainBackgroundColourId          = 0x2340000,
    treeviewHighlightColourId       = 0x2340002,
};

#endif   // __JUCER_COMMONHEADERS_JUCEHEADER__
