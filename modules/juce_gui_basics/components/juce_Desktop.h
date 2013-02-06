/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#ifndef __JUCE_DESKTOP_JUCEHEADER__
#define __JUCE_DESKTOP_JUCEHEADER__

#include "juce_Component.h"
#include "../layout/juce_ComponentAnimator.h"
class MouseInputSource;
class MouseInputSourceInternal;
class MouseListener;


//==============================================================================
/**
    Classes can implement this interface and register themselves with the Desktop class
    to receive callbacks when the currently focused component changes.

    @see Desktop::addFocusChangeListener, Desktop::removeFocusChangeListener
*/
class JUCE_API  FocusChangeListener
{
public:
    /** Destructor. */
    virtual ~FocusChangeListener()  {}

    /** Callback to indicate that the currently focused component has changed. */
    virtual void globalFocusChanged (Component* focusedComponent) = 0;
};


//==============================================================================
/**
    Describes and controls aspects of the computer's desktop.

*/
class JUCE_API  Desktop  : private DeletedAtShutdown,
                           private Timer,
                           private AsyncUpdater
{
public:
    //==============================================================================
    /** There's only one dektop object, and this method will return it.
    */
    static Desktop& JUCE_CALLTYPE getInstance();

    //==============================================================================
    /** Returns the mouse position.

        The co-ordinates are relative to the top-left of the main monitor.

        Note that this is just a shortcut for calling getMainMouseSource().getScreenPosition(), and
        you should only resort to grabbing the global mouse position if there's really no
        way to get the coordinates via a mouse event callback instead.
    */
    static Point<int> getMousePosition();

    /** Makes the mouse pointer jump to a given location.
        The co-ordinates are relative to the top-left of the main monitor.
    */
    static void setMousePosition (const Point<int>& newPosition);

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

        The disablement will only happen while the Juce application is the foreground
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

    /** Unregisters a MouseListener that was added with the addGlobalMouseListener()
        method.

        @see addGlobalMouseListener
    */
    void removeGlobalMouseListener (MouseListener* listener);

    //==============================================================================
    /** Registers a MouseListener that will receive a callback whenever the focused
        component changes.
    */
    void addFocusChangeListener (FocusChangeListener* listener);

    /** Unregisters a listener that was added with addFocusChangeListener(). */
    void removeFocusChangeListener (FocusChangeListener* listener);

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
        to hide as much on-screen paraphenalia as possible.
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

        Returns nullptr if the co-ordinates are inside a non-Juce window.
    */
    Component* findComponentAt (const Point<int>& screenPosition) const;

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
    /** Returns the number of MouseInputSource objects the system has at its disposal.
        In a traditional single-mouse system, there might be only one object. On a multi-touch
        system, there could be one input source per potential finger. The number of mouse
        sources returned here may increase dynamically as the program runs.
        To find out how many mouse events are currently happening, use getNumDraggingMouseSources().
        @see getMouseSource
    */
    int getNumMouseSources() const noexcept                         { return mouseSources.size(); }

    /** Returns one of the system's MouseInputSource objects.
        The index should be from 0 to getNumMouseSources() - 1. Out-of-range indexes will return
        a null pointer.
        In a traditional single-mouse system, there might be only one object. On a multi-touch
        system, there could be one input source per potential finger.
    */
    MouseInputSource* getMouseSource (int index) const noexcept     { return mouseSources [index]; }

    /** Returns the main mouse input device that the system is using.
        @see getNumMouseSources()
    */
    MouseInputSource& getMainMouseSource() const noexcept           { return *mouseSources.getUnchecked(0); }

    /** Returns the number of mouse-sources that are currently being dragged.
        In a traditional single-mouse system, this will be 0 or 1, depending on whether a
        juce component has the button down on it. In a multi-touch system, this could
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
    /** In a tablet device which can be turned around, this is used to inidicate the orientation. */
    enum DisplayOrientation
    {
        upright                 = 1,  /**< Indicates that the display is the normal way up. */
        upsideDown              = 2,  /**< Indicates that the display is upside-down. */
        rotatedClockwise        = 4,  /**< Indicates that the display is turned 90 degrees clockwise from its upright position. */
        rotatedAntiClockwise    = 8,  /**< Indicates that the display is turned 90 degrees anti-clockwise from its upright position. */

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

    /** Returns whether the display is allowed to auto-rotate to the given orientation.
        Each orientation can be enabled using setOrientationEnabled(). By default, all orientations are allowed.
    */
    bool isOrientationEnabled (DisplayOrientation orientation) const noexcept;

    //==============================================================================
    class JUCE_API  Displays
    {
    public:
        /** Contains details about a display device. */
        struct Display
        {
            /** This is the bounds of the area of this display which isn't covered by
                OS-dependent objects like the taskbar, menu bar, etc. */
            Rectangle<int> userArea;

            /** This is the total physical area of this display, including any taskbars, etc */
            Rectangle<int> totalArea;

            /** This is the scale-factor of this display.
                For higher-resolution displays, this may be a value greater than 1.0
            */
            double scale;

            /** This will be true if this is the user's main screen. */
            bool isMain;
        };

        /** Returns the display which acts as user's main screen. */
        const Display& getMainDisplay() const noexcept;

        /** Returns the display which contains a particular point.
            If the point lies outside all the displays, the nearest one will be returned.
        */
        const Display& getDisplayContaining (const Point<int>& position) const noexcept;

        /** Returns a RectangleList made up of all the displays. */
        RectangleList getRectangleList (bool userAreasOnly) const;

        /** Returns the smallest bounding box which contains all the displays. */
        Rectangle<int> getTotalBounds (bool userAreasOnly) const;

        /** The list of displays. */
        Array<Display> displays;

       #ifndef DOXYGEN
        /** @internal */
        void refresh();
       #endif

    private:
        friend class Desktop;
        Displays();
        ~Displays();
        void findDisplays();
    };

    const Displays& getDisplays() const noexcept       { return displays; }

    //==============================================================================
    /** True if the OS supports semitransparent windows */
    static bool canUseSemiTransparentWindows() noexcept;

private:
    //==============================================================================
    static Desktop* instance;

    friend class Component;
    friend class ComponentPeer;
    friend class MouseInputSource;
    friend class MouseInputSourceInternal;
    friend class DeletedAtShutdown;
    friend class TopLevelWindowManager;

    OwnedArray <MouseInputSource> mouseSources;
    bool addMouseInputSource();

    ListenerList <MouseListener> mouseListeners;
    ListenerList <FocusChangeListener> focusListeners;

    Array <Component*> desktopComponents;

    Displays displays;

    Point<int> lastFakeMouseMove;
    void sendMouseMove();

    int mouseClickCounter, mouseWheelCounter;
    void incrementMouseClickCounter() noexcept;
    void incrementMouseWheelCounter() noexcept;

    ScopedPointer<Timer> dragRepeater;

    ScopedPointer<LookAndFeel> defaultLookAndFeel;
    WeakReference<LookAndFeel> currentLookAndFeel;

    Component* kioskModeComponent;
    Rectangle<int> kioskComponentOriginalBounds;

    int allowedOrientations;

    ComponentAnimator animator;

    void timerCallback();
    void resetTimer();
    ListenerList <MouseListener>& getMouseListeners();

    void addDesktopComponent (Component*);
    void removeDesktopComponent (Component*);
    void componentBroughtToFront (Component*);

    void setKioskComponent (Component*, bool enableOrDisable, bool allowMenusAndBars);

    void triggerFocusCallback();
    void handleAsyncUpdate();

    Desktop();
    ~Desktop();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Desktop)
};


#endif   // __JUCE_DESKTOP_JUCEHEADER__
