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

//==============================================================================
/**
    A base class for buttons.

    This contains all the logic for button behaviours such as enabling/disabling,
    responding to shortcut keystrokes, auto-repeating when held down, toggle-buttons
    and radio groups, etc.

    @see TextButton, DrawableButton, ToggleButton

    @tags{GUI}
*/
class JUCE_API  Button  : public Component,
                          public SettableTooltipClient
{
protected:
    //==============================================================================
    /** Creates a button.

        @param buttonName   the text to put in the button (the component's name is also
                            initially set to this string, but these can be changed later
                            using the setName() and setButtonText() methods)
    */
    explicit Button (const String& buttonName);

public:
    /** Destructor. */
    ~Button() override;

    //==============================================================================
    /** Changes the button's text.
        @see getButtonText
    */
    void setButtonText (const String& newText);

    /** Returns the text displayed in the button.
        @see setButtonText
    */
    const String& getButtonText() const               { return text; }

    //==============================================================================
    /** Returns true if the button is currently being held down.
        @see isOver
    */
    bool isDown() const noexcept;

    /** Returns true if the mouse is currently over the button.
        This will be also be true if the button is being held down.
        @see isDown
    */
    bool isOver() const noexcept;

    //==============================================================================
    /** Indicates that the button's on/off state is toggleable.

        By default this is false, and will only be true for ToggleButtons, buttons that
        are a part of a radio button group, and buttons for which
        getClickingTogglesState() == true, however you can use this method to manually
        indicate that a button is toggleable.

        This will present the button as toggleable to accessibility clients and add an
        accessible "toggle" action for the button that invokes setToggleState().

        @see ToggleButton, isToggleable, setToggleState, setClickingTogglesState, setRadioGroupId
    */
    void setToggleable (bool shouldBeToggleable);

    /** Returns true if the button's on/off state is toggleable.

        @see setToggleable, setClickingTogglesState
    */
    bool isToggleable() const noexcept                          { return canBeToggled || clickTogglesState; }

    /** A button has an on/off state associated with it, and this changes that.

        By default buttons are 'off' and for simple buttons that you click to perform
        an action you won't change this. Toggle buttons, however will want to
        change their state when turned on or off.

        @param shouldBeOn       whether to set the button's toggle state to be on or
                                off. If it's a member of a button group, this will
                                always try to turn it on, and to turn off any other
                                buttons in the group
        @param notification     determines the behaviour if the value changes - this
                                can invoke a synchronous call to clicked(), but
                                sendNotificationAsync is not supported
        @see getToggleState, setRadioGroupId
    */
    void setToggleState (bool shouldBeOn, NotificationType notification);

    /** Returns true if the button is 'on'.

        By default buttons are 'off' and for simple buttons that you click to perform
        an action you won't change this. Toggle buttons, however will want to
        change their state when turned on or off.

        @see setToggleState
    */
    bool getToggleState() const noexcept                        { return isOn.getValue(); }

    /** Returns the Value object that represents the button's toggle state.

        You can use this Value object to connect the button's state to external values or setters,
        either by taking a copy of the Value, or by using Value::referTo() to make it point to
        your own Value object.

        @see getToggleState, Value
    */
    Value& getToggleStateValue() noexcept                       { return isOn; }

    /** This tells the button to automatically flip the toggle state when
        the button is clicked.

        If set to true, then before the clicked() callback occurs, the toggle-state
        of the button is flipped. This will also cause isToggleable() to return true.

        @see isToggleable
    */
    void setClickingTogglesState (bool shouldAutoToggleOnClick) noexcept;

    /** Returns true if this button is set to be an automatic toggle-button.
        This returns the last value that was passed to setClickingTogglesState().
    */
    bool getClickingTogglesState() const noexcept               { return clickTogglesState; }

    //==============================================================================
    /** Enables the button to act as a member of a mutually-exclusive group
        of 'radio buttons'.

        If the group ID is set to a non-zero number, then this button will
        act as part of a group of buttons with the same ID, only one of
        which can be 'on' at the same time. Note that when it's part of
        a group, clicking a toggle-button that's 'on' won't turn it off.

        To find other buttons with the same ID, this button will search through
        its sibling components for ToggleButtons, so all the buttons for a
        particular group must be placed inside the same parent component.

        Set the group ID back to zero if you want it to act as a normal toggle
        button again.

        The notification argument lets you specify how other buttons should react
        to being turned on or off in response to this call.

        @see getRadioGroupId
    */
    void setRadioGroupId (int newGroupId, NotificationType notification = sendNotification);

    /** Returns the ID of the group to which this button belongs.
        (See setRadioGroupId() for an explanation of this).
    */
    int getRadioGroupId() const noexcept                        { return radioGroupId; }

    //==============================================================================
    /**
        Used to receive callbacks when a button is clicked.

        @see Button::addListener, Button::removeListener
    */
    class JUCE_API  Listener
    {
    public:
        /** Destructor. */
        virtual ~Listener() = default;

        /** Called when the button is clicked. */
        virtual void buttonClicked (Button*) = 0;

        /** Called when the button's state changes. */
        virtual void buttonStateChanged (Button*)  {}
    };

    /** Registers a listener to receive events when this button's state changes.
        If the listener is already registered, this will not register it again.
        @see removeListener
    */
    void addListener (Listener* newListener);

    /** Removes a previously-registered button listener
        @see addListener
    */
    void removeListener (Listener* listener);

    //==============================================================================
    /** You can assign a lambda to this callback object to have it called when the button is clicked. */
    std::function<void()> onClick;

    /** You can assign a lambda to this callback object to have it called when the button's state changes. */
    std::function<void()> onStateChange;

    //==============================================================================
    /** Causes the button to act as if it's been clicked.

        This will asynchronously make the button draw itself going down and up, and
        will then call back the clicked() method as if mouse was clicked on it.

        @see clicked
    */
    virtual void triggerClick();

    //==============================================================================
    /** Sets a command ID for this button to automatically invoke when it's clicked.

        When the button is pressed, it will use the given manager to trigger the
        command ID.

        Obviously be careful that the ApplicationCommandManager doesn't get deleted
        before this button is. To disable the command triggering, call this method and
        pass nullptr as the command manager.

        If generateTooltip is true, then the button's tooltip will be automatically
        generated based on the name of this command and its current shortcut key.

        @see addShortcut, getCommandID
    */
    void setCommandToTrigger (ApplicationCommandManager* commandManagerToUse,
                              CommandID commandID,
                              bool generateTooltip);

    /** Returns the command ID that was set by setCommandToTrigger(). */
    CommandID getCommandID() const noexcept             { return commandID; }

    //==============================================================================
    /** Assigns a shortcut key to trigger the button.

        The button registers itself with its top-level parent component for keypresses.

        Note that a different way of linking buttons to keypresses is by using the
        setCommandToTrigger() method to invoke a command.

        @see clearShortcuts
    */
    void addShortcut (const KeyPress&);

    /** Removes all key shortcuts that had been set for this button.
        @see addShortcut
    */
    void clearShortcuts();

    /** Returns true if the given keypress is a shortcut for this button.
        @see addShortcut
    */
    bool isRegisteredForShortcut (const KeyPress&) const;

    //==============================================================================
    /** Sets an auto-repeat speed for the button when it is held down.

        (Auto-repeat is disabled by default).

        @param initialDelayInMillisecs      how long to wait after the mouse is pressed before
                                            triggering the next click. If this is zero, auto-repeat
                                            is disabled
        @param repeatDelayInMillisecs       the frequently subsequent repeated clicks should be
                                            triggered
        @param minimumDelayInMillisecs      if this is greater than 0, the auto-repeat speed will
                                            get faster, the longer the button is held down, up to the
                                            minimum interval specified here
    */
    void setRepeatSpeed (int initialDelayInMillisecs,
                         int repeatDelayInMillisecs,
                         int minimumDelayInMillisecs = -1) noexcept;

    /** Sets whether the button click should happen when the mouse is pressed or released.

        By default the button is only considered to have been clicked when the mouse is
        released, but setting this to true will make it call the clicked() method as soon
        as the button is pressed.

        This is useful if the button is being used to show a pop-up menu, as it allows
        the click to be used as a drag onto the menu.
    */
    void setTriggeredOnMouseDown (bool isTriggeredOnMouseDown) noexcept;

    /** Returns whether the button click happens when the mouse is pressed or released.
        @see setTriggeredOnMouseDown
    */
    bool getTriggeredOnMouseDown() const noexcept;

    /** Returns the number of milliseconds since the last time the button
        went into the 'down' state.
    */
    uint32 getMillisecondsSinceButtonDown() const noexcept;

    //==============================================================================
    /** Sets the tooltip for this button.
        @see TooltipClient, TooltipWindow
    */
    void setTooltip (const String& newTooltip) override;

    //==============================================================================
    /** A combination of these flags are used by setConnectedEdges(). */
    enum ConnectedEdgeFlags
    {
        ConnectedOnLeft = 1,
        ConnectedOnRight = 2,
        ConnectedOnTop = 4,
        ConnectedOnBottom = 8
    };

    /** Hints about which edges of the button might be connected to adjoining buttons.

        The value passed in is a bitwise combination of any of the values in the
        ConnectedEdgeFlags enum.

        E.g. if you are placing two buttons adjacent to each other, you could use this to
        indicate which edges are touching, and the LookAndFeel might choose to drawn them
        without rounded corners on the edges that connect. It's only a hint, so the
        LookAndFeel can choose to ignore it if it's not relevant for this type of
        button.
    */
    void setConnectedEdges (int connectedEdgeFlags);

    /** Returns the set of flags passed into setConnectedEdges(). */
    int getConnectedEdgeFlags() const noexcept          { return connectedEdgeFlags; }

    /** Indicates whether the button adjoins another one on its left edge.
        @see setConnectedEdges
    */
    bool isConnectedOnLeft() const noexcept             { return (connectedEdgeFlags & ConnectedOnLeft) != 0; }

    /** Indicates whether the button adjoins another one on its right edge.
        @see setConnectedEdges
    */
    bool isConnectedOnRight() const noexcept            { return (connectedEdgeFlags & ConnectedOnRight) != 0; }

    /** Indicates whether the button adjoins another one on its top edge.
        @see setConnectedEdges
    */
    bool isConnectedOnTop() const noexcept              { return (connectedEdgeFlags & ConnectedOnTop) != 0; }

    /** Indicates whether the button adjoins another one on its bottom edge.
        @see setConnectedEdges
    */
    bool isConnectedOnBottom() const noexcept           { return (connectedEdgeFlags & ConnectedOnBottom) != 0; }


    //==============================================================================
    /** Used by setState(). */
    enum ButtonState
    {
        buttonNormal,
        buttonOver,
        buttonDown
    };

    /** Can be used to force the button into a particular state.

        This only changes the button's appearance, it won't trigger a click, or stop any mouse-clicks
        from happening.

        The state that you set here will only last until it is automatically changed when the mouse
        enters or exits the button, or the mouse-button is pressed or released.
    */
    void setState (ButtonState newState);

    /** Returns the button's current over/down/up state. */
    ButtonState getState() const noexcept               { return buttonState; }

    //==============================================================================
    /** This abstract base class is implemented by LookAndFeel classes to provide
        button-drawing functionality.
    */
    struct JUCE_API  LookAndFeelMethods
    {
        virtual ~LookAndFeelMethods() = default;

        virtual void drawButtonBackground (Graphics&, Button&, const Colour& backgroundColour,
                                           bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) = 0;

        virtual Font getTextButtonFont (TextButton&, int buttonHeight) = 0;
        virtual int getTextButtonWidthToFitText (TextButton&, int buttonHeight) = 0;

        /** Draws the text for a TextButton. */
        virtual void drawButtonText (Graphics&, TextButton&,
                                     bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) = 0;

        /** Draws the contents of a standard ToggleButton. */
        virtual void drawToggleButton (Graphics&, ToggleButton&,
                                       bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) = 0;

        virtual void changeToggleButtonWidthToFitText (ToggleButton&) = 0;

        virtual void drawTickBox (Graphics&, Component&, float x, float y, float w, float h,
                                  bool ticked, bool isEnabled,
                                  bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) = 0;

        virtual void drawDrawableButton (Graphics&, DrawableButton&,
                                         bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) = 0;
    };

    //==============================================================================
    /** @cond */
    [[deprecated ("This method's parameters have changed.")]]
    void setToggleState (bool, bool);
    /** @endcond */

protected:
    //==============================================================================
    /** This method is called when the button has been clicked.

        Subclasses can override this to perform whatever actions they need to do.
        In general, you wouldn't use this method to receive clicks, but should get your callbacks
        by attaching a std::function to the onClick callback, or adding a Button::Listener.
        @see triggerClick, onClick
    */
    virtual void clicked();

    /** This method is called when the button has been clicked.

        By default it just calls clicked(), but you might want to override it to handle
        things like clicking when a modifier key is pressed, etc.

        @see ModifierKeys
    */
    virtual void clicked (const ModifierKeys& modifiers);

    /** Subclasses should override this to actually paint the button's contents.

        It's better to use this than the paint method, because it gives you information
        about the over/down state of the button.

        @param g                                the graphics context to use
        @param shouldDrawButtonAsHighlighted    true if the button is either in the 'over' or 'down' state
        @param shouldDrawButtonAsDown           true if the button should be drawn in the 'down' position
    */
    virtual void paintButton (Graphics& g,
                              bool shouldDrawButtonAsHighlighted,
                              bool shouldDrawButtonAsDown) = 0;

    /** Called when the button's up/down/over state changes.

        Subclasses can override this if they need to do something special when the button
        goes up or down.

        @see isDown, isOver
    */
    virtual void buttonStateChanged();

    //==============================================================================
    /** @internal */
    virtual void internalClickCallback (const ModifierKeys&);
    /** @internal */
    void handleCommandMessage (int commandId) override;
    /** @internal */
    void mouseEnter (const MouseEvent&) override;
    /** @internal */
    void mouseExit (const MouseEvent&) override;
    /** @internal */
    void mouseDown (const MouseEvent&) override;
    /** @internal */
    void mouseDrag (const MouseEvent&) override;
    /** @internal */
    void mouseUp (const MouseEvent&) override;
    /** @internal */
    bool keyPressed (const KeyPress&) override;
    /** @internal */
    using Component::keyStateChanged;
    /** @internal */
    void paint (Graphics&) override;
    /** @internal */
    void parentHierarchyChanged() override;
    /** @internal */
    void visibilityChanged() override;
    /** @internal */
    void focusGained (FocusChangeType) override;
    /** @internal */
    void focusLost (FocusChangeType) override;
    /** @internal */
    void enablementChanged() override;
    /** @internal */
    std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override;

private:
    //==============================================================================
    Array<KeyPress> shortcuts;
    WeakReference<Component> keySource;
    String text;
    ListenerList<Listener> buttonListeners;

    struct CallbackHelper;
    std::unique_ptr<CallbackHelper> callbackHelper;
    uint32 buttonPressTime = 0, lastRepeatTime = 0;
    ApplicationCommandManager* commandManagerToUse = nullptr;
    int autoRepeatDelay = -1, autoRepeatSpeed = 0, autoRepeatMinimumDelay = -1;
    int radioGroupId = 0, connectedEdgeFlags = 0;
    CommandID commandID = {};
    ButtonState buttonState = buttonNormal, lastStatePainted = buttonNormal;

    Value isOn;
    bool canBeToggled = false;
    bool lastToggleState = false;
    bool clickTogglesState = false;
    bool needsToRelease = false;
    bool needsRepainting = false;
    bool isKeyDown = false;
    bool triggerOnMouseDown = false;
    bool generateTooltip = false;

    void checkToggleableState (bool wasToggleable);

    void repeatTimerCallback();
    bool keyStateChangedCallback();
    void applicationCommandListChangeCallback();
    void updateAutomaticTooltip (const ApplicationCommandInfo&);

    ButtonState updateState();
    ButtonState updateState (bool isOver, bool isDown);
    bool isShortcutPressed() const;
    void turnOffOtherButtonsInGroup (NotificationType click, NotificationType state);

    void flashButtonState();
    void sendClickMessage (const ModifierKeys&);
    void sendStateMessage();
    void setToggleState (bool shouldBeOn, NotificationType click, NotificationType state);

    bool isMouseSourceOver (const MouseEvent& e);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Button)
};


} // namespace juce
