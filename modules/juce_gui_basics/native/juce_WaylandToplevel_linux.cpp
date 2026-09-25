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

// Both backends produce an xdg-shell toplevel. libdecor creates and drives its own internally.
enum class WaylandToplevelBackend
{
    xdgShell,
    libdecor
};

struct WaylandDecorationCapabilities
{
    bool decorationManagerAvailable = false;
    bool libdecorAvailable = false;
};

static WaylandToplevelBackend chooseWaylandToplevelBackend (WaylandToplevel::NativeTitleBar nativeTitleBar,
                                                            WaylandDecorationCapabilities capabilities)
{
    // Frameless and JUCE-drawn windows do not need native decorations. A native title bar only
    // needs libdecor when the compositor cannot provide server-side decorations.
    if (nativeTitleBar == WaylandToplevel::NativeTitleBar::yes
        && ! capabilities.decorationManagerAvailable
        && capabilities.libdecorAvailable)
    {
        return WaylandToplevelBackend::libdecor;
    }

    return WaylandToplevelBackend::xdgShell;
}

// Both points come from libdecor_frame_translate_coordinate, so they are in frame-local
// coordinates with the frame's top-left at the origin.
static Rectangle<int> makeLibdecorPopupParentBounds (Rectangle<int> contentBounds,
                                                     Point<int> contentTopLeftInFrame,
                                                     Point<int> contentBottomRightInFrame)
{
    const auto frameOrigin = contentBounds.getPosition() - contentTopLeftInFrame;

    // The frame's top-left is at the origin, so the content's bottom-right corner equals the size.
    const auto size = contentBottomRightInFrame;
    return { frameOrigin.x, frameOrigin.y, size.x, size.y };
}

static libdecor_resize_edge getLibdecorResizeEdge (uint32_t xdgResizeEdge)
{
    switch (xdgResizeEdge)
    {
        case WaylandProtocol::xdgToplevelResizeEdgeTop:          return libdecorResizeEdgeTop;
        case WaylandProtocol::xdgToplevelResizeEdgeBottom:       return libdecorResizeEdgeBottom;
        case WaylandProtocol::xdgToplevelResizeEdgeLeft:         return libdecorResizeEdgeLeft;
        case WaylandProtocol::xdgToplevelResizeEdgeTopLeft:      return libdecorResizeEdgeTopLeft;
        case WaylandProtocol::xdgToplevelResizeEdgeBottomLeft:   return libdecorResizeEdgeBottomLeft;
        case WaylandProtocol::xdgToplevelResizeEdgeRight:        return libdecorResizeEdgeRight;
        case WaylandProtocol::xdgToplevelResizeEdgeTopRight:     return libdecorResizeEdgeTopRight;
        case WaylandProtocol::xdgToplevelResizeEdgeBottomRight:  return libdecorResizeEdgeBottomRight;
    }

    return libdecorResizeEdgeNone;
}

//==============================================================================
static uint32_t parseToplevelStates (const wl_array* states)
{
    if (states == nullptr || states->data == nullptr)
        return 0;

    const Span toplevelStates { static_cast<const uint32_t*> (states->data),
                                states->size / sizeof (uint32_t) };
    uint32_t result = 0;

    for (const auto state : toplevelStates)
        if (WaylandProtocol::xdgToplevelStateMaximized <= state && state <= WaylandProtocol::xdgToplevelStateSuspended)
            result |= WaylandProtocol::xdgToplevelStateMask (state);

    return result;
}

static ComponentPeer::OptionalBorderSize getFrameSizeForXdgDecorationMode (uint32_t mode)
{
    if (mode == WaylandProtocol::zxdgToplevelDecorationV1ModeClientSide)
        return ComponentPeer::OptionalBorderSize { BorderSize<int>() };

    return {};
}

void WaylandFullScreenState::setFullScreenRequested (bool shouldBeFullScreen)
{
    // A configure sent before this request may still be on its way, so the first configure
    // that disagrees is ignored. A request that changes nothing expects no configure.
    const auto expectsConfigure = shouldBeFullScreen != wantsFullScreen
                               || shouldBeFullScreen != confirmedFullScreen;

    reportsRequestedState = expectsConfigure;
    ignoresNextDisagreement = expectsConfigure;
    wantsFullScreen = shouldBeFullScreen;
}

void WaylandFullScreenState::configureReceived (bool compositorReportsFullScreen)
{
    confirmedFullScreen = compositorReportsFullScreen;

    if (confirmedFullScreen == wantsFullScreen)
    {
        reportsRequestedState = false;
        ignoresNextDisagreement = false;
    }
    else if (ignoresNextDisagreement)
    {
        ignoresNextDisagreement = false;
    }
    else
    {
        reportsRequestedState = false;
    }
}

void WaylandFullScreenState::toplevelDestroyed()
{
    confirmedFullScreen = false;
    reportsRequestedState = wantsFullScreen;
    ignoresNextDisagreement = wantsFullScreen;
}

bool WaylandFullScreenState::isFullScreen() const
{
    return reportsRequestedState ? wantsFullScreen : confirmedFullScreen;
}

bool WaylandFullScreenState::isFullScreenRequested() const
{
    return wantsFullScreen;
}

//==============================================================================
static String getAppId()
{
    if (auto* app = JUCEApplicationBase::getInstance())
        return app->getApplicationName();

    return {};
}

//==============================================================================
class XdgShellToplevel final : public WaylandToplevel,
                               private WaylandLoopListener
{
public:
    static std::unique_ptr<XdgShellToplevel> create (Delegate& delegate, detail::WaylandPeerDiagnostics& diagnostics,
                                                     wl_surface& surface, const String& title,
                                                     NativeTitleBar nativeTitleBar, bool fullScreenRequested)
    {
        auto result = std::make_unique<XdgShellToplevel> (delegate, diagnostics, surface, title,
                                                          nativeTitleBar, fullScreenRequested);

        if (result->xdgToplevel != nullptr)
            return result;

        return nullptr;
    }

    XdgShellToplevel (Delegate& delegateIn, detail::WaylandPeerDiagnostics& diagnosticsIn,
                      wl_surface& surfaceIn, const String& title, NativeTitleBar nativeTitleBar,
                      bool fullScreenRequested)
        : delegate (delegateIn),
          diagnostics (diagnosticsIn),
          surface (surfaceIn)
    {
        auto* windowSystem = WaylandWindowSystem::getInstance();

        xdgSurface.reset (WaylandProtocol::xdgWmBaseGetXdgSurface (windowSystem->getXdgWmBase(), &surface));

        if (xdgSurface == nullptr)
            return;

        xdgToplevel.reset (WaylandProtocol::xdgSurfaceGetToplevel (xdgSurface.get()));

        if (xdgToplevel == nullptr)
            return;

        if (nativeTitleBar == NativeTitleBar::yes)
            if (auto* decorationManager = windowSystem->getDecorationManager())
                decoration.reset (WaylandProtocol::zxdgDecorationManagerV1GetToplevelDecoration (decorationManager,
                                                                                                 xdgToplevel.get()));

        WaylandProtocol::xdgSurfaceAddListener (xdgSurface.get(), &surfaceListener, this);
        WaylandProtocol::xdgToplevelAddListener (xdgToplevel.get(), &toplevelListener, this);

        if (decoration != nullptr)
        {
            WaylandProtocol::zxdgToplevelDecorationV1AddListener (decoration.get(), &decorationListener, this);
            WaylandProtocol::zxdgToplevelDecorationV1SetMode (decoration.get(),
                                                              WaylandProtocol::zxdgToplevelDecorationV1ModeServerSide);

            // Assume the mode we asked for until the compositor answers, so a window that is
            // about to be framed is never described as frameless.
            frameSize = {};
        }

        // Resend the state because the compositor discards all toplevel state on unmap.
        setTitle (title);

        if (const auto appId = getAppId(); appId.isNotEmpty())
            WaylandProtocol::xdgToplevelSetAppId (xdgToplevel.get(), appId.toRawUTF8());

        if (fullScreenRequested)
            requestFullScreen (true);

        WaylandWindowSystem::getInstance()->addLoopListener (*this);
    }

    ~XdgShellToplevel() override
    {
        WaylandWindowSystem::getInstance()->removeLoopListener (*this);
    }

    ComponentPeer::OptionalBorderSize getFrameSizeIfPresent() const override   { return frameSize; }
    xdg_surface* getXdgSurface() const override                                { return xdgSurface.get(); }
    xdg_toplevel* getXdgToplevel() const override                              { return xdgToplevel.get(); }
    Rectangle<int> getPopupParentBounds (Rectangle<int> bounds) const override { return bounds; }
    void popupGrabStarted (Component&) override                                {}
    void popupGrabEnded() override                                             {}

    void show() override
    {
        if (hasRequestedShow)
            return;

        hasRequestedShow = true;
        delegate.commitSurface();
        WaylandWindowSystem::getInstance()->flush();
    }

    void setTitle (const String& title) override
    {
        WaylandProtocol::xdgToplevelSetTitle (xdgToplevel.get(), title.toRawUTF8());
    }

    void setSizeConstraints (WaylandSizeConstraints constraints) override
    {
        WaylandProtocol::xdgToplevelSetMinSize (xdgToplevel.get(), constraints.minimum.x, constraints.minimum.y);
        WaylandProtocol::xdgToplevelSetMaxSize (xdgToplevel.get(), constraints.maximum.x, constraints.maximum.y);
    }

    void commitSizeConstraints (Point<int>) override
    {
        if (! hasRequestedShow)
            return;

        delegate.commitSurface();
        WaylandWindowSystem::getInstance()->flush();
    }

    void requestMinimise() override
    {
        WaylandProtocol::xdgToplevelSetMinimized (xdgToplevel.get());
    }

    void requestFullScreen (bool shouldBeFullScreen) override
    {
        if (shouldBeFullScreen)
            WaylandProtocol::xdgToplevelSetFullscreen (xdgToplevel.get());
        else
            WaylandProtocol::xdgToplevelUnsetFullscreen (xdgToplevel.get());
    }

    void requestInteractiveMove (wl_seat& seat, uint32_t serial) override
    {
        WaylandProtocol::xdgToplevelMove (xdgToplevel.get(), &seat, serial);
    }

    void requestInteractiveResize (wl_seat& seat, uint32_t serial, uint32_t resizeEdge) override
    {
        WaylandProtocol::xdgToplevelResize (xdgToplevel.get(), &seat, serial, resizeEdge);
    }

    void contentResized (Point<int>) override
    {
        // xdg-shell gets the new content size from the next surface commit.
    }

private:
    using XdgSurfaceHandle = std::unique_ptr<xdg_surface,
                                             FunctionPointerDestructor<WaylandProtocol::xdgSurfaceDestroy>>;
    using XdgToplevelHandle = std::unique_ptr<xdg_toplevel,
                                              FunctionPointerDestructor<WaylandProtocol::xdgToplevelDestroy>>;
    using DecorationHandle = std::unique_ptr<zxdg_toplevel_decoration_v1,
                                             FunctionPointerDestructor<WaylandProtocol::zxdgToplevelDecorationV1Destroy>>;

    void waylandDidDispatchPendingEvents() override
    {
        const ScopeGuard scope { [&]
        {
            lastConfigureSerial.reset();
            pendingContentSize.reset();
        } };

        if (! lastConfigureSerial.has_value())
            return;

        const ConfigureInfo info
        {
            pendingContentSize,
            hasState (WaylandProtocol::xdgToplevelStateFullscreen),
            hasState (WaylandProtocol::xdgToplevelStateActivated),
            hasState (WaylandProtocol::xdgToplevelStateSuspended),
            isMaximisedOrTiled(),
            hasState (WaylandProtocol::xdgToplevelStateResizing)
        };

        delegate.prepareToplevelConfigure (info);
        WaylandProtocol::xdgSurfaceAckConfigure (xdgSurface.get(), *lastConfigureSerial);
        delegate.finishToplevelConfigure();
    }

    bool hasState (uint32_t state) const
    {
        return (lastConfigureStates & WaylandProtocol::xdgToplevelStateMask (state)) != 0;
    }

    bool isMaximisedOrTiled() const
    {
        return hasState (WaylandProtocol::xdgToplevelStateMaximized)
            || hasState (WaylandProtocol::xdgToplevelStateTiledLeft)
            || hasState (WaylandProtocol::xdgToplevelStateTiledRight)
            || hasState (WaylandProtocol::xdgToplevelStateTiledTop)
            || hasState (WaylandProtocol::xdgToplevelStateTiledBottom);
    }

    void handlePendingConfigure (int32_t width, int32_t height, wl_array* states)
    {
        lastConfigureStates = parseToplevelStates (states);
        pendingContentSize = width > 0 && height > 0 ? std::optional { Point { width, height } }
                                                     : std::nullopt;
    }

    void handleDecorationConfigure (uint32_t mode)
    {
        frameSize = getFrameSizeForXdgDecorationMode (mode);

        diagnostics.lastDecorationMode = mode;
    }

    void handleSurfaceConfigure (uint32_t serial)
    {
        lastConfigureSerial = serial;
    }

    static const xdg_surface_listener surfaceListener;
    static const xdg_toplevel_listener toplevelListener;
    static const zxdg_toplevel_decoration_v1_listener decorationListener;

    Delegate& delegate;
    detail::WaylandPeerDiagnostics& diagnostics;
    wl_surface& surface;
    XdgSurfaceHandle xdgSurface;
    XdgToplevelHandle xdgToplevel;

    // The protocol requires destroying a decoration before its toplevel.
    DecorationHandle decoration;

    std::optional<Point<int>> pendingContentSize;
    std::optional<uint32_t> lastConfigureSerial;
    uint32_t lastConfigureStates = 0;
    ComponentPeer::OptionalBorderSize frameSize { BorderSize<int>() };
    bool hasRequestedShow = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (XdgShellToplevel)
    JUCE_DECLARE_NON_MOVEABLE (XdgShellToplevel)
};

const xdg_surface_listener XdgShellToplevel::surfaceListener
{
    [] (void* data, xdg_surface*, uint32_t serial)
    {
        static_cast<XdgShellToplevel*> (data)->handleSurfaceConfigure (serial);
    }
};

const xdg_toplevel_listener XdgShellToplevel::toplevelListener
{
    [] (void* data, xdg_toplevel*, int32_t width, int32_t height, wl_array* states)
    {
        static_cast<XdgShellToplevel*> (data)->handlePendingConfigure (width, height, states);
    },
    [] (void* data, xdg_toplevel*)
    {
        static_cast<XdgShellToplevel*> (data)->delegate.toplevelCloseRequested();
    },
    [] (void*, xdg_toplevel*, int32_t, int32_t)
    {
        // The compositor supplies these bounds as a sizing hint, which JUCE does not currently use.
    },
    [] (void*, xdg_toplevel*, wl_array*)
    {
        // JUCE does not currently use the compositor's list of supported window operations.
    }
};

const zxdg_toplevel_decoration_v1_listener XdgShellToplevel::decorationListener
{
    [] (void* data, zxdg_toplevel_decoration_v1*, uint32_t mode)
    {
        static_cast<XdgShellToplevel*> (data)->handleDecorationConfigure (mode);
    }
};

//==============================================================================
class LibdecorToplevel final : public WaylandToplevel,
                               private WaylandLoopListener
{
public:
    static std::unique_ptr<LibdecorToplevel> create (Delegate& delegate,
                                                     detail::WaylandPeerDiagnostics& diagnostics,
                                                     wl_surface& surface,
                                                     const String& title,
                                                     bool fullScreenRequested)
    {
        auto result = std::make_unique<LibdecorToplevel> (delegate, diagnostics, surface, title,
                                                          fullScreenRequested);

        if (result->libdecorFrame != nullptr)
            return result;

        return nullptr;
    }

    LibdecorToplevel (Delegate& delegateIn, detail::WaylandPeerDiagnostics& diagnosticsIn,
                      wl_surface& surfaceIn, const String& title, bool fullScreenRequested)
        : delegate (delegateIn),
          diagnostics (diagnosticsIn),
          surface (surfaceIn)
    {
        auto* context = WaylandWindowSystem::getInstance()->getLibdecor();

        if (context == nullptr)
            return;

        libdecorFrame = LibdecorAPI::decorate (context, &surface, &frameInterface, this);

        if (libdecorFrame == nullptr)
            return;

        LibdecorAPI::frameSetTitle (libdecorFrame.get(), title);

        if (const auto appId = getAppId(); appId.isNotEmpty())
            LibdecorAPI::frameSetAppId (libdecorFrame.get(), appId);

        if (fullScreenRequested)
            LibdecorAPI::frameSetFullscreen (libdecorFrame.get(), nullptr);

        WaylandWindowSystem::getInstance()->addLoopListener (*this);
    }

    ~LibdecorToplevel() override
    {
        WaylandWindowSystem::getInstance()->removeLoopListener (*this);
    }

    ComponentPeer::OptionalBorderSize getFrameSizeIfPresent() const override
    {
        // Libdecor uses its plugin's border sizes internally, but its public API does not expose them.
        return {};
    }

    xdg_surface* getXdgSurface() const override
    {
        return LibdecorAPI::frameGetXdgSurface (libdecorFrame.get());
    }

    xdg_toplevel* getXdgToplevel() const override
    {
        return LibdecorAPI::frameGetXdgToplevel (libdecorFrame.get());
    }

    Rectangle<int> getPopupParentBounds (Rectangle<int> contentBounds) const override
    {
        const auto contentTopLeftInFrame = LibdecorAPI::frameTranslateCoordinate (libdecorFrame.get(), {});
        const auto contentBottomRightInFrame = LibdecorAPI::frameTranslateCoordinate (libdecorFrame.get(),
                                                                                      { contentBounds.getWidth(),
                                                                                        contentBounds.getHeight() });

        return makeLibdecorPopupParentBounds (contentBounds, contentTopLeftInFrame, contentBottomRightInFrame);
    }

    void popupGrabStarted (Component& componentToDismiss) override
    {
        if (activePopupGrab.has_value())
        {
            ++activePopupGrab->popupCount;
            return;
        }

        activePopupGrab.emplace (ActivePopupGrab { &componentToDismiss,
                                                   WaylandWindowSystem::getInstance()->getSeatName() });

        // Without a seat name, libdecor cannot be told about the grab, but the popup count
        // still tracks the nested popups so the dismiss callback can close them.
        if (activePopupGrab->seatName.isNotEmpty())
            LibdecorAPI::framePopupGrab (libdecorFrame.get(), activePopupGrab->seatName);
    }

    void popupGrabEnded() override
    {
        if (! activePopupGrab.has_value())
            return;

        if (--activePopupGrab->popupCount > 0)
            return;

        if (activePopupGrab->seatName.isNotEmpty())
            LibdecorAPI::framePopupUngrab (libdecorFrame.get(), activePopupGrab->seatName);

        activePopupGrab.reset();
    }

    void show() override
    {
        if (hasRequestedShow)
            return;

        hasRequestedShow = true;
        LibdecorAPI::frameMap (libdecorFrame.get());
        WaylandWindowSystem::getInstance()->flush();
    }

    void setTitle (const String& title) override
    {
        LibdecorAPI::frameSetTitle (libdecorFrame.get(), title);
    }

    void setSizeConstraints (WaylandSizeConstraints constraints) override
    {
        LibdecorAPI::frameSetMinContentSize (libdecorFrame.get(), constraints.minimum);
        LibdecorAPI::frameSetMaxContentSize (libdecorFrame.get(), constraints.maximum);
    }

    void commitSizeConstraints (Point<int> contentSize) override
    {
        if (! hasRequestedShow)
            return;

        sendLibdecorFrameState (contentSize, nullptr);
        handleCommitRequested();
    }

    void requestMinimise() override
    {
        LibdecorAPI::frameSetMinimized (libdecorFrame.get());
    }

    void requestFullScreen (bool shouldBeFullScreen) override
    {
        if (shouldBeFullScreen)
            LibdecorAPI::frameSetFullscreen (libdecorFrame.get(), nullptr);
        else
            LibdecorAPI::frameUnsetFullscreen (libdecorFrame.get());
    }

    void requestInteractiveMove (wl_seat& seat, uint32_t serial) override
    {
        LibdecorAPI::frameMove (libdecorFrame.get(), &seat, serial);
    }

    void requestInteractiveResize (wl_seat& seat, uint32_t serial, uint32_t resizeEdge) override
    {
        LibdecorAPI::frameResize (libdecorFrame.get(), &seat, serial, getLibdecorResizeEdge (resizeEdge));
    }

    void contentResized (Point<int> contentSize) override
    {
        sendLibdecorFrameState (contentSize, nullptr);
    }

private:
    struct ActivePopupGrab
    {
        WeakReference<Component> componentToDismiss;
        String seatName;
        int popupCount = 1;
    };

    void handleConfigure ([[maybe_unused]] libdecor_frame* frame, libdecor_configuration* configuration)
    {
        // A configure arrived for a frame this backend did not create, which means the frame was
        // given the wrong user data when it was created. Check the libdecor_decorate call.
        jassert (frame == libdecorFrame.get());

        if (const auto configuredState = LibdecorAPI::getConfigurationWindowState (configuration))
            windowState = *configuredState;

        const ConfigureInfo info
        {
            LibdecorAPI::getConfigurationContentSize (configuration, libdecorFrame.get()),
            hasWindowState (libdecorWindowStateFullscreen),
            hasWindowState (libdecorWindowStateActive),
            hasWindowState (libdecorWindowStateSuspended),
            isMaximisedOrTiled(),
            // Configurations do not expose the xdg-shell resizing state.
            std::nullopt
        };

        const auto chosenContentSize = delegate.prepareToplevelConfigure (info);

        // A live resize can deliver several configures per dispatch. Each one must be
        // acknowledged, but the decoration plugin redraws its whole-window shadow buffer on every
        // commit that changes the content size, which is expensive for large windows. Intermediate
        // configures are therefore acknowledged with the size the plugin already has, and the new
        // size is committed once after the dispatch, together with the repaint and resize
        // notification.
        if (lastSentContentSize.has_value() && *lastSentContentSize != chosenContentSize)
        {
            sendLibdecorFrameState (*lastSentContentSize, configuration);
            pendingContentSize = chosenContentSize;
        }
        else
        {
            sendLibdecorFrameState (chosenContentSize, configuration);
            pendingContentSize.reset();
        }

        configureFinishPending = true;
    }

    void waylandDidDispatchPendingEvents() override
    {
        if (const auto contentSize = std::exchange (pendingContentSize, std::nullopt))
            sendLibdecorFrameState (*contentSize, nullptr);

        // The delegate may destroy this toplevel, so nothing may follow the call.
        if (std::exchange (configureFinishPending, false))
            delegate.finishToplevelConfigure();
    }

    bool hasWindowState (libdecor_window_state state) const
    {
        return (windowState & state) != 0;
    }

    bool isMaximisedOrTiled() const
    {
        return hasWindowState (libdecorWindowStateMaximized)
            || hasWindowState (libdecorWindowStateTiledLeft)
            || hasWindowState (libdecorWindowStateTiledRight)
            || hasWindowState (libdecorWindowStateTiledTop)
            || hasWindowState (libdecorWindowStateTiledBottom);
    }

    void sendLibdecorFrameState (Point<int> contentSize, libdecor_configuration* configuration)
    {
        if (! LibdecorAPI::isUsable())
            return;

        const auto state = LibdecorAPI::createState (contentSize);

        // If this fires, libdecor could not allocate the state needed for this commit.
        jassert (state != nullptr);

        if (state != nullptr)
        {
            LibdecorAPI::frameCommit (libdecorFrame.get(), state.get(), configuration);
            lastSentContentSize = contentSize;
        }
    }

    void handleCommitRequested()
    {
        ++diagnostics.commitsSubmitted;
        delegate.commitSurface();
        WaylandWindowSystem::getInstance()->flush();
    }

    void handlePopupDismissalRequested()
    {
        if (activePopupGrab.has_value())
            if (auto* componentToDismiss = activePopupGrab->componentToDismiss.get())
                dismissWaylandPopup (*componentToDismiss);
    }

    static libdecor_frame_interface frameInterface;

    Delegate& delegate;
    detail::WaylandPeerDiagnostics& diagnostics;
    wl_surface& surface;
    LibdecorFrameHandle libdecorFrame;
    libdecor_window_state windowState = libdecorWindowStateNone;
    // Present while any popup above this frame holds a grab.
    std::optional<ActivePopupGrab> activePopupGrab;
    // The content size most recently committed to libdecor, which is what its plugin has drawn.
    std::optional<Point<int>> lastSentContentSize;
    // A configured size that still has to be committed once the current dispatch finishes.
    std::optional<Point<int>> pendingContentSize;
    bool hasRequestedShow = false;
    bool configureFinishPending = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LibdecorToplevel)
    JUCE_DECLARE_NON_MOVEABLE (LibdecorToplevel)
};

// The reserved slots are spelled out because JUCE builds with -Wmissing-field-initializers.
libdecor_frame_interface LibdecorToplevel::frameInterface
{
    [] (libdecor_frame* frame, libdecor_configuration* configuration, void* data)
    {
        static_cast<LibdecorToplevel*> (data)->handleConfigure (frame, configuration);
    },
    [] (libdecor_frame*, void* data)
    {
        static_cast<LibdecorToplevel*> (data)->delegate.toplevelCloseRequested();
    },
    [] (libdecor_frame*, void* data)
    {
        static_cast<LibdecorToplevel*> (data)->handleCommitRequested();
    },
    [] (libdecor_frame*, const char*, void* data)
    {
        static_cast<LibdecorToplevel*> (data)->handlePopupDismissalRequested();
    },
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr
};

//==============================================================================
std::unique_ptr<WaylandToplevel> createWaylandToplevel (WaylandToplevel::Delegate& delegate,
                                                        detail::WaylandPeerDiagnostics& diagnostics,
                                                        wl_surface& surface,
                                                        const String& title,
                                                        WaylandToplevel::NativeTitleBar nativeTitleBar,
                                                        bool fullScreenRequested)
{
    auto* windowSystem = WaylandWindowSystem::getInstance();
    diagnostics.toplevelUsesLibdecor = false;
    const WaylandDecorationCapabilities capabilities
    {
        windowSystem->getDecorationManager() != nullptr,
        windowSystem->getLibdecor() != nullptr && LibdecorAPI::isUsable()
    };

    if (chooseWaylandToplevelBackend (nativeTitleBar, capabilities) == WaylandToplevelBackend::libdecor)
        if (auto result = LibdecorToplevel::create (delegate, diagnostics, surface, title, fullScreenRequested))
        {
            diagnostics.toplevelUsesLibdecor = true;
            return result;
        }

    // The xdg-shell backend is the default and is also used if libdecor creation fails.
    return XdgShellToplevel::create (delegate, diagnostics, surface, title, nativeTitleBar, fullScreenRequested);
}

#if JUCE_UNIT_TESTS

//==============================================================================
class ChooseWaylandToplevelBackendTests final : public UnitTest
{
public:
    ChooseWaylandToplevelBackendTests()
        : UnitTest ("chooseWaylandToplevelBackend", UnitTestCategories::gui) {}

    void runTest() override
    {
        constexpr auto nativeTitleBar = WaylandToplevel::NativeTitleBar::yes;
        constexpr auto noNativeTitleBar = WaylandToplevel::NativeTitleBar::no;

        const WaylandDecorationCapabilities noDecorationSupport;

        WaylandDecorationCapabilities serverSideDecorations;
        serverSideDecorations.decorationManagerAvailable = true;
        serverSideDecorations.libdecorAvailable = true;

        WaylandDecorationCapabilities libdecorOnly;
        libdecorOnly.libdecorAvailable = true;

        testCase ("JUCE-drawn title bars use the xdg-shell backend", [&]
        {
            expect (chooseWaylandToplevelBackend (noNativeTitleBar, libdecorOnly) == WaylandToplevelBackend::xdgShell);
        });

        testCase ("Native title bars use server-side decorations when available", [&]
        {
            expect (chooseWaylandToplevelBackend (nativeTitleBar, serverSideDecorations) == WaylandToplevelBackend::xdgShell);
        });

        testCase ("Native title bars use libdecor when server-side decorations are unavailable", [&]
        {
            expect (chooseWaylandToplevelBackend (nativeTitleBar, libdecorOnly) == WaylandToplevelBackend::libdecor);
        });

        testCase ("Native title bars use xdg-shell when neither decoration backend is available", [&]
        {
            expect (chooseWaylandToplevelBackend (nativeTitleBar, noDecorationSupport) == WaylandToplevelBackend::xdgShell);
        });
    }
};

static ChooseWaylandToplevelBackendTests chooseWaylandToplevelBackendTests;

//==============================================================================
class MakeLibdecorPopupParentBoundsTests final : public UnitTest
{
public:
    MakeLibdecorPopupParentBoundsTests()
        : UnitTest ("makeLibdecorPopupParentBounds", UnitTestCategories::gui) {}

    void runTest() override
    {
        testCase ("Libdecor frame coordinates expand the parent bounds over the left and top borders", [&]
        {
            expect (makeLibdecorPopupParentBounds ({ 100, 50, 400, 300 },
                                                   { 10, 30 },
                                                   { 410, 330 })
                    == Rectangle<int> (90, 20, 410, 330));
        });

        testCase ("A frame with no borders leaves the content bounds unchanged", [&]
        {
            expect (makeLibdecorPopupParentBounds ({ 100, 50, 400, 300 },
                                                   { 0, 0 },
                                                   { 400, 300 })
                    == Rectangle<int> (100, 50, 400, 300));
        });
    }
};

static MakeLibdecorPopupParentBoundsTests makeLibdecorPopupParentBoundsTests;

//==============================================================================
class ParseToplevelStatesTests final : public UnitTest
{
public:
    ParseToplevelStatesTests()
        : UnitTest ("parseToplevelStates", UnitTestCategories::gui) {}

    void runTest() override
    {
        testCase ("An xdg_toplevel configure event with no states reports no state flags", [&]
        {
            expectEquals ((int) parseToplevelStates (nullptr), 0);

            const wl_array unallocatedStates {};
            expectEquals ((int) parseToplevelStates (&unallocatedStates), 0);

            uint32_t storage[] { WaylandProtocol::xdgToplevelStateActivated };
            const wl_array emptyStates { 0, sizeof (storage), storage };
            expectEquals ((int) parseToplevelStates (&emptyStates), 0);
        });

        testCase ("Undefined xdg_toplevel state values are ignored", [&]
        {
            uint32_t storage[] { 0,
                                 WaylandProtocol::xdgToplevelStateActivated,
                                 WaylandProtocol::xdgToplevelStateSuspended + 1 };
            const wl_array states { sizeof (storage), sizeof (storage), storage };

            expectEquals ((int) parseToplevelStates (&states),
                          (int) WaylandProtocol::xdgToplevelStateMask (WaylandProtocol::xdgToplevelStateActivated));
        });
    }
};

static ParseToplevelStatesTests parseToplevelStatesTests;

//==============================================================================
class GetFrameSizeForXdgDecorationModeTests final : public UnitTest
{
public:
    GetFrameSizeForXdgDecorationModeTests()
        : UnitTest ("getFrameSizeForXdgDecorationMode", UnitTestCategories::gui) {}

    void runTest() override
    {
        testCase ("Server-side decoration frame size is unavailable", [&]
        {
            expect (! getFrameSizeForXdgDecorationMode (WaylandProtocol::zxdgToplevelDecorationV1ModeServerSide));
        });

        testCase ("Client-side decoration mode has a known zero frame in the xdg-shell backend", [&]
        {
            const auto frameSize = getFrameSizeForXdgDecorationMode (WaylandProtocol::zxdgToplevelDecorationV1ModeClientSide);
            expect (frameSize && *frameSize == BorderSize<int>());
        });
    }
};

static GetFrameSizeForXdgDecorationModeTests getFrameSizeForXdgDecorationModeTests;

//==============================================================================
class WaylandFullScreenStateTests final : public UnitTest
{
public:
    WaylandFullScreenStateTests()
        : UnitTest ("WaylandFullScreenState", UnitTestCategories::gui) {}

    void runTest() override
    {
        constexpr auto fullScreen = true;
        constexpr auto windowed = false;

        testCase ("The initial state is windowed", [&]
        {
            WaylandFullScreenState state;
            expect (! state.isFullScreen());
            expect (! state.isFullScreenRequested());
        });

        testCase ("A fullscreen request is reported until confirmed, then compositor changes are reported", [&]
        {
            WaylandFullScreenState state;
            state.setFullScreenRequested (fullScreen);
            expect (state.isFullScreen());

            // We cannot tell whether the compositor rejected the request or sent this configure
            // before it received the request.
            state.configureReceived (windowed);
            expect (state.isFullScreen());

            state.configureReceived (fullScreen);
            expect (state.isFullScreen());

            state.configureReceived (windowed);
            expect (! state.isFullScreen());
        });

        testCase ("A stale fullscreen configure does not hide a request to leave fullscreen", [&]
        {
            WaylandFullScreenState state;
            state.setFullScreenRequested (fullScreen);
            state.configureReceived (fullScreen);
            state.setFullScreenRequested (windowed);
            expect (! state.isFullScreen());

            state.configureReceived (fullScreen);
            expect (! state.isFullScreen());

            state.configureReceived (windowed);
            expect (! state.isFullScreen());
        });

        testCase ("Repeating a fullscreen request that is still unconfirmed ignores one more configure", [&]
        {
            WaylandFullScreenState state;
            state.setFullScreenRequested (fullScreen);
            state.setFullScreenRequested (fullScreen);
            expect (state.isFullScreen());

            // A configure sent before the repeated request may still be on its way.
            state.configureReceived (windowed);
            expect (state.isFullScreen());

            state.configureReceived (windowed);
            expect (! state.isFullScreen());
        });

        testCase ("Repeating a confirmed fullscreen request does not hide a later fullscreen exit", [&]
        {
            WaylandFullScreenState state;
            state.setFullScreenRequested (fullScreen);
            state.configureReceived (fullScreen);
            state.setFullScreenRequested (fullScreen);

            state.configureReceived (windowed);
            expect (! state.isFullScreen());
        });

        testCase ("A fullscreen configure does not override a newer request to stay windowed", [&]
        {
            WaylandFullScreenState state;
            state.setFullScreenRequested (fullScreen);
            state.setFullScreenRequested (windowed);
            expect (! state.isFullScreen());

            state.configureReceived (fullScreen);
            expect (! state.isFullScreen());

            state.configureReceived (windowed);
            expect (! state.isFullScreen());
        });

        testCase ("A repeated fullscreen configure confirms that the compositor kept the window fullscreen", [&]
        {
            WaylandFullScreenState state;
            state.setFullScreenRequested (fullScreen);
            state.setFullScreenRequested (windowed);
            expect (! state.isFullScreen());

            // This configure may still be answering the request that was just replaced.
            state.configureReceived (fullScreen);
            expect (! state.isFullScreen());

            // A second fullscreen configure cannot answer the replaced request, so it
            // confirms the compositor's current state.
            state.configureReceived (fullScreen);
            expect (state.isFullScreen());
        });

        testCase ("A fullscreen request is remembered across hide and show, and can be cancelled while hidden", [&]
        {
            WaylandFullScreenState state;
            state.setFullScreenRequested (fullScreen);
            state.configureReceived (fullScreen);
            state.toplevelDestroyed();
            expect (state.isFullScreen());
            expect (state.isFullScreenRequested());

            state.configureReceived (fullScreen);
            expect (state.isFullScreen());
            state.toplevelDestroyed();
            state.setFullScreenRequested (windowed);
            expect (! state.isFullScreenRequested());
            expect (! state.isFullScreen());

            state.configureReceived (windowed);
            expect (! state.isFullScreen());
        });

        testCase ("Destroying a fullscreen toplevel clears its state, and a new toplevel can report fullscreen", [&]
        {
            WaylandFullScreenState state;
            state.configureReceived (fullScreen);
            expect (state.isFullScreen());

            state.toplevelDestroyed();
            expect (! state.isFullScreen());
            expect (! state.isFullScreenRequested());

            state.configureReceived (fullScreen);
            expect (state.isFullScreen());
        });
    }
};

static WaylandFullScreenStateTests waylandFullScreenStateTests;

#endif

} // namespace juce
