/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

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

#ifndef JUCE_MESSAGELISTENER_H_INCLUDED
#define JUCE_MESSAGELISTENER_H_INCLUDED


//==============================================================================
/**
    MessageListener subclasses can post and receive Message objects.

    @see Message, MessageManager, ActionListener, ChangeListener
*/
class JUCE_API  MessageListener
{
public:
    //==============================================================================
    MessageListener() noexcept;

    /** Destructor. */
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
                            automatically by the message queue, so make sure it's
                            allocated on the heap, not the stack!
        @see handleMessage
    */
    void postMessage (Message* message) const;

private:
    WeakReference<MessageListener>::Master masterReference;
    friend class WeakReference<MessageListener>;
};


#endif   // JUCE_MESSAGELISTENER_H_INCLUDED
