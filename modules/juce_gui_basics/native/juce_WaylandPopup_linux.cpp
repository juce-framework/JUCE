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

WaylandPopupPlacement makeWaylandPopupPlacement (Rectangle<int> popupBounds,
                                                 const WaylandPopupParentCoordinates& parentCoordinates)
{
    const auto parentRelativePosition = parentCoordinates.logicalToSurface (
        popupBounds.getPosition() - parentCoordinates.logicalBounds.getPosition());
    const auto parentSurfaceSize = parentCoordinates.getSurfaceSize();
    const Point<int> anchorPosition
    {
        jlimit (0, jmax (0, parentSurfaceSize.x - 1), parentRelativePosition.x),
        jlimit (0, jmax (0, parentSurfaceSize.y - 1), parentRelativePosition.y)
    };

    // Positions use the parent surface's coordinate system. Popup dimensions use
    // the popup surface's coordinate system and must not inherit the parent's scale.
    return
    {
        { anchorPosition.x, anchorPosition.y, 1, 1 },
        { jmax (1, popupBounds.getWidth()), jmax (1, popupBounds.getHeight()) },
        parentRelativePosition - anchorPosition
    };
}

Rectangle<int> convertWaylandPopupConfigureToLogicalBounds (
    Rectangle<int> parentRelativeSurfaceBounds,
    const WaylandPopupParentCoordinates& parentCoordinates)
{
    return parentRelativeSurfaceBounds.withPosition (
        parentCoordinates.surfaceToLogical (parentRelativeSurfaceBounds.getPosition())
            + parentCoordinates.logicalBounds.getPosition());
}

using XdgPositionerHandle = std::unique_ptr<xdg_positioner,
                                            FunctionPointerDestructor<WaylandProtocol::xdgPositionerDestroy>>;

static XdgPositionerHandle createPositioner (xdg_wm_base& wmBase, const WaylandPopupPlacement& placement)
{
    XdgPositionerHandle positioner { WaylandProtocol::xdgWmBaseCreatePositioner (&wmBase) };

    if (positioner == nullptr)
        return {};

    const auto anchor = placement.anchorRectangle;
    WaylandProtocol::xdgPositionerSetSize (positioner.get(), placement.popupSize.x, placement.popupSize.y);
    WaylandProtocol::xdgPositionerSetAnchorRect (positioner.get(), anchor.getX(), anchor.getY(),
                                                 anchor.getWidth(), anchor.getHeight());
    WaylandProtocol::xdgPositionerSetAnchor (positioner.get(), placement.anchor);
    WaylandProtocol::xdgPositionerSetGravity (positioner.get(), placement.gravity);
    WaylandProtocol::xdgPositionerSetConstraintAdjustment (positioner.get(), placement.constraintAdjustment);
    WaylandProtocol::xdgPositionerSetOffset (positioner.get(), placement.offset.x, placement.offset.y);
    return positioner;
}

static Rectangle<int> getRequestedParentRelativeBounds (const WaylandPopupPlacement& placement)
{
    const auto position = placement.anchorRectangle.getPosition() + placement.offset;
    return { position.x, position.y, placement.popupSize.x, placement.popupSize.y };
}

std::optional<WaylandPopupGrab> getWaylandPopupGrab (int styleFlags)
{
    if ((styleFlags & ComponentPeer::windowIsTemporary) == 0)
        return std::nullopt;

    // JUCE menus ignore key presses, so these flags serve as a default grab policy.
    return (styleFlags & ComponentPeer::windowIgnoresMouseClicks) == 0
           && (styleFlags & ComponentPeer::windowIgnoresKeyPresses) != 0 ? WaylandPopupGrab::yes : WaylandPopupGrab::no;
}

bool isWaylandHoverPopup (int styleFlags)
{
    // Tooltips ignore both kinds of input. Drag images ignore only mouse input and keep their source owner.
    constexpr auto flags = ComponentPeer::windowIgnoresMouseClicks | ComponentPeer::windowIgnoresKeyPresses;
    return (styleFlags & flags) == flags;
}

static WaylandPopupParentListener* findPopupListener (const WaylandPopupContext& context, wl_surface* surface)
{
    if (surface != nullptr)
        for (const auto& entry : context.surfaces)
            if (entry.surface == surface)
                return entry.listener;

    return nullptr;
}

std::optional<WaylandPopupOwner> findWaylandPopupOwner (const WaylandPopupContext& context, int styleFlags)
{
    auto* inputSurface = isWaylandHoverPopup (styleFlags) && context.pointerSurface != nullptr ? context.pointerSurface
                                                                                            : context.inputSurface;
    auto* input = findPopupListener (context, inputSurface);
    auto* toplevel = input != nullptr ? input->getToplevelSurface() : nullptr;

    if (toplevel == nullptr)
        return std::nullopt;

    return WaylandPopupOwner { toplevel };
}

std::optional<WaylandPopupParent> findWaylandPopupParent (const WaylandPopupContext& context,
                                                          wl_surface* toplevelSurface, WaylandPopupGrab grab)
{
    if (toplevelSurface == nullptr)
        return std::nullopt;

    // Only use a grab serial from an input event belonging to this popup's owning toplevel.
    const auto triggerSerial = std::invoke ([&]() -> std::shared_ptr<WaylandPopupGrabSerial>
    {
        if (context.trigger.has_value())
            if (auto* trigger = findPopupListener (context, context.trigger->surface);
                trigger != nullptr && trigger->getToplevelSurface() == toplevelSurface)
                return context.trigger->grabSerial;

        return nullptr;
    });
    const auto makeParent = [] (const WaylandPopupParentCandidate& candidate,
                                std::shared_ptr<WaylandPopupGrabSerial> serial)
    {
        return WaylandPopupParent { candidate.parentSurface,
                                    candidate.parentXdgSurface,
                                    candidate.parentCoordinates,
                                    std::move (serial),
                                    candidate.mappedPopup.has_value() ? candidate.mappedPopup->componentToDismiss : nullptr };
    };

    if (grab == WaylandPopupGrab::yes)
    {
        for (auto it = context.surfaces.rbegin(); it != context.surfaces.rend(); ++it)
        {
            if (it->listener == nullptr || it->listener->getToplevelSurface() != toplevelSurface)
                continue;

            const auto candidate = it->listener->getPopupParentCandidate();

            if (! candidate.has_value() || ! candidate->mappedPopup.has_value())
                continue;

            // An explicit popup grab requires a toplevel or an explicitly grabbed popup parent.
            const auto serial = std::invoke ([&]() -> std::shared_ptr<WaylandPopupGrabSerial>
            {
                if (! candidate->mappedPopup->grabSerial.has_value())
                    return nullptr;

                if (triggerSerial != nullptr)
                    return triggerSerial;

                return std::make_shared<WaylandPopupGrabSerial> (*candidate->mappedPopup->grabSerial);
            });

            return makeParent (*candidate, serial);
        }
    }

    if (auto* toplevel = findPopupListener (context, toplevelSurface))
        if (const auto candidate = toplevel->getPopupParentCandidate(); candidate.has_value())
            return makeParent (*candidate, grab == WaylandPopupGrab::yes ? triggerSerial : nullptr);

    return std::nullopt;
}

std::vector<wl_surface*> findWaylandPopupDescendantsTopmostFirst (const WaylandPopupContext& context,
                                                                  xdg_surface* ancestorXdgSurface)
{
    struct Candidate
    {
        wl_surface* surface = nullptr;
        xdg_surface* xdgSurface = nullptr;
        xdg_surface* parentXdgSurface = nullptr;
    };

    std::vector<Candidate> remaining;

    for (const auto& entry : context.surfaces)
    {
        if (entry.listener == nullptr)
            continue;

        if (const auto candidate = entry.listener->getPopupParentCandidate();
            candidate.has_value()
            && candidate->mappedPopup.has_value()
            && candidate->mappedPopup->parentXdgSurface != nullptr)
        {
            remaining.push_back ({ entry.surface,
                                   candidate->parentXdgSurface,
                                   candidate->mappedPopup->parentXdgSurface });
        }
    }

    std::vector<Candidate> parentFirst;

    const auto isDescendantParent = [&] (xdg_surface* parent)
    {
        if (parent == ancestorXdgSurface)
            return true;

        return std::any_of (parentFirst.begin(), parentFirst.end(),
                            [parent] (const auto& candidate) { return candidate.xdgSurface == parent; });
    };

    // A popup may remap onto a peer created after it, so registration order alone cannot
    // establish the hierarchy. Repeated passes claim each level after its parent.
    for (bool claimed = true; claimed;)
    {
        claimed = false;

        for (auto it = remaining.begin(); it != remaining.end();)
        {
            if (isDescendantParent (it->parentXdgSurface))
            {
                parentFirst.push_back (*it);
                it = remaining.erase (it);
                claimed = true;
            }
            else
            {
                ++it;
            }
        }
    }

    std::vector<wl_surface*> result;
    result.reserve (parentFirst.size());

    for (auto it = parentFirst.rbegin(); it != parentFirst.rend(); ++it)
        result.push_back (it->surface);

    return result;
}

void dismissWaylandPopup (Component& componentToDismiss)
{
    const WeakReference<Component> deletionChecker { &componentToDismiss };
    componentToDismiss.exitModalState (0);

    if (deletionChecker != nullptr)
        deletionChecker->setVisible (false);
}

std::unique_ptr<WaylandPopup> WaylandPopup::create (Delegate& delegate,
                                                    wl_surface& surface,
                                                    xdg_surface& parent,
                                                    const WaylandPopupPlacement& placement,
                                                    std::optional<uint32_t> grabSerial)
{
    auto result = std::unique_ptr<WaylandPopup> { new WaylandPopup (delegate,
                                                                    surface,
                                                                    parent,
                                                                    placement,
                                                                    grabSerial) };

    if (result->xdgPopup != nullptr)
        return result;

    return nullptr;
}

WaylandPopup::WaylandPopup (Delegate& delegateIn,
                            wl_surface& surfaceIn,
                            xdg_surface& parent,
                            const WaylandPopupPlacement& placement,
                            std::optional<uint32_t> grabSerial)
    : delegate (delegateIn),
      surface (surfaceIn),
      requestedParentRelativeBounds (getRequestedParentRelativeBounds (placement))
{
    auto* windowSystem = WaylandWindowSystem::getInstance();
    auto* wmBase = windowSystem->getXdgWmBase();

    if (wmBase == nullptr)
        return;

    xdgSurface.reset (WaylandProtocol::xdgWmBaseGetXdgSurface (wmBase, &surface));

    if (xdgSurface == nullptr)
        return;

    const auto positioner = createPositioner (*wmBase, placement);

    if (positioner == nullptr)
        return;

    xdgPopup.reset (WaylandProtocol::xdgSurfaceGetPopup (xdgSurface.get(), &parent, positioner.get()));

    if (xdgPopup == nullptr)
        return;

    WaylandProtocol::xdgSurfaceAddListener (xdgSurface.get(), &surfaceListener, this);
    WaylandProtocol::xdgPopupAddListener (xdgPopup.get(), &popupListener, this);

    if (grabSerial.has_value())
        if (auto* seat = windowSystem->getSeat())
            WaylandProtocol::xdgPopupGrab (xdgPopup.get(), seat, *grabSerial);
}

void WaylandPopup::show()
{
    if (mapRequested)
        return;

    mapRequested = true;
    WaylandProtocol::wlSurfaceCommit (&surface);
    WaylandWindowSystem::getInstance()->flush();
}

bool WaylandPopup::reposition (const WaylandPopupPlacement& placement)
{
    if (! mapRequested || xdgPopup == nullptr)
        return false;

    auto* windowSystem = WaylandWindowSystem::getInstance();
    auto* wmBase = windowSystem->getXdgWmBase();

    if (wmBase == nullptr)
        return false;

    const auto positioner = createPositioner (*wmBase, placement);

    if (positioner == nullptr
        || ! WaylandProtocol::xdgPopupReposition (xdgPopup.get(), positioner.get(), nextRepositionToken))
    {
        return false;
    }

    ++nextRepositionToken;
    requestedParentRelativeBounds = getRequestedParentRelativeBounds (placement);
    windowSystem->flush();
    return true;
}

void WaylandPopup::handlePopupConfigure (Rectangle<int> parentRelativeBounds)
{
    pendingConfiguredBounds = parentRelativeBounds;
}

void WaylandPopup::handleSurfaceConfigure (uint32_t serial)
{
    const auto bounds = std::exchange (pendingConfiguredBounds, std::nullopt)
                            .value_or (requestedParentRelativeBounds);

    WaylandProtocol::xdgSurfaceAckConfigure (xdgSurface.get(), serial);

    delegate.popupConfigured (bounds);
}

const xdg_surface_listener WaylandPopup::surfaceListener
{
    [] (void* data, xdg_surface*, uint32_t serial)
    {
        static_cast<WaylandPopup*> (data)->handleSurfaceConfigure (serial);
    }
};

const xdg_popup_listener WaylandPopup::popupListener
{
    [] (void* data, xdg_popup*, int32_t x, int32_t y, int32_t width, int32_t height)
    {
        static_cast<WaylandPopup*> (data)->handlePopupConfigure ({ x, y, width, height });
    },
    [] (void* data, xdg_popup*)
    {
        static_cast<WaylandPopup*> (data)->delegate.popupDismissed();
    },
    // This event only echoes the reposition request token.
    // The next configure provides the chosen bounds and the serial.
    [] (void*, xdg_popup*, uint32_t) {}
};

#if JUCE_UNIT_TESTS

// GCC 16 reports a null destination for the initializer-list assignment of a vector at -O3,
// so the tests assign a constructed vector instead.
using PopupSurfaces = std::vector<WaylandPopupContext::Surface>;

namespace
{
    struct TestPopupParentListener final : WaylandPopupParentListener
    {
        wl_surface* getToplevelSurface() const override { return toplevelSurface; }

        std::optional<WaylandPopupParentCandidate> getPopupParentCandidate() const override
        {
            return candidate;
        }

        void popupGrabStarted (Component&) override {}
        void popupGrabEnded() override {}
        void dismissPopupForAncestorUnmap() override {}

        std::optional<WaylandPopupParentCandidate> candidate;
        wl_surface* toplevelSurface = nullptr;
    };

    template <typename Type>
    Type* fakePopupHandle (uintptr_t value)
    {
        return reinterpret_cast<Type*> (value);
    }
}

//==============================================================================
class WaylandPopupOwnerTests final : public UnitTest
{
public:
    WaylandPopupOwnerTests()
        : UnitTest ("WaylandPopupOwner", UnitTestCategories::gui) {}

    void runTest() override
    {
        testCase ("An unmapped input window supplies a toplevel owner", [&]
        {
            TestPopupParentListener window;
            auto* surface = fakePopupHandle<wl_surface> (1);
            window.toplevelSurface = surface;
            WaylandPopupContext context;
            context.surfaces = PopupSurfaces { { surface, &window } };
            context.inputSurface = surface;
            const auto owner = findWaylandPopupOwner (context, ComponentPeer::windowIsTemporary);
            expect (owner.has_value() && owner->toplevelSurface == surface);
        });

        testCase ("A tooltip selects the hovered window and a drag image selects the source window", [&]
        {
            TestPopupParentListener sourceWindow, hoveredWindow;
            auto* sourceSurface = fakePopupHandle<wl_surface> (1);
            auto* hoveredSurface = fakePopupHandle<wl_surface> (2);
            sourceWindow.toplevelSurface = sourceSurface;
            hoveredWindow.toplevelSurface = hoveredSurface;
            WaylandPopupContext context;
            context.surfaces = PopupSurfaces { { sourceSurface, &sourceWindow }, { hoveredSurface, &hoveredWindow } };
            context.inputSurface = sourceSurface;
            context.pointerSurface = hoveredSurface;
            constexpr auto dragFlags = ComponentPeer::windowIsTemporary | ComponentPeer::windowIgnoresMouseClicks;
            constexpr auto tooltipFlags = dragFlags | ComponentPeer::windowIgnoresKeyPresses;
            const auto tooltip = findWaylandPopupOwner (context, tooltipFlags);
            const auto dragImage = findWaylandPopupOwner (context, dragFlags);
            expect (tooltip.has_value() && tooltip->toplevelSurface == hoveredSurface);
            expect (dragImage.has_value() && dragImage->toplevelSurface == sourceSurface);
        });

        testCase ("A popup opened from a temporary window uses that window's toplevel", [&]
        {
            TestPopupParentListener helper;
            auto* helperSurface = fakePopupHandle<wl_surface> (1);
            auto* toplevelSurface = fakePopupHandle<wl_surface> (2);
            helper.toplevelSurface = toplevelSurface;
            WaylandPopupContext context;
            context.surfaces = PopupSurfaces { { helperSurface, &helper } };
            context.inputSurface = helperSurface;
            const auto owner = findWaylandPopupOwner (context, ComponentPeer::windowIsTemporary);
            expect (owner.has_value() && owner->toplevelSurface == toplevelSurface);
        });

        testCase ("No input surface gives no popup owner even when a mapped popup exists", [&]
        {
            TestPopupParentListener popup;
            auto* surface = fakePopupHandle<wl_surface> (1);
            popup.toplevelSurface = fakePopupHandle<wl_surface> (2);
            popup.candidate = WaylandPopupParentCandidate {
                surface, fakePopupHandle<xdg_surface> (3), {},
                WaylandPopupParentCandidate::MappedPopup { nullptr, 42 }
            };
            WaylandPopupContext context;
            context.surfaces = PopupSurfaces { { surface, &popup } };
            expect (! findWaylandPopupOwner (context, ComponentPeer::windowIsTemporary).has_value());
        });
    }
};

static WaylandPopupOwnerTests waylandPopupOwnerTests;

//==============================================================================
class WaylandPopupParentTests final : public UnitTest
{
public:
    WaylandPopupParentTests()
        : UnitTest ("WaylandPopupParent", UnitTestCategories::gui) {}

    void runTest() override
    {
        testCase ("The last registered mapped popup in the supplied toplevel becomes the parent", [&]
        {
            Component olderComponent;
            Component newerComponent;
            TestPopupParentListener olderPopup;
            TestPopupParentListener newerPopup;
            auto* olderPopupSurface = fakePopupHandle<wl_surface> (1);
            auto* newerPopupSurface = fakePopupHandle<wl_surface> (2);
            auto* olderXdgSurface = fakePopupHandle<xdg_surface> (3);
            auto* newerXdgSurface = fakePopupHandle<xdg_surface> (4);
            olderPopup.toplevelSurface = olderPopupSurface;
            olderPopup.candidate = WaylandPopupParentCandidate {
                olderPopupSurface,
                olderXdgSurface,
                {},
                WaylandPopupParentCandidate::MappedPopup { &olderComponent, std::nullopt }
            };
            newerPopup.toplevelSurface = olderPopupSurface;
            newerPopup.candidate = WaylandPopupParentCandidate {
                newerPopupSurface,
                newerXdgSurface,
                {},
                WaylandPopupParentCandidate::MappedPopup { &newerComponent, std::nullopt }
            };

            WaylandPopupContext context;
            context.surfaces = PopupSurfaces { { olderPopupSurface, &olderPopup }, { newerPopupSurface, &newerPopup } };
            const auto parent = findWaylandPopupParent (context, olderPopupSurface, WaylandPopupGrab::yes);

            expect (parent.has_value());

            if (! parent.has_value())
                return;

            expect (parent->parentSurface == newerPopupSurface);
            expect (parent->parentXdgSurface == newerXdgSurface);
            expect (parent->componentToDismiss == &newerComponent);
        });

        testCase ("A popup opened from a subsurface uses the registered toplevel", [&]
        {
            TestPopupParentListener listener, ancestor;
            auto* inputSurface = fakePopupHandle<wl_surface> (1);
            auto* ancestorSurface = fakePopupHandle<wl_surface> (2);
            auto* ancestorXdgSurface = fakePopupHandle<xdg_surface> (3);
            listener.toplevelSurface = ancestorSurface;
            listener.candidate = WaylandPopupParentCandidate {
                ancestorSurface,
                ancestorXdgSurface,
                {},
                std::nullopt
            };

            ancestor.toplevelSurface = ancestorSurface;
            ancestor.candidate = listener.candidate;
            WaylandPopupContext context;
            context.surfaces = PopupSurfaces { { inputSurface, &listener }, { ancestorSurface, &ancestor } };
            context.inputSurface = inputSurface;
            const auto owner = findWaylandPopupOwner (context, ComponentPeer::windowIsTemporary);
            expect (owner.has_value() && owner->toplevelSurface == ancestorSurface);
            const auto parent = owner.has_value() ? findWaylandPopupParent (context, owner->toplevelSurface, WaylandPopupGrab::yes)
                                                   : std::nullopt;

            expect (parent.has_value());

            if (! parent.has_value())
                return;

            expect (parent->parentSurface == ancestorSurface);
            expect (parent->parentXdgSurface == ancestorXdgSurface);
        });

        testCase ("Destroying the component to dismiss does not remove a mapped popup from parent selection", [&]
        {
            auto component = std::make_unique<Component>();
            WeakReference<Component> componentToDismiss { component.get() };
            expect (componentToDismiss != nullptr);
            TestPopupParentListener listener;
            auto* surface = fakePopupHandle<wl_surface> (1);
            auto* xdgSurface = fakePopupHandle<xdg_surface> (2);
            component.reset();
            expect (componentToDismiss == nullptr);
            listener.toplevelSurface = surface;
            listener.candidate = WaylandPopupParentCandidate {
                surface,
                xdgSurface,
                {},
                WaylandPopupParentCandidate::MappedPopup { componentToDismiss.get(), std::nullopt }
            };

            WaylandPopupContext context;
            context.surfaces = PopupSurfaces { { surface, &listener } };
            const auto parent = findWaylandPopupParent (context, surface, WaylandPopupGrab::yes);

            expect (parent.has_value());

            if (! parent.has_value())
                return;

            expect (parent->parentSurface == surface);
            expect (parent->parentXdgSurface == xdgSurface);
            expect (parent->componentToDismiss == nullptr);
        });

        testCase ("A child reuses its parent's explicit-grab serial when no trigger is available", [&]
        {
            Component componentToDismiss;
            TestPopupParentListener listener;
            auto* surface = fakePopupHandle<wl_surface> (1);
            listener.toplevelSurface = surface;
            listener.candidate = WaylandPopupParentCandidate {
                surface,
                fakePopupHandle<xdg_surface> (2),
                {},
                WaylandPopupParentCandidate::MappedPopup { &componentToDismiss, 42 }
            };

            WaylandPopupContext context;
            context.surfaces = PopupSurfaces { { surface, &listener } };
            const auto parent = findWaylandPopupParent (context, surface, WaylandPopupGrab::yes);

            expect (parent.has_value());

            if (! parent.has_value())
                return;

            expect (parent->grabSerial != nullptr);

            if (parent->grabSerial != nullptr)
                expect (parent->grabSerial->claim() == std::optional<uint32_t> { 42 });
        });

        testCase ("A child of an ungrabbed popup has no grab serial", [&]
        {
            Component componentToDismiss;
            TestPopupParentListener listener;
            auto* surface = fakePopupHandle<wl_surface> (1);
            listener.toplevelSurface = surface;
            listener.candidate = WaylandPopupParentCandidate {
                surface,
                fakePopupHandle<xdg_surface> (2),
                {},
                WaylandPopupParentCandidate::MappedPopup { &componentToDismiss, std::nullopt }
            };

            WaylandPopupContext context;
            context.surfaces = PopupSurfaces { { surface, &listener } };
            const auto parent = findWaylandPopupParent (context, surface, WaylandPopupGrab::yes);

            expect (parent.has_value());

            if (! parent.has_value())
                return;

            expect (parent->grabSerial == nullptr);
        });

        testCase ("A submenu ignores a newer tooltip when choosing its parent", [&]
        {
            Component componentToDismiss;
            TestPopupParentListener menuListener;
            TestPopupParentListener tooltipListener;
            auto* menuSurface = fakePopupHandle<wl_surface> (1);
            auto* menuXdgSurface = fakePopupHandle<xdg_surface> (2);
            auto* tooltipSurface = fakePopupHandle<wl_surface> (3);

            // The mapped menu is available as the submenu's parent.
            menuListener.toplevelSurface = menuSurface;
            menuListener.candidate = WaylandPopupParentCandidate {
                menuSurface,
                menuXdgSurface,
                {},
                WaylandPopupParentCandidate::MappedPopup { &componentToDismiss, std::nullopt }
            };

            tooltipListener.toplevelSurface = menuSurface;
            WaylandPopupContext context;
            context.surfaces = PopupSurfaces { { menuSurface, &menuListener }, { tooltipSurface, &tooltipListener } };

            // Parent selection ignores the tooltip and finds the menu.
            const auto parent = findWaylandPopupParent (context, menuSurface, WaylandPopupGrab::yes);

            expect (parent.has_value());

            if (! parent.has_value())
                return;

            expect (parent->parentSurface == menuSurface);
            expect (parent->parentXdgSurface == menuXdgSurface);
        });

        testCase ("A popup without a grab is parented to the toplevel even when a menu is open", [&]
        {
            Component componentToDismiss;
            TestPopupParentListener windowListener;
            TestPopupParentListener menuListener;
            auto* windowSurface = fakePopupHandle<wl_surface> (1);
            auto* windowXdgSurface = fakePopupHandle<xdg_surface> (2);
            auto* menuSurface = fakePopupHandle<wl_surface> (3);
            windowListener.toplevelSurface = windowSurface;
            windowListener.candidate = WaylandPopupParentCandidate {
                windowSurface,
                windowXdgSurface,
                {},
                std::nullopt
            };
            menuListener.toplevelSurface = windowSurface;
            menuListener.candidate = WaylandPopupParentCandidate {
                menuSurface,
                fakePopupHandle<xdg_surface> (4),
                {},
                WaylandPopupParentCandidate::MappedPopup { &componentToDismiss, 42 }
            };

            WaylandPopupContext context;
            context.surfaces = PopupSurfaces { { windowSurface, &windowListener }, { menuSurface, &menuListener } };

            const auto parent = findWaylandPopupParent (context, windowSurface, WaylandPopupGrab::no);

            expect (parent.has_value());

            if (! parent.has_value())
                return;

            expect (parent->parentSurface == windowSurface);
            expect (parent->parentXdgSurface == windowXdgSurface);
            expect (parent->componentToDismiss == nullptr);
            expect (parent->grabSerial == nullptr);
        });

        testCase ("An input serial is returned only when a popup grab is requested", [&]
        {
            TestPopupParentListener windowListener;
            auto* windowSurface = fakePopupHandle<wl_surface> (1);
            auto* windowXdgSurface = fakePopupHandle<xdg_surface> (2);
            windowListener.toplevelSurface = windowSurface;
            windowListener.candidate = WaylandPopupParentCandidate {
                windowSurface,
                windowXdgSurface,
                {},
                std::nullopt
            };

            WaylandPopupContext context;
            context.surfaces = PopupSurfaces { { windowSurface, &windowListener } };
            context.trigger = WaylandPopupTrigger { windowSurface, std::make_shared<WaylandPopupGrabSerial> (42) };

            const auto helperParent = findWaylandPopupParent (context, windowSurface, WaylandPopupGrab::no);
            const auto menuParent = findWaylandPopupParent (context, windowSurface, WaylandPopupGrab::yes);

            expect (helperParent.has_value() && helperParent->parentXdgSurface == windowXdgSurface);
            expect (helperParent.has_value() && helperParent->grabSerial == nullptr);
            expect (menuParent.has_value() && menuParent->grabSerial != nullptr);
        });

        testCase ("An ungrabbed window's toplevel candidate can parent a menu", [&]
        {
            TestPopupParentListener windowListener;
            TestPopupParentListener helperListener;
            auto* windowSurface = fakePopupHandle<wl_surface> (1);
            auto* windowXdgSurface = fakePopupHandle<xdg_surface> (2);
            auto* helperSurface = fakePopupHandle<wl_surface> (3);
            windowListener.toplevelSurface = windowSurface;
            windowListener.candidate = WaylandPopupParentCandidate {
                windowSurface,
                windowXdgSurface,
                {},
                std::nullopt
            };

            // A grabbed popup requires a grabbed popup or toplevel parent, so an ungrabbed helper offers its toplevel.
            helperListener.toplevelSurface = windowSurface;
            helperListener.candidate = windowListener.candidate;

            WaylandPopupContext context;
            context.surfaces = PopupSurfaces { { windowSurface, &windowListener }, { helperSurface, &helperListener } };
            const auto parent = findWaylandPopupParent (context, windowSurface, WaylandPopupGrab::yes);

            expect (parent.has_value());

            if (! parent.has_value())
                return;

            expect (parent->parentSurface == windowSurface);
            expect (parent->parentXdgSurface == windowXdgSurface);
        });

        testCase ("A mapped popup from another toplevel is ignored when selecting a parent", [&]
        {
            TestPopupParentListener window, otherPopup;
            auto* toplevelSurface = fakePopupHandle<wl_surface> (1);
            auto* popupSurface = fakePopupHandle<wl_surface> (2);
            window.toplevelSurface = toplevelSurface;
            window.candidate = WaylandPopupParentCandidate { toplevelSurface, fakePopupHandle<xdg_surface> (3), {}, std::nullopt };
            otherPopup.toplevelSurface = fakePopupHandle<wl_surface> (4);
            otherPopup.candidate = WaylandPopupParentCandidate {
                popupSurface, fakePopupHandle<xdg_surface> (5), {},
                WaylandPopupParentCandidate::MappedPopup { nullptr, 42 }
            };
            WaylandPopupContext context;
            context.surfaces = PopupSurfaces { { toplevelSurface, &window }, { popupSurface, &otherPopup } };
            const auto parent = findWaylandPopupParent (context, toplevelSurface, WaylandPopupGrab::yes);
            expect (parent.has_value() && parent->parentSurface == toplevelSurface);
        });

        testCase ("A trigger serial is used only when its source belongs to the supplied toplevel", [&]
        {
            TestPopupParentListener window, otherWindow;
            auto* toplevelSurface = fakePopupHandle<wl_surface> (1);
            auto* otherSurface = fakePopupHandle<wl_surface> (2);
            window.toplevelSurface = toplevelSurface;
            window.candidate = WaylandPopupParentCandidate { toplevelSurface, fakePopupHandle<xdg_surface> (3), {}, std::nullopt };
            otherWindow.toplevelSurface = otherSurface;
            WaylandPopupContext context;
            context.surfaces = PopupSurfaces { { toplevelSurface, &window }, { otherSurface, &otherWindow } };
            const auto serial = std::make_shared<WaylandPopupGrabSerial> (42);
            context.trigger = WaylandPopupTrigger { toplevelSurface, serial };
            const auto sameWindow = findWaylandPopupParent (context, toplevelSurface, WaylandPopupGrab::yes);
            expect (sameWindow.has_value() && sameWindow->grabSerial == serial);

            context.trigger = WaylandPopupTrigger { otherSurface, serial };
            const auto differentWindow = findWaylandPopupParent (context, toplevelSurface, WaylandPopupGrab::yes);
            expect (differentWindow.has_value() && differentWindow->grabSerial == nullptr);
        });

        testCase ("A toplevel without a mapped candidate has no popup parent", [&]
        {
            TestPopupParentListener window;
            auto* toplevelSurface = fakePopupHandle<wl_surface> (1);
            window.toplevelSurface = toplevelSurface;
            WaylandPopupContext context;
            context.surfaces = PopupSurfaces { { toplevelSurface, &window } };
            expect (! findWaylandPopupParent (context, toplevelSurface, WaylandPopupGrab::no).has_value());
        });

        testCase ("Popup descendants are returned topmost first when registration order differs", [&]
        {
            TestPopupParentListener child;
            TestPopupParentListener grandchild;
            auto* ancestorXdgSurface = fakePopupHandle<xdg_surface> (10);
            auto* childSurface = fakePopupHandle<wl_surface> (11);
            auto* childXdgSurface = fakePopupHandle<xdg_surface> (12);
            auto* grandchildSurface = fakePopupHandle<wl_surface> (13);
            auto* grandchildXdgSurface = fakePopupHandle<xdg_surface> (14);
            child.candidate = WaylandPopupParentCandidate {
                childSurface,
                childXdgSurface,
                {},
                WaylandPopupParentCandidate::MappedPopup { nullptr, std::nullopt, ancestorXdgSurface }
            };
            grandchild.candidate = WaylandPopupParentCandidate {
                grandchildSurface,
                grandchildXdgSurface,
                {},
                WaylandPopupParentCandidate::MappedPopup { nullptr, std::nullopt, childXdgSurface }
            };

            WaylandPopupContext context;

            // A popup may remap onto a peer created later, so a child can be registered before its parent.
            context.surfaces = PopupSurfaces { { grandchildSurface, &grandchild }, { childSurface, &child } };
            const auto descendants = findWaylandPopupDescendantsTopmostFirst (context, ancestorXdgSurface);

            expect (descendants == std::vector<wl_surface*> { grandchildSurface, childSurface });
        });

        testCase ("Only mapped popup descendants of the ancestor are returned", [&]
        {
            TestPopupParentListener child;
            TestPopupParentListener unrelated;
            TestPopupParentListener toplevel;
            TestPopupParentListener orphan;
            TestPopupParentListener unmapped;
            auto* ancestorXdgSurface = fakePopupHandle<xdg_surface> (10);
            auto* childSurface = fakePopupHandle<wl_surface> (11);
            auto* unrelatedSurface = fakePopupHandle<wl_surface> (20);
            auto* toplevelSurface = fakePopupHandle<wl_surface> (30);
            auto* orphanSurface = fakePopupHandle<wl_surface> (40);
            child.candidate = WaylandPopupParentCandidate {
                childSurface,
                fakePopupHandle<xdg_surface> (12),
                {},
                WaylandPopupParentCandidate::MappedPopup { nullptr, std::nullopt, ancestorXdgSurface }
            };
            unrelated.candidate = WaylandPopupParentCandidate {
                unrelatedSurface,
                fakePopupHandle<xdg_surface> (21),
                {},
                WaylandPopupParentCandidate::MappedPopup { nullptr, std::nullopt, fakePopupHandle<xdg_surface> (22) }
            };
            toplevel.candidate = WaylandPopupParentCandidate {
                toplevelSurface,
                fakePopupHandle<xdg_surface> (31),
                {},
                std::nullopt
            };
            orphan.candidate = WaylandPopupParentCandidate {
                orphanSurface,
                fakePopupHandle<xdg_surface> (41),
                {},
                WaylandPopupParentCandidate::MappedPopup { nullptr, std::nullopt, nullptr }
            };

            WaylandPopupContext context;
            context.surfaces = PopupSurfaces { { childSurface, &child },
                                 { unrelatedSurface, &unrelated },
                                 { toplevelSurface, &toplevel },
                                 { orphanSurface, &orphan },
                                 { fakePopupHandle<wl_surface> (50), &unmapped },
                                 { fakePopupHandle<wl_surface> (60), nullptr } };
            const auto descendants = findWaylandPopupDescendantsTopmostFirst (context, ancestorXdgSurface);

            expect (descendants == std::vector<wl_surface*> { childSurface });
        });
    }
};

static WaylandPopupParentTests waylandPopupParentTests;

//==============================================================================
class WaylandPopupPlacementTests final : public UnitTest
{
public:
    WaylandPopupPlacementTests()
        : UnitTest ("WaylandPopupPlacement", UnitTestCategories::gui) {}

    void runTest() override
    {
        testCase ("Requested popup bounds determine a parent-relative anchor, size, and offset", [&]
        {
            const auto placement = makeWaylandPopupPlacement ({ 130, 90, 200, 100 },
                                                              { { 100, 50, 400, 300 }, 1.0 });

            expect (placement.anchorRectangle == Rectangle<int> (30, 40, 1, 1));
            expect (placement.popupSize == Point<int> (200, 100));
            expect (placement.offset.isOrigin());
            expectEquals ((int) placement.anchor, (int) WaylandProtocol::xdgPositionerAnchorTopLeft);
            expectEquals ((int) placement.gravity, (int) WaylandProtocol::xdgPositionerGravityBottomRight);
        });

        testCase ("Empty popup dimensions are raised to the protocol minimum", [&]
        {
            const auto placement = makeWaylandPopupPlacement ({ 10, 20, 0, -1 }, {});

            expect (placement.popupSize == Point<int> (1, 1));
        });

        testCase ("Requested positions above and to the left of the parent use the first valid anchor plus an offset", [&]
        {
            const auto placement = makeWaylandPopupPlacement ({ 80, 40, 200, 100 },
                                                              { { 100, 50, 400, 300 }, 1.0 });

            expect (placement.anchorRectangle == Rectangle<int> (0, 0, 1, 1));
            expect (placement.offset == Point<int> (-20, -10));
        });

        testCase ("Requested positions below and to the right of the parent use the last valid anchor plus an offset", [&]
        {
            const auto placement = makeWaylandPopupPlacement ({ 650, 410, 200, 100 },
                                                              { { 100, 50, 400, 300 }, 1.0 });

            expect (placement.anchorRectangle == Rectangle<int> (399, 299, 1, 1));
            expect (placement.offset == Point<int> (151, 61));
        });

        testCase ("An anchor at the parent edge and its offset recover the requested parent-relative bounds", [&]
        {
            const WaylandPopupParentCoordinates parentCoordinates { { 100, 50, 400, 300 }, 1.0 };
            const Rectangle<int> popupBounds { 650, 410, 240, 120 };
            const auto placement = makeWaylandPopupPlacement (popupBounds, parentCoordinates);
            const auto requestedBounds = getRequestedParentRelativeBounds (placement);
            const auto expectedBounds = popupBounds.translated (-parentCoordinates.logicalBounds.getX(),
                                                                 -parentCoordinates.logicalBounds.getY());

            expect (requestedBounds == expectedBounds,
                    "Expected " + expectedBounds.toString() + ", got " + requestedBounds.toString());
        });

        testCase ("A host scale override converts parent-relative popup positions to surface coordinates", [&]
        {
            const WaylandPopupParentCoordinates parentCoordinates { { 30, 60, 481, 319 }, 0.75 };
            const auto placement = makeWaylandPopupPlacement ({ 509, 219, 125, 20 }, parentCoordinates);

            expect (placement.anchorRectangle == Rectangle<int> (359, 119, 1, 1));
            expect (placement.popupSize == Point<int> (125, 20));
            expect (placement.offset.isOrigin());
        });

        testCase ("Popup configure positions are converted from parent surface coordinates to logical coordinates", [&]
        {
            const WaylandPopupParentCoordinates parentCoordinates { { 30, 60, 481, 319 }, 0.75 };
            const auto bounds = convertWaylandPopupConfigureToLogicalBounds ({ 359, 119, 125, 20 },
                                                                             parentCoordinates);

            expect (bounds == Rectangle<int> (509, 219, 125, 20));
        });

        testCase ("Temporary window flags select whether to request a popup grab", [&]
        {
            constexpr auto popupMenuFlags = ComponentPeer::windowIsTemporary
                                          | ComponentPeer::windowIgnoresKeyPresses
                                          | ComponentPeer::windowHasDropShadow;
            constexpr auto tooltipFlags = popupMenuFlags | ComponentPeer::windowIgnoresMouseClicks;
            constexpr auto dragImageFlags = ComponentPeer::windowIsTemporary
                                          | ComponentPeer::windowIgnoresMouseClicks;
            constexpr auto callOutBoxFlags = ComponentPeer::windowIsTemporary;
            constexpr auto dragShadowFlags = ComponentPeer::windowIsTemporary
                                           | ComponentPeer::windowHasDropShadow;

            expect (getWaylandPopupGrab (popupMenuFlags) == WaylandPopupGrab::yes);
            expect (getWaylandPopupGrab (callOutBoxFlags) == WaylandPopupGrab::no);
            expect (getWaylandPopupGrab (dragShadowFlags) == WaylandPopupGrab::no);
            expect (getWaylandPopupGrab (tooltipFlags) == WaylandPopupGrab::no);
            expect (getWaylandPopupGrab (dragImageFlags) == WaylandPopupGrab::no);
            expect (! getWaylandPopupGrab (ComponentPeer::windowAppearsOnTaskbar).has_value());
        });
    }
};

static WaylandPopupPlacementTests waylandPopupPlacementTests;

#endif

} // namespace juce
