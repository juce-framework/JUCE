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

#ifndef __JUCE_CALLBACKMESSAGE_JUCEHEADER__
#define __JUCE_CALLBACKMESSAGE_JUCEHEADER__

#include "juce_Message.h"

//==============================================================================
/**
    A message that calls a custom function when it gets delivered.

    You can use this class to fire off actions that you want to be performed later
    on the message thread.

    Unlike other Message objects, these don't get sent to a MessageListener, you
    just call the post() method to send them, and when they arrive, your
    messageCallback() method will automatically be invoked.

    @see MessageListener, MessageManager, ActionListener, ChangeListener
*/
class JUCE_API  CallbackMessage   : public Message
{
public:
    //==============================================================================
    CallbackMessage() throw();

    /** Destructor. */
    ~CallbackMessage() throw();

    //==============================================================================
    /** Called when the message is delivered.

        You should implement this method and make it do whatever action you want
        to perform.

        Note that like all other messages, this object will be deleted immediately
        after this method has been invoked.
    */
    virtual void messageCallback() = 0;

    /** Instead of sending this message to a MessageListener, just call this method
        to post it to the event queue.

        After you've called this, this object will belong to the MessageManager,
        which will delete it later. So make sure you don't delete the object yourself,
        call post() more than once, or call post() on a stack-based obect!
    */
    void post();

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    CallbackMessage (const CallbackMessage&);
    const CallbackMessage& operator= (const CallbackMessage&);
};


#endif   // __JUCE_CALLBACKMESSAGE_JUCEHEADER__
