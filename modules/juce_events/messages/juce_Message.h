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

#ifndef JUCE_MESSAGE_H_INCLUDED
#define JUCE_MESSAGE_H_INCLUDED

class MessageListener;


//==============================================================================
/** The base class for objects that can be sent to a MessageListener.

    If you want to send a message that carries some kind of custom data, just
    create a subclass of Message with some appropriate member variables to hold
    your data.

    Always create a new instance of a Message object on the heap, as it will be
    deleted automatically after the message has been delivered.

    @see MessageListener, MessageManager, ActionListener, ChangeListener
*/
class JUCE_API  Message  : public MessageManager::MessageBase
{
public:
    //==============================================================================
    /** Creates an uninitialised message. */
    Message() noexcept;
    ~Message();

    typedef ReferenceCountedObjectPtr<Message> Ptr;

    //==============================================================================
private:
    friend class MessageListener;
    WeakReference<MessageListener> recipient;
    void messageCallback() override;

    // Avoid the leak-detector because for plugins, the host can unload our DLL with undelivered
    // messages still in the system event queue. These aren't harmful, but can cause annoying assertions.
    JUCE_DECLARE_NON_COPYABLE (Message)
};


#endif   // JUCE_MESSAGE_H_INCLUDED
