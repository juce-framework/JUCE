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

    Note that this class was essential back in the days before C++11, but in modern
    times you may prefer to use MessageManager::callAsync() with a lambda.

    @see MessageManager::callAsync, MessageListener, ActionListener, ChangeListener

    @tags{Events}
*/
class JUCE_API  CallbackMessage   : public MessageManager::MessageBase
{
public:
    //==============================================================================
    CallbackMessage() = default;

    /** Destructor. */
    ~CallbackMessage() override = default;

    //==============================================================================
    /** Called when the message is delivered.

        You should implement this method and make it do whatever action you want
        to perform.

        Note that like all other messages, this object will be deleted immediately
        after this method has been invoked.
    */
    void messageCallback() override = 0;

private:
    // Avoid the leak-detector because for plugins, the host can unload our DLL with undelivered
    // messages still in the system event queue. These aren't harmful, but can cause annoying assertions.
    JUCE_DECLARE_NON_COPYABLE (CallbackMessage)
};

} // namespace juce
