/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

#ifndef JUCE_COMPONENTPEER_H_INCLUDED
#define JUCE_COMPONENTPEER_H_INCLUDED


//==============================================================================
/**
    The Component class uses a ComponentPeer internally to create and manage a real
    operating-system window.

    This is an abstract base class - the platform specific code contains implementations of
    it for the various platforms.

    User-code should very rarely need to have any involvement with this class.

    @see Component::createNewPeer
*/
class JUCE_API  ComponentPeer
{
public:
    //==============================================================================
    /** A combination of these flags is passed to the ComponentPeer constructor. */
    enum StyleFlags
    {
        windowAppearsOnTaskbar      = (1 << 0),    /**< Indicates that the window should have a corresponding
                                                        entry on the taskbar (ignored on MacOSX) */
        windowIsTemporary           = (1 << 1),    /**< Indicates that the window is a temporary popup, like a menu,
                                                        tooltip, etc. */
        windowIgnoresMouseClicks    = (1 << 2),    /**< Indicates that the window should let mouse clicks pass
                                                        through it (may not be possible on some platforms). */
        windowHasTitleBar           = (1 << 3),    /**< Indicates that the window should have a normal OS-specific
                                                        title bar and frame. if not specified, the window will be
                                                        borderless. */
        windowIsResizable           = (1 << 4),    /**< Indicates that the window should have a resizable border. */
        windowHasMinimiseButton     = (1 << 5),    /**< Indicates that if the window has a title bar, it should have a
                                                        minimise button on it. */
        windowHasMaximiseButton     = (1 << 6),    /**< Indicates that if the window has a title bar, it should have a
                                                        maximise button on it. */
        windowHasCloseButton        = (1 << 7),    /**< Indicates that if the window has a title bar, it should have a
                                                        close button on it. */
        windowHasDropShadow         = (1 << 8),    /**< Indicates that the window should have a drop-shadow (this may
                                                        not be possible on all platforms). */
        windowRepaintedExplictly    = (1 << 9),    /**< Not intended for public use - this tells a window not to
                                                        do its own repainting, but only to repaint when the
                                                        performAnyPendingRepaintsNow() method is called. */
        windowIgnoresKeyPresses     = (1 << 10),   /**< Tells the window not to catch any keypresses. This can
                                                        be used for things like plugin windows, to stop them interfering
                                                        with the host's shortcut keys */
        windowIsSemiTransparent     = (1 << 31)    /**< Not intended for public use - makes a window transparent. */

    };

    //==============================================================================
    /** Creates a peer.

        The component is the one that we intend to represent, and the style flags are
        a combination of the values in the StyleFlags enum
    */
    ComponentPeer (Component& component, int styleFlags);

    /** Destructor. */
    virtual ~ComponentPeer();

    //==============================================================================
    /** Returns the component being represented by this peer. */
    Component& getComponent() noexcept                      { return component; }

    /** Returns the set of style flags that were set when the window was created.
        @see Component::addToDesktop
    */
    int getStyleFlags() const noexcept                      { return styleFlags; }

    /** Returns a unique ID for this peer.
        Each peer that is created is given a different ID.
    */
    uint32 getUniqueID() const noexcept                     { return uniqueID; }

    //==============================================================================
    /** Returns the raw handle to whatever kind of window is being used.

        On windows, this is probably a HWND, on the mac, it's likely to be a WindowRef,
        but remember there's no guarantees what you'll get back.
    */
    virtual void* getNativeHandle() const = 0;

    /** Shows or hides the window. */
    virtual void setVisible (bool shouldBeVisible) = 0;

    /** Changes the title of the window. */
    virtual void setTitle (const String& title) = 0;

    /** If this type of window is capable of indicating that the document in it has been
        edited, then this changes its status.

        For example in OSX, this changes the appearance of the close button.
        @returns true if the window has a mechanism for showing this, or false if not.
    */
    virtual bool setDocumentEditedStatus (bool edited);

    /** If this type of window is capable of indicating that it represents a file, then
        this lets you set the file.

        E.g. in OSX it'll show an icon for the file in the title bar.
    */
    virtual void setRepresentedFile (const File&);

    //==============================================================================
    /** Moves and resizes the window.

        If the native window is contained in another window, then the coordinates are
        relative to the parent window's origin, not the screen origin.

        This should result in a callback to handleMovedOrResized().
    */
    virtual void setBounds (const Rectangle<int>& newBounds, bool isNowFullScreen) = 0;

    /** Updates the peer's bounds to match its component. */
    void updateBounds();

    /** Returns the current position and size of the window.

        If the native window is contained in another window, then the coordinates are
        relative to the parent window's origin, not the screen origin.
    */
    virtual Rectangle<int> getBounds() const = 0;

    /** Converts a position relative to the top-left of this component to screen coordinates. */
    virtual Point<int> localToGlobal (Point<int> relativePosition) = 0;

    /** Converts a rectangle relative to the top-left of this component to screen coordinates. */
    virtual Rectangle<int> localToGlobal (const Rectangle<int>& relativePosition);

    /** Converts a screen coordinate to a position relative to the top-left of this component. */
    virtual Point<int> globalToLocal (Point<int> screenPosition) = 0;

    /** Converts a screen area to a position relative to the top-left of this component. */
    virtual Rectangle<int> globalToLocal (const Rectangle<int>& screenPosition);

    /** Returns the area in peer coordinates that is covered by the given sub-comp (which
        may be at any depth)
    */
    Rectangle<int> getAreaCoveredBy (Component& subComponent) const;

    /** Minimises the window. */
    virtual void setMinimised (bool shouldBeMinimised) = 0;

    /** True if the window is currently minimised. */
    virtual bool isMinimised() const = 0;

    /** Enable/disable fullscreen mode for the window. */
    virtual void setFullScreen (bool shouldBeFullScreen) = 0;

    /** True if the window is currently full-screen. */
    virtual bool isFullScreen() const = 0;

    /** True if the window is in kiosk-mode. */
    virtual bool isKioskMode() const;

    /** Sets the size to restore to if fullscreen mode is turned off. */
    void setNonFullScreenBounds (const Rectangle<int>& newBounds) noexcept;

    /** Returns the size to restore to if fullscreen mode is turned off. */
    const Rectangle<int>& getNonFullScreenBounds() const noexcept;

    /** Attempts to change the icon associated with this window. */
    virtual void setIcon (const Image& newIcon) = 0;

    /** Sets a constrainer to use if the peer can resize itself.
        The constrainer won't be deleted by this object, so the caller must manage its lifetime.
    */
    void setConstrainer (ComponentBoundsConstrainer* newConstrainer) noexcept;

    /** Returns the current constrainer, if one has been set. */
    ComponentBoundsConstrainer* getConstrainer() const noexcept             { return constrainer; }

    /** Checks if a point is in the window.

        The position is relative to the top-left of this window, in unscaled peer coordinates.
        If trueIfInAChildWindow is false, then this returns false if the point is actually
        inside a child of this window.
    */
    virtual bool contains (Point<int> localPos, bool trueIfInAChildWindow) const = 0;

    /** Returns the size of the window frame that's around this window.
        Whether or not the window has a normal window frame depends on the flags
        that were set when the window was created by Component::addToDesktop()
    */
    virtual BorderSize<int> getFrameSize() const = 0;

    /** This is called when the window's bounds change.
        A peer implementation must call this when the window is moved and resized, so that
        this method can pass the message on to the component.
    */
    void handleMovedOrResized();

    /** This is called if the screen resolution changes.
        A peer implementation must call this if the monitor arrangement changes or the available
        screen size changes.
    */
    virtual void handleScreenSizeChange();

    //==============================================================================
    /** This is called to repaint the component into the given context. */
    void handlePaint (LowLevelGraphicsContext& contextToPaintTo);

    //==============================================================================
    /** Sets this window to either be always-on-top or normal.
        Some kinds of window might not be able to do this, so should return false.
    */
    virtual bool setAlwaysOnTop (bool alwaysOnTop) = 0;

    /** Brings the window to the top, optionally also giving it focus. */
    virtual void toFront (bool makeActive) = 0;

    /** Moves the window to be just behind another one. */
    virtual void toBehind (ComponentPeer* other) = 0;

    /** Called when the window is brought to the front, either by the OS or by a call
        to toFront().
    */
    void handleBroughtToFront();

    //==============================================================================
    /** True if the window has the keyboard focus. */
    virtual bool isFocused() const = 0;

    /** Tries to give the window keyboard focus. */
    virtual void grabFocus() = 0;

    /** Called when the window gains keyboard focus. */
    void handleFocusGain();
    /** Called when the window loses keyboard focus. */
    void handleFocusLoss();

    Component* getLastFocusedSubcomponent() const noexcept;

    /** Called when a key is pressed.
        For keycode info, see the KeyPress class.
        Returns true if the keystroke was used.
    */
    bool handleKeyPress (int keyCode, juce_wchar textCharacter);

    /** Called whenever a key is pressed or released.
        Returns true if the keystroke was used.
    */
    bool handleKeyUpOrDown (bool isKeyDown);

    /** Called whenever a modifier key is pressed or released. */
    void handleModifierKeysChange();

    //==============================================================================
    /** Tells the window that text input may be required at the given position.
        This may cause things like a virtual on-screen keyboard to appear, depending
        on the OS.
    */
    virtual void textInputRequired (Point<int> position, TextInputTarget&) = 0;

    /** If there's some kind of OS input-method in progress, this should dismiss it. */
    virtual void dismissPendingTextInput();

    /** Returns the currently focused TextInputTarget, or null if none is found. */
    TextInputTarget* findCurrentTextInputTarget();

    //==============================================================================
    /** Invalidates a region of the window to be repainted asynchronously. */
    virtual void repaint (const Rectangle<int>& area) = 0;

    /** This can be called (from the message thread) to cause the immediate redrawing
        of any areas of this window that need repainting.

        You shouldn't ever really need to use this, it's mainly for special purposes
        like supporting audio plugins where the host's event loop is out of our control.
    */
    virtual void performAnyPendingRepaintsNow() = 0;

    /** Changes the window's transparency. */
    virtual void setAlpha (float newAlpha) = 0;

    //==============================================================================
    void handleMouseEvent (int touchIndex, const Point<int> positionWithinPeer, const ModifierKeys newMods, int64 time);
    void handleMouseWheel (int touchIndex, const Point<int> positionWithinPeer, int64 time, const MouseWheelDetails&);
    void handleMagnifyGesture (int touchIndex, const Point<int> positionWithinPeer, int64 time, float scaleFactor);

    void handleUserClosingWindow();

    struct DragInfo
    {
        StringArray files;
        String text;
        Point<int> position;

        bool isEmpty() const noexcept       { return files.size() == 0 && text.isEmpty(); }
        void clear() noexcept               { files.clear(); text.clear(); }
    };

    bool handleDragMove (const DragInfo&);
    bool handleDragExit (const DragInfo&);
    bool handleDragDrop (const DragInfo&);

    //==============================================================================
    /** Returns the number of currently-active peers.
        @see getPeer
    */
    static int getNumPeers() noexcept;

    /** Returns one of the currently-active peers.
        @see getNumPeers
    */
    static ComponentPeer* getPeer (int index) noexcept;

    /** Returns the peer that's attached to the given component, or nullptr if there isn't one. */
    static ComponentPeer* getPeerFor (const Component*) noexcept;

    /** Checks if this peer object is valid.
        @see getNumPeers
    */
    static bool isValidPeer (const ComponentPeer* peer) noexcept;

    //==============================================================================
    virtual StringArray getAvailableRenderingEngines() = 0;
    virtual int getCurrentRenderingEngine() const;
    virtual void setCurrentRenderingEngine (int index);

protected:
    //==============================================================================
    Component& component;
    const int styleFlags;
    Rectangle<int> lastNonFullscreenBounds;
    ComponentBoundsConstrainer* constrainer;

private:
    //==============================================================================
    WeakReference<Component> lastFocusedComponent, dragAndDropTargetComponent;
    Component* lastDragAndDropCompUnderMouse;
    const uint32 uniqueID;
    bool isWindowMinimised;
    Component* getTargetForKeyPress();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ComponentPeer)
};


#endif   // JUCE_COMPONENTPEER_H_INCLUDED
