/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-license
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#pragma once


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
#include "../Settings/jucer_StoredSettings.h"
#include "../Utility/UI/jucer_Icons.h"
#include "../Utility/Helpers/jucer_MiscUtilities.h"
#include "../Utility/Helpers/jucer_CodeHelpers.h"
#include "../Utility/Helpers/jucer_FileHelpers.h"
#include "../Utility/Helpers/jucer_RelativePath.h"
#include "../Utility/Helpers/jucer_ValueSourceHelpers.h"
#include "../Utility/Helpers/jucer_PresetIDs.h"
#include "jucer_CommandIDs.h"

//==============================================================================
const char* const projectItemDragType   = "Project Items";
const char* const drawableItemDragType  = "Drawable Items";
const char* const componentItemDragType = "Components";

enum ColorIds
{
    backgroundColorId                = 0x2340000,
    secondaryBackgroundColorId       = 0x2340001,
    defaultTextColorId               = 0x2340002,
    widgetTextColorId                = 0x2340003,
    defaultButtonBackgroundColorId   = 0x2340004,
    secondaryButtonBackgroundColorId = 0x2340005,
    userButtonBackgroundColorId      = 0x2340006,
    defaultIconColorId               = 0x2340007,
    treeIconColorId                  = 0x2340008,
    defaultHighlightColorId          = 0x2340009,
    defaultHighlightedTextColorId    = 0x234000a,
    codeEditorLineNumberColorId      = 0x234000b,
    activeTabIconColorId             = 0x234000c,
    inactiveTabBackgroundColorId     = 0x234000d,
    inactiveTabIconColorId           = 0x234000e,
    contentHeaderBackgroundColorId   = 0x234000f,
    widgetBackgroundColorId          = 0x2340010,
    secondaryWidgetBackgroundColorId = 0x2340011,
};
