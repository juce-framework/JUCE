/*
  ==============================================================================

   This file is part of the JUCE 7 technical preview.
   Copyright (c) 2022 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For the technical preview this file cannot be licensed commercially.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{
    void sendAccessibilityAutomationEvent (const AccessibilityHandler&, EVENTID);
    void sendAccessibilityPropertyChangedEvent (const AccessibilityHandler&, PROPERTYID, VARIANT);
}

#include "juce_win32_UIAProviderBase.h"
#include "juce_win32_UIAExpandCollapseProvider.h"
#include "juce_win32_UIAGridItemProvider.h"
#include "juce_win32_UIAGridProvider.h"
#include "juce_win32_UIAInvokeProvider.h"
#include "juce_win32_UIARangeValueProvider.h"
#include "juce_win32_UIASelectionProvider.h"
#include "juce_win32_UIATextProvider.h"
#include "juce_win32_UIAToggleProvider.h"
#include "juce_win32_UIATransformProvider.h"
#include "juce_win32_UIAValueProvider.h"
#include "juce_win32_UIAWindowProvider.h"
