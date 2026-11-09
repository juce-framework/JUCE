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

class DropShadower::ShadowWindow final : public Component
{
public:
    ShadowWindow (Component& comp, const DropShadow& ds, Rectangle<int> initialBounds)
        : target (&comp), shadow (ds)
    {
        setAccessible (false);
        setInterceptsMouseClicks (false, false);
        setAlwaysOnTop (comp.isAlwaysOnTop());
        setBounds (initialBounds);

        if (comp.isOnDesktop())
        {
           #if JUCE_WINDOWS
            const ScopedThreadDPIAwarenessSetter scope { comp.getWindowHandle() };
           #endif

            addToDesktop (DesktopWindowOptions().withStyleFlags (ComponentPeer::windowIgnoresMouseClicks
                                                                 | ComponentPeer::windowIsTemporary
                                                                 | ComponentPeer::windowIgnoresKeyPresses)
                                                .withAssociatedComponent (&comp));

            if (auto* compPeer = comp.getPeer())
                if (auto* selfPeer = getPeer())
                    selfPeer->setCustomPlatformScaleFactor (compPeer->getPlatformScaleFactor());
        }
        else if (auto* parent = comp.getParentComponent())
        {
            parent->addChildComponent (this);
        }

        toBehind (&comp);
        setVisible (true);
    }

    void paint (Graphics& g) override
    {
        if (auto* c = target.get())
        {
            const auto ownerArea = getLocalArea (c, c->getLocalBounds());
            g.excludeClipRegion (ownerArea);
            shadow.drawForRectangle (g, ownerArea);
        }
    }

    float getDesktopScaleFactor() const override
    {
        if (target != nullptr)
            return target->getDesktopScaleFactor();

        return Component::getDesktopScaleFactor();
    }

private:
    WeakReference<Component> target;
    DropShadow shadow;

    JUCE_DECLARE_NON_COPYABLE (ShadowWindow)
};

class DropShadower::ParentVisibilityChangedListener final : public ComponentListener
{
public:
    ParentVisibilityChangedListener (Component& r, ComponentListener& l)
        : root (&r), listener (&l)
    {
        updateParentHierarchy();
    }

    ~ParentVisibilityChangedListener() override
    {
        for (auto& compEntry : observedComponents)
            if (auto* comp = compEntry.get())
                comp->removeComponentListener (this);
    }

    void componentVisibilityChanged (Component& component) override
    {
        if (root != &component)
            listener->componentVisibilityChanged (*root);
    }

    void componentParentHierarchyChanged (Component& component) override
    {
        if (root == &component)
            updateParentHierarchy();
    }

private:
    class ComponentWithWeakReference
    {
    public:
        explicit ComponentWithWeakReference (Component& c)
            : ptr (&c), ref (&c) {}

        Component* get() const { return ref.get(); }

        bool operator< (const ComponentWithWeakReference& other) const { return ptr < other.ptr; }

    private:
        Component* ptr;
        WeakReference<Component> ref;
    };

    void updateParentHierarchy()
    {
        const auto lastSeenComponents = std::exchange (observedComponents, std::invoke ([&]
        {
            std::set<ComponentWithWeakReference> result;

            for (auto node = root; node != nullptr; node = node->getParentComponent())
                result.emplace (*node);

            return result;
        }));

        const auto withDifference = [] (const auto& rangeA, const auto& rangeB, auto&& callback)
        {
            std::vector<ComponentWithWeakReference> result;
            std::set_difference (rangeA.begin(), rangeA.end(), rangeB.begin(), rangeB.end(), std::back_inserter (result));

            for (const auto& item : result)
                if (auto* c = item.get())
                    callback (*c);
        };

        withDifference (lastSeenComponents, observedComponents, [this] (auto& comp) { comp.removeComponentListener (this); });
        withDifference (observedComponents, lastSeenComponents, [this] (auto& comp) { comp.addComponentListener (this); });
    }

    Component* root = nullptr;
    ComponentListener* listener = nullptr;
    std::set<ComponentWithWeakReference> observedComponents;

    JUCE_DECLARE_NON_COPYABLE (ParentVisibilityChangedListener)
    JUCE_DECLARE_NON_MOVEABLE (ParentVisibilityChangedListener)
};

class DropShadower::ScaleWatcher
{
public:
    explicit ScaleWatcher (DropShadower& shadower)
    {
        if (shadower.owner == nullptr)
            return;

        notifier.emplace (shadower.owner.get(), [&shadower] (auto x)
        {
            if (auto* sw = shadower.shadowWindow.get())
                if (sw->isOnDesktop())
                    if (auto* peer = sw->getPeer())
                        peer->setCustomPlatformScaleFactor (x);

            shadower.updateShadows();
        });
    }

private:
    std::optional<NativeScaleFactorNotifier> notifier;
};

//==============================================================================
DropShadower::DropShadower (const DropShadow& ds)  : shadow (ds)  {}

DropShadower::~DropShadower()
{
    if (owner != nullptr)
    {
        owner->removeComponentListener (this);
        owner = nullptr;
    }

    updateParent();

    const ScopedValueSetter<bool> setter (reentrant, true);
    shadowWindow.reset();
}

void DropShadower::setOwner (Component* componentToFollow)
{
    if (componentToFollow != owner)
    {
        if (owner != nullptr)
            owner->removeComponentListener (this);

        // (the component can't be null)
        jassert (componentToFollow != nullptr);

        owner = componentToFollow;
        jassert (owner != nullptr);

        updateParent();
        owner->addComponentListener (this);

        // The visibility of `owner` is transitively affected by the visibility of its parents. Thus we need to trigger the
        // componentVisibilityChanged() event in case it changes for any of the parents.
        visibilityChangedListener = std::make_unique<ParentVisibilityChangedListener> (*owner,
                                                                                       static_cast<ComponentListener&> (*this));

        scaleWatcher = std::make_unique<ScaleWatcher> (*this);

        updateShadows();
    }
}

void DropShadower::updateParent()
{
    if (Component* p = lastParentComp)
        p->removeComponentListener (this);

    lastParentComp = owner != nullptr ? owner->getParentComponent() : nullptr;

    if (Component* p = lastParentComp)
        p->addComponentListener (this);
}

void DropShadower::componentMovedOrResized (Component& c, bool /*wasMoved*/, bool /*wasResized*/)
{
    if (owner == &c)
        updateShadows();
}

void DropShadower::componentBroughtToFront (Component& c)
{
    if (owner == &c)
        updateShadows();
}

void DropShadower::componentChildrenChanged (Component&)
{
    updateShadows();
}

void DropShadower::componentParentHierarchyChanged (Component& c)
{
    if (owner == &c)
    {
        updateParent();
        updateShadows();
    }
}

void DropShadower::componentVisibilityChanged (Component& c)
{
    if (owner == &c)
        updateShadows();
}

void DropShadower::updateShadows()
{
    if (reentrant)
        return;

    const ScopedValueSetter<bool> setter (reentrant, true);

    if (owner != nullptr
        && owner->isShowing()
        && owner->getWidth() > 0 && owner->getHeight() > 0
        && (Desktop::canUseSemiTransparentWindows() || owner->getParentComponent() != nullptr))
    {
        const auto shadowEdge = jmax (shadow.offset.x, shadow.offset.y) + shadow.radius;
        const auto shadowBounds = owner->getLocalBounds().expanded (shadowEdge) + owner->getPosition();

        // there seem to be rare situations where the dropshadower may be deleted by
        // callbacks during this function, so use a weak ref to watch out for this
        const WeakReference self (this);

        const auto isNewWindow = shadowWindow == nullptr;

        if (isNewWindow)
            shadowWindow = std::make_unique<ShadowWindow> (*owner, shadow, shadowBounds);
        else
            shadowWindow->setAlwaysOnTop (owner->isAlwaysOnTop());

        if (self == nullptr)
            return;

        {
            using Scope = decltype (std::declval<ComponentPeer>().setMultimonitorPositionOverride ({}));
            const auto scope = std::invoke ([&]() -> std::optional<Scope>
            {
                auto* peer = owner->isOnDesktop() ? owner->getPeer() : nullptr;

                if (peer == nullptr)
                    return std::nullopt;

                auto* shadowPeer = shadowWindow->getPeer();

                if (shadowPeer == nullptr)
                    return std::nullopt;

                using SH = detail::ScalingHelpers;
                const auto localPos = SH::scaledScreenPosToUnscaled (*owner, Point { -shadowEdge, -shadowEdge }.toFloat());
                const auto multimonitorPos = peer->localToMultimonitor (localPos);

                return shadowPeer->setMultimonitorPositionOverride (multimonitorPos.roundToInt());
            });

            // A new window already has these bounds, but its native window was positioned
            // without the override, which matters when it straddles displays of different scales.
            if (isNewWindow)
            {
                if (scope.has_value())
                    if (auto* shadowPeer = shadowWindow->getPeer())
                        shadowPeer->updateBounds();
            }
            else
            {
                shadowWindow->setBounds (shadowBounds);
            }
        }

        if (self == nullptr)
            return;

        shadowWindow->toBehind (owner.get());
    }
    else
    {
        shadowWindow.reset();
    }
}

} // namespace juce
