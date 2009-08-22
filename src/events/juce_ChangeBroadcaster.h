/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

#ifndef __JUCE_CHANGEBROADCASTER_JUCEHEADER__
#define __JUCE_CHANGEBROADCASTER_JUCEHEADER__

#include "juce_ChangeListenerList.h"


//==============================================================================
/** Manages a list of ChangeListeners, and can send them messages.

    To quickly add methods to your class that can add/remove change
    listeners and broadcast to them, you can derive from this.

    @see ChangeListenerList, ChangeListener
*/
class JUCE_API  ChangeBroadcaster
{
public:
    //==============================================================================
    /** Creates an ChangeBroadcaster. */
    ChangeBroadcaster() throw();

    /** Destructor. */
    virtual ~ChangeBroadcaster();

    //==============================================================================
    /** Adds a listener to the list.

        (Trying to add a listener that's already on the list will have no effect).
    */
    void addChangeListener (ChangeListener* const listener) throw();

    /** Removes a listener from the list.

        If the listener isn't on the list, this won't have any effect.
    */
    void removeChangeListener (ChangeListener* const listener) throw();

    /** Removes all listeners from the list. */
    void removeAllChangeListeners() throw();

    //==============================================================================
    /** Broadcasts a change message to all the registered listeners.

        The message will be delivered asynchronously by the event thread, so this
        method will not directly call any of the listeners. For a synchronous
        message, use sendSynchronousChangeMessage().

        @see ChangeListenerList::sendActionMessage
    */
    void sendChangeMessage (void* objectThatHasChanged) throw();

    /** Sends a synchronous change message to all the registered listeners.

        @see ChangeListenerList::sendSynchronousChangeMessage
    */
    void sendSynchronousChangeMessage (void* objectThatHasChanged);

    /** If a change message has been sent but not yet dispatched, this will
        use sendSynchronousChangeMessage() to make the callback immediately.
    */
    void dispatchPendingMessages();


private:
    //==============================================================================
    ChangeListenerList changeListenerList;

    ChangeBroadcaster (const ChangeBroadcaster&);
    const ChangeBroadcaster& operator= (const ChangeBroadcaster&);
};


#endif   // __JUCE_CHANGEBROADCASTER_JUCEHEADER__
