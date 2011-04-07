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

// (This file gets included by juce_mac_NativeCode.mm, rather than being
// compiled on its own).
#if JUCE_INCLUDED_FILE

//==============================================================================
class NSViewComponentInternal  : public ComponentMovementWatcher
{
public:
    //==============================================================================
    NSViewComponentInternal (NSView* const view_, Component& owner_)
        : ComponentMovementWatcher (&owner_),
          owner (owner_),
          currentPeer (nullptr),
          view (view_)
    {
        [view_ retain];

        if (owner.isShowing())
            componentPeerChanged();
    }

    ~NSViewComponentInternal()
    {
        [view removeFromSuperview];
        [view release];
    }

    //==============================================================================
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
        NSViewComponentPeer* const peer = dynamic_cast <NSViewComponentPeer*> (owner.getPeer());

        if (currentPeer != peer)
        {
            if ([view superview] != nil)
                [view removeFromSuperview]; // Must be careful not to call this unless it's required - e.g. some Apple AU views
                                            // override the call and use it as a sign that they're being deleted, which breaks everything..

            currentPeer = peer;

            if (peer != nullptr)
            {
                [peer->view addSubview: view];
                componentMovedOrResized (false, false);
            }
        }

        [view setHidden: ! owner.isShowing()];
    }

    void componentVisibilityChanged()
    {
        componentPeerChanged();
    }

    const Rectangle<int> getViewBounds() const
    {
        NSRect r = [view frame];
        return Rectangle<int> (0, 0, (int) r.size.width, (int) r.size.height);
    }

private:
    Component& owner;
    NSViewComponentPeer* currentPeer;

public:
    NSView* const view;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NSViewComponentInternal);
};

//==============================================================================
NSViewComponent::NSViewComponent()
{
}

NSViewComponent::~NSViewComponent()
{
}

void NSViewComponent::setView (void* view)
{
    if (view != getView())
    {
        if (view != nullptr)
            info = new NSViewComponentInternal ((NSView*) view, *this);
        else
            info = nullptr;
    }
}

void* NSViewComponent::getView() const
{
    return info == nullptr ? nullptr : info->view;
}

void NSViewComponent::resizeToFitView()
{
    if (info != nullptr)
        setBounds (info->getViewBounds());
}

void NSViewComponent::paint (Graphics&)
{
}

#endif
