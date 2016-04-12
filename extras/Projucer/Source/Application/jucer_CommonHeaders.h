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

#ifndef JUCER_COMMONHEADERS_H_INCLUDED
#define JUCER_COMMONHEADERS_H_INCLUDED

//==============================================================================
struct TargetOS
{
    enum OS
    {
        windows = 0,
        osx,
        linux,
        unknown
    };

    static OS getThisOS() noexcept
    {
       #if JUCE_WINDOWS
        return windows;
       #elif JUCE_MAC
        return osx;
       #elif JUCE_LINUX
        return linux;
       #else
        return unknown;
       #endif
    }
};

typedef TargetOS::OS DependencyPathOS;

//==============================================================================
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

enum ColourIds
{
    mainBackgroundColourId          = 0x2340000,
};

#endif   // JUCER_COMMONHEADERS_H_INCLUDED
