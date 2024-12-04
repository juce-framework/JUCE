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

ChangeBroadcaster::ChangeBroadcaster() noexcept
{
    broadcastCallback.owner = this;
}

ChangeBroadcaster::~ChangeBroadcaster()
{
}

void ChangeBroadcaster::addChangeListener (ChangeListener* const listener)
{
    // Listeners can only be safely added when the event thread is locked
    // You can  use a MessageManagerLock if you need to call this from another thread.
    JUCE_ASSERT_MESSAGE_MANAGER_IS_LOCKED

    changeListeners.add (listener);
    anyListeners = true;
}

void ChangeBroadcaster::removeChangeListener (ChangeListener* const listener)
{
    // Listeners can only be safely removed when the event thread is locked
    // You can  use a MessageManagerLock if you need to call this from another thread.
    JUCE_ASSERT_MESSAGE_MANAGER_IS_LOCKED

    changeListeners.remove (listener);
    anyListeners = changeListeners.size() > 0;
}

void ChangeBroadcaster::removeAllChangeListeners()
{
    // Listeners can only be safely removed when the event thread is locked
    // You can  use a MessageManagerLock if you need to call this from another thread.
    JUCE_ASSERT_MESSAGE_MANAGER_IS_LOCKED

    changeListeners.clear();
    anyListeners = false;
}

void ChangeBroadcaster::sendChangeMessage()
{
    if (anyListeners)
        broadcastCallback.triggerAsyncUpdate();
}

void ChangeBroadcaster::sendSynchronousChangeMessage()
{
    // This can only be called by the event thread.
    JUCE_ASSERT_MESSAGE_MANAGER_IS_LOCKED

    broadcastCallback.cancelPendingUpdate();
    callListeners();
}

void ChangeBroadcaster::dispatchPendingMessages()
{
    broadcastCallback.handleUpdateNowIfNeeded();
}

void ChangeBroadcaster::callListeners()
{
    changeListeners.call ([this] (ChangeListener& l) { l.changeListenerCallback (this); });
}

//==============================================================================
ChangeBroadcaster::ChangeBroadcasterCallback::ChangeBroadcasterCallback()
    : owner (nullptr)
{
}

void ChangeBroadcaster::ChangeBroadcasterCallback::handleAsyncUpdate()
{
    jassert (owner != nullptr);
    owner->callListeners();
}

} // namespace juce
