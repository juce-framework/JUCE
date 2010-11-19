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

#include "juce_ChangeBroadcaster.h"
#include "juce_MessageManager.h"


//==============================================================================
class ChangeBroadcaster::ChangeBroadcasterMessage  : public CallbackMessage
{
public:
    ChangeBroadcasterMessage (ChangeBroadcaster* const owner_)
        : owner (owner_)
    {
    }

    void messageCallback()
    {
        if (owner != 0 && owner->pendingMessage.value == this)
            owner->sendSynchronousChangeMessage();
    }

    ChangeBroadcaster* owner;
};

//==============================================================================
ChangeBroadcaster::ChangeBroadcaster() throw()
{
    // are you trying to create this object before or after juce has been intialised??
    jassert (MessageManager::instance != 0);
}

ChangeBroadcaster::~ChangeBroadcaster()
{
    // all event-based objects must be deleted BEFORE juce is shut down!
    jassert (MessageManager::instance != 0);

    invalidatePendingMessage();
}

void ChangeBroadcaster::addChangeListener (ChangeListener* const listener)
{
    // Listeners can only be safely added when the event thread is locked...
    jassert (MessageManager::getInstance()->currentThreadHasLockedMessageManager());

    changeListeners.add (listener);
}

void ChangeBroadcaster::removeChangeListener (ChangeListener* const listener)
{
    // Listeners can only be safely added when the event thread is locked...
    jassert (MessageManager::getInstance()->currentThreadHasLockedMessageManager());

    changeListeners.remove (listener);
}

void ChangeBroadcaster::removeAllChangeListeners()
{
    // Listeners can only be safely added when the event thread is locked...
    jassert (MessageManager::getInstance()->currentThreadHasLockedMessageManager());

    changeListeners.clear();
}

void ChangeBroadcaster::invalidatePendingMessage()
{
    ChangeBroadcasterMessage* const oldMessage = pendingMessage.exchange (0);
    if (oldMessage != 0)
        oldMessage->owner = 0;
}

void ChangeBroadcaster::sendChangeMessage()
{
    if (pendingMessage.value == 0 && changeListeners.size() > 0)
    {
        ScopedPointer<ChangeBroadcasterMessage> pending (new ChangeBroadcasterMessage (this));

        if (pendingMessage.compareAndSetBool (pending, 0))
            pending.release()->post();
    }
}

void ChangeBroadcaster::sendSynchronousChangeMessage()
{
    // This can only be called by the event thread.
    jassert (MessageManager::getInstance()->isThisTheMessageThread());

    invalidatePendingMessage();
    changeListeners.call (&ChangeListener::changeListenerCallback, this);
}

void ChangeBroadcaster::dispatchPendingMessages()
{
    if (pendingMessage.get() != 0)
        sendSynchronousChangeMessage();
}


END_JUCE_NAMESPACE
