/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2016 - ROLI Ltd.

   Permission is granted to use this software under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license/

   Permission to use, copy, modify, and/or distribute this software for any
   purpose with or without fee is hereby granted, provided that the above
   copyright notice and this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES WITH REGARD
   TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
   FITNESS. IN NO EVENT SHALL ISC BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT,
   OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
   USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
   TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
   OF THIS SOFTWARE.

   -----------------------------------------------------------------------------

   To release a closed-source product which uses other parts of JUCE not
   licensed under the ISC terms, commercial licenses are available: visit
   www.juce.com for more information.

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
