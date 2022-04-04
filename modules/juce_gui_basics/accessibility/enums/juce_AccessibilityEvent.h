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

    /** Indicates that the title of the UI element has changed.

        This should be called on the handler whose title has changed.
    */
    titleChanged,

    /** Indicates that the structure of the UI elements has changed in a
        significant way.

        This should be called on the top-level handler whose structure has changed.
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
