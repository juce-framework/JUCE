/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

#ifndef __JUCE_COMPONENTPEER_JUCEHEADER__
#define __JUCE_COMPONENTPEER_JUCEHEADER__

class Component;
class Graphics;
#include "../mouse/juce_MouseCursor.h"
#include "../../../events/juce_MessageListener.h"
#include "../../../text/juce_StringArray.h"
#include "../../graphics/geometry/juce_RectangleList.h"
class ComponentBoundsConstrainer;
class ComponentDeletionWatcher;


//==============================================================================
/**
    The base class for window objects that wrap a component as a real operating
    system object.

    This is an abstract base class - the platform specific code contains default
    implementations of it that create and manage windows.

    @see Component::createNewPeer
*/
class JUCE_API  ComponentPeer    : public MessageListener
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
                                                        title bar and frame\. if not specified, the window will be
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
    ComponentPeer (Component* const component,
                   const int styleFlags) throw();

    /** Destructor. */
    virtual ~ComponentPeer();

    //==============================================================================
    /** Returns the component being represented by this peer. */
    Component* getComponent() const throw()                 { return component; }

    /** Returns the set of style flags that were set when the window was created.

        @see Component::addToDesktop
    */
    int getStyleFlags() const throw()                       { return styleFlags; }


    //==============================================================================
    /** Returns the raw handle to whatever kind of window is being used.

        On windows, this is probably a HWND, on the mac, it's likely to be a WindowRef,
        but rememeber there's no guarantees what you'll get back.
    */
    virtual void* getNativeHandle() const = 0;

    /** Shows or hides the window. */
    virtual void setVisible (bool shouldBeVisible) = 0;

    /** Changes the title of the window. */
    virtual void setTitle (const String& title) = 0;

    /** Moves the window without changing its size.

        If the native window is contained in another window, then the co-ordinates are
        relative to the parent window's origin, not the screen origin.

        This should result in a callback to handleMovedOrResized().
    */
    virtual void setPosition (int x, int y) = 0;

    /** Resizes the window without changing its position.

        This should result in a callback to handleMovedOrResized().
    */
    virtual void setSize (int w, int h) = 0;

    /** Moves and resizes the window.

        If the native window is contained in another window, then the co-ordinates are
        relative to the parent window's origin, not the screen origin.

        This should result in a callback to handleMovedOrResized().
    */
    virtual void setBounds (int x, int y, int w, int h, const bool isNowFullScreen) = 0;

    /** Returns the current position and size of the window.

        If the native window is contained in another window, then the co-ordinates are
        relative to the parent window's origin, not the screen origin.
    */
    virtual void getBounds (int& x, int& y, int& w, int& h) const = 0;

    /** Returns the x-position of this window, relative to the screen's origin. */
    virtual int getScreenX() const = 0;

    /** Returns the y-position of this window, relative to the screen's origin. */
    virtual int getScreenY() const = 0;

    /** Converts a position relative to the top-left of this component to screen co-ordinates. */
    virtual void relativePositionToGlobal (int& x, int& y) = 0;

    /** Converts a screen co-ordinate to a position relative to the top-left of this component. */
    virtual void globalPositionToRelative (int& x, int& y) = 0;

    /** Minimises the window. */
    virtual void setMinimised (bool shouldBeMinimised) = 0;

    /** True if the window is currently minimised. */
    virtual bool isMinimised() const = 0;

    /** Enable/disable fullscreen mode for the window. */
    virtual void setFullScreen (bool shouldBeFullScreen) = 0;

    /** True if the window is currently full-screen. */
    virtual bool isFullScreen() const = 0;

    /** Sets the size to restore to if fullscreen mode is turned off. */
    void setNonFullScreenBounds (const Rectangle& newBounds) throw();

    /** Returns the size to restore to if fullscreen mode is turned off. */
    const Rectangle& getNonFullScreenBounds() const throw();

    /** Attempts to change the icon associated with this window.
    */
    virtual void setIcon (const Image& newIcon) = 0;

    /** Sets a constrainer to use if the peer can resize itself.

        The constrainer won't be deleted by this object, so the caller must manage its lifetime.
    */
    void setConstrainer (ComponentBoundsConstrainer* const newConstrainer) throw();

    /** Returns the current constrainer, if one has been set. */
    ComponentBoundsConstrainer* getConstrainer() const throw()              { return constrainer; }

    /** Checks if a point is in the window.

        Coordinates are relative to the top-left of this window. If trueIfInAChildWindow
        is false, then this returns false if the point is actually inside a child of this
        window.
    */
    virtual bool contains (int x, int y, bool trueIfInAChildWindow) const = 0;

    /** Returns the size of the window frame that's around this window.

        Whether or not the window has a normal window frame depends on the flags
        that were set when the window was created by Component::addToDesktop()
    */
    virtual const BorderSize getFrameSize() const = 0;

    /** This is called when the window's bounds change.

        A peer implementation must call this when the window is moved and resized, so that
        this method can pass the message on to the component.
    */
    void handleMovedOrResized();

    /** This is called if the screen resolution changes.

        A peer implementation must call this if the monitor arrangement changes or the available
        screen size changes.
    */
    void handleScreenSizeChange();

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

    /** Tells the window that text input may be required at the given position.

        This may cause things like a virtual on-screen keyboard to appear, depending
        on the OS.
    */
    virtual void textInputRequired (int x, int y) = 0;

    /** Called when the window gains keyboard focus. */
    void handleFocusGain();
    /** Called when the window loses keyboard focus. */
    void handleFocusLoss();

    Component* getLastFocusedSubcomponent() const throw();

    /** Called when a key is pressed.

        For keycode info, see the KeyPress class.
        Returns true if the keystroke was used.
    */
    bool handleKeyPress (const int keyCode,
                         const juce_wchar textCharacter);

    /** Called whenever a key is pressed or released.
        Returns true if the keystroke was used.
    */
    bool handleKeyUpOrDown (const bool isKeyDown);

    /** Called whenever a modifier key is pressed or released. */
    void handleModifierKeysChange();

    //==============================================================================
    /** Invalidates a region of the window to be repainted asynchronously. */
    virtual void repaint (int x, int y, int w, int h) = 0;

    /** This can be called (from the message thread) to cause the immediate redrawing
        of any areas of this window that need repainting.

        You shouldn't ever really need to use this, it's mainly for special purposes
        like supporting audio plugins where the host's event loop is out of our control.
    */
    virtual void performAnyPendingRepaintsNow() = 0;

    //==============================================================================
    void handleMouseEnter (int x, int y, const int64 time);
    void handleMouseMove  (int x, int y, const int64 time);
    void handleMouseDown  (int x, int y, const int64 time);
    void handleMouseDrag  (int x, int y, const int64 time);
    void handleMouseUp    (const int oldModifiers, int x, int y, const int64 time);
    void handleMouseExit  (int x, int y, const int64 time);
    void handleMouseWheel (const int amountX, const int amountY, const int64 time);

    /** Causes a mouse-move callback to be made asynchronously. */
    void sendFakeMouseMove() throw();

    void handleUserClosingWindow();

    void handleFileDragMove (const StringArray& files, int x, int y);
    void handleFileDragExit (const StringArray& files);
    void handleFileDragDrop (const StringArray& files, int x, int y);

    //==============================================================================
    /** Resets the masking region.

        The subclass should call this every time it's about to call the handlePaint
        method.

        @see addMaskedRegion
    */
    void clearMaskedRegion() throw();

    /** Adds a rectangle to the set of areas not to paint over.

        A component can call this on its peer during its paint() method, to signal
        that the painting code should ignore a given region. The reason
        for this is to stop embedded windows (such as OpenGL) getting painted over.

        The masked region is cleared each time before a paint happens, so a component
        will have to make sure it calls this every time it's painted.
    */
    void addMaskedRegion (int x, int y, int w, int h) throw();

    //==============================================================================
    /** Returns the number of currently-active peers.

        @see getPeer
    */
    static int getNumPeers() throw();

    /** Returns one of the currently-active peers.

        @see getNumPeers
    */
    static ComponentPeer* getPeer (const int index) throw();

    /** Checks if this peer object is valid.

        @see getNumPeers
    */
    static bool isValidPeer (const ComponentPeer* const peer) throw();

    //==============================================================================
    static void bringModalComponentToFront();

    //==============================================================================
    virtual const StringArray getAvailableRenderingEngines() throw();
    virtual int getCurrentRenderingEngine() throw();
    virtual void setCurrentRenderingEngine (int index) throw();

    //==============================================================================
    juce_UseDebuggingNewOperator

protected:
    Component* const component;
    const int styleFlags;
    RectangleList maskedRegion;
    Rectangle lastNonFullscreenBounds;
    uint32 lastPaintTime;
    ComponentBoundsConstrainer* constrainer;

    static void updateCurrentModifiers() throw();

    /** @internal */
    void handleMessage (const Message& message);

private:
    //==============================================================================
    Component* lastFocusedComponent;
    ScopedPointer <ComponentDeletionWatcher> dragAndDropTargetComponent;
    Component* lastDragAndDropCompUnderMouse;
    bool fakeMouseMessageSent : 1, isWindowMinimised : 1;

    friend class Component;
    static ComponentPeer* getPeerFor (const Component* const component) throw();

    void setLastDragDropTarget (Component* comp);

    ComponentPeer (const ComponentPeer&);
    const ComponentPeer& operator= (const ComponentPeer&);
};



#endif   // __JUCE_COMPONENTPEER_JUCEHEADER__
