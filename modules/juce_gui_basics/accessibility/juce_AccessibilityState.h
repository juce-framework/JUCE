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

/** Represents the state of an accessible UI element.

    An instance of this class is returned by `AccessibilityHandler::getCurrentState()`
    to convey its current state to an accessibility client.

    @see AccessibilityHandler

    @tags{Accessibility}
*/
class JUCE_API  AccessibleState
{
public:
    /** Constructor.

        Represents a "default" state with no flags set. To set a flag, use one of the
        `withX()` methods - these can be chained together to set multiple flags.
    */
    AccessibleState() = default;

    //==============================================================================
    /** Sets the checkable flag and returns the new state.

        @see isCheckable
    */
    [[nodiscard]] AccessibleState withCheckable() const noexcept            { return withFlag (Flags::checkable); }

    /** Sets the checked flag and returns the new state.

        @see isChecked
    */
    [[nodiscard]] AccessibleState withChecked() const noexcept              { return withFlag (Flags::checked); }

    /** Sets the collapsed flag and returns the new state.

        @see isCollapsed
    */
    [[nodiscard]] AccessibleState withCollapsed() const noexcept            { return withFlag (Flags::collapsed); }

    /** Sets the expandable flag and returns the new state.

        @see isExpandable
    */
    [[nodiscard]] AccessibleState withExpandable() const noexcept           { return withFlag (Flags::expandable); }

    /** Sets the expanded flag and returns the new state.

        @see isExpanded
    */
    [[nodiscard]] AccessibleState withExpanded() const noexcept             { return withFlag (Flags::expanded); }

    /** Sets the focusable flag and returns the new state.

        @see isFocusable
    */
    [[nodiscard]] AccessibleState withFocusable() const noexcept            { return withFlag (Flags::focusable); }

    /** Sets the focused flag and returns the new state.

        @see isFocused
    */
    [[nodiscard]] AccessibleState withFocused() const noexcept              { return withFlag (Flags::focused); }

    /** Sets the ignored flag and returns the new state.

        @see isIgnored
    */
    [[nodiscard]] AccessibleState withIgnored() const noexcept              { return withFlag (Flags::ignored); }

    /** Sets the selectable flag and returns the new state.

        @see isSelectable
    */
    [[nodiscard]] AccessibleState withSelectable() const noexcept           { return withFlag (Flags::selectable); }

    /** Sets the multiSelectable flag and returns the new state.

        @see isMultiSelectable
    */
    [[nodiscard]] AccessibleState withMultiSelectable() const noexcept      { return withFlag (Flags::multiSelectable); }

    /** Sets the selected flag and returns the new state.

        @see isSelected
    */
    [[nodiscard]] AccessibleState withSelected() const noexcept             { return withFlag (Flags::selected); }

    /** Sets the accessible offscreen flag and returns the new state.

        @see isSelected
    */
    [[nodiscard]] AccessibleState withAccessibleOffscreen() const noexcept  { return withFlag (Flags::accessibleOffscreen); }

    //==============================================================================
    /** Returns true if the UI element is checkable.

        @see withCheckable
    */
    bool isCheckable() const noexcept            { return isFlagSet (Flags::checkable); }

    /** Returns true if the UI element is checked.

        @see withChecked
    */
    bool isChecked() const noexcept              { return isFlagSet (Flags::checked); }

    /** Returns true if the UI element is collapsed.

        @see withCollapsed
    */
    bool isCollapsed() const noexcept            { return isFlagSet (Flags::collapsed); }

    /** Returns true if the UI element is expandable.

        @see withExpandable
    */
    bool isExpandable() const noexcept           { return isFlagSet (Flags::expandable); }

    /** Returns true if the UI element is expanded.

        @see withExpanded
    */
    bool isExpanded() const noexcept             { return isFlagSet (Flags::expanded); }

    /** Returns true if the UI element is focusable.

        @see withFocusable
    */
    bool isFocusable() const noexcept            { return isFlagSet (Flags::focusable); }

    /** Returns true if the UI element is focused.

        @see withFocused
    */
    bool isFocused() const noexcept              { return isFlagSet (Flags::focused); }

    /** Returns true if the UI element is ignored.

        @see withIgnored
    */
    bool isIgnored() const noexcept              { return isFlagSet (Flags::ignored); }

    /** Returns true if the UI element supports multiple item selection.

        @see withMultiSelectable
    */
    bool isMultiSelectable() const noexcept      { return isFlagSet (Flags::multiSelectable); }

    /** Returns true if the UI element is selectable.

        @see withSelectable
    */
    bool isSelectable() const noexcept           { return isFlagSet (Flags::selectable); }

    /** Returns true if the UI element is selected.

        @see withSelected
    */
    bool isSelected() const noexcept             { return isFlagSet (Flags::selected); }

    /** Returns true if the UI element is accessible offscreen.

        @see withSelected
    */
    bool isAccessibleOffscreen() const noexcept  { return isFlagSet (Flags::accessibleOffscreen); }

private:
    enum Flags
    {
        checkable           = (1 << 0),
        checked             = (1 << 1),
        collapsed           = (1 << 2),
        expandable          = (1 << 3),
        expanded            = (1 << 4),
        focusable           = (1 << 5),
        focused             = (1 << 6),
        ignored             = (1 << 7),
        multiSelectable     = (1 << 8),
        selectable          = (1 << 9),
        selected            = (1 << 10),
        accessibleOffscreen = (1 << 11)
    };

    [[nodiscard]] AccessibleState withFlag (int flag) const noexcept
    {
        auto copy = *this;
        copy.flags |= flag;

        return copy;
    }

    bool isFlagSet (int flag) const noexcept
    {
        return (flags & flag) != 0;
    }

    int flags = 0;
};

} // namespace juce
