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

// If you get an error here, you might need to make sure that
// your build config files don't specify "C++" as one of the
// flags in OTHER_CFLAGS, because that stops the compiler building
// obj-c files correctly.
@class dummyclassname;

// Horrible carbon-based fix for a cocoa bug, where an NSWindow that wraps a carbon
// window fails to keep its position updated when the user drags the window around..
#define WINDOWPOSITION_BODGE 1
#define JUCE_MAC_WINDOW_VISIBITY_BODGE 1

#if WINDOWPOSITION_BODGE
 #include <Carbon/Carbon.h>
#endif

#include <Cocoa/Cocoa.h>
#include "../juce_PluginHeaders.h"

#if JucePlugin_Build_RTAS

//==============================================================================
void initialiseMacRTAS()
{
   #if ! JUCE_64BIT
    NSApplicationLoad();
   #endif
}

void* attachSubWindow (void* hostWindowRef, Component* comp)
{
    JUCE_AUTORELEASEPOOL

    NSWindow* hostWindow = [[NSWindow alloc] initWithWindowRef: hostWindowRef];
    [hostWindow retain];
    [hostWindow setCanHide: YES];
    [hostWindow setReleasedWhenClosed: YES];
    NSRect oldWindowFrame = [hostWindow frame];

    NSView* content = [hostWindow contentView];
    NSRect f = [content frame];
    f.size.width = comp->getWidth();
    f.size.height = comp->getHeight();
    [content setFrame: f];

    NSRect hostWindowScreenFrame = [[hostWindow screen] frame];

   #if WINDOWPOSITION_BODGE
    {
        Rect winBounds;
        GetWindowBounds ((WindowRef) hostWindowRef, kWindowContentRgn, &winBounds);
        NSRect w = [hostWindow frame];
        w.origin.x = winBounds.left;
        w.origin.y = hostWindowScreenFrame.size.height + hostWindowScreenFrame.origin.y - winBounds.bottom;
        [hostWindow setFrame: w display: NO animate: NO];
    }
   #endif

    NSPoint windowPos = [hostWindow convertBaseToScreen: f.origin];
    windowPos.x = windowPos.x + jmax (0.0f, (oldWindowFrame.size.width - f.size.width) / 2.0f);
    windowPos.y = hostWindowScreenFrame.size.height  + hostWindowScreenFrame.origin.y - (windowPos.y + f.size.height);

    comp->setTopLeftPosition ((int) windowPos.x, (int) windowPos.y);

   #if ! JucePlugin_EditorRequiresKeyboardFocus
    comp->addToDesktop (ComponentPeer::windowIsTemporary | ComponentPeer::windowIgnoresKeyPresses);
   #else
    comp->addToDesktop (ComponentPeer::windowIsTemporary);
   #endif

    comp->setVisible (true);

    NSView* pluginView = (NSView*) comp->getWindowHandle();
    NSWindow* pluginWindow = [pluginView window];

    [hostWindow addChildWindow: pluginWindow
                       ordered: NSWindowAbove];
    [hostWindow orderFront: nil];
    [pluginWindow orderFront: nil];

    attachWindowHidingHooks (comp, (WindowRef) hostWindowRef, hostWindow);

    return hostWindow;
}

void removeSubWindow (void* nsWindow, Component* comp)
{
    JUCE_AUTORELEASEPOOL

    NSView* pluginView = (NSView*) comp->getWindowHandle();
    NSWindow* hostWindow = (NSWindow*) nsWindow;
    NSWindow* pluginWindow = [pluginView window];

    removeWindowHidingHooks (comp);
    [hostWindow removeChildWindow: pluginWindow];
    comp->removeFromDesktop();
    [hostWindow release];
}

namespace
{
    bool isJuceWindow (WindowRef w)
    {
        for (int i = ComponentPeer::getNumPeers(); --i >= 0;)
        {
            ComponentPeer* peer = ComponentPeer::getPeer(i);
            NSView* view = (NSView*) peer->getNativeHandle();

            if ([[view window] windowRef] == w)
                return true;
        }

        return false;
    }
}

void forwardCurrentKeyEventToHostWindow()
{
    WindowRef w = FrontNonFloatingWindow();
    WindowRef original = w;

    while (IsValidWindowPtr (w) && isJuceWindow (w))
    {
        w = GetNextWindowOfClass (w, kDocumentWindowClass, true);

        if (w == original)
            break;
    }

    if (! isJuceWindow (w))
    {
        ActivateWindow (w, true);
        [NSApp postEvent: [NSApp currentEvent] atStart: YES];
    }
}

#endif
