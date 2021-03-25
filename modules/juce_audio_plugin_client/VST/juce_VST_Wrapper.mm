/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#include <juce_core/system/juce_TargetPlatform.h>

#if JUCE_MAC

#include "../utility/juce_CheckSettingMacros.h"

#if JucePlugin_Build_VST || JucePlugin_Build_VST3

#define JUCE_MAC_WINDOW_VISIBITY_BODGE 1

#include "../utility/juce_IncludeSystemHeaders.h"
#include "../utility/juce_IncludeModuleHeaders.h"
#include "../utility/juce_FakeMouseMoveGenerator.h"
#include "../utility/juce_CarbonVisibility.h"

//==============================================================================
namespace juce
{

#if ! JUCE_64BIT
JUCE_API void updateEditorCompBoundsVST (Component*);
void updateEditorCompBoundsVST (Component* comp)
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

static pascal OSStatus viewBoundsChangedEvent (EventHandlerCallRef, EventRef, void* user)
{
    updateEditorCompBoundsVST ((Component*) user);
    return noErr;
}

static bool shouldManuallyCloseHostWindow()
{
    return getHostType().isCubase7orLater() || getHostType().isRenoise() || ((SystemStats::getOperatingSystemType() & 0xff) >= 12);
}
#endif

//==============================================================================
JUCE_API void initialiseMacVST();
void initialiseMacVST()
{
   #if ! JUCE_64BIT
    NSApplicationLoad();
   #endif
}

JUCE_API void* attachComponentToWindowRefVST (Component* comp, void* parentWindowOrView, bool isNSView);
void* attachComponentToWindowRefVST (Component* comp, void* parentWindowOrView, bool isNSView)
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
            InstallEventHandler (GetControlEventTarget (dummyView), NewEventHandlerUPP (viewBoundsChangedEvent), 1, &kControlBoundsChangedEvent, (void*) comp, &ref);
            comp->getProperties().set ("boundsEventRef", String::toHexString ((pointer_sized_int) (void*) ref));

            updateEditorCompBoundsVST (comp);

           #if ! JucePlugin_EditorRequiresKeyboardFocus
            comp->addToDesktop (ComponentPeer::windowIsTemporary | ComponentPeer::windowIgnoresKeyPresses);
           #else
            comp->addToDesktop (ComponentPeer::windowIsTemporary);
           #endif

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

            attachWindowHidingHooks (comp, (WindowRef) parentWindowOrView, hostWindow);

            return hostWindow;
        }
       #endif

        ignoreUnused (isNSView);
        NSView* parentView = [(NSView*) parentWindowOrView retain];

       #if JucePlugin_EditorRequiresKeyboardFocus
        comp->addToDesktop (0, parentView);
       #else
        comp->addToDesktop (ComponentPeer::windowIgnoresKeyPresses, parentView);
       #endif

        // (this workaround is because Wavelab provides a zero-size parent view..)
        if ([parentView frame].size.height == 0)
            [((NSView*) comp->getWindowHandle()) setFrameOrigin: NSZeroPoint];

        comp->setVisible (true);
        comp->toFront (false);

        [[parentView window] setAcceptsMouseMovedEvents: YES];
        return parentView;
    }
}

JUCE_API void detachComponentFromWindowRefVST (Component* comp, void* window, bool isNSView);
void detachComponentFromWindowRefVST (Component* comp, void* window, bool isNSView)
{
    JUCE_AUTORELEASEPOOL
    {
       #if ! JUCE_64BIT
        if (! isNSView)
        {
            EventHandlerRef ref = (EventHandlerRef) (void*) (pointer_sized_int)
                                        comp->getProperties() ["boundsEventRef"].toString().getHexValue64();
            RemoveEventHandler (ref);

            removeWindowHidingHooks (comp);

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

        ignoreUnused (isNSView);
        comp->removeFromDesktop();
        [(id) window release];
    }
}

JUCE_API void setNativeHostWindowSizeVST (void* window, Component* component, int newWidth, int newHeight, bool isNSView);
void setNativeHostWindowSizeVST (void* window, Component* component, int newWidth, int newHeight, bool isNSView)
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

        ignoreUnused (isNSView);

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

JUCE_API void checkWindowVisibilityVST (void* window, Component* comp, bool isNSView);
void checkWindowVisibilityVST (void* window, Component* comp, bool isNSView)
{
    ignoreUnused (window, comp, isNSView);

   #if ! JUCE_64BIT
    if (! isNSView)
        comp->setVisible ([((NSWindow*) window) isVisible]);
   #endif
}

JUCE_API bool forwardCurrentKeyEventToHostVST (Component* comp, bool isNSView);
bool forwardCurrentKeyEventToHostVST (Component* comp, bool isNSView)
{
   #if ! JUCE_64BIT
    if (! isNSView)
    {
        NSWindow* win = [(NSView*) comp->getWindowHandle() window];
        [[win parentWindow] makeKeyWindow];
        repostCurrentNSEvent();
        return true;
    }
   #endif

    ignoreUnused (comp, isNSView);
    return false;
}

} // (juce namespace)

#endif
#endif
