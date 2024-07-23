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
} // namespace juce

#include "juce_UIAProviderBase_windows.h"
#include "juce_UIAExpandCollapseProvider_windows.h"
#include "juce_UIAGridItemProvider_windows.h"
#include "juce_UIAGridProvider_windows.h"
#include "juce_UIAInvokeProvider_windows.h"
#include "juce_UIARangeValueProvider_windows.h"
#include "juce_UIASelectionProvider_windows.h"
#include "juce_UIATextProvider_windows.h"
#include "juce_UIAToggleProvider_windows.h"
#include "juce_UIATransformProvider_windows.h"
#include "juce_UIAValueProvider_windows.h"
#include "juce_UIAWindowProvider_windows.h"
