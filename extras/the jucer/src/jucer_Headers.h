/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#ifndef __JUCER_HEADERS_JUCEHEADER__
#define __JUCER_HEADERS_JUCEHEADER__

#ifdef _MSC_VER
 #pragma warning (disable: 4100 4505)
#endif

#define DONT_LIST_JUCE_AUTOLINKEDLIBS 1

//==============================================================================
// Normally you'd just include juce.h here, but I'm doing it this way instead
// so that I'm guaranteed to spot any bugs that might creep in when using the
// macro-free include method..
#include "../../../src/juce_WithoutMacros.h"
#include "../../../src/juce_DefineMacros.h"

//==============================================================================
#define JUCER_MAJOR_VERSION  1
#define JUCER_MINOR_VERSION  11

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
