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

#ifndef __JUCE_CHANGELISTENERLIST_JUCEHEADER__
#define __JUCE_CHANGELISTENERLIST_JUCEHEADER__

#include "juce_ChangeListener.h"
#include "juce_MessageListener.h"
#include "../../juce_core/containers/juce_SortedSet.h"
#include "../../juce_core/threads/juce_ScopedLock.h"


//==============================================================================
/**
    A set of ChangeListeners.

    Listeners can be added and removed from the list, and change messages can be
    broadcast to all the listeners.

    @see ChangeListener, ChangeBroadcaster
*/
class JUCE_API  ChangeListenerList  : public MessageListener
{
public:
    //==============================================================================
    /** Creates an empty list. */
    ChangeListenerList() throw();

    /** Destructor. */
    ~ChangeListenerList() throw();

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
    /** Posts an asynchronous change message to all the listeners.

        If a message has already been sent and hasn't yet been delivered, this
        method won't send another - in this way it coalesces multiple frequent
        changes into fewer actual callbacks to the ChangeListeners. Contrast this
        with the ActionListener, which posts a new event for every call to its
        sendActionMessage() method.

        Only listeners which are on the list when the change event is delivered
        will receive the event - and this may include listeners that weren't on
        the list when the change message was sent.

        @param objectThatHasChanged     this pointer is passed to the
                                        ChangeListener::changeListenerCallback() method,
                                        and can be any value the application needs
        @see sendSynchronousChangeMessage
    */
    void sendChangeMessage (void* objectThatHasChanged) throw();

    /** This will synchronously callback all the ChangeListeners.

        Use this if you need to synchronously force a call to all the
        listeners' ChangeListener::changeListenerCallback() methods.
    */
    void sendSynchronousChangeMessage (void* objectThatHasChanged);

    /** If a change message has been sent but not yet dispatched, this will
        use sendSynchronousChangeMessage() to make the callback immediately.
    */
    void dispatchPendingMessages();

    //==============================================================================
    /** @internal */
    void handleMessage (const Message&);

    juce_UseDebuggingNewOperator

private:
    SortedSet <void*> listeners;
    CriticalSection lock;
    void* lastChangedObject;
    bool messagePending;

    ChangeListenerList (const ChangeListenerList&);
    const ChangeListenerList& operator= (const ChangeListenerList&);
};


#endif   // __JUCE_CHANGELISTENERLIST_JUCEHEADER__
