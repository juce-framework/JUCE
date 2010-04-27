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

#ifndef __JUCER_HEADERS_JUCEHEADER__
#define __JUCER_HEADERS_JUCEHEADER__

#ifdef _MSC_VER
 #pragma warning (disable: 4100 4505)
 #define DONT_LIST_JUCE_AUTOLINKEDLIBS 1
#endif

//==============================================================================
#include "../JuceLibraryCode/JuceHeader.h"
#include "utility/jucer_StoredSettings.h"
#include "utility/jucer_UtilityFunctions.h"
#include "utility/jucer_RelativePath.h"
#include "utility/jucer_ValueRemapperSource.h"
#include "ui/jucer_CommandIDs.h"

//==============================================================================
extern ApplicationCommandManager* commandManager;

//==============================================================================
static const char* const newLine = "\r\n";

const char* const projectItemDragType   = "Project Items";
const char* const drawableItemDragType  = "Drawable Items";
const char* const componentItemDragType = "Components";

const char* const textFileExtensions    = "cpp;h;hpp;mm;m;c;txt;xml;plist;rtf;html;htm;php;py;rb;cs";
const char* const sourceFileExtensions  = "cpp;mm;m;c;h;hpp";
const char* const headerFileExtensions  = "h;hpp";

const int numSwatchColours = 24;
const int snapSizes[] = { 2, 3, 4, 5, 6, 8, 10, 12, 16, 20, 24, 32 };


#endif   // __JUCER_HEADERS_JUCEHEADER__
