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

#pragma once

#include <juce_core/system/juce_TargetPlatform.h>

#if JUCE_MAC

#include <juce_audio_plugin_client/detail/juce_CheckSettingMacros.h>
#include <juce_audio_plugin_client/detail/juce_IncludeSystemHeaders.h>
#include <juce_audio_plugin_client/detail/juce_VSTWindowUtilities.h>

namespace juce::detail
{

#if JUCE_32BIT
class EventReposter  : private CallbackMessage
{
public:
    EventReposter() : e ([[NSApp currentEvent] retain])  {}
    ~EventReposter() override  { [e release]; }

    static void repostCurrentNSEvent()
    {
        (new EventReposter())->post();
    }

private:
    void messageCallback() override
    {
        [NSApp postEvent: e atStart: YES];
    }

    NSEvent* e;
};

void VSTWindowUtilities::updateEditorCompBoundsVST (Component* comp)
{
    HIViewRef dummyView = (HIViewRef) (void*) (pointer_sized_int)
                            comp->getProperties() ["dummyViewRef"].toString().getHexValue64();

    HIRect r;
    HIViewGetFrame (dummyView, &r);
    HIViewRef root;
    HIViewFindByID (HIViewGetRoot (HIViewGetWindow (dummyView)), kHIViewWindowContentID, &root);
    HIViewConvertRect (&r, HIViewGetSuperview (dummyView), root);

    Rect windowPos;
    GetWindowBounds (HIViewGetWindow (dummyView), kWindowContentRgn, &windowPos);

    comp->setTopLeftPosition ((int) (windowPos.left + r.origin.x),
                              (int) (windowPos.top + r.origin.y));
}

pascal OSStatus VSTWindowUtilities::viewBoundsChangedEvent (EventHandlerCallRef, EventRef, void* user)
{
    updateEditorCompBoundsVST ((Component*) user);
    return noErr;
}

bool VSTWindowUtilities::shouldManuallyCloseHostWindow()
{
    return getHostType().isCubase7orLater()
        || getHostType().isRenoise()
        || ((SystemStats::getOperatingSystemType() & 0xff) >= 12);
}
#endif

void* VSTWindowUtilities::attachComponentToWindowRefVST (Component* comp,
                                                         int desktopFlags,
                                                         void* parentWindowOrView,
                                                         [[maybe_unused]] bool isNSView)
{
    JUCE_AUTORELEASEPOOL
    {
       #if ! JUCE_64BIT
        if (! isNSView)
        {
            NSWindow* hostWindow = [[NSWindow alloc] initWithWindowRef: parentWindowOrView];

            if (shouldManuallyCloseHostWindow())
            {
                [hostWindow setReleasedWhenClosed: NO];
            }
            else
            {
                [hostWindow retain];
                [hostWindow setReleasedWhenClosed: YES];
            }

            [hostWindow setCanHide: YES];

            HIViewRef parentView = 0;

            WindowAttributes attributes;
            GetWindowAttributes ((WindowRef) parentWindowOrView, &attributes);

            if ((attributes & kWindowCompositingAttribute) != 0)
            {
                HIViewRef root = HIViewGetRoot ((WindowRef) parentWindowOrView);
                HIViewFindByID (root, kHIViewWindowContentID, &parentView);

                if (parentView == 0)
                    parentView = root;
            }
            else
            {
                GetRootControl ((WindowRef) parentWindowOrView, (ControlRef*) &parentView);

                if (parentView == 0)
                    CreateRootControl ((WindowRef) parentWindowOrView, (ControlRef*) &parentView);
            }

            // It seems that the only way to successfully position our overlaid window is by putting a dummy
            // HIView into the host's carbon window, and then catching events to see when it gets repositioned
            HIViewRef dummyView = 0;
            HIImageViewCreate (0, &dummyView);
            HIRect r = { {0, 0}, { (float) comp->getWidth(), (float) comp->getHeight()} };
            HIViewSetFrame (dummyView, &r);
            HIViewAddSubview (parentView, dummyView);
            comp->getProperties().set ("dummyViewRef", String::toHexString ((pointer_sized_int) (void*) dummyView));

            EventHandlerRef ref;
            const EventTypeSpec kControlBoundsChangedEvent = { kEventClassControl, kEventControlBoundsChanged };
            InstallEventHandler (GetControlEventTarget (dummyView),
                                 NewEventHandlerUPP (viewBoundsChangedEvent),
                                 1,
                                 &kControlBoundsChangedEvent,
                                 (void*) comp,
                                 &ref);
            comp->getProperties().set ("boundsEventRef", String::toHexString ((pointer_sized_int) (void*) ref));

            updateEditorCompBoundsVST (comp);

            const auto defaultFlags =
                   #if ! JucePlugin_EditorRequiresKeyboardFocus
                    ComponentPeer::windowIsTemporary | ComponentPeer::windowIgnoresKeyPresses;
                   #else
                    ComponentPeer::windowIsTemporary;
                   #endif
            comp->addToDesktop (desktopFlags | defaultFlags);

            comp->setVisible (true);
            comp->toFront (false);

            NSView* pluginView = (NSView*) comp->getWindowHandle();
            NSWindow* pluginWindow = [pluginView window];
            [pluginWindow setExcludedFromWindowsMenu: YES];
            [pluginWindow setCanHide: YES];

            [hostWindow addChildWindow: pluginWindow
                               ordered: NSWindowAbove];
            [hostWindow orderFront: nil];
            [pluginWindow orderFront: nil];

            return hostWindow;
        }
       #endif

        NSView* parentView = [(NSView*) parentWindowOrView retain];

        const auto defaultFlags =
               #if JucePlugin_EditorRequiresKeyboardFocus
                0;
               #else
                ComponentPeer::windowIgnoresKeyPresses;
               #endif
        comp->addToDesktop (desktopFlags | defaultFlags, parentView);

        // (this workaround is because Wavelab provides a zero-size parent view..)
        if ([parentView frame].size.height == 0)
            [((NSView*) comp->getWindowHandle()) setFrameOrigin: NSZeroPoint];

        comp->setVisible (true);
        comp->toFront (false);

        [[parentView window] setAcceptsMouseMovedEvents: YES];
        return parentView;
    }
}

void VSTWindowUtilities::detachComponentFromWindowRefVST (Component* comp,
                                                          void* window,
                                                          [[maybe_unused]] bool isNSView)
{
    JUCE_AUTORELEASEPOOL
    {
       #if ! JUCE_64BIT
        if (! isNSView)
        {
            EventHandlerRef ref = (EventHandlerRef) (void*) (pointer_sized_int)
                                        comp->getProperties() ["boundsEventRef"].toString().getHexValue64();
            RemoveEventHandler (ref);

            CFUniquePtr<HIViewRef> dummyView ((HIViewRef) (void*) (pointer_sized_int)
                                                comp->getProperties() ["dummyViewRef"].toString().getHexValue64());

            if (HIViewIsValid (dummyView.get()))
                dummyView = nullptr;

            NSWindow* hostWindow = (NSWindow*) window;
            NSView* pluginView = (NSView*) comp->getWindowHandle();
            NSWindow* pluginWindow = [pluginView window];

            [pluginView retain];
            [hostWindow removeChildWindow: pluginWindow];
            [pluginWindow close];
            comp->removeFromDesktop();
            [pluginView release];

            if (shouldManuallyCloseHostWindow())
                [hostWindow close];
            else
                [hostWindow release];

           #if JUCE_MODAL_LOOPS_PERMITTED
            static bool needToRunMessageLoop = ! getHostType().isReaper();

            // The event loop needs to be run between closing the window and deleting the plugin,
            // presumably to let the cocoa objects get tidied up. Leaving out this line causes crashes
            // in Live when you delete the plugin with its window open.
            // (Doing it this way rather than using a single longer timeout means that we can guarantee
            // how many messages will be dispatched, which seems to be vital in Reaper)
            if (needToRunMessageLoop)
                for (int i = 20; --i >= 0;)
                    MessageManager::getInstance()->runDispatchLoopUntil (1);
           #endif

            return;
        }
       #endif

        comp->removeFromDesktop();
        [(id) window release];
    }
}

void VSTWindowUtilities::initialiseMacVST()
{
   #if ! JUCE_64BIT
    NSApplicationLoad();
   #endif
}

void VSTWindowUtilities::setNativeHostWindowSizeVST (void* window,
                                                     Component* component,
                                                     int newWidth,
                                                     int newHeight,
                                                     [[maybe_unused]] bool isNSView)
{
    JUCE_AUTORELEASEPOOL
    {
       #if ! JUCE_64BIT
        if (! isNSView)
        {
            if (HIViewRef dummyView = (HIViewRef) (void*) (pointer_sized_int)
                                         component->getProperties() ["dummyViewRef"].toString().getHexValue64())
            {
                HIRect frameRect;
                HIViewGetFrame (dummyView, &frameRect);
                frameRect.size.width = newWidth;
                frameRect.size.height = newHeight;
                HIViewSetFrame (dummyView, &frameRect);
            }

            return;
        }
       #endif

        if (NSView* hostView = (NSView*) window)
        {
            const int dx = newWidth  - component->getWidth();
            const int dy = newHeight - component->getHeight();

            NSRect r = [hostView frame];
            r.size.width += dx;
            r.size.height += dy;
            r.origin.y -= dy;
            [hostView setFrame: r];
        }
    }
}

void VSTWindowUtilities::checkWindowVisibilityVST ([[maybe_unused]] void* window,
                                                   [[maybe_unused]] Component* comp,
                                                   [[maybe_unused]] bool isNSView)
{
   #if ! JUCE_64BIT
    if (! isNSView)
        comp->setVisible ([((NSWindow*) window) isVisible]);
   #endif
}

bool VSTWindowUtilities::forwardCurrentKeyEventToHostVST ([[maybe_unused]] Component* comp,
                                                          [[maybe_unused]] bool isNSView)
{
   #if ! JUCE_64BIT
    if (! isNSView)
    {
        NSWindow* win = [(NSView*) comp->getWindowHandle() window];
        [[win parentWindow] makeKeyWindow];
        EventReposter::repostCurrentNSEvent();
        return true;
    }
   #endif

    return false;
}

} // namespace juce::detail

#endif
