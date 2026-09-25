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

   JUCE End User Licence Agreement: https://juce.com/legal/juce-9-licence/
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

// Opaque libxkbcommon handles, loaded at runtime.
struct xkb_context;
struct xkb_keymap;
struct xkb_state;

//==============================================================================
struct WaylandInputHandlerDelegate
{
    virtual ~WaylandInputHandlerDelegate() = default;

    virtual void inputSerialReceived (uint32_t serial) = 0;
    virtual wl_display* getDisplay() const = 0;
};

//==============================================================================
/*
    Callbacks from the Wayland input handler into a peer.

    A peer inherits this privately and the handler only ever holds this
    interface (not a Peer type). Times are already converted to match
    Time::currentTimeMillis().
*/
struct WaylandInputHandlerListener : WaylandPopupParentListener
{
    virtual ~WaylandInputHandlerListener() = default;

    virtual void keyboardFocusGained() = 0;
    virtual void keyboardFocusLost() = 0;
    virtual void modifierKeysChanged() = 0;

    virtual void pointerEntered() = 0;
    virtual void pointerMoved (Point<float> position, int64 time) = 0;
    virtual void pointerButton (Point<float> position, bool pressed, int64 time) = 0;
    virtual void pointerWheel (Point<float> position, const MouseWheelDetails& wheel, int64 time) = 0;
    virtual Point<float> convertPointerPositionToGlobal (Point<float> position) = 0;
    virtual int getPointerCursorScale() const = 0;

    virtual void touchEvent (int touchIndex, Point<float> position, ModifierKeys mods, int64 time) = 0;

    // Hidden peers and peers that ignore key presses cannot receive keyboard focus.
    virtual bool canReceiveKeyboardFocus() const = 0;

    // An explicitly grabbed popup may have keyboard focus while ignoring key presses.
    // Return the surface that should receive those key events instead.
    virtual wl_surface* getKeyEventFallbackSurface() const = 0;
    virtual void keyStateChanged (bool isDown) = 0;
    virtual void keyPressed (int keyCode, juce_wchar character) = 0;
};

//==============================================================================
/*
    Collects wl_touch events until each frame completes, then dispatches completed
    frames in arrival order. Active touches occupy slots whose indices are used as
    JUCE touch indices. The final wl_touch.up completes a frame even if the
    compositor omits wl_touch.frame.

    Event delivery goes through Dispatcher so exercising nested event-loop
    behaviour does not need a compositor.
*/
class WaylandTouchQueue final
{
public:
    struct Dispatcher
    {
        virtual ~Dispatcher() = default;

        virtual bool hasTouchTarget (wl_surface*) = 0;

        // May run a nested event loop that calls back into the queue.
        virtual void dispatchTouchEvent (wl_surface*, int touchIndex, Point<float> position,
                                         ModifierKeys mods, uint32_t time,
                                         std::optional<uint32_t> popupGrabSerial) = 0;
    };

    explicit WaylandTouchQueue (Dispatcher& dispatcherIn) : dispatcher (dispatcherIn) {}

    void touchDown (uint32_t time, wl_surface*, int32_t id, int32_t x, int32_t y,
                    std::optional<uint32_t> popupGrabSerial = std::nullopt);
    void touchMotion (uint32_t time, int32_t id, int32_t x, int32_t y);
    void touchUp (uint32_t time, int32_t id);
    void frameComplete();

    // Events are dispatched a frame at a time, so this reflects the contacts as seen by the event
    // being dispatched rather than everything the compositor has sent.
    bool hasTouchesDown() const noexcept;

    // Discards all queued events and releases active touches at an offscreen position
    // so a drag ends without becoming a click.
    void cancel();

    // A peer destroyed during a touch never receives an up event, so clear its slots
    // before a later touch reuses the same id.
    void surfaceRemoved (wl_surface*);

private:
    struct PendingTouchDown
    {
        uint32_t time = 0;
        wl_surface* surface = nullptr;
        int32_t id = 0;
        int32_t x = 0;
        int32_t y = 0;
        std::optional<uint32_t> popupGrabSerial;
    };

    struct PendingTouchMotion
    {
        uint32_t time = 0;
        int32_t id = 0;
        int32_t x = 0;
        int32_t y = 0;
    };

    struct PendingTouchUp
    {
        uint32_t time = 0;
        int32_t id = 0;
    };

    using PendingTouch = std::variant<PendingTouchDown, PendingTouchMotion, PendingTouchUp>;

    // wl_touch.up and cancel carry only the id, so each slot keeps the surface and last
    // position. MultiTouchMapper is not reusable here: it treats id 0 as a free slot,
    // but wl_touch ids commonly start at 0.
    struct TouchSlot
    {
        bool active = false;
        int32_t id = 0;
        Point<float> pos;
        wl_surface* surface = nullptr;
    };

    void dispatchCompletedFrames();
    void applyTouchDown (const PendingTouchDown&);
    void applyTouchMotion (const PendingTouchMotion&);
    void applyTouchUp (const PendingTouchUp&);
    void releaseActiveTouches();
    int findActiveTouchIndex (int32_t id) const;

    Dispatcher& dispatcher;
    std::vector<TouchSlot> touchSlots;
    std::vector<PendingTouch> openFrameTouches;
    std::vector<PendingTouch> completedTouches;
    // Counts queued downs against ups, so a final up that arrives without a wl_touch.frame can
    // complete its frame.
    int queuedTouchPoints = 0;
    uint32_t lastTouchTimeMs = 0;
    uint32_t generation = 0;
    bool isDispatching = false;

    JUCE_DECLARE_NON_COPYABLE (WaylandTouchQueue)
};

//==============================================================================
class WaylandPointerAxisState final
{
public:
    // wl_pointer versions below 5 never send frame events
    enum class FrameCompletion
    {
        notRequired,
        required
    };

    void bindPointer (uint32_t version);

    [[nodiscard]] FrameCompletion addAxisValue (uint32_t axisIndex, int32_t value);
    void setAxisSource (uint32_t source);
    void addDiscreteAxisSteps (uint32_t axisIndex, int32_t steps);
    void addAxisValue120 (uint32_t axisIndex, int32_t value);
    void setAxisRelativeDirection (uint32_t axisIndex, uint32_t direction);

    // Returns events indexed by wl_pointer.axis.
    [[nodiscard]] std::array<std::optional<MouseWheelDetails>, 2> commitFrame();

private:
    struct AxisAccumulator
    {
        std::optional<float> value;
        std::optional<int> discrete;
        std::optional<int> value120;
        bool inverted = false;
    };

    AxisAccumulator axes[2];
    std::optional<uint32_t> frameAxisSource;
    uint32_t pointerVersion = 0;
};

//==============================================================================
/*
    Binds wl_seat devices and turns their events into listener callbacks.

    Peers register their surface on construction and unregister on destruction, so
    event targets resolve against the {surface, listener} entries here rather than a
    scan of the global peer list.
*/
class WaylandInputHandler final : private Timer,
                                  private WaylandTouchQueue::Dispatcher
{
public:
    struct InputEventSerial
    {
        wl_surface* sourceSurface = nullptr;
        uint32_t value = 0;
    };

    explicit WaylandInputHandler (WaylandInputHandlerDelegate&);
    ~WaylandInputHandler() override;

    void addListener (wl_surface*, WaylandInputHandlerListener&);
    void removeListener (WaylandInputHandlerListener&);
    WaylandInputHandlerListener* findListenerForSurface (wl_surface*) const;
    bool requestKeyboardFocus (wl_surface*);
    bool hasKeyboardFocus (wl_surface*) const;
    void releaseKeyboardFocus (wl_surface*);
    void surfaceUnmapped (wl_surface*);

    void seatCapabilitiesChanged (wl_seat*, uint32_t capabilities);

    // Must run while the owner still exposes this handler: the focus-loss and touch-release
    // callbacks it fires can destroy peers, whose unregistration has to find its way back here.
    void unbindAllDevices();

    // Message-thread only: answered from a pressed-key set that event dispatch mutates there.
    bool isKeyCurrentlyDown (int keyCode) const;

    std::optional<WaylandCursor::PointerTarget> getCursorTarget (wl_surface*) const;
    std::optional<uint32_t> getHeldPressSerial (wl_surface*) const;
    void externalDragStarted (wl_surface*);
    std::optional<WaylandCursor::PointerTarget> getExternalDragCursorTarget() const;
    void externalDragEnded();

    void dragHandedToCompositor (wl_surface*);
    WaylandPopupContext getPopupContext();

    // Grab notifications resolve the toplevel listener per call, so they no-op
    // once that peer is gone.
    void popupGrabStarted (wl_surface* toplevelSurface, Component& componentToDismiss);
    void popupGrabEnded (wl_surface* toplevelSurface);

    // Wayland has no global pointer position. This is the last position reported over one
    // of our surfaces (and the origin before any pointer event arrives).
    Point<float> getCurrentMousePosition() const noexcept { return lastPointerGlobalPos; }

    bool isPointerBound() const noexcept    { return pointer != nullptr; }
    bool isKeyboardBound() const noexcept   { return keyboard != nullptr; }
    bool isTouchBound() const noexcept      { return touch != nullptr; }
    bool hasKeyboardFocus() const noexcept  { return peerFocusListener != nullptr; }

    // The most recent key, button, or touch press, for requests that must prove they follow
    // user input. Some requests also need the surface that received the event.
    std::optional<InputEventSerial> getLatestInputSerial() const noexcept { return latestInputSerial; }

private:
    //==============================================================================
    // libxkbcommon turns the compositor's keymap fd into keysyms and Unicode.
    // If the library is missing, the keyboard stays unbound and the pointer still works.
    class XkbSupport final
    {
    public:
        XkbSupport() = default;

        bool load();

        bool isAvailable() const noexcept   { return context != nullptr; }
        bool hasState() const noexcept      { return state != nullptr; }

        void buildKeymap (const char* keymapString);
        void resetKeymap();
        void updateMask (uint32_t depressed, uint32_t latched, uint32_t locked, uint32_t group);

        uint32_t keyGetOneSym (uint32_t keycode) const  { return state != nullptr ? stateKeyGetOneSym (state.get(), keycode) : 0; }
        uint32_t keyGetUtf32 (uint32_t keycode) const   { return state != nullptr ? stateKeyGetUtf32 (state.get(), keycode) : 0; }
        bool keymapKeyRepeats (uint32_t keycode) const  { return keymap != nullptr && keymapKeyRepeatsFn (keymap.get(), keycode) != 0; }

        // The unshifted symbol on the key's current layout, for matching held keys the way X11
        // matches physical keys regardless of shift state.
        uint32_t keyGetBaseSym (uint32_t keycode) const;

        int currentModifierFlags() const;

    private:
        bool modActive (uint32_t index) const;

        // Value mirrors XKB_MOD_INVALID in <xkbcommon/xkbcommon.h>.
        enum : uint32_t { xkbModInvalid = 0xffffffff };

        template <typename Handle>
        using XkbPtr = std::unique_ptr<Handle, void (*) (Handle*)>;

        using ContextNewFn           = xkb_context* (*) (int);
        using ContextUnrefFn         = void (*) (xkb_context*);
        using KeymapNewFromStringFn  = xkb_keymap* (*) (xkb_context*, const char*, int, int);
        using KeymapUnrefFn          = void (*) (xkb_keymap*);
        using StateNewFn             = xkb_state* (*) (xkb_keymap*);
        using StateUnrefFn           = void (*) (xkb_state*);
        using StateUpdateMaskFn      = int (*) (xkb_state*, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
        using StateKeyGetOneSymFn    = uint32_t (*) (xkb_state*, uint32_t);
        using StateKeyGetUtf32Fn     = uint32_t (*) (xkb_state*, uint32_t);
        using KeymapKeyRepeatsFn     = int (*) (xkb_keymap*, uint32_t);
        using ModGetIndexFn          = uint32_t (*) (xkb_keymap*, const char*);
        using StateModIndexIsActiveFn = int (*) (xkb_state*, uint32_t, int);
        using StateKeyGetLayoutFn    = uint32_t (*) (xkb_state*, uint32_t);
        using KeymapKeyGetSymsByLevelFn = int (*) (xkb_keymap*, uint32_t, uint32_t, uint32_t, const uint32_t**);

        DynamicLibrary lib { "libxkbcommon.so.0" };

        ContextNewFn contextNew = nullptr;
        ContextUnrefFn contextUnref = nullptr;
        KeymapNewFromStringFn keymapNewFromString = nullptr;
        KeymapUnrefFn keymapUnref = nullptr;
        StateNewFn stateNew = nullptr;
        StateUnrefFn stateUnref = nullptr;
        StateUpdateMaskFn stateUpdateMask = nullptr;
        StateKeyGetOneSymFn stateKeyGetOneSym = nullptr;
        StateKeyGetUtf32Fn stateKeyGetUtf32 = nullptr;
        KeymapKeyRepeatsFn keymapKeyRepeatsFn = nullptr;
        ModGetIndexFn modGetIndex = nullptr;
        StateModIndexIsActiveFn stateModIndexIsActive = nullptr;
        StateKeyGetLayoutFn stateKeyGetLayout = nullptr;
        KeymapKeyGetSymsByLevelFn keymapKeyGetSymsByLevel = nullptr;

        XkbPtr<xkb_context> context { nullptr, nullptr };
        XkbPtr<xkb_keymap>  keymap  { nullptr, nullptr };
        XkbPtr<xkb_state>   state   { nullptr, nullptr };
        uint32_t shiftIndex = xkbModInvalid;
        uint32_t ctrlIndex = xkbModInvalid;
        uint32_t altIndex = xkbModInvalid;

        JUCE_DECLARE_NON_COPYABLE (XkbSupport)
    };

    //==============================================================================
    struct SurfaceListener
    {
        wl_surface* surface = nullptr;
        WaylandInputHandlerListener* listener = nullptr;

        // The toplevel remembers which peer to restore when native focus returns.
        WaylandInputHandlerListener* rememberedFocus = nullptr;
    };

    struct PointerFocus
    {
        PointerFocus (wl_surface* surfaceIn, WaylandInputHandlerListener& listenerIn, uint32_t serialIn)
            : surface (surfaceIn), listener (listenerIn), serial (serialIn) {}

        wl_surface* surface;
        WaylandInputHandlerListener& listener;
        uint32_t serial;
    };

    struct NativeKeyboardFocus
    {
        NativeKeyboardFocus (wl_surface* surfaceIn, WaylandInputHandlerListener& listenerIn)
            : surface (surfaceIn), listener (listenerIn) {}

        wl_surface* surface;
        WaylandInputHandlerListener& listener;
    };

    struct PressedKey
    {
        uint32_t evdevKey;
        uint32_t sym;
        uint32_t baseSym;
    };

    struct RepeatKey
    {
        uint32_t evdevKey;
        int keyCode;
        juce_wchar character;
    };

    struct InputEvent
    {
        // Key events may be delivered to a different peer from the native focus surface that supplied the grab serial.
        wl_surface* targetSurface = nullptr;
        std::optional<WaylandPopupTrigger> trigger;
    };

    enum class PressDevice { pointer, keyboard, touch };

    struct RetainedPress
    {
        WaylandPopupTrigger trigger;
        PressDevice device;
    };

    struct DragCandidate
    {
        wl_surface* surface = nullptr;
        uint32_t serial = 0;
        int mouseButtonFlag = 0;
        std::optional<WaylandCursor::PointerTarget> cursorTarget;
    };

    struct ExternalDrag
    {
        wl_surface* surface = nullptr;
        int unreleasedButtonFlag = 0;
        std::optional<WaylandCursor::PointerTarget> cursorTarget;
        Point<float> releasePosition;
    };

    //==============================================================================
    void bindPointer (wl_seat*);
    void bindKeyboard (wl_seat*);
    void bindTouch (wl_seat*);
    void unbindPointer();
    void unbindKeyboard();
    void unbindTouch();

    void onPointerEnter (uint32_t serial, wl_surface*, int32_t surfaceX, int32_t surfaceY);
    void onPointerLeave();
    void onPointerMotion (uint32_t time, int32_t surfaceX, int32_t surfaceY);
    void onPointerButton (uint32_t serial, uint32_t time, uint32_t button, uint32_t buttonState);
    void onPointerAxis (uint32_t time, uint32_t axis, int32_t value);
    void onPointerFrame();
    void dispatchPointerMotion();

    void onKeymap (uint32_t format, int fd, uint32_t size);
    void onKeyboardEnter (wl_surface* surface, Span<const uint32_t> keys);
    void onKeyboardLeave (wl_surface* surface);
    void handleFocusLossSyncDone (wl_callback*);
    void updatePeerFocus();
    void scheduleFocusUpdate();
    void setPeerFocusListener (WaylandInputHandlerListener*);
    void rememberKeyboardFocus (WaylandInputHandlerListener&);
    bool sharesKeyboardFocus (const WaylandInputHandlerListener&) const;
    WaylandInputHandlerListener* resolveNativeKeyEventTarget() const;
    void onKey (uint32_t serial, uint32_t time, uint32_t key, uint32_t keyState);
    void onModifiers (uint32_t depressed, uint32_t latched, uint32_t locked, uint32_t group);
    void onRepeatInfo (int rate, int delay);
    void updatePressedKeys (uint32_t evdevKey, uint32_t sym, uint32_t baseSym, bool down);

    void startKeyRepeat (uint32_t evdevKey, int keyCode, juce_wchar unicodeChar, bool keyPressed);
    void stopKeyRepeat();
    void timerCallback() override;

    wl_surface* findSurfaceForListener (const WaylandInputHandlerListener*) const;
    WaylandInputHandlerListener* findKeyEventTarget() const;

    static std::optional<WaylandPopupTrigger> makePopupTrigger (wl_surface*, std::optional<uint32_t> serial);
    std::optional<WaylandPopupTrigger> getAvailablePopupTrigger();
    void retainCurrentPressForPopup (PressDevice);
    void dropRetainedPress (PressDevice);
    void reportInputSerial (wl_surface*, uint32_t);

    bool hasTouchTarget (wl_surface*) override;
    void dispatchTouchEvent (wl_surface*, int touchIndex, Point<float> position,
                             ModifierKeys mods, uint32_t time,
                             std::optional<uint32_t> popupGrabSerial) override;

    //==============================================================================
    static const wl_pointer_listener pointerListener;
    static const wl_keyboard_listener keyboardListener;
    static const wl_callback_listener focusLossSyncListener;
    static const wl_touch_listener touchListener;

    WaylandInputHandlerDelegate& delegate;
    XkbSupport xkb;

    std::unique_ptr<wl_pointer, FunctionPointerDestructor<WaylandProtocol::destroyPointer>> pointer;
    std::unique_ptr<wl_keyboard, FunctionPointerDestructor<WaylandProtocol::destroyKeyboard>> keyboard;
    std::unique_ptr<wl_touch, FunctionPointerDestructor<WaylandProtocol::destroyTouch>> touch;

    std::vector<SurfaceListener> surfaceListeners;
    std::optional<PointerFocus> pointerFocus;
    std::optional<NativeKeyboardFocus> nativeKeyboardFocus;

    // A temporary window's peer may have focus while its toplevel or a menu has native focus.
    WaylandInputHandlerListener* peerFocusListener = nullptr;

    // Wait for queued keyboard enter events before treating a keyboard leave as a loss of peer focus.
    std::unique_ptr<wl_callback, FunctionPointerDestructor<WaylandProtocol::wlCallbackDestroy>> focusLossSync;

    // Restored after nested callbacks so popup creation sees the event that caused it.
    InputEvent currentInputEvent;

    // Keep the last press for popups that open asynchronously from it, such as ComboBox menus.
    std::optional<RetainedPress> retainedPress;
    std::variant<std::monostate, DragCandidate, ExternalDrag> dragState;
    std::optional<InputEventSerial> latestInputSerial;
    int activePopupGrabs = 0;

    WaylandTouchQueue touchQueue { *this };
    std::vector<PressedKey> pressedKeys;

    Point<float> lastPointerPos;
    Point<float> lastPointerGlobalPos;
    uint32_t lastPointerTimeMs = 0;
    WaylandPointerAxisState axisState;

    std::optional<RepeatKey> repeatKey;
    int repeatIntervalMs = 40;
    int keyRepeatRate = 25;
    int keyRepeatDelayMs = 400;

   #if JUCE_UNIT_TESTS
    friend class WaylandInputHandlerDragTests;
    friend class WaylandPopupContextTests;
    friend class WaylandHeldKeyTests;
    friend class WaylandInputHandlerFocusTests;
   #endif

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WaylandInputHandler)
};

} // namespace juce
