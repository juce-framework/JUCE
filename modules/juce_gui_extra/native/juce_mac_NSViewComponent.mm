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

class NSViewAttachment  : public ReferenceCountedObject,
                          public ComponentMovementWatcher
{
public:
    NSViewAttachment (NSView* const v, Component& comp)
        : ComponentMovementWatcher (&comp),
          view (v), owner (comp),
          currentPeer (nullptr), frameChangeCallback (nullptr)
    {
        [view retain];
        [view setPostsFrameChangedNotifications: YES];

        if (owner.isShowing())
            componentPeerChanged();

        static ViewFrameChangeCallbackClass cls;
        frameChangeCallback = [cls.createInstance() init];
        ViewFrameChangeCallbackClass::setTarget (frameChangeCallback, &owner);

        [[NSNotificationCenter defaultCenter]  addObserver: frameChangeCallback
                                                  selector: @selector (frameChanged:)
                                                      name: NSViewFrameDidChangeNotification
                                                    object: view];
    }

    ~NSViewAttachment()
    {
        [[NSNotificationCenter defaultCenter] removeObserver: frameChangeCallback];
        [frameChangeCallback release];

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
        if (ComponentPeer* const peer = owner.getTopLevelComponent()->getPeer())
        {
            NSRect r = makeNSRect (peer->getAreaCoveredBy (owner));
            r.origin.y = [[view superview] frame].size.height - (r.origin.y + r.size.height);
            [view setFrame: r];
        }
    }

    void componentPeerChanged() override
    {
        ComponentPeer* const peer = owner.getPeer();

        if (currentPeer != peer)
        {
            removeFromParent();
            currentPeer = peer;

            if (peer != nullptr)
            {
                NSView* const peerView = (NSView*) peer->getNativeHandle();
                [peerView addSubview: view];
                componentMovedOrResized (false, false);
            }
        }

        [view setHidden: ! owner.isShowing()];
    }

    void componentVisibilityChanged() override
    {
        componentPeerChanged();
    }

    NSView* const view;

private:
    Component& owner;
    ComponentPeer* currentPeer;
    id frameChangeCallback;

    void removeFromParent()
    {
        if ([view superview] != nil)
            [view removeFromSuperview]; // Must be careful not to call this unless it's required - e.g. some Apple AU views
                                        // override the call and use it as a sign that they're being deleted, which breaks everything..
    }

    //==============================================================================
    struct ViewFrameChangeCallbackClass   : public ObjCClass<NSObject>
    {
        ViewFrameChangeCallbackClass()  : ObjCClass<NSObject> ("JUCE_NSViewCallback_")
        {
            addIvar<Component*> ("target");
            addMethod (@selector (frameChanged:),  frameChanged, "v@:@");
            registerClass();
        }

        static void setTarget (id self, Component* c)
        {
            object_setInstanceVariable (self, "target", c);
        }

    private:
        static void frameChanged (id self, SEL, NSNotification*)
        {
            if (Component* const target = getIvar<Component*> (self, "target"))
                target->childBoundsChanged (nullptr);
        }

        JUCE_DECLARE_NON_COPYABLE (ViewFrameChangeCallbackClass);
    };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NSViewAttachment)
};

//==============================================================================
NSViewComponent::NSViewComponent() {}
NSViewComponent::~NSViewComponent() {}

void NSViewComponent::setView (void* const view)
{
    if (view != getView())
    {
        attachment = nullptr;

        if (view != nullptr)
            attachment = attachViewToComponent (*this, view);
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
        NSRect r = [static_cast<NSViewAttachment*> (attachment.get())->view frame];
        setBounds (Rectangle<int> ((int) r.size.width, (int) r.size.height));
    }
}

void NSViewComponent::paint (Graphics&) {}

ReferenceCountedObject* NSViewComponent::attachViewToComponent (Component& comp, void* const view)
{
    return new NSViewAttachment ((NSView*) view, comp);
}
