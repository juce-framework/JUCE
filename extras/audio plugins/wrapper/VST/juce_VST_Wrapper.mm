/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

//==============================================================================
#include "../juce_IncludeCharacteristics.h"

#if JucePlugin_Build_VST

#include <Cocoa/Cocoa.h>

#define ADD_CARBON_BODGE 1   // see note below..

#include <Carbon/Carbon.h>

#include "../juce_PluginHeaders.h"

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

    NSView* content = [hostWindow contentView];
    NSRect f = [content frame];
    NSPoint windowPos = [hostWindow convertBaseToScreen: f.origin];
    windowPos.y = [[NSScreen mainScreen] frame].size.height - (windowPos.y + f.size.height);
    comp->setTopLeftPosition ((int) windowPos.x, (int) windowPos.y);

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
    // Adds a callback bodge to work around some problems with wrapped
    // carbon windows..
    const EventTypeSpec eventsToCatch[] = {
        { kEventClassWindow, kEventWindowShown },
        { kEventClassWindow, kEventWindowHidden }
    };

    EventHandlerRef ref;
    InstallWindowEventHandler ((WindowRef) windowRef,
                               NewEventHandlerUPP (windowVisibilityBodge),
                               GetEventTypeCount (eventsToCatch), eventsToCatch,
                               (void*) hostWindow, &ref);
    comp->setComponentProperty ("carbonEventRef", String::toHexString ((pointer_sized_int) (void*) ref));
#endif

    return hostWindow;
}

void detachComponentFromWindowRef (Component* comp, void* nsWindow)
{
    const ScopedAutoReleasePool pool;

#if ADD_CARBON_BODGE
    EventHandlerRef ref = (EventHandlerRef) (void*) (pointer_sized_int)
                                comp->getComponentProperty ("carbonEventRef", false, String::empty).getHexValue64();
    RemoveEventHandler (ref);
#endif

    NSWindow* hostWindow = (NSWindow*) nsWindow;
    NSView* pluginView = (NSView*) comp->getWindowHandle();
    NSWindow* pluginWindow = [pluginView window];

    [hostWindow removeChildWindow: pluginWindow];
    comp->removeFromDesktop();

    [hostWindow release];

    // The event loop needs to be run between closing the window and deleting the plugin,
    // presumably to let the cocoa objects get tidied up. Leaving out this line causes crashes
    // in Live when you delete the plugin with its window open.
    MessageManager::getInstance()->runDispatchLoopUntil (10);
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

        [[hostWindow contentView] setNeedsDisplay: YES];
    }
}

void checkWindowVisibility (void* nsWindow, Component* comp)
{
    NSWindow* hostWindow = (NSWindow*) nsWindow;
    comp->setVisible ([hostWindow isVisible]);
}

END_JUCE_NAMESPACE

#endif
