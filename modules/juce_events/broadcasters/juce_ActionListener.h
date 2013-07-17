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

#ifndef JUCE_ACTIONLISTENER_H_INCLUDED
#define JUCE_ACTIONLISTENER_H_INCLUDED


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


#endif   // JUCE_ACTIONLISTENER_H_INCLUDED
