/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

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

#ifndef __JUCER_COMMONINCLUDES_H__
#define __JUCER_COMMONINCLUDES_H__


#include "utility/jucer_StoredSettings.h"
#include "utility/jucer_MiscUtilities.h"
#include "utility/jucer_CodeHelpers.h"
#include "utility/jucer_FileHelpers.h"
#include "utility/jucer_RelativePath.h"
#include "utility/jucer_ValueSourceHelpers.h"
#include "utility/jucer_PresetIDs.h"
#include "ui/jucer_CommandIDs.h"

//==============================================================================
extern ApplicationCommandManager* commandManager;

//==============================================================================
static const char* const newLine = "\r\n";

const char* const projectItemDragType   = "Project Items";
const char* const drawableItemDragType  = "Drawable Items";
const char* const componentItemDragType = "Components";

const char* const sourceFileExtensions  = "cpp;mm;m;c;h;hpp";
const char* const headerFileExtensions  = "h;hpp";


#endif