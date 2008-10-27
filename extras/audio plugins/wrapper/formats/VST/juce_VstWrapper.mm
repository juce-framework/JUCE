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
#include <Cocoa/Cocoa.h>
#include "../../../../../juce.h"


BEGIN_JUCE_NAMESPACE

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

    [hostWindow addChildWindow: pluginWindow 
                       ordered: NSWindowAbove];
    [hostWindow orderFront: nil];
    [pluginWindow orderFront: nil];
    return hostWindow;
}

void detachComponentFromWindowRef (Component* comp, void* nsWindow)
{
    NSWindow* hostWindow = (NSWindow*) nsWindow;
    if (hostWindow != 0)
    {
        const ScopedAutoReleasePool pool;

        NSView* pluginView = (NSView*) comp->getWindowHandle();
        NSWindow* pluginWindow = [pluginView window];

        [hostWindow removeChildWindow: pluginWindow];
        comp->removeFromDesktop();

        [hostWindow release];
    }
}

void setNativeHostWindowSize (void* nsWindow, Component* component, int newWidth, int newHeight)
{
    NSWindow* hostWindow = (NSWindow*) nsWindow;
    if (hostWindow != 0)
    {
        ScopedAutoReleasePool pool;

        NSRect f = [hostWindow frame];
        f.size.width += newWidth - component->getWidth();
        f.size.height += newHeight - component->getHeight();
        [hostWindow setFrame: f display: YES];
    }
}

void checkWindowVisibility (void* nsWindow, Component* comp)
{
    NSWindow* hostWindow = (NSWindow*) nsWindow;
    comp->setVisible ([hostWindow isVisible]);
}

END_JUCE_NAMESPACE
