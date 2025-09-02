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

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
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

namespace juce::detail
{

constexpr char colourPropertyPrefix[] = "jcclr_";

//==============================================================================
struct ComponentHelpers
{
    using SH = ScalingHelpers;

   #if JUCE_MODAL_LOOPS_PERMITTED
    static void* runModalLoopCallback (void* userData)
    {
        return (void*) (pointer_sized_int) static_cast<Component*> (userData)->runModalLoop();
    }
   #endif

    static Identifier getColourPropertyID (int colourID)
    {
        char buffer[32];
        auto* end = buffer + numElementsInArray (buffer) - 1;
        auto* t = end;
        *t = 0;

        for (auto v = (uint32) colourID;;)
        {
            *--t = "0123456789abcdef" [v & 15];
            v >>= 4;

            if (v == 0)
                break;
        }

        for (int i = (int) sizeof (colourPropertyPrefix) - 1; --i >= 0;)
            *--t = colourPropertyPrefix[i];

        return t;
    }

    //==============================================================================
    static bool hitTest (Component& comp, Point<float> localPoint)
    {
        const auto intPoint = localPoint.roundToInt();
        return Rectangle<int> { comp.getWidth(), comp.getHeight() }.contains (intPoint)
            && comp.hitTest (intPoint.x, intPoint.y);
    }

    // converts an unscaled position within a peer to the local position within that peer's component
    template <typename PointOrRect>
    static PointOrRect rawPeerPositionToLocal (const Component& comp, PointOrRect pos) noexcept
    {
        if (comp.isTransformed())
            pos = pos.transformedBy (comp.getTransform().inverted());

        return SH::unscaledScreenPosToScaled (comp, pos);
    }

    // converts a position within a peer's component to the unscaled position within the peer
    template <typename PointOrRect>
    static PointOrRect localPositionToRawPeerPos (const Component& comp, PointOrRect pos) noexcept
    {
        if (comp.isTransformed())
            pos = pos.transformedBy (comp.getTransform());

        return SH::scaledScreenPosToUnscaled (comp, pos);
    }

    template <typename PointOrRect>
    static PointOrRect convertFromParentSpace (const Component& comp, const PointOrRect pointInParentSpace)
    {
        const auto transformed = comp.affineTransform != nullptr ? pointInParentSpace.transformedBy (comp.affineTransform->inverted())
                                                                 : pointInParentSpace;

        if (comp.isOnDesktop())
        {
            if (auto* peer = comp.getPeer())
                return SH::unscaledScreenPosToScaled (comp, peer->globalToLocal (SH::scaledScreenPosToUnscaled (transformed)));

            jassertfalse;
            return transformed;
        }

        if (comp.getParentComponent() == nullptr)
            return SH::subtractPosition (SH::unscaledScreenPosToScaled (comp, SH::scaledScreenPosToUnscaled (transformed)), comp);

        return SH::subtractPosition (transformed, comp);
    }

    template <typename PointOrRect>
    static PointOrRect convertToParentSpace (const Component& comp, const PointOrRect pointInLocalSpace)
    {
        const auto preTransform = [&]
        {
            if (comp.isOnDesktop())
            {
                if (auto* peer = comp.getPeer())
                    return SH::unscaledScreenPosToScaled (peer->localToGlobal (SH::scaledScreenPosToUnscaled (comp, pointInLocalSpace)));

                jassertfalse;
                return pointInLocalSpace;
            }

            if (comp.getParentComponent() == nullptr)
                return SH::unscaledScreenPosToScaled (SH::scaledScreenPosToUnscaled (comp, SH::addPosition (pointInLocalSpace, comp)));

            return SH::addPosition (pointInLocalSpace, comp);
        }();

        return comp.affineTransform != nullptr ? preTransform.transformedBy (*comp.affineTransform)
                                               : preTransform;
    }

    template <typename PointOrRect>
    static PointOrRect convertFromDistantParentSpace (const Component* parent, const Component& target, PointOrRect coordInParent)
    {
        auto* directParent = target.getParentComponent();
        jassert (directParent != nullptr);

        if (directParent == parent)
            return convertFromParentSpace (target, coordInParent);

        JUCE_BEGIN_IGNORE_WARNINGS_MSVC (6011)
        return convertFromParentSpace (target, convertFromDistantParentSpace (parent, *directParent, coordInParent));
        JUCE_END_IGNORE_WARNINGS_MSVC
    }

    template <typename PointOrRect>
    static PointOrRect convertCoordinate (const Component* target, const Component* source, PointOrRect p)
    {
        while (source != nullptr)
        {
            if (source == target)
                return p;

            JUCE_BEGIN_IGNORE_WARNINGS_MSVC (6011)

            if (source->isParentOf (target))
                return convertFromDistantParentSpace (source, *target, p);

            JUCE_END_IGNORE_WARNINGS_MSVC

            p = convertToParentSpace (*source, p);
            source = source->getParentComponent();
        }

        jassert (source == nullptr);
        if (target == nullptr)
            return p;

        auto* topLevelComp = target->getTopLevelComponent();

        p = convertFromParentSpace (*topLevelComp, p);

        if (topLevelComp == target)
            return p;

        return convertFromDistantParentSpace (topLevelComp, *target, p);
    }

    static bool clipChildComponent (const Component& child,
                                    Graphics& g,
                                    const Rectangle<int> clipRect,
                                    Point<int> delta)
    {
        if (! child.isVisible() || child.isTransformed())
            return false;

        const auto newClip = clipRect.getIntersection (child.boundsRelativeToParent);

        if (newClip.isEmpty())
            return false;

        if (child.isOpaque() && child.componentTransparency == 0)
        {
            g.excludeClipRegion (newClip + delta);
            return true;
        }

        const auto childPos = child.getPosition();
        return clipObscuredRegions (child, g, newClip - childPos, childPos + delta);
    }

    static bool clipObscuredRegions (const Component& comp,
                                     Graphics& g,
                                     const Rectangle<int> clipRect,
                                     Point<int> delta)
    {
        auto wasClipped = false;

        for (int i = comp.childComponentList.size(); --i >= 0;)
            wasClipped |= clipChildComponent (*comp.childComponentList.getUnchecked (i), g, clipRect, delta);

        return wasClipped;
    }

    static Rectangle<int> getParentOrMainMonitorBounds (const Component& comp)
    {
        if (auto* p = comp.getParentComponent())
            return p->getLocalBounds();

        return Desktop::getInstance().getDisplays().getPrimaryDisplay()->userArea;
    }

    static void releaseAllCachedImageResources (Component& c)
    {
        c.invalidateCachedImageResources();

        for (auto* child : c.childComponentList)
            releaseAllCachedImageResources (*child);
    }

    //==============================================================================
    static bool modalWouldBlockComponent (const Component& maybeBlocked, Component* modal)
    {
        return modal != nullptr
            && modal != &maybeBlocked
            && ! modal->isParentOf (&maybeBlocked)
            && ! modal->canModalEventBeSentToComponent (&maybeBlocked);
    }

    template <typename Function>
    static void sendMouseEventToComponentsThatAreBlockedByModal (Component& modal, Function&& function)
    {
        for (auto& ms : Desktop::getInstance().getMouseSources())
            if (auto* c = ms.getComponentUnderMouse())
                if (modalWouldBlockComponent (*c, &modal))
                    function (c, ms, SH::screenPosToLocalPos (*c, ms.getScreenPosition()), Time::getCurrentTime());
    }

    class ModalComponentManagerChangeNotifier
    {
    public:
        static auto& getInstance()
        {
            static ModalComponentManagerChangeNotifier instance;
            return instance;
        }

        ErasedScopeGuard addListener (std::function<void()> l)
        {
            return listeners.addListener (std::move (l));
        }

        void modalComponentManagerChanged()
        {
            listeners.call();
        }

    private:
        ModalComponentManagerChangeNotifier() = default;

        detail::CallbackListenerList<> listeners;
    };
};

} // namespace juce::detail
