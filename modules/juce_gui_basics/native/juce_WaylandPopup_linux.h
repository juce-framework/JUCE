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

// Grabbed popups join the menu chain. Ungrabbed popups use its toplevel as their parent.
enum class WaylandPopupGrab { no, yes };

// Hands one input event's serial to at most one claimant, so two popups created
// from the same event cannot both request a grab with it.
class WaylandPopupGrabSerial final
{
public:
    explicit WaylandPopupGrabSerial (uint32_t serialIn) : serial (serialIn) {}

    bool isAvailable() const noexcept { return serial.has_value(); }
    std::optional<uint32_t> claim()   { return std::exchange (serial, std::nullopt); }

private:
    std::optional<uint32_t> serial;
};

struct WaylandPopupTrigger
{
    wl_surface* surface = nullptr;
    std::shared_ptr<WaylandPopupGrabSerial> grabSerial;
};

struct WaylandPopupParentCoordinates
{
    Rectangle<int> logicalBounds;
    double logicalToSurfaceScale = 1.0;

    Point<int> logicalToSurface (Point<int> position) const
    {
        return { roundToInt (position.x * logicalToSurfaceScale),
                 roundToInt (position.y * logicalToSurfaceScale) };
    }

    Point<int> surfaceToLogical (Point<int> position) const
    {
        return { roundToInt (position.x / logicalToSurfaceScale),
                 roundToInt (position.y / logicalToSurfaceScale) };
    }

    Point<int> getSurfaceSize() const
    {
        return logicalToSurface ({ logicalBounds.getWidth(), logicalBounds.getHeight() });
    }
};

struct WaylandPopupParentCandidate
{
    struct MappedPopup
    {
        // May be null if this peer outlives the component that would be dismissed.
        Component* componentToDismiss = nullptr;
        std::optional<uint32_t> grabSerial;

        // Teardown follows this link to dismiss descendant popups before their parent.
        xdg_surface* parentXdgSurface = nullptr;
    };

    wl_surface* parentSurface = nullptr;
    xdg_surface* parentXdgSurface = nullptr;

    // xdg_popup coordinates are relative to window geometry, including decorations.
    WaylandPopupParentCoordinates parentCoordinates;

    std::optional<MappedPopup> mappedPopup;
};

struct WaylandPopupParentListener
{
    virtual ~WaylandPopupParentListener() = default;

    // Toplevels return their own surface. Popups return their retained toplevel, and
    // subsurfaces follow their parent. This is available before mapping and is null
    // when no live toplevel is known.
    virtual wl_surface* getToplevelSurface() const = 0;

    // No value while the listener has no mapped xdg_surface a popup could parent to.
    virtual std::optional<WaylandPopupParentCandidate> getPopupParentCandidate() const = 0;

    // A child popup reports its grab directly to the window that owns the decorations.
    // This lets libdecor close all nested popups after title-bar interaction. Only a
    // listener with a toplevel acts on these notifications.
    virtual void popupGrabStarted (Component& componentToDismiss) = 0;
    virtual void popupGrabEnded() = 0;

    // Removes the popup role immediately, then dismisses its JUCE component after the
    // ancestor has finished unmapping.
    virtual void dismissPopupForAncestorUnmap() = 0;
};

struct WaylandPopupContext
{
    struct Surface
    {
        wl_surface* surface = nullptr;
        WaylandPopupParentListener* listener = nullptr;
    };

    std::vector<Surface> surfaces;

    // Prefer the current input event, then an available grab trigger. Without either, keyboard focus
    // takes priority over pointer focus so popups belong to the focused window.
    wl_surface* inputSurface = nullptr;

    // Keep the input event's surface with its serial so grab requests stay within that toplevel.
    std::optional<WaylandPopupTrigger> trigger;

    // Hover popups use the pointer's surface so a reused tooltip follows the hovered window.
    wl_surface* pointerSurface = nullptr;
};

struct WaylandPopupOwner
{
    wl_surface* toplevelSurface = nullptr;
};

// Protocol objects and grab serials are resolved again whenever the popup maps.
struct WaylandPopupParent
{
    wl_surface* parentSurface = nullptr;
    xdg_surface* parentXdgSurface = nullptr;
    WaylandPopupParentCoordinates parentCoordinates;
    std::shared_ptr<WaylandPopupGrabSerial> grabSerial;
    Component* componentToDismiss = nullptr;
};

struct WaylandPopupPlacement
{
    Rectangle<int> anchorRectangle;
    Point<int> popupSize;
    Point<int> offset;
    uint32_t anchor = WaylandProtocol::xdgPositionerAnchorTopLeft;
    uint32_t gravity = WaylandProtocol::xdgPositionerGravityBottomRight;
    uint32_t constraintAdjustment = WaylandProtocol::xdgPositionerConstraintAdjustmentSlideX
                                  | WaylandProtocol::xdgPositionerConstraintAdjustmentSlideY
                                  | WaylandProtocol::xdgPositionerConstraintAdjustmentFlipX
                                  | WaylandProtocol::xdgPositionerConstraintAdjustmentFlipY;
};

WaylandPopupPlacement makeWaylandPopupPlacement (Rectangle<int> popupBounds,
                                                 const WaylandPopupParentCoordinates& parentCoordinates);
Rectangle<int> convertWaylandPopupConfigureToLogicalBounds (
    Rectangle<int> parentRelativeSurfaceBounds,
    const WaylandPopupParentCoordinates& parentCoordinates);
std::optional<WaylandPopupGrab> getWaylandPopupGrab (int styleFlags);
bool isWaylandHoverPopup (int styleFlags);
std::optional<WaylandPopupOwner> findWaylandPopupOwner (const WaylandPopupContext&, int styleFlags);
std::optional<WaylandPopupParent> findWaylandPopupParent (const WaylandPopupContext&,
                                                          wl_surface* toplevelSurface, WaylandPopupGrab);

// Returns mapped popup descendants in the order required for protocol-correct teardown.
std::vector<wl_surface*> findWaylandPopupDescendantsTopmostFirst (const WaylandPopupContext&,
                                                                  xdg_surface* ancestorXdgSurface);

// Ends the popup's modal state and hides it, matching what the compositor expects
// after it dismisses a popup. Safe if application code destroys the component.
void dismissWaylandPopup (Component& componentToDismiss);

class WaylandPopup final
{
public:
    struct Delegate
    {
        virtual ~Delegate() = default;

        virtual void popupConfigured (Rectangle<int> parentRelativeBounds) = 0;
        virtual void popupDismissed() = 0;
    };

    static std::unique_ptr<WaylandPopup> create (Delegate&,
                                                 wl_surface&,
                                                 xdg_surface& parent,
                                                 const WaylandPopupPlacement&,
                                                 std::optional<uint32_t> grabSerial);

    void show();
    bool reposition (const WaylandPopupPlacement&);
    xdg_surface* getXdgSurface() const noexcept { return xdgSurface.get(); }

private:
    WaylandPopup (Delegate&,
                  wl_surface&,
                  xdg_surface& parent,
                  const WaylandPopupPlacement&,
                  std::optional<uint32_t> grabSerial);

    void handlePopupConfigure (Rectangle<int> parentRelativeBounds);
    void handleSurfaceConfigure (uint32_t serial);

    static const xdg_surface_listener surfaceListener;
    static const xdg_popup_listener popupListener;

    using XdgSurfaceHandle = std::unique_ptr<xdg_surface,
                                             FunctionPointerDestructor<WaylandProtocol::xdgSurfaceDestroy>>;
    using XdgPopupHandle = std::unique_ptr<xdg_popup,
                                           FunctionPointerDestructor<WaylandProtocol::xdgPopupDestroy>>;

    Delegate& delegate;
    wl_surface& surface;
    Rectangle<int> requestedParentRelativeBounds;
    std::optional<Rectangle<int>> pendingConfiguredBounds;
    XdgSurfaceHandle xdgSurface;
    XdgPopupHandle xdgPopup;
    bool mapRequested = false;
    uint32_t nextRepositionToken = 1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WaylandPopup)
    JUCE_DECLARE_NON_MOVEABLE (WaylandPopup)
};

} // namespace juce
