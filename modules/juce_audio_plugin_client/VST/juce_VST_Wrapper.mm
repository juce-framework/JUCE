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

// Your project must contain an AppConfig.h file with your project-specific settings in it,
// and your header search path must make it accessible to the module's files.
#include "AppConfig.h"

#include "../utility/juce_CheckSettingMacros.h"

#if JucePlugin_Build_VST

#define JUCE_MAC_WINDOW_VISIBITY_BODGE 1

#include "../utility/juce_IncludeSystemHeaders.h"
#include "../utility/juce_IncludeModuleHeaders.h"
#include "../utility/juce_FakeMouseMoveGenerator.h"
#include "../utility/juce_CarbonVisibility.h"
#include "../utility/juce_PluginHostType.h"

//==============================================================================
namespace juce
{

#if ! JUCE_64BIT
void updateEditorCompBounds (Component*);
void updateEditorCompBounds (Component* comp)
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
    updateEditorCompBounds ((Component*) user);
    return noErr;
}
#endif

//==============================================================================
void initialiseMac();
void initialiseMac()
{
   #if ! JUCE_64BIT
    NSApplicationLoad();
   #endif
}

void* attachComponentToWindowRef (Component* comp, void* windowRef);
void* attachComponentToWindowRef (Component* comp, void* windowRef)
{
    JUCE_AUTORELEASEPOOL
    {
      #if JUCE_64BIT
        NSView* parentView = (NSView*) windowRef;

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
      #else
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
        HIRect r = { {0, 0}, { (float) comp->getWidth(), (float) comp->getHeight()} };
        HIViewSetFrame (dummyView, &r);
        HIViewAddSubview (parentView, dummyView);
        comp->getProperties().set ("dummyViewRef", String::toHexString ((pointer_sized_int) (void*) dummyView));

        EventHandlerRef ref;
        const EventTypeSpec kControlBoundsChangedEvent = { kEventClassControl, kEventControlBoundsChanged };
        InstallEventHandler (GetControlEventTarget (dummyView), NewEventHandlerUPP (viewBoundsChangedEvent), 1, &kControlBoundsChangedEvent, (void*) comp, &ref);
        comp->getProperties().set ("boundsEventRef", String::toHexString ((pointer_sized_int) (void*) ref));

        updateEditorCompBounds (comp);

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

        attachWindowHidingHooks (comp, (WindowRef) windowRef, hostWindow);

        return hostWindow;
      #endif
    }
}

void detachComponentFromWindowRef (Component* comp, void* nsWindow);
void detachComponentFromWindowRef (Component* comp, void* nsWindow)
{
    JUCE_AUTORELEASEPOOL
    {
       #if JUCE_64BIT
        comp->removeFromDesktop();
       #else
        EventHandlerRef ref = (EventHandlerRef) (void*) (pointer_sized_int)
                                    comp->getProperties() ["boundsEventRef"].toString().getHexValue64();
        RemoveEventHandler (ref);

        removeWindowHidingHooks (comp);

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

        // The event loop needs to be run between closing the window and deleting the plugin,
        // presumably to let the cocoa objects get tidied up. Leaving out this line causes crashes
        // in Live and Reaper when you delete the plugin with its window open.
        // (Doing it this way rather than using a single longer timout means that we can guarantee
        // how many messages will be dispatched, which seems to be vital in Reaper)
        for (int i = 20; --i >= 0;)
            MessageManager::getInstance()->runDispatchLoopUntil (1);
       #endif
    }
}

void setNativeHostWindowSize (void* nsWindow, Component* component, int newWidth, int newHeight);
void setNativeHostWindowSize (void* nsWindow, Component* component, int newWidth, int newHeight)
{
    JUCE_AUTORELEASEPOOL
    {
       #if JUCE_64BIT
        if (NSView* hostView = (NSView*) nsWindow)
        {
            // xxx is this necessary, or do the hosts detect a change in the child view and do this automatically?
            [hostView setFrameSize: NSMakeSize ([hostView frame].size.width + (newWidth - component->getWidth()),
                                                [hostView frame].size.height + (newHeight - component->getHeight()))];
        }
       #else
        (void) nsWindow;

        if (HIViewRef dummyView = (HIViewRef) (void*) (pointer_sized_int)
                                     component->getProperties() ["dummyViewRef"].toString().getHexValue64())
        {
            HIRect frameRect;
            HIViewGetFrame (dummyView, &frameRect);
            frameRect.size.width = newWidth;
            frameRect.size.height = newHeight;
            HIViewSetFrame (dummyView, &frameRect);
        }
       #endif
    }
}

void checkWindowVisibility (void* nsWindow, Component* comp);
void checkWindowVisibility (void* nsWindow, Component* comp)
{
   #if ! JUCE_64BIT
    comp->setVisible ([((NSWindow*) nsWindow) isVisible]);
   #endif
}

bool forwardCurrentKeyEventToHost (Component* comp);
bool forwardCurrentKeyEventToHost (Component* comp)
{
   #if JUCE_64BIT
    (void) comp;
    return false;
   #else
    NSWindow* win = [(NSView*) comp->getWindowHandle() window];
    [[win parentWindow] makeKeyWindow];
    repostCurrentNSEvent();
    return true;
   #endif
}

} // (juce namespace)

#endif
