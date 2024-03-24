/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

#pragma once


//==============================================================================
#ifdef linux
 #undef linux
#endif

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
       #elif JUCE_LINUX || JUCE_BSD
        return linux;
       #else
        return unknown;
       #endif
    }
};

typedef TargetOS::OS DependencyPathOS;

//==============================================================================
#include "../Settings/jucer_StoredSettings.h"
#include "../Utility/UI/jucer_Icons.h"
#include "../Utility/Helpers/jucer_MiscUtilities.h"
#include "../Utility/Helpers/jucer_CodeHelpers.h"
#include "../Utility/Helpers/jucer_FileHelpers.h"
#include "../Utility/Helpers/jucer_ValueSourceHelpers.h"
#include "../Utility/Helpers/jucer_PresetIDs.h"
#include "jucer_CommandIDs.h"

//==============================================================================
const char* const projectItemDragType   = "Project Items";
const char* const drawableItemDragType  = "Drawable Items";
const char* const componentItemDragType = "Components";

enum ColourIds
{
    backgroundColourId                = 0x2340000,
    secondaryBackgroundColourId       = 0x2340001,
    defaultTextColourId               = 0x2340002,
    widgetTextColourId                = 0x2340003,
    defaultButtonBackgroundColourId   = 0x2340004,
    secondaryButtonBackgroundColourId = 0x2340005,
    userButtonBackgroundColourId      = 0x2340006,
    defaultIconColourId               = 0x2340007,
    treeIconColourId                  = 0x2340008,
    defaultHighlightColourId          = 0x2340009,
    defaultHighlightedTextColourId    = 0x234000a,
    codeEditorLineNumberColourId      = 0x234000b,
    activeTabIconColourId             = 0x234000c,
    inactiveTabBackgroundColourId     = 0x234000d,
    inactiveTabIconColourId           = 0x234000e,
    contentHeaderBackgroundColourId   = 0x234000f,
    widgetBackgroundColourId          = 0x2340010,
    secondaryWidgetBackgroundColourId = 0x2340011,
};

//==============================================================================
static constexpr int projucerMajorVersion = ProjectInfo::versionNumber >> 16;
