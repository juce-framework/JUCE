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

//==============================================================================
/**
    Classes can implement this interface and register themselves with the Desktop class
    to receive callbacks when the currently focused component changes.

    @see Desktop::addFocusChangeListener, Desktop::removeFocusChangeListener

    @tags{GUI}
*/
class JUCE_API  FocusChangeListener
{
public:
    /** Destructor. */
    virtual ~FocusChangeListener() = default;

    /** Callback to indicate that the currently focused component has changed. */
    virtual void globalFocusChanged (Component* focusedComponent) = 0;
};

//==============================================================================
/**
    Classes can implement this interface and register themselves with the Desktop class
    to receive callbacks when the operating system dark mode setting changes. The
    Desktop::isDarkModeActive() method can then be used to query the current setting.

    @see Desktop::addDarkModeSettingListener, Desktop::removeDarkModeSettingListener,
         Desktop::isDarkModeActive

    @tags{GUI}
*/
class JUCE_API  DarkModeSettingListener
{
public:
    /** Destructor. */
    virtual ~DarkModeSettingListener() = default;

    /** Callback to indicate that the dark mode setting has changed. */
    virtual void darkModeSettingChanged() = 0;
};

//==============================================================================
/**
    Describes and controls aspects of the computer's desktop.

    @tags{GUI}
*/
class JUCE_API  Desktop  : private DeletedAtShutdown,
                           private Timer,
                           private AsyncUpdater
{
public:
    //==============================================================================
    /** There's only one desktop object, and this method will return it. */
    static Desktop& JUCE_CALLTYPE getInstance();

    //==============================================================================
    /** Returns the mouse position.

        The coordinates are relative to the top-left of the main monitor.

        Note that this is just a shortcut for calling getMainMouseSource().getScreenPosition(), and
        you should only resort to grabbing the global mouse position if there's really no
        way to get the coordinates via a mouse event callback instead.
    */
    static Point<int> getMousePosition();

    /** Makes the mouse pointer jump to a given location.
        The coordinates are relative to the top-left of the main monitor.
        Note that this is a pretty old method, kept around mainly for backwards-compatibility,
        and you should use the MouseInputSource class directly in new code.
    */
    static void setMousePosition (Point<int> newPosition);

    /** Returns the last position at which a mouse button was pressed.

        Note that this is just a shortcut for calling getMainMouseSource().getLastMouseDownPosition(),
        and in a multi-touch environment, it doesn't make much sense. ALWAYS prefer to
        get this information via other means, such as MouseEvent::getMouseDownScreenPosition()
        if possible, and only ever call this as a last resort.
    */
    static Point<int> getLastMouseDownPosition();

    /** Returns the number of times the mouse button has been clicked since the app started.
        Each mouse-down event increments this number by 1.
        @see getMouseWheelMoveCounter
    */
    int getMouseButtonClickCounter() const noexcept;

    /** Returns the number of times the mouse wheel has been moved since the app started.
        Each mouse-wheel event increments this number by 1.
        @see getMouseButtonClickCounter
    */
    int getMouseWheelMoveCounter() const noexcept;

    //==============================================================================
    /** This lets you prevent the screensaver from becoming active.

        Handy if you're running some sort of presentation app where having a screensaver
        appear would be annoying.

        Pass false to disable the screensaver, and true to re-enable it. (Note that this
        won't enable a screensaver unless the user has actually set one up).

        The disablement will only happen while the JUCE application is the foreground
        process - if another task is running in front of it, then the screensaver will
        be unaffected.

        @see isScreenSaverEnabled
    */
    static void setScreenSaverEnabled (bool isEnabled);

    /** Returns true if the screensaver has not been turned off.

        This will return the last value passed into setScreenSaverEnabled(). Note that
        it won't tell you whether the user is actually using a screen saver, just
        whether this app is deliberately preventing one from running.

        @see setScreenSaverEnabled
    */
    static bool isScreenSaverEnabled();

    //==============================================================================
    /** Registers a MouseListener that will receive all mouse events that occur on
        any component.

        @see removeGlobalMouseListener
    */
    void addGlobalMouseListener (MouseListener* listener);

    /** Unregisters a MouseListener that was added with addGlobalMouseListener().

        @see addGlobalMouseListener
    */
    void removeGlobalMouseListener (MouseListener* listener);

    //==============================================================================
    /** Registers a FocusChangeListener that will receive a callback whenever the focused
        component changes.

        @see removeFocusChangeListener
    */
    void addFocusChangeListener (FocusChangeListener* listener);

    /** Unregisters a FocusChangeListener that was added with addFocusChangeListener().

        @see addFocusChangeListener
    */
    void removeFocusChangeListener (FocusChangeListener* listener);

    //==============================================================================
    /** Registers a DarkModeSettingListener that will receive a callback when the
        operating system dark mode setting changes. To query whether dark mode is on
        use the isDarkModeActive() method.

        @see isDarkModeActive, removeDarkModeSettingListener
    */
    void addDarkModeSettingListener (DarkModeSettingListener* listener);

    /** Unregisters a DarkModeSettingListener that was added with addDarkModeSettingListener().

        @see addDarkModeSettingListener
    */
    void removeDarkModeSettingListener (DarkModeSettingListener* listener);

    /** True if the operating system "dark mode" is active.

        To receive a callback when this setting changes implement the DarkModeSettingListener
        interface and use the addDarkModeSettingListener() to register a listener.

        @see addDarkModeSettingListener, removeDarkModeSettingListener
    */
    bool isDarkModeActive() const;

    //==============================================================================
    /** Takes a component and makes it full-screen, removing the taskbar, dock, etc.

        The component must already be on the desktop for this method to work. It will
        be resized to completely fill the screen and any extraneous taskbars, menu bars,
        etc will be hidden.

        To exit kiosk mode, just call setKioskModeComponent (nullptr). When this is called,
        the component that's currently being used will be resized back to the size
        and position it was in before being put into this mode.

        If allowMenusAndBars is true, things like the menu and dock (on mac) are still
        allowed to pop up when the mouse moves onto them. If this is false, it'll try
        to hide as much on-screen paraphernalia as possible.
    */
    void setKioskModeComponent (Component* componentToUse,
                                bool allowMenusAndBars = true);

    /** Returns the component that is currently being used in kiosk-mode.

        This is the component that was last set by setKioskModeComponent(). If none
        has been set, this returns nullptr.
    */
    Component* getKioskModeComponent() const noexcept               { return kioskModeComponent; }

    //==============================================================================
    /** Returns the number of components that are currently active as top-level
        desktop windows.

        @see getComponent, Component::addToDesktop
    */
    int getNumComponents() const noexcept;

    /** Returns one of the top-level desktop window components.

        The index is from 0 to getNumComponents() - 1. This could return 0 if the
        index is out-of-range.

        @see getNumComponents, Component::addToDesktop
    */
    Component* getComponent (int index) const noexcept;

    /** Finds the component at a given screen location.

        This will drill down into top-level windows to find the child component at
        the given position.

        Returns nullptr if the coordinates are inside a non-JUCE window.
    */
    Component* findComponentAt (Point<int> screenPosition) const;

    /** The Desktop object has a ComponentAnimator instance which can be used for performing
        your animations.

        Having a single shared ComponentAnimator object makes it more efficient when multiple
        components are being moved around simultaneously. It's also more convenient than having
        to manage your own instance of one.

        @see ComponentAnimator
    */
    ComponentAnimator& getAnimator() noexcept                       { return animator; }

    //==============================================================================
    /** Returns the current default look-and-feel for components which don't have one
        explicitly set.
        @see setDefaultLookAndFeel
    */
    LookAndFeel& getDefaultLookAndFeel() noexcept;

    /** Changes the default look-and-feel.
        @param newDefaultLookAndFeel    the new look-and-feel object to use - if this is
                                        set to nullptr, it will revert to using the system's
                                        default one. The object passed-in must be deleted by the
                                        caller when it's no longer needed.
        @see getDefaultLookAndFeel
    */
    void setDefaultLookAndFeel (LookAndFeel* newDefaultLookAndFeel);

    //==============================================================================
    /** Provides access to the array of mouse sources, for iteration.
        In a traditional single-mouse system, there might be only one MouseInputSource. On a
        multi-touch system, there could be one input source per potential finger. The number
        of mouse sources returned here may increase dynamically as the program runs.
        To find out how many mouse events are currently happening, use getNumDraggingMouseSources().
    */
    const Array<MouseInputSource>& getMouseSources() const noexcept;

    /** Returns the number of MouseInputSource objects the system has at its disposal.
        In a traditional single-mouse system, there might be only one MouseInputSource. On a
        multi-touch system, there could be one input source per potential finger. The number
        of mouse sources returned here may increase dynamically as the program runs.
        To find out how many mouse events are currently happening, use getNumDraggingMouseSources().
        @see getMouseSource
    */
    int getNumMouseSources() const noexcept;

    /** Returns one of the system's MouseInputSource objects.
        The index should be from 0 to getNumMouseSources() - 1. Out-of-range indexes will return
        a null pointer.
        In a traditional single-mouse system, there might be only one object. On a multi-touch
        system, there could be one input source per potential finger.
    */
    MouseInputSource* getMouseSource (int index) const noexcept;

    /** Returns the main mouse input device that the system is using.
        @see getNumMouseSources()
    */
    MouseInputSource getMainMouseSource() const noexcept;

    /** Returns the number of mouse-sources that are currently being dragged.
        In a traditional single-mouse system, this will be 0 or 1, depending on whether a
        JUCE component has the button down on it. In a multi-touch system, this could
        be any number from 0 to the number of simultaneous touches that can be detected.
    */
    int getNumDraggingMouseSources() const noexcept;

    /** Returns one of the mouse sources that's currently being dragged.
        The index should be between 0 and getNumDraggingMouseSources() - 1. If the index is
        out of range, or if no mice or fingers are down, this will return a null pointer.
    */
    MouseInputSource* getDraggingMouseSource (int index) const noexcept;

    /** Ensures that a non-stop stream of mouse-drag events will be sent during the
        current mouse-drag operation.

        This allows you to make sure that mouseDrag() events are sent continuously, even
        when the mouse isn't moving. This can be useful for things like auto-scrolling
        components when the mouse is near an edge.

        Call this method during a mouseDown() or mouseDrag() callback, specifying the
        minimum interval between consecutive mouse drag callbacks. The callbacks
        will continue until the mouse is released, and then the interval will be reset,
        so you need to make sure it's called every time you begin a drag event.
        Passing an interval of 0 or less will cancel the auto-repeat.

        @see mouseDrag
    */
    void beginDragAutoRepeat (int millisecondsBetweenCallbacks);

    //==============================================================================
    /** In a tablet/mobile device which can be turned around, this is used to indicate the orientation. */
    enum DisplayOrientation
    {
        upright                 = 1,  /**< Indicates that the device is the normal way up. */
        upsideDown              = 2,  /**< Indicates that the device is upside-down. */
        rotatedClockwise        = 4,  /**< Indicates that the device is turned 90 degrees clockwise from its upright position. */
        rotatedAntiClockwise    = 8,  /**< Indicates that the device is turned 90 degrees anti-clockwise from its upright position. */

        allOrientations         = 1 + 2 + 4 + 8   /**< A combination of all the orientation values */
    };

    /** In a tablet device which can be turned around, this returns the current orientation. */
    DisplayOrientation getCurrentOrientation() const;

    /** Sets which orientations the display is allowed to auto-rotate to.

        For devices that support rotating desktops, this lets you specify which of the orientations your app can use.

        The parameter is a bitwise or-ed combination of the values in DisplayOrientation, and must contain at least one
        set bit.
    */
    void setOrientationsEnabled (int allowedOrientations);

    /** Returns the set of orientations the display is allowed to rotate to.
        @see setOrientationsEnabled
    */
    int getOrientationsEnabled() const noexcept;

    /** Returns whether the display is allowed to auto-rotate to the given orientation.
        Each orientation can be enabled using setOrientationEnabled(). By default, all orientations are allowed.
    */
    bool isOrientationEnabled (DisplayOrientation orientation) const noexcept;

    //==============================================================================
    /** Returns the Displays object representing the connected displays.

        @see Displays
    */
    const Displays& getDisplays() const noexcept        { return *displays; }

    //==============================================================================
    /** Sets a global scale factor to be used for all desktop windows.
        Setting this will also scale the monitor sizes that are returned by getDisplays().
    */
    void setGlobalScaleFactor (float newScaleFactor) noexcept;

    /** Returns the current global scale factor, as set by setGlobalScaleFactor().
        @see setGlobalScaleFactor
    */
    float getGlobalScaleFactor() const noexcept         { return masterScaleFactor; }

    //==============================================================================
    /** True if the OS supports semitransparent windows */
    static bool canUseSemiTransparentWindows() noexcept;

   #if JUCE_MAC && ! defined (DOXYGEN)
    [[deprecated ("This macOS-specific method has been deprecated in favour of the cross-platform "
                  " isDarkModeActive() method.")]]
    static bool isOSXDarkModeActive()  { return Desktop::getInstance().isDarkModeActive(); }
   #endif

    //==============================================================================
    /** Returns true on a headless system where there are no connected displays. */
    bool isHeadless() const noexcept;

private:
    //==============================================================================
    static Desktop* instance;

    friend class Component;
    friend class ComponentPeer;
    friend class detail::MouseInputSourceImpl;
    friend class DeletedAtShutdown;
    friend class detail::TopLevelWindowManager;
    friend class Displays;

    std::unique_ptr<detail::MouseInputSourceList> mouseSources;

    ListenerList<MouseListener> mouseListeners;
    ListenerList<FocusChangeListener> focusListeners;
    ListenerList<DarkModeSettingListener> darkModeSettingListeners;

    Array<Component*> desktopComponents;
    Array<ComponentPeer*> peers;

    std::unique_ptr<Displays> displays;

    Point<float> lastFakeMouseMove;
    void sendMouseMove();

    int mouseClickCounter = 0, mouseWheelCounter = 0;
    void incrementMouseClickCounter() noexcept;
    void incrementMouseWheelCounter() noexcept;

    std::unique_ptr<LookAndFeel> defaultLookAndFeel;
    WeakReference<LookAndFeel> currentLookAndFeel;

    std::unique_ptr<FocusOutline> focusOutline;

    Component* kioskModeComponent = nullptr;
    Rectangle<int> kioskComponentOriginalBounds;
    bool kioskModeReentrant = false;

    int allowedOrientations = allOrientations;
    void allowedOrientationsChanged();

    float masterScaleFactor;

    ComponentAnimator animator;

    void timerCallback() override;
    void resetTimer();
    ListenerList<MouseListener>& getMouseListeners();

    void addDesktopComponent (Component*);
    void removeDesktopComponent (Component*);
    void componentBroughtToFront (Component*);

    void setKioskComponent (Component*, bool shouldBeEnabled, bool allowMenusAndBars);

    void triggerFocusCallback();
    void updateFocusOutline();
    void handleAsyncUpdate() override;

    static Point<float> getMousePositionFloat();

    static double getDefaultMasterScale();

    Desktop();
    ~Desktop() override;

    //==============================================================================
    class NativeDarkModeChangeDetectorImpl;
    std::unique_ptr<NativeDarkModeChangeDetectorImpl> nativeDarkModeChangeDetectorImpl;

    static std::unique_ptr<NativeDarkModeChangeDetectorImpl> createNativeDarkModeChangeDetectorImpl();
    void darkModeChanged();

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Desktop)
};

} // namespace juce
