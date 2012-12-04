/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

class AsyncUpdater::AsyncUpdaterMessage  : public CallbackMessage
{
public:
    AsyncUpdaterMessage (AsyncUpdater& au)  : owner (au) {}

    void messageCallback()
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
    message = new AsyncUpdaterMessage (*this);
}

AsyncUpdater::~AsyncUpdater()
{
    // You're deleting this object with a background thread while there's an update
    // pending on the main event thread - that's pretty dodgy threading, as the callback could
    // happen after this destructor has finished. You should either use a MessageManagerLock while
    // deleting this object, or find some other way to avoid such a race condition.
    jassert ((! isUpdatePending()) || MessageManager::getInstance()->currentThreadHasLockedMessageManager());

    message->shouldDeliver.set (0);
}

void AsyncUpdater::triggerAsyncUpdate()
{
    if (message->shouldDeliver.compareAndSetBool (1, 0))
        message->post();
}

void AsyncUpdater::cancelPendingUpdate() noexcept
{
    message->shouldDeliver.set (0);
}

void AsyncUpdater::handleUpdateNowIfNeeded()
{
    // This can only be called by the event thread.
    jassert (MessageManager::getInstance()->currentThreadHasLockedMessageManager());

    if (message->shouldDeliver.exchange (0) != 0)
        handleAsyncUpdate();
}

bool AsyncUpdater::isUpdatePending() const noexcept
{
    return message->shouldDeliver.value != 0;
}
