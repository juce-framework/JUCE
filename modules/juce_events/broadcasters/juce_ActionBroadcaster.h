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

#ifndef JUCE_ACTIONBROADCASTER_H_INCLUDED
#define JUCE_ACTIONBROADCASTER_H_INCLUDED


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

    SortedSet<ActionListener*> actionListeners;
    CriticalSection actionListenerLock;

    JUCE_DECLARE_NON_COPYABLE (ActionBroadcaster)
};


#endif   // JUCE_ACTIONBROADCASTER_H_INCLUDED
