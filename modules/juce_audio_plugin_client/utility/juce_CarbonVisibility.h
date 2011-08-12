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

#ifndef __JUCE_CARBONVISIBILITY_JUCEHEADER__
#define __JUCE_CARBONVISIBILITY_JUCEHEADER__

//==============================================================================
#if JUCE_SUPPORT_CARBON && JUCE_MAC_WINDOW_VISIBITY_BODGE

/* When you wrap a WindowRef as an NSWindow, it seems to bugger up the HideWindow
   function, so when the host tries (and fails) to hide the window, this stuff catches
   the event and forces it to update.
*/
static pascal OSStatus windowVisibilityBodge (EventHandlerCallRef, EventRef e, void* user)
{
    NSWindow* hostWindow = (NSWindow*) user;

    switch (GetEventKind (e))
    {
        case kEventWindowInit:    [hostWindow display]; break;
        case kEventWindowShown:   [hostWindow orderFront: nil]; break;
        case kEventWindowHidden:  [hostWindow orderOut: nil]; break;
    }

    return eventNotHandledErr;
}

static void attachWindowHidingHooks (Component* comp, void* hostWindowRef, NSWindow* nsWindow)
{
    const EventTypeSpec eventsToCatch[] =
    {
        { kEventClassWindow, kEventWindowInit },
        { kEventClassWindow, kEventWindowShown },
        { kEventClassWindow, kEventWindowHidden }
    };

    EventHandlerRef ref;
    InstallWindowEventHandler ((WindowRef) hostWindowRef,
                               NewEventHandlerUPP (windowVisibilityBodge),
                               GetEventTypeCount (eventsToCatch), eventsToCatch,
                               (void*) nsWindow, &ref);

    comp->getProperties().set ("carbonEventRef", String::toHexString ((pointer_sized_int) (void*) ref));
}

static void removeWindowHidingHooks (Component* comp)
{
    if (comp != nullptr)
        RemoveEventHandler ((EventHandlerRef) (void*) (pointer_sized_int)
                              comp->getProperties() ["carbonEventRef"].toString().getHexValue64());
}

#elif JUCE_MAC
 static void attachWindowHidingHooks (void*, void*, void*) {}
 static void removeWindowHidingHooks (void*) {}
#endif

#endif   // __JUCE_CARBONVISIBILITY_JUCEHEADER__
