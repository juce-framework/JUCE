/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

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

ChangeBroadcaster::ChangeBroadcaster() noexcept
{
    callback.owner = this;
}

ChangeBroadcaster::~ChangeBroadcaster()
{
}

void ChangeBroadcaster::addChangeListener (ChangeListener* const listener)
{
    // Listeners can only be safely added when the event thread is locked
    // You can  use a MessageManagerLock if you need to call this from another thread.
    jassert (MessageManager::getInstance()->currentThreadHasLockedMessageManager());

    changeListeners.add (listener);
}

void ChangeBroadcaster::removeChangeListener (ChangeListener* const listener)
{
    // Listeners can only be safely added when the event thread is locked
    // You can  use a MessageManagerLock if you need to call this from another thread.
    jassert (MessageManager::getInstance()->currentThreadHasLockedMessageManager());

    changeListeners.remove (listener);
}

void ChangeBroadcaster::removeAllChangeListeners()
{
    // Listeners can only be safely added when the event thread is locked
    // You can  use a MessageManagerLock if you need to call this from another thread.
    jassert (MessageManager::getInstance()->currentThreadHasLockedMessageManager());

    changeListeners.clear();
}

void ChangeBroadcaster::sendChangeMessage()
{
    if (changeListeners.size() > 0)
        callback.triggerAsyncUpdate();
}

void ChangeBroadcaster::sendSynchronousChangeMessage()
{
    // This can only be called by the event thread.
    jassert (MessageManager::getInstance()->isThisTheMessageThread());

    callback.cancelPendingUpdate();
    callListeners();
}

void ChangeBroadcaster::dispatchPendingMessages()
{
    callback.handleUpdateNowIfNeeded();
}

void ChangeBroadcaster::callListeners()
{
    changeListeners.call (&ChangeListener::changeListenerCallback, this);
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
