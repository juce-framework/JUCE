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

void WaylandWindowSystem::DisconnectDisplay::operator() (wl_display* ptr) const
{
    if (ptr != nullptr)
        WaylandClientSymbols::getInstance()->wl_display_disconnect (ptr);
}

namespace
{
   #if JUCE_DEBUG && ! JUCE_DISABLE_ASSERTIONS
    void logConnectionError (int errorCode, wl_display* display)
    {
        if (errorCode == EPROTO)
        {
            const wl_interface* interface = nullptr;
            uint32_t objectId = 0;
            const auto protocolError = WaylandClientSymbols::getInstance()->wl_display_get_protocol_error (display, &interface, &objectId);
            const auto* interfaceName = interface != nullptr && interface->name != nullptr
                                      ? interface->name
                                      : "unknown interface";

            DBG ("ERROR: Wayland protocol error " << (uint64) protocolError
                 << " on " << interfaceName << " object " << (uint64) objectId);
            return;
        }

        DBG ("ERROR: Wayland connection lost: " << strerror (errorCode) << " (" << errorCode << ")");
    }
   #endif

    String getEnv (const char* name)
    {
        return SystemStats::getEnvironmentVariable (name, {});
    }

    bool rotationSwapsWidthAndHeight (uint32_t transform)
    {
        return transform == WaylandProtocol::wlOutputTransform90
            || transform == WaylandProtocol::wlOutputTransform270
            || transform == WaylandProtocol::wlOutputTransformFlipped90
            || transform == WaylandProtocol::wlOutputTransformFlipped270;
    }

    struct DisplayGeometry
    {
        Rectangle<int> physicalBounds;
        Rectangle<float> logicalBounds;
        double scale = 1.0;
    };

    DisplayGeometry getDisplayGeometry (const WaylandWindowSystem::BoundOutput::State& state, float masterScale)
    {
        const auto physicalSize = rotationSwapsWidthAndHeight (state.transform)
                                ? Point<int> { state.modeSize.y, state.modeSize.x }
                                : state.modeSize;

        const auto hasLogicalGeometry = state.logicalPosition.has_value()
                                     && state.logicalSize.has_value()
                                     && state.logicalSize->x > 0
                                     && state.logicalSize->y > 0;

        const auto compositorLogicalBounds = std::invoke ([&]
        {
            if (hasLogicalGeometry)
                return Rectangle<double> { (double) state.logicalPosition->x, (double) state.logicalPosition->y,
                                           (double) state.logicalSize->x, (double) state.logicalSize->y };

            const auto fallbackScale = (double) jmax (1, state.integerScaleFactor);
            return Rectangle<double> { (double) state.fallbackLogicalPosition.x,
                                       (double) state.fallbackLogicalPosition.y,
                                       physicalSize.x / fallbackScale,
                                       physicalSize.y / fallbackScale };
        });

        const auto outputScale = std::invoke ([&]
        {
            if (! hasLogicalGeometry)
                return (double) jmax (1, state.integerScaleFactor);

            const auto horizontalScale = physicalSize.x / (double) state.logicalSize->x;
            const auto verticalScale   = physicalSize.y / (double) state.logicalSize->y;
            return (horizontalScale + verticalScale) * 0.5;
        });

        // Wayland does not provide a global hardware-pixel position. This value lets JUCE
        // convert points within the display, while logicalBounds remains the layout source.
        const auto physicalPosition = (compositorLogicalBounds.getPosition() * outputScale).roundToInt();

        return { { physicalPosition.x, physicalPosition.y, physicalSize.x, physicalSize.y },
                 (compositorLogicalBounds / masterScale).toFloat(),
                 masterScale * outputScale };
    }

    double getDpi (Point<int> modeSize, int physicalWidthMm, int physicalHeightMm)
    {
        // wl_output may report zero physical dimensions, so match the X11 backend's 96 DPI fallback.
        if (physicalWidthMm <= 0 || physicalHeightMm <= 0)
            return 96.0;

        const auto horizontalDpi = (double) modeSize.x * 25.4 / (double) physicalWidthMm;
        const auto verticalDpi   = (double) modeSize.y * 25.4 / (double) physicalHeightMm;

        return (horizontalDpi + verticalDpi) * 0.5;
    }

    const wl_registry_listener registryListener
    {
        [] (void* data, wl_registry* registry, uint32_t name, const char* interface, uint32_t version)
        {
            static_cast<WaylandWindowSystem*> (data)->handleRegistryGlobal (registry, name, interface, version);
        },
        [] (void* data, wl_registry*, uint32_t name)
        {
            static_cast<WaylandWindowSystem*> (data)->handleRegistryGlobalRemove (name);
        }
    };

    const wl_output_listener outputListener
    {
        [] (void* data, wl_output*, int32_t x, int32_t y, int32_t physicalWidth, int32_t physicalHeight,
            int32_t, const char*, const char*, int32_t transform)
        {
            static_cast<WaylandWindowSystem::BoundOutput*> (data)->handleGeometry (x, y, physicalWidth, physicalHeight, (uint32_t) transform);
        },
        [] (void* data, wl_output*, uint32_t flags, int32_t width, int32_t height, int32_t refresh)
        {
            static_cast<WaylandWindowSystem::BoundOutput*> (data)->handleMode (flags, width, height, refresh);
        },
        [] (void* data, wl_output*)
        {
            static_cast<WaylandWindowSystem::BoundOutput*> (data)->handleOutputDone();
        },
        [] (void* data, wl_output*, int32_t scale)
        {
            static_cast<WaylandWindowSystem::BoundOutput*> (data)->handleScale (scale);
        },
        [] (void* data, wl_output*, const char* name)
        {
            static_cast<WaylandWindowSystem::BoundOutput*> (data)->handleName (name);
        },
        [] (void* data, wl_output*, const char* description)
        {
            static_cast<WaylandWindowSystem::BoundOutput*> (data)->handleDescription (description);
        }
    };

    const zxdg_output_v1_listener xdgOutputListener
    {
        [] (void* data, zxdg_output_v1*, int32_t x, int32_t y)
        {
            static_cast<WaylandWindowSystem::BoundOutput*> (data)->handleLogicalPosition (x, y);
        },
        [] (void* data, zxdg_output_v1*, int32_t width, int32_t height)
        {
            static_cast<WaylandWindowSystem::BoundOutput*> (data)->handleLogicalSize (width, height);
        },
        [] (void* data, zxdg_output_v1*)
        {
            static_cast<WaylandWindowSystem::BoundOutput*> (data)->handleXdgOutputDone();
        },
        [] (void* data, zxdg_output_v1*, const char* name)
        {
            static_cast<WaylandWindowSystem::BoundOutput*> (data)->handleName (name);
        },
        [] (void* data, zxdg_output_v1*, const char* description)
        {
            static_cast<WaylandWindowSystem::BoundOutput*> (data)->handleDescription (description);
        }
    };

    const xdg_wm_base_listener wmBaseListener
    {
        [] (void*, xdg_wm_base* wmBase, uint32_t serial)
        {
            WaylandProtocol::xdgWmBasePong (wmBase, serial);
        }
    };

    const wl_seat_listener seatListener
    {
        [] (void* data, wl_seat*, uint32_t capabilities)
        {
            static_cast<WaylandWindowSystem*> (data)->handleSeatCapabilities (capabilities);
        },
        [] (void* data, wl_seat*, const char* name)
        {
            static_cast<WaylandWindowSystem*> (data)->handleSeatName (name);
        }
    };

    const zxdg_exported_v2_listener exportedListener
    {
        [] (void* data, zxdg_exported_v2*, const char* handle)
        {
            static_cast<WaylandWindowSystem::ExportedSurfaceHandle*> (data)->handle = String (CharPointer_UTF8 (handle));
        }
    };
}

//==============================================================================
auto WaylandWindowSystem::BoundOutput::State::tie() const
{
    return std::tie (fallbackLogicalPosition, modeSize, logicalPosition, logicalSize, physicalWidthMm,
                     physicalHeightMm, refreshRateMilliHz, integerScaleFactor, transform, name, description);
}

bool WaylandWindowSystem::BoundOutput::State::operator== (const State& other) const   { return tie() == other.tie(); }
bool WaylandWindowSystem::BoundOutput::State::operator!= (const State& other) const   { return tie() != other.tie(); }

WaylandWindowSystem::BoundOutput::BoundOutput (WaylandWindowSystem& windowSystemIn, uint32_t registryNameIn,
                                               Handle handleIn)
    : windowSystem (windowSystemIn),
      registryName (registryNameIn),
      handle (std::move (handleIn)),
      waitsForOutputDoneEvent (2 <= WaylandProtocol::getOutputVersion (handle.get()))
{
}

void WaylandWindowSystem::BoundOutput::bindXdgOutput (zxdg_output_manager_v1* manager)
{
    if (manager == nullptr || xdgOutput != nullptr)
        return;

    xdgOutput.reset (WaylandProtocol::zxdgOutputManagerV1GetXdgOutput (manager, handle.get()));

    if (xdgOutput != nullptr)
        WaylandProtocol::zxdgOutputV1AddListener (xdgOutput.get(), &xdgOutputListener, this);
}

void WaylandWindowSystem::BoundOutput::handleGeometry (int32_t x, int32_t y, int32_t physicalWidth,
                                                       int32_t physicalHeight, uint32_t transform)
{
    pendingState.fallbackLogicalPosition = { x, y };
    pendingState.physicalWidthMm = physicalWidth;
    pendingState.physicalHeightMm = physicalHeight;
    pendingState.transform = transform;

    commitAfterEventIfRequired();
}

void WaylandWindowSystem::BoundOutput::handleMode (uint32_t flags, int32_t width, int32_t height, int32_t refresh)
{
    if ((flags & WaylandProtocol::wlOutputModeCurrent) == 0)
        return;

    pendingState.modeSize = { width, height };
    pendingState.refreshRateMilliHz = refresh;

    commitAfterEventIfRequired();
}

void WaylandWindowSystem::BoundOutput::handleScale (int32_t scale)
{
    pendingState.integerScaleFactor = jmax (1, scale);

    commitAfterEventIfRequired();
}

void WaylandWindowSystem::BoundOutput::handleOutputDone()
{
    commitPendingState();
}

void WaylandWindowSystem::BoundOutput::handleLogicalPosition (int32_t x, int32_t y)
{
    pendingState.logicalPosition = Point<int> { x, y };
}

void WaylandWindowSystem::BoundOutput::handleLogicalSize (int32_t width, int32_t height)
{
    pendingState.logicalSize = Point<int> { width, height };
}

void WaylandWindowSystem::BoundOutput::handleXdgOutputDone()
{
    // Version 3 uses wl_output.done so both protocols become current together.
    if (xdgOutput != nullptr && WaylandProtocol::getXdgOutputVersion (xdgOutput.get()) < 3)
        commitPendingState();
}

void WaylandWindowSystem::BoundOutput::handleName (const char* name)
{
    pendingState.name = String { CharPointer_UTF8 (name != nullptr ? name : "") };
}

void WaylandWindowSystem::BoundOutput::handleDescription (const char* description)
{
    pendingState.description = String { CharPointer_UTF8 (description != nullptr ? description : "") };
}

void WaylandWindowSystem::BoundOutput::commitPendingState()
{
    if (currentState == pendingState)
        return;

    currentState = pendingState;
    windowSystem.triggerAsyncUpdate();
}

void WaylandWindowSystem::BoundOutput::commitAfterEventIfRequired()
{
    if (! waitsForOutputDoneEvent)
        commitPendingState();
}

//==============================================================================
WaylandWindowSystem::WaylandWindowSystem() = default;

WaylandWindowSystem::~WaylandWindowSystem()
{
    disconnect();
    LibdecorSymbols::deleteInstance();
    WaylandClientSymbols::deleteInstance();
    clearSingletonInstance();
}

bool WaylandWindowSystem::shouldUseWaylandBackend()
{
    // All plugin windows stay on X11 until secondary windows inherit their owner's backend.
    if (! JUCEApplicationBase::isStandaloneApp())
        return false;

    return getEnv ("WAYLAND_DISPLAY").isNotEmpty();
}

bool WaylandWindowSystem::isWaylandAvailable()
{
    if (! attemptedConnection)
    {
        attemptedConnection = true;
        available = connect();

        if (! available)
            disconnect();
    }

    return available;
}

bool WaylandWindowSystem::connect()
{
    if (! shouldUseWaylandBackend())
        return false;

    connectionLost = false;

    auto* symbols = WaylandClientSymbols::getInstance();

    if (! symbols->loadAllSymbols())
        return false;

    WaylandProtocol::initialiseInterfaces (*symbols);

    display.reset (symbols->wl_display_connect (nullptr));

    if (display == nullptr)
        return false;

    registry.reset (WaylandProtocol::wlDisplayGetRegistry (display.get()));

    if (registry == nullptr)
        return false;

    inputHandler = std::make_unique<WaylandInputHandler> (static_cast<WaylandInputHandlerDelegate&> (*this));
    dataDevice = std::make_unique<WaylandDataDevice> (static_cast<WaylandDataDeviceDelegate&> (*this));
    textInput = std::make_unique<WaylandTextInput> (static_cast<WaylandTextInputDelegate&> (*this));
    WaylandProtocol::wlRegistryAddListener (registry.get(), &registryListener, this);

    // The first roundtrip delivers the registry globals. The second delivers the events
    // emitted by binding to them (e.g. wl_output geometry/mode/scale).
    if (symbols->wl_display_roundtrip (display.get()) < 0
        || symbols->wl_display_roundtrip (display.get()) < 0)
    {
        return false;
    }

    registryGlobalsBound = compositor != nullptr && shm != nullptr && xdgWmBase != nullptr;

    if (! registryGlobalsBound)
        return false;

    // Decline Wayland when shared-memory buffers cannot be created. Otherwise windows
    // render blank and peer creation cannot fall back to X11.
    if (! WaylandShmBuffer::canCreate())
        return false;

    // Use libdecor for native title bars when the compositor does not provide decorations.
    if (decorationManager == nullptr && LibdecorSymbols::getInstance()->loadAllSymbols())
    {
        libdecorErrorReceived = false;
        libdecorContext = LibdecorAPI::createContext (display.get(), &libdecorInterface);

        if (libdecorContext != nullptr)
        {
            if (symbols->wl_display_roundtrip (display.get()) < 0)
                return false;

            // A context that failed during its first roundtrip is dropped, and decorated
            // windows come up frameless instead.
            if (libdecorErrorReceived)
                libdecorContext.reset();
        }
    }

    cursor.emplace (*compositor, *shm);

    fd = libdecorContext != nullptr ? LibdecorAPI::getFd (libdecorContext.get())
                                    : symbols->wl_display_get_fd (display.get());

    if (fd < 0)
        return false;

    registerEventFd();
    flush();

    return true;
}

void WaylandWindowSystem::disconnect()
{
    unregisterEventFd();
    cancelPendingUpdate();

    if (dataDevice != nullptr)
        dataDevice->unbind();

    if (textInput != nullptr)
        textInput->unbind();

    // Unbind input devices while inputHandler is still set: the focus-loss and touch-release
    // callbacks can destroy peers, and removeInputListener must still reach the handler.
    // Then destroy it so its wl_pointer/wl_keyboard/wl_touch proxies go before the seat.
    if (inputHandler != nullptr)
        inputHandler->unbindAllDevices();

    inputHandler.reset();
    textInput.reset();
    textInputManager.reset();
    dataDevice.reset();
    cursor.reset();
    libdecorContext.reset();

    xdgActivation.reset();
    alphaModifier.reset();
    fractionalScaleManager.reset();
    viewporter.reset();
    exporter.reset();
    decorationManager.reset();
    xdgWmBase.reset();
    dataDeviceManager.reset();
    seat.reset();

    // Peers may still refer to outputs containing their surfaces, so notify them before destruction.
    for (const auto& output : outputs)
        notifyOutputWillBeDestroyed (*output);

    outputs.clear();
    xdgOutputManager.reset();
    shm.reset();
    subcompositor.reset();
    compositor.reset();
    registry.reset();
    display.reset();

    registryGlobalsBound = false;
    seatBound = false;
    alphaModifierRegistryName.reset();
    xdgActivationRegistryName.reset();
    seatName.clear();
    mainOutputName.clear();
    available = false;
    connectionLost = false;

    // The libdecor symbol table stays unusable on purpose, since nothing reconnects after this point.
    libdecorErrorReceived = false;
}

void WaylandWindowSystem::registerEventFd()
{
    if (fdRegistered || fd < 0)
        return;

    LinuxEventLoop::registerFdCallback (fd,
                                        [this] (int)
                                        {
                                            dispatchPendingEvents();
                                        });
    fdRegistered = true;
}

void WaylandWindowSystem::unregisterEventFd()
{
    if (! fdRegistered || fd < 0)
        return;

    LinuxEventLoop::unregisterFdCallback (fd);
    fdRegistered = false;
}

void WaylandWindowSystem::dispatchPendingEvents()
{
    if (display == nullptr || connectionLost)
        return;

    auto* symbols = WaylandClientSymbols::getInstance();
    auto* displayPtr = display.get();

    const auto connectionFailedAfter = [this, symbols, displayPtr] (int result)
    {
        if (result >= 0)
            return false;

        // Display operations may fail for recoverable reasons such as EAGAIN.
        // A nonzero display error is fatal and means the display can no longer be used.
        // See wl_display_get_error: https://wayland.freedesktop.org/docs/html/apb.html
        const auto errorCode = symbols->wl_display_get_error (displayPtr);

        if (errorCode == 0)
            return false;

       #if JUCE_DEBUG && ! JUCE_DISABLE_ASSERTIONS
        logConnectionError (errorCode, displayPtr);
       #endif

        handleConnectionLost();
        return true;
    };

    const ScopeGuard scope { [&] { loopListeners.call ([&] (auto& l) { l.waylandDidDispatchPendingEvents(); }); } };

    // libdecor's dispatch reads the display itself and pumps whatever its decoration plugin needs.
    if (libdecorContext != nullptr)
    {
        if (connectionFailedAfter (LibdecorAPI::dispatch (libdecorContext.get(), 0)))
            return;

        // A libdecor error results in no decorations. Later windows use plain xdg-shell,
        // and the wl_display calls below take over on the registered fd, which stays open because a
        // failed context is leaked rather than destroyed.
        if (libdecorErrorReceived)
        {
            libdecorContext.reset();
            DBG ("Wayland: libdecor stopped working, new windows will be undecorated");
        }
        else
        {
            // libdecor's dispatch might have invoked callbacks that queue new Wayland requests.
            flush();
            return;
        }
    }

    if (connectionFailedAfter (symbols->wl_display_dispatch_pending (displayPtr)))
        return;

    for (;;)
    {
        const auto prepareResult = symbols->wl_display_prepare_read (displayPtr);

        if (prepareResult == 0)
            break;

        if (connectionFailedAfter (prepareResult))
            return;

        if (connectionFailedAfter (symbols->wl_display_dispatch_pending (displayPtr)))
            return;
    }

    // read_events consumes the read intention even on failure, so no cancel_read here.
    const auto readResult = symbols->wl_display_read_events (displayPtr);

    if (connectionFailedAfter (readResult))
        return;

    if (readResult >= 0 && connectionFailedAfter (symbols->wl_display_dispatch_pending (displayPtr)))
        return;

    connectionFailedAfter (symbols->wl_display_flush (displayPtr));
}

void WaylandWindowSystem::handleConnectionLost()
{
    connectionLost = true;
    unregisterEventFd();

    if (JUCEApplicationBase::isStandaloneApp())
        MessageManager::getInstance()->stopDispatchLoop();
}

libdecor_interface WaylandWindowSystem::libdecorInterface
{
    &handleLibdecorError,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr
};

void WaylandWindowSystem::handleLibdecorError (libdecor*, [[maybe_unused]] libdecor_error error,
                                               [[maybe_unused]] const char* message)
{
    // Both libdecor errors are fatal.
    LibdecorAPI::markUnusableAfterError();

    if (auto* instance = getInstanceWithoutCreating())
        instance->libdecorErrorReceived = true;

   #if JUCE_DEBUG
    const auto* errorName = std::invoke ([&]() -> const char*
    {
        switch (error)
        {
            case libdecorErrorCompositorIncompatible:     return "compositor incompatible";
            case libdecorErrorInvalidFrameConfiguration:  return "invalid frame configuration";
        }

        return "unrecognised error";
    });

    DBG ("ERROR: libdecor: " << errorName << ": "
         << String { CharPointer_UTF8 (message != nullptr ? message : "unknown error") });
   #endif
}

void WaylandWindowSystem::flush()
{
    if (display != nullptr && ! connectionLost)
        WaylandClientSymbols::getInstance()->wl_display_flush (display.get());
}

void WaylandWindowSystem::handleRegistryGlobal (wl_registry* registryIn, uint32_t name, const char* interface, uint32_t version)
{
    const String interfaceName { CharPointer_UTF8 (interface) };

    if (interfaceName == "wl_compositor" && compositor == nullptr)
    {
        compositor.reset (WaylandProtocol::bindCompositor (registryIn, name, version));
    }
    else if (interfaceName == "wl_subcompositor" && subcompositor == nullptr)
    {
        subcompositor.reset (WaylandProtocol::bindSubcompositor (registryIn, name, version));
    }
    else if (interfaceName == "wl_shm" && shm == nullptr)
    {
        shm.reset (WaylandProtocol::bindShm (registryIn, name, version));
    }
    else if (interfaceName == "wl_output")
    {
        // Bind every output, including displays connected after the initial registry scan.
        if (auto* bound = WaylandProtocol::bindOutput (registryIn, name, version))
        {
            auto& output = *outputs.emplace_back (std::make_unique<BoundOutput> (*this, name,
                                                                                 BoundOutput::Handle { bound }));
            WaylandProtocol::wlOutputAddListener (output.handle.get(), &outputListener, &output);
            output.bindXdgOutput (xdgOutputManager.get());
        }
    }
    else if (interfaceName == "zxdg_output_manager_v1" && xdgOutputManager == nullptr)
    {
        xdgOutputManager.reset (WaylandProtocol::bindZxdgOutputManagerV1 (registryIn, name, version));

        for (const auto& output : outputs)
            output->bindXdgOutput (xdgOutputManager.get());
    }
    else if (interfaceName == "wl_seat" && seat == nullptr)
    {
        seat.reset (WaylandProtocol::bindSeat (registryIn, name, version));

        if (seat != nullptr)
        {
            seatBound = true;
            WaylandProtocol::wlSeatAddListener (seat.get(), &seatListener, this);
            ensureDataDeviceBound();
            ensureTextInputBound();
        }
    }
    else if (interfaceName == "zwp_text_input_manager_v3" && textInputManager == nullptr)
    {
        textInputManager.reset (WaylandProtocol::bindTextInputManagerV3 (registryIn, name, version));
        ensureTextInputBound();
    }
    else if (interfaceName == "wl_data_device_manager" && dataDeviceManager == nullptr)
    {
        dataDeviceManager.reset (WaylandProtocol::bindDataDeviceManager (registryIn, name, version));
        ensureDataDeviceBound();
    }
    else if (interfaceName == "xdg_wm_base" && xdgWmBase == nullptr)
    {
        xdgWmBase.reset (WaylandProtocol::bindXdgWmBase (registryIn, name, version));

        if (xdgWmBase != nullptr)
            WaylandProtocol::xdgWmBaseAddListener (xdgWmBase.get(), &wmBaseListener, this);
    }
    else if (interfaceName == "zxdg_decoration_manager_v1" && decorationManager == nullptr)
    {
        decorationManager.reset (WaylandProtocol::bindZxdgDecorationManagerV1 (registryIn, name, version));
    }
    else if (interfaceName == "zxdg_exporter_v2" && exporter == nullptr)
    {
        exporter.reset (WaylandProtocol::bindZxdgExporterV2 (registryIn, name, version));
    }
    else if (interfaceName == "wp_viewporter" && viewporter == nullptr)
    {
        viewporter.reset (WaylandProtocol::bindWpViewporter (registryIn, name, version));
    }
    else if (interfaceName == "wp_fractional_scale_manager_v1" && fractionalScaleManager == nullptr)
    {
        fractionalScaleManager.reset (WaylandProtocol::bindWpFractionalScaleManagerV1 (registryIn, name, version));
    }
    else if (interfaceName == "wp_alpha_modifier_v1" && alphaModifier == nullptr)
    {
        alphaModifier.reset (WaylandProtocol::bindWpAlphaModifierV1 (registryIn, name, version));

        if (alphaModifier != nullptr)
            alphaModifierRegistryName = name;
    }
    else if (interfaceName == "xdg_activation_v1" && xdgActivation == nullptr)
    {
        xdgActivation.reset (WaylandProtocol::bindXdgActivationV1 (registryIn, name, version));

        if (xdgActivation != nullptr)
            xdgActivationRegistryName = name;
    }
}

void WaylandWindowSystem::handleRegistryGlobalRemove (uint32_t name)
{
    // Requests sent to a removed global are ignored, so stop using this handle and allow rebinding.
    if (alphaModifierRegistryName == name)
    {
        alphaModifier.reset();
        alphaModifierRegistryName.reset();
        return;
    }

    if (xdgActivationRegistryName == name)
    {
        xdgActivation.reset();
        xdgActivationRegistryName.reset();
        return;
    }

    const auto matchesName = [name] (const std::unique_ptr<BoundOutput>& output) { return output->registryName == name; };
    const auto it = std::find_if (outputs.begin(), outputs.end(), matchesName);

    // Outputs and optional globals can be removed and rebound.
    // Removal notifications for other globals are ignored.
    if (it == outputs.end())
        return;

    notifyOutputWillBeDestroyed (**it);
    outputs.erase (it);
    triggerAsyncUpdate();
}

void WaylandWindowSystem::notifyOutputWillBeDestroyed (const BoundOutput& output)
{
    auto* outputHandle = output.handle.get();
    outputListeners.call ([outputHandle] (WaylandOutputListener& l) { l.outputWillBeDestroyed (outputHandle); });
}

void WaylandWindowSystem::toplevelMapped (ComponentPeer& peer)
{
    toplevelMappingListeners.call ([&] (WaylandToplevelMappingListener& l) { l.waylandToplevelMapped (peer); });
}

std::optional<WaylandInputHandler::InputEventSerial> WaylandWindowSystem::getLatestInputSerial() const noexcept
{
    return inputHandler != nullptr ? inputHandler->getLatestInputSerial() : std::nullopt;
}

#if JUCE_WAYLAND_PEER_DIAGNOSTICS
detail::WaylandPeerDiagnostics::TextInput WaylandWindowSystem::getTextInputDiagnostics (wl_surface* surface) const
{
    return textInput != nullptr ? textInput->getDiagnostics (surface) : detail::WaylandPeerDiagnostics::TextInput{};
}
#endif

void WaylandWindowSystem::addTextInputClient (wl_surface* surface, WaylandTextInputClient& client)
{
    if (textInput != nullptr)
        textInput->addClient (surface, client);
}

void WaylandWindowSystem::removeTextInputClient (WaylandTextInputClient& client)
{
    if (textInput != nullptr)
        textInput->removeClient (client);
}

void WaylandWindowSystem::textInputRequired (wl_surface* surface, TextInputTarget& target)
{
    if (textInput != nullptr)
        textInput->requireInput (surface, target);
}

void WaylandWindowSystem::refreshTextInputState (wl_surface* surface)
{
    if (textInput != nullptr)
        textInput->refreshState (surface);
}

void WaylandWindowSystem::closeInputMethodContext (wl_surface* surface)
{
    if (textInput != nullptr)
        textInput->closeContext (surface);
}

void WaylandWindowSystem::dismissPendingTextInput (wl_surface* surface)
{
    if (textInput != nullptr)
        textInput->dismissPendingInput (surface);
}

void WaylandWindowSystem::handleSeatCapabilities (uint32_t capabilities)
{
    inputHandler->seatCapabilitiesChanged (seat.get(), capabilities);
}

void WaylandWindowSystem::handleSeatName (const char* name)
{
    seatName = String::fromUTF8 (name != nullptr ? name : "");
}

void WaylandWindowSystem::ensureDataDeviceBound()
{
    if (dataDevice != nullptr)
        dataDevice->bind (dataDeviceManager.get(), seat.get());
}

void WaylandWindowSystem::ensureTextInputBound()
{
    if (textInput != nullptr)
        textInput->bind (textInputManager.get(), seat.get());
}

void WaylandWindowSystem::inputSerialReceived (uint32_t serial)
{
    if (dataDevice != nullptr)
        dataDevice->inputSerialReceived (serial);
}

void WaylandWindowSystem::addInputListener (wl_surface* surface, WaylandInputHandlerListener& listener)
{
    inputHandler->addListener (surface, listener);
}

void WaylandWindowSystem::removeInputListener (WaylandInputHandlerListener& listener)
{
    if (inputHandler != nullptr)
        inputHandler->removeListener (listener);
}

WaylandInputHandlerListener* WaylandWindowSystem::findInputListener (wl_surface* surface) const
{
    return inputHandler != nullptr ? inputHandler->findListenerForSurface (surface) : nullptr;
}

bool WaylandWindowSystem::requestKeyboardFocus (wl_surface* surface)
{
    return inputHandler != nullptr && inputHandler->requestKeyboardFocus (surface);
}

bool WaylandWindowSystem::hasKeyboardFocus (wl_surface* surface) const
{
    return inputHandler != nullptr && inputHandler->hasKeyboardFocus (surface);
}

void WaylandWindowSystem::releaseKeyboardFocus (wl_surface* surface)
{
    if (inputHandler != nullptr)
        inputHandler->releaseKeyboardFocus (surface);
}

void WaylandWindowSystem::surfaceUnmapped (wl_surface* surface)
{
    if (inputHandler != nullptr)
        inputHandler->surfaceUnmapped (surface);
}

void WaylandWindowSystem::addDataDeviceListener (wl_surface* surface, WaylandDataDeviceListener& listener)
{
    if (dataDevice != nullptr)
        dataDevice->addListener (surface, listener);
}

void WaylandWindowSystem::removeDataDeviceListener (WaylandDataDeviceListener& listener)
{
    if (dataDevice != nullptr)
        dataDevice->removeListener (listener);
}

void WaylandWindowSystem::copyTextToClipboard (const String& text)
{
    if (dataDevice != nullptr)
        dataDevice->copyTextToClipboard (text);
}

String WaylandWindowSystem::getTextFromClipboard()
{
    return dataDevice != nullptr ? dataDevice->getTextFromClipboard() : String{};
}

bool WaylandWindowSystem::externalDragFileInit (wl_surface* surface, const StringArray& files,
                                                bool canMoveFiles, std::function<void()> callback)
{
    if (dataDevice == nullptr || inputHandler == nullptr)
        return false;

    if (const auto serial = getHeldPressSerial (surface))
    {
        const auto started = dataDevice->externalDragFileInit (surface, *serial, files, canMoveFiles,
                                                               std::move (callback));

        if (started)
        {
            inputHandler->externalDragStarted (surface);
            showExternalDragCursor (WaylandProtocol::wlDataDeviceManagerDndActionNone);
        }

        return started;
    }

    return false;
}

bool WaylandWindowSystem::externalDragTextInit (wl_surface* surface, const String& text,
                                                std::function<void()> callback)
{
    if (dataDevice == nullptr || inputHandler == nullptr)
        return false;

    if (const auto serial = getHeldPressSerial (surface))
    {
        const auto started = dataDevice->externalDragTextInit (surface, *serial, text, std::move (callback));

        if (started)
        {
            inputHandler->externalDragStarted (surface);
            showExternalDragCursor (WaylandProtocol::wlDataDeviceManagerDndActionNone);
        }

        return started;
    }

    return false;
}

void WaylandWindowSystem::dragSourceActionChanged (uint32_t action)
{
    showExternalDragCursor (action);
}

void WaylandWindowSystem::showExternalDragCursor (uint32_t action)
{
    if (! cursor.has_value() || inputHandler == nullptr)
        return;

    const auto target = inputHandler->getExternalDragCursorTarget();

    if (! target.has_value())
        return;

    auto cursorAction = WaylandCursor::DragAction::none;

    if (action == WaylandProtocol::wlDataDeviceManagerDndActionCopy)
        cursorAction = WaylandCursor::DragAction::copy;
    else if (action == WaylandProtocol::wlDataDeviceManagerDndActionMove)
        cursorAction = WaylandCursor::DragAction::move;

    cursor->showDrag (*target, cursorAction);
    flush();
}

void WaylandWindowSystem::externalDragEnded()
{
    if (inputHandler == nullptr)
        return;

    if (cursor.has_value())
    {
        if (const auto target = inputHandler->getExternalDragCursorTarget())
        {
            cursor->restoreCursor (*target);
            flush();
        }
    }

    inputHandler->externalDragEnded();
}

std::optional<WaylandPopupOwner> WaylandWindowSystem::findPopupOwner (int styleFlags)
{
    return inputHandler != nullptr ? findWaylandPopupOwner (inputHandler->getPopupContext(), styleFlags) : std::nullopt;
}

std::optional<WaylandPopupParent> WaylandWindowSystem::findPopupParent (wl_surface* toplevelSurface, WaylandPopupGrab grab)
{
    return inputHandler != nullptr ? findWaylandPopupParent (inputHandler->getPopupContext(), toplevelSurface, grab)
                                   : std::nullopt;
}

void WaylandWindowSystem::dismissPopupDescendants (xdg_surface* ancestorXdgSurface)
{
    if (inputHandler == nullptr)
        return;

    const auto descendants = findWaylandPopupDescendantsTopmostFirst (inputHandler->getPopupContext(),
                                                                      ancestorXdgSurface);

    for (auto* surface : descendants)
        if (auto* listener = inputHandler->findListenerForSurface (surface))
            listener->dismissPopupForAncestorUnmap();
}

std::optional<uint32_t> WaylandWindowSystem::getHeldPressSerial (wl_surface* surface) const
{
    if (inputHandler != nullptr)
        return inputHandler->getHeldPressSerial (surface);

    return std::nullopt;
}

void WaylandWindowSystem::dragHandedToCompositor (wl_surface* surface)
{
    if (inputHandler != nullptr)
        inputHandler->dragHandedToCompositor (surface);
}

void WaylandWindowSystem::popupGrabStarted (wl_surface* toplevelSurface, Component& componentToDismiss)
{
    if (inputHandler != nullptr)
        inputHandler->popupGrabStarted (toplevelSurface, componentToDismiss);
}

void WaylandWindowSystem::popupGrabEnded (wl_surface* toplevelSurface)
{
    if (inputHandler != nullptr)
        inputHandler->popupGrabEnded (toplevelSurface);
}

Point<float> WaylandWindowSystem::getCurrentMousePosition() const noexcept
{
    return inputHandler != nullptr ? inputHandler->getCurrentMousePosition()
                                   : Point<float>{};
}

void WaylandWindowSystem::showCursor (wl_surface* surface, MouseCursor::StandardCursorType type)
{
    if (! cursor.has_value() || inputHandler == nullptr)
        return;

    if (const auto target = inputHandler->getCursorTarget (surface))
    {
        cursor->show (*target, type);
        flush();
    }
}

void WaylandWindowSystem::showCursor (wl_surface* surface, const detail::CustomMouseCursorInfo* info)
{
    if (info == nullptr || ! cursor.has_value() || inputHandler == nullptr)
        return;

    if (const auto target = inputHandler->getCursorTarget (surface))
    {
        cursor->show (*target, info);
        flush();
    }
}

void WaylandWindowSystem::removeCustomCursorCache (const detail::CustomMouseCursorInfo& info)
{
    if (cursor.has_value())
        cursor->removeCustomCursorCache (info);
}

bool WaylandWindowSystem::isKeyCurrentlyDown (int keyCode) const
{
    if (inputHandler != nullptr)
        return inputHandler->isKeyCurrentlyDown (keyCode);

    return false;
}

void WaylandWindowSystem::setDisplaysChangedCallback (std::function<void()> callback)
{
    displaysChangedCallback = std::move (callback);
}

void WaylandWindowSystem::handleAsyncUpdate()
{
    // Update peer scales before publishing the new display configuration.
    outputListeners.call ([] (WaylandOutputListener& l) { l.outputConfigurationChanged(); });
    NullCheckedInvocation::invoke (displaysChangedCallback);
}

std::optional<int> WaylandWindowSystem::getScaleForOutput (wl_output* output) const
{
    for (const auto& candidate : outputs)
        if (candidate->handle.get() == output && candidate->currentState.has_value())
            return candidate->currentState->integerScaleFactor;

    return {};
}

int WaylandWindowSystem::getLargestOutputScale() const
{
    auto result = 1;

    for (const auto& output : outputs)
        if (output->currentState.has_value())
            result = jmax (result, output->currentState->integerScaleFactor);

    return result;
}

Array<Displays::Display> WaylandWindowSystem::findDisplays (float masterScale)
{
    Array<Displays::Display> result;

    const auto hasUsableMode = [] (const std::unique_ptr<BoundOutput>& output)
    {
        return output->currentState.has_value()
            && output->currentState->modeSize.x > 0
            && output->currentState->modeSize.y > 0;
    };

    const auto preferredMainOutput = std::invoke ([&]
    {
        if (mainOutputName.isEmpty())
            return outputs.end();

        return std::find_if (outputs.begin(), outputs.end(), [&] (const auto& output)
        {
            return hasUsableMode (output) && output->currentState->name == mainOutputName;
        });
    });

    const auto fallbackMainOutput = std::find_if (outputs.begin(), outputs.end(), hasUsableMode);
    const auto mainOutput = preferredMainOutput != outputs.end() ? preferredMainOutput : fallbackMainOutput;

    if (mainOutput != outputs.end() && mainOutputName.isEmpty())
        mainOutputName = (*mainOutput)->currentState->name;

    for (const auto& output : outputs)
    {
        // Expose an output only after the compositor has finished reporting a usable mode.
        if (! hasUsableMode (output))
            continue;

        const auto& state = *output->currentState;
        const auto geometry = getDisplayGeometry (state, masterScale);

        Displays::Display displayInfo;

        // Wayland has no primary-output setting, so retain the first output's stable name when possible.
        displayInfo.isMain = mainOutput != outputs.end() && output.get() == mainOutput->get();
        displayInfo.scale = geometry.scale;
        displayInfo.physicalBounds = geometry.physicalBounds;
        displayInfo.logicalBounds = geometry.logicalBounds;
        // Core Wayland has no work-area information.
        displayInfo.userBounds = displayInfo.logicalBounds;
        // Physical dimensions describe the panel in its normal orientation, so use the unrotated mode size.
        displayInfo.dpi = getDpi (state.modeSize, state.physicalWidthMm, state.physicalHeightMm);
        displayInfo.verticalFrequencyHz = state.refreshRateMilliHz > 0 ? std::optional<double> ((double) state.refreshRateMilliHz / 1000.0)
                                                                       : std::nullopt;

        result.add (displayInfo);
    }

    if (result.isEmpty())
    {
        // Preserve the existing fallback because some JUCE display queries assume a primary display exists.
        Displays::Display displayInfo;
        displayInfo.isMain = true;
        displayInfo.scale = masterScale;
        displayInfo.dpi = 96.0;
        displayInfo.physicalBounds = { 1024, 768 };
        displayInfo.logicalBounds = displayInfo.physicalBounds.toFloat() / masterScale;
        displayInfo.userBounds = displayInfo.logicalBounds;

        result.add (displayInfo);
    }

    return result;
}

std::unique_ptr<WaylandWindowSystem::ExportedSurfaceHandle> WaylandWindowSystem::exportSurfaceForExternalParenting (wl_surface* surface)
{
    if (exporter == nullptr || surface == nullptr || display == nullptr)
        return nullptr;

    auto result = std::make_unique<ExportedSurfaceHandle>();
    result->exported.reset (WaylandProtocol::zxdgExporterV2ExportToplevel (exporter.get(), surface));

    if (result->exported == nullptr)
        return nullptr;

    WaylandProtocol::zxdgExportedV2AddListener (result->exported.get(), &exportedListener, result.get());

    // The handle event arrives asynchronously, so roundtrip once here to hand callers a usable handle.
    if (WaylandClientSymbols::getInstance()->wl_display_roundtrip (display.get()) < 0 || result->handle.isEmpty())
        return nullptr;

    return result;
}

#if JUCE_UNIT_TESTS

//==============================================================================
class GetDisplayGeometryTests final : public UnitTest
{
public:
    GetDisplayGeometryTests()
        : UnitTest ("getDisplayGeometry", UnitTestCategories::gui) {}

    void runTest() override
    {
        testCase ("Logical bounds remain connected when neighbouring outputs use different scales", [&]
        {
            WaylandWindowSystem::BoundOutput::State first;
            first.modeSize = { 1920, 1080 };
            first.logicalPosition = Point<int> { 0, 0 };
            first.logicalSize = Point<int> { 1920, 1080 };

            WaylandWindowSystem::BoundOutput::State second;
            second.modeSize = { 3840, 2160 };
            second.logicalPosition = Point<int> { 1920, 0 };
            second.logicalSize = Point<int> { 1920, 1080 };

            const auto firstGeometry = getDisplayGeometry (first, 1.0f);
            const auto secondGeometry = getDisplayGeometry (second, 1.0f);

            expectEquals (firstGeometry.logicalBounds.getRight(), secondGeometry.logicalBounds.getX());
            expectEquals (firstGeometry.scale, 1.0);
            expectEquals (secondGeometry.scale, 2.0);
        });

        testCase ("Logical size determines a fractional display scale", [&]
        {
            WaylandWindowSystem::BoundOutput::State state;
            state.modeSize = { 3840, 2160 };
            state.logicalPosition = Point<int> { 0, 0 };
            state.logicalSize = Point<int> { 2560, 1440 };

            const auto geometry = getDisplayGeometry (state, 1.0f);

            expect (geometry.logicalBounds == Rectangle<float> { 0.0f, 0.0f, 2560.0f, 1440.0f });
            expect (geometry.physicalBounds == Rectangle<int> { 0, 0, 3840, 2160 });
            expectEquals (geometry.scale, 1.5);
        });

        testCase ("Logical size already accounts for output rotation", [&]
        {
            WaylandWindowSystem::BoundOutput::State state;
            state.modeSize = { 1920, 1080 };
            state.transform = WaylandProtocol::wlOutputTransform90;
            state.logicalPosition = Point<int> { 0, 0 };
            state.logicalSize = Point<int> { 540, 960 };

            const auto geometry = getDisplayGeometry (state, 1.0f);

            expect (geometry.logicalBounds == Rectangle<float> { 0.0f, 0.0f, 540.0f, 960.0f });
            expect (geometry.physicalBounds == Rectangle<int> { 0, 0, 1080, 1920 });
            expectEquals (geometry.scale, 2.0);
        });

        testCase ("Integer output scale determines display geometry when xdg-output is unavailable", [&]
        {
            WaylandWindowSystem::BoundOutput::State state;
            state.fallbackLogicalPosition = { 1920, 0 };
            state.modeSize = { 3840, 2160 };
            state.integerScaleFactor = 2;

            const auto geometry = getDisplayGeometry (state, 1.0f);

            expect (geometry.logicalBounds == Rectangle<float> { 1920.0f, 0.0f, 1920.0f, 1080.0f });
            expect (geometry.physicalBounds == Rectangle<int> { 3840, 0, 3840, 2160 });
            expectEquals (geometry.scale, 2.0);
        });

        testCase ("The JUCE desktop scale changes logical bounds without changing hardware size", [&]
        {
            WaylandWindowSystem::BoundOutput::State state;
            state.modeSize = { 2000, 1000 };
            state.logicalPosition = Point<int> { 200, 100 };
            state.logicalSize = Point<int> { 1000, 500 };

            const auto geometry = getDisplayGeometry (state, 1.25f);

            expect (geometry.logicalBounds == Rectangle<float> { 160.0f, 80.0f, 800.0f, 400.0f });
            expectEquals (geometry.physicalBounds.getWidth(), 2000);
            expectEquals (geometry.physicalBounds.getHeight(), 1000);
            expectEquals (geometry.scale, 2.5);
        });
    }
};

static GetDisplayGeometryTests getDisplayGeometryTests;

#endif

} // namespace juce
