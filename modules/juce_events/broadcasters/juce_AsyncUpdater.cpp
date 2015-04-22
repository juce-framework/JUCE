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

class AsyncUpdater::AsyncUpdaterMessage  : public CallbackMessage
{
public:
    AsyncUpdaterMessage (AsyncUpdater& au)  : owner (au) {}

    void messageCallback() override
    {
        if (shouldDeliver.compareAndSetBool (0, 1))
            owner.handleAsyncUpdate();
    }

    Atomic<int> shouldDeliver;

private:
    AsyncUpdater& owner;

    JUCE_DECLARE_NON_COPYABLE (AsyncUpdaterMessage)
};

//==============================================================================
AsyncUpdater::AsyncUpdater()
{
    activeMessage = new AsyncUpdaterMessage (*this);
}

AsyncUpdater::~AsyncUpdater()
{
    // You're deleting this object with a background thread while there's an update
    // pending on the main event thread - that's pretty dodgy threading, as the callback could
    // happen after this destructor has finished. You should either use a MessageManagerLock while
    // deleting this object, or find some other way to avoid such a race condition.
    jassert ((! isUpdatePending()) || MessageManager::getInstance()->currentThreadHasLockedMessageManager());

    activeMessage->shouldDeliver.set (0);
}

void AsyncUpdater::triggerAsyncUpdate()
{
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
    jassert (MessageManager::getInstance()->currentThreadHasLockedMessageManager());

    if (activeMessage->shouldDeliver.exchange (0) != 0)
        handleAsyncUpdate();
}

bool AsyncUpdater::isUpdatePending() const noexcept
{
    return activeMessage->shouldDeliver.value != 0;
}
