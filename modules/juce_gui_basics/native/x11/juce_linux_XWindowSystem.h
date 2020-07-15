/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
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
namespace XWindowSystemUtilities
{
    //==============================================================================
    /** A handy struct that uses XLockDisplay and XUnlockDisplay to lock the X server
        using RAII.

        @tags{GUI}
    */
    struct ScopedXLock
    {
        ScopedXLock();
        ~ScopedXLock();
    };

    //==============================================================================
    /** Gets a specified window property and stores its associated data, freeing it
        on deletion.

        @tags{GUI}
    */
    struct GetXProperty
    {
        GetXProperty (::Window windowH, Atom property, long offset,
                      long length, bool shouldDelete, Atom requestedType);
        ~GetXProperty();

        bool success = false;
        unsigned char* data = nullptr;
        unsigned long numItems = 0, bytesLeft = 0;
        Atom actualType;
        int actualFormat = -1;
    };

    //==============================================================================
    /** Initialises and stores some atoms for the display.

        @tags{GUI}
    */
    struct Atoms
    {
        enum ProtocolItems
        {
            TAKE_FOCUS = 0,
            DELETE_WINDOW = 1,
            PING = 2
        };

        Atoms() = default;
        explicit Atoms (::Display*);

        static Atom getIfExists (::Display*, const char* name);
        static Atom getCreating (::Display*, const char* name);

        static String getName (::Display*, Atom);
        static bool isMimeTypeFile (::Display*, Atom);

        static constexpr unsigned long DndVersion = 3;

        Atom protocols, protocolList[3], changeState, state, userTime, activeWin, pid, windowType, windowState,
             XdndAware, XdndEnter, XdndLeave, XdndPosition, XdndStatus, XdndDrop, XdndFinished, XdndSelection,
             XdndTypeList, XdndActionList, XdndActionDescription, XdndActionCopy, XdndActionPrivate,
             XembedMsgType, XembedInfo, allowedActions[5], allowedMimeTypes[4], utf8String, clipboard, targets;
    };
}

//==============================================================================
template<typename WindowHandle>
class LinuxComponentPeer;

class XWindowSystem  : public DeletedAtShutdown
{
public:
    //==============================================================================
    ::Window createWindow (::Window parentWindow, LinuxComponentPeer<::Window>* peer) const;
    void destroyWindow    (::Window windowH);

    void setTitle (::Window windowH, const String& title) const;
    void setIcon (::Window windowH, const Image& newIcon) const;
    void setVisible (::Window windowH, bool shouldBeVisible) const;
    void setBounds (::Window windowH, Rectangle<int> newBounds, bool fullScreen) const;

    BorderSize<int> getBorderSize   (::Window windowH) const;
    Rectangle<int>  getWindowBounds (::Window windowH, ::Window parentWindow);
    Point<int> getParentScreenPosition() const;

    bool contains (::Window windowH, Point<int> localPos) const;

    void setMinimised (::Window windowH, bool shouldBeMinimised) const;
    bool isMinimised  (::Window windowH) const;

    void toFront  (::Window windowH, bool makeActive) const;
    void toBehind (::Window windowH, ::Window otherWindow) const;

    bool isFocused (::Window windowH) const;
    bool grabFocus (::Window windowH) const;

    bool canUseSemiTransparentWindows() const;
    bool canUseARGBImages() const;

    int getNumPaintsPending (::Window windowH) const;

    Image createImage (int width, int height, bool argb) const;
    void blitToWindow (::Window windowH, Image image, Rectangle<int> destinationRect, Rectangle<int> totalRect) const;

    void setScreenSaverEnabled (bool enabled) const;

    Point<float> getCurrentMousePosition() const;
    void setMousePosition (Point<float> pos) const;

    void* createCustomMouseCursorInfo (const Image& image, Point<int> hotspot) const;
    void deleteMouseCursor (void* cursorHandle) const;
    void* createStandardMouseCursor (MouseCursor::StandardCursorType type) const;
    void showCursor (::Window windowH, void* cursorHandle) const;

    bool isKeyCurrentlyDown (int keyCode) const;
    ModifierKeys getNativeRealtimeModifiers() const;

    Array<Displays::Display> findDisplays (float masterScale) const;

    ::Window createKeyProxy (::Window windowH) const;
    void deleteKeyProxy (::Window keyProxy) const;

    bool externalDragFileInit (LinuxComponentPeer<::Window>* peer, const StringArray& files, bool canMove, std::function<void()>&& callback) const;
    bool externalDragTextInit (LinuxComponentPeer<::Window>* peer, const String& text, std::function<void()>&& callback) const;

    void copyTextToClipboard (const String& clipText);
    String getTextFromClipboard() const;

    String getLocalClipboardContent() const    { return localClipboardContent; }

    ::Display* getDisplay()                    { return display; }
    XWindowSystemUtilities::Atoms& getAtoms()  { return atoms; }

    //==============================================================================
    void handleWindowMessage (LinuxComponentPeer<::Window>* peer, XEvent& event) const;

    //==============================================================================
    JUCE_DECLARE_SINGLETON (XWindowSystem, false)

private:
    XWindowSystem();
    ~XWindowSystem();

    //==============================================================================
    bool initialiseXDisplay();
    void destroyXDisplay();

    //==============================================================================
    ::Window getFocusWindow (::Window windowH) const;

    bool isParentWindowOf (::Window windowH, ::Window possibleChild) const;
    bool isFrontWindow (::Window windowH) const;

    //==============================================================================
    void xchangeProperty (::Window windowH, Atom property, Atom type, int format, const void* data, int numElements) const;

    void removeWindowDecorations (::Window windowH) const;
    void addWindowButtons        (::Window windowH, int styleFlags) const;
    void setWindowType           (::Window windowH, int styleFlags) const;

    void initialisePointerMap();
    void deleteIconPixmaps (::Window windowH) const;
    void updateModifierMappings() const;

    long getUserTime (::Window windowH) const;

    //==============================================================================
    void handleKeyPressEvent        (LinuxComponentPeer<::Window>*, XKeyEvent&) const;
    void handleKeyReleaseEvent      (LinuxComponentPeer<::Window>*, const XKeyEvent&) const;
    void handleWheelEvent           (LinuxComponentPeer<::Window>*, const XButtonPressedEvent&, float) const;
    void handleButtonPressEvent     (LinuxComponentPeer<::Window>*, const XButtonPressedEvent&, int) const;
    void handleButtonPressEvent     (LinuxComponentPeer<::Window>*, const XButtonPressedEvent&) const;
    void handleButtonReleaseEvent   (LinuxComponentPeer<::Window>*, const XButtonReleasedEvent&) const;
    void handleMotionNotifyEvent    (LinuxComponentPeer<::Window>*, const XPointerMovedEvent&) const;
    void handleEnterNotifyEvent     (LinuxComponentPeer<::Window>*, const XEnterWindowEvent&) const;
    void handleLeaveNotifyEvent     (LinuxComponentPeer<::Window>*, const XLeaveWindowEvent&) const;
    void handleFocusInEvent         (LinuxComponentPeer<::Window>*) const;
    void handleFocusOutEvent        (LinuxComponentPeer<::Window>*) const;
    void handleExposeEvent          (LinuxComponentPeer<::Window>*, XExposeEvent&) const;
    void handleConfigureNotifyEvent (LinuxComponentPeer<::Window>*, XConfigureEvent&) const;
    void handleGravityNotify        (LinuxComponentPeer<::Window>*) const;
    void handleMappingNotify        (XMappingEvent&) const;
    void handleClientMessageEvent   (LinuxComponentPeer<::Window>*, XClientMessageEvent&, XEvent&) const;
    void handleXEmbedMessage        (LinuxComponentPeer<::Window>*, XClientMessageEvent&) const;

    //==============================================================================
    bool xIsAvailable = false;

    XWindowSystemUtilities::Atoms atoms;
    ::Display* display = nullptr;
    Colormap colormap = {};
    Visual* visual = nullptr;

    int depth = 0, shmCompletionEvent = 0;
    int pointerMap[5] = {};
    String localClipboardContent;

    Point<int> parentScreenPosition;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (XWindowSystem)
};

} // namespace juce
