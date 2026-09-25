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

static WaylandSizeConstraints getWaylandSizeConstraints (const ComponentBoundsConstrainer& constrainer)
{
    if (const auto* bordered = dynamic_cast<const BorderedComponentBoundsConstrainer*> (&constrainer))
    {
        if (const auto* wrapped = bordered->getWrappedConstrainer())
        {
            const auto constraints = getWaylandSizeConstraints (*wrapped);
            const auto border = bordered->getAdditionalBorder();
            const Point<int> borderSize { border.getLeftAndRight(), border.getTopAndBottom() };

            const auto addBorder = [borderSize] (Point<int> size)
            {
                const auto addExtent = [] (int extent, int borderExtent)
                {
                    return (int) jmin ((int64) std::numeric_limits<int>::max(),
                                       (int64) extent + borderExtent);
                };

                return Point<int> { addExtent (size.x, borderSize.x),
                                    addExtent (size.y, borderSize.y) };
            };

            return { addBorder (constraints.minimum), addBorder (constraints.maximum) };
        }
    }

    return { { constrainer.getMinimumWidth(), constrainer.getMinimumHeight() },
             { constrainer.getMaximumWidth(), constrainer.getMaximumHeight() } };
}

struct WaylandResizeAxes
{
    bool vertical = false;
    bool horizontal = false;

    bool operator== (const WaylandResizeAxes& other) const
    {
        return vertical == other.vertical && horizontal == other.horizontal;
    }

    bool operator!= (const WaylandResizeAxes& other) const
    {
        return ! operator== (other);
    }
};

// An aspect-constrained window can jump if the resize axes change during a drag, so retain the
// initially selected axes until the next resize.
class WaylandResizeAxesTracker final
{
public:
    WaylandResizeAxes update (std::optional<Point<int>> configuredSize, Point<int> currentSize,
                              std::optional<bool> resizing)
    {
        // A false state ends the resize. A true state may outlive the input gesture, so a proposal
        // matching the current size also marks the start of a new resize.
        if (resizing.has_value() && ! *resizing)
        {
            dragAxes.reset();

            if (configuredSize.has_value())
                lastConfiguredSize = configuredSize;

            return {};
        }

        if (! configuredSize.has_value())
            return {};

        const auto changedAxes = std::invoke ([&]() -> WaylandResizeAxes
        {
            if (! lastConfiguredSize.has_value())
                return {};

            const auto widthChange = std::abs (configuredSize->x - lastConfiguredSize->x);
            const auto heightChange = std::abs (configuredSize->y - lastConfiguredSize->y);

            // A width resize may still produce a small height change, and vice versa. When one
            // change is at least twice the other, treat the smaller change as noise.
            if (widthChange > 0 && widthChange >= 2 * heightChange)
                return { false, true };

            if (heightChange > 0 && heightChange >= 2 * widthChange)
                return { true, false };

            return { heightChange > 0, widthChange > 0 };
        });

        lastConfiguredSize = configuredSize;

        if (*configuredSize == currentSize)
        {
            dragAxes.reset();
            return {};
        }

        if (! dragAxes.has_value() && changedAxes != WaylandResizeAxes{})
            dragAxes = changedAxes;

        return dragAxes.value_or (WaylandResizeAxes{});
    }

private:
    std::optional<Point<int>> lastConfiguredSize;
    std::optional<WaylandResizeAxes> dragAxes;
};

// Like other peers do, apply constraints to the whole window, including title bar and borders
static Point<int> constrainWaylandConfiguredSize (ComponentBoundsConstrainer& constrainer,
                                                  BorderSize<int> frame,
                                                  Point<int> previousSize,
                                                  Point<int> configuredSize,
                                                  WaylandResizeAxes axes)
{
    const auto addFrame = [&] (Point<int> size)
    {
        return Rectangle<int> (size.x + frame.getLeftAndRight(), size.y + frame.getTopAndBottom());
    };

    auto bounds = addFrame (configuredSize);

    // A toplevel configure has no position. The maximum-sized limit rectangle prevents checkBounds
    // from applying its on-screen position constraints.
    const auto limit = std::numeric_limits<int>::max();
    constrainer.checkBounds (bounds, addFrame (previousSize), { limit, limit },
                             false, false, axes.vertical, axes.horizontal);

    return { jmax (1, bounds.getWidth() - frame.getLeftAndRight()),
             jmax (1, bounds.getHeight() - frame.getTopAndBottom()) };
}

static uint32_t getXdgResizeEdgeForZone (ResizableBorderComponent::Zone zone)
{
    using F = ResizableBorderComponent::Zone::Zones;

    switch (zone.getZoneFlags())
    {
        case F::top | F::left:      return WaylandProtocol::xdgToplevelResizeEdgeTopLeft;
        case F::top:                return WaylandProtocol::xdgToplevelResizeEdgeTop;
        case F::top | F::right:     return WaylandProtocol::xdgToplevelResizeEdgeTopRight;
        case F::right:              return WaylandProtocol::xdgToplevelResizeEdgeRight;
        case F::bottom | F::right:  return WaylandProtocol::xdgToplevelResizeEdgeBottomRight;
        case F::bottom:             return WaylandProtocol::xdgToplevelResizeEdgeBottom;
        case F::bottom | F::left:   return WaylandProtocol::xdgToplevelResizeEdgeBottomLeft;
        case F::left:               return WaylandProtocol::xdgToplevelResizeEdgeLeft;
    }

    return WaylandProtocol::xdgToplevelResizeEdgeNone;
}

enum class WaylandSurfaceRoleKind
{
    toplevel,
    popup,
    subsurface
};

static WaylandSurfaceRoleKind getWaylandSurfaceRoleKind (int styleFlags, bool hasExplicitParent)
{
    if (hasExplicitParent && (styleFlags & ComponentPeer::windowIsTemporary) != 0)
        return WaylandSurfaceRoleKind::subsurface;

    return getWaylandPopupGrab (styleFlags).has_value() ? WaylandSurfaceRoleKind::popup
                                                        : WaylandSurfaceRoleKind::toplevel;
}

static Rectangle<int> getOpenGLVisibleBufferBounds (Rectangle<int> componentLogicalBounds,
                                                    Rectangle<int> topLevelLogicalBounds,
                                                    Rectangle<int> bufferBounds)
{
    const auto visibleBounds = componentLogicalBounds.getIntersection (topLevelLogicalBounds);

    if (visibleBounds.isEmpty())
        return {};

    return WaylandSurfaceScale::mapLogicalRectToBuffer (visibleBounds - componentLogicalBounds.getPosition(),
                                                        componentLogicalBounds.withZeroOrigin(),
                                                        bufferBounds);
}

//==============================================================================
class WaylandComponentPeer final : public ComponentPeer,
                                   private WaylandToplevel::Delegate,
                                   private WaylandPopup::Delegate,
                                   private WaylandRepaintManager::Delegate,
                                   private WaylandInputHandlerListener,
                                   private WaylandDataDeviceListener,
                                   private WaylandTextInputClient,
                                   private WaylandOutputListener,
                                   private WaylandToplevelMappingListener
{
public:
    WaylandComponentPeer (Component& comp,
                          int windowStyleFlags,
                          WaylandComponentPeer* explicitParent)
        : ComponentPeer (comp, windowStyleFlags),
          logicalBounds (comp.getBoundsInParent())
    {
        JUCE_ASSERT_MESSAGE_MANAGER_IS_LOCKED

        initialToplevelOwner = getActiveToplevelOwnerCandidate();

        auto* windowSystem = WaylandWindowSystem::getInstance();

        if (! windowSystem->isWaylandAvailable())
            return;

        surface.reset (WaylandProtocol::wlCompositorCreateSurface (windowSystem->getCompositor()));

        if (surface == nullptr)
            return;

        callbackState = std::make_unique<CallbackState>();
        callbackState->peer = this;
        windowSystem->addToplevelMappingListener (*this);

        getNativeRealtimeModifiers = [] { return getLinuxRealtimeModifiers (LinuxInputBackend::wayland); };

        if ((windowStyleFlags & windowIgnoresMouseClicks) != 0)
        {
            // An empty input region lets pointer events reach the surface beneath.
            auto* compositor = windowSystem->getCompositor();
            const RegionHandle emptyInputRegion { WaylandProtocol::wlCompositorCreateRegion (compositor) };

            if (emptyInputRegion != nullptr)
                WaylandProtocol::wlSurfaceSetInputRegion (surface.get(), emptyInputRegion.get());
        }

        windowTitle = component.getName();

        switch (getWaylandSurfaceRoleKind (windowStyleFlags, explicitParent != nullptr))
        {
            case WaylandSurfaceRoleKind::toplevel:
                if (! createToplevel())
                    return;
                break;

            case WaylandSurfaceRoleKind::popup:
                if (const auto popupGrab = getWaylandPopupGrab (windowStyleFlags))
                {
                    auto& popupRole = surfaceRole.emplace<PopupRole> (*popupGrab);
                    resolvePopupOwner (popupRole);
                }
                else
                {
                    return;
                }
                break;

            case WaylandSurfaceRoleKind::subsurface:
                jassert (explicitParent != nullptr);
                surfaceRole.emplace<SubsurfaceRole> (*explicitParent);
                diagnostics.usesSubsurfaceRole = true;
                break;
        }

        repainter = std::make_unique<WaylandRepaintManager> (static_cast<WaylandRepaintManager::Delegate&> (*this),
                                                             diagnostics);

        // A peer created for a component that already has an alpha never receives a setAlpha call.
        setWindowAlpha (comp.getAlpha());

        // No repaint or listener notification is needed while the peer is being constructed.
        surfaceScale.setOutputScale (getScaleForSurfaceOutputs());
        createScaleObjects();
        updateToplevelSizeConstraints();
        WaylandProtocol::wlSurfaceAddListener (surface.get(), &surfaceListener, callbackState.get());
        windowSystem->addInputListener (surface.get(), *this);
        windowSystem->addDataDeviceListener (surface.get(), *this);
        windowSystem->addTextInputClient (surface.get(), *this);
        windowSystem->addOutputListener (*this);
        diagnostics.isWaylandBackend = true;
    }

    ~WaylandComponentPeer() override
    {
        JUCE_ASSERT_MESSAGE_MANAGER_IS_LOCKED

        // Each OpenGL child surface keeps a reference to this peer. The OpenGL attachment must
        // detach first. If this fires, an OpenGLContext outlived the window it was attached to.
        jassert (openGLSurfaces.empty());

        // Listener data stays reachable until each proxy is destroyed. Clear the weak
        // peer first so queued callbacks no-op before callbackState is freed.
        if (callbackState != nullptr)
            callbackState->peer = nullptr;

        WaylandWindowSystem::getInstance()->removeTextInputClient (*this);
        WaylandWindowSystem::getInstance()->removeInputListener (*this);
        WaylandWindowSystem::getInstance()->removeDataDeviceListener (*this);
        WaylandWindowSystem::getInstance()->removeOutputListener (*this);
        WaylandWindowSystem::getInstance()->removeToplevelMappingListener (*this);

        alphaModifierSurface.reset();

        // This destroys wl_buffers that the compositor may still hold before the surface is destroyed.
        repainter = nullptr;

        frameCallback.reset();
        pendingActivationToken.reset();

        if (auto* popupRole = getPopupRole(); popupRole != nullptr && popupRole->popup != nullptr)
            WaylandWindowSystem::getInstance()->dismissPopupDescendants (popupRole->popup->getXdgSurface());

        endPopupGrab();

        // Destroy the popup or toplevel before the wl_surface they were created from.
        surfaceRole = {};

        scaleObjects.reset();
        surface.reset();

        callbackState = nullptr;
        WaylandWindowSystem::getInstance()->flush();
    }

    void* getNativeHandle() const override
    {
        return surface.get();
    }

    void setVisible (bool shouldBeVisible) override
    {
        visible = shouldBeVisible;

        if (surface == nullptr)
            return;

        if (visible)
        {
            if (auto* subsurfaceRole = getSubsurfaceRole(); subsurfaceRole != nullptr)
            {
                showSubsurface (*subsurfaceRole);
                return;
            }

            if (auto* popupRole = getPopupRole())
            {
                if (showPopup (*popupRole))
                    return;

                // An owned popup must not become an independent window when its toplevel
                // is unmapped or destroyed.
                if (popupRole->owner.has_value())
                    return;

                // A wl_surface cannot change roles, so a popup that cannot be created falls back
                // to a freestanding toplevel for the lifetime of this peer.
                surfaceRole = ToplevelRole{};
                diagnostics.usesPopupRole = false;
            }

            if (auto* toplevelRole = getToplevelRole())
                showToplevel (*toplevelRole);

            return;
        }

        const WeakReference<WaylandComponentPeer> safeThis { this };
        WaylandWindowSystem::getInstance()->releaseKeyboardFocus (surface.get());

        if (safeThis == nullptr)
            return;

        if (auto* subsurfaceRole = getSubsurfaceRole())
            hideSubsurface (*subsurfaceRole);
        else if (auto* popupRole = getPopupRole())
            hidePopup (*popupRole);
        else if (auto* toplevelRole = getToplevelRole())
            hideToplevel (*toplevelRole);
    }

    void setTitle (const String& title) override
    {
        windowTitle = title;

        if (auto* toplevel = getToplevel())
            toplevel->setTitle (title);
    }

    void setBounds (const Rectangle<int>& newBounds, bool) override
    {
        const auto corrected = newBounds.withSize (jmax (1, newBounds.getWidth()),
                                                   jmax (1, newBounds.getHeight()));

        if (logicalBounds == corrected)
        {
            if (updateToplevelSizeConstraints())
                getToplevel()->commitSizeConstraints (getSurfaceContentSize());

            return;
        }

        const auto movedWithoutResizing = corrected.withZeroOrigin() == logicalBounds.withZeroOrigin();
        logicalBounds = corrected;
        refreshTextInputState();
        updateToplevelSizeConstraints();

        if (auto* subsurfaceRole = getSubsurfaceRole())
        {
            if (subsurfaceRole->subsurface != nullptr)
            {
                if (auto* parent = subsurfaceRole->parent.get())
                {
                    if (const auto positionCommit = parent->updateSubsurfacePosition (*subsurfaceRole->subsurface, logicalBounds))
                        recordPositionForCommit (*positionCommit);
                }
                else
                {
                    configured = false;
                }
            }

            if (configured)
            {
                handleMovedOrResized();

                if (! movedWithoutResizing)
                    repaint (logicalBounds.withZeroOrigin());
            }

            return;
        }

        if (auto* popupRole = getPopupRole(); popupRole != nullptr && popupRole->popup != nullptr)
        {
            const auto placement = makeWaylandPopupPlacement (logicalBounds, popupRole->parent.parentCoordinates);

            if (popupRole->popup->reposition (placement))
            {
                // The configure response supplies the accepted bounds and starts the repaint.
                return;
            }
        }

        if (configured)
        {
            // ComponentPeer has no separate notification for a JUCE title-bar drag. For windows
            // without native decorations, treat a position-only bounds change during a held press
            // as an interactive move.
            if (! hasNativeTitleBar()
                && movedWithoutResizing
                && startInteractiveMoveOrResize (WaylandProtocol::xdgToplevelResizeEdgeNone))
            {
                handleMovedOrResized();
                return;
            }

            if (auto* toplevel = getToplevel())
                toplevel->contentResized (getSurfaceContentSize());

            handleMovedOrResized();
            repaint (logicalBounds.withZeroOrigin());
        }
    }

    Rectangle<int> getBounds() const override
    {
        return logicalBounds;
    }

    OptionalBorderSize getFrameSizeIfPresent() const override
    {
        if (auto* toplevel = getToplevel())
            return toplevel->getFrameSizeIfPresent();

        return OptionalBorderSize { BorderSize<int>() };
    }

    BorderSize<int> getFrameSize() const override
    {
        return {};
    }

    using ComponentPeer::localToGlobal;
    using ComponentPeer::globalToLocal;

    Point<float> localToGlobal (Point<float> point) override
    {
        // Wayland has no global window coordinates, so use the requested logical bounds.
        return point + logicalBounds.getPosition().toFloat();
    }

    Point<float> globalToLocal (Point<float> point) override
    {
        // Use the inverse of the requested-bounds coordinate approximation above.
        return point - logicalBounds.getPosition().toFloat();
    }

    StringArray getAvailableRenderingEngines() override
    {
        return { "Software Renderer" };
    }

    void setMinimised (bool shouldBeMinimised) override
    {
        auto* toplevel = getToplevel();

        // xdg-shell has no request to restore a minimised window.
        if (! shouldBeMinimised || toplevel == nullptr)
            return;

        toplevel->requestMinimise();
        wantsMinimised = true;
    }

    bool isMinimised() const override
    {
        return wantsMinimised;
    }

    bool isShowing() const override
    {
        if (const auto* subsurfaceRole = getSubsurfaceRole())
            return visible && configured
                && subsurfaceRole->parent != nullptr
                && subsurfaceRole->parent->isShowing();

        return visible && configured && ! wantsMinimised;
    }

    void setFullScreen (bool shouldBeFullScreen) override
    {
        fullScreenState.setFullScreenRequested (shouldBeFullScreen);

        if (auto* toplevel = getToplevel())
            toplevel->requestFullScreen (shouldBeFullScreen);
    }

    bool isFullScreen() const override
    {
        return fullScreenState.isFullScreen();
    }

    void startHostManagedResize (Point<int>, ResizableBorderComponent::Zone zone) override
    {
        if (updateToplevelSizeConstraints())
            getToplevel()->commitSizeConstraints (getSurfaceContentSize());

        startInteractiveMoveOrResize (getXdgResizeEdgeForZone (zone));
    }

    bool contains (Point<int> localPos, bool) const override
    {
        // Each peer handles input for its own surface, so local-bounds hit-testing is sufficient.
        return logicalBounds.withZeroOrigin().contains (localPos);
    }

    void toFront (bool makeActive) override
    {
        // TooltipWindow calls toFront when retargeting a visible tooltip, even if its bounds are unchanged.
        if (visible && isWaylandHoverPopup (getStyleFlags()))
            if (auto* popupRole = getPopupRole())
                showPopup (*popupRole);

        // Wayland cannot arbitrarily raise a surface, so update JUCE's window bookkeeping only.
        if (makeActive)
            grabFocus();

        handleBroughtToFront();
    }

    void toBehind (ComponentPeer*) override
    {
        // Wayland has no request for placing one surface behind another.
    }

    bool isFocused() const override
    {
        return WaylandWindowSystem::getInstance()->hasKeyboardFocus (surface.get());
    }

    void grabFocus() override
    {
        if (! canReceiveKeyboardFocus())
            return;

        auto* windowSystem = WaylandWindowSystem::getInstance();

        if (windowSystem->requestKeyboardFocus (surface.get()))
            return;

        if (auto* toplevel = findPeer (getToplevelSurface()))
            toplevel->requestActivation();
    }

    void repaint (const Rectangle<int>& area) override
    {
        // Text and caret changes can happen without keyboard events
        refreshTextInputState();

        if (repainter != nullptr)
            repainter->repaint (area.getIntersection (logicalBounds.withZeroOrigin()));
    }

    void performAnyPendingRepaintsNow() override
    {
        if (repainter != nullptr)
            repainter->performAnyPendingRepaintsNow();
    }

    void setIcon (const Image&) override
    {
        // This backend does not implement a toplevel icon protocol.
    }

    double getPlatformScaleFactor() const noexcept override
    {
        return surfaceScale.getScaleFactor();
    }

    void setCustomPlatformScaleFactor (std::optional<double> scaleIn) override
    {
        handleScaleUpdate (surfaceScale.setOverrideScale (scaleIn));
    }

    std::optional<double> getCustomPlatformScaleFactor() const override
    {
        return surfaceScale.getOverrideScale();
    }

    void setAlpha (float newAlpha) override
    {
        if (repainter != nullptr)
            setWindowAlpha (newAlpha);
    }

    bool setAlwaysOnTop (bool) override
    {
        // Wayland has no request for always-on-top state. Report success because a false
        // return makes Component recreate the window.
        return true;
    }

    void textInputRequired (Point<int>, TextInputTarget& target) override
    {
        WaylandWindowSystem::getInstance()->textInputRequired (surface.get(), target);
    }

    void closeInputMethodContext() override
    {
        WaylandWindowSystem::getInstance()->closeInputMethodContext (surface.get());
    }

    void dismissPendingTextInput() override
    {
        WaylandWindowSystem::getInstance()->dismissPendingTextInput (surface.get());
    }

    std::unique_ptr<WaylandOpenGLSurface> createOpenGLSurface (Component& target);

#if JUCE_WAYLAND_PEER_DIAGNOSTICS
    detail::WaylandPeerDiagnostics getDiagnostics() const
    {
        auto result = diagnostics;

        auto* windowSystem = WaylandWindowSystem::getInstance();
        result.registryGlobalsBound = windowSystem->areRegistryGlobalsBound();
        result.seatBound = windowSystem->isSeatBound();
        result.keyboardBound = windowSystem->isKeyboardBound();
        result.pointerBound = windowSystem->isPointerBound();
        result.touchBound = windowSystem->isTouchBound();
        result.textInput = windowSystem->getTextInputDiagnostics (surface.get());
        result.surfaceScale = getPlatformScaleFactor();
        result.fractionalScaleActive = scaleObjects.has_value() && scaleObjects->hasFractionalScale();
        result.boundOutputs = windowSystem->getNumBoundOutputs();
        result.enteredOutputs = surfaceOutputs.size();

        if (repainter != nullptr)
        {
            result.bufferPoolSize = repainter->getBufferPoolSize();
            result.busyBuffers = repainter->getBusyBufferCount();
        }

        return result;
    }
#endif

private:
    using ViewportHandle = std::unique_ptr<wp_viewport, FunctionPointerDestructor<WaylandProtocol::wpViewportDestroy>>;
    using FractionalScaleHandle = std::unique_ptr<wp_fractional_scale_v1,
                                                  FunctionPointerDestructor<WaylandProtocol::wpFractionalScaleV1Destroy>>;
    using AlphaModifierSurfaceHandle = std::unique_ptr<wp_alpha_modifier_surface_v1,
                                                       FunctionPointerDestructor<WaylandProtocol::wpAlphaModifierSurfaceV1Destroy>>;
    using RegionHandle = std::unique_ptr<wl_region, FunctionPointerDestructor<WaylandProtocol::wlRegionDestroy>>;

    struct CallbackState
    {
        WeakReference<WaylandComponentPeer> peer;
    };

    static WeakReference<WaylandComponentPeer> getPeerFromCallbackState (void* data)
    {
        if (auto* state = static_cast<CallbackState*> (data))
            return state->peer;

        return {};
    }

    struct PopupGrab
    {
        WeakReference<Component> componentToDismiss;
        uint32_t serial;
    };

    enum class CommitChanges { no, yes };

    struct ToplevelRole
    {
        std::unique_ptr<WaylandToplevel> toplevel;
        std::optional<WaylandSizeConstraints> sizeConstraints;
        WaylandToplevelRelationship relationship;
    };

    struct PopupRole
    {
        explicit PopupRole (WaylandPopupGrab grabIn) : grabPolicy (grabIn) {}

        struct Owner
        {
            WeakReference<WaylandComponentPeer> toplevel;
        };

        WaylandPopupGrab grabPolicy;
        std::optional<Owner> owner;
        WaylandPopupParent parent;
        std::unique_ptr<WaylandPopup> popup;
        std::optional<PopupGrab> grab;
    };

    struct SubsurfaceRole
    {
        explicit SubsurfaceRole (WaylandComponentPeer& parentIn) : parent (&parentIn) {}

        WeakReference<WaylandComponentPeer> parent;
        std::unique_ptr<WaylandSubsurface> subsurface;
    };

    using CallbackHandle = std::unique_ptr<wl_callback, FunctionPointerDestructor<WaylandProtocol::wlCallbackDestroy>>;

    // A sync reply confirms that the compositor has processed the preceding commit request.
    struct CommitSync
    {
        CallbackHandle callback;
        uint64 commitSequence;
    };

    ToplevelRole* getToplevelRole() noexcept { return std::get_if<ToplevelRole> (&surfaceRole); }
    const ToplevelRole* getToplevelRole() const noexcept { return std::get_if<ToplevelRole> (&surfaceRole); }
    PopupRole* getPopupRole() noexcept { return std::get_if<PopupRole> (&surfaceRole); }
    const PopupRole* getPopupRole() const noexcept { return std::get_if<PopupRole> (&surfaceRole); }
    SubsurfaceRole* getSubsurfaceRole() noexcept { return std::get_if<SubsurfaceRole> (&surfaceRole); }
    const SubsurfaceRole* getSubsurfaceRole() const noexcept { return std::get_if<SubsurfaceRole> (&surfaceRole); }

    WaylandToplevel* getToplevel() const noexcept
    {
        auto* toplevelRole = getToplevelRole();
        return toplevelRole != nullptr ? toplevelRole->toplevel.get() : nullptr;
    }

    WaylandPopup* getPopup() const noexcept
    {
        auto* popupRole = getPopupRole();
        return popupRole != nullptr ? popupRole->popup.get() : nullptr;
    }

    class OpenGLSurface final : public WaylandOpenGLSurface
    {
    public:
        using SurfaceHandle = std::unique_ptr<wl_surface,
                                              FunctionPointerDestructor<WaylandProtocol::wlSurfaceDestroy>>;

        static std::unique_ptr<WaylandOpenGLSurface> create (WaylandComponentPeer& peer, Component& component)
        {
            auto* windowSystem = WaylandWindowSystem::getInstance();
            auto* compositor = windowSystem->getCompositor();
            auto* viewporter = windowSystem->getViewporter();

            if (compositor == nullptr
                || viewporter == nullptr
                || peer.surface == nullptr
                || peer.getBufferMappingMethod() != WaylandSurfaceScale::BufferMappingMethod::viewport)
                return {};

            SurfaceHandle surface { WaylandProtocol::wlCompositorCreateSurface (compositor) };

            if (surface == nullptr)
                return {};

            auto subsurface = WaylandSubsurface::create (*surface, *peer.surface);

            if (subsurface == nullptr)
                return {};

            ViewportHandle viewport { WaylandProtocol::wpViewporterGetViewport (viewporter, surface.get()) };

            if (viewport == nullptr)
                return {};

            const RegionHandle emptyInputRegion { WaylandProtocol::wlCompositorCreateRegion (compositor) };

            if (emptyInputRegion == nullptr)
                return {};

            WaylandProtocol::wlSurfaceSetInputRegion (surface.get(), emptyInputRegion.get());
            return rawToUniquePtr (new OpenGLSurface (peer,
                                                      component,
                                                      *windowSystem,
                                                      std::move (surface),
                                                      std::move (subsurface),
                                                      std::move (viewport)));
        }

        ~OpenGLSurface() override
        {
            // The GL thread has stopped and this runs on the message thread, so a frame
            // callback cannot be created or completed concurrently.
            peer.openGLSurfaces.erase (this);
            frameCallback.reset();
            viewport.reset();
            subsurface.reset();
            surface.reset();
            windowSystem.flush();
        }

        wl_display* getDisplay() const noexcept override { return windowSystem.getDisplay(); }
        wl_surface* getSurface() const noexcept override { return surface.get(); }

        BoundsUpdate updateBounds (Point<int> attachedBufferSize) override
        {
            const auto previous = std::exchange (crop, computeCrop());
            const auto bufferSize = getBufferSize();

            ++peer.diagnostics.openGLBoundsUpdates;
            peer.diagnostics.openGLCropWidth  = crop.visibleSurfaceBounds.getWidth();
            peer.diagnostics.openGLCropHeight = crop.visibleSurfaceBounds.getHeight();

            if (! isVisible())
            {
                if (! previous.visibleSurfaceBounds.isEmpty())
                    hide (previous.componentLogicalBounds);

                return { bufferSize, false };
            }

            if (crop != previous)
                updateSubsurfaceBounds (previous.componentLogicalBounds.getUnion (crop.componentLogicalBounds),
                                        attachedBufferSize);

            return { bufferSize, true };
        }

        void applyViewport (Point<int> bufferSize) override
        {
            const auto source = getOpenGLVisibleBufferBounds (crop.componentLogicalBounds,
                                                              crop.topLevelLogicalBounds,
                                                              { bufferSize.x, bufferSize.y });

            // An empty viewport source is a protocol error
            jassert (! source.isEmpty());

            if (source.isEmpty())
                return;

            WaylandProtocol::wpViewportSetSource (viewport.get(), source.getX(), source.getY(),
                                                  source.getWidth(), source.getHeight());
            WaylandProtocol::wpViewportSetDestination (viewport.get(),
                                                       crop.visibleSurfaceBounds.getWidth(),
                                                       crop.visibleSurfaceBounds.getHeight());
        }

        bool requestFrameCallback (std::function<void()> callback) override
        {
            const std::scoped_lock lock { frameCallbackMutex };

            if (frameCallback != nullptr)
                return false;

            frameCallback.reset (WaylandProtocol::wlSurfaceFrame (surface.get()));

            if (frameCallback == nullptr)
                return false;

            frameCallbackFunction = std::move (callback);

            if (WaylandProtocol::wlCallbackAddListener (frameCallback.get(), &frameListener, this) != 0)
            {
                frameCallback.reset();
                frameCallbackFunction = nullptr;
                return false;
            }

            return true;
        }

        void handleParentCommit()
        {
            if (std::exchange (waitingForParentCommit, false))
                subsurface->setDesync();
        }

    private:
        struct Crop
        {
            Rectangle<int> componentLogicalBounds;
            Rectangle<int> topLevelLogicalBounds;
            Rectangle<int> visibleSurfaceBounds;

            auto tie() const { return std::tie (componentLogicalBounds, topLevelLogicalBounds, visibleSurfaceBounds); }
            bool operator== (const Crop& other) const { return tie() == other.tie(); }
            bool operator!= (const Crop& other) const { return tie() != other.tie(); }
        };

        OpenGLSurface (WaylandComponentPeer& peerIn,
                       Component& componentIn,
                       WaylandWindowSystem& windowSystemIn,
                       SurfaceHandle surfaceIn,
                       std::unique_ptr<WaylandSubsurface> subsurfaceIn,
                       ViewportHandle viewportIn)
            : peer (peerIn),
              component (componentIn),
              windowSystem (windowSystemIn),
              surface (std::move (surfaceIn)),
              subsurface (std::move (subsurfaceIn)),
              viewport (std::move (viewportIn))
        {
            peer.openGLSurfaces.insert (this);
        }

        bool isVisible() const noexcept { return ! crop.visibleSurfaceBounds.isEmpty(); }

        Crop computeCrop() const
        {
            if (component.getPeer() != &peer)
                return {};

            const auto componentLogicalBounds = peer.getAreaCoveredBy (component);
            const auto topLevelLogicalBounds = peer.logicalBounds.withZeroOrigin();
            const auto visibleLogicalBounds = componentLogicalBounds.getIntersection (topLevelLogicalBounds);

            if (visibleLogicalBounds.isEmpty())
                return { componentLogicalBounds, topLevelLogicalBounds, {} };

            const auto surfaceSize = peer.getSurfaceContentSize();
            return { componentLogicalBounds,
                     topLevelLogicalBounds,
                     WaylandSurfaceScale::mapLogicalRectToSurface (visibleLogicalBounds,
                                                                   topLevelLogicalBounds,
                                                                   { surfaceSize.x, surfaceSize.y }) };
        }

        Point<int> getBufferSize() const
        {
            if (crop.componentLogicalBounds.isEmpty())
                return {};

            const auto bounds = peer.surfaceScale.getBufferGeometry (crop.componentLogicalBounds.withZeroOrigin(),
                                                                     WaylandSurfaceScale::BufferMappingMethod::viewport).bufferBounds;
            return { jmax (1, bounds.getWidth()), jmax (1, bounds.getHeight()) };
        }

        void updateSubsurfaceBounds (Rectangle<int> repaintBounds, Point<int> attachedBufferSize)
        {
            subsurface->setSync();
            subsurface->setPosition (crop.visibleSurfaceBounds.getPosition());
            waitingForParentCommit = true;

            if (attachedBufferSize.x > 0 && attachedBufferSize.y > 0)
            {
                applyViewport (attachedBufferSize);
                WaylandProtocol::wlSurfaceCommit (surface.get());
            }

            peer.repaint (repaintBounds);
        }

        void hide (Rectangle<int> repaintBounds)
        {
            WaylandProtocol::wlSurfaceAttach (surface.get(), nullptr, 0, 0);
            WaylandProtocol::wlSurfaceCommit (surface.get());
            peer.repaint (repaintBounds);

            // The pending frame callback belongs to the unmapped content and may never arrive.
            const std::scoped_lock lock { frameCallbackMutex };
            frameCallback.reset();
            frameCallbackFunction = nullptr;
        }

        void handleFrameDone (wl_callback* callback)
        {
            std::function<void()> function;

            {
                const std::scoped_lock lock { frameCallbackMutex };

                if (callback != frameCallback.get())
                    return;

                frameCallback.reset();
                function = std::move (frameCallbackFunction);
            }

            NullCheckedInvocation::invoke (function);
        }

        static const wl_callback_listener frameListener;

        WaylandComponentPeer& peer;
        Component& component;
        WaylandWindowSystem& windowSystem;
        SurfaceHandle surface;
        std::unique_ptr<WaylandSubsurface> subsurface;
        ViewportHandle viewport;
        std::mutex frameCallbackMutex;
        CallbackHandle frameCallback;
        std::function<void()> frameCallbackFunction;
        Crop crop;
        bool waitingForParentCommit = false;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OpenGLSurface)
    };

    class ScaleObjects final
    {
    public:
        ScaleObjects (ViewportHandle viewportIn, FractionalScaleHandle fractionalScaleIn)
            : viewport (std::move (viewportIn)),
              fractionalScale (std::move (fractionalScaleIn))
        {
            // This object represents viewport-backed mapping, so it is invalid without a viewport.
            jassert (viewport != nullptr);
        }

        wp_viewport* getViewport() const noexcept { return viewport.get(); }
        bool hasFractionalScale() const noexcept { return fractionalScale != nullptr; }

    private:
        ViewportHandle viewport;
        FractionalScaleHandle fractionalScale;
    };

    bool isFrameCallbackPending() const override
    {
        return frameCallback != nullptr;
    }

    std::optional<WaylandRepaintManager::FrameState> getFrameState() const override
    {
        if (! configured || ! visible || surface == nullptr || frameCallback != nullptr)
            return std::nullopt;

        const auto bounds = logicalBounds.withZeroOrigin();
        return WaylandRepaintManager::FrameState { bounds,
                                                   surfaceScale.getBufferGeometry (bounds, getBufferMappingMethod()) };
    }

    std::unique_ptr<WaylandShmBuffer> createBuffer (int width, int height,
                                                    const wl_buffer_listener* listener,
                                                    void* listenerData) override
    {
        auto* shm = WaylandWindowSystem::getInstance()->getShm();

        if (shm == nullptr)
            return nullptr;

        auto buffer = WaylandShmBuffer::create (*shm, width, height);

        if (buffer == nullptr)
            return nullptr;

        WaylandProtocol::wlBufferAddListener (buffer->handle.get(), listener, listenerData);
        return buffer;
    }

    void paint (Image& target,
                const RectangleList<int>& paintRegions,
                Rectangle<int> bufferBounds,
                Rectangle<int> logicalBoundsIn) override
    {
        for (const auto& region : paintRegions)
            target.clear (region);

        auto context = getComponent().getLookAndFeel().createGraphicsContext (target, Point<int>(), paintRegions);

        // Map logical component coordinates across the whole rounded buffer.
        context->addTransform (AffineTransform::scale ((float) bufferBounds.getWidth() / (float) logicalBoundsIn.getWidth(),
                                                       (float) bufferBounds.getHeight() / (float) logicalBoundsIn.getHeight()));
        handlePaint (*context);
    }

    void copyToBuffer (const Image& source,
                       WaylandShmBuffer& buffer,
                       const RectangleList<int>& paintRegions) override
    {
        // Without the alpha modifier the window alpha has to be baked into the pixels.
        const auto alphaMultiplier = std::invoke ([&]() -> std::optional<uint8>
        {
            if (alphaModifierSurface != nullptr || 1.0f <= windowAlpha)
                return std::nullopt;

            return (uint8) roundToInt (windowAlpha * 255.0f);
        });

        copyImageRegionsToWaylandShmBuffer (source, buffer, paintRegions, alphaMultiplier);
    }

    void submitFrame (const WaylandSurfaceScale::BufferGeometry& geometry,
                      WaylandShmBuffer& buffer,
                      const RectangleList<int>& damageRegions) override
    {
        WaylandProtocol::wlSurfaceSetBufferScale (surface.get(), geometry.bufferScale);

        if (geometry.viewportDestination.has_value())
        {
            // A viewport destination is only produced when scaleObjects exists.
            WaylandProtocol::wpViewportSetDestination (scaleObjects->getViewport(),
                                                       geometry.viewportDestination->getWidth(),
                                                       geometry.viewportDestination->getHeight());
        }

        updateOpaqueRegionIfChanged();

        WaylandProtocol::wlSurfaceAttach (surface.get(), buffer.handle.get(), 0, 0);

        for (const auto& region : damageRegions)
            WaylandProtocol::wlSurfaceDamageBuffer (surface.get(), region.getX(), region.getY(), region.getWidth(), region.getHeight());

        const auto wasMapped = hasCommittedBuffer;
        requestFrameCallback();
        commitSurface();
        hasCommittedBuffer = true;

        for (auto* openGLSurface : openGLSurfaces)
            openGLSurface->handleParentCommit();

        // Children can restore their parent only after this toplevel's mapping commit is queued.
        if (! wasMapped && getToplevelRole() != nullptr)
            WaylandWindowSystem::getInstance()->toplevelMapped (*this);

        WaylandWindowSystem::getInstance()->flush();
    }

    void setWindowAlpha (float newAlpha)
    {
        const auto clamped = jlimit (0.0f, 1.0f, newAlpha);

        if (approximatelyEqual (clamped, windowAlpha))
            return;

        const auto hadBakedAlpha = alphaModifierSurface == nullptr && windowAlpha < 1.0f;

        windowAlpha = clamped;

        if (! createAlphaModifierSurfaceIfNeeded())
        {
            // Without the protocol the alpha is multiplied into the buffer while blitting, so
            // every pixel of the window changes.
            repaint (logicalBounds.withZeroOrigin());
            return;
        }

        const auto factor = std::round ((double) clamped * (double) std::numeric_limits<uint32_t>::max());
        WaylandProtocol::wpAlphaModifierSurfaceV1SetMultiplier (alphaModifierSurface.get(), (uint32_t) factor);

        if (hadBakedAlpha)
        {
            // Buffers painted before the modifier existed still have alpha baked into their pixels,
            // so a repaint replaces them in the same commit that applies the multiplier.
            repaint (logicalBounds.withZeroOrigin());
            return;
        }

        // The compositor applies the new window alpha on the next surface commit, so an idle
        // window needs a commit of its own.
        if (configured && visible && surface != nullptr)
        {
            updateOpaqueRegionIfChanged();
            ++diagnostics.commitsSubmitted;
            commitSurface();
            WaylandWindowSystem::getInstance()->flush();
        }
    }

    // Asking the compositor for a second alpha-modifier surface for the same wl_surface is a
    // protocol error, so the handle is created once and kept.
    bool createAlphaModifierSurfaceIfNeeded()
    {
        if (alphaModifierSurface != nullptr)
            return true;

        auto* alphaModifier = WaylandWindowSystem::getInstance()->getAlphaModifier();

        if (alphaModifier == nullptr || surface == nullptr)
            return false;

        alphaModifierSurface.reset (WaylandProtocol::wpAlphaModifierV1GetSurface (alphaModifier, surface.get()));
        return alphaModifierSurface != nullptr;
    }

    void updateOpaqueRegionIfChanged()
    {
        auto* compositor = WaylandWindowSystem::getInstance()->getCompositor();

        if (surface == nullptr || compositor == nullptr)
            return;

        const auto shouldBeOpaque = getComponent().isOpaque() && 1.0f <= windowAlpha;

        if (lastCommittedOpaque == shouldBeOpaque)
            return;

        if (! shouldBeOpaque)
        {
            WaylandProtocol::wlSurfaceSetOpaqueRegion (surface.get(), nullptr);
            lastCommittedOpaque = shouldBeOpaque;
            return;
        }

        // set_opaque_region copies the region, so the handle can go out of scope straight away.
        const RegionHandle region { WaylandProtocol::wlCompositorCreateRegion (compositor) };

        if (region == nullptr)
            return;

        // The compositor clips the opaque region to the surface. An oversized rectangle avoids
        // the fractional-scale case, where the buffer rounds up and a computed rectangle would
        // either miss or overclaim the last row and column.
        WaylandProtocol::wlRegionAdd (region.get(), 0, 0,
                                      std::numeric_limits<int32_t>::max(), std::numeric_limits<int32_t>::max());
        WaylandProtocol::wlSurfaceSetOpaqueRegion (surface.get(), region.get());

        lastCommittedOpaque = shouldBeOpaque;
    }

    bool hasNativeTitleBar() const
    {
        return (getStyleFlags() & windowHasTitleBar) != 0;
    }

    bool createToplevel()
    {
        auto* toplevelRole = getToplevelRole();

        if (toplevelRole == nullptr)
            return false;

        const auto nativeTitleBar = hasNativeTitleBar() ? WaylandToplevel::NativeTitleBar::yes
                                                        : WaylandToplevel::NativeTitleBar::no;

        toplevelRole->relationship = {};
        toplevelRole->sizeConstraints.reset();
        resizeAxesTracker = {};
        toplevelRole->toplevel = createWaylandToplevel (*this, diagnostics, *surface, windowTitle, nativeTitleBar,
                                                        fullScreenState.isFullScreenRequested());

        if (toplevelRole->toplevel == nullptr)
            return false;

        updateToplevelRelationship (CommitChanges::no);
        updateToplevelSizeConstraints();
        return true;
    }

    void showToplevel (ToplevelRole& toplevelRole)
    {
        captureDialogOwner();
        updateToplevelRelationship (CommitChanges::no);

        if (toplevelRole.toplevel == nullptr && ! createToplevel())
            return;

        toplevelRole.toplevel->show();
        repaint (logicalBounds.withZeroOrigin());
    }

    bool showPopup (PopupRole& popupRole)
    {
        if (! resolvePopupOwner (popupRole))
            return false;

        if (popupRole.popup == nullptr && ! createPopup (popupRole))
            return false;

        popupRole.popup->show();
        repaint (logicalBounds.withZeroOrigin());
        return true;
    }

    void showSubsurface (SubsurfaceRole& subsurfaceRole)
    {
        if (subsurfaceRole.subsurface == nullptr && ! createSubsurface (subsurfaceRole))
            return;

        repaint (logicalBounds.withZeroOrigin());
    }

    void submitUnmapCommit()
    {
        // The compositor may not deliver the release for a button held on an unmapped surface.
        WaylandWindowSystem::getInstance()->surfaceUnmapped (surface.get());

        // A pending frame callback belongs to the old mapping and may never arrive.
        frameCallback.reset();
        WaylandProtocol::wlSurfaceAttach (surface.get(), nullptr, 0, 0);
        commitSurface();
        WaylandWindowSystem::getInstance()->flush();
        ++diagnostics.unmapCommits;

        configured = false;
        hasCommittedBuffer = false;
    }

    void hideToplevel (ToplevelRole& toplevelRole)
    {
        if (toplevelRole.toplevel == nullptr)
            return;

        // Weston does not configure a surface remapped after an empty-buffer unmap,
        // so destroy and recreate its xdg_toplevel.
        toplevelRole.relationship = {};
        toplevelRole.toplevel.reset();
        submitUnmapCommit();

        // The compositor discards all toplevel state on unmap.
        fullScreenState.toplevelDestroyed();
        wantsMinimised = false;
        dialogOwner = nullptr;
    }

    void hidePopup (PopupRole& popupRole)
    {
        if (popupRole.popup == nullptr)
            return;

        WaylandWindowSystem::getInstance()->dismissPopupDescendants (popupRole.popup->getXdgSurface());
        unmapPopup (popupRole);
    }

    void unmapPopup (PopupRole& popupRole)
    {
        // Weston does not configure a surface remapped after an empty-buffer unmap,
        // so destroy and recreate its xdg_popup.
        endPopupGrab();
        popupRole.popup.reset();
        submitUnmapCommit();
    }

    void hideSubsurface (SubsurfaceRole& subsurfaceRole)
    {
        if (subsurfaceRole.subsurface == nullptr)
            return;

        subsurfaceRole.subsurface.reset();
        submitUnmapCommit();
    }

    bool isDialogToplevel() const
    {
        if (getToplevelRole() == nullptr || (getStyleFlags() & windowIsTemporary) != 0)
            return false;

        return dynamic_cast<const AlertWindow*> (&component) != nullptr
            || dynamic_cast<const DialogWindow*> (&component) != nullptr;
    }

    Component* getActiveToplevelOwnerCandidate() const
    {
        auto* active = TopLevelWindow::getActiveTopLevelWindow();

        if (active == nullptr || active == component.getTopLevelComponent() || active->getPeer() == this)
            return nullptr;

        return active;
    }

    void captureDialogOwner()
    {
        if (! isDialogToplevel() || dialogOwner != nullptr)
            return;

        auto* candidate = getActiveToplevelOwnerCandidate();

        if (candidate == nullptr)
            candidate = initialToplevelOwner.get();

        auto* candidatePeer = candidate != nullptr ? dynamic_cast<WaylandComponentPeer*> (candidate->getPeer()) : nullptr;

        if (candidatePeer != nullptr && ! candidatePeer->ownerChainContains (*this))
            dialogOwner = candidate->getTopLevelComponent();
    }

    WaylandComponentPeer* getWaylandOwnerPeer() const
    {
        return dialogOwner != nullptr ? dynamic_cast<WaylandComponentPeer*> (dialogOwner->getPeer()) : nullptr;
    }

    bool ownerChainContains (const WaylandComponentPeer& peer) const
    {
        for (auto* p = this; p != nullptr; p = p->getWaylandOwnerPeer())
            if (p == &peer)
                return true;

        return false;
    }

    bool updateToplevelParent()
    {
        auto* toplevelRole = getToplevelRole();

        if (toplevelRole == nullptr || toplevelRole->toplevel == nullptr)
            return false;

        auto* ownerPeer = getWaylandOwnerPeer();
        auto* ownerToplevel = ownerPeer != nullptr ? ownerPeer->getToplevel() : nullptr;
        auto* xdgToplevel = toplevelRole->toplevel->getXdgToplevel();
        auto* ownerXdgToplevel = ownerToplevel != nullptr ? ownerToplevel->getXdgToplevel() : nullptr;

        if (xdgToplevel == nullptr)
            return false;

        return toplevelRole->relationship.updateParent (*xdgToplevel, ownerXdgToplevel);
    }

    void updateToplevelRelationship (CommitChanges commitChanges = CommitChanges::yes)
    {
        const auto parentChanged = updateToplevelParent();

        if (parentChanged && commitChanges == CommitChanges::yes && surface != nullptr && visible)
            commitSurface();

        if (parentChanged)
            WaylandWindowSystem::getInstance()->flush();
    }

    bool startInteractiveMoveOrResize (uint32_t resizeEdge)
    {
        auto* toplevel = getToplevel();

        if (toplevel == nullptr)
            return false;

        auto* windowSystem = WaylandWindowSystem::getInstance();
        auto* seat = windowSystem->getSeat();
        const auto serial = windowSystem->getHeldPressSerial (surface.get());

        if (seat == nullptr || ! serial.has_value())
            return false;

        if (resizeEdge == WaylandProtocol::xdgToplevelResizeEdgeNone)
            toplevel->requestInteractiveMove (*seat, *serial);
        else
            toplevel->requestInteractiveResize (*seat, *serial, resizeEdge);

        windowSystem->dragHandedToCompositor (surface.get());
        windowSystem->flush();
        return true;
    }

    bool createSubsurface (SubsurfaceRole& subsurfaceRole)
    {
        auto* parent = subsurfaceRole.parent.get();

        if (parent == nullptr || parent->surface == nullptr)
            return false;

        subsurfaceRole.subsurface = WaylandSubsurface::create (*surface, *parent->surface);

        if (subsurfaceRole.subsurface == nullptr)
            return false;

        configured = true;

        if (const auto positionCommit = parent->updateSubsurfacePosition (*subsurfaceRole.subsurface, logicalBounds))
            recordPositionForCommit (*positionCommit);

        return true;
    }

    // Returns the parent commit that will apply the position.
    std::optional<uint64> updateSubsurfacePosition (WaylandSubsurface& child, Rectangle<int> childLogicalBounds)
    {
        if (surface == nullptr)
            return std::nullopt;

        child.setPosition (getWaylandSubsurfacePosition (childLogicalBounds,
                                                         logicalBounds,
                                                         getSurfaceContentSize()));

        // Subsurface position takes effect with the next parent commit. Repaint the parent
        // so the position is applied with a complete buffer update.
        nextCommitCarriesPosition = true;
        repaint (logicalBounds.withZeroOrigin());
        return nextCommitSequence();
    }

    static WaylandComponentPeer* findPeer (wl_surface* surfaceToFind)
    {
        return static_cast<WaylandComponentPeer*> (WaylandWindowSystem::getInstance()->findInputListener (surfaceToFind));
    }

    bool resolvePopupOwner (PopupRole& popupRole)
    {
        // Popups keep their first owner. Only hover popups may switch toplevels when reused.
        if (popupRole.owner.has_value() && ! isWaylandHoverPopup (getStyleFlags()))
            return popupRole.owner->toplevel != nullptr;

        auto* windowSystem = WaylandWindowSystem::getInstance();
        const auto owner = windowSystem->findPopupOwner (getStyleFlags());

        if (! owner.has_value())
            return popupRole.owner.has_value() && popupRole.owner->toplevel != nullptr;

        auto* toplevel = findPeer (owner->toplevelSurface);

        if (toplevel == nullptr)
            return false;

        if (popupRole.owner.has_value() && popupRole.owner->toplevel.get() == toplevel)
            return true;

        // A visible tooltip may be reused over another window without recreating its peer.
        if (popupRole.popup != nullptr)
            hidePopup (popupRole);

        popupRole.owner = PopupRole::Owner { toplevel };
        return true;
    }

    bool createPopup (PopupRole& popupRole)
    {
        auto* windowSystem = WaylandWindowSystem::getInstance();

        // Resolve the parent and grab serial on each map because both can become stale.
        if (auto refreshed = windowSystem->findPopupParent (popupRole.owner->toplevel->surface.get(), popupRole.grabPolicy))
            popupRole.parent = std::move (*refreshed);
        else
            return false;

        auto& parent = popupRole.parent;

        if (parent.parentXdgSurface == nullptr)
            return false;

        // Popup components may recreate their peer before becoming visible, for example when
        // setOpaque() changes. Claim the serial only when the popup maps and a seat exists.
        const auto grabSerial = std::invoke ([&]() -> std::optional<uint32_t>
        {
            if (parent.grabSerial == nullptr || windowSystem->getSeat() == nullptr)
                return std::nullopt;

            return parent.grabSerial->claim();
        });

        const auto placement = makeWaylandPopupPlacement (logicalBounds, parent.parentCoordinates);
        popupRole.popup = WaylandPopup::create (*this,
                                                *surface,
                                                *parent.parentXdgSurface,
                                                placement,
                                                grabSerial);

        if (popupRole.popup == nullptr)
            return false;

        diagnostics.usesPopupRole = true;
        diagnostics.popupHasGrab = grabSerial.has_value();

        if (grabSerial.has_value())
        {
            auto* componentToDismiss = parent.componentToDismiss != nullptr ? parent.componentToDismiss
                                                                            : &component;
            popupRole.grab = PopupGrab { componentToDismiss, *grabSerial };
            windowSystem->popupGrabStarted (getToplevelSurface(), *componentToDismiss);
        }

        return true;
    }

    void endPopupGrab()
    {
        if (auto* popupRole = getPopupRole(); popupRole != nullptr && popupRole->grab.has_value())
        {
            popupRole->grab.reset();
            diagnostics.popupHasGrab = false;
            WaylandWindowSystem::getInstance()->popupGrabEnded (getToplevelSurface());
        }
    }

    Point<int> getSurfaceContentSize() const
    {
        const auto surfaceSize = surfaceScale.getSurfaceSize (logicalBounds.withZeroOrigin(),
                                                              getBufferMappingMethod());
        return { jmax (1, roundToInt (surfaceSize.x)), jmax (1, roundToInt (surfaceSize.y)) };
    }

    Point<int> getSurfaceConstraintSize (Point<int> logicalSize) const
    {
        const auto scale = surfaceScale.getLogicalToSurfaceScale (getBufferMappingMethod());
        const auto convert = [scale] (int extent)
        {
            if (extent <= 0)
                return 0;

            return (int) std::round (jlimit (1.0,
                                             (double) std::numeric_limits<int32_t>::max(),
                                             extent * scale));
        };

        return { convert (logicalSize.x), convert (logicalSize.y) };
    }

    bool isToplevelResizable()
    {
        if ((getStyleFlags() & windowIsResizable) != 0)
            return true;

        // JUCE-drawn resize controls do not request a compositor-drawn resizable frame.
        if (const auto* window = dynamic_cast<const ResizableWindow*> (&getComponent()))
            return window->isResizable();

        return false;
    }

    // xdg_toplevel cannot express an aspect ratio, so constrain each proposed size before replying.
    Point<int> constrainConfiguredSize (Point<int> configuredSize, WaylandResizeAxes axes, bool maximisedOrTiled) const
    {
        auto* boundsConstrainer = getConstrainer();

        if (boundsConstrainer == nullptr || fullScreenState.isFullScreen() || maximisedOrTiled || isKioskMode())
            return configuredSize;

        const auto frame = std::invoke ([&]() -> BorderSize<int>
        {
            if (const auto frameSize = getFrameSizeIfPresent())
                return *frameSize;

            return {};
        });

        return constrainWaylandConfiguredSize (*boundsConstrainer,
                                               frame,
                                               { logicalBounds.getWidth(), logicalBounds.getHeight() },
                                               configuredSize,
                                               axes);
    }

    bool updateToplevelSizeConstraints()
    {
        auto* toplevelRole = getToplevelRole();

        if (toplevelRole == nullptr || toplevelRole->toplevel == nullptr)
            return false;

        const auto constraints = std::invoke ([&]() -> WaylandSizeConstraints
        {
            if (! isToplevelResizable())
            {
                const auto fixedSize = getSurfaceContentSize();
                return { fixedSize, fixedSize };
            }

            if (const auto* ourConstrainer = getConstrainer())
            {
                const auto logical = getWaylandSizeConstraints (*ourConstrainer);
                const auto minimum = getSurfaceConstraintSize (logical.minimum);
                auto maximum = getSurfaceConstraintSize (logical.maximum);

                if (maximum.x != 0)
                    maximum.x = jmax (minimum.x, maximum.x);

                if (maximum.y != 0)
                    maximum.y = jmax (minimum.y, maximum.y);

                return { minimum, maximum };
            }

            return {};
        });

        if (toplevelRole->sizeConstraints.has_value() && *toplevelRole->sizeConstraints == constraints)
            return false;

        toplevelRole->toplevel->setSizeConstraints (constraints);
        toplevelRole->sizeConstraints = constraints;
        return true;
    }

    int getScaleForSurfaceOutputs() const
    {
        auto* windowSystem = WaylandWindowSystem::getInstance();

        return surfaceOutputs.getLargestScale (windowSystem->getLargestOutputScale(),
                                               [windowSystem] (wl_output* output) { return windowSystem->getScaleForOutput (output); });
    }

    void updateOutputScale()
    {
        const auto update = surfaceScale.setOutputScale (getScaleForSurfaceOutputs());
        // Refreshing here avoids cursor-specific change tracking in WaylandSurfaceScale::Update.
        Desktop::getInstance().getMainMouseSource().forceMouseCursorUpdate();
        handleScaleUpdate (update);
    }

    void handleScaleUpdate (const WaylandSurfaceScale::Update& update)
    {
        // Queue a full repaint before notifying listeners because a listener may destroy this peer.
        // This also ensures a full repaint when a scale change leaves the buffer dimensions unchanged.
        if (update.bufferGeometryChanged)
        {
            updateToplevelSizeConstraints();
            repaint (logicalBounds.withZeroOrigin());
        }

        if (update.scaleFactorToReport.has_value())
            scaleFactorListeners.call ([&] (ScaleFactorListener& l) { l.nativeScaleFactorChanged (*update.scaleFactorToReport); });
    }

    void createScaleObjects()
    {
        scaleObjects.reset();

        auto* windowSystem = WaylandWindowSystem::getInstance();
        auto* viewporterGlobal = windowSystem->getViewporter();

        // wp_viewporter also serves host scale overrides when the fractional scale manager is unavailable.
        if (viewporterGlobal == nullptr)
            return;

        ViewportHandle viewport { WaylandProtocol::wpViewporterGetViewport (viewporterGlobal, surface.get()) };

        if (viewport == nullptr)
            return;

        FractionalScaleHandle fractionalScale;
        auto* scaleManager = windowSystem->getFractionalScaleManager();

        if (scaleManager != nullptr)
        {
            fractionalScale.reset (WaylandProtocol::wpFractionalScaleManagerV1GetFractionalScale (scaleManager,
                                                                                                  surface.get()));

            if (fractionalScale != nullptr)
                WaylandProtocol::wpFractionalScaleV1AddListener (fractionalScale.get(),
                                                                 &fractionalScaleListener,
                                                                 callbackState.get());
        }

        scaleObjects.emplace (std::move (viewport), std::move (fractionalScale));
    }

    uint64 nextCommitSequence() const { return lastCommitSequence + 1; }

    void commitSurface() override
    {
        WaylandProtocol::wlSurfaceCommit (surface.get());
        ++lastCommitSequence;

        if (! std::exchange (nextCommitCarriesPosition, false))
            return;

        auto* display = WaylandWindowSystem::getInstance()->getDisplay();
        CommitSync sync { CallbackHandle { WaylandProtocol::wlDisplaySync (display) }, lastCommitSequence };

        if (sync.callback == nullptr)
            return;

        WaylandProtocol::wlCallbackAddListener (sync.callback.get(), &commitSyncListener, callbackState.get());
        commitSyncs.push_back (std::move (sync));
    }

    void handleCommitSyncDone (wl_callback* callback)
    {
        if (commitSyncs.empty() || commitSyncs.front().callback.get() != callback)
        {
            // Check callback ownership and queue assignment if these syncs complete out of order.
            jassertfalse;
            return;
        }

        lastProcessedCommitSequence = commitSyncs.front().commitSequence;
        commitSyncs.erase (commitSyncs.begin());
    }

    bool requestFrameCallback()
    {
        if (frameCallback != nullptr)
            return true;

        frameCallback.reset (WaylandProtocol::wlSurfaceFrame (surface.get()));

        if (frameCallback != nullptr)
            WaylandProtocol::wlCallbackAddListener (frameCallback.get(), &frameListener, callbackState.get());

        return frameCallback != nullptr;
    }

    void requestIdleFrameCallbackForVBlankListeners()
    {
        // A frame callback committed before the first buffer may never fire because the surface
        // is not yet mapped, which would block the initial repaint.
        if (! configured || ! visible || suspended || ! hasCommittedBuffer || surface == nullptr)
            return;

        if (frameCallback != nullptr || vBlankListeners.isEmpty())
            return;

        // A queued repaint requests the frame callback in its own commit.
        if (repainter != nullptr && repainter->hasPendingRepaints())
            return;

        if (! requestFrameCallback())
            return;

        // A frame callback request takes effect on the next commit.
        ++diagnostics.frameCallbackOnlyCommits;
        commitSurface();
        WaylandWindowSystem::getInstance()->flush();
    }

    void vBlankListenerPresenceChanged() override
    {
        requestIdleFrameCallbackForVBlankListeners();
    }

    //==============================================================================
    Point<int> prepareToplevelConfigure (const WaylandToplevel::ConfigureInfo& info) override
    {
        const auto configuredSize = info.contentSize.value_or (Point<int>{});

        ++diagnostics.configuresReceived;
        diagnostics.lastConfigureWidth = configuredSize.x;
        diagnostics.lastConfigureHeight = configuredSize.y;
        diagnostics.firstConfigureReceived = true;
        diagnostics.lastConfigureActivated = info.activated;
        diagnostics.lastConfigureFullScreen = info.fullScreen;
        diagnostics.lastConfigureSuspended = info.suspended;

        fullScreenState.configureReceived (info.fullScreen);
        configured = true;

        // xdg-shell has no unminimise event, so an activated configure means the compositor showed us again.
        if (info.activated)
            wantsMinimised = false;

        const auto wasSuspended = std::exchange (suspended, info.suspended);

        // A suspended surface is not presented, so a pending frame callback may never arrive and
        // would block later repaints. Reset again when leaving suspension in case a repaint
        // committed while suspended requested another callback.
        if (suspended || wasSuspended)
            frameCallback.reset();

        // A JUCE host override makes the pending-size conversion depend on the current output scale.
        configureScaleUpdate = surfaceScale.setOutputScale (getScaleForSurfaceOutputs());

        const auto configuredLogicalSize = std::invoke ([&]() -> std::optional<Point<int>>
        {
            if (! info.contentSize.has_value())
                return std::nullopt;

            const auto mappingMethod = getBufferMappingMethod();
            return Point<int> { surfaceScale.convertSurfaceExtentToLogical (info.contentSize->x, mappingMethod),
                                surfaceScale.convertSurfaceExtentToLogical (info.contentSize->y, mappingMethod) };
        });

        // The resize state can change without a proposed size.
        const auto axes = resizeAxesTracker.update (configuredLogicalSize,
                                                    { logicalBounds.getWidth(), logicalBounds.getHeight() },
                                                    info.resizing);

        // Reuse the original surface size when constraints make no change, avoiding another scale conversion.
        std::optional<Point<int>> acceptedSize;

        if (configuredLogicalSize.has_value())
        {
            const auto constrainedSize = constrainConfiguredSize (*configuredLogicalSize, axes, info.maximisedOrTiled);
            logicalBounds = logicalBounds.withSize (constrainedSize.x, constrainedSize.y);

            if (constrainedSize == *configuredLogicalSize)
                acceptedSize = info.contentSize;
        }

        updateToplevelSizeConstraints();

        return acceptedSize.value_or (getSurfaceContentSize());
    }

    void finishToplevelConfigure() override
    {
        const auto scaleUpdate = std::exchange (configureScaleUpdate, std::nullopt);

        if (scaleUpdate.has_value() && scaleUpdate->bufferGeometryChanged)
        {
            // A scale-factor listener may destroy this peer.
            const WeakReference<WaylandComponentPeer> deletionChecker { this };
            handleScaleUpdate (*scaleUpdate);

            if (deletionChecker == nullptr)
                return;
        }
        else
        {
            repaint (logicalBounds.withZeroOrigin());
        }

        // A resized() override may destroy this peer.
        const WeakReference<WaylandComponentPeer> deletionChecker { this };
        handleMovedOrResized();

        if (deletionChecker == nullptr)
            return;

        // The compositor has asked for a frame at the configured size, so a frame callback from
        // the previous size no longer gates painting. Painting here, as Qt does on configure,
        // commits the new size within the dispatch that delivered it.
        frameCallback.reset();
        performAnyPendingRepaintsNow();
    }

    void toplevelCloseRequested() override
    {
        // User code can destroy the window inside handleUserClosingWindow, which would free
        // the libdecor frame while libdecor is still dispatching the close button's pointer event.
        MessageManager::callAsync ([safeComponent = Component::SafePointer<Component> { &getComponent() }]
        {
            if (safeComponent == nullptr)
                return;

            if (safeComponent->isCurrentlyBlockedByAnotherModalComponent())
                return;

            if (auto* peer = safeComponent->getPeer())
                peer->handleUserClosingWindow();
        });
    }

    void popupConfigured (Rectangle<int> parentRelativeBounds) override
    {
        if (auto* popupRole = getPopupRole())
        {
            configured = true;
            ++diagnostics.popupConfigures;
            logicalBounds = convertWaylandPopupConfigureToLogicalBounds (parentRelativeBounds,
                                                                         popupRole->parent.parentCoordinates);

            repaint (logicalBounds.withZeroOrigin());
            handleMovedOrResized();
        }
    }

    void popupDismissed() override
    {
        ++diagnostics.popupDoneEvents;

        // The compositor sends popup_done to each popup it dismisses, so each peer only dismisses its own component.
        dismissWaylandPopup (component);
    }

    void handleSurfaceEnter (wl_output* output)
    {
        surfaceOutputs.add (output);
        updateOutputScale();
    }

    void handleSurfaceLeave (wl_output* output)
    {
        surfaceOutputs.remove (output);
        updateOutputScale();
    }

    void handlePreferredBufferScale (int32_t scale)
    {
        const auto update = surfaceScale.setPreferredBufferScale (scale);
        // Refreshing here avoids cursor-specific change tracking in WaylandSurfaceScale::Update.
        Desktop::getInstance().getMainMouseSource().forceMouseCursorUpdate();
        handleScaleUpdate (update);
    }

    void handlePreferredFractionalScale (uint32_t scale120)
    {
        diagnostics.lastPreferredScale120 = (int) scale120;
        handleScaleUpdate (surfaceScale.setPreferredFractionalScale120 ((int) scale120));
    }

    void handleActivationTokenDone (const char* token)
    {
        auto* windowSystem = WaylandWindowSystem::getInstance();

        // The compositor may grant focus before it returns the activation token.
        if (auto* activation = windowSystem->getXdgActivation(); activation != nullptr && ! isFocused())
        {
            WaylandProtocol::xdgActivationV1Activate (activation, token, surface.get());
            windowSystem->flush();
        }

        pendingActivationToken.reset();
    }

    void handleFrameDone (wl_callback* callback)
    {
        if (callback != frameCallback.get())
            return;

        frameCallback.reset();
        diagnostics.frameCallbackFired = true;
        ++diagnostics.frameCallbacksReceived;

        // VBlank listeners and component painting can both destroy this peer.
        const WeakReference<WaylandComponentPeer> deletionChecker { this };

        callVBlankListeners (Time::getMillisecondCounterHiRes() * 0.001);

        if (deletionChecker == nullptr)
            return;

        if (repainter != nullptr)
            repainter->performAnyPendingRepaintsNow();

        if (deletionChecker == nullptr)
            return;

        // If nothing committed above, keep the vblank stream going
        requestIdleFrameCallbackForVBlankListeners();
    }

    //==============================================================================
    void requestActivation()
    {
        auto* windowSystem = WaylandWindowSystem::getInstance();
        auto* activation = windowSystem->getXdgActivation();

        if (activation == nullptr)
            return;

        pendingActivationToken.reset (WaylandProtocol::xdgActivationV1GetActivationToken (activation));

        if (pendingActivationToken == nullptr)
            return;

        WaylandProtocol::xdgActivationTokenV1AddListener (pendingActivationToken.get(), &activationTokenListener, callbackState.get());

        const auto inputEvent = windowSystem->getLatestInputSerial();

        if (inputEvent.has_value())
        {
            if (auto* seat = windowSystem->getSeat())
                WaylandProtocol::xdgActivationTokenV1SetSerial (pendingActivationToken.get(), inputEvent->value, seat);

            if (inputEvent->sourceSurface != nullptr)
                WaylandProtocol::xdgActivationTokenV1SetSurface (pendingActivationToken.get(), inputEvent->sourceSurface);
        }

        WaylandProtocol::xdgActivationTokenV1Commit (pendingActivationToken.get());
        windowSystem->flush();
    }

    void keyboardFocusGained() override { handleFocusGain(); }

    void keyboardFocusLost() override   { handleFocusLoss(); }

    wl_surface* getToplevelSurface() const override
    {
        if (getToplevelRole() != nullptr)
            return surface.get();

        if (const auto* popupRole = getPopupRole(); popupRole != nullptr && popupRole->owner.has_value())
            if (auto* toplevel = popupRole->owner->toplevel.get())
                return toplevel->surface.get();

        if (const auto* subsurfaceRole = getSubsurfaceRole())
            if (auto* parent = subsurfaceRole->parent.get())
                return parent->getToplevelSurface();

        return nullptr;
    }

    void modifierKeysChanged() override { handleModifierKeysChange(); }

    void keyStateChanged (bool isDown) override
    {
        const WeakReference<WaylandComponentPeer> deletionChecker { this };
        handleKeyUpOrDown (isDown);

        if (deletionChecker != nullptr)
            refreshTextInputState();
    }

    void keyPressed (int keyCode, juce_wchar character) override
    {
        const WeakReference<WaylandComponentPeer> deletionChecker { this };
        handleKeyPress (keyCode, character);

        if (deletionChecker != nullptr)
            refreshTextInputState();
    }

    bool canReceiveKeyboardFocus() const override
    {
        return visible && (getStyleFlags() & windowIgnoresKeyPresses) == 0;
    }

    wl_surface* getKeyEventFallbackSurface() const override
    {
        auto* toplevelSurface = getToplevelSurface();
        return toplevelSurface != surface.get() ? toplevelSurface : nullptr;
    }

    void pointerMoved (Point<float> position, int64 time) override
    {
        handleMouseEvent (MouseInputSource::InputSourceType::mouse, convertPointerPositionToLogical (position),
                          ModifierKeys::getCurrentModifiers(), MouseInputSource::defaultPressure,
                          MouseInputSource::defaultOrientation, time);
    }

    void pointerEntered() override
    {
        Desktop::getInstance().getMainMouseSource().forceMouseCursorUpdate();
    }

    Point<float> convertPointerPositionToGlobal (Point<float> position) override
    {
        return localToGlobal (convertPointerPositionToLogical (position));
    }

    void pointerButton (Point<float> position, bool pressed, int64 time) override
    {
        if (pressed)
        {
            toFront (true);

            // Application code run by toFront() may destroy this peer.
            if (! isValidPeer (this))
                return;
        }

        handleMouseEvent (MouseInputSource::InputSourceType::mouse, convertPointerPositionToLogical (position),
                          ModifierKeys::getCurrentModifiers(), MouseInputSource::defaultPressure,
                          MouseInputSource::defaultOrientation, time);
    }

    void pointerWheel (Point<float> position, const MouseWheelDetails& wheel, int64 time) override
    {
        handleMouseWheel (MouseInputSource::InputSourceType::mouse, convertPointerPositionToLogical (position), time, wheel);
    }

    void refreshTextInputState()
    {
        WaylandWindowSystem::getInstance()->refreshTextInputState (surface.get());
    }

    wl_surface* getTextInputFocusSurface() const override
    {
        if (const auto* subsurfaceRole = getSubsurfaceRole())
        {
            auto* parent = subsurfaceRole->parent.get();
            return parent != nullptr ? parent->getTextInputFocusSurface() : nullptr;
        }

        return surface.get();
    }

    Rectangle<int> getTextInputCaretRectangle (TextInputTarget& target) const override
    {
        auto* targetComponent = dynamic_cast<Component*> (&target);
        auto* focusPeer = findPeer (getTextInputFocusSurface());

        if (targetComponent == nullptr || focusPeer == nullptr)
            return {};

        const auto local = focusPeer->component.getLocalArea (targetComponent, target.getCaretRectangle());
        const auto scale = focusPeer->surfaceScale.getLogicalToSurfaceScale (focusPeer->getBufferMappingMethod());
        return (local.toDouble() * scale).getSmallestIntegerContainer();
    }

    TextInputTarget* getTextInputTarget() override
    {
        return findCurrentTextInputTarget();
    }

    bool dataDragMoved (const ComponentPeer::DragInfo& info) override
    {
        return handleDragMove (convertDataDragInfo (info));
    }

    void dataDragExited (const ComponentPeer::DragInfo& info) override
    {
        handleDragExit (convertDataDragInfo (info));
    }

    bool dataDropped (const ComponentPeer::DragInfo& info) override
    {
        return handleDragDrop (convertDataDragInfo (info));
    }

    int getPointerCursorScale() const override
    {
        return surfaceScale.getIntegerCompositorScale();
    }

    std::optional<WaylandPopupParentCandidate> getPopupParentCandidate() const override
    {
        if (const auto* subsurfaceRole = getSubsurfaceRole())
        {
            if (subsurfaceRole->subsurface == nullptr)
                return std::nullopt;

            if (auto* parent = subsurfaceRole->parent.get())
                return parent->getPopupParentCandidate();

            return std::nullopt;
        }

        if (auto* popupRole = getPopupRole())
        {
            // A hidden popup has no xdg_surface and its previous serial is no longer valid.
            if (popupRole->popup == nullptr)
                return std::nullopt;

            // A grabbed popup cannot use an ungrabbed popup as its protocol parent.
            if (popupRole->grabPolicy != WaylandPopupGrab::yes)
            {
                if (auto* parentListener = WaylandWindowSystem::getInstance()->findInputListener (popupRole->parent.parentSurface))
                    return parentListener->getPopupParentCandidate();

                return std::nullopt;
            }

            const auto mappedPopup = std::invoke ([&]() -> WaylandPopupParentCandidate::MappedPopup
            {
                if (const auto& grab = popupRole->grab)
                    return { grab->componentToDismiss.get(), grab->serial, popupRole->parent.parentXdgSurface };

                return { nullptr, std::nullopt, popupRole->parent.parentXdgSurface };
            });

            return WaylandPopupParentCandidate { surface.get(),
                                                 popupRole->popup->getXdgSurface(),
                                                 { logicalBounds,
                                                   surfaceScale.getLogicalToSurfaceScale (getBufferMappingMethod()) },
                                                 mappedPopup };
        }

        auto* toplevel = getToplevel();

        if (toplevel == nullptr || toplevel->getXdgSurface() == nullptr)
            return std::nullopt;

        return WaylandPopupParentCandidate { surface.get(),
                                             toplevel->getXdgSurface(),
                                             { toplevel->getPopupParentBounds (logicalBounds),
                                               surfaceScale.getLogicalToSurfaceScale (getBufferMappingMethod()) },
                                             std::nullopt };
    }

    void popupGrabStarted (Component& componentToDismissIn) override
    {
        if (auto* toplevel = getToplevel())
            toplevel->popupGrabStarted (componentToDismissIn);
    }

    void popupGrabEnded() override
    {
        if (auto* toplevel = getToplevel())
            toplevel->popupGrabEnded();
    }

    void dismissPopupForAncestorUnmap() override
    {
        auto* popupRole = getPopupRole();

        if (popupRole == nullptr || popupRole->popup == nullptr)
            return;

        visible = false;
        unmapPopup (*popupRole);
        MessageManager::callAsync ([target = Component::SafePointer<Component> (&component)]
        {
            if (target != nullptr)
                dismissWaylandPopup (*target);
        });
    }

    void touchEvent (int touchIndex, Point<float> position, ModifierKeys mods, int64 time) override
    {
        handleMouseEvent (MouseInputSource::InputSourceType::touch, convertPointerPositionToLogical (position), mods,
                          MouseInputSource::defaultPressure, MouseInputSource::defaultOrientation,
                          time, {}, touchIndex);
    }

    WaylandSurfaceScale::BufferMappingMethod getBufferMappingMethod() const
    {
        return scaleObjects.has_value() ? WaylandSurfaceScale::BufferMappingMethod::viewport
                                        : WaylandSurfaceScale::BufferMappingMethod::integerBufferScale;
    }

    const WaylandComponentPeer* getSubsurfaceParent() const
    {
        if (const auto* subsurfaceRole = getSubsurfaceRole())
            return subsurfaceRole->parent.get();

        return nullptr;
    }

    void recordPositionForCommit (uint64 positionCommit)
    {
        if (const auto* parent = getSubsurfaceParent())
            positionHistory.recordPositionForCommit (positionCommit, logicalBounds.getPosition(),
                                                     parent->lastProcessedCommitSequence);
    }

    Point<float> convertPointerPositionToLogical (Point<float> position) const
    {
        return convertSurfacePointToLogical (position)
             + (getEstimatedSurfacePosition() - logicalBounds.getPosition()).toFloat();
    }

    // A requested subsurface move changes logicalBounds before the parent commits it. Use the
    // position estimated from the parent's commit replies so a window following the pointer
    // does not add its own pending move to the reported pointer position.
    Point<int> getEstimatedSurfacePosition() const
    {
        if (const auto* parent = getSubsurfaceParent())
            return positionHistory.getPositionForCommit (parent->lastProcessedCommitSequence)
                       .value_or (logicalBounds.getPosition());

        return logicalBounds.getPosition();
    }

    Point<float> convertSurfacePointToLogical (Point<float> position) const
    {
        return surfaceScale.convertSurfacePointToLogical (position,
                                                          logicalBounds.withZeroOrigin(),
                                                          getBufferMappingMethod());
    }

    ComponentPeer::DragInfo convertDataDragInfo (const ComponentPeer::DragInfo& info) const
    {
        auto result = info;
        result.position = convertSurfacePointToLogical (info.position.toFloat()).roundToInt();
        return result;
    }

    //==============================================================================
    void outputConfigurationChanged() override
    {
        updateOutputScale();
    }

    void outputWillBeDestroyed (wl_output* output) override
    {
        // A display hot-unplug is not guaranteed to produce wl_surface.leave, so stop tracking its wl_output here.
        surfaceOutputs.remove (output);
    }

    void waylandToplevelMapped (ComponentPeer& peer) override
    {
        if (dialogOwner != nullptr && dialogOwner->getPeer() == &peer)
        {
            if (auto* toplevelRole = getToplevelRole())
                toplevelRole->relationship.invalidateParent();

            updateToplevelRelationship();
        }
    }

    //==============================================================================
    static const wl_surface_listener surfaceListener;
    static const wl_callback_listener frameListener;
    static const wl_callback_listener commitSyncListener;
    static const wp_fractional_scale_v1_listener fractionalScaleListener;
    static const xdg_activation_token_v1_listener activationTokenListener;

    friend ComponentPeer* createWaylandComponentPeer (Component&, int, void*);

    std::unique_ptr<CallbackState> callbackState;

    std::unique_ptr<wl_surface, FunctionPointerDestructor<WaylandProtocol::wlSurfaceDestroy>> surface;
    std::variant<ToplevelRole, PopupRole, SubsurfaceRole> surfaceRole;
    CallbackHandle frameCallback;
    std::unique_ptr<xdg_activation_token_v1, FunctionPointerDestructor<WaylandProtocol::xdgActivationTokenV1Destroy>> pendingActivationToken;
    std::optional<ScaleObjects> scaleObjects;
    AlphaModifierSurfaceHandle alphaModifierSurface;

    Rectangle<int> logicalBounds;
    String windowTitle;
    WaylandFullScreenState fullScreenState;
    std::optional<bool> lastCommittedOpaque;
    float windowAlpha = 1.0f;
    bool configured = false;
    bool visible = false;
    bool suspended = false;
    bool hasCommittedBuffer = false;
    bool wantsMinimised = false;
    std::optional<WaylandSurfaceScale::Update> configureScaleUpdate;
    WaylandResizeAxesTracker resizeAxesTracker;
    WaylandSurfaceScale surfaceScale;
    WaylandSurfaceOutputs surfaceOutputs;
    WeakReference<Component> initialToplevelOwner;
    WeakReference<Component> dialogOwner;

    detail::WaylandPeerDiagnostics diagnostics;
    std::unique_ptr<WaylandRepaintManager> repainter;
    std::set<OpenGLSurface*> openGLSurfaces;
    uint64 lastCommitSequence = 0;
    uint64 lastProcessedCommitSequence = 0;
    bool nextCommitCarriesPosition = false;
    std::vector<CommitSync> commitSyncs;
    WaylandSurfacePositionHistory positionHistory;

    JUCE_DECLARE_WEAK_REFERENCEABLE (WaylandComponentPeer)
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WaylandComponentPeer)
};

const wl_surface_listener WaylandComponentPeer::surfaceListener
{
    [] (void* data, wl_surface*, wl_output* output)
    {
        if (auto peer = getPeerFromCallbackState (data))
            peer->handleSurfaceEnter (output);
    },
    [] (void* data, wl_surface*, wl_output* output)
    {
        if (auto peer = getPeerFromCallbackState (data))
            peer->handleSurfaceLeave (output);
    },
    [] (void* data, wl_surface*, int32_t scale)
    {
        if (auto peer = getPeerFromCallbackState (data))
            peer->handlePreferredBufferScale (scale);
    },
    // Rotated output presentation is not implemented, so ignore preferred_buffer_transform.
    [] (void*, wl_surface*, uint32_t) {}
};

const wl_callback_listener WaylandComponentPeer::frameListener
{
    [] (void* data, wl_callback* callback, uint32_t)
    {
        if (auto peer = getPeerFromCallbackState (data))
            peer->handleFrameDone (callback);
    }
};

const wl_callback_listener WaylandComponentPeer::commitSyncListener
{
    [] (void* data, wl_callback* callback, uint32_t)
    {
        if (auto peer = getPeerFromCallbackState (data))
            peer->handleCommitSyncDone (callback);
    }
};

const wp_fractional_scale_v1_listener WaylandComponentPeer::fractionalScaleListener
{
    [] (void* data, wp_fractional_scale_v1*, uint32_t scale120)
    {
        if (auto peer = getPeerFromCallbackState (data))
            peer->handlePreferredFractionalScale (scale120);
    }
};

const xdg_activation_token_v1_listener WaylandComponentPeer::activationTokenListener
{
    [] (void* data, xdg_activation_token_v1*, const char* token)
    {
        if (auto peer = getPeerFromCallbackState (data))
            peer->handleActivationTokenDone (token);
    }
};

const wl_callback_listener WaylandComponentPeer::OpenGLSurface::frameListener
{
    [] (void* data, wl_callback* callback, uint32_t)
    {
        static_cast<OpenGLSurface*> (data)->handleFrameDone (callback);
    }
};

std::unique_ptr<WaylandOpenGLSurface> WaylandComponentPeer::createOpenGLSurface (Component& target)
{
    return OpenGLSurface::create (*this, target);
}

ComponentPeer* createWaylandComponentPeer (Component& component,
                                           int styleFlags,
                                           void* nativeWindowToAttachTo)
{
    auto* explicitParentSurface = static_cast<wl_surface*> (nativeWindowToAttachTo);
    auto* parentListener = WaylandWindowSystem::getInstance()->findInputListener (explicitParentSurface);
    auto* explicitParent = parentListener != nullptr ? static_cast<WaylandComponentPeer*> (parentListener)
                                                     : nullptr;

    if (explicitParentSurface != nullptr && explicitParent == nullptr)
        return nullptr;

    return new WaylandComponentPeer (component,
                                     styleFlags,
                                     explicitParent);
}

bool isWaylandComponentPeer (const ComponentPeer* peer)
{
    return dynamic_cast<const WaylandComponentPeer*> (peer) != nullptr;
}

//==============================================================================
class WaylandMouseCursor final
{
public:
    explicit WaylandMouseCursor (MouseCursor::StandardCursorType type)
        : cursorInfo (type) {}

    explicit WaylandMouseCursor (const detail::CustomMouseCursorInfo& info)
        : cursorInfo (info) {}

    ~WaylandMouseCursor()
    {
        if (const auto* info = std::get_if<detail::CustomMouseCursorInfo> (&cursorInfo))
            if (auto* windowSystem = WaylandWindowSystem::getInstanceWithoutCreating())
                windowSystem->removeCustomCursorCache (*info);
    }

    static void showInWindow (const WaylandMouseCursor* handle, ComponentPeer& peer)
    {
        if (! isWaylandComponentPeer (&peer))
            return;

        if (auto* windowSystem = WaylandWindowSystem::getInstanceWithoutCreating())
        {
            auto* surface = static_cast<wl_surface*> (peer.getNativeHandle());

            if (handle == nullptr)
                windowSystem->showCursor (surface, MouseCursor::NormalCursor);
            else if (const auto* type = std::get_if<MouseCursor::StandardCursorType> (&handle->cursorInfo))
                windowSystem->showCursor (surface, *type);
            else if (const auto* info = std::get_if<detail::CustomMouseCursorInfo> (&handle->cursorInfo))
                windowSystem->showCursor (surface, info);
        }
    }

private:
    std::variant<MouseCursor::StandardCursorType, detail::CustomMouseCursorInfo> cursorInfo;

    JUCE_DECLARE_NON_COPYABLE (WaylandMouseCursor)
    JUCE_DECLARE_NON_MOVEABLE (WaylandMouseCursor)
};

static bool performWaylandExternalDragDropOfFiles (ComponentPeer& peer, const StringArray& files,
                                                   bool canMoveFiles, std::function<void()> callback)
{
    return WaylandWindowSystem::getInstance()->externalDragFileInit (static_cast<wl_surface*> (peer.getNativeHandle()),
                                                                     files, canMoveFiles, std::move (callback));
}

static bool performWaylandExternalDragDropOfText (ComponentPeer& peer, const String& text, std::function<void()> callback)
{
    return WaylandWindowSystem::getInstance()->externalDragTextInit (static_cast<wl_surface*> (peer.getNativeHandle()),
                                                                     text, std::move (callback));
}

std::unique_ptr<WaylandOpenGLSurface> createWaylandOpenGLSurface (Component& component)
{
    if (auto* peer = dynamic_cast<WaylandComponentPeer*> (component.getPeer()))
        return peer->createOpenGLSurface (component);

    return {};
}

#if JUCE_WAYLAND_PEER_DIAGNOSTICS
std::optional<detail::WaylandPeerDiagnostics> detail::getWaylandPeerDiagnostics (ComponentPeer* peer)
{
    if (auto* waylandPeer = dynamic_cast<WaylandComponentPeer*> (peer))
        return waylandPeer->getDiagnostics();

    return {};
}
#endif

//==============================================================================
//==============================================================================
#if JUCE_UNIT_TESTS

class WaylandOpenGLSurfaceCropTests final : public UnitTest
{
public:
    WaylandOpenGLSurfaceCropTests()
        : UnitTest ("WaylandOpenGLSurfaceCrop", UnitTestCategories::gui) {}

    void runTest() override
    {
        const Rectangle<int> topLevelBounds { 100, 80 };

        testCase ("A surface inside the window keeps its entire buffer visible", [&]
        {
            expect (getOpenGLVisibleBufferBounds ({ 10, 20, 40, 30 }, topLevelBounds, { 80, 60 })
                    == Rectangle<int> (80, 60));
        });

        testCase ("A surface crossing the window's left edge excludes buffer pixels outside the window", [&]
        {
            expect (getOpenGLVisibleBufferBounds ({ -20, 10, 50, 40 }, topLevelBounds, { 100, 80 })
                    == Rectangle<int> (40, 0, 60, 80));
        });

        testCase ("A surface crossing the window's bottom-right corner keeps only the visible buffer pixels", [&]
        {
            expect (getOpenGLVisibleBufferBounds ({ 70, 60, 50, 40 }, topLevelBounds, { 100, 80 })
                    == Rectangle<int> (0, 0, 60, 40));
        });

        testCase ("A surface outside the window has no visible buffer pixels", [&]
        {
            expect (getOpenGLVisibleBufferBounds ({ 110, 10, 50, 40 }, topLevelBounds, { 100, 80 })
                    .isEmpty());
        });

        testCase ("A fractional buffer scale rounds the crop outwards without exceeding the buffer", [&]
        {
            expect (getOpenGLVisibleBufferBounds ({ -15, -7, 33, 21 }, topLevelBounds, { 50, 32 })
                    == Rectangle<int> (22, 10, 28, 22));
        });

        testCase ("A crop stays inside an attached buffer that has not been resized yet", [&]
        {
            expect (getOpenGLVisibleBufferBounds ({ 70, 60, 50, 40 }, topLevelBounds, { 25, 20 })
                    == Rectangle<int> (0, 0, 15, 10));
        });
    }
};

static WaylandOpenGLSurfaceCropTests waylandOpenGLSurfaceCropTests;

//==============================================================================
class WaylandSurfaceRoleTests final : public UnitTest
{
public:
    WaylandSurfaceRoleTests()
        : UnitTest ("WaylandSurfaceRole", UnitTestCategories::gui) {}

    void runTest() override
    {
        constexpr auto temporary = ComponentPeer::windowIsTemporary;
        constexpr auto passive = temporary | ComponentPeer::windowIgnoresMouseClicks;

        testCase ("An explicit parent makes a temporary window a subsurface", [&]
        {
            expect (getWaylandSurfaceRoleKind (temporary, true) == WaylandSurfaceRoleKind::subsurface);
            expect (getWaylandSurfaceRoleKind (passive, true) == WaylandSurfaceRoleKind::subsurface);
        });

        testCase ("Parentless temporary windows keep their popup roles", [&]
        {
            expect (getWaylandSurfaceRoleKind (temporary, false) == WaylandSurfaceRoleKind::popup);
            expect (getWaylandSurfaceRoleKind (passive, false) == WaylandSurfaceRoleKind::popup);
        });

        testCase ("An explicit parent does not turn a regular window into a subsurface", [&]
        {
            expect (getWaylandSurfaceRoleKind (0, true) == WaylandSurfaceRoleKind::toplevel);
        });
    }
};

static WaylandSurfaceRoleTests waylandSurfaceRoleTests;

//==============================================================================
class WaylandSizeConstraintsTests final : public UnitTest
{
public:
    WaylandSizeConstraintsTests()
        : UnitTest ("WaylandSizeConstraints", UnitTestCategories::gui) {}

    void runTest() override
    {
        testCase ("A bordered constrainer adds its border to wrapped size limits", [&]
        {
            ComponentBoundsConstrainer wrapped;
            wrapped.setSizeLimits (400, 200, 1024, 700);

            const BorderedConstrainer bordered { wrapped, { 10, 20, 30, 40 } };
            const auto constraints = getWaylandSizeConstraints (bordered);

            expect (constraints.minimum == Point<int> { 460, 240 });
            expect (constraints.maximum == Point<int> { 1084, 740 });
        });
    }

private:
    class BorderedConstrainer final : public BorderedComponentBoundsConstrainer
    {
    public:
        BorderedConstrainer (ComponentBoundsConstrainer& wrappedIn, BorderSize<int> borderIn)
            : wrapped (wrappedIn), border (borderIn) {}

        ComponentBoundsConstrainer* getWrappedConstrainer() const override { return &wrapped; }
        BorderSize<int> getAdditionalBorder() const override               { return border; }

    private:
        ComponentBoundsConstrainer& wrapped;
        BorderSize<int> border;
    };
};

static WaylandSizeConstraintsTests waylandSizeConstraintsTests;

//==============================================================================
class WaylandResizeAxesTests final : public UnitTest
{
public:
    WaylandResizeAxesTests()
        : UnitTest ("WaylandResizeAxesTracker", UnitTestCategories::gui) {}

    void runTest() override
    {
        const Point<int> size { 400, 300 };
        const WaylandResizeAxes none;
        const WaylandResizeAxes vertical { true, false };
        const WaylandResizeAxes horizontal { false, true };
        const WaylandResizeAxes both { true, true };

        const std::optional<bool> noResizeState;
        const Point<int> wider { 450, 300 };
        const Point<int> afterFirstDrag { 450, 338 };

        testCase ("The first size change after a current-size configure determines the resize axes", [&]
        {
            WaylandResizeAxesTracker tracker;
            expect (tracker.update (size, size, noResizeState) == none);
            expect (tracker.update (wider, size, noResizeState) == horizontal);
        });

        testCase ("Changes to height alone select the vertical axis and changes to both dimensions select both axes", [&]
        {
            WaylandResizeAxesTracker verticalDrag;
            verticalDrag.update (size, size, noResizeState);
            expect (verticalDrag.update (Point<int> { 400, 250 }, size, noResizeState) == vertical);

            WaylandResizeAxesTracker cornerDrag;
            cornerDrag.update (size, size, noResizeState);
            expect (cornerDrag.update (Point<int> { 450, 250 }, size, noResizeState) == both);
        });

        testCase ("A one-pixel width change is ignored when the height changes substantially", [&]
        {
            WaylandResizeAxesTracker tracker;
            tracker.update (size, size, noResizeState);
            expect (tracker.update (Point<int> { 401, 250 }, size, noResizeState) == vertical);
        });

        testCase ("Later size changes do not replace the axes selected at the start of a resize", [&]
        {
            WaylandResizeAxesTracker tracker;
            tracker.update (size, size, noResizeState);
            tracker.update (wider, size, noResizeState);
            expect (tracker.update (Point<int> { 450, 301 }, afterFirstDrag, noResizeState) == horizontal);
            expect (tracker.update (Point<int> { 460, 299 }, afterFirstDrag, noResizeState) == horizontal);
        });

        testCase ("Repeating a proposed size preserves the selected axes", [&]
        {
            WaylandResizeAxesTracker tracker;
            tracker.update (size, size, noResizeState);
            tracker.update (wider, size, noResizeState);
            expect (tracker.update (wider, afterFirstDrag, noResizeState) == horizontal);
        });

        testCase ("A current-size configure clears the axes before the next resize", [&]
        {
            WaylandResizeAxesTracker tracker;
            tracker.update (size, size, noResizeState);
            tracker.update (wider, size, noResizeState);
            expect (tracker.update (afterFirstDrag, afterFirstDrag, noResizeState) == none);
            expect (tracker.update (Point<int> { 450, 400 }, afterFirstDrag, noResizeState) == vertical);
        });

        testCase ("A false resizing state clears the axes before the next resize", [&]
        {
            WaylandResizeAxesTracker tracker;
            tracker.update (size, size, true);
            tracker.update (wider, size, true);
            expect (tracker.update (Point<int> { 452, 340 }, afterFirstDrag, false) == none);
            expect (tracker.update (Point<int> { 452, 400 }, { 452, 340 }, true) == vertical);
        });

        testCase ("A false resizing state clears the axes even when no size is proposed", [&]
        {
            WaylandResizeAxesTracker tracker;
            tracker.update (size, size, true);
            tracker.update (wider, size, true);
            expect (tracker.update (std::nullopt, afterFirstDrag, false) == none);
            expect (tracker.update (Point<int> { 450, 400 }, afterFirstDrag, true) == vertical);
        });

        testCase ("A configure without a size preserves the axes when the resizing state is unavailable", [&]
        {
            WaylandResizeAxesTracker tracker;
            tracker.update (size, size, noResizeState);
            tracker.update (wider, size, noResizeState);
            expect (tracker.update (std::nullopt, afterFirstDrag, noResizeState) == none);
            expect (tracker.update (Point<int> { 460, 301 }, afterFirstDrag, noResizeState) == horizontal);
        });
    }
};

static WaylandResizeAxesTests waylandResizeAxesTests;

//==============================================================================
class WaylandConfiguredSizeTests final : public UnitTest
{
public:
    WaylandConfiguredSizeTests()
        : UnitTest ("constrainWaylandConfiguredSize", UnitTestCategories::gui) {}

    void runTest() override
    {
        constexpr auto aspectRatio = 2.0;

        ComponentBoundsConstrainer constrainer;
        constrainer.setSizeLimits (200, 100, 2000, 2000);
        constrainer.setFixedAspectRatio (aspectRatio);

        const Point<int> previousSize { 400, 200 };
        const WaylandResizeAxes vertical { true, false };
        const WaylandResizeAxes horizontal { false, true };

        testCase ("A horizontal resize preserves the proposed width and derives the height from the aspect ratio", [&]
        {
            expect (constrainWaylandConfiguredSize (constrainer, {}, previousSize, { 500, 200 }, horizontal)
                    == Point<int> { 500, 250 });
        });

        testCase ("A vertical resize preserves the proposed height and derives the width from the aspect ratio", [&]
        {
            expect (constrainWaylandConfiguredSize (constrainer, {}, previousSize, { 400, 150 }, vertical)
                    == Point<int> { 300, 150 });
        });

        testCase ("Repeating a proposed size produces the same constrained size", [&]
        {
            const auto first = constrainWaylandConfiguredSize (constrainer, {}, previousSize, { 500, 200 }, horizontal);
            const auto repeated = constrainWaylandConfiguredSize (constrainer, {}, first, { 500, 200 }, horizontal);
            expect (repeated == first);
        });

        testCase ("The native frame is included while constraining and removed from the returned content size", [&]
        {
            const BorderSize<int> titleBar { 30, 0, 0, 0 };
            const Point<int> proposedContentSize { 500, 200 };
            const auto expectedFramedHeight = roundToInt (proposedContentSize.x / aspectRatio);
            const Point<int> expectedContentSize { proposedContentSize.x,
                                                   expectedFramedHeight - titleBar.getTop() };

            expect (constrainWaylandConfiguredSize (constrainer, titleBar, previousSize, proposedContentSize, horizontal)
                    == expectedContentSize);
        });
    }
};

static WaylandConfiguredSizeTests waylandConfiguredSizeTests;

#endif

} // namespace juce
