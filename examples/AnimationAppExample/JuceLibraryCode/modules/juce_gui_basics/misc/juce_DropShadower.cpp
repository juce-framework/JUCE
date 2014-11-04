/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

class DropShadower::ShadowWindow  : public Component
{
public:
    ShadowWindow (Component* comp, const DropShadow& ds)
        : target (comp), shadow (ds)
    {
        setVisible (true);
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
DropShadower::DropShadower (const DropShadow& ds)
   : owner (nullptr), shadow (ds), reentrant (false)
{
}

DropShadower::~DropShadower()
{
    if (owner != nullptr)
    {
        owner->removeComponentListener (this);
        owner = nullptr;
    }

    updateParent();

    reentrant = true;
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

    const ScopedValueSetter<bool> setter (reentrant, true, false);

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
                sw->setAlwaysOnTop (owner->isAlwaysOnTop());

            if (sw != nullptr)
            {
                switch (i)
                {
                    case 0: sw->setBounds (x - shadowEdge, y, shadowEdge, h); break;
                    case 1: sw->setBounds (x + w, y, shadowEdge, h); break;
                    case 2: sw->setBounds (x, y, w, shadowEdge); break;
                    case 3: sw->setBounds (x, owner->getBottom(), w, shadowEdge); break;
                    default: break;
                }
            }

            if (sw != nullptr)
                sw->toBehind (i == 3 ? owner : shadowWindows.getUnchecked (i + 1));

            if (sw == nullptr)
                return;
        }
    }
    else
    {
        shadowWindows.clear();
    }
}
