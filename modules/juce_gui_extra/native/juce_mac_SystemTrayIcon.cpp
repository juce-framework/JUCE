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

namespace MouseCursorHelpers
{
    extern NSImage* createNSImage (const Image&);
}

class SystemTrayIconComponent::Pimpl
{
public:
    Pimpl (SystemTrayIconComponent& iconComp, const Image& im)
        : owner (iconComp), statusItem (nil),
          statusIcon (MouseCursorHelpers::createNSImage (im))
    {
        static SystemTrayCallbackClass cls;
        callback = [cls.createInstance() init];
        SystemTrayCallbackClass::setOwner (callback, this);

        statusItem = [[[NSStatusBar systemStatusBar] statusItemWithLength: NSSquareStatusItemLength] retain];

        [statusItem setHighlightMode: YES];

        setIconSize();
        [statusItem setImage: statusIcon];
        [statusItem setTarget: callback];
        [statusItem setAction: @selector (statusItemAction:)];
    }

    ~Pimpl()
    {
        [statusItem release];
        [statusIcon release];
        [callback release];
    }

    void updateIcon (const Image& newImage)
    {
        [statusIcon release];
        statusIcon = MouseCursorHelpers::createNSImage (newImage);
        setIconSize();
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
    NSObject* callback;

    void setIconSize()
    {
        [statusIcon setSize: NSMakeSize (20.0f, 20.0f)];
    }

    struct SystemTrayCallbackClass   : public ObjCClass <NSObject>
    {
        SystemTrayCallbackClass()  : ObjCClass <NSObject> ("JUCESystemTray_")
        {
            addIvar<SystemTrayIconComponent::Pimpl*> ("owner");
            addMethod (@selector (statusItemAction:), statusItemAction, "v@:@");

            registerClass();
        }

        static void setOwner (id self, SystemTrayIconComponent::Pimpl* owner)
        {
            object_setInstanceVariable (self, "owner", owner);
        }

    private:
        static void statusItemAction (id self, SEL, id /*sender*/)
        {
            if (SystemTrayIconComponent::Pimpl* const owner = getIvar<SystemTrayIconComponent::Pimpl*> (self, "owner"))
                owner->handleStatusItemAction ([NSApp currentEvent]);
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

void SystemTrayIconComponent::setIconTooltip (const String& /* tooltip */)
{
    // xxx not yet implemented!
}
