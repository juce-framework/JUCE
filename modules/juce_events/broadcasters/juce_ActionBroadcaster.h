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

#ifndef __JUCE_ACTIONBROADCASTER_JUCEHEADER__
#define __JUCE_ACTIONBROADCASTER_JUCEHEADER__

#include "juce_ActionListener.h"


//==============================================================================
/** Manages a list of ActionListeners, and can send them messages.

    To quickly add methods to your class that can add/remove action
    listeners and broadcast to them, you can derive from this.

    @see ActionListener, ChangeListener
*/
class JUCE_API  ActionBroadcaster
{
public:
    //==============================================================================
    /** Creates an ActionBroadcaster. */
    ActionBroadcaster();

    /** Destructor. */
    virtual ~ActionBroadcaster();

    //==============================================================================
    /** Adds a listener to the list.
        Trying to add a listener that's already on the list will have no effect.
    */
    void addActionListener (ActionListener* listener);

    /** Removes a listener from the list.
        If the listener isn't on the list, this won't have any effect.
    */
    void removeActionListener (ActionListener* listener);

    /** Removes all listeners from the list. */
    void removeAllActionListeners();

    //==============================================================================
    /** Broadcasts a message to all the registered listeners.
        @see ActionListener::actionListenerCallback
    */
    void sendActionMessage (const String& message) const;


private:
    //==============================================================================
    friend class WeakReference<ActionBroadcaster>;
    WeakReference<ActionBroadcaster>::Master masterReference;

    class ActionMessage;
    friend class ActionMessage;

    SortedSet <ActionListener*> actionListeners;
    CriticalSection actionListenerLock;

    JUCE_DECLARE_NON_COPYABLE (ActionBroadcaster)
};


#endif   // __JUCE_ACTIONBROADCASTER_JUCEHEADER__
