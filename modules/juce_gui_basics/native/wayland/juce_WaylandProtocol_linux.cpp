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

// Request wrappers and wl_interface tables are written here so JUCE can load libwayland
// at runtime without Wayland development headers. This mirrors how JUCE handles X11Symbols.
//
// Wayland protocols are append-only. New requests only ever get higher
// numbers. Opcodes and argument signatures:
//   https://gitlab.freedesktop.org/wayland/wayland/-/blob/main/protocol/wayland.xml
//   https://gitlab.freedesktop.org/wayland/wayland-protocols/-/blob/main/stable/xdg-shell/xdg-shell.xml
//   https://gitlab.freedesktop.org/wayland/wayland-protocols/-/blob/main/unstable/xdg-decoration/xdg-decoration-unstable-v1.xml
//   https://gitlab.freedesktop.org/wayland/wayland-protocols/-/blob/main/unstable/xdg-foreign/xdg-foreign-unstable-v2.xml
//   https://gitlab.freedesktop.org/wayland/wayland-protocols/-/blob/main/unstable/xdg-output/xdg-output-unstable-v1.xml
//   https://gitlab.freedesktop.org/wayland/wayland-protocols/-/blob/main/stable/viewporter/viewporter.xml
//   https://gitlab.freedesktop.org/wayland/wayland-protocols/-/blob/main/staging/fractional-scale/fractional-scale-v1.xml
//   https://gitlab.freedesktop.org/wayland/wayland-protocols/-/blob/main/staging/alpha-modifier/alpha-modifier-v1.xml
//   https://gitlab.freedesktop.org/wayland/wayland-protocols/-/blob/main/staging/xdg-activation/xdg-activation-v1.xml
//   https://gitlab.freedesktop.org/wayland/wayland-protocols/-/blob/main/unstable/text-input/text-input-unstable-v3.xml
namespace juce::WaylandProtocol
{

namespace
{
    constexpr uint32_t destroyFlag = WL_MARSHAL_FLAG_DESTROY;

    struct SupportedProtocolVersions
    {
        SupportedProtocolVersions() = delete;

        static constexpr uint32_t compositor = 6;
        static constexpr uint32_t subcompositor = 1;
        static constexpr uint32_t shm = 1;
        static constexpr uint32_t dataDeviceManager = 3;
        static constexpr uint32_t output = 4;

        //   v1          capabilities, get_pointer/keyboard/touch, core pointer/keyboard/touch events
        //   v2          wl_seat.name
        //   v3          release destructors on wl_pointer/wl_keyboard/wl_touch
        //   v4          wl_keyboard.repeat_info (rate and delay, rate 0 means no repeat)
        //   v5          wl_seat.release, pointer frame, axis_source, axis_stop, axis_discrete
        //   v6          touch shape and orientation, axis_source gains wheel_tilt
        //   v7          no event changes (keymap fd must be mapped MAP_PRIVATE)
        //   v8   1.21   pointer axis_value120 (axis_discrete no longer sent)
        //   v9   1.22   pointer axis_relative_direction
        //   v10  1.24   key state gains repeated (compositor-driven repeat when repeat_info rate 0)
        //   v11  1.26   pointer warp event
        static constexpr uint32_t seat = 10;
    };

    // Request opcodes in protocol declaration order
    // Append-only. Never reorder these, only add at the end.
    enum class WlDisplayRequest           : uint32_t { sync, getRegistry };
    enum class WlRegistryRequest          : uint32_t { bind };
    enum class WlCompositorRequest        : uint32_t { createSurface, createRegion };
    enum class WlSubcompositorRequest     : uint32_t { destroy, getSubsurface };
    enum class WlSubsurfaceRequest        : uint32_t { destroy, setPosition, placeAbove, placeBelow,
                                                       setSync, setDesync };
    enum class WlRegionRequest            : uint32_t { destroy, add, subtract };
    enum class WlSurfaceRequest           : uint32_t { destroy, attach, damage, frame, setOpaqueRegion,
                                                       setInputRegion, commit, setBufferTransform,
                                                       setBufferScale, damageBuffer };
    enum class WlShmRequest               : uint32_t { createPool };
    enum class WlShmPoolRequest           : uint32_t { createBuffer, destroy };
    enum class WlBufferRequest            : uint32_t { destroy };
    enum class WlDataOfferRequest         : uint32_t { accept, receive, destroy, finish, setActions };
    enum class WlDataSourceRequest        : uint32_t { offer, destroy, setActions };
    enum class WlDataDeviceRequest        : uint32_t { startDrag, setSelection, release };
    enum class WlDataDeviceManagerRequest : uint32_t { createDataSource, getDataDevice };
    enum class WlOutputRequest            : uint32_t { release };
    enum class WlSeatRequest              : uint32_t { getPointer, getKeyboard, getTouch, release };
    enum class WlPointerRequest           : uint32_t { setCursor, release };
    enum class WlKeyboardRequest          : uint32_t { release };
    enum class WlTouchRequest             : uint32_t { release };
    enum class ZwpTextInputManagerV3Request : uint32_t { destroy, getTextInput };
    enum class ZwpTextInputV3Request        : uint32_t { destroy, enable, disable, setSurroundingText,
                                                      setTextChangeCause, setContentType,
                                                      setCursorRectangle, commit };
    enum class XdgWmBaseRequest     : uint32_t { destroy, createPositioner, getXdgSurface, pong };
    enum class XdgSurfaceRequest    : uint32_t { destroy, getToplevel, getPopup, setWindowGeometry, ackConfigure };
    enum class XdgPositionerRequest : uint32_t { destroy, setSize, setAnchorRect, setAnchor, setGravity,
                                                 setConstraintAdjustment, setOffset, setReactive,
                                                 setParentSize, setParentConfigure };
    enum class XdgPopupRequest      : uint32_t { destroy, grab, reposition };
    enum class XdgToplevelRequest   : uint32_t { destroy, setParent, setTitle, setAppId, showWindowMenu,
                                                 move, resize, setMaxSize, setMinSize, setMaximized, unsetMaximized,
                                                 setFullscreen, unsetFullscreen, setMinimized };
    enum class XdgActivationV1Request      : uint32_t { destroy, getActivationToken, activate };
    enum class XdgActivationTokenV1Request : uint32_t { setSerial, setAppId, setSurface, commit, destroy };
    enum class ZxdgDecorationManagerV1Request  : uint32_t { destroy, getToplevelDecoration };
    enum class ZxdgToplevelDecorationV1Request : uint32_t { destroy, setMode, unsetMode };
    enum class ZxdgExporterV2Request : uint32_t { destroy, exportToplevel };
    enum class ZxdgExportedV2Request : uint32_t { destroy };
    enum class ZxdgOutputManagerV1Request : uint32_t { destroy, getXdgOutput };
    enum class ZxdgOutputV1Request        : uint32_t { destroy };
    enum class WpViewporterRequest   : uint32_t { destroy, getViewport };
    enum class WpViewportRequest     : uint32_t { destroy, setSource, setDestination };
    enum class WpFractionalScaleManagerV1Request : uint32_t { destroy, getFractionalScale };
    enum class WpFractionalScaleV1Request        : uint32_t { destroy };
    enum class WpAlphaModifierV1Request          : uint32_t { destroy, getSurface };
    enum class WpAlphaModifierSurfaceV1Request   : uint32_t { destroy, setMultiplier };

    wl_interface zwpTextInputManagerV3Interface;
    wl_interface zwpTextInputV3Interface;
    wl_interface xdgPositionerInterface;
    wl_interface xdgPopupInterface;
    wl_interface xdgSurfaceInterface;
    wl_interface xdgToplevelInterface;
    wl_interface xdgWmBaseInterface;
    wl_interface xdgActivationV1Interface;
    wl_interface xdgActivationTokenV1Interface;
    wl_interface zxdgDecorationManagerV1Interface;
    wl_interface zxdgToplevelDecorationV1Interface;
    wl_interface zxdgExporterV2Interface;
    wl_interface zxdgExportedV2Interface;
    wl_interface zxdgOutputManagerV1Interface;
    wl_interface zxdgOutputV1Interface;
    wl_interface wpViewporterInterface;
    wl_interface wpViewportInterface;
    wl_interface wpFractionalScaleManagerV1Interface;
    wl_interface wpFractionalScaleV1Interface;
    wl_interface wpAlphaModifierV1Interface;
    wl_interface wpAlphaModifierSurfaceV1Interface;

    const wl_interface* xdgWmBaseCreatePositionerTypes[] { &xdgPositionerInterface };
    const wl_interface* xdgWmBaseGetXdgSurfaceTypes[]    { &xdgSurfaceInterface, nullptr };

    const wl_message xdgWmBaseRequests[]
    {
        { "destroy", "", nullptr },
        { "create_positioner", "n", xdgWmBaseCreatePositionerTypes },
        { "get_xdg_surface", "no", xdgWmBaseGetXdgSurfaceTypes },
        { "pong", "u", nullptr }
    };

    const wl_message xdgWmBaseEvents[]
    {
        { "ping", "u", nullptr }
    };

    const wl_interface* xdgSurfaceGetToplevelTypes[] { &xdgToplevelInterface };
    const wl_interface* xdgSurfaceGetPopupTypes[]    { &xdgPopupInterface, &xdgSurfaceInterface, &xdgPositionerInterface };

    const wl_message xdgSurfaceRequests[]
    {
        { "destroy", "", nullptr },
        { "get_toplevel", "n", xdgSurfaceGetToplevelTypes },
        { "get_popup", "n?oo", xdgSurfaceGetPopupTypes },
        { "set_window_geometry", "iiii", nullptr },
        { "ack_configure", "u", nullptr }
    };

    const wl_message xdgSurfaceEvents[]
    {
        { "configure", "u", nullptr }
    };

    const wl_interface* xdgToplevelSetParentTypes[]      { &xdgToplevelInterface };
    const wl_interface* xdgToplevelShowWindowMenuTypes[] { nullptr, nullptr, nullptr, nullptr };
    const wl_interface* xdgToplevelMoveTypes[]           { nullptr, nullptr };
    const wl_interface* xdgToplevelResizeTypes[]         { nullptr, nullptr, nullptr };
    const wl_interface* xdgToplevelSetFullscreenTypes[]  { nullptr };

    const wl_message xdgToplevelRequests[]
    {
        { "destroy", "", nullptr },
        { "set_parent", "?o", xdgToplevelSetParentTypes },
        { "set_title", "s", nullptr },
        { "set_app_id", "s", nullptr },
        { "show_window_menu", "ouii", xdgToplevelShowWindowMenuTypes },
        { "move", "ou", xdgToplevelMoveTypes },
        { "resize", "ouu", xdgToplevelResizeTypes },
        { "set_max_size", "ii", nullptr },
        { "set_min_size", "ii", nullptr },
        { "set_maximized", "", nullptr },
        { "unset_maximized", "", nullptr },
        { "set_fullscreen", "?o", xdgToplevelSetFullscreenTypes },
        { "unset_fullscreen", "", nullptr },
        { "set_minimized", "", nullptr }
    };

    const wl_message xdgToplevelEvents[]
    {
        { "configure", "iia", nullptr },
        { "close", "", nullptr },
        { "configure_bounds", "4ii", nullptr },
        { "wm_capabilities", "5a", nullptr }
    };

    const wl_message xdgPositionerRequests[]
    {
        { "destroy", "", nullptr },
        { "set_size", "ii", nullptr },
        { "set_anchor_rect", "iiii", nullptr },
        { "set_anchor", "u", nullptr },
        { "set_gravity", "u", nullptr },
        { "set_constraint_adjustment", "u", nullptr },
        { "set_offset", "ii", nullptr },
        { "set_reactive", "3", nullptr },
        { "set_parent_size", "3ii", nullptr },
        { "set_parent_configure", "3u", nullptr }
    };

    const wl_interface* xdgPopupGrabTypes[]       { nullptr, nullptr };
    const wl_interface* xdgPopupRepositionTypes[] { &xdgPositionerInterface, nullptr };

    const wl_message xdgPopupRequests[]
    {
        { "destroy", "", nullptr },
        { "grab", "ou", xdgPopupGrabTypes },
        { "reposition", "3ou", xdgPopupRepositionTypes }
    };

    const wl_message xdgPopupEvents[]
    {
        { "configure", "iiii", nullptr },
        { "popup_done", "", nullptr },
        { "repositioned", "3u", nullptr }
    };

    const wl_interface* xdgActivationV1GetActivationTokenTypes[] { &xdgActivationTokenV1Interface };
    const wl_interface* xdgActivationV1ActivateTypes[]           { nullptr, nullptr };

    const wl_message xdgActivationV1Requests[]
    {
        { "destroy", "", nullptr },
        { "get_activation_token", "n", xdgActivationV1GetActivationTokenTypes },
        { "activate", "so", xdgActivationV1ActivateTypes }
    };

    const wl_interface* xdgActivationTokenV1SetSerialTypes[]  { nullptr, nullptr };
    const wl_interface* xdgActivationTokenV1SetSurfaceTypes[] { nullptr };

    const wl_message xdgActivationTokenV1Requests[]
    {
        { "set_serial", "uo", xdgActivationTokenV1SetSerialTypes },
        { "set_app_id", "s", nullptr },
        { "set_surface", "o", xdgActivationTokenV1SetSurfaceTypes },
        { "commit", "", nullptr },
        { "destroy", "", nullptr }
    };

    const wl_message xdgActivationTokenV1Events[]
    {
        { "done", "s", nullptr }
    };

    const wl_interface* zxdgDecorationManagerV1GetToplevelDecorationTypes[] { &zxdgToplevelDecorationV1Interface, &xdgToplevelInterface };

    const wl_message zxdgDecorationManagerV1Requests[]
    {
        { "destroy", "", nullptr },
        { "get_toplevel_decoration", "no", zxdgDecorationManagerV1GetToplevelDecorationTypes }
    };

    const wl_message zxdgToplevelDecorationV1Requests[]
    {
        { "destroy", "", nullptr },
        { "set_mode", "u", nullptr },
        { "unset_mode", "", nullptr }
    };

    const wl_message zxdgToplevelDecorationV1Events[]
    {
        { "configure", "u", nullptr }
    };

    const wl_interface* zxdgExporterV2ExportToplevelTypes[] { &zxdgExportedV2Interface, nullptr };

    const wl_message zxdgExporterV2Requests[]
    {
        { "destroy", "", nullptr },
        { "export_toplevel", "no", zxdgExporterV2ExportToplevelTypes }
    };

    const wl_message zxdgExportedV2Requests[] { { "destroy", "", nullptr } };
    const wl_message zxdgExportedV2Events[]   { { "handle", "s", nullptr } };

    const wl_interface* zxdgOutputManagerV1GetXdgOutputTypes[] { &zxdgOutputV1Interface, nullptr };

    const wl_message zxdgOutputManagerV1Requests[]
    {
        { "destroy", "", nullptr },
        { "get_xdg_output", "no", zxdgOutputManagerV1GetXdgOutputTypes }
    };

    const wl_message zxdgOutputV1Requests[] { { "destroy", "", nullptr } };

    const wl_message zxdgOutputV1Events[]
    {
        { "logical_position", "ii", nullptr },
        { "logical_size", "ii", nullptr },
        { "done", "", nullptr },
        { "name", "2s", nullptr },
        { "description", "2s", nullptr }
    };

    const wl_interface* wpViewporterGetViewportTypes[] { &wpViewportInterface, nullptr };

    const wl_message wpViewporterRequests[]
    {
        { "destroy", "", nullptr },
        { "get_viewport", "no", wpViewporterGetViewportTypes }
    };

    const wl_message wpViewportRequests[]
    {
        { "destroy", "", nullptr },
        { "set_source", "ffff", nullptr },
        { "set_destination", "ii", nullptr }
    };

    const wl_interface* wpFractionalScaleManagerV1GetFractionalScaleTypes[] { &wpFractionalScaleV1Interface, nullptr };

    const wl_message wpFractionalScaleManagerV1Requests[]
    {
        { "destroy", "", nullptr },
        { "get_fractional_scale", "no", wpFractionalScaleManagerV1GetFractionalScaleTypes }
    };

    const wl_message wpFractionalScaleV1Requests[] { { "destroy", "", nullptr } };
    const wl_message wpFractionalScaleV1Events[]   { { "preferred_scale", "u", nullptr } };

    const wl_interface* wpAlphaModifierV1GetSurfaceTypes[] { &wpAlphaModifierSurfaceV1Interface, nullptr };

    const wl_message wpAlphaModifierV1Requests[]
    {
        { "destroy", "", nullptr },
        { "get_surface", "no", wpAlphaModifierV1GetSurfaceTypes }
    };

    const wl_message wpAlphaModifierSurfaceV1Requests[]
    {
        { "destroy", "", nullptr },
        { "set_multiplier", "u", nullptr }
    };

    const wl_interface* zwpTextInputManagerV3GetTextInputTypes[] { &zwpTextInputV3Interface, nullptr };

    const wl_message zwpTextInputManagerV3Requests[]
    {
        { "destroy", "", nullptr },
        { "get_text_input", "no", zwpTextInputManagerV3GetTextInputTypes }
    };

    const wl_message zwpTextInputV3Requests[]
    {
        { "destroy", "", nullptr },
        { "enable", "", nullptr },
        { "disable", "", nullptr },
        { "set_surrounding_text", "sii", nullptr },
        { "set_text_change_cause", "u", nullptr },
        { "set_content_type", "uu", nullptr },
        { "set_cursor_rectangle", "iiii", nullptr },
        { "commit", "", nullptr }
    };

    const wl_interface* zwpTextInputV3EnterTypes[] { nullptr };
    const wl_interface* zwpTextInputV3LeaveTypes[] { nullptr };

    const wl_message zwpTextInputV3Events[]
    {
        { "enter", "o", zwpTextInputV3EnterTypes },
        { "leave", "o", zwpTextInputV3LeaveTypes },
        { "preedit_string", "?sii", nullptr },
        { "commit_string", "?s", nullptr },
        { "delete_surrounding_text", "uu", nullptr },
        { "done", "u", nullptr }
    };

    void initialiseInterfaceObjects()
    {
        zwpTextInputManagerV3Interface = { "zwp_text_input_manager_v3", 1, numElementsInArray (zwpTextInputManagerV3Requests), zwpTextInputManagerV3Requests, 0, nullptr };
        zwpTextInputV3Interface = { "zwp_text_input_v3", 1, numElementsInArray (zwpTextInputV3Requests), zwpTextInputV3Requests, numElementsInArray (zwpTextInputV3Events), zwpTextInputV3Events };
        xdgPositionerInterface = { "xdg_positioner", 6, numElementsInArray (xdgPositionerRequests), xdgPositionerRequests, 0, nullptr };
        xdgPopupInterface      = { "xdg_popup",      6, numElementsInArray (xdgPopupRequests),      xdgPopupRequests,      numElementsInArray (xdgPopupEvents),      xdgPopupEvents };
        xdgSurfaceInterface    = { "xdg_surface",    6, numElementsInArray (xdgSurfaceRequests),    xdgSurfaceRequests,    numElementsInArray (xdgSurfaceEvents),    xdgSurfaceEvents };
        xdgToplevelInterface   = { "xdg_toplevel",   6, numElementsInArray (xdgToplevelRequests),   xdgToplevelRequests,   numElementsInArray (xdgToplevelEvents),   xdgToplevelEvents };
        xdgWmBaseInterface     = { "xdg_wm_base",    6, numElementsInArray (xdgWmBaseRequests),     xdgWmBaseRequests,     numElementsInArray (xdgWmBaseEvents),     xdgWmBaseEvents };
        xdgActivationV1Interface      = { "xdg_activation_v1", 1, numElementsInArray (xdgActivationV1Requests), xdgActivationV1Requests, 0, nullptr };
        xdgActivationTokenV1Interface = { "xdg_activation_token_v1", 1, numElementsInArray (xdgActivationTokenV1Requests), xdgActivationTokenV1Requests, numElementsInArray (xdgActivationTokenV1Events), xdgActivationTokenV1Events };
        zxdgDecorationManagerV1Interface  = { "zxdg_decoration_manager_v1", 1, numElementsInArray (zxdgDecorationManagerV1Requests), zxdgDecorationManagerV1Requests, 0, nullptr };
        zxdgToplevelDecorationV1Interface = { "zxdg_toplevel_decoration_v1", 1, numElementsInArray (zxdgToplevelDecorationV1Requests), zxdgToplevelDecorationV1Requests, numElementsInArray (zxdgToplevelDecorationV1Events), zxdgToplevelDecorationV1Events };
        zxdgExporterV2Interface = { "zxdg_exporter_v2", 1, numElementsInArray (zxdgExporterV2Requests), zxdgExporterV2Requests, 0, nullptr };
        zxdgExportedV2Interface = { "zxdg_exported_v2", 1, numElementsInArray (zxdgExportedV2Requests), zxdgExportedV2Requests, numElementsInArray (zxdgExportedV2Events), zxdgExportedV2Events };
        zxdgOutputManagerV1Interface = { "zxdg_output_manager_v1", 3, numElementsInArray (zxdgOutputManagerV1Requests), zxdgOutputManagerV1Requests, 0, nullptr };
        zxdgOutputV1Interface = { "zxdg_output_v1", 3, numElementsInArray (zxdgOutputV1Requests), zxdgOutputV1Requests, numElementsInArray (zxdgOutputV1Events), zxdgOutputV1Events };
        wpViewporterInterface   = { "wp_viewporter", 1, numElementsInArray (wpViewporterRequests), wpViewporterRequests, 0, nullptr };
        wpViewportInterface     = { "wp_viewport", 1, numElementsInArray (wpViewportRequests), wpViewportRequests, 0, nullptr };
        wpFractionalScaleManagerV1Interface = { "wp_fractional_scale_manager_v1", 1, numElementsInArray (wpFractionalScaleManagerV1Requests), wpFractionalScaleManagerV1Requests, 0, nullptr };
        wpFractionalScaleV1Interface        = { "wp_fractional_scale_v1", 1, numElementsInArray (wpFractionalScaleV1Requests), wpFractionalScaleV1Requests, numElementsInArray (wpFractionalScaleV1Events), wpFractionalScaleV1Events };
        wpAlphaModifierV1Interface        = { "wp_alpha_modifier_v1", 1, numElementsInArray (wpAlphaModifierV1Requests), wpAlphaModifierV1Requests, 0, nullptr };
        wpAlphaModifierSurfaceV1Interface = { "wp_alpha_modifier_surface_v1", 1, numElementsInArray (wpAlphaModifierSurfaceV1Requests), wpAlphaModifierSurfaceV1Requests, 0, nullptr };
    }

    uint32_t getVersion (wl_proxy* proxy)
    {
        return WaylandClientSymbols::getInstance()->wl_proxy_get_version (proxy);
    }

    template <typename Object>
    wl_proxy* proxy (Object* object)
    {
        return reinterpret_cast<wl_proxy*> (object);
    }

    template <typename Object>
    Object* object (wl_proxy* proxyIn)
    {
        return reinterpret_cast<Object*> (proxyIn);
    }

    void* registryBind (wl_registry* registry, uint32_t name, const wl_interface* interface, uint32_t version)
    {
        return WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (registry),
                                                                            toUnderlyingType (WlRegistryRequest::bind),
                                                                            interface,
                                                                            version,
                                                                            0,
                                                                            name,
                                                                            interface->name,
                                                                            version,
                                                                            nullptr);
    }

    uint32_t getBindVersion (uint32_t advertisedVersion,
                             const wl_interface& interface,
                             uint32_t highestSupportedVersion)
    {
        // Clamping to interface.version prevents an older libwayland from receiving events it cannot dispatch.
        return jmin (advertisedVersion, (uint32_t) interface.version, highestSupportedVersion);
    }

    template <typename Object>
    Object* bindGlobal (wl_registry* registry,
                        uint32_t name,
                        uint32_t advertisedVersion,
                        const wl_interface& interface,
                        uint32_t highestSupportedVersion)
    {
        return static_cast<Object*> (registryBind (registry,
                                                   name,
                                                   &interface,
                                                   getBindVersion (advertisedVersion, interface, highestSupportedVersion)));
    }

    template <typename Object>
    Object* bindGlobal (wl_registry* registry,
                        uint32_t name,
                        uint32_t advertisedVersion,
                        const wl_interface& interface)
    {
        return bindGlobal<Object> (registry, name, advertisedVersion, interface, (uint32_t) interface.version);
    }
}

//==============================================================================
// Wayland represents wl_fixed_t as signed 24.8 fixed point.
static constexpr int wlFixedScale = 256;

Point<float> fixedToPoint (int32_t x, int32_t y)
{
    return { (float) x / wlFixedScale, (float) y / wlFixedScale };
}

void initialiseInterfaces (const WaylandClientSymbols& symbols)
{
    initialiseInterfaceObjects();
    zwpTextInputManagerV3GetTextInputTypes[1]               = symbols.wl_seat_interface;
    zwpTextInputV3EnterTypes[0]                             = symbols.wl_surface_interface;
    zwpTextInputV3LeaveTypes[0]                             = symbols.wl_surface_interface;
    xdgWmBaseGetXdgSurfaceTypes[1]                          = symbols.wl_surface_interface;
    xdgPopupGrabTypes[0]                                    = symbols.wl_seat_interface;
    xdgToplevelShowWindowMenuTypes[0]                       = symbols.wl_seat_interface;
    xdgToplevelMoveTypes[0]                                 = symbols.wl_seat_interface;
    xdgToplevelResizeTypes[0]                               = symbols.wl_seat_interface;
    xdgToplevelSetFullscreenTypes[0]                        = symbols.wl_output_interface;
    xdgActivationV1ActivateTypes[1]                         = symbols.wl_surface_interface;
    xdgActivationTokenV1SetSerialTypes[1]                   = symbols.wl_seat_interface;
    xdgActivationTokenV1SetSurfaceTypes[0]                  = symbols.wl_surface_interface;
    zxdgExporterV2ExportToplevelTypes[1]                    = symbols.wl_surface_interface;
    zxdgOutputManagerV1GetXdgOutputTypes[1]                 = symbols.wl_output_interface;
    wpViewporterGetViewportTypes[1]                         = symbols.wl_surface_interface;
    wpFractionalScaleManagerV1GetFractionalScaleTypes[1]    = symbols.wl_surface_interface;
    wpAlphaModifierV1GetSurfaceTypes[1]                     = symbols.wl_surface_interface;
}

wl_registry* wlDisplayGetRegistry (wl_display* display)
{
    return object<wl_registry> (WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (display),
                                                                                             toUnderlyingType (WlDisplayRequest::getRegistry),
                                                                                             WaylandClientSymbols::getInstance()->wl_registry_interface,
                                                                                             1,
                                                                                             0,
                                                                                             nullptr));
}

wl_callback* wlDisplaySync (wl_display* display)
{
    return object<wl_callback> (WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (display),
                                                                                             toUnderlyingType (WlDisplayRequest::sync),
                                                                                             WaylandClientSymbols::getInstance()->wl_callback_interface,
                                                                                             1,
                                                                                             0,
                                                                                             nullptr));
}

int wlRegistryAddListener (wl_registry* registry, const wl_registry_listener* listener, void* data)
{
    return WaylandClientSymbols::getInstance()->wl_proxy_add_listener (proxy (registry), reinterpret_cast<void (**) (void)> (const_cast<wl_registry_listener*> (listener)), data);
}

void wlRegistryDestroy (wl_registry* registry)
{
    WaylandClientSymbols::getInstance()->wl_proxy_destroy (proxy (registry));
}

wl_compositor* bindCompositor (wl_registry* registry, uint32_t name, uint32_t version)
{
    return bindGlobal<wl_compositor> (registry, name, version, *WaylandClientSymbols::getInstance()->wl_compositor_interface, SupportedProtocolVersions::compositor);
}

wl_surface* wlCompositorCreateSurface (wl_compositor* compositor)
{
    return object<wl_surface> (WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (compositor),
                                                                                            toUnderlyingType (WlCompositorRequest::createSurface),
                                                                                            WaylandClientSymbols::getInstance()->wl_surface_interface,
                                                                                            getVersion (proxy (compositor)),
                                                                                            0,
                                                                                            nullptr));
}

wl_region* wlCompositorCreateRegion (wl_compositor* compositor)
{
    return object<wl_region> (WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (compositor),
                                                                                        toUnderlyingType (WlCompositorRequest::createRegion),
                                                                                        WaylandClientSymbols::getInstance()->wl_region_interface,
                                                                                        getVersion (proxy (compositor)),
                                                                                        0,
                                                                                        nullptr));
}

void wlCompositorDestroy (wl_compositor* compositor)
{
    WaylandClientSymbols::getInstance()->wl_proxy_destroy (proxy (compositor));
}

wl_subcompositor* bindSubcompositor (wl_registry* registry, uint32_t name, uint32_t version)
{
    return bindGlobal<wl_subcompositor> (registry, name, version,
                                         *WaylandClientSymbols::getInstance()->wl_subcompositor_interface,
                                         SupportedProtocolVersions::subcompositor);
}

wl_subsurface* wlSubcompositorGetSubsurface (wl_subcompositor* subcompositor,
                                             wl_surface* surface,
                                             wl_surface* parent)
{
    return object<wl_subsurface> (WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (subcompositor),
                                                                                               toUnderlyingType (WlSubcompositorRequest::getSubsurface),
                                                                                               WaylandClientSymbols::getInstance()->wl_subsurface_interface,
                                                                                               getVersion (proxy (subcompositor)),
                                                                                               0,
                                                                                               nullptr,
                                                                                               surface,
                                                                                               parent));
}

void wlSubcompositorDestroy (wl_subcompositor* subcompositor)
{
    WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (subcompositor),
                                                                 toUnderlyingType (WlSubcompositorRequest::destroy),
                                                                 nullptr,
                                                                 getVersion (proxy (subcompositor)),
                                                                 destroyFlag);
}

void wlSubsurfaceDestroy (wl_subsurface* subsurface)
{
    WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (subsurface),
                                                                 toUnderlyingType (WlSubsurfaceRequest::destroy),
                                                                 nullptr,
                                                                 getVersion (proxy (subsurface)),
                                                                 destroyFlag);
}

void wlSubsurfaceSetPosition (wl_subsurface* subsurface, int32_t x, int32_t y)
{
    WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (subsurface),
                                                                 toUnderlyingType (WlSubsurfaceRequest::setPosition),
                                                                 nullptr,
                                                                 getVersion (proxy (subsurface)),
                                                                 0,
                                                                 x,
                                                                 y);
}

void wlSubsurfaceSetSync (wl_subsurface* subsurface)
{
    WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (subsurface),
                                                                 toUnderlyingType (WlSubsurfaceRequest::setSync),
                                                                 nullptr,
                                                                 getVersion (proxy (subsurface)),
                                                                 0);
}

void wlSubsurfaceSetDesync (wl_subsurface* subsurface)
{
    WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (subsurface),
                                                                 toUnderlyingType (WlSubsurfaceRequest::setDesync),
                                                                 nullptr,
                                                                 getVersion (proxy (subsurface)),
                                                                 0);
}

int wlSurfaceAddListener (wl_surface* surface, const wl_surface_listener* listener, void* data)
{
    return WaylandClientSymbols::getInstance()->wl_proxy_add_listener (proxy (surface), reinterpret_cast<void (**) (void)> (const_cast<wl_surface_listener*> (listener)), data);
}

void wlSurfaceDestroy (wl_surface* surface)
{
    WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (surface), toUnderlyingType (WlSurfaceRequest::destroy), nullptr, getVersion (proxy (surface)), destroyFlag);
}

void wlSurfaceAttach (wl_surface* surface, wl_buffer* buffer, int32_t x, int32_t y)
{
    WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (surface), toUnderlyingType (WlSurfaceRequest::attach), nullptr, getVersion (proxy (surface)), 0, buffer, x, y);
}

void wlSurfaceDamageBuffer (wl_surface* surface, int32_t x, int32_t y, int32_t width, int32_t height)
{
    const auto version = getVersion (proxy (surface));

    if (version >= 4)
        WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (surface), toUnderlyingType (WlSurfaceRequest::damageBuffer), nullptr, version, 0, x, y, width, height);
    else
        // wl_surface.damage measures in surface coordinates rather than buffer pixels, so damaging everything is the only correct translation.
        WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (surface), toUnderlyingType (WlSurfaceRequest::damage), nullptr, version, 0, 0, 0, std::numeric_limits<int32_t>::max(), std::numeric_limits<int32_t>::max());
}

wl_callback* wlSurfaceFrame (wl_surface* surface)
{
    return object<wl_callback> (WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (surface),
                                                                                             toUnderlyingType (WlSurfaceRequest::frame),
                                                                                             WaylandClientSymbols::getInstance()->wl_callback_interface,
                                                                                             getVersion (proxy (surface)),
                                                                                             0,
                                                                                             nullptr));
}

void wlSurfaceSetOpaqueRegion (wl_surface* surface, wl_region* region)
{
    WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (surface), toUnderlyingType (WlSurfaceRequest::setOpaqueRegion), nullptr, getVersion (proxy (surface)), 0, region);
}

void wlSurfaceSetInputRegion (wl_surface* surface, wl_region* region)
{
    WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (surface), toUnderlyingType (WlSurfaceRequest::setInputRegion), nullptr, getVersion (proxy (surface)), 0, region);
}

void wlSurfaceSetBufferScale (wl_surface* surface, int32_t scale)
{
    WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (surface), toUnderlyingType (WlSurfaceRequest::setBufferScale), nullptr, getVersion (proxy (surface)), 0, scale);
}

void wlSurfaceCommit (wl_surface* surface)
{
    WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (surface), toUnderlyingType (WlSurfaceRequest::commit), nullptr, getVersion (proxy (surface)), 0);
}

void wlRegionAdd (wl_region* region, int32_t x, int32_t y, int32_t width, int32_t height)
{
    WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (region), toUnderlyingType (WlRegionRequest::add), nullptr, getVersion (proxy (region)), 0, x, y, width, height);
}

void wlRegionDestroy (wl_region* region)
{
    WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (region), toUnderlyingType (WlRegionRequest::destroy), nullptr, getVersion (proxy (region)), destroyFlag);
}

int wlCallbackAddListener (wl_callback* callback, const wl_callback_listener* listener, void* data)
{
    return WaylandClientSymbols::getInstance()->wl_proxy_add_listener (proxy (callback), reinterpret_cast<void (**) (void)> (const_cast<wl_callback_listener*> (listener)), data);
}

void wlCallbackDestroy (wl_callback* callback)
{
    WaylandClientSymbols::getInstance()->wl_proxy_destroy (proxy (callback));
}

wl_shm* bindShm (wl_registry* registry, uint32_t name, uint32_t version)
{
    return bindGlobal<wl_shm> (registry, name, version,
                               *WaylandClientSymbols::getInstance()->wl_shm_interface,
                               SupportedProtocolVersions::shm);
}

wl_shm_pool* wlShmCreatePool (wl_shm* shm, int fd, int32_t size)
{
    return object<wl_shm_pool> (WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (shm),
                                                                                             toUnderlyingType (WlShmRequest::createPool),
                                                                                             WaylandClientSymbols::getInstance()->wl_shm_pool_interface,
                                                                                             getVersion (proxy (shm)),
                                                                                             0,
                                                                                             nullptr,
                                                                                             fd,
                                                                                             size));
}

wl_buffer* wlShmPoolCreateBuffer (wl_shm_pool* pool, int32_t offset, int32_t width, int32_t height, int32_t stride, uint32_t format)
{
    return object<wl_buffer> (WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (pool),
                                                                                           toUnderlyingType (WlShmPoolRequest::createBuffer),
                                                                                           WaylandClientSymbols::getInstance()->wl_buffer_interface,
                                                                                           getVersion (proxy (pool)),
                                                                                           0,
                                                                                           nullptr,
                                                                                           offset,
                                                                                           width,
                                                                                           height,
                                                                                           stride,
                                                                                           format));
}

void wlShmDestroy (wl_shm* shm)
{
    WaylandClientSymbols::getInstance()->wl_proxy_destroy (proxy (shm));
}

void wlShmPoolDestroy (wl_shm_pool* pool)
{
    WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (pool), toUnderlyingType (WlShmPoolRequest::destroy), nullptr, getVersion (proxy (pool)), destroyFlag);
}

int wlBufferAddListener (wl_buffer* buffer, const wl_buffer_listener* listener, void* data)
{
    return WaylandClientSymbols::getInstance()->wl_proxy_add_listener (proxy (buffer), reinterpret_cast<void (**) (void)> (const_cast<wl_buffer_listener*> (listener)), data);
}

void wlBufferDestroy (wl_buffer* buffer)
{
    WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (buffer), toUnderlyingType (WlBufferRequest::destroy), nullptr, getVersion (proxy (buffer)), destroyFlag);
}

wl_data_device_manager* bindDataDeviceManager (wl_registry* registry, uint32_t name, uint32_t version)
{
    return bindGlobal<wl_data_device_manager> (registry, name, version,
                                               *WaylandClientSymbols::getInstance()->wl_data_device_manager_interface,
                                               SupportedProtocolVersions::dataDeviceManager);
}

wl_data_source* wlDataDeviceManagerCreateDataSource (wl_data_device_manager* manager)
{
    return object<wl_data_source> (WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (manager),
                                                                                                toUnderlyingType (WlDataDeviceManagerRequest::createDataSource),
                                                                                                WaylandClientSymbols::getInstance()->wl_data_source_interface,
                                                                                                getVersion (proxy (manager)),
                                                                                                0,
                                                                                                nullptr));
}

wl_data_device* wlDataDeviceManagerGetDataDevice (wl_data_device_manager* manager, wl_seat* seat)
{
    return object<wl_data_device> (WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (manager),
                                                                                                toUnderlyingType (WlDataDeviceManagerRequest::getDataDevice),
                                                                                                WaylandClientSymbols::getInstance()->wl_data_device_interface,
                                                                                                getVersion (proxy (manager)),
                                                                                                0,
                                                                                                nullptr,
                                                                                                seat));
}

void destroyDataDeviceManager (wl_data_device_manager* manager)
{
    WaylandClientSymbols::getInstance()->wl_proxy_destroy (proxy (manager));
}

int wlDataSourceAddListener (wl_data_source* source, const wl_data_source_listener* listener, void* data)
{
    return WaylandClientSymbols::getInstance()->wl_proxy_add_listener (proxy (source), reinterpret_cast<void (**) (void)> (const_cast<wl_data_source_listener*> (listener)), data);
}

uint32_t getDataSourceVersion (wl_data_source* source)
{
    return getVersion (proxy (source));
}

void wlDataSourceOffer (wl_data_source* source, const char* mimeType)
{
    WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (source), toUnderlyingType (WlDataSourceRequest::offer), nullptr, getVersion (proxy (source)), 0, mimeType);
}

void wlDataSourceSetActions (wl_data_source* source, uint32_t actions)
{
    const auto version = getVersion (proxy (source));

    if (version < dataDeviceActionNegotiationVersion)
        return;

    WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (source), toUnderlyingType (WlDataSourceRequest::setActions), nullptr, version, 0, actions);
}

void wlDataSourceDestroy (wl_data_source* source)
{
    WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (source), toUnderlyingType (WlDataSourceRequest::destroy), nullptr, getVersion (proxy (source)), destroyFlag);
}

int wlDataOfferAddListener (wl_data_offer* offer, const wl_data_offer_listener* listener, void* data)
{
    return WaylandClientSymbols::getInstance()->wl_proxy_add_listener (proxy (offer), reinterpret_cast<void (**) (void)> (const_cast<wl_data_offer_listener*> (listener)), data);
}

uint32_t getDataOfferVersion (wl_data_offer* offer)
{
    return getVersion (proxy (offer));
}

void wlDataOfferAccept (wl_data_offer* offer, uint32_t serial, const char* mimeType)
{
    WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (offer), toUnderlyingType (WlDataOfferRequest::accept), nullptr, getVersion (proxy (offer)), 0, serial, mimeType);
}

void wlDataOfferReceive (wl_data_offer* offer, const char* mimeType, int fd)
{
    WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (offer), toUnderlyingType (WlDataOfferRequest::receive), nullptr, getVersion (proxy (offer)), 0, mimeType, fd);
}

bool wlDataOfferFinish (wl_data_offer* offer)
{
    const auto version = getVersion (proxy (offer));

    if (version < dataDeviceActionNegotiationVersion)
        return false;

    WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (offer), toUnderlyingType (WlDataOfferRequest::finish), nullptr, version, 0);
    return true;
}

bool wlDataOfferSetActions (wl_data_offer* offer, uint32_t actions, uint32_t preferredAction)
{
    const auto version = getVersion (proxy (offer));

    if (version < dataDeviceActionNegotiationVersion)
        return false;

    WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (offer), toUnderlyingType (WlDataOfferRequest::setActions), nullptr, version, 0, actions, preferredAction);
    return true;
}

void wlDataOfferDestroy (wl_data_offer* offer)
{
    WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (offer), toUnderlyingType (WlDataOfferRequest::destroy), nullptr, getVersion (proxy (offer)), destroyFlag);
}

int wlDataDeviceAddListener (wl_data_device* device, const wl_data_device_listener* listener, void* data)
{
    return WaylandClientSymbols::getInstance()->wl_proxy_add_listener (proxy (device), reinterpret_cast<void (**) (void)> (const_cast<wl_data_device_listener*> (listener)), data);
}

void wlDataDeviceStartDrag (wl_data_device* device, wl_data_source* source, wl_surface* origin,
                            wl_surface* icon, uint32_t serial)
{
    WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (device), toUnderlyingType (WlDataDeviceRequest::startDrag), nullptr, getVersion (proxy (device)), 0, source, origin, icon, serial);
}

void wlDataDeviceSetSelection (wl_data_device* device, wl_data_source* source, uint32_t serial)
{
    WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (device), toUnderlyingType (WlDataDeviceRequest::setSelection), nullptr, getVersion (proxy (device)), 0, source, serial);
}

void destroyDataDevice (wl_data_device* device)
{
    const auto version = getVersion (proxy (device));

    // wl_data_device.release was added in version 2. Older proxies must be destroyed locally.
    if (version < 2)
    {
        WaylandClientSymbols::getInstance()->wl_proxy_destroy (proxy (device));
        return;
    }

    WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (device), toUnderlyingType (WlDataDeviceRequest::release), nullptr, version, destroyFlag);
}

wl_output* bindOutput (wl_registry* registry, uint32_t name, uint32_t version)
{
    return bindGlobal<wl_output> (registry, name, version,
                                  *WaylandClientSymbols::getInstance()->wl_output_interface,
                                  SupportedProtocolVersions::output);
}

uint32_t getOutputVersion (wl_output* output)
{
    return getVersion (proxy (output));
}

int wlOutputAddListener (wl_output* output, const wl_output_listener* listener, void* data)
{
    return WaylandClientSymbols::getInstance()->wl_proxy_add_listener (proxy (output), reinterpret_cast<void (**) (void)> (const_cast<wl_output_listener*> (listener)), data);
}

void destroyOutput (wl_output* output)
{
    const auto version = getVersion (proxy (output));

    // wl_output.release exists only from version 3.
    if (version >= 3)
        WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (output), toUnderlyingType (WlOutputRequest::release), nullptr, version, destroyFlag);
    else
        WaylandClientSymbols::getInstance()->wl_proxy_destroy (proxy (output));
}

zxdg_output_manager_v1* bindZxdgOutputManagerV1 (wl_registry* registry, uint32_t name, uint32_t version)
{
    return bindGlobal<zxdg_output_manager_v1> (registry, name, version, zxdgOutputManagerV1Interface);
}

void zxdgOutputManagerV1Destroy (zxdg_output_manager_v1* manager)
{
    WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (manager), toUnderlyingType (ZxdgOutputManagerV1Request::destroy), nullptr, getVersion (proxy (manager)), destroyFlag);
}

zxdg_output_v1* zxdgOutputManagerV1GetXdgOutput (zxdg_output_manager_v1* manager, wl_output* output)
{
    return object<zxdg_output_v1> (WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (manager),
                                                                                                toUnderlyingType (ZxdgOutputManagerV1Request::getXdgOutput),
                                                                                                &zxdgOutputV1Interface,
                                                                                                getVersion (proxy (manager)),
                                                                                                0,
                                                                                                nullptr,
                                                                                                output));
}

uint32_t getXdgOutputVersion (zxdg_output_v1* output)
{
    return getVersion (proxy (output));
}

int zxdgOutputV1AddListener (zxdg_output_v1* output, const zxdg_output_v1_listener* listener, void* data)
{
    return WaylandClientSymbols::getInstance()->wl_proxy_add_listener (proxy (output), reinterpret_cast<void (**) (void)> (const_cast<zxdg_output_v1_listener*> (listener)), data);
}

void zxdgOutputV1Destroy (zxdg_output_v1* output)
{
    WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (output), toUnderlyingType (ZxdgOutputV1Request::destroy), nullptr, getVersion (proxy (output)), destroyFlag);
}

wl_seat* bindSeat (wl_registry* registry, uint32_t name, uint32_t version)
{
    return bindGlobal<wl_seat> (registry, name, version,
                                *WaylandClientSymbols::getInstance()->wl_seat_interface,
                                SupportedProtocolVersions::seat);
}

int wlSeatAddListener (wl_seat* seat, const wl_seat_listener* listener, void* data)
{
    return WaylandClientSymbols::getInstance()->wl_proxy_add_listener (proxy (seat), reinterpret_cast<void (**) (void)> (const_cast<wl_seat_listener*> (listener)), data);
}

wl_pointer* wlSeatGetPointer (wl_seat* seat)
{
    return object<wl_pointer> (WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (seat),
                                                                                            toUnderlyingType (WlSeatRequest::getPointer),
                                                                                            WaylandClientSymbols::getInstance()->wl_pointer_interface,
                                                                                            getVersion (proxy (seat)),
                                                                                            0,
                                                                                            nullptr));
}

wl_keyboard* wlSeatGetKeyboard (wl_seat* seat)
{
    return object<wl_keyboard> (WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (seat),
                                                                                             toUnderlyingType (WlSeatRequest::getKeyboard),
                                                                                             WaylandClientSymbols::getInstance()->wl_keyboard_interface,
                                                                                             getVersion (proxy (seat)),
                                                                                             0,
                                                                                             nullptr));
}

wl_touch* wlSeatGetTouch (wl_seat* seat)
{
    return object<wl_touch> (WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (seat),
                                                                                          toUnderlyingType (WlSeatRequest::getTouch),
                                                                                          WaylandClientSymbols::getInstance()->wl_touch_interface,
                                                                                          getVersion (proxy (seat)),
                                                                                          0,
                                                                                          nullptr));
}

void destroySeat (wl_seat* seat)
{
    const auto version = getVersion (proxy (seat));

    // wl_seat.release exists only from version 5.
    if (version >= 5)
        WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (seat), toUnderlyingType (WlSeatRequest::release), nullptr, version, destroyFlag);
    else
        WaylandClientSymbols::getInstance()->wl_proxy_destroy (proxy (seat));
}

void wlPointerSetCursor (wl_pointer* pointer, uint32_t serial, wl_surface* surface, int32_t hotspotX, int32_t hotspotY)
{
    WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (pointer), toUnderlyingType (WlPointerRequest::setCursor), nullptr, getVersion (proxy (pointer)), 0, serial, surface, hotspotX, hotspotY);
}

int wlPointerAddListener (wl_pointer* pointer, const wl_pointer_listener* listener, void* data)
{
    return WaylandClientSymbols::getInstance()->wl_proxy_add_listener (proxy (pointer), reinterpret_cast<void (**) (void)> (const_cast<wl_pointer_listener*> (listener)), data);
}

void destroyPointer (wl_pointer* pointer)
{
    const auto version = getVersion (proxy (pointer));

    // wl_pointer.release exists only from version 3.
    if (version >= 3)
        WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (pointer), toUnderlyingType (WlPointerRequest::release), nullptr, version, destroyFlag);
    else
        WaylandClientSymbols::getInstance()->wl_proxy_destroy (proxy (pointer));
}

int wlKeyboardAddListener (wl_keyboard* keyboard, const wl_keyboard_listener* listener, void* data)
{
    return WaylandClientSymbols::getInstance()->wl_proxy_add_listener (proxy (keyboard), reinterpret_cast<void (**) (void)> (const_cast<wl_keyboard_listener*> (listener)), data);
}

void destroyKeyboard (wl_keyboard* keyboard)
{
    const auto version = getVersion (proxy (keyboard));

    // wl_keyboard.release exists only from version 3.
    if (version >= 3)
        WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (keyboard), toUnderlyingType (WlKeyboardRequest::release), nullptr, version, destroyFlag);
    else
        WaylandClientSymbols::getInstance()->wl_proxy_destroy (proxy (keyboard));
}

zwp_text_input_manager_v3* bindTextInputManagerV3 (wl_registry* registry, uint32_t name, uint32_t version)
{
    return bindGlobal<zwp_text_input_manager_v3> (registry, name, version, zwpTextInputManagerV3Interface);
}

void zwpTextInputManagerV3Destroy (zwp_text_input_manager_v3* manager)
{
    WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (manager), toUnderlyingType (ZwpTextInputManagerV3Request::destroy), nullptr, getVersion (proxy (manager)), destroyFlag);
}

zwp_text_input_v3* zwpTextInputManagerV3GetTextInput (zwp_text_input_manager_v3* manager, wl_seat* seat)
{
    return object<zwp_text_input_v3> (WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (manager),
                                                                                                   toUnderlyingType (ZwpTextInputManagerV3Request::getTextInput),
                                                                                                   &zwpTextInputV3Interface,
                                                                                                   getVersion (proxy (manager)),
                                                                                                   0,
                                                                                                   nullptr,
                                                                                                   seat));
}

int zwpTextInputV3AddListener (zwp_text_input_v3* textInput, const zwp_text_input_v3_listener* listener, void* data)
{
    return WaylandClientSymbols::getInstance()->wl_proxy_add_listener (proxy (textInput), reinterpret_cast<void (**) (void)> (const_cast<zwp_text_input_v3_listener*> (listener)), data);
}

void zwpTextInputV3Destroy (zwp_text_input_v3* textInput)
{
    WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (textInput), toUnderlyingType (ZwpTextInputV3Request::destroy), nullptr, getVersion (proxy (textInput)), destroyFlag);
}

void zwpTextInputV3Enable (zwp_text_input_v3* textInput)
{
    WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (textInput), toUnderlyingType (ZwpTextInputV3Request::enable), nullptr, getVersion (proxy (textInput)), 0);
}

void zwpTextInputV3Disable (zwp_text_input_v3* textInput)
{
    WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (textInput), toUnderlyingType (ZwpTextInputV3Request::disable), nullptr, getVersion (proxy (textInput)), 0);
}

void zwpTextInputV3SetSurroundingText (zwp_text_input_v3* textInput, const char* text, int32_t cursor, int32_t anchor)
{
    WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (textInput), toUnderlyingType (ZwpTextInputV3Request::setSurroundingText), nullptr, getVersion (proxy (textInput)), 0, text, cursor, anchor);
}

void zwpTextInputV3SetTextChangeCause (zwp_text_input_v3* textInput, uint32_t cause)
{
    WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (textInput), toUnderlyingType (ZwpTextInputV3Request::setTextChangeCause), nullptr, getVersion (proxy (textInput)), 0, cause);
}

void zwpTextInputV3SetContentType (zwp_text_input_v3* textInput, uint32_t hint, uint32_t purpose)
{
    WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (textInput), toUnderlyingType (ZwpTextInputV3Request::setContentType), nullptr, getVersion (proxy (textInput)), 0, hint, purpose);
}

void zwpTextInputV3SetCursorRectangle (zwp_text_input_v3* textInput, int32_t x, int32_t y, int32_t width, int32_t height)
{
    WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (textInput), toUnderlyingType (ZwpTextInputV3Request::setCursorRectangle), nullptr, getVersion (proxy (textInput)), 0, x, y, width, height);
}

void zwpTextInputV3Commit (zwp_text_input_v3* textInput)
{
    WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (textInput), toUnderlyingType (ZwpTextInputV3Request::commit), nullptr, getVersion (proxy (textInput)), 0);
}

int wlTouchAddListener (wl_touch* touch, const wl_touch_listener* listener, void* data)
{
    return WaylandClientSymbols::getInstance()->wl_proxy_add_listener (proxy (touch), reinterpret_cast<void (**) (void)> (const_cast<wl_touch_listener*> (listener)), data);
}

void destroyTouch (wl_touch* touch)
{
    const auto version = getVersion (proxy (touch));

    // wl_touch.release exists only from version 3.
    if (version >= 3)
        WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (touch), toUnderlyingType (WlTouchRequest::release), nullptr, version, destroyFlag);
    else
        WaylandClientSymbols::getInstance()->wl_proxy_destroy (proxy (touch));
}

xdg_wm_base* bindXdgWmBase (wl_registry* registry, uint32_t name, uint32_t version)
{
    return bindGlobal<xdg_wm_base> (registry, name, version, xdgWmBaseInterface);
}

int xdgWmBaseAddListener (xdg_wm_base* wmBase, const xdg_wm_base_listener* listener, void* data)
{
    return WaylandClientSymbols::getInstance()->wl_proxy_add_listener (proxy (wmBase), reinterpret_cast<void (**) (void)> (const_cast<xdg_wm_base_listener*> (listener)), data);
}

void xdgWmBasePong (xdg_wm_base* wmBase, uint32_t serial)
{
    WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (wmBase), toUnderlyingType (XdgWmBaseRequest::pong), nullptr, getVersion (proxy (wmBase)), 0, serial);
}

xdg_positioner* xdgWmBaseCreatePositioner (xdg_wm_base* wmBase)
{
    return object<xdg_positioner> (WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (wmBase),
                                                                                                toUnderlyingType (XdgWmBaseRequest::createPositioner),
                                                                                                &xdgPositionerInterface,
                                                                                                getVersion (proxy (wmBase)),
                                                                                                0,
                                                                                                nullptr));
}

void xdgWmBaseDestroy (xdg_wm_base* wmBase)
{
    WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (wmBase), toUnderlyingType (XdgWmBaseRequest::destroy), nullptr, getVersion (proxy (wmBase)), destroyFlag);
}

xdg_surface* xdgWmBaseGetXdgSurface (xdg_wm_base* wmBase, wl_surface* surface)
{
    return object<xdg_surface> (WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (wmBase),
                                                                                             toUnderlyingType (XdgWmBaseRequest::getXdgSurface),
                                                                                             &xdgSurfaceInterface,
                                                                                             getVersion (proxy (wmBase)),
                                                                                             0,
                                                                                             nullptr,
                                                                                             surface));
}

int xdgSurfaceAddListener (xdg_surface* surface, const xdg_surface_listener* listener, void* data)
{
    return WaylandClientSymbols::getInstance()->wl_proxy_add_listener (proxy (surface), reinterpret_cast<void (**) (void)> (const_cast<xdg_surface_listener*> (listener)), data);
}

xdg_toplevel* xdgSurfaceGetToplevel (xdg_surface* surface)
{
    return object<xdg_toplevel> (WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (surface),
                                                                                              toUnderlyingType (XdgSurfaceRequest::getToplevel),
                                                                                              &xdgToplevelInterface,
                                                                                              getVersion (proxy (surface)),
                                                                                              0,
                                                                                              nullptr));
}

xdg_popup* xdgSurfaceGetPopup (xdg_surface* surface, xdg_surface* parent, xdg_positioner* positioner)
{
    return object<xdg_popup> (WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (surface),
                                                                                           toUnderlyingType (XdgSurfaceRequest::getPopup),
                                                                                           &xdgPopupInterface,
                                                                                           getVersion (proxy (surface)),
                                                                                           0,
                                                                                           nullptr,
                                                                                           parent,
                                                                                           positioner));
}

void xdgSurfaceAckConfigure (xdg_surface* surface, uint32_t serial)
{
    WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (surface), toUnderlyingType (XdgSurfaceRequest::ackConfigure), nullptr, getVersion (proxy (surface)), 0, serial);
}

void xdgSurfaceDestroy (xdg_surface* surface)
{
    WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (surface), toUnderlyingType (XdgSurfaceRequest::destroy), nullptr, getVersion (proxy (surface)), destroyFlag);
}

void xdgPositionerSetSize (xdg_positioner* positioner, int32_t width, int32_t height)
{
    WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (positioner), toUnderlyingType (XdgPositionerRequest::setSize), nullptr, getVersion (proxy (positioner)), 0, width, height);
}

void xdgPositionerSetAnchorRect (xdg_positioner* positioner, int32_t x, int32_t y, int32_t width, int32_t height)
{
    WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (positioner), toUnderlyingType (XdgPositionerRequest::setAnchorRect), nullptr, getVersion (proxy (positioner)), 0, x, y, width, height);
}

void xdgPositionerSetAnchor (xdg_positioner* positioner, uint32_t anchor)
{
    WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (positioner), toUnderlyingType (XdgPositionerRequest::setAnchor), nullptr, getVersion (proxy (positioner)), 0, anchor);
}

void xdgPositionerSetGravity (xdg_positioner* positioner, uint32_t gravity)
{
    WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (positioner), toUnderlyingType (XdgPositionerRequest::setGravity), nullptr, getVersion (proxy (positioner)), 0, gravity);
}

void xdgPositionerSetConstraintAdjustment (xdg_positioner* positioner, uint32_t adjustment)
{
    WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (positioner), toUnderlyingType (XdgPositionerRequest::setConstraintAdjustment), nullptr, getVersion (proxy (positioner)), 0, adjustment);
}

void xdgPositionerSetOffset (xdg_positioner* positioner, int32_t x, int32_t y)
{
    WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (positioner), toUnderlyingType (XdgPositionerRequest::setOffset), nullptr, getVersion (proxy (positioner)), 0, x, y);
}

void xdgPositionerDestroy (xdg_positioner* positioner)
{
    WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (positioner), toUnderlyingType (XdgPositionerRequest::destroy), nullptr, getVersion (proxy (positioner)), destroyFlag);
}

int xdgPopupAddListener (xdg_popup* popup, const xdg_popup_listener* listener, void* data)
{
    return WaylandClientSymbols::getInstance()->wl_proxy_add_listener (proxy (popup), reinterpret_cast<void (**) (void)> (const_cast<xdg_popup_listener*> (listener)), data);
}

void xdgPopupGrab (xdg_popup* popup, wl_seat* seat, uint32_t serial)
{
    WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (popup), toUnderlyingType (XdgPopupRequest::grab), nullptr, getVersion (proxy (popup)), 0, seat, serial);
}

bool xdgPopupReposition (xdg_popup* popup, xdg_positioner* positioner, uint32_t token)
{
    const auto version = getVersion (proxy (popup));

    if (version < 3)
        return false;

    WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (popup), toUnderlyingType (XdgPopupRequest::reposition), nullptr, version, 0, positioner, token);
    return true;
}

void xdgPopupDestroy (xdg_popup* popup)
{
    WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (popup), toUnderlyingType (XdgPopupRequest::destroy), nullptr, getVersion (proxy (popup)), destroyFlag);
}

int xdgToplevelAddListener (xdg_toplevel* toplevel, const xdg_toplevel_listener* listener, void* data)
{
    return WaylandClientSymbols::getInstance()->wl_proxy_add_listener (proxy (toplevel), reinterpret_cast<void (**) (void)> (const_cast<xdg_toplevel_listener*> (listener)), data);
}

void xdgToplevelSetParent (xdg_toplevel* toplevel, xdg_toplevel* parent)
{
    WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (toplevel), toUnderlyingType (XdgToplevelRequest::setParent), nullptr, getVersion (proxy (toplevel)), 0, parent);
}

void xdgToplevelSetTitle (xdg_toplevel* toplevel, const char* title)
{
    WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (toplevel), toUnderlyingType (XdgToplevelRequest::setTitle), nullptr, getVersion (proxy (toplevel)), 0, title);
}

void xdgToplevelSetAppId (xdg_toplevel* toplevel, const char* appId)
{
    WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (toplevel), toUnderlyingType (XdgToplevelRequest::setAppId), nullptr, getVersion (proxy (toplevel)), 0, appId);
}

void xdgToplevelMove (xdg_toplevel* toplevel, wl_seat* seat, uint32_t serial)
{
    WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (toplevel), toUnderlyingType (XdgToplevelRequest::move), nullptr, getVersion (proxy (toplevel)), 0, seat, serial);
}

void xdgToplevelResize (xdg_toplevel* toplevel, wl_seat* seat, uint32_t serial, uint32_t resizeEdge)
{
    WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (toplevel), toUnderlyingType (XdgToplevelRequest::resize), nullptr, getVersion (proxy (toplevel)), 0, seat, serial, resizeEdge);
}

void xdgToplevelSetMaxSize (xdg_toplevel* toplevel, int32_t width, int32_t height)
{
    WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (toplevel), toUnderlyingType (XdgToplevelRequest::setMaxSize), nullptr, getVersion (proxy (toplevel)), 0, width, height);
}

void xdgToplevelSetMinSize (xdg_toplevel* toplevel, int32_t width, int32_t height)
{
    WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (toplevel), toUnderlyingType (XdgToplevelRequest::setMinSize), nullptr, getVersion (proxy (toplevel)), 0, width, height);
}

void xdgToplevelDestroy (xdg_toplevel* toplevel)
{
    WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (toplevel), toUnderlyingType (XdgToplevelRequest::destroy), nullptr, getVersion (proxy (toplevel)), destroyFlag);
}

void xdgToplevelSetMinimized (xdg_toplevel* toplevel)
{
    WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (toplevel), toUnderlyingType (XdgToplevelRequest::setMinimized), nullptr, getVersion (proxy (toplevel)), 0);
}

void xdgToplevelSetFullscreen (xdg_toplevel* toplevel)
{
    WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (toplevel), toUnderlyingType (XdgToplevelRequest::setFullscreen), nullptr, getVersion (proxy (toplevel)), 0, (wl_output*) nullptr);
}

void xdgToplevelUnsetFullscreen (xdg_toplevel* toplevel)
{
    WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (toplevel), toUnderlyingType (XdgToplevelRequest::unsetFullscreen), nullptr, getVersion (proxy (toplevel)), 0);
}

xdg_activation_v1* bindXdgActivationV1 (wl_registry* registry, uint32_t name, uint32_t version)
{
    return bindGlobal<xdg_activation_v1> (registry, name, version, xdgActivationV1Interface);
}

void xdgActivationV1Destroy (xdg_activation_v1* activation)
{
    WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (activation), toUnderlyingType (XdgActivationV1Request::destroy), nullptr, getVersion (proxy (activation)), destroyFlag);
}

xdg_activation_token_v1* xdgActivationV1GetActivationToken (xdg_activation_v1* activation)
{
    return object<xdg_activation_token_v1> (WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (activation),
                                                                                                         toUnderlyingType (XdgActivationV1Request::getActivationToken),
                                                                                                         &xdgActivationTokenV1Interface,
                                                                                                         getVersion (proxy (activation)),
                                                                                                         0,
                                                                                                         nullptr));
}

void xdgActivationV1Activate (xdg_activation_v1* activation, const char* token, wl_surface* surface)
{
    WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (activation), toUnderlyingType (XdgActivationV1Request::activate), nullptr, getVersion (proxy (activation)), 0, token, surface);
}

int xdgActivationTokenV1AddListener (xdg_activation_token_v1* token, const xdg_activation_token_v1_listener* listener, void* data)
{
    return WaylandClientSymbols::getInstance()->wl_proxy_add_listener (proxy (token), reinterpret_cast<void (**) (void)> (const_cast<xdg_activation_token_v1_listener*> (listener)), data);
}

void xdgActivationTokenV1SetSerial (xdg_activation_token_v1* token, uint32_t serial, wl_seat* seat)
{
    WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (token), toUnderlyingType (XdgActivationTokenV1Request::setSerial), nullptr, getVersion (proxy (token)), 0, serial, seat);
}

void xdgActivationTokenV1SetSurface (xdg_activation_token_v1* token, wl_surface* surface)
{
    WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (token), toUnderlyingType (XdgActivationTokenV1Request::setSurface), nullptr, getVersion (proxy (token)), 0, surface);
}

void xdgActivationTokenV1Commit (xdg_activation_token_v1* token)
{
    WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (token), toUnderlyingType (XdgActivationTokenV1Request::commit), nullptr, getVersion (proxy (token)), 0);
}

void xdgActivationTokenV1Destroy (xdg_activation_token_v1* token)
{
    WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (token), toUnderlyingType (XdgActivationTokenV1Request::destroy), nullptr, getVersion (proxy (token)), destroyFlag);
}

zxdg_decoration_manager_v1* bindZxdgDecorationManagerV1 (wl_registry* registry, uint32_t name, uint32_t version)
{
    return bindGlobal<zxdg_decoration_manager_v1> (registry, name, version, zxdgDecorationManagerV1Interface);
}

void zxdgDecorationManagerV1Destroy (zxdg_decoration_manager_v1* manager)
{
    WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (manager), toUnderlyingType (ZxdgDecorationManagerV1Request::destroy), nullptr, getVersion (proxy (manager)), destroyFlag);
}

zxdg_toplevel_decoration_v1* zxdgDecorationManagerV1GetToplevelDecoration (zxdg_decoration_manager_v1* manager, xdg_toplevel* toplevel)
{
    return object<zxdg_toplevel_decoration_v1> (WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (manager),
                                                                                                             toUnderlyingType (ZxdgDecorationManagerV1Request::getToplevelDecoration),
                                                                                                             &zxdgToplevelDecorationV1Interface,
                                                                                                             getVersion (proxy (manager)),
                                                                                                             0,
                                                                                                             nullptr,
                                                                                                             toplevel));
}

int zxdgToplevelDecorationV1AddListener (zxdg_toplevel_decoration_v1* decoration, const zxdg_toplevel_decoration_v1_listener* listener, void* data)
{
    return WaylandClientSymbols::getInstance()->wl_proxy_add_listener (proxy (decoration), reinterpret_cast<void (**) (void)> (const_cast<zxdg_toplevel_decoration_v1_listener*> (listener)), data);
}

void zxdgToplevelDecorationV1SetMode (zxdg_toplevel_decoration_v1* decoration, uint32_t mode)
{
    WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (decoration), toUnderlyingType (ZxdgToplevelDecorationV1Request::setMode), nullptr, getVersion (proxy (decoration)), 0, mode);
}

void zxdgToplevelDecorationV1Destroy (zxdg_toplevel_decoration_v1* decoration)
{
    WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (decoration), toUnderlyingType (ZxdgToplevelDecorationV1Request::destroy), nullptr, getVersion (proxy (decoration)), destroyFlag);
}

wp_viewporter* bindWpViewporter (wl_registry* registry, uint32_t name, uint32_t version)
{
    return bindGlobal<wp_viewporter> (registry, name, version, wpViewporterInterface);
}

void wpViewporterDestroy (wp_viewporter* viewporter)
{
    WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (viewporter), toUnderlyingType (WpViewporterRequest::destroy), nullptr, getVersion (proxy (viewporter)), destroyFlag);
}

wp_viewport* wpViewporterGetViewport (wp_viewporter* viewporter, wl_surface* surface)
{
    return object<wp_viewport> (WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (viewporter),
                                                                                             toUnderlyingType (WpViewporterRequest::getViewport),
                                                                                             &wpViewportInterface,
                                                                                             getVersion (proxy (viewporter)),
                                                                                             0,
                                                                                             nullptr,
                                                                                             surface));
}

void wpViewportSetSource (wp_viewport* viewport, int32_t x, int32_t y, int32_t width, int32_t height)
{
    WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (viewport),
                                                                 toUnderlyingType (WpViewportRequest::setSource),
                                                                 nullptr,
                                                                 getVersion (proxy (viewport)),
                                                                 0,
                                                                 x * wlFixedScale,
                                                                 y * wlFixedScale,
                                                                 width * wlFixedScale,
                                                                 height * wlFixedScale);
}

void wpViewportSetDestination (wp_viewport* viewport, int32_t width, int32_t height)
{
    WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (viewport), toUnderlyingType (WpViewportRequest::setDestination), nullptr, getVersion (proxy (viewport)), 0, width, height);
}

void wpViewportDestroy (wp_viewport* viewport)
{
    WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (viewport), toUnderlyingType (WpViewportRequest::destroy), nullptr, getVersion (proxy (viewport)), destroyFlag);
}

wp_fractional_scale_manager_v1* bindWpFractionalScaleManagerV1 (wl_registry* registry, uint32_t name, uint32_t version)
{
    return bindGlobal<wp_fractional_scale_manager_v1> (registry, name, version, wpFractionalScaleManagerV1Interface);
}

void wpFractionalScaleManagerV1Destroy (wp_fractional_scale_manager_v1* manager)
{
    WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (manager), toUnderlyingType (WpFractionalScaleManagerV1Request::destroy), nullptr, getVersion (proxy (manager)), destroyFlag);
}

wp_fractional_scale_v1* wpFractionalScaleManagerV1GetFractionalScale (wp_fractional_scale_manager_v1* manager, wl_surface* surface)
{
    return object<wp_fractional_scale_v1> (WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (manager),
                                                                                                        toUnderlyingType (WpFractionalScaleManagerV1Request::getFractionalScale),
                                                                                                        &wpFractionalScaleV1Interface,
                                                                                                        getVersion (proxy (manager)),
                                                                                                        0,
                                                                                                        nullptr,
                                                                                                        surface));
}

int wpFractionalScaleV1AddListener (wp_fractional_scale_v1* fractionalScale, const wp_fractional_scale_v1_listener* listener, void* data)
{
    return WaylandClientSymbols::getInstance()->wl_proxy_add_listener (proxy (fractionalScale), reinterpret_cast<void (**) (void)> (const_cast<wp_fractional_scale_v1_listener*> (listener)), data);
}

void wpFractionalScaleV1Destroy (wp_fractional_scale_v1* fractionalScale)
{
    WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (fractionalScale), toUnderlyingType (WpFractionalScaleV1Request::destroy), nullptr, getVersion (proxy (fractionalScale)), destroyFlag);
}

wp_alpha_modifier_v1* bindWpAlphaModifierV1 (wl_registry* registry, uint32_t name, uint32_t version)
{
    return bindGlobal<wp_alpha_modifier_v1> (registry, name, version, wpAlphaModifierV1Interface);
}

void wpAlphaModifierV1Destroy (wp_alpha_modifier_v1* modifier)
{
    WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (modifier), toUnderlyingType (WpAlphaModifierV1Request::destroy), nullptr, getVersion (proxy (modifier)), destroyFlag);
}

wp_alpha_modifier_surface_v1* wpAlphaModifierV1GetSurface (wp_alpha_modifier_v1* modifier, wl_surface* surface)
{
    return object<wp_alpha_modifier_surface_v1> (WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (modifier),
                                                                                                              toUnderlyingType (WpAlphaModifierV1Request::getSurface),
                                                                                                              &wpAlphaModifierSurfaceV1Interface,
                                                                                                              getVersion (proxy (modifier)),
                                                                                                              0,
                                                                                                              nullptr,
                                                                                                              surface));
}

void wpAlphaModifierSurfaceV1Destroy (wp_alpha_modifier_surface_v1* modifierSurface)
{
    WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (modifierSurface), toUnderlyingType (WpAlphaModifierSurfaceV1Request::destroy), nullptr, getVersion (proxy (modifierSurface)), destroyFlag);
}

void wpAlphaModifierSurfaceV1SetMultiplier (wp_alpha_modifier_surface_v1* modifierSurface, uint32_t factor)
{
    WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (modifierSurface), toUnderlyingType (WpAlphaModifierSurfaceV1Request::setMultiplier), nullptr, getVersion (proxy (modifierSurface)), 0, factor);
}

zxdg_exporter_v2* bindZxdgExporterV2 (wl_registry* registry, uint32_t name, uint32_t version)
{
    return bindGlobal<zxdg_exporter_v2> (registry, name, version, zxdgExporterV2Interface);
}

void zxdgExporterV2Destroy (zxdg_exporter_v2* exporter)
{
    WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (exporter), toUnderlyingType (ZxdgExporterV2Request::destroy), nullptr, getVersion (proxy (exporter)), destroyFlag);
}

zxdg_exported_v2* zxdgExporterV2ExportToplevel (zxdg_exporter_v2* exporter, wl_surface* surface)
{
    return object<zxdg_exported_v2> (WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (exporter),
                                                                                                  toUnderlyingType (ZxdgExporterV2Request::exportToplevel),
                                                                                                  &zxdgExportedV2Interface,
                                                                                                  getVersion (proxy (exporter)),
                                                                                                  0,
                                                                                                  nullptr,
                                                                                                  surface));
}

int zxdgExportedV2AddListener (zxdg_exported_v2* exported, const zxdg_exported_v2_listener* listener, void* data)
{
    return WaylandClientSymbols::getInstance()->wl_proxy_add_listener (proxy (exported), reinterpret_cast<void (**) (void)> (const_cast<zxdg_exported_v2_listener*> (listener)), data);
}

void zxdgExportedV2Destroy (zxdg_exported_v2* exported)
{
    WaylandClientSymbols::getInstance()->wl_proxy_marshal_flags (proxy (exported), toUnderlyingType (ZxdgExportedV2Request::destroy), nullptr, getVersion (proxy (exported)), destroyFlag);
}

#if JUCE_UNIT_TESTS

class WaylandProtocolBindVersionTests final : public UnitTest
{
public:
    WaylandProtocolBindVersionTests()
        : UnitTest ("WaylandProtocol bind version", UnitTestCategories::gui) {}

    void runTest() override
    {
        testCase ("The compositor's advertised version limits a bind", [&]
        {
            wl_interface interface{};
            interface.version = 4;
            expectEquals ((int) getBindVersion (2, interface, 3), 2);
        });

        testCase ("The data-device manager stays within the version JUCE supports", [&]
        {
            wl_interface interface{};
            interface.version = 4;
            expectEquals ((int) getBindVersion (4, interface, SupportedProtocolVersions::dataDeviceManager), 3);
        });

        testCase ("The loaded libwayland interface version limits a newer advertised global", [&]
        {
            wl_interface interface{};
            interface.version = 2;
            expectEquals ((int) getBindVersion (4, interface, 3), 2);
        });
    }
};

static WaylandProtocolBindVersionTests waylandProtocolBindVersionTests;

#endif

} // namespace juce::WaylandProtocol
