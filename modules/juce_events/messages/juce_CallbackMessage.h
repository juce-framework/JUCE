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

#ifndef JUCE_CALLBACKMESSAGE_H_INCLUDED
#define JUCE_CALLBACKMESSAGE_H_INCLUDED


//==============================================================================
/**
    A message that invokes a callback method when it gets delivered.

    You can use this class to fire off actions that you want to be performed later
    on the message thread.

    To use it, create a subclass of CallbackMessage which implements the messageCallback()
    method, then call post() to dispatch it. The event thread will then invoke your
    messageCallback() method later on, and will automatically delete the message object
    afterwards.

    Always create a new instance of a CallbackMessage on the heap, as it will be
    deleted automatically after the message has been delivered.

    @see MessageManager, MessageListener, ActionListener, ChangeListener
*/
class JUCE_API  CallbackMessage   : public MessageManager::MessageBase
{
public:
    //==============================================================================
    CallbackMessage() noexcept {}

    /** Destructor. */
    ~CallbackMessage() {}

    //==============================================================================
    /** Called when the message is delivered.

        You should implement this method and make it do whatever action you want
        to perform.

        Note that like all other messages, this object will be deleted immediately
        after this method has been invoked.
    */
    virtual void messageCallback() = 0;

private:
    // Avoid the leak-detector because for plugins, the host can unload our DLL with undelivered
    // messages still in the system event queue. These aren't harmful, but can cause annoying assertions.
    JUCE_DECLARE_NON_COPYABLE (CallbackMessage)
};


#endif   // JUCE_CALLBACKMESSAGE_H_INCLUDED
