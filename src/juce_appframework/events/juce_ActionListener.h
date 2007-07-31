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

#ifndef __JUCE_ACTIONLISTENER_JUCEHEADER__
#define __JUCE_ACTIONLISTENER_JUCEHEADER__

#include "../../juce_core/text/juce_String.h"


//==============================================================================
/**
    Receives callbacks to indicate that some kind of event has occurred.

    Used by various classes, e.g. buttons when they are pressed, to tell listeners
    about something that's happened.

    @see ActionListenerList, ActionBroadcaster, ChangeListener
*/
class JUCE_API  ActionListener
{
public:
    /** Destructor. */
    virtual ~ActionListener()  {}

    /** Overridden by your subclass to receive the callback.

        @param message  the string that was specified when the event was triggered
                        by a call to ActionListenerList::sendActionMessage()
    */
    virtual void actionListenerCallback (const String& message) = 0;
};


#endif   // __JUCE_ACTIONLISTENER_JUCEHEADER__
