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

#include "juce_MessageManager.h"
#include "juce_ActionBroadcaster.h"
#include "../application/juce_Application.h"
#include "../gui/components/juce_Component.h"
#include "../threads/juce_Thread.h"
#include "../threads/juce_ScopedLock.h"
#include "../core/juce_Time.h"


//==============================================================================
// platform-specific functions..
bool juce_dispatchNextMessageOnSystemQueue (bool returnIfNoPendingMessages);
bool juce_postMessageToSystemQueue (Message* message);

//==============================================================================
MessageManager* MessageManager::instance = 0;

static const int quitMessageId = 0xfffff321;

MessageManager::MessageManager() throw()
  : quitMessagePosted (false),
    quitMessageReceived (false),
    threadWithLock (0)
{
    messageThreadId = Thread::getCurrentThreadId();

    if (JUCEApplication::isStandaloneApp())
        Thread::setCurrentThreadName ("Juce Message Thread");
}

MessageManager::~MessageManager() throw()
{
    broadcaster = 0;

    doPlatformSpecificShutdown();

    // If you hit this assertion, then you've probably leaked some kind of MessageListener object..
    jassert (messageListeners.size() == 0);

    jassert (instance == this);
    instance = 0;  // do this last in case this instance is still needed by doPlatformSpecificShutdown()
}

MessageManager* MessageManager::getInstance() throw()
{
    if (instance == 0)
    {
        instance = new MessageManager();
        doPlatformSpecificInitialisation();
    }

    return instance;
}

void MessageManager::postMessageToQueue (Message* const message)
{
    if (quitMessagePosted || ! juce_postMessageToSystemQueue (message))
        Message::Ptr deleter (message); // (this will delete messages that were just created with a 0 ref count)
}

//==============================================================================
CallbackMessage::CallbackMessage() throw() {}
CallbackMessage::~CallbackMessage() {}

void CallbackMessage::post()
{
    if (MessageManager::instance != 0)
        MessageManager::instance->postMessageToQueue (this);
}

//==============================================================================
// not for public use..
void MessageManager::deliverMessage (Message* const message)
{
    JUCE_TRY
    {
        MessageListener* const recipient = message->messageRecipient;

        if (recipient == 0)
        {
            CallbackMessage* const callbackMessage = dynamic_cast <CallbackMessage*> (message);

            if (callbackMessage != 0)
            {
                callbackMessage->messageCallback();
            }
            else if (message->intParameter1 == quitMessageId)
            {
                quitMessageReceived = true;
            }
        }
        else if (messageListeners.contains (recipient))
        {
            recipient->handleMessage (*message);
        }
    }
    JUCE_CATCH_EXCEPTION
}

//==============================================================================
#if ! (JUCE_MAC || JUCE_IOS)
void MessageManager::runDispatchLoop()
{
    jassert (isThisTheMessageThread()); // must only be called by the message thread

    runDispatchLoopUntil (-1);
}

void MessageManager::stopDispatchLoop()
{
    postMessageToQueue (new Message (quitMessageId, 0, 0, 0));
    quitMessagePosted = true;
}

bool MessageManager::runDispatchLoopUntil (int millisecondsToRunFor)
{
    jassert (isThisTheMessageThread()); // must only be called by the message thread

    const int64 endTime = Time::currentTimeMillis() + millisecondsToRunFor;

    while ((millisecondsToRunFor < 0 || endTime > Time::currentTimeMillis())
            && ! quitMessageReceived)
    {
        JUCE_TRY
        {
            if (! juce_dispatchNextMessageOnSystemQueue (millisecondsToRunFor >= 0))
            {
                const int msToWait = (int) (endTime - Time::currentTimeMillis());

                if (msToWait > 0)
                    Thread::sleep (jmin (5, msToWait));
            }
        }
        JUCE_CATCH_EXCEPTION
    }

    return ! quitMessageReceived;
}

#endif

//==============================================================================
void MessageManager::deliverBroadcastMessage (const String& value)
{
    if (broadcaster != 0)
        broadcaster->sendActionMessage (value);
}

void MessageManager::registerBroadcastListener (ActionListener* const listener)
{
    if (broadcaster == 0)
        broadcaster = new ActionBroadcaster();

    broadcaster->addActionListener (listener);
}

void MessageManager::deregisterBroadcastListener (ActionListener* const listener)
{
    if (broadcaster != 0)
        broadcaster->removeActionListener (listener);
}

//==============================================================================
bool MessageManager::isThisTheMessageThread() const throw()
{
    return Thread::getCurrentThreadId() == messageThreadId;
}

void MessageManager::setCurrentThreadAsMessageThread()
{
    const Thread::ThreadID thisThread = Thread::getCurrentThreadId();

    if (messageThreadId != thisThread)
    {
        messageThreadId = thisThread;

        // This is needed on windows to make sure the message window is created by this thread
        doPlatformSpecificShutdown();
        doPlatformSpecificInitialisation();
    }
}

bool MessageManager::currentThreadHasLockedMessageManager() const throw()
{
    const Thread::ThreadID thisThread = Thread::getCurrentThreadId();
    return thisThread == messageThreadId || thisThread == threadWithLock;
}

//==============================================================================
//==============================================================================
/*  The only safe way to lock the message thread while another thread does
    some work is by posting a special message, whose purpose is to tie up the event
    loop until the other thread has finished its business.

    Any other approach can get horribly deadlocked if the OS uses its own hidden locks which
    get locked before making an event callback, because if the same OS lock gets indirectly
    accessed from another thread inside a MM lock, you're screwed. (this is exactly what happens
    in Cocoa).
*/
class MessageManagerLock::BlockingMessage   : public CallbackMessage
{
public:
    BlockingMessage() {}

    void messageCallback()
    {
        lockedEvent.signal();
        releaseEvent.wait();
    }

    WaitableEvent lockedEvent, releaseEvent;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BlockingMessage);
};

//==============================================================================
MessageManagerLock::MessageManagerLock (Thread* const threadToCheck)
    : locked (false)
{
    init (threadToCheck, 0);
}

MessageManagerLock::MessageManagerLock (ThreadPoolJob* const jobToCheckForExitSignal)
    : locked (false)
{
    init (0, jobToCheckForExitSignal);
}

void MessageManagerLock::init (Thread* const threadToCheck, ThreadPoolJob* const job)
{
    if (MessageManager::instance != 0)
    {
        if (MessageManager::instance->currentThreadHasLockedMessageManager())
        {
            locked = true;   // either we're on the message thread, or this is a re-entrant call.
        }
        else
        {
            if (threadToCheck == 0 && job == 0)
            {
                MessageManager::instance->lockingLock.enter();
            }
            else
            {
                while (! MessageManager::instance->lockingLock.tryEnter())
                {
                    if ((threadToCheck != 0 && threadToCheck->threadShouldExit())
                          || (job != 0 && job->shouldExit()))
                        return;

                    Thread::sleep (1);
                }
            }

            blockingMessage = new BlockingMessage();
            blockingMessage->post();

            while (! blockingMessage->lockedEvent.wait (20))
            {
                if ((threadToCheck != 0 && threadToCheck->threadShouldExit())
                      || (job != 0 && job->shouldExit()))
                {
                    blockingMessage->releaseEvent.signal();
                    blockingMessage = 0;
                    MessageManager::instance->lockingLock.exit();
                    return;
                }
            }

            jassert (MessageManager::instance->threadWithLock == 0);

            MessageManager::instance->threadWithLock = Thread::getCurrentThreadId();
            locked = true;
        }
    }
}

MessageManagerLock::~MessageManagerLock() throw()
{
    if (blockingMessage != 0)
    {
        jassert (MessageManager::instance == 0 || MessageManager::instance->currentThreadHasLockedMessageManager());

        blockingMessage->releaseEvent.signal();
        blockingMessage = 0;

        if (MessageManager::instance != 0)
        {
            MessageManager::instance->threadWithLock = 0;
            MessageManager::instance->lockingLock.exit();
        }
    }
}


END_JUCE_NAMESPACE
