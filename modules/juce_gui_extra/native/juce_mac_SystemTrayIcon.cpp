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

extern NSMenu* createNSMenu (const PopupMenu&, const String& name, int topLevelMenuId,
                             int topLevelIndex, bool addDelegate);

class SystemTrayIconComponent::Pimpl  : private Timer
{
public:
    Pimpl (SystemTrayIconComponent& iconComp, const Image& im)
        : owner (iconComp), statusIcon (imageToNSImage (im))
    {
        static SystemTrayViewClass cls;
        view = [cls.createInstance() init];
        SystemTrayViewClass::setOwner (view, this);
        SystemTrayViewClass::setImage (view, statusIcon);

        setIconSize();

        statusItem = [[[NSStatusBar systemStatusBar] statusItemWithLength: NSSquareStatusItemLength] retain];
        [statusItem setView: view];

        SystemTrayViewClass::frameChanged (view, SEL(), nullptr);

        [[NSNotificationCenter defaultCenter]  addObserver: view
                                                  selector: @selector (frameChanged:)
                                                      name: NSWindowDidMoveNotification
                                                    object: nil];
    }

    ~Pimpl()
    {
        [[NSNotificationCenter defaultCenter]  removeObserver: view];
        [[NSStatusBar systemStatusBar] removeStatusItem: statusItem];
        SystemTrayViewClass::setOwner (view, nullptr);
        SystemTrayViewClass::setImage (view, nil);
        [statusItem release];
        [view release];
        [statusIcon release];
    }

    void updateIcon (const Image& newImage)
    {
        [statusIcon release];
        statusIcon = imageToNSImage (newImage);
        setIconSize();
        SystemTrayViewClass::setImage (view, statusIcon);
        [statusItem setView: view];
    }

    void setHighlighted (bool shouldHighlight)
    {
        isHighlighted = shouldHighlight;
        [view setNeedsDisplay: true];
    }

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
                                             pressure, MouseInputSource::invalidOrientation, MouseInputSource::invalidRotation,
                                             MouseInputSource::invalidTiltX, MouseInputSource::invalidTiltY,
                                             &owner, &owner, now, {}, now, 1, false));

                owner.mouseUp (MouseEvent (mouseSource, {}, eventMods.withoutMouseButtons(), pressure,
                                           MouseInputSource::invalidOrientation, MouseInputSource::invalidRotation,
                                           MouseInputSource::invalidTiltX, MouseInputSource::invalidTiltY,
                                           &owner, &owner, now, {}, now, 1, false));
            }
            else if (type == NSEventTypeMouseMoved)
            {
                owner.mouseMove (MouseEvent (mouseSource, {}, eventMods, pressure,
                                             MouseInputSource::invalidOrientation, MouseInputSource::invalidRotation,
                                             MouseInputSource::invalidTiltX, MouseInputSource::invalidTiltY,
                                             &owner, &owner, now, {}, now, 1, false));
            }
        }
    }

    void showMenu (const PopupMenu& menu)
    {
        if (NSMenu* m = createNSMenu (menu, "MenuBarItem", -2, -3, true))
        {
            setHighlighted (true);
            stopTimer();
            [statusItem popUpStatusItemMenu: m];
            startTimer (1);
        }
    }

    SystemTrayIconComponent& owner;
    NSStatusItem* statusItem = nil;

private:
    NSImage* statusIcon = nil;
    NSControl* view = nil;
    bool isHighlighted = false;

    void setIconSize()
    {
        [statusIcon setSize: NSMakeSize (20.0f, 20.0f)];
    }

    void timerCallback() override
    {
        stopTimer();
        setHighlighted (false);
    }

    struct SystemTrayViewClass : public ObjCClass<NSControl>
    {
        SystemTrayViewClass()  : ObjCClass<NSControl> ("JUCESystemTrayView_")
        {
            addIvar<Pimpl*> ("owner");
            addIvar<NSImage*> ("image");

            addMethod (@selector (mouseDown:),      handleEventDown, "v@:@");
            addMethod (@selector (rightMouseDown:), handleEventDown, "v@:@");
            addMethod (@selector (drawRect:),       drawRect,        "v@:@");
            addMethod (@selector (frameChanged:),   frameChanged,    "v@:@");

            registerClass();
        }

        static Pimpl* getOwner (id self)                { return getIvar<Pimpl*> (self, "owner"); }
        static NSImage* getImage (id self)              { return getIvar<NSImage*> (self, "image"); }
        static void setOwner (id self, Pimpl* owner)    { object_setInstanceVariable (self, "owner", owner); }
        static void setImage (id self, NSImage* image)  { object_setInstanceVariable (self, "image", image); }

        static void frameChanged (id self, SEL, NSNotification*)
        {
            if (auto* owner = getOwner (self))
            {
                NSRect r = [[[owner->statusItem view] window] frame];
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
                [owner->statusItem drawStatusBarBackgroundInRect: bounds
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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Pimpl)
};


//==============================================================================
void SystemTrayIconComponent::setIconImage (const Image& newImage)
{
    if (newImage.isValid())
    {
        if (pimpl == nullptr)
            pimpl.reset (new Pimpl (*this, newImage));
        else
            pimpl->updateIcon (newImage);
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

void SystemTrayIconComponent::setHighlighted (bool highlight)
{
    if (pimpl != nullptr)
        pimpl->setHighlighted (highlight);
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
    return pimpl != nullptr ? pimpl->statusItem : nullptr;
}

void SystemTrayIconComponent::showDropdownMenu (const PopupMenu& menu)
{
    if (pimpl != nullptr)
        pimpl->showMenu (menu);
}

} // namespace juce
