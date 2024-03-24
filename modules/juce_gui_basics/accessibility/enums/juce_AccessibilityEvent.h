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

} // namespace juce
