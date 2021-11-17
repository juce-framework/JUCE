/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#pragma once


//==============================================================================
// The GCC extensions define linux somewhere in the headers, so undef it here...
#if JUCE_GCC
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
