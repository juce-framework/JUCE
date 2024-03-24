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

class ActionBroadcaster::ActionMessage final : public MessageManager::MessageBase
{
public:
    ActionMessage (const ActionBroadcaster* ab,
                   const String& messageText, ActionListener* l) noexcept
        : broadcaster (const_cast<ActionBroadcaster*> (ab)),
          message (messageText),
          listener (l)
    {}

    void messageCallback() override
    {
        if (auto b = broadcaster.get())
            if (b->actionListeners.contains (listener))
                listener->actionListenerCallback (message);
    }

private:
    WeakReference<ActionBroadcaster> broadcaster;
    const String message;
    ActionListener* const listener;

    JUCE_DECLARE_NON_COPYABLE (ActionMessage)
};

//==============================================================================
ActionBroadcaster::ActionBroadcaster()
{
    // are you trying to create this object before or after juce has been initialised??
    JUCE_ASSERT_MESSAGE_MANAGER_EXISTS
}

ActionBroadcaster::~ActionBroadcaster()
{
    // all event-based objects must be deleted BEFORE juce is shut down!
    JUCE_ASSERT_MESSAGE_MANAGER_EXISTS
}

void ActionBroadcaster::addActionListener (ActionListener* const listener)
{
    const ScopedLock sl (actionListenerLock);

    if (listener != nullptr)
        actionListeners.add (listener);
}

void ActionBroadcaster::removeActionListener (ActionListener* const listener)
{
    const ScopedLock sl (actionListenerLock);
    actionListeners.removeValue (listener);
}

void ActionBroadcaster::removeAllActionListeners()
{
    const ScopedLock sl (actionListenerLock);
    actionListeners.clear();
}

void ActionBroadcaster::sendActionMessage (const String& message) const
{
    const ScopedLock sl (actionListenerLock);

    for (int i = actionListeners.size(); --i >= 0;)
        (new ActionMessage (this, message, actionListeners.getUnchecked (i)))->post();
}

} // namespace juce
