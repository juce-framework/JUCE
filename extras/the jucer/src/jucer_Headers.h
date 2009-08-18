/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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
#endif

#define DONT_LIST_JUCE_AUTOLINKEDLIBS 1

//==============================================================================
#include "../../../juce_amalgamated.h"

//==============================================================================
#define JUCER_MAJOR_VERSION  1
#define JUCER_MINOR_VERSION  12

//==============================================================================
#include "BinaryData.h"
#include "utility/jucer_StoredSettings.h"
#include "utility/jucer_UtilityFunctions.h"
#include "ui/jucer_CommandIDs.h"

//==============================================================================
const int editorEdgeGap = 4;

const int numSwatchColours = 24;

extern ApplicationCommandManager* commandManager;



#endif   // __JUCER_HEADERS_JUCEHEADER__
