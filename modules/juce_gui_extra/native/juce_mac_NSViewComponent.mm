/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

class NSViewComponent::Pimpl  : public ComponentMovementWatcher
{
public:
    Pimpl (NSView* const view_, Component& owner_)
        : ComponentMovementWatcher (&owner_),
          view (view_),
          owner (owner_),
          currentPeer (nullptr)
    {
        [view_ retain];

        if (owner.isShowing())
            componentPeerChanged();
    }

    ~Pimpl()
    {
        removeFromParent();
        [view release];
    }

    void componentMovedOrResized (Component& comp, bool wasMoved, bool wasResized)
    {
        ComponentMovementWatcher::componentMovedOrResized (comp, wasMoved, wasResized);

        // The ComponentMovementWatcher version of this method avoids calling
        // us when the top-level comp is resized, but for an NSView we need to know this
        // because with inverted co-ords, we need to update the position even if the
        // top-left pos hasn't changed
        if (comp.isOnDesktop() && wasResized)
            componentMovedOrResized (wasMoved, wasResized);
    }

    void componentMovedOrResized (bool /*wasMoved*/, bool /*wasResized*/)
    {
        Component* const topComp = owner.getTopLevelComponent();

        if (topComp->getPeer() != nullptr)
        {
            const Point<int> pos (topComp->getLocalPoint (&owner, Point<int>()));

            NSRect r = NSMakeRect ((float) pos.getX(), (float) pos.getY(), (float) owner.getWidth(), (float) owner.getHeight());
            r.origin.y = [[view superview] frame].size.height - (r.origin.y + r.size.height);

            [view setFrame: r];
        }
    }

    void componentPeerChanged()
    {
        ComponentPeer* const peer = owner.getPeer();

        if (currentPeer != peer)
        {
            removeFromParent();
            currentPeer = peer;

            if (peer != nullptr)
            {
                NSView* peerView = (NSView*) peer->getNativeHandle();
                [peerView addSubview: view];
                componentMovedOrResized (false, false);
            }
        }

        [view setHidden: ! owner.isShowing()];
    }

    void componentVisibilityChanged()
    {
        componentPeerChanged();
    }

    Rectangle<int> getViewBounds() const
    {
        NSRect r = [view frame];
        return Rectangle<int> ((int) r.size.width, (int) r.size.height);
    }

    NSView* const view;

private:
    Component& owner;
    ComponentPeer* currentPeer;

    void removeFromParent()
    {
        if ([view superview] != nil)
            [view removeFromSuperview]; // Must be careful not to call this unless it's required - e.g. some Apple AU views
                                        // override the call and use it as a sign that they're being deleted, which breaks everything..
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Pimpl);
};

//==============================================================================
NSViewComponent::NSViewComponent() {}
NSViewComponent::~NSViewComponent() {}

void NSViewComponent::setView (void* const view)
{
    if (view != getView())
    {
        pimpl = nullptr;

        if (view != nullptr)
            pimpl = new Pimpl ((NSView*) view, *this);
    }
}

void* NSViewComponent::getView() const
{
    return pimpl == nullptr ? nullptr : pimpl->view;
}

void NSViewComponent::resizeToFitView()
{
    if (pimpl != nullptr)
        setBounds (pimpl->getViewBounds());
}

void NSViewComponent::paint (Graphics&) {}
