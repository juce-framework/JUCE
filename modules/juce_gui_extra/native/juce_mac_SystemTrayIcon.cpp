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

namespace MouseCursorHelpers
{
    extern NSImage* createNSImage (const Image&);
}

class SystemTrayIconComponent::Pimpl
{
public:
    Pimpl (SystemTrayIconComponent& iconComp, const Image& im)
        : owner (iconComp), statusItem (nil),
          statusIcon (MouseCursorHelpers::createNSImage (im)),
          isHighlighted (false)
    {
        static SystemTrayViewClass cls;
        view = [cls.createInstance() init];
        SystemTrayViewClass::setOwner (view, this);
        SystemTrayViewClass::setImage (view, statusIcon);

        setIconSize();

        statusItem = [[[NSStatusBar systemStatusBar] statusItemWithLength: NSSquareStatusItemLength] retain];
        [statusItem setView: view];
    }

    ~Pimpl()
    {
        [[NSStatusBar systemStatusBar] removeStatusItem: statusItem];
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

            NSRect r = [[e window] frame];
            r.origin.y = [[[NSScreen screens] objectAtIndex: 0] frame].size.height - r.origin.y - r.size.height;
            owner.setBounds (convertToRectInt (r));

            const Time now (Time::getCurrentTime());

            if (isLeft || isRight)  // Only mouse up is sent by the OS, so simulate a down/up
            {
                owner.mouseDown (MouseEvent (Desktop::getInstance().getMainMouseSource(),
                                             Point<int>(),
                                             eventMods.withFlags (isLeft ? ModifierKeys::leftButtonModifier
                                                                         : ModifierKeys::rightButtonModifier),
                                             &owner, &owner, now,
                                             Point<int>(), now, 1, false));

                owner.mouseUp (MouseEvent (Desktop::getInstance().getMainMouseSource(),
                                           Point<int>(), eventMods.withoutMouseButtons(),
                                           &owner, &owner, now,
                                           Point<int>(), now, 1, false));
            }
            else if (type == NSMouseMoved)
            {
                owner.mouseMove (MouseEvent (Desktop::getInstance().getMainMouseSource(),
                                             Point<int>(), eventMods,
                                             &owner, &owner, now,
                                             Point<int>(), now, 1, false));
            }
        }
    }

private:
    SystemTrayIconComponent& owner;
    NSStatusItem* statusItem;
    NSImage* statusIcon;
    NSControl* view;
    bool isHighlighted;

    void setIconSize()
    {
        [statusIcon setSize: NSMakeSize (20.0f, 20.0f)];
    }

    struct SystemTrayViewClass : public ObjCClass <NSControl>
    {
        SystemTrayViewClass()  : ObjCClass <NSControl> ("JUCESystemTrayView_")
        {
            addIvar<Pimpl*> ("owner");
            addIvar<NSImage*> ("image");

            addMethod (@selector (mouseDown:),      handleEventDown, "v@:@");
            addMethod (@selector (rightMouseDown:), handleEventDown, "v@:@");
            addMethod (@selector (drawRect:),       drawRect,        "v@:@");

            registerClass();
        }

        static Pimpl* getOwner (id self)                { return getIvar<Pimpl*> (self, "owner"); }
        static NSImage* getImage (id self)              { return getIvar<NSImage*> (self, "image"); }
        static void setOwner (id self, Pimpl* owner)    { object_setInstanceVariable (self, "owner", owner); }
        static void setImage (id self, NSImage* image)  { object_setInstanceVariable (self, "image", image); }

    private:
        static void handleEventDown (id self, SEL, NSEvent* e)
        {
            if (Pimpl* const owner = getOwner (self))
            {
                owner->setHighlighted (! owner->isHighlighted);
                owner->handleStatusItemAction (e);
            }
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
