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

/** An action that can be performed by an accessible UI element.

    @tags{Accessibility}
*/
enum class AccessibilityActionType
{
    /** Represents a "press" action.

        This will be called when the user "clicks" the UI element using an
        accessibility client.
    */
    press,

    /** Represents a "toggle" action.

        This will be called when the user toggles the state of a UI element,
        for example a toggle button or the selection of a list item.
    */
    toggle,

    /** Indicates that the UI element has received focus.

        This will be called when a UI element receives focus from an accessibility
        client, or keyboard focus from the application.
    */
    focus,

    /** Represents the user showing a contextual menu for a UI element.

        This will be called for UI elements which expand and collapse to
        show contextual information or menus, or show a popup.
    */
    showMenu
};

/** A simple wrapper for building a collection of supported accessibility actions
    and corresponding callbacks for a UI element.

    Pass one of these when constructing an `AccessibilityHandler` to enable users
    to interact with a UI element via the supported actions.

    @tags{Accessibility}
*/
class JUCE_API  AccessibilityActions
{
public:
    /** Constructor.

        Creates a default AccessibilityActions object with no action callbacks.
    */
    AccessibilityActions() = default;

    /** Adds an action.

        When the user performs this action with an accessibility client
        `actionCallback` will be called.

        Returns a reference to itself so that several calls can be chained.
    */
    AccessibilityActions& addAction (AccessibilityActionType type,
                                     std::function<void()> actionCallback)
    {
        actionMap[type] = std::move (actionCallback);
        return *this;
    }

    /** Returns true if the specified action is supported. */
    bool contains (AccessibilityActionType type) const
    {
        return actionMap.find (type) != actionMap.end();
    }

    /** If an action has been registered for the provided action type, invokes the
        action and returns true. Otherwise, returns false.
    */
    bool invoke (AccessibilityActionType type) const
    {
        auto iter = actionMap.find (type);

        if (iter == actionMap.end())
            return false;

        iter->second();
        return true;
    }

private:
    std::map<AccessibilityActionType, std::function<void()>> actionMap;
};

} // namespace juce
