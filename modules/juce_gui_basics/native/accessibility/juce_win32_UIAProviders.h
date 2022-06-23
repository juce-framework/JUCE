/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

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
