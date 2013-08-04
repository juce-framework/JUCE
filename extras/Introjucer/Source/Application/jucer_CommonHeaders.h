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

#ifndef __JUCER_COMMONHEADERS_JUCEHEADER__
#define __JUCER_COMMONHEADERS_JUCEHEADER__


#include "../Utility/jucer_StoredSettings.h"
#include "../Utility/jucer_Icons.h"
#include "../Utility/jucer_MiscUtilities.h"
#include "../Utility/jucer_CodeHelpers.h"
#include "../Utility/jucer_FileHelpers.h"
#include "../Utility/jucer_RelativePath.h"
#include "../Utility/jucer_ValueSourceHelpers.h"
#include "../Utility/jucer_PresetIDs.h"
#include "jucer_CommandIDs.h"

//==============================================================================
const char* const projectItemDragType   = "Project Items";
const char* const drawableItemDragType  = "Drawable Items";
const char* const componentItemDragType = "Components";

const char* const sourceFileExtensions          = "cpp;mm;m;c;cc;cxx";
const char* const headerFileExtensions          = "h;hpp;hxx;hh;inl";
const char* const sourceOrHeaderFileExtensions  = "cpp;mm;m;c;cc;cxx;h;hpp;hxx;hh;inl";

enum ColourIds
{
    mainBackgroundColourId          = 0x2340000,
    treeviewHighlightColourId       = 0x2340002,
};

#endif   // __JUCER_COMMONHEADERS_JUCEHEADER__
