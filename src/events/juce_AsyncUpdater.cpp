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

#include "../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_AsyncUpdater.h"
#include "juce_CallbackMessage.h"
#include "juce_MessageManager.h"


//==============================================================================
class AsyncUpdaterMessage  : public CallbackMessage
{
public:
    AsyncUpdaterMessage (AsyncUpdater& owner_)
        : owner (owner_)
    {
    }

    void messageCallback()
    {
        if (shouldDeliver.compareAndSetBool (0, 1))
            owner.handleAsyncUpdate();
    }

    Atomic<int> shouldDeliver;

private:
    AsyncUpdater& owner;
};

//==============================================================================
AsyncUpdater::AsyncUpdater()
{
    message = new AsyncUpdaterMessage (*this);
}

inline Atomic<int>& AsyncUpdater::getDeliveryFlag() const noexcept
{
    return static_cast <AsyncUpdaterMessage*> (message.getObject())->shouldDeliver;
}

AsyncUpdater::~AsyncUpdater()
{
    // You're deleting this object with a background thread while there's an update
    // pending on the main event thread - that's pretty dodgy threading, as the callback could
    // happen after this destructor has finished. You should either use a MessageManagerLock while
    // deleting this object, or find some other way to avoid such a race condition.
    jassert ((! isUpdatePending()) || MessageManager::getInstance()->currentThreadHasLockedMessageManager());

    getDeliveryFlag().set (0);
}

void AsyncUpdater::triggerAsyncUpdate()
{
    if (getDeliveryFlag().compareAndSetBool (1, 0))
        message->post();
}

void AsyncUpdater::cancelPendingUpdate() noexcept
{
    getDeliveryFlag().set (0);
}

void AsyncUpdater::handleUpdateNowIfNeeded()
{
    // This can only be called by the event thread.
    jassert (MessageManager::getInstance()->currentThreadHasLockedMessageManager());

    if (getDeliveryFlag().exchange (0) != 0)
        handleAsyncUpdate();
}

bool AsyncUpdater::isUpdatePending() const noexcept
{
    return getDeliveryFlag().value != 0;
}


END_JUCE_NAMESPACE
