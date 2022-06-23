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

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wdeprecated-declarations")

extern NSMenu* createNSMenu (const PopupMenu&, const String& name, int topLevelMenuId,
                             int topLevelIndex, bool addDelegate);

//==============================================================================
struct StatusItemContainer   : public Timer
{
    //==============================================================================
    StatusItemContainer (SystemTrayIconComponent& iconComp, const Image& im)
        : owner (iconComp), statusIcon (imageToNSImage (ScaledImage (im)))
    {
    }

    virtual void configureIcon() = 0;
    virtual void setHighlighted (bool shouldHighlight) = 0;

    //==============================================================================
    void setIconSize()
    {
        [statusIcon.get() setSize: NSMakeSize (20.0f, 20.0f)];
    }

    void updateIcon (const Image& newImage)
    {
        statusIcon.reset (imageToNSImage (ScaledImage (newImage)));
        setIconSize();
        configureIcon();
    }

    void showMenu (const PopupMenu& menu)
    {
        if (NSMenu* m = createNSMenu (menu, "MenuBarItem", -2, -3, true))
        {
            setHighlighted (true);
            stopTimer();

            // There's currently no good alternative to this.
            [statusItem.get() popUpStatusItemMenu: m];

            startTimer (1);
        }
    }

    //==============================================================================
    void timerCallback() override
    {
        stopTimer();
        setHighlighted (false);
    }

    //==============================================================================
    SystemTrayIconComponent& owner;

    NSUniquePtr<NSStatusItem> statusItem;
    NSUniquePtr<NSImage> statusIcon;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StatusItemContainer)
};

//==============================================================================
struct API_AVAILABLE (macos (10.10)) ButtonBasedStatusItem : public StatusItemContainer
{
    //==============================================================================
    ButtonBasedStatusItem (SystemTrayIconComponent& iconComp, const Image& im)
        : StatusItemContainer (iconComp, im)
    {
        static ButtonEventForwarderClass cls;
        eventForwarder.reset ([cls.createInstance() init]);
        ButtonEventForwarderClass::setOwner (eventForwarder.get(), this);

        setIconSize();
        configureIcon();

        statusItem.reset ([[[NSStatusBar systemStatusBar] statusItemWithLength: NSSquareStatusItemLength] retain]);
        auto button = [statusItem.get() button];
        button.image = statusIcon.get();
        button.target = eventForwarder.get();
        button.action = @selector (handleEvent:);
       #if defined (MAC_OS_X_VERSION_10_12) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_12
        [button sendActionOn: NSEventMaskLeftMouseDown | NSEventMaskRightMouseDown | NSEventMaskScrollWheel];
       #else
        [button sendActionOn: NSLeftMouseDownMask | NSRightMouseDownMask | NSScrollWheelMask];
       #endif
    }

    ~ButtonBasedStatusItem() override
    {
        [statusItem.get() button].image = nullptr;
    }

    void configureIcon() override
    {
        [statusIcon.get() setTemplate: true];
        [statusItem.get() button].image = statusIcon.get();
    }

    void setHighlighted (bool shouldHighlight) override
    {
        [[statusItem.get() button] setHighlighted: shouldHighlight];
    }

    //==============================================================================
    void handleEvent()
    {
        auto e = [NSApp currentEvent];
        NSEventType type = [e type];

        const bool isLeft  = (type == NSEventTypeLeftMouseDown);
        const bool isRight = (type == NSEventTypeRightMouseDown);

        if (owner.isCurrentlyBlockedByAnotherModalComponent())
        {
            if (isLeft || isRight)
                if (auto* current = Component::getCurrentlyModalComponent())
                    current->inputAttemptWhenModal();
        }
        else
        {
            auto eventMods = ComponentPeer::getCurrentModifiersRealtime();

            if (([e modifierFlags] & NSEventModifierFlagCommand) != 0)
                eventMods = eventMods.withFlags (ModifierKeys::commandModifier);

            auto now = Time::getCurrentTime();
            auto mouseSource = Desktop::getInstance().getMainMouseSource();
            auto pressure = (float) e.pressure;

            if (isLeft || isRight)
            {
                owner.mouseDown ({ mouseSource, {},
                                   eventMods.withFlags (isLeft ? ModifierKeys::leftButtonModifier
                                                               : ModifierKeys::rightButtonModifier),
                                   pressure,
                                   MouseInputSource::defaultOrientation, MouseInputSource::defaultRotation,
                                   MouseInputSource::defaultTiltX, MouseInputSource::defaultTiltY,
                                   &owner, &owner, now, {}, now, 1, false });

                owner.mouseUp   ({ mouseSource, {},
                                   eventMods.withoutMouseButtons(),
                                   pressure,
                                   MouseInputSource::defaultOrientation, MouseInputSource::defaultRotation,
                                   MouseInputSource::defaultTiltX, MouseInputSource::defaultTiltY,
                                   &owner, &owner, now, {}, now, 1, false });
            }
            else if (type == NSEventTypeMouseMoved)
            {
                owner.mouseMove (MouseEvent (mouseSource, {}, eventMods, pressure,
                                             MouseInputSource::defaultOrientation, MouseInputSource::defaultRotation,
                                             MouseInputSource::defaultTiltX, MouseInputSource::defaultTiltY,
                                             &owner, &owner, now, {}, now, 1, false));
            }
        }
    }

    //==============================================================================
    class ButtonEventForwarderClass   : public ObjCClass<NSObject>
    {
    public:
        ButtonEventForwarderClass() : ObjCClass<NSObject> ("JUCEButtonEventForwarderClass_")
        {
            addIvar<ButtonBasedStatusItem*> ("owner");

            addMethod (@selector (handleEvent:), handleEvent);

            registerClass();
        }

        static ButtonBasedStatusItem* getOwner (id self)               { return getIvar<ButtonBasedStatusItem*> (self, "owner"); }
        static void setOwner (id self, ButtonBasedStatusItem* owner)   { object_setInstanceVariable (self, "owner", owner); }

    private:
        static void handleEvent (id self, SEL, id)
        {
            if (auto* owner = getOwner (self))
                owner->handleEvent();
        }
    };

    //==============================================================================
    NSUniquePtr<NSObject> eventForwarder;
};

//==============================================================================
struct ViewBasedStatusItem   : public StatusItemContainer
{
    //==============================================================================
    ViewBasedStatusItem (SystemTrayIconComponent& iconComp, const Image& im)
        : StatusItemContainer (iconComp, im)
    {
        static SystemTrayViewClass cls;
        view.reset ([cls.createInstance() init]);
        SystemTrayViewClass::setOwner (view.get(), this);
        SystemTrayViewClass::setImage (view.get(), statusIcon.get());

        setIconSize();

        statusItem.reset ([[[NSStatusBar systemStatusBar] statusItemWithLength: NSSquareStatusItemLength] retain]);
        [statusItem.get() setView: view.get()];

        SystemTrayViewClass::frameChanged (view.get(), SEL(), nullptr);

        JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wundeclared-selector")
        [[NSNotificationCenter defaultCenter]  addObserver: view.get()
                                                  selector: @selector (frameChanged:)
                                                      name: NSWindowDidMoveNotification
                                                    object: nil];
        JUCE_END_IGNORE_WARNINGS_GCC_LIKE
    }

    ~ViewBasedStatusItem() override
    {
        [[NSNotificationCenter defaultCenter] removeObserver: view.get()];
        [[NSStatusBar systemStatusBar] removeStatusItem: statusItem.get()];
        SystemTrayViewClass::setOwner (view.get(), nullptr);
        SystemTrayViewClass::setImage (view.get(), nil);
    }

    void configureIcon() override
    {
        SystemTrayViewClass::setImage (view.get(), statusIcon.get());
        [statusItem.get() setView: view.get()];
    }

    void setHighlighted (bool shouldHighlight) override
    {
        isHighlighted = shouldHighlight;
        [view.get() setNeedsDisplay: true];
    }

    //==============================================================================
    void handleStatusItemAction (NSEvent* e)
    {
        NSEventType type = [e type];

        const bool isLeft  = (type == NSEventTypeLeftMouseDown  || type == NSEventTypeLeftMouseUp);
        const bool isRight = (type == NSEventTypeRightMouseDown || type == NSEventTypeRightMouseUp);

        if (owner.isCurrentlyBlockedByAnotherModalComponent())
        {
            if (isLeft || isRight)
                if (auto* current = Component::getCurrentlyModalComponent())
                    current->inputAttemptWhenModal();
        }
        else
        {
            auto eventMods = ComponentPeer::getCurrentModifiersRealtime();

            if (([e modifierFlags] & NSEventModifierFlagCommand) != 0)
                eventMods = eventMods.withFlags (ModifierKeys::commandModifier);

            auto now = Time::getCurrentTime();
            auto mouseSource = Desktop::getInstance().getMainMouseSource();
            auto pressure = (float) e.pressure;

            if (isLeft || isRight)  // Only mouse up is sent by the OS, so simulate a down/up
            {
                setHighlighted (true);
                startTimer (150);

                owner.mouseDown (MouseEvent (mouseSource, {},
                                             eventMods.withFlags (isLeft ? ModifierKeys::leftButtonModifier
                                                                         : ModifierKeys::rightButtonModifier),
                                             pressure, MouseInputSource::defaultOrientation, MouseInputSource::defaultRotation,
                                             MouseInputSource::defaultTiltX, MouseInputSource::defaultTiltY,
                                             &owner, &owner, now, {}, now, 1, false));

                owner.mouseUp (MouseEvent (mouseSource, {}, eventMods.withoutMouseButtons(), pressure,
                                           MouseInputSource::defaultOrientation, MouseInputSource::defaultRotation,
                                           MouseInputSource::defaultTiltX, MouseInputSource::defaultTiltY,
                                           &owner, &owner, now, {}, now, 1, false));
            }
            else if (type == NSEventTypeMouseMoved)
            {
                owner.mouseMove (MouseEvent (mouseSource, {}, eventMods, pressure,
                                             MouseInputSource::defaultOrientation, MouseInputSource::defaultRotation,
                                             MouseInputSource::defaultTiltX, MouseInputSource::defaultTiltY,
                                             &owner, &owner, now, {}, now, 1, false));
            }
        }
    }

    //==============================================================================
    struct SystemTrayViewClass : public ObjCClass<NSControl>
    {
        SystemTrayViewClass()  : ObjCClass<NSControl> ("JUCESystemTrayView_")
        {
            addIvar<ViewBasedStatusItem*> ("owner");
            addIvar<NSImage*> ("image");

            addMethod (@selector (mouseDown:),      handleEventDown);
            addMethod (@selector (rightMouseDown:), handleEventDown);
            addMethod (@selector (drawRect:),       drawRect);

            JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wundeclared-selector")
            addMethod (@selector (frameChanged:),   frameChanged);
            JUCE_END_IGNORE_WARNINGS_GCC_LIKE

            registerClass();
        }

        static ViewBasedStatusItem* getOwner (id self)               { return getIvar<ViewBasedStatusItem*> (self, "owner"); }
        static NSImage* getImage (id self)                           { return getIvar<NSImage*> (self, "image"); }
        static void setOwner (id self, ViewBasedStatusItem* owner)   { object_setInstanceVariable (self, "owner", owner); }
        static void setImage (id self, NSImage* image)               { object_setInstanceVariable (self, "image", image); }

        static void frameChanged (id self, SEL, NSNotification*)
        {
            if (auto* owner = getOwner (self))
            {
                NSRect r = [[[owner->statusItem.get() view] window] frame];
                NSRect sr = [[[NSScreen screens] objectAtIndex: 0] frame];
                r.origin.y = sr.size.height - r.origin.y - r.size.height;
                owner->owner.setBounds (convertToRectInt (r));
            }
        }

    private:
        static void handleEventDown (id self, SEL, NSEvent* e)
        {
            if (auto* owner = getOwner (self))
                owner->handleStatusItemAction (e);
        }

        static void drawRect (id self, SEL, NSRect)
        {
            NSRect bounds = [self bounds];

            if (auto* owner = getOwner (self))
                [owner->statusItem.get() drawStatusBarBackgroundInRect: bounds
                                                         withHighlight: owner->isHighlighted];

            if (NSImage* const im = getImage (self))
            {
                NSSize imageSize = [im size];

                [im drawInRect: NSMakeRect (bounds.origin.x + ((bounds.size.width  - imageSize.width)  / 2.0f),
                                            bounds.origin.y + ((bounds.size.height - imageSize.height) / 2.0f),
                                            imageSize.width, imageSize.height)
                      fromRect: NSZeroRect
                     operation: NSCompositingOperationSourceOver
                      fraction: 1.0f];
            }
        }
    };

    //==============================================================================
    NSUniquePtr<NSControl> view;
    bool isHighlighted = false;
};

//==============================================================================
class SystemTrayIconComponent::Pimpl
{
public:
    //==============================================================================
    Pimpl (SystemTrayIconComponent& iconComp, const Image& im)
    {
        if (@available (macOS 10.10, *))
            statusItemHolder = std::make_unique<ButtonBasedStatusItem> (iconComp, im);
        else
            statusItemHolder = std::make_unique<ViewBasedStatusItem> (iconComp, im);
    }

    //==============================================================================
    std::unique_ptr<StatusItemContainer> statusItemHolder;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Pimpl)
};

//==============================================================================
void SystemTrayIconComponent::setIconImage (const Image&, const Image& templateImage)
{
    if (templateImage.isValid())
    {
        if (pimpl == nullptr)
            pimpl.reset (new Pimpl (*this, templateImage));
        else
            pimpl->statusItemHolder->updateIcon (templateImage);
    }
    else
    {
        pimpl.reset();
    }
}

void SystemTrayIconComponent::setIconTooltip (const String&)
{
    // xxx not yet implemented!
}

void SystemTrayIconComponent::setHighlighted (bool shouldHighlight)
{
    if (pimpl != nullptr)
        pimpl->statusItemHolder->setHighlighted (shouldHighlight);
}

void SystemTrayIconComponent::showInfoBubble (const String& /*title*/, const String& /*content*/)
{
    // xxx Not implemented!
}

void SystemTrayIconComponent::hideInfoBubble()
{
    // xxx Not implemented!
}

void* SystemTrayIconComponent::getNativeHandle() const
{
    return pimpl != nullptr ? pimpl->statusItemHolder->statusItem.get() : nullptr;
}

void SystemTrayIconComponent::showDropdownMenu (const PopupMenu& menu)
{
    if (pimpl != nullptr)
        pimpl->statusItemHolder->showMenu (menu);
}

JUCE_END_IGNORE_WARNINGS_GCC_LIKE

} // namespace juce
