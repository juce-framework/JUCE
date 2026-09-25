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

//==============================================================================
/*
    Receives output changes that may affect a peer's scale.
*/
struct WaylandOutputListener
{
    virtual ~WaylandOutputListener() = default;

    // Called after one or more outputs were added, removed, or changed.
    virtual void outputConfigurationChanged() = 0;

    // Called before the output handle is destroyed so that listeners can stop referring to it.
    virtual void outputWillBeDestroyed (wl_output* output) = 0;
};

struct WaylandToplevelMappingListener
{
    virtual ~WaylandToplevelMappingListener() = default;

    virtual void waylandToplevelMapped (ComponentPeer& peer) = 0;
};

/*  Receives callbacks each time Wayland's event fd gets notified. */
struct WaylandLoopListener
{
    virtual ~WaylandLoopListener() = default;

    /*  Called after events are dispatched from wl_display_dispatch_pending.

        Some compositors are very eager to send configuration requests, but actually enacting
        a layout change (resizing Components) can be very expensive if those components do things
        like shaping text or allocating textures.

        Instead of attempting to reconfigure and acknowledge every request, it's more efficient to
        accumulate changes and to commit only the most recent configuration after processing all
        pending messages.

        There may be scope for further heuristics to improve performance here. For example,
        it's probably not necessary to commit multiple changes that happen in the space of a single
        frame, in the event that the Wayland fd wakes up the app multiple times in the space of a
        single frame. That would add even more complexity, though, so we'll hold off for the time
        being.
    */
    virtual void waylandDidDispatchPendingEvents() = 0;
};

//==============================================================================
class WaylandWindowSystem final : public DeletedAtShutdown,
                                  private AsyncUpdater,
                                  private WaylandDataDeviceDelegate,
                                  private WaylandTextInputDelegate,
                                  private WaylandInputHandlerDelegate
{
public:
    struct ExportedSurfaceHandle
    {
        std::unique_ptr<zxdg_exported_v2, FunctionPointerDestructor<WaylandProtocol::zxdgExportedV2Destroy>> exported;
        String handle;
    };

    struct BoundOutput
    {
        using Handle = std::unique_ptr<wl_output, FunctionPointerDestructor<WaylandProtocol::destroyOutput>>;
        using XdgOutputHandle = std::unique_ptr<zxdg_output_v1, FunctionPointerDestructor<WaylandProtocol::zxdgOutputV1Destroy>>;

        struct State
        {
            bool operator== (const State&) const;
            bool operator!= (const State&) const;

            Point<int> fallbackLogicalPosition; // Used only when the logical layout below is unavailable.
            Point<int> modeSize; // Hardware pixels before output rotation.

            // These use the same logical coordinate system as window positions.
            std::optional<Point<int>> logicalPosition;
            std::optional<Point<int>> logicalSize;
            int physicalWidthMm = 0;
            int physicalHeightMm = 0;
            int refreshRateMilliHz = 0;
            int integerScaleFactor = 1;
            uint32_t transform = WaylandProtocol::wlOutputTransformNormal;
            String name;
            String description;

        private:
            auto tie() const;
        };

        BoundOutput (WaylandWindowSystem& windowSystemIn, uint32_t registryNameIn, Handle handleIn);

        void bindXdgOutput (zxdg_output_manager_v1*);
        void handleGeometry (int32_t x, int32_t y, int32_t physicalWidth, int32_t physicalHeight, uint32_t transform);
        void handleMode (uint32_t flags, int32_t width, int32_t height, int32_t refresh);
        void handleScale (int32_t scale);
        void handleOutputDone();
        void handleLogicalPosition (int32_t x, int32_t y);
        void handleLogicalSize (int32_t width, int32_t height);
        void handleXdgOutputDone();
        void handleName (const char*);
        void handleDescription (const char*);

        void commitPendingState();

        // Version 1 has no done event, so each property event completes its own batch.
        void commitAfterEventIfRequired();

        WaylandWindowSystem& windowSystem;
        uint32_t registryName = 0;
        Handle handle;
        XdgOutputHandle xdgOutput;
        State pendingState;
        std::optional<State> currentState;
        const bool waitsForOutputDoneEvent;

        JUCE_DECLARE_NON_COPYABLE (BoundOutput)
    };

    bool isWaylandAvailable();
    static bool shouldUseWaylandBackend();

    wl_display* getDisplay() const noexcept override    { return display.get(); }
    wl_compositor* getCompositor() const noexcept       { return compositor.get(); }
    wl_subcompositor* getSubcompositor() const noexcept { return subcompositor.get(); }
    wl_shm* getShm() const noexcept                     { return shm.get(); }
    xdg_wm_base* getXdgWmBase() const noexcept          { return xdgWmBase.get(); }
    zxdg_decoration_manager_v1* getDecorationManager() const noexcept { return decorationManager.get(); }
    libdecor* getLibdecor() const noexcept              { return libdecorContext.get(); }
    wl_seat* getSeat() const noexcept                   { return seat.get(); }
    String getSeatName() const noexcept                 { return seatName; }
    wp_viewporter* getViewporter() const noexcept       { return viewporter.get(); }
    wp_fractional_scale_manager_v1* getFractionalScaleManager() const noexcept { return fractionalScaleManager.get(); }
    wp_alpha_modifier_v1* getAlphaModifier() const noexcept { return alphaModifier.get(); }
    xdg_activation_v1* getXdgActivation() const noexcept    { return xdgActivation.get(); }
    int getNumBoundOutputs() const noexcept             { return (int) outputs.size(); }
    bool areRegistryGlobalsBound() const noexcept       { return registryGlobalsBound; }
    bool isSeatBound() const noexcept                   { return seatBound; }
    bool isKeyboardBound() const noexcept               { return inputHandler != nullptr && inputHandler->isKeyboardBound(); }
    bool isPointerBound() const noexcept                { return inputHandler != nullptr && inputHandler->isPointerBound(); }
    bool isTouchBound() const noexcept                  { return inputHandler != nullptr && inputHandler->isTouchBound(); }
    bool hasKeyboardFocus() const noexcept              { return inputHandler != nullptr && inputHandler->hasKeyboardFocus(); }
    std::optional<WaylandInputHandler::InputEventSerial> getLatestInputSerial() const noexcept;

    // Removing is a no-op when no handler was ever created.
    void addInputListener (wl_surface*, WaylandInputHandlerListener&);
    void removeInputListener (WaylandInputHandlerListener&);
    WaylandInputHandlerListener* findInputListener (wl_surface*) const;
    bool requestKeyboardFocus (wl_surface*);
    bool hasKeyboardFocus (wl_surface*) const;
    void releaseKeyboardFocus (wl_surface*);
    void surfaceUnmapped (wl_surface*);
    void addDataDeviceListener (wl_surface*, WaylandDataDeviceListener&);
    void removeDataDeviceListener (WaylandDataDeviceListener&);

    void copyTextToClipboard (const String&);
    String getTextFromClipboard();
    bool externalDragFileInit (wl_surface*, const StringArray&, bool canMoveFiles, std::function<void()> callback);
    bool externalDragTextInit (wl_surface*, const String&, std::function<void()> callback);

    std::optional<WaylandPopupOwner> findPopupOwner (int styleFlags);
    std::optional<WaylandPopupParent> findPopupParent (wl_surface* toplevelSurface, WaylandPopupGrab);
    void dismissPopupDescendants (xdg_surface* ancestorXdgSurface);

    std::optional<uint32_t> getHeldPressSerial (wl_surface*) const;
    void dragHandedToCompositor (wl_surface*);

    // Grab notifications resolve the toplevel by surface per call, so they no-op once
    // that peer is gone.
    void popupGrabStarted (wl_surface* toplevelSurface, Component& componentToDismiss);
    void popupGrabEnded (wl_surface* toplevelSurface);
    Point<float> getCurrentMousePosition() const noexcept;
    void showCursor (wl_surface*, MouseCursor::StandardCursorType);
    void showCursor (wl_surface*, const detail::CustomMouseCursorInfo*);
    void removeCustomCursorCache (const detail::CustomMouseCursorInfo&);

    void addLoopListener    (WaylandLoopListener& listener)         { loopListeners.add    (listener); }
    void removeLoopListener (WaylandLoopListener& listener)         { loopListeners.remove (listener); }
    void addOutputListener (WaylandOutputListener& listener)        { outputListeners.add (&listener); }
    void removeOutputListener (WaylandOutputListener& listener)     { outputListeners.remove (&listener); }
    void addToplevelMappingListener (WaylandToplevelMappingListener& listener) { toplevelMappingListeners.add (&listener); }
    void removeToplevelMappingListener (WaylandToplevelMappingListener& listener) { toplevelMappingListeners.remove (&listener); }
    void toplevelMapped (ComponentPeer& peer);

    // Called once after one or more output changes have been applied.
    void setDisplaysChangedCallback (std::function<void()> callback);
    bool hasDisplaysChangedCallback() const noexcept                { return displaysChangedCallback != nullptr; }

    // Returns the last complete scale for an output, or nothing until that scale is known.
    std::optional<int> getScaleForOutput (wl_output*) const;

    // A surface has no outputs before its first buffer is mapped, so use the largest
    // output scale to keep that initial buffer sharp on any output.
    int getLargestOutputScale() const;

   #if JUCE_WAYLAND_PEER_DIAGNOSTICS
    detail::WaylandPeerDiagnostics::TextInput getTextInputDiagnostics (wl_surface*) const;
   #endif

    void addTextInputClient (wl_surface*, WaylandTextInputClient&);
    void removeTextInputClient (WaylandTextInputClient&);
    void textInputRequired (wl_surface*, TextInputTarget&);
    void refreshTextInputState (wl_surface*);
    void closeInputMethodContext (wl_surface*);
    void dismissPendingTextInput (wl_surface*);

    bool isKeyCurrentlyDown (int keyCode) const;

    Array<Displays::Display> findDisplays (float masterScale);

    // Exports a surface through xdg_foreign so another process can parent to it (portal dialogs).
    // Returns nullptr when the compositor lacks zxdg_exporter_v2 or no handle arrived.
    std::unique_ptr<ExportedSurfaceHandle> exportSurfaceForExternalParenting (wl_surface*);

    void flush() override;
    void handleRegistryGlobal (wl_registry*, uint32_t name, const char* interface, uint32_t version);
    void handleRegistryGlobalRemove (uint32_t name);
    void handleSeatCapabilities (uint32_t capabilities);
    void handleSeatName (const char* name);

    JUCE_DECLARE_SINGLETON_INLINE (WaylandWindowSystem, false)

private:
    WaylandWindowSystem();
    ~WaylandWindowSystem() override;

    bool connect();
    void disconnect();
    void registerEventFd();
    void unregisterEventFd();
    void dispatchPendingEvents();
    void handleConnectionLost();
    void ensureDataDeviceBound();
    void ensureTextInputBound();
    void inputSerialReceived (uint32_t serial) override;
    void dragSourceActionChanged (uint32_t action) override;
    void externalDragEnded() override;
    void showExternalDragCursor (uint32_t action);
    void notifyOutputWillBeDestroyed (const BoundOutput&);
    static void handleLibdecorError (libdecor*, libdecor_error, const char*);

    // Notify listeners once after all output events already received in this message-loop cycle.
    void handleAsyncUpdate() override;

    bool attemptedConnection = false;
    bool available = false;
    bool connectionLost = false;
    // Set when libdecor reports an error, and cleared per connection, unlike the permanent
    // LibdecorSymbols flag.
    bool libdecorErrorReceived = false;
    bool fdRegistered = false;
    bool registryGlobalsBound = false;
    bool seatBound = false;

    struct DisconnectDisplay
    {
        void operator() (wl_display*) const;
    };

    std::unique_ptr<wl_display, DisconnectDisplay> display;
    LibdecorContextHandle libdecorContext;
    std::unique_ptr<wl_registry, FunctionPointerDestructor<WaylandProtocol::wlRegistryDestroy>> registry;
    std::unique_ptr<wl_compositor, FunctionPointerDestructor<WaylandProtocol::wlCompositorDestroy>> compositor;
    std::unique_ptr<wl_subcompositor, FunctionPointerDestructor<WaylandProtocol::wlSubcompositorDestroy>> subcompositor;
    std::unique_ptr<wl_shm, FunctionPointerDestructor<WaylandProtocol::wlShmDestroy>> shm;
    std::unique_ptr<zxdg_output_manager_v1, FunctionPointerDestructor<WaylandProtocol::zxdgOutputManagerV1Destroy>> xdgOutputManager;
    std::vector<std::unique_ptr<BoundOutput>> outputs;
    std::unique_ptr<wl_data_device_manager,
                    FunctionPointerDestructor<WaylandProtocol::destroyDataDeviceManager>> dataDeviceManager;
    std::unique_ptr<zwp_text_input_manager_v3, FunctionPointerDestructor<WaylandProtocol::zwpTextInputManagerV3Destroy>> textInputManager;
    std::unique_ptr<wl_seat, FunctionPointerDestructor<WaylandProtocol::destroySeat>> seat;
    std::unique_ptr<xdg_wm_base, FunctionPointerDestructor<WaylandProtocol::xdgWmBaseDestroy>> xdgWmBase;
    std::unique_ptr<zxdg_decoration_manager_v1, FunctionPointerDestructor<WaylandProtocol::zxdgDecorationManagerV1Destroy>> decorationManager;
    std::unique_ptr<zxdg_exporter_v2, FunctionPointerDestructor<WaylandProtocol::zxdgExporterV2Destroy>> exporter;
    std::unique_ptr<wp_viewporter, FunctionPointerDestructor<WaylandProtocol::wpViewporterDestroy>> viewporter;
    std::unique_ptr<wp_fractional_scale_manager_v1, FunctionPointerDestructor<WaylandProtocol::wpFractionalScaleManagerV1Destroy>> fractionalScaleManager;
    std::unique_ptr<wp_alpha_modifier_v1, FunctionPointerDestructor<WaylandProtocol::wpAlphaModifierV1Destroy>> alphaModifier;
    std::optional<uint32_t> alphaModifierRegistryName;
    std::unique_ptr<xdg_activation_v1, FunctionPointerDestructor<WaylandProtocol::xdgActivationV1Destroy>> xdgActivation;
    std::optional<uint32_t> xdgActivationRegistryName;

    std::optional<WaylandCursor> cursor;
    std::unique_ptr<WaylandInputHandler> inputHandler;
    std::unique_ptr<WaylandDataDevice> dataDevice;
    std::unique_ptr<WaylandTextInput> textInput;
    ListenerList<WaylandOutputListener> outputListeners;
    ListenerList<WaylandToplevelMappingListener> toplevelMappingListeners;
    ListenerList<WaylandLoopListener> loopListeners;
    std::function<void()> displaysChangedCallback;
    String mainOutputName;
    String seatName;

    int fd = -1;

    static libdecor_interface libdecorInterface;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WaylandWindowSystem)
};

} // namespace juce
