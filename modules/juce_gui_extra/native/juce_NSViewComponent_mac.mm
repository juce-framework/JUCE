/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
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

class NSViewAttachment  : public ReferenceCountedObject,
                          public ComponentMovementWatcher
{
public:
    NSViewAttachment (NSView* v, Component& comp)
        : ComponentMovementWatcher (&comp),
          view (v), owner (comp)
    {
        [view retain];
        [view setPostsFrameChangedNotifications: YES];
        updateAlpha();

        if (owner.isShowing())
            componentPeerChanged();
    }

    ~NSViewAttachment() override
    {
        removeFromParent();
        [view release];
    }

    void componentMovedOrResized (Component& comp, bool wasMoved, bool wasResized) override
    {
        ComponentMovementWatcher::componentMovedOrResized (comp, wasMoved, wasResized);

        // The ComponentMovementWatcher version of this method avoids calling
        // us when the top-level comp is resized, but if we're listening to the
        // top-level comp we still want the NSView to track its size.
        if (comp.isOnDesktop() && wasResized)
            componentMovedOrResized (wasMoved, wasResized);
    }

    void componentMovedOrResized (bool /*wasMoved*/, bool /*wasResized*/) override
    {
        if (auto* peer = owner.getTopLevelComponent()->getPeer())
        {
            const auto newArea = peer->getAreaCoveredBy (owner);

            if (convertToRectInt ([view frame]) != newArea)
                [view setFrame: makeNSRect (newArea)];
        }
    }

    void componentPeerChanged() override
    {
        auto* peer = owner.getPeer();

        if (std::exchange (currentPeer, peer) != peer)
        {
            if (peer != nullptr)
            {
                auto peerView = (NSView*) peer->getNativeHandle();
                [peerView addSubview: view];

                if (@available (macOS 10.10, *))
                {
                    previousAccessibilityParent = [view accessibilityParent];
                    [view setAccessibilityParent:static_cast<id> (owner.getAccessibilityHandler()->getNativeImplementation())];
                }

                componentMovedOrResized (false, false);
            }
            else
            {
                removeFromParent();
            }
        }

        [view setHidden: ! owner.isShowing()];
    }

    void componentVisibilityChanged() override
    {
        componentPeerChanged();
    }

    void updateAlpha()
    {
        [view setAlphaValue: (CGFloat) owner.getAlpha()];
    }

    NSView* const view;

    using Ptr = ReferenceCountedObjectPtr<NSViewAttachment>;

private:
    Component& owner;
    ComponentPeer* currentPeer = nullptr;
    NSViewFrameWatcher frameWatcher { view, [this] { owner.childBoundsChanged (nullptr); } };
    id previousAccessibilityParent = nil;

    void removeFromParent()
    {
        if (@available (macOS 10.10, *))
            if (previousAccessibilityParent != nil)
                [view setAccessibilityParent: previousAccessibilityParent];

        if ([view superview] != nil)
            [view removeFromSuperview]; // Must be careful not to call this unless it's required - e.g. some Apple AU views
                                        // override the call and use it as a sign that they're being deleted, which breaks everything..
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NSViewAttachment)
};

//==============================================================================
NSViewComponent::NSViewComponent() = default;
NSViewComponent::~NSViewComponent()
{
    AccessibilityHandler::setNativeChildForComponent (*this, nullptr);
}

void NSViewComponent::setView (void* view)
{
    if (view != getView())
    {
        auto old = attachment;

        attachment = nullptr;

        if (view != nullptr)
            attachment = attachViewToComponent (*this, view);

        old = nullptr;

        AccessibilityHandler::setNativeChildForComponent (*this, getView());
    }
}

void* NSViewComponent::getView() const
{
    return attachment != nullptr ? static_cast<NSViewAttachment*> (attachment.get())->view
                                 : nullptr;
}

void NSViewComponent::resizeToFitView()
{
    if (attachment != nullptr)
    {
        auto* view = static_cast<NSViewAttachment*> (attachment.get())->view;
        auto r = [view frame];
        setBounds (Rectangle<int> ((int) r.size.width, (int) r.size.height));

        if (auto* peer = getTopLevelComponent()->getPeer())
        {
            const auto position = peer->getAreaCoveredBy (*this).getPosition();
            [view setFrameOrigin: convertToCGPoint (position)];
        }
    }
}

void NSViewComponent::resizeViewToFit()
{
    if (attachment != nullptr)
        static_cast<NSViewAttachment*> (attachment.get())->componentMovedOrResized (true, true);
}

void NSViewComponent::paint (Graphics&) {}

void NSViewComponent::alphaChanged()
{
    if (attachment != nullptr)
        (static_cast<NSViewAttachment*> (attachment.get()))->updateAlpha();
}

ReferenceCountedObject* NSViewComponent::attachViewToComponent (Component& comp, void* view)
{
    return new NSViewAttachment ((NSView*) view, comp);
}

std::unique_ptr<AccessibilityHandler> NSViewComponent::createAccessibilityHandler()
{
    return std::make_unique<AccessibilityHandler> (*this, AccessibilityRole::group);
}

} // namespace juce
