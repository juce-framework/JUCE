/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#include "../../juce_core/basics/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_MessageManager.h"
#include "juce_ActionListenerList.h"
#include "../application/juce_Application.h"
#include "../gui/components/juce_Component.h"
#include "../../juce_core/threads/juce_Thread.h"
#include "../../juce_core/basics/juce_Time.h"


//==============================================================================
// platform-specific functions..
bool juce_dispatchNextMessageOnSystemQueue (bool returnIfNoPendingMessages);
bool juce_postMessageToSystemQueue (void* message);


//==============================================================================
MessageManager* MessageManager::instance = 0;

static const int quitMessageId = 0xfffff321;

MessageManager::MessageManager() throw()
  : broadcastListeners (0),
    quitMessagePosted (false),
    quitMessageReceived (false),
    useMaximumForceWhenQuitting (true),
    messageCounter (0),
    lastMessageCounter (-1),
    isInMessageDispatcher (0),
    needToGetRidOfWaitCursor (false),
    timeBeforeWaitCursor (0),
    lastActivityCheckOkTime (0)
{
    currentLockingThreadId = messageThreadId = Thread::getCurrentThreadId();
}

MessageManager::~MessageManager() throw()
{
    jassert (instance == this);
    instance = 0;
    deleteAndZero (broadcastListeners);

    doPlatformSpecificShutdown();
}

MessageManager* MessageManager::getInstance() throw()
{
    if (instance == 0)
    {
        instance = new MessageManager();
        doPlatformSpecificInitialisation();

        instance->setTimeBeforeShowingWaitCursor (500);
    }

    return instance;
}

void MessageManager::postMessageToQueue (Message* const message)
{
    if (quitMessagePosted || ! juce_postMessageToSystemQueue (message))
        delete message;
}

//==============================================================================
// not for public use..
void MessageManager::deliverMessage (void* message)
{
    const MessageManagerLock lock;

    Message* const m = (Message*) message;
    MessageListener* const recipient = m->messageRecipient;

    if (messageListeners.contains (recipient))
    {
        JUCE_TRY
        {
            recipient->handleMessage (*m);
        }
        JUCE_CATCH_EXCEPTION

        if (needToGetRidOfWaitCursor)
        {
            needToGetRidOfWaitCursor = false;
            MouseCursor::hideWaitCursor();
        }

        ++messageCounter;
    }
    else if (recipient == 0 && m->intParameter1 == quitMessageId)
    {
        quitMessageReceived = true;
        useMaximumForceWhenQuitting = (m->intParameter2 != 0);
    }

    delete m;
}

//==============================================================================
bool MessageManager::dispatchNextMessage (const bool returnImmediatelyIfNoMessages,
                                          bool* const wasAMessageDispatched)
{
    if (quitMessageReceived)
    {
        if (wasAMessageDispatched != 0)
            *wasAMessageDispatched = false;

        return false;
    }

    ++isInMessageDispatcher;

    bool result = false;

    JUCE_TRY
    {
        result = juce_dispatchNextMessageOnSystemQueue (returnImmediatelyIfNoMessages);

        if (wasAMessageDispatched != 0)
            *wasAMessageDispatched = result;

        if (instance == 0)
            return false;
    }
    JUCE_CATCH_EXCEPTION

    --isInMessageDispatcher;
    ++messageCounter;

    return result || ! returnImmediatelyIfNoMessages;
}

void MessageManager::dispatchPendingMessages (int maxNumberOfMessagesToDispatch)
{
    jassert (isThisTheMessageThread()); // must only be called by the message thread

    while (--maxNumberOfMessagesToDispatch >= 0 && ! quitMessageReceived)
    {
        ++isInMessageDispatcher;

        bool carryOn = false;

        JUCE_TRY
        {
            carryOn = juce_dispatchNextMessageOnSystemQueue (true);
        }
        JUCE_CATCH_EXCEPTION

        --isInMessageDispatcher;
        ++messageCounter;

        if (! carryOn)
            break;
    }
}

bool MessageManager::runDispatchLoop()
{
    jassert (isThisTheMessageThread()); // must only be called by the message thread

    while (dispatchNextMessage())
    {
    }

    return useMaximumForceWhenQuitting;
}

//==============================================================================
void MessageManager::postQuitMessage (const bool useMaximumForce)
{
    Message* const m = new Message (quitMessageId, (useMaximumForce) ? 1 : 0, 0, 0);
    m->messageRecipient = 0;
    postMessageToQueue (m);

    quitMessagePosted = true;
}

bool MessageManager::hasQuitMessageBeenPosted() const throw()
{
    return quitMessagePosted;
}

//==============================================================================
void MessageManager::deliverBroadcastMessage (const String& value)
{
    if (broadcastListeners != 0)
        broadcastListeners->sendActionMessage (value);
}

void MessageManager::registerBroadcastListener (ActionListener* const listener) throw()
{
    if (broadcastListeners == 0)
        broadcastListeners = new ActionListenerList();

    broadcastListeners->addActionListener (listener);
}

void MessageManager::deregisterBroadcastListener (ActionListener* const listener) throw()
{
    if (broadcastListeners != 0)
        broadcastListeners->removeActionListener (listener);
}

//==============================================================================
// This gets called occasionally by the timer thread (to save using an extra thread
// for it).
void MessageManager::inactivityCheckCallback() throw()
{
    if (instance != 0)
        instance->inactivityCheckCallbackInt();
}

void MessageManager::inactivityCheckCallbackInt() throw()
{
    const unsigned int now = Time::getApproximateMillisecondCounter();

    if (isInMessageDispatcher > 0
         && lastMessageCounter == messageCounter
         && timeBeforeWaitCursor > 0
         && lastActivityCheckOkTime > 0
         && ! ModifierKeys::getCurrentModifiersRealtime().isAnyMouseButtonDown())
    {
        if (now >= lastActivityCheckOkTime + timeBeforeWaitCursor
             && ! needToGetRidOfWaitCursor)
        {
            // been in the same message call too long..
            MouseCursor::showWaitCursor();
            needToGetRidOfWaitCursor = true;
        }
    }
    else
    {
        lastActivityCheckOkTime = now;
        lastMessageCounter = messageCounter;
    }
}

void MessageManager::delayWaitCursor() throw()
{
    if (instance != 0)
    {
        instance->messageCounter++;

        if (instance->needToGetRidOfWaitCursor)
        {
            instance->needToGetRidOfWaitCursor = false;
            MouseCursor::hideWaitCursor();
        }
    }
}

void MessageManager::setTimeBeforeShowingWaitCursor (const int millisecs) throw()
{
     // if this is a bit too small you'll get a lot of unwanted hourglass cursors..
    jassert (millisecs <= 0 || millisecs > 200);

    timeBeforeWaitCursor = millisecs;

    if (millisecs > 0)
        startTimer (millisecs / 2); // (see timerCallback() for explanation of this)
    else
        stopTimer();
}

void MessageManager::timerCallback()
{
    // dummy callback - the message manager is just a Timer to ensure that there are always
    // some events coming in - otherwise it'll show the egg-timer/beachball-of-death.
    ++messageCounter;
}

int MessageManager::getTimeBeforeShowingWaitCursor() const throw()
{
    return timeBeforeWaitCursor;
}

bool MessageManager::isThisTheMessageThread() const throw()
{
    return Thread::getCurrentThreadId() == messageThreadId;
}

void MessageManager::setCurrentMessageThread (const int threadId) throw()
{
    messageThreadId = threadId;
}

bool MessageManager::currentThreadHasLockedMessageManager() const throw()
{
    return Thread::getCurrentThreadId() == currentLockingThreadId;
}

//==============================================================================
MessageManagerLock::MessageManagerLock() throw()
    : locked (false)
{
    if (MessageManager::instance != 0)
    {
        MessageManager::instance->messageDispatchLock.enter();
        lastLockingThreadId = MessageManager::instance->currentLockingThreadId;
        MessageManager::instance->currentLockingThreadId = Thread::getCurrentThreadId();
        locked = true;
    }
}

MessageManagerLock::MessageManagerLock (Thread* const thread) throw()
    : locked (false)
{
    jassert (thread != 0);  // This will only work if you give it a valid thread!

    if (MessageManager::instance != 0)
    {
        for (;;)
        {
            if (MessageManager::instance->messageDispatchLock.tryEnter())
            {
                locked = true;
                lastLockingThreadId = MessageManager::instance->currentLockingThreadId;
                MessageManager::instance->currentLockingThreadId = Thread::getCurrentThreadId();
                break;
            }

            if (thread != 0 && thread->threadShouldExit())
                break;

            Thread::sleep (1);
        }
    }
}

MessageManagerLock::~MessageManagerLock() throw()
{
    if (locked && MessageManager::instance != 0)
    {
        MessageManager::instance->currentLockingThreadId = lastLockingThreadId;
        MessageManager::instance->messageDispatchLock.exit();
    }
}


END_JUCE_NAMESPACE
