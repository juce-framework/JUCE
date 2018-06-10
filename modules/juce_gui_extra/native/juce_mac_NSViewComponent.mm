/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

struct NSViewResizeWatcher
{
    NSViewResizeWatcher() : callback (nil) {}

    virtual ~NSViewResizeWatcher()
    {
        // must call detachViewWatcher() first
        jassert (callback == nil);
    }

    void attachViewWatcher (NSView* view)
    {
        static ViewFrameChangeCallbackClass cls;
        callback = [cls.createInstance() init];
        ViewFrameChangeCallbackClass::setTarget (callback, this);

        [[NSNotificationCenter defaultCenter]  addObserver: callback
                                                  selector: @selector (frameChanged:)
                                                      name: NSViewFrameDidChangeNotification
                                                    object: view];
    }

    void detachViewWatcher()
    {
        if (callback != nil)
        {
            [[NSNotificationCenter defaultCenter] removeObserver: callback];
            [callback release];
            callback = nil;
        }
    }

    virtual void viewResized() = 0;

private:
    id callback;

    //==============================================================================
    struct ViewFrameChangeCallbackClass   : public ObjCClass<NSObject>
    {
        ViewFrameChangeCallbackClass()  : ObjCClass<NSObject> ("JUCE_NSViewCallback_")
        {
            addIvar<NSViewResizeWatcher*> ("target");
            addMethod (@selector (frameChanged:),  frameChanged, "v@:@");
            registerClass();
        }

        static void setTarget (id self, NSViewResizeWatcher* c)
        {
            object_setInstanceVariable (self, "target", c);
        }

    private:
        static void frameChanged (id self, SEL, NSNotification*)
        {
            if (auto* target = getIvar<NSViewResizeWatcher*> (self, "target"))
                target->viewResized();
        }

        JUCE_DECLARE_NON_COPYABLE (ViewFrameChangeCallbackClass)
    };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NSViewResizeWatcher)
};

//==============================================================================
class NSViewAttachment  : public ReferenceCountedObject,
                          public ComponentMovementWatcher,
                          private NSViewResizeWatcher
{
public:
    NSViewAttachment (NSView* v, Component& comp)
        : ComponentMovementWatcher (&comp),
          view (v), owner (comp),
          currentPeer (nullptr)
    {
        [view retain];
        [view setPostsFrameChangedNotifications: YES];
        updateAlpha();

        if (owner.isShowing())
            componentPeerChanged();

        attachViewWatcher (view);
    }

    ~NSViewAttachment()
    {
        detachViewWatcher();
        removeFromParent();
        [view release];
    }

    void componentMovedOrResized (Component& comp, bool wasMoved, bool wasResized) override
    {
        ComponentMovementWatcher::componentMovedOrResized (comp, wasMoved, wasResized);

        // The ComponentMovementWatcher version of this method avoids calling
        // us when the top-level comp is resized, but for an NSView we need to know this
        // because with inverted coordinates, we need to update the position even if the
        // top-left pos hasn't changed
        if (comp.isOnDesktop() && wasResized)
            componentMovedOrResized (wasMoved, wasResized);
    }

    void componentMovedOrResized (bool /*wasMoved*/, bool /*wasResized*/) override
    {
        if (auto* peer = owner.getTopLevelComponent()->getPeer())
        {
            auto r = makeNSRect (peer->getAreaCoveredBy (owner));
            r.origin.y = [[view superview] frame].size.height - (r.origin.y + r.size.height);
            [view setFrame: r];
        }
    }

    void componentPeerChanged() override
    {
        auto* peer = owner.getPeer();

        if (currentPeer != peer)
        {
            currentPeer = peer;

            if (peer != nullptr)
            {
                auto peerView = (NSView*) peer->getNativeHandle();
                [peerView addSubview: view];
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

    void viewResized() override
    {
        owner.childBoundsChanged (nullptr);
    }

    void updateAlpha()
    {
        [view setAlphaValue: (CGFloat) owner.getAlpha()];
    }

    NSView* const view;

    using Ptr = ReferenceCountedObjectPtr<NSViewAttachment>;

private:
    Component& owner;
    ComponentPeer* currentPeer;

    void removeFromParent()
    {
        if ([view superview] != nil)
            [view removeFromSuperview]; // Must be careful not to call this unless it's required - e.g. some Apple AU views
                                        // override the call and use it as a sign that they're being deleted, which breaks everything..
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NSViewAttachment)
};

//==============================================================================
NSViewComponent::NSViewComponent() {}
NSViewComponent::~NSViewComponent() {}

void NSViewComponent::setView (void* view)
{
    if (view != getView())
    {
        auto old = attachment;

        attachment = nullptr;

        if (view != nullptr)
            attachment = attachViewToComponent (*this, view);

        old = nullptr;
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
        auto r = [static_cast<NSViewAttachment*> (attachment.get())->view frame];
        setBounds (Rectangle<int> ((int) r.size.width, (int) r.size.height));
    }
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

} // namespace juce
