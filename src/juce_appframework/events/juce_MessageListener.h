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

#ifndef __JUCE_MESSAGELISTENER_JUCEHEADER__
#define __JUCE_MESSAGELISTENER_JUCEHEADER__

#include "juce_Message.h"


//==============================================================================
/**
    MessageListener subclasses can post and receive Message objects.

    @see Message, MessageManager, ActionListener, ChangeListener
*/
class JUCE_API  MessageListener
{
protected:
    //==============================================================================
    /** Creates a MessageListener. */
    MessageListener() throw();

public:
    //==============================================================================
    /** Destructor.

        When a MessageListener is deleted, it removes itself from a global list
        of registered listeners, so that the isValidMessageListener() method
        will no longer return true.
    */
    virtual ~MessageListener();

    //==============================================================================
    /** This is the callback method that receives incoming messages.

        This is called by the MessageManager from its dispatch loop.

        @see postMessage
    */
    virtual void handleMessage (const Message& message) = 0;

    //==============================================================================
    /** Sends a message to the message queue, for asynchronous delivery to this listener
        later on.

        This method can be called safely by any thread.

        @param message      the message object to send - this will be deleted
                            automatically by the message queue, so don't keep any
                            references to it after calling this method.
        @see handleMessage
    */
    void postMessage (Message* const message) const throw();

    //==============================================================================
    /** Checks whether this MessageListener has been deleted.

        Although not foolproof, this method is safe to call on dangling or null
        pointers. A list of active MessageListeners is kept internally, so this
        checks whether the object is on this list or not.

        Note that it's possible to get a false-positive here, if an object is
        deleted and another is subsequently created that happens to be at the
        exact same memory location, but I can't think of a good way of avoiding
        this.
    */
    bool isValidMessageListener() const throw();
};


#endif   // __JUCE_MESSAGELISTENER_JUCEHEADER__
