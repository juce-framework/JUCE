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

#ifndef __JUCE_ACTIONLISTENERLIST_JUCEHEADER__
#define __JUCE_ACTIONLISTENERLIST_JUCEHEADER__

#include "juce_ActionListener.h"
#include "juce_MessageListener.h"
#include "../../juce_core/containers/juce_SortedSet.h"
#include "../../juce_core/threads/juce_CriticalSection.h"


//==============================================================================
/**
    A set of ActionListeners.

    Listeners can be added and removed from the list, and messages can be
    broadcast to all the listeners.

    @see ActionListener, ActionBroadcaster
*/
class JUCE_API  ActionListenerList  : public MessageListener
{
public:
    //==============================================================================
    /** Creates an empty list. */
    ActionListenerList() throw();

    /** Destructor. */
    ~ActionListenerList() throw();

    //==============================================================================
    /** Adds a listener to the list.

        (Trying to add a listener that's already on the list will have no effect).
    */
    void addActionListener (ActionListener* const listener) throw();

    /** Removes a listener from the list.

        If the listener isn't on the list, this won't have any effect.
    */
    void removeActionListener (ActionListener* const listener) throw();

    /** Removes all listeners from the list. */
    void removeAllActionListeners() throw();

    /** Broadcasts a message to all the registered listeners.

        This sends the message asynchronously.

        If a listener is on the list when this method is called but is removed from
        the list before the message arrives, it won't receive the message. Similarly
        listeners that are added to the list after the message is sent but before it
        arrives won't get the message either.
    */
    void sendActionMessage (const String& message) const;

    //==============================================================================
    /** @internal */
    void handleMessage (const Message&);

    juce_UseDebuggingNewOperator

private:
    SortedSet <void*> actionListeners_;
    CriticalSection actionListenerLock_;

    ActionListenerList (const ActionListenerList&);
    const ActionListenerList& operator= (const ActionListenerList&);
};


#endif   // __JUCE_ACTIONLISTENERLIST_JUCEHEADER__
