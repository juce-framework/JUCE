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

#ifndef __JUCE_ACTIONBROADCASTER_JUCEHEADER__
#define __JUCE_ACTIONBROADCASTER_JUCEHEADER__

#include "juce_ActionListenerList.h"


//==============================================================================
/** Manages a list of ActionListeners, and can send them messages.

    To quickly add methods to your class that can add/remove action
    listeners and broadcast to them, you can derive from this.

    @see ActionListenerList, ActionListener
*/
class JUCE_API  ActionBroadcaster
{
public:
    //==============================================================================
    /** Creates an ActionBroadcaster. */
    ActionBroadcaster() throw();

    /** Destructor. */
    virtual ~ActionBroadcaster();

    //==============================================================================
    /** Adds a listener to the list.

        (Trying to add a listener that's already on the list will have no effect).
    */
    void addActionListener (ActionListener* const listener);

    /** Removes a listener from the list.

        If the listener isn't on the list, this won't have any effect.
    */
    void removeActionListener (ActionListener* const listener);

    /** Removes all listeners from the list. */
    void removeAllActionListeners();

    //==============================================================================
    /** Broadcasts a message to all the registered listeners.

        @see ActionListenerList::sendActionMessage
    */
    void sendActionMessage (const String& message) const;


private:
    //==============================================================================
    ActionListenerList actionListenerList;

    ActionBroadcaster (const ActionBroadcaster&);
    const ActionBroadcaster& operator= (const ActionBroadcaster&);
};


#endif   // __JUCE_ACTIONBROADCASTER_JUCEHEADER__
