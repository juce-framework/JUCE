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

class AccessibilityNativeHandle;

/** Base class for accessible Components.

    This class wraps a Component and provides methods that allow an accessibility client,
    such as VoiceOver on macOS, or Narrator on Windows, to control it.

    It handles hierarchical navigation, properties, state, and various interfaces.

    @tags{Accessibility}
*/
class JUCE_API  AccessibilityHandler
{
public:
    /** Utility struct which holds one or more accessibility interfaces.

        The main purpose of this class is to provide convenience constructors from each
        of the four types of accessibility interface.
    */
    struct JUCE_API  Interfaces
    {
        Interfaces() = default;

        Interfaces (std::unique_ptr<AccessibilityValueInterface> ptr)  : value (std::move (ptr))  {}
        Interfaces (std::unique_ptr<AccessibilityTextInterface>  ptr)  : text  (std::move (ptr))  {}
        Interfaces (std::unique_ptr<AccessibilityTableInterface> ptr)  : table (std::move (ptr))  {}
        Interfaces (std::unique_ptr<AccessibilityCellInterface>  ptr)  : cell  (std::move (ptr))  {}

        Interfaces (std::unique_ptr<AccessibilityValueInterface> valueIn,
                    std::unique_ptr<AccessibilityTextInterface>  textIn,
                    std::unique_ptr<AccessibilityTableInterface> tableIn,
                    std::unique_ptr<AccessibilityCellInterface>  cellIn)
            : value (std::move (valueIn)),
              text  (std::move (textIn)),
              table (std::move (tableIn)),
              cell  (std::move (cellIn))
        {
        }

        std::unique_ptr<AccessibilityValueInterface> value;
        std::unique_ptr<AccessibilityTextInterface>  text;
        std::unique_ptr<AccessibilityTableInterface> table;
        std::unique_ptr<AccessibilityCellInterface>  cell;
    };

    /** Constructor.

        This will create a AccessibilityHandler which wraps the provided Component and makes
        it visible to accessibility clients. You must also specify a role for the UI element
        from the `AccessibilityRole` list which best describes it.

        To enable users to interact with the UI element you should provide the set of supported
        actions and their associated callbacks via the `accessibilityActions` parameter.

        For UI elements that support more complex interaction the value, text, table, and cell
        interfaces should be implemented as required and passed as the final argument of this
        constructor. See the documentation of these classes for more information about the
        types of control they represent and which methods need to be implemented.
    */
    AccessibilityHandler (Component& componentToWrap,
                          AccessibilityRole accessibilityRole,
                          AccessibilityActions actions = {},
                          Interfaces interfaces = {});

    /** Destructor. */
    virtual ~AccessibilityHandler();

    //==============================================================================
    /** Returns the Component that this handler represents. */
    const Component& getComponent() const noexcept   { return component; }

    /** Returns the Component that this handler represents. */
    Component& getComponent() noexcept               { return component; }

    //==============================================================================
    /** The type of UI element that this accessibility handler represents.

        @see AccessibilityRole
    */
    AccessibilityRole getRole() const noexcept       { return role; }

    /** The title of the UI element.

        This will be read out by the system and should be concise, preferably matching
        the visible title of the UI element (if any). For example, this might be the
        text of a button or a simple label.

        The default implementation will call `Component::getTitle()`, but you can override
        this to return a different string if required.

        If neither a name nor a description is provided then the UI element may be
        ignored by accessibility clients.

        This must be a localised string.
    */
    virtual String getTitle() const                  { return component.getTitle(); }

    /** A short description of the UI element.

        This may be read out by the system. It should not include the type of the UI
        element and should ideally be a single word, for example "Open" for a button
        that opens a window.

        The default implementation will call `Component::getDescription()`, but you
        can override this to return a different string if required.

        If neither a name nor a description is provided then the UI element may be
        ignored by accessibility clients.

        This must be a localised string.
    */
    virtual String getDescription() const            { return component.getDescription(); }

    /** Some help text for the UI element (if required).

        This may be read out by the system. This string functions in a similar way to
        a tooltip, for example "Click to open window." for a button which opens a window.

        The default implementation will call `Component::getHelpText()`, but you can
        override this to return a different string if required.

        This must be a localised string.
    */
    virtual String getHelp() const                   { return component.getHelpText(); }

    /** Returns the current state of the UI element.

        The default implementation of this method will set the focusable flag and, if
        this UI element is currently focused, will also set the focused flag.
    */
    virtual AccessibleState getCurrentState() const;

    /** Returns true if this UI element should be ignored by accessibility clients. */
    bool isIgnored() const;

    /** Returns true if this UI element is visible within its parent.

        This will always return true for UI elements with the AccessibleState::accessibleOffscreen
        flag set.
    */
    bool isVisibleWithinParent() const;

    //==============================================================================
    /** Returns the set of actions that the UI element supports and the associated
        callbacks.
    */
    const AccessibilityActions& getActions() const noexcept;

    /** Returns the value interface for this UI element, or nullptr if it is not supported.

        @see AccessibilityValueInterface
    */
    AccessibilityValueInterface* getValueInterface() const;

    /** Returns the table interface for this UI element, or nullptr if it is not supported.

        @see AccessibilityTableInterface
    */
    AccessibilityTableInterface* getTableInterface() const;

    /** Returns the cell interface for this UI element, or nullptr if it is not supported.

        @see AccessibilityCellInterface
    */
    AccessibilityCellInterface* getCellInterface() const;

    /** Returns the text interface for this UI element, or nullptr if it is not supported.

        @see AccessibilityTextInterface
    */
    AccessibilityTextInterface* getTextInterface() const;

    //==============================================================================
    /** Returns the first unignored parent of this UI element in the accessibility hierarchy,
        or nullptr if this is a root element without a parent.
    */
    AccessibilityHandler* getParent() const;

    /** Returns the unignored children of this UI element in the accessibility hierarchy. */
    std::vector<AccessibilityHandler*> getChildren() const;

    /** Checks whether a given UI element is a child of this one in the accessibility
        hierarchy.
    */
    bool isParentOf (const AccessibilityHandler* possibleChild) const noexcept;

    /** Returns the deepest child of this UI element in the accessibility hierarchy that
        contains the given screen point, or nullptr if there is no child at this point.
    */
    AccessibilityHandler* getChildAt (Point<int> screenPoint);

    /** Returns the deepest UI element which currently has focus.

        This can be a child of this UI element or, if no child is focused,
        this element itself.

        Note that this can be different to the value of the Component with keyboard
        focus returned by Component::getCurrentlyFocusedComponent().

        @see hasFocus
    */
    AccessibilityHandler* getChildFocus();

    /** Returns true if this UI element has the focus.

        @param trueIfChildFocused  if this is true, this method will also return true
                                   if any child of this UI element in the accessibility
                                   hierarchy has focus
    */
    bool hasFocus (bool trueIfChildFocused) const;

    /** Tries to give focus to this UI element.

        If the UI element is focusable and not ignored this will update the currently focused
        element, try to give keyboard focus to the Component it represents, and notify any
        listening accessibility clients that the current focus has changed.

        @see hasFocus, giveAwayFocus
    */
    void grabFocus();

    /** If this UI element or any of its children in the accessibility hierarchy currently
        have focus, this will defocus it.

        This will also give away the keyboard focus from the Component it represents, and
        notify any listening accessibility clients that the current focus has changed.

        @see hasFocus, grabFocus
    */
    void giveAwayFocus() const;

    //==============================================================================
    /** Used to send a notification to any observing accessibility clients that something
        has changed in the UI element.

        @see AccessibilityEvent
    */
    void notifyAccessibilityEvent (AccessibilityEvent event) const;

    /** A priority level that can help an accessibility client determine how to handle
        an announcement request.

        Exactly what this controls is platform-specific, but generally a low priority
        announcement will be read when the screen reader is free, whereas a high priority
        announcement will interrupt the current speech.
    */
    enum class AnnouncementPriority
    {
        low,
        medium,
        high
    };

    /** Posts an announcement to be made to the user.

        @param announcementString   a localised string containing the announcement to be read out
        @param priority             the appropriate priority level for the announcement
    */
    static void postAnnouncement (const String& announcementString, AnnouncementPriority priority);

    //==============================================================================
    /** @internal */
    AccessibilityNativeHandle* getNativeImplementation() const;
    /** @internal */
    std::type_index getTypeIndex() const  { return typeIndex; }

private:
    //==============================================================================
    friend class AccessibilityNativeHandle;

    //==============================================================================
    void grabFocusInternal (bool);
    void giveAwayFocusInternal() const;
    void takeFocus();

    static AccessibilityHandler* currentlyFocusedHandler;

    //==============================================================================
    Component& component;
    std::type_index typeIndex;

    const AccessibilityRole role;
    AccessibilityActions actions;

    Interfaces interfaces;

    //==============================================================================
    class AccessibilityNativeImpl;
    std::unique_ptr<AccessibilityNativeImpl> nativeImpl;

    static std::unique_ptr<AccessibilityNativeImpl> createNativeImpl (AccessibilityHandler&);

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AccessibilityHandler)
};

} // namespace juce
