/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

// (This file gets included by juce_mac_NativeCode.mm, rather than being
// compiled on its own).
#ifdef JUCE_INCLUDED_FILE

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
    : info (0)
{
}

NSViewComponent::~NSViewComponent()
{
    delete info;
}

void NSViewComponent::setView (void* view)
{
    if (view != getView())
    {
        deleteAndZero (info);

        if (view != 0)
            info = new NSViewComponentInternal ((NSView*) view, this);
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
