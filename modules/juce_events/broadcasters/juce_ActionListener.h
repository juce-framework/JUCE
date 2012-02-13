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

#ifndef __JUCE_ACTIONLISTENER_JUCEHEADER__
#define __JUCE_ACTIONLISTENER_JUCEHEADER__


//==============================================================================
/**
    Receives callbacks to indicate that some kind of event has occurred.

    Used by various classes, e.g. buttons when they are pressed, to tell listeners
    about something that's happened.

    @see ActionBroadcaster, ChangeListener
*/
class JUCE_API  ActionListener
{
public:
    /** Destructor. */
    virtual ~ActionListener()  {}

    /** Overridden by your subclass to receive the callback.

        @param message  the string that was specified when the event was triggered
                        by a call to ActionBroadcaster::sendActionMessage()
    */
    virtual void actionListenerCallback (const String& message) = 0;
};


#endif   // __JUCE_ACTIONLISTENER_JUCEHEADER__
