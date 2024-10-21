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

class AsyncUpdater::AsyncUpdaterMessage final : public CallbackMessage
{
public:
    AsyncUpdaterMessage (AsyncUpdater& au)  : owner (au) {}

    void messageCallback() override
    {
        if (shouldDeliver.compareAndSetBool (0, 1))
            owner.handleAsyncUpdate();
    }

    AsyncUpdater& owner;
    Atomic<int> shouldDeliver;

    JUCE_DECLARE_NON_COPYABLE (AsyncUpdaterMessage)
};

//==============================================================================
AsyncUpdater::AsyncUpdater()
{
    activeMessage = *new AsyncUpdaterMessage (*this);
}

AsyncUpdater::~AsyncUpdater()
{
    // You're deleting this object with a background thread while there's an update
    // pending on the main event thread - that's pretty dodgy threading, as the callback could
    // happen after this destructor has finished. You should either use a MessageManagerLock while
    // deleting this object, or find some other way to avoid such a race condition.
    jassert ((! isUpdatePending())
              || MessageManager::getInstanceWithoutCreating() == nullptr
              || MessageManager::getInstanceWithoutCreating()->currentThreadHasLockedMessageManager());

    activeMessage->shouldDeliver.set (0);
}

void AsyncUpdater::triggerAsyncUpdate()
{
    // If you're calling this before (or after) the MessageManager is
    // running, then you're not going to get any callbacks!
    JUCE_ASSERT_MESSAGE_MANAGER_EXISTS

    if (activeMessage->shouldDeliver.compareAndSetBool (1, 0))
        if (! activeMessage->post())
            cancelPendingUpdate(); // if the message queue fails, this avoids getting
                                   // trapped waiting for the message to arrive
}

void AsyncUpdater::cancelPendingUpdate() noexcept
{
    activeMessage->shouldDeliver.set (0);
}

void AsyncUpdater::handleUpdateNowIfNeeded()
{
    // This can only be called by the event thread.
    JUCE_ASSERT_MESSAGE_MANAGER_IS_LOCKED

    if (activeMessage->shouldDeliver.exchange (0) != 0)
        handleAsyncUpdate();
}

bool AsyncUpdater::isUpdatePending() const noexcept
{
    return activeMessage->shouldDeliver.value != 0;
}

} // namespace juce
