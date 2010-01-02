/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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
    Component* const owner;
    NSViewComponentPeer* currentPeer;
    bool wasShowing;

public:
    NSView* const view;

    //==============================================================================
    NSViewComponentInternal (NSView* const view_, Component* const owner_)
        : ComponentMovementWatcher (owner_),
          owner (owner_),
          currentPeer (0),
          wasShowing (false),
          view (view_)
    {
        [view_ retain];

        if (owner_->isShowing())
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
        Component* const topComp = owner->getTopLevelComponent();

        if (topComp->getPeer() != 0)
        {
            int x = 0, y = 0;
            owner->relativePositionToOtherComponent (topComp, x, y);

            NSRect r;
            r.origin.x = (float) x;
            r.origin.y = (float) y;
            r.size.width = (float) owner->getWidth();
            r.size.height = (float) owner->getHeight();
            r.origin.y = [[view superview] frame].size.height - (r.origin.y + r.size.height);

            [view setFrame: r];
        }
    }

    void componentPeerChanged()
    {
        NSViewComponentPeer* const peer = dynamic_cast <NSViewComponentPeer*> (owner->getPeer());

        if (currentPeer != peer)
        {
            [view removeFromSuperview];
            currentPeer = peer;

            if (peer != 0)
            {
                [peer->view addSubview: view];
                componentMovedOrResized (false, false);
            }
        }

        [view setHidden: ! owner->isShowing()];
    }

    void componentVisibilityChanged (Component&)
    {
        componentPeerChanged();
    }

    juce_UseDebuggingNewOperator

private:
    NSViewComponentInternal (const NSViewComponentInternal&);
    const NSViewComponentInternal& operator= (const NSViewComponentInternal&);
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
        if (view != 0)
            info = new NSViewComponentInternal ((NSView*) view, this);
        else
            info = 0;
    }
}

void* NSViewComponent::getView() const
{
    return info == 0 ? 0 : info->view;
}

void NSViewComponent::paint (Graphics& g)
{
}

#endif
