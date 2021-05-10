/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

class DropShadower::ShadowWindow  : public Component
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

    if (owner == nullptr)
    {
        shadowWindows.clear();
        return;
    }

    if (owner->isShowing()
         && owner->getWidth() > 0 && owner->getHeight() > 0
         && (Desktop::canUseSemiTransparentWindows() || owner->getParentComponent() != nullptr))
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
