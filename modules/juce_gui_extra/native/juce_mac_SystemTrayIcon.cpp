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
    //==============================================================================
    Pimpl (SystemTrayIconComponent& iconComp, const Image& im)
        : owner (iconComp), statusIcon (imageToNSImage (im))
    {
        static ButtonEventForwarderClass cls;
        eventForwarder.reset ([cls.createInstance() init]);
        ButtonEventForwarderClass::setOwner (eventForwarder.get(), this);

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

    //==============================================================================
    void updateIcon (const Image& newImage)
    {
        statusIcon.reset (imageToNSImage (newImage));
        configureIcon();
        [statusItem.get() button].image = statusIcon.get();
    }

    void setHighlighted (bool shouldHighlight)
    {
        [[statusItem.get() button] setHighlighted: shouldHighlight];
    }

    void showMenu (const PopupMenu& menu)
    {
        if (NSMenu* m = createNSMenu (menu, "MenuBarItem", -2, -3, true))
        {
            setHighlighted (true);
            stopTimer();

            // There's currently no good alternative to this...
           #if JUCE_CLANG && ! (defined (MAC_OS_X_VERSION_10_16) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_16)
            #pragma clang diagnostic push
            #pragma clang diagnostic ignored "-Wdeprecated-declarations"
            #define JUCE_DEPRECATION_IGNORED 1
           #endif

            [statusItem.get() popUpStatusItemMenu: m];

           #if JUCE_DEPRECATION_IGNORED
            #pragma clang diagnostic pop
            #undef JUCE_DEPRECATION_IGNORED
           #endif

            startTimer (1);
        }
    }

    //==============================================================================
    NSStatusItem* getStatusItem()
    {
        return statusItem.get();
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
                                   MouseInputSource::invalidOrientation, MouseInputSource::invalidRotation,
                                   MouseInputSource::invalidTiltX, MouseInputSource::invalidTiltY,
                                   &owner, &owner, now, {}, now, 1, false });

                owner.mouseUp   ({ mouseSource, {},
                                   eventMods.withoutMouseButtons(),
                                   pressure,
                                   MouseInputSource::invalidOrientation, MouseInputSource::invalidRotation,
                                   MouseInputSource::invalidTiltX, MouseInputSource::invalidTiltY,
                                   &owner, &owner, now, {}, now, 1, false });
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

private:
    //==============================================================================
    void configureIcon()
    {
        [statusIcon.get() setSize: NSMakeSize (20.0f, 20.0f)];
        [statusIcon.get() setTemplate: true];
    }

    void timerCallback() override
    {
        stopTimer();
        setHighlighted (false);
    }

    //==============================================================================
    class ButtonEventForwarderClass   : public ObjCClass<NSObject>
    {
    public:
        ButtonEventForwarderClass() : ObjCClass<NSObject> ("JUCEButtonEventForwarderClass_")
        {
            addIvar<Pimpl*> ("owner");

            addMethod (@selector (handleEvent:), handleEvent, "v@:@");

            registerClass();
        }

        static Pimpl* getOwner (id self)                { return getIvar<Pimpl*> (self, "owner"); }
        static void setOwner (id self, Pimpl* owner)    { object_setInstanceVariable (self, "owner", owner); }

    private:
        static void handleEvent (id self, SEL, id)
        {
            if (auto* owner = getOwner (self))
                owner->handleEvent();
        }
    };

    //==============================================================================
    SystemTrayIconComponent& owner;
    std::unique_ptr<NSStatusItem, NSObjectDeleter> statusItem;
    std::unique_ptr<NSObject, NSObjectDeleter> eventForwarder;
    std::unique_ptr<NSImage, NSObjectDeleter> statusIcon;

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
            pimpl->updateIcon (templateImage);
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
    return pimpl != nullptr ? pimpl->getStatusItem() : nullptr;
}

void SystemTrayIconComponent::showDropdownMenu (const PopupMenu& menu)
{
    if (pimpl != nullptr)
        pimpl->showMenu (menu);
}

} // namespace juce
