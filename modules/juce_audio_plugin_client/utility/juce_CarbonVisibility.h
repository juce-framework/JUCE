/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

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

#ifndef JUCE_CARBONVISIBILITY_H_INCLUDED
#define JUCE_CARBONVISIBILITY_H_INCLUDED

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

inline void attachWindowHidingHooks (Component* comp, void* hostWindowRef, NSWindow* nsWindow)
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

inline void removeWindowHidingHooks (Component* comp)
{
    if (comp != nullptr)
        RemoveEventHandler ((EventHandlerRef) (void*) (pointer_sized_int)
                              comp->getProperties() ["carbonEventRef"].toString().getHexValue64());
}

#elif JUCE_MAC
 inline void attachWindowHidingHooks (void*, void*, void*) {}
 inline void removeWindowHidingHooks (void*) {}
#endif

#endif   // JUCE_CARBONVISIBILITY_H_INCLUDED
