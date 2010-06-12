/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

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

//==============================================================================
#include "../juce_IncludeCharacteristics.h"

#if JucePlugin_Build_VST

#include <Cocoa/Cocoa.h>
#include <Carbon/Carbon.h>

#include "../juce_PluginHeaders.h"

#define ADD_CARBON_BODGE 1   // see note below..

//==============================================================================
BEGIN_JUCE_NAMESPACE

#if ADD_CARBON_BODGE
/* When you wrap a WindowRef as an NSWindow, it seems to bugger up the HideWindow
   function, so when the host tries (and fails) to hide the window, this catches
   the event and does the job properly.
*/

static pascal OSStatus windowVisibilityBodge (EventHandlerCallRef, EventRef e, void* user)
{
    NSWindow* hostWindow = (NSWindow*) user;

    switch (GetEventKind (e))
    {
    case kEventWindowInit:
        [hostWindow display];
        break;
    case kEventWindowShown:
        [hostWindow orderFront: nil];
        break;
    case kEventWindowHidden:
        [hostWindow orderOut: nil];
        break;
    }

    return eventNotHandledErr;
}
#endif

static void updateComponentPos (Component* const comp)
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
    updateComponentPos ((Component*) user);
    return noErr;
}

//==============================================================================
void initialiseMac()
{
    NSApplicationLoad();
}

void* attachComponentToWindowRef (Component* comp, void* windowRef)
{
    const ScopedAutoReleasePool pool;

    NSWindow* hostWindow = [[NSWindow alloc] initWithWindowRef: windowRef];
    [hostWindow retain];
    [hostWindow setCanHide: YES];
    [hostWindow setReleasedWhenClosed: YES];

    HIViewRef parentView = 0;

    WindowAttributes attributes;
    GetWindowAttributes ((WindowRef) windowRef, &attributes);
    if ((attributes & kWindowCompositingAttribute) != 0)
    {
        HIViewRef root = HIViewGetRoot ((WindowRef) windowRef);
        HIViewFindByID (root, kHIViewWindowContentID, &parentView);

        if (parentView == 0)
            parentView = root;
    }
    else
    {
        GetRootControl ((WindowRef) windowRef, (ControlRef*) &parentView);

        if (parentView == 0)
            CreateRootControl ((WindowRef) windowRef, (ControlRef*) &parentView);
    }

    // It seems that the only way to successfully position our overlaid window is by putting a dummy
    // HIView into the host's carbon window, and then catching events to see when it gets repositioned
    HIViewRef dummyView = 0;
    HIImageViewCreate (0, &dummyView);
    HIRect r = { {0, 0}, {comp->getWidth(), comp->getHeight()} };
    HIViewSetFrame (dummyView, &r);
    HIViewAddSubview (parentView, dummyView);
    comp->getProperties().set ("dummyViewRef", String::toHexString ((pointer_sized_int) (void*) dummyView));

    EventHandlerRef ref;
    const EventTypeSpec kControlBoundsChangedEvent = { kEventClassControl, kEventControlBoundsChanged };
    InstallEventHandler (GetControlEventTarget (dummyView), NewEventHandlerUPP (viewBoundsChangedEvent), 1, &kControlBoundsChangedEvent, (void*) comp, &ref);
    comp->getProperties().set ("boundsEventRef", String::toHexString ((pointer_sized_int) (void*) ref));

    updateComponentPos (comp);

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

#if ADD_CARBON_BODGE
    {
        // Adds a callback bodge to work around some problems with wrapped
        // carbon windows..
        const EventTypeSpec eventsToCatch[] = {
            { kEventClassWindow, kEventWindowInit },
            { kEventClassWindow, kEventWindowShown },
            { kEventClassWindow, kEventWindowHidden }
        };

        InstallWindowEventHandler ((WindowRef) windowRef,
                                   NewEventHandlerUPP (windowVisibilityBodge),
                                   GetEventTypeCount (eventsToCatch), eventsToCatch,
                                   (void*) hostWindow, &ref);
        comp->getProperties().set ("carbonEventRef", String::toHexString ((pointer_sized_int) (void*) ref));
    }

#endif

    return hostWindow;
}

void detachComponentFromWindowRef (Component* comp, void* nsWindow)
{
    {
        const ScopedAutoReleasePool pool;

        EventHandlerRef ref = (EventHandlerRef) (void*) (pointer_sized_int)
                                    comp->getProperties() ["boundsEventRef"].toString().getHexValue64();
        RemoveEventHandler (ref);

#if ADD_CARBON_BODGE
        ref = (EventHandlerRef) (void*) (pointer_sized_int)
                  comp->getProperties() ["carbonEventRef"].toString().getHexValue64();
        RemoveEventHandler (ref);
#endif

        HIViewRef dummyView = (HIViewRef) (void*) (pointer_sized_int)
                                comp->getProperties() ["dummyViewRef"].toString().getHexValue64();

        if (HIViewIsValid (dummyView))
            CFRelease (dummyView);

        NSWindow* hostWindow = (NSWindow*) nsWindow;
        NSView* pluginView = (NSView*) comp->getWindowHandle();
        NSWindow* pluginWindow = [pluginView window];

        [hostWindow removeChildWindow: pluginWindow];
        comp->removeFromDesktop();

        [hostWindow release];
    }

    // The event loop needs to be run between closing the window and deleting the plugin,
    // presumably to let the cocoa objects get tidied up. Leaving out this line causes crashes
    // in Live and Reaper when you delete the plugin with its window open.
    // (Doing it this way rather than using a single longer timout means that we can guarantee
    // how many messages will be dispatched, which seems to be vital in Reaper)
    for (int i = 20; --i >= 0;)
        MessageManager::getInstance()->runDispatchLoopUntil (1);
}

void setNativeHostWindowSize (void* nsWindow, Component* component, int newWidth, int newHeight)
{
    NSWindow* hostWindow = (NSWindow*) nsWindow;
    if (hostWindow != 0)
    {
        ScopedAutoReleasePool pool;

        // Can't use the cocoa NSWindow resizing code, or it messes up in Live.
        Rect r;
        GetWindowBounds ((WindowRef) [hostWindow windowRef], kWindowContentRgn, &r);
        r.right += newWidth - component->getWidth();
        r.bottom += newHeight - component->getHeight();
        SetWindowBounds ((WindowRef) [hostWindow windowRef], kWindowContentRgn, &r);

        r.left = r.top = 0;
        InvalWindowRect ((WindowRef) [hostWindow windowRef], &r);
        //[[hostWindow contentView] setNeedsDisplay: YES];
    }
}

void checkWindowVisibility (void* nsWindow, Component* comp)
{
    NSWindow* hostWindow = (NSWindow*) nsWindow;
    comp->setVisible ([hostWindow isVisible]);
}

void forwardCurrentKeyEventToHost (Component* comp)
{
    NSWindow* win = [(NSView*) comp->getWindowHandle() window];
    [[win parentWindow] makeKeyWindow];
    [NSApp postEvent: [NSApp currentEvent] atStart: YES];
}


END_JUCE_NAMESPACE

#endif
