/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

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

namespace MouseCursorHelpers
{
    extern NSImage* createNSImage (const Image&);
}

extern NSMenu* createNSMenu (const PopupMenu&, const String& name, int topLevelMenuId,
                             int topLevelIndex, bool addDelegate);

class SystemTrayIconComponent::Pimpl  : private Timer
{
public:
    Pimpl (SystemTrayIconComponent& iconComp, const Image& im)
        : owner (iconComp), statusItem (nil),
          statusIcon (MouseCursorHelpers::createNSImage (im)),
          view (nil), isHighlighted (false)
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
        statusIcon = MouseCursorHelpers::createNSImage (newImage);
        setIconSize();
        SystemTrayViewClass::setImage (view, statusIcon);
    }

    void setHighlighted (bool shouldHighlight)
    {
        isHighlighted = shouldHighlight;
        [view setNeedsDisplay: true];
    }

    void handleStatusItemAction (NSEvent* e)
    {
        NSEventType type = [e type];

        const bool isLeft  = (type == NSLeftMouseDown  || type == NSLeftMouseUp);
        const bool isRight = (type == NSRightMouseDown || type == NSRightMouseUp);

        if (owner.isCurrentlyBlockedByAnotherModalComponent())
        {
            if (isLeft || isRight)
                if (Component* const current = Component::getCurrentlyModalComponent())
                    current->inputAttemptWhenModal();
        }
        else
        {
            ModifierKeys eventMods (ModifierKeys::getCurrentModifiersRealtime());

            if (([e modifierFlags] & NSCommandKeyMask) != 0)
                eventMods = eventMods.withFlags (ModifierKeys::commandModifier);

            const Time now (Time::getCurrentTime());

            MouseInputSource mouseSource = Desktop::getInstance().getMainMouseSource();
            const float pressure = (float) e.pressure;

            if (isLeft || isRight)  // Only mouse up is sent by the OS, so simulate a down/up
            {
                setHighlighted (true);
                startTimer (150);

                owner.mouseDown (MouseEvent (mouseSource, Point<float>(),
                                             eventMods.withFlags (isLeft ? ModifierKeys::leftButtonModifier
                                                                         : ModifierKeys::rightButtonModifier),
                                             pressure, &owner, &owner, now,
                                             Point<float>(), now, 1, false));

                owner.mouseUp (MouseEvent (mouseSource, Point<float>(), eventMods.withoutMouseButtons(),
                                           pressure, &owner, &owner, now,
                                           Point<float>(), now, 1, false));
            }
            else if (type == NSMouseMoved)
            {
                owner.mouseMove (MouseEvent (mouseSource, Point<float>(), eventMods,
                                             pressure, &owner, &owner, now,
                                             Point<float>(), now, 1, false));
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
    NSStatusItem* statusItem;

private:
    NSImage* statusIcon;
    NSControl* view;
    bool isHighlighted;

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
            if (Pimpl* const owner = getOwner (self))
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
            if (Pimpl* const owner = getOwner (self))
                owner->handleStatusItemAction (e);
        }

        static void drawRect (id self, SEL, NSRect)
        {
            NSRect bounds = [self bounds];

            if (Pimpl* const owner = getOwner (self))
                [owner->statusItem drawStatusBarBackgroundInRect: bounds
                                                   withHighlight: owner->isHighlighted];

            if (NSImage* const im = getImage (self))
            {
                NSSize imageSize = [im size];

                [im drawInRect: NSMakeRect (bounds.origin.x + ((bounds.size.width  - imageSize.width)  / 2.0f),
                                            bounds.origin.y + ((bounds.size.height - imageSize.height) / 2.0f),
                                            imageSize.width, imageSize.height)
                      fromRect: NSZeroRect
                     operation: NSCompositeSourceOver
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
            pimpl = new Pimpl (*this, newImage);
        else
            pimpl->updateIcon (newImage);
    }
    else
    {
        pimpl = nullptr;
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
