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

namespace juce
{

/** A list of events that can be notified to any subscribed accessibility clients.

    To post a notification, call `AccessibilityHandler::notifyAccessibilityEvent`
    on the associated handler with the appropriate `AccessibilityEvent` type and
    listening clients will be notified.

    @tags{Accessibility}
*/
enum class AccessibilityEvent
{
    /** Indicates that the UI element's value has changed.

        This should be called on the handler that implements `AccessibilityValueInterface`
        for the UI element that has changed.
    */
    valueChanged,

    /** Indicates that the structure of the UI elements has changed in a
        significant way.

        This should be posted on the top-level handler whose structure has changed.
    */
    structureChanged,

    /** Indicates that the selection of a text element has changed.

        This should be called on the handler that implements `AccessibilityTextInterface`
        for the text element that has changed.
    */
    textSelectionChanged,

    /** Indicates that the visible text of a text element has changed.

        This should be called on the handler that implements `AccessibilityTextInterface`
        for the text element that has changed.
    */
    textChanged,

    /** Indicates that the selection of rows in a list or table has changed.

        This should be called on the handler that implements `AccessibilityTableInterface`
        for the UI element that has changed.
    */
    rowSelectionChanged
};

}
