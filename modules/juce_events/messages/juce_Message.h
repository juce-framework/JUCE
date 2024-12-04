/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

class MessageListener;


//==============================================================================
/** The base class for objects that can be sent to a MessageListener.

    If you want to send a message that carries some kind of custom data, just
    create a subclass of Message with some appropriate member variables to hold
    your data.

    Always create a new instance of a Message object on the heap, as it will be
    deleted automatically after the message has been delivered.

    @see MessageListener, MessageManager, ActionListener, ChangeListener

    @tags{Events}
*/
class JUCE_API  Message  : public MessageManager::MessageBase
{
public:
    //==============================================================================
    /** Creates an uninitialised message. */
    Message() noexcept;
    ~Message() override;

    using Ptr = ReferenceCountedObjectPtr<Message>;

    //==============================================================================
private:
    friend class MessageListener;
    WeakReference<MessageListener> recipient;
    void messageCallback() override;

    // Avoid the leak-detector because for plugins, the host can unload our DLL with undelivered
    // messages still in the system event queue. These aren't harmful, but can cause annoying assertions.
    JUCE_DECLARE_NON_COPYABLE (Message)
};

} // namespace juce
