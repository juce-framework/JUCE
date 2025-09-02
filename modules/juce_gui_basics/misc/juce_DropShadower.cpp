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

namespace juce
{

class DropShadower::ShadowWindow final : public Component
{
public:
    ShadowWindow (Component* comp, const DropShadow& ds)
        : target (comp), shadow (ds)
    {
        setVisible (true);
        setAccessible (false);
        setInterceptsMouseClicks (false, false);

        if (comp->isOnDesktop())
        {
           #if JUCE_WINDOWS
            const auto scope = [&]() -> std::optional<ScopedThreadDPIAwarenessSetter>
            {
                if (comp != nullptr)
                    if (auto* handle = comp->getWindowHandle())
                        return ScopedThreadDPIAwarenessSetter (handle);

                return {};
            }();
           #endif

            setSize (1, 1); // to keep the OS happy by not having zero-size windows
            addToDesktop (ComponentPeer::windowIgnoresMouseClicks
                            | ComponentPeer::windowIsTemporary
                            | ComponentPeer::windowIgnoresKeyPresses);
        }
        else if (Component* const parent = comp->getParentComponent())
        {
            parent->addChildComponent (this);
        }
    }

    void paint (Graphics& g) override
    {
        if (Component* c = target)
            shadow.drawForRectangle (g, getLocalArea (c, c->getLocalBounds()));
    }

    void resized() override
    {
        repaint();  // (needed for correct repainting)
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

class DropShadower::VirtualDesktopWatcher final  : public ComponentListener,
                                                   private Timer
{
public:
    //==============================================================================
    VirtualDesktopWatcher (Component& c) : component (&c)
    {
        component->addComponentListener (this);
        update();
    }

    ~VirtualDesktopWatcher() override
    {
        stopTimer();

        if (auto* c = component.get())
            c->removeComponentListener (this);
    }

    bool shouldHideDropShadow() const
    {
        return hasReasonToHide;
    }

    void addListener (void* listener, std::function<void()> cb)
    {
        listeners[listener] = std::move (cb);
    }

    void removeListener (void* listener)
    {
        listeners.erase (listener);
    }

    //==============================================================================
    void componentParentHierarchyChanged (Component& c) override
    {
        if (component.get() == &c)
            update();
    }

private:
    //==============================================================================
    void update()
    {
        bool newHasReasonToHide = false;

        if (! component.wasObjectDeleted() && isWindows && component->isOnDesktop())
        {
            startTimerHz (5);

            WeakReference<VirtualDesktopWatcher> weakThis (this);

            // During scaling changes this call can trigger a call to HWNDComponentPeer::handleDPIChanging()
            // which deletes this VirtualDesktopWatcher.
            newHasReasonToHide = ! detail::WindowingHelpers::isWindowOnCurrentVirtualDesktop (component->getWindowHandle());

            if (weakThis == nullptr)
                return;
        }
        else
        {
            stopTimer();
        }

        if (std::exchange (hasReasonToHide, newHasReasonToHide) != newHasReasonToHide)
            for (auto& l : listeners)
                l.second();
    }

    void timerCallback() override
    {
        update();
    }

    //==============================================================================
    WeakReference<Component> component;
    const bool isWindows = (SystemStats::getOperatingSystemType() & SystemStats::Windows) != 0;
    bool hasReasonToHide = false;
    std::map<void*, std::function<void()>> listeners;

    JUCE_DECLARE_WEAK_REFERENCEABLE (VirtualDesktopWatcher)
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VirtualDesktopWatcher)
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
        const auto lastSeenComponents = std::exchange (observedComponents, [&]
        {
            std::set<ComponentWithWeakReference> result;

            for (auto node = root; node != nullptr; node = node->getParentComponent())
                result.emplace (*node);

            return result;
        }());

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

//==============================================================================
DropShadower::DropShadower (const DropShadow& ds)  : shadow (ds)  {}

DropShadower::~DropShadower()
{
    if (virtualDesktopWatcher != nullptr)
        virtualDesktopWatcher->removeListener (this);

    if (owner != nullptr)
    {
        owner->removeComponentListener (this);
        owner = nullptr;
    }

    updateParent();

    const ScopedValueSetter<bool> setter (reentrant, true);
    shadowWindows.clear();
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

        virtualDesktopWatcher = std::make_unique<VirtualDesktopWatcher> (*owner);
        virtualDesktopWatcher->addListener (this, [this]() { updateShadows(); });

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
        && (Desktop::canUseSemiTransparentWindows() || owner->getParentComponent() != nullptr)
        && (virtualDesktopWatcher == nullptr || ! virtualDesktopWatcher->shouldHideDropShadow()))
    {
        while (shadowWindows.size() < 4)
            shadowWindows.add (new ShadowWindow (owner, shadow));

        const int shadowEdge = jmax (shadow.offset.x, shadow.offset.y) + shadow.radius;
        const int x = owner->getX();
        const int y = owner->getY() - shadowEdge;
        const int w = owner->getWidth();
        const int h = owner->getHeight() + shadowEdge + shadowEdge;

        for (int i = 4; --i >= 0;)
        {
            // there seem to be rare situations where the dropshadower may be deleted by
            // callbacks during this loop, so use a weak ref to watch out for this..
            WeakReference<Component> sw (shadowWindows[i]);

            if (sw != nullptr)
            {
                sw->setAlwaysOnTop (owner->isAlwaysOnTop());

                if (sw == nullptr)
                    return;

                switch (i)
                {
                    case 0: sw->setBounds (x - shadowEdge, y, shadowEdge, h); break;
                    case 1: sw->setBounds (x + w, y, shadowEdge, h); break;
                    case 2: sw->setBounds (x, y, w, shadowEdge); break;
                    case 3: sw->setBounds (x, owner->getBottom(), w, shadowEdge); break;
                    default: break;
                }

                if (sw == nullptr)
                    return;

                sw->toBehind (i == 3 ? owner.get() : shadowWindows.getUnchecked (i + 1));
            }
        }
    }
    else
    {
        shadowWindows.clear();
    }
}

} // namespace juce
