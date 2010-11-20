/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

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
#include "../containers/juce_ScopedPointer.h"
#include "juce_MessageManager.h"


//==============================================================================
class AsyncUpdater::AsyncUpdaterMessage  : public CallbackMessage
{
public:
    AsyncUpdaterMessage (AsyncUpdater& owner_)
        : owner (owner_)
    {
    }

    void messageCallback()
    {
        if (owner.pendingMessage.compareAndSetBool (0, this))
            owner.handleAsyncUpdate();
    }

    AsyncUpdater& owner;
};

//==============================================================================
AsyncUpdater::AsyncUpdater() throw()
{
}

AsyncUpdater::~AsyncUpdater()
{
    // You're deleting this object with a background thread while there's an update
    // pending on the main event thread - that's pretty dodgy threading, as the callback could
    // happen after this destructor has finished. You should either use a MessageManagerLock while
    // deleting this object, or find some other way to avoid such a race condition.
    jassert (/*(! isUpdatePending()) ||*/ MessageManager::getInstance()->currentThreadHasLockedMessageManager());

    pendingMessage = 0;
}

void AsyncUpdater::triggerAsyncUpdate()
{
    if (pendingMessage.value == 0)
    {
        ScopedPointer<AsyncUpdaterMessage> pending (new AsyncUpdaterMessage (*this));

        if (pendingMessage.compareAndSetBool (pending, 0))
            pending.release()->post();
    }
}

void AsyncUpdater::cancelPendingUpdate() throw()
{
    pendingMessage = 0;
}

void AsyncUpdater::handleUpdateNowIfNeeded()
{
    // This can only be called by the event thread.
    jassert (MessageManager::getInstance()->currentThreadHasLockedMessageManager());

    if (pendingMessage.exchange (0) != 0)
        handleAsyncUpdate();
}

bool AsyncUpdater::isUpdatePending() const throw()
{
    return pendingMessage.value != 0;
}


END_JUCE_NAMESPACE
