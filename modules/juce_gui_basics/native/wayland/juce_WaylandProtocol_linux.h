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

struct wl_buffer;
struct wl_callback;
struct wl_compositor;
struct wl_data_device;
struct wl_data_device_manager;
struct wl_data_offer;
struct wl_data_source;
struct wl_display;
struct wl_keyboard;
struct wl_output;
struct wl_pointer;
struct wl_proxy;
struct wl_region;
struct wl_registry;
struct wl_seat;
struct wl_shm;
struct wl_shm_pool;
struct wl_subcompositor;
struct wl_subsurface;
struct wl_surface;
struct wl_touch;

struct wp_viewporter;
struct wp_viewport;
struct wp_fractional_scale_manager_v1;
struct wp_fractional_scale_v1;
struct wp_alpha_modifier_v1;
struct wp_alpha_modifier_surface_v1;

struct xdg_wm_base;
struct xdg_popup;
struct xdg_positioner;
struct xdg_surface;
struct xdg_toplevel;
struct xdg_activation_v1;
struct xdg_activation_token_v1;

struct zxdg_decoration_manager_v1;
struct zxdg_toplevel_decoration_v1;
struct zxdg_exporter_v2;
struct zxdg_exported_v2;
struct zxdg_output_manager_v1;
struct zwp_text_input_manager_v3;
struct zwp_text_input_v3;
struct zxdg_output_v1;

// Full definitions for the types forward-declared below live in juce_WaylandProtocolTypes_linux.h,
// kept off this public header so they can't clash with a real <wayland-client.h>.
struct wl_interface;

struct wl_registry_listener;
struct wl_surface_listener;
struct wl_output_listener;
struct wl_callback_listener;
struct wl_buffer_listener;
struct wl_data_device_listener;
struct wl_data_offer_listener;
struct wl_data_source_listener;
struct wl_seat_listener;
struct wl_pointer_listener;
struct wl_keyboard_listener;
struct wl_touch_listener;
struct xdg_wm_base_listener;
struct xdg_popup_listener;
struct xdg_surface_listener;
struct xdg_toplevel_listener;
struct xdg_activation_token_v1_listener;
struct zxdg_toplevel_decoration_v1_listener;
struct zxdg_exported_v2_listener;
struct zwp_text_input_v3_listener;
struct zxdg_output_v1_listener;
struct wp_fractional_scale_v1_listener;

namespace juce
{
class WaylandClientSymbols;
} // namespace juce

namespace juce::WaylandProtocol
{
// Values mirror the protocol enums in wayland.xml, xdg-shell.xml and xdg-decoration-unstable-v1.xml.
enum : uint32_t
{
    wlOutputModeCurrent = 0x1
};

// Mirrors xdg_positioner.anchor.
enum : uint32_t
{
    xdgPositionerAnchorNone        = 0,
    xdgPositionerAnchorTop         = 1,
    xdgPositionerAnchorBottom      = 2,
    xdgPositionerAnchorLeft        = 3,
    xdgPositionerAnchorRight       = 4,
    xdgPositionerAnchorTopLeft     = 5,
    xdgPositionerAnchorBottomLeft  = 6,
    xdgPositionerAnchorTopRight    = 7,
    xdgPositionerAnchorBottomRight = 8
};

// Mirrors xdg_positioner.gravity.
enum : uint32_t
{
    xdgPositionerGravityNone        = 0,
    xdgPositionerGravityTop         = 1,
    xdgPositionerGravityBottom      = 2,
    xdgPositionerGravityLeft        = 3,
    xdgPositionerGravityRight       = 4,
    xdgPositionerGravityTopLeft     = 5,
    xdgPositionerGravityBottomLeft  = 6,
    xdgPositionerGravityTopRight    = 7,
    xdgPositionerGravityBottomRight = 8
};

// Mirrors xdg_positioner.constraint_adjustment.
enum : uint32_t
{
    xdgPositionerConstraintAdjustmentSlideX  = 1 << 0,
    xdgPositionerConstraintAdjustmentSlideY  = 1 << 1,
    xdgPositionerConstraintAdjustmentFlipX   = 1 << 2,
    xdgPositionerConstraintAdjustmentFlipY   = 1 << 3,
    xdgPositionerConstraintAdjustmentResizeX = 1 << 4,
    xdgPositionerConstraintAdjustmentResizeY = 1 << 5
};

// Mirrors wl_output.transform.
enum : uint32_t
{
    wlOutputTransformNormal = 0,
    wlOutputTransform90 = 1,
    wlOutputTransform180 = 2,
    wlOutputTransform270 = 3,
    wlOutputTransformFlipped = 4,
    wlOutputTransformFlipped90 = 5,
    wlOutputTransformFlipped180 = 6,
    wlOutputTransformFlipped270 = 7
};

enum : uint32_t
{
    wlShmFormatARGB8888 = 0,
    wlShmFormatXRGB8888 = 1
};

enum : uint32_t
{
    xdgToplevelStateMaximized = 1,
    xdgToplevelStateFullscreen = 2,
    xdgToplevelStateResizing = 3,
    xdgToplevelStateActivated = 4,
    xdgToplevelStateTiledLeft = 5,
    xdgToplevelStateTiledRight = 6,
    xdgToplevelStateTiledTop = 7,
    xdgToplevelStateTiledBottom = 8,
    xdgToplevelStateSuspended = 9
};

// Mirrors enum xdg_toplevel_resize_edge.
enum : uint32_t
{
    xdgToplevelResizeEdgeNone = 0,
    xdgToplevelResizeEdgeTop = 1,
    xdgToplevelResizeEdgeBottom = 2,
    xdgToplevelResizeEdgeLeft = 4,
    xdgToplevelResizeEdgeTopLeft = 5,
    xdgToplevelResizeEdgeBottomLeft = 6,
    xdgToplevelResizeEdgeRight = 8,
    xdgToplevelResizeEdgeTopRight = 9,
    xdgToplevelResizeEdgeBottomRight = 10
};

enum : uint32_t
{
    zxdgToplevelDecorationV1ModeClientSide = 1,
    zxdgToplevelDecorationV1ModeServerSide = 2
};

enum : uint32_t
{
    wlSeatCapabilityPointer = 1,
    wlSeatCapabilityKeyboard = 2,
    wlSeatCapabilityTouch = 4
};

enum : uint32_t
{
    wlPointerButtonStateReleased = 0,
    wlPointerButtonStatePressed = 1
};

enum : uint32_t
{
    wlPointerAxisVerticalScroll = 0,
    wlPointerAxisHorizontalScroll = 1
};

// Mirrors wl_pointer.axis_source.
enum : uint32_t
{
    wlPointerAxisSourceWheel = 0,
    wlPointerAxisSourceFinger = 1,
    wlPointerAxisSourceContinuous = 2,
    wlPointerAxisSourceWheelTilt = 3
};

// Mirrors wl_pointer.axis_relative_direction.
enum : uint32_t
{
    wlPointerAxisRelativeDirectionIdentical = 0,
    wlPointerAxisRelativeDirectionInverted = 1
};

enum : uint32_t
{
    wlKeyboardKeyStateReleased = 0,
    wlKeyboardKeyStatePressed = 1,
    wlKeyboardKeyStateRepeated = 2
};

enum : uint32_t
{
    wlKeyboardKeymapFormatXkbV1 = 1
};

// Mirrors wl_data_device_manager.dnd_action.
enum : uint32_t
{
    wlDataDeviceManagerDndActionNone = 0,
    wlDataDeviceManagerDndActionCopy = 1,
    wlDataDeviceManagerDndActionMove = 2,
    wlDataDeviceManagerDndActionAsk = 4
};

constexpr uint32_t dataDeviceActionNegotiationVersion = 3;

enum : uint32_t
{
    zwpTextInputV3ChangeCauseInputMethod = 0,
    zwpTextInputV3ChangeCauseOther = 1
};

enum : uint32_t
{
    zwpTextInputV3ContentHintNone = 0,
    zwpTextInputV3ContentHintSensitiveData = 0x80
};

enum : uint32_t
{
    zwpTextInputV3ContentPurposeNormal = 0,
    zwpTextInputV3ContentPurposeDigits = 2,
    zwpTextInputV3ContentPurposeNumber = 3,
    zwpTextInputV3ContentPurposePhone = 4,
    zwpTextInputV3ContentPurposeUrl = 5,
    zwpTextInputV3ContentPurposeEmail = 6,
    zwpTextInputV3ContentPurposePassword = 8
};

constexpr uint32_t xdgToplevelStateMask (uint32_t state) { return 1u << (state - 1); }

Point<float> fixedToPoint (int32_t x, int32_t y);
void initialiseInterfaces (const WaylandClientSymbols&);

wl_registry* wlDisplayGetRegistry (wl_display*);
wl_callback* wlDisplaySync (wl_display*);

int wlRegistryAddListener (wl_registry*, const wl_registry_listener*, void*);
void wlRegistryDestroy (wl_registry*);

wl_compositor* bindCompositor (wl_registry*, uint32_t name, uint32_t version);
wl_surface* wlCompositorCreateSurface (wl_compositor*);
wl_region* wlCompositorCreateRegion (wl_compositor*);
void wlCompositorDestroy (wl_compositor*);

wl_subcompositor* bindSubcompositor (wl_registry*, uint32_t name, uint32_t version);
wl_subsurface* wlSubcompositorGetSubsurface (wl_subcompositor*, wl_surface*, wl_surface* parent);
void wlSubcompositorDestroy (wl_subcompositor*);

void wlSubsurfaceDestroy (wl_subsurface*);
void wlSubsurfaceSetPosition (wl_subsurface*, int32_t x, int32_t y);
void wlSubsurfaceSetSync (wl_subsurface*);
void wlSubsurfaceSetDesync (wl_subsurface*);

int wlSurfaceAddListener (wl_surface*, const wl_surface_listener*, void*);
void wlSurfaceDestroy (wl_surface*);
void wlSurfaceAttach (wl_surface*, wl_buffer*, int32_t x, int32_t y);
void wlSurfaceDamageBuffer (wl_surface*, int32_t x, int32_t y, int32_t width, int32_t height);
wl_callback* wlSurfaceFrame (wl_surface*);
// Passing a null region tells the compositor that no part of the surface is opaque.
void wlSurfaceSetOpaqueRegion (wl_surface*, wl_region*);
// Passing an empty region makes the surface ignore input.
void wlSurfaceSetInputRegion (wl_surface*, wl_region*);
void wlSurfaceSetBufferScale (wl_surface*, int32_t scale);
void wlSurfaceCommit (wl_surface*);

void wlRegionAdd (wl_region*, int32_t x, int32_t y, int32_t width, int32_t height);
void wlRegionDestroy (wl_region*);

int wlCallbackAddListener (wl_callback*, const wl_callback_listener*, void*);
void wlCallbackDestroy (wl_callback*);

wl_shm* bindShm (wl_registry*, uint32_t name, uint32_t version);
wl_shm_pool* wlShmCreatePool (wl_shm*, int fd, int32_t size);
wl_buffer* wlShmPoolCreateBuffer (wl_shm_pool*, int32_t offset, int32_t width, int32_t height,
                                  int32_t stride, uint32_t format);
void wlShmDestroy (wl_shm*);
void wlShmPoolDestroy (wl_shm_pool*);

int wlBufferAddListener (wl_buffer*, const wl_buffer_listener*, void*);
void wlBufferDestroy (wl_buffer*);

wl_data_device_manager* bindDataDeviceManager (wl_registry*, uint32_t name, uint32_t version);
wl_data_source* wlDataDeviceManagerCreateDataSource (wl_data_device_manager*);
wl_data_device* wlDataDeviceManagerGetDataDevice (wl_data_device_manager*, wl_seat*);
void destroyDataDeviceManager (wl_data_device_manager*);

int wlDataSourceAddListener (wl_data_source*, const wl_data_source_listener*, void*);
uint32_t getDataSourceVersion (wl_data_source*);
void wlDataSourceOffer (wl_data_source*, const char* mimeType);
void wlDataSourceSetActions (wl_data_source*, uint32_t actions);
void wlDataSourceDestroy (wl_data_source*);

int wlDataOfferAddListener (wl_data_offer*, const wl_data_offer_listener*, void*);
uint32_t getDataOfferVersion (wl_data_offer*);
void wlDataOfferAccept (wl_data_offer*, uint32_t serial, const char* mimeType);
void wlDataOfferReceive (wl_data_offer*, const char* mimeType, int fd);
bool wlDataOfferFinish (wl_data_offer*);
bool wlDataOfferSetActions (wl_data_offer*, uint32_t actions, uint32_t preferredAction);
void wlDataOfferDestroy (wl_data_offer*);

int wlDataDeviceAddListener (wl_data_device*, const wl_data_device_listener*, void*);
void wlDataDeviceStartDrag (wl_data_device*, wl_data_source*, wl_surface* origin, wl_surface* icon,
                            uint32_t serial);
void wlDataDeviceSetSelection (wl_data_device*, wl_data_source*, uint32_t serial);
void destroyDataDevice (wl_data_device*);

wl_output* bindOutput (wl_registry*, uint32_t name, uint32_t version);
uint32_t getOutputVersion (wl_output*);
int wlOutputAddListener (wl_output*, const wl_output_listener*, void*);
void destroyOutput (wl_output*);

zxdg_output_manager_v1* bindZxdgOutputManagerV1 (wl_registry*, uint32_t name, uint32_t version);
void zxdgOutputManagerV1Destroy (zxdg_output_manager_v1*);
zxdg_output_v1* zxdgOutputManagerV1GetXdgOutput (zxdg_output_manager_v1*, wl_output*);
uint32_t getXdgOutputVersion (zxdg_output_v1*);
int zxdgOutputV1AddListener (zxdg_output_v1*, const zxdg_output_v1_listener*, void*);
void zxdgOutputV1Destroy (zxdg_output_v1*);

wl_seat* bindSeat (wl_registry*, uint32_t name, uint32_t version);
int wlSeatAddListener (wl_seat*, const wl_seat_listener*, void*);
wl_pointer* wlSeatGetPointer (wl_seat*);
wl_keyboard* wlSeatGetKeyboard (wl_seat*);
wl_touch* wlSeatGetTouch (wl_seat*);
void destroySeat (wl_seat*);

void wlPointerSetCursor (wl_pointer*, uint32_t serial, wl_surface*, int32_t hotspotX, int32_t hotspotY);
int wlPointerAddListener (wl_pointer*, const wl_pointer_listener*, void*);
void destroyPointer (wl_pointer*);

int wlKeyboardAddListener (wl_keyboard*, const wl_keyboard_listener*, void*);
void destroyKeyboard (wl_keyboard*);

zwp_text_input_manager_v3* bindTextInputManagerV3 (wl_registry*, uint32_t name, uint32_t version);
void zwpTextInputManagerV3Destroy (zwp_text_input_manager_v3*);
zwp_text_input_v3* zwpTextInputManagerV3GetTextInput (zwp_text_input_manager_v3*, wl_seat*);

int zwpTextInputV3AddListener (zwp_text_input_v3*, const zwp_text_input_v3_listener*, void*);
void zwpTextInputV3Destroy (zwp_text_input_v3*);
void zwpTextInputV3Enable (zwp_text_input_v3*);
void zwpTextInputV3Disable (zwp_text_input_v3*);
void zwpTextInputV3SetSurroundingText (zwp_text_input_v3*, const char* text, int32_t cursor, int32_t anchor);
void zwpTextInputV3SetTextChangeCause (zwp_text_input_v3*, uint32_t cause);
void zwpTextInputV3SetContentType (zwp_text_input_v3*, uint32_t hint, uint32_t purpose);
void zwpTextInputV3SetCursorRectangle (zwp_text_input_v3*, int32_t x, int32_t y, int32_t width, int32_t height);
void zwpTextInputV3Commit (zwp_text_input_v3*);

int wlTouchAddListener (wl_touch*, const wl_touch_listener*, void*);
void destroyTouch (wl_touch*);

xdg_wm_base* bindXdgWmBase (wl_registry*, uint32_t name, uint32_t version);
int xdgWmBaseAddListener (xdg_wm_base*, const xdg_wm_base_listener*, void*);
void xdgWmBasePong (xdg_wm_base*, uint32_t serial);
xdg_positioner* xdgWmBaseCreatePositioner (xdg_wm_base*);
void xdgWmBaseDestroy (xdg_wm_base*);

xdg_surface* xdgWmBaseGetXdgSurface (xdg_wm_base*, wl_surface*);
int xdgSurfaceAddListener (xdg_surface*, const xdg_surface_listener*, void*);
xdg_toplevel* xdgSurfaceGetToplevel (xdg_surface*);
xdg_popup* xdgSurfaceGetPopup (xdg_surface*, xdg_surface* parent, xdg_positioner*);
void xdgSurfaceAckConfigure (xdg_surface*, uint32_t serial);
void xdgSurfaceDestroy (xdg_surface*);

void xdgPositionerSetSize (xdg_positioner*, int32_t width, int32_t height);
void xdgPositionerSetAnchorRect (xdg_positioner*, int32_t x, int32_t y, int32_t width, int32_t height);
void xdgPositionerSetAnchor (xdg_positioner*, uint32_t anchor);
void xdgPositionerSetGravity (xdg_positioner*, uint32_t gravity);
void xdgPositionerSetConstraintAdjustment (xdg_positioner*, uint32_t adjustment);
void xdgPositionerSetOffset (xdg_positioner*, int32_t x, int32_t y);
void xdgPositionerDestroy (xdg_positioner*);

int xdgPopupAddListener (xdg_popup*, const xdg_popup_listener*, void*);
void xdgPopupGrab (xdg_popup*, wl_seat*, uint32_t serial);
bool xdgPopupReposition (xdg_popup*, xdg_positioner*, uint32_t token);
void xdgPopupDestroy (xdg_popup*);

int xdgToplevelAddListener (xdg_toplevel*, const xdg_toplevel_listener*, void*);
void xdgToplevelSetParent (xdg_toplevel*, xdg_toplevel* parent);
void xdgToplevelSetTitle (xdg_toplevel*, const char*);
void xdgToplevelSetAppId (xdg_toplevel*, const char*);
void xdgToplevelMove (xdg_toplevel*, wl_seat*, uint32_t serial);
void xdgToplevelResize (xdg_toplevel*, wl_seat*, uint32_t serial, uint32_t resizeEdge);
void xdgToplevelSetMaxSize (xdg_toplevel*, int32_t width, int32_t height);
void xdgToplevelSetMinSize (xdg_toplevel*, int32_t width, int32_t height);
void xdgToplevelDestroy (xdg_toplevel*);
void xdgToplevelSetMinimized (xdg_toplevel*);
void xdgToplevelSetFullscreen (xdg_toplevel*);
void xdgToplevelUnsetFullscreen (xdg_toplevel*);

xdg_activation_v1* bindXdgActivationV1 (wl_registry*, uint32_t name, uint32_t version);
void xdgActivationV1Destroy (xdg_activation_v1*);
xdg_activation_token_v1* xdgActivationV1GetActivationToken (xdg_activation_v1*);
void xdgActivationV1Activate (xdg_activation_v1*, const char* token, wl_surface*);
int xdgActivationTokenV1AddListener (xdg_activation_token_v1*, const xdg_activation_token_v1_listener*, void*);
void xdgActivationTokenV1SetSerial (xdg_activation_token_v1*, uint32_t serial, wl_seat*);
void xdgActivationTokenV1SetSurface (xdg_activation_token_v1*, wl_surface*);
void xdgActivationTokenV1Commit (xdg_activation_token_v1*);
void xdgActivationTokenV1Destroy (xdg_activation_token_v1*);

zxdg_decoration_manager_v1* bindZxdgDecorationManagerV1 (wl_registry*, uint32_t name, uint32_t version);
void zxdgDecorationManagerV1Destroy (zxdg_decoration_manager_v1*);
zxdg_toplevel_decoration_v1* zxdgDecorationManagerV1GetToplevelDecoration (zxdg_decoration_manager_v1*, xdg_toplevel*);
int zxdgToplevelDecorationV1AddListener (zxdg_toplevel_decoration_v1*, const zxdg_toplevel_decoration_v1_listener*, void*);
void zxdgToplevelDecorationV1SetMode (zxdg_toplevel_decoration_v1*, uint32_t mode);
void zxdgToplevelDecorationV1Destroy (zxdg_toplevel_decoration_v1*);

wp_viewporter* bindWpViewporter (wl_registry*, uint32_t name, uint32_t version);
void wpViewporterDestroy (wp_viewporter*);
wp_viewport* wpViewporterGetViewport (wp_viewporter*, wl_surface*);
void wpViewportSetSource (wp_viewport*, int32_t x, int32_t y, int32_t width, int32_t height);
void wpViewportSetDestination (wp_viewport*, int32_t width, int32_t height);
void wpViewportDestroy (wp_viewport*);

wp_fractional_scale_manager_v1* bindWpFractionalScaleManagerV1 (wl_registry*, uint32_t name, uint32_t version);
void wpFractionalScaleManagerV1Destroy (wp_fractional_scale_manager_v1*);
wp_fractional_scale_v1* wpFractionalScaleManagerV1GetFractionalScale (wp_fractional_scale_manager_v1*, wl_surface*);
int wpFractionalScaleV1AddListener (wp_fractional_scale_v1*, const wp_fractional_scale_v1_listener*, void*);
void wpFractionalScaleV1Destroy (wp_fractional_scale_v1*);

wp_alpha_modifier_v1* bindWpAlphaModifierV1 (wl_registry*, uint32_t name, uint32_t version);
void wpAlphaModifierV1Destroy (wp_alpha_modifier_v1*);
wp_alpha_modifier_surface_v1* wpAlphaModifierV1GetSurface (wp_alpha_modifier_v1*, wl_surface*);
void wpAlphaModifierSurfaceV1Destroy (wp_alpha_modifier_surface_v1*);
// The factor spans the full uint32_t range, where 0 is fully transparent and 0xffffffff is opaque.
void wpAlphaModifierSurfaceV1SetMultiplier (wp_alpha_modifier_surface_v1*, uint32_t factor);

zxdg_exporter_v2* bindZxdgExporterV2 (wl_registry*, uint32_t name, uint32_t version);
void zxdgExporterV2Destroy (zxdg_exporter_v2*);
zxdg_exported_v2* zxdgExporterV2ExportToplevel (zxdg_exporter_v2*, wl_surface*);
int zxdgExportedV2AddListener (zxdg_exported_v2*, const zxdg_exported_v2_listener*, void*);
void zxdgExportedV2Destroy (zxdg_exported_v2*);
} // namespace juce::WaylandProtocol
