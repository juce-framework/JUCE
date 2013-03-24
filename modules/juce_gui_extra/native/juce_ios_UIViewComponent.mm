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

class UIViewComponent::Pimpl  : public ComponentMovementWatcher
{
public:
    Pimpl (UIView* const v, Component& comp)
        : ComponentMovementWatcher (&comp),
          view (v),
          owner (comp),
          currentPeer (nullptr)
    {
        [view retain];

        if (owner.isShowing())
            componentPeerChanged();
    }

    ~Pimpl()
    {
        [view removeFromSuperview];
        [view release];
    }

    void componentMovedOrResized (bool /*wasMoved*/, bool /*wasResized*/)
    {
        Component* const topComp = owner.getTopLevelComponent();

        if (topComp->getPeer() != nullptr)
        {
            const Point<int> pos (topComp->getLocalPoint (&owner, Point<int>()));

            [view setFrame: CGRectMake ((float) pos.x, (float) pos.y,
                                        (float) owner.getWidth(), (float) owner.getHeight())];
        }
    }

    void componentPeerChanged()
    {
        ComponentPeer* const peer = owner.getPeer();

        if (currentPeer != peer)
        {
            if ([view superview] != nil)
                [view removeFromSuperview]; // Must be careful not to call this unless it's required - e.g. some Apple AU views
                                            // override the call and use it as a sign that they're being deleted, which breaks everything..
            currentPeer = peer;

            if (peer != nullptr)
            {
                UIView* peerView = (UIView*) peer->getNativeHandle();
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
        CGRect r = [view frame];
        return Rectangle<int> ((int) r.size.width, (int) r.size.height);
    }

    UIView* const view;

private:
    Component& owner;
    ComponentPeer* currentPeer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Pimpl)
};

//==============================================================================
UIViewComponent::UIViewComponent() {}
UIViewComponent::~UIViewComponent() {}

void UIViewComponent::setView (void* const view)
{
    if (view != getView())
    {
        pimpl = nullptr;

        if (view != nullptr)
            pimpl = new Pimpl ((UIView*) view, *this);
    }
}

void* UIViewComponent::getView() const
{
    return pimpl == nullptr ? nullptr : pimpl->view;
}

void UIViewComponent::resizeToFitView()
{
    if (pimpl != nullptr)
        setBounds (pimpl->getViewBounds());
}

void UIViewComponent::paint (Graphics&) {}
