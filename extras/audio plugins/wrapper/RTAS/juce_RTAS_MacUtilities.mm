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

// If you get an error here, you might need to make sure that
// your build config files don't specify "C++" as one of the
// flags in OTHER_CFLAGS, because that stops the compiler building
// obj-c files correctly.
@class dummyclassname;

// Horrible carbon-based fix for a cocoa bug, where an NSWindow that wraps a carbon
// window fails to keep its position updated when the user drags the window around..
#define WINDOWPOSITON_BODGE 1

#if WINDOWPOSITON_BODGE
 #include <Carbon/Carbon.h>
#endif

#include <Cocoa/Cocoa.h>
#include "../juce_PluginHeaders.h"

#if JucePlugin_Build_RTAS

//==============================================================================
void initialiseMacRTAS()
{
    NSApplicationLoad();
}

void* attachSubWindow (void* hostWindowRef, Component* comp)
{
    const ScopedAutoReleasePool pool;

    NSWindow* hostWindow = [[NSWindow alloc] initWithWindowRef: hostWindowRef];
    [hostWindow retain];
    [hostWindow setCanHide: YES];
    [hostWindow setReleasedWhenClosed: YES];

    NSView* content = [hostWindow contentView];
    NSRect f = [content frame];
    f.size.width = comp->getWidth();
    f.size.height = comp->getHeight();
    [content setFrame: f];

#if WINDOWPOSITON_BODGE
    {
        Rect winBounds;
        GetWindowBounds ((WindowRef) hostWindowRef, kWindowContentRgn, &winBounds);
        NSRect w = [hostWindow frame];
        w.origin.x = winBounds.left;
        w.origin.y = [[NSScreen mainScreen] frame].size.height - winBounds.bottom;
        [hostWindow setFrame: w display: NO animate: NO];
    }
#endif

    NSPoint windowPos = [hostWindow convertBaseToScreen: f.origin];
    windowPos.y = [[NSScreen mainScreen] frame].size.height - (windowPos.y + f.size.height);

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
    return hostWindow;
}

void removeSubWindow (void* nsWindow, Component* comp)
{
    const ScopedAutoReleasePool pool;

    NSView* pluginView = (NSView*) comp->getWindowHandle();
    NSWindow* hostWindow = (NSWindow*) nsWindow;
    NSWindow* pluginWindow = [pluginView window];

    [hostWindow removeChildWindow: pluginWindow];
    comp->removeFromDesktop();
    [hostWindow release];
}

#endif
