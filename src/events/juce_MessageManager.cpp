/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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
#include "juce_ActionListenerList.h"
#include "../application/juce_Application.h"
#include "../gui/components/juce_Component.h"
#include "../threads/juce_Thread.h"
#include "../threads/juce_ScopedLock.h"
#include "../core/juce_Time.h"


//==============================================================================
// platform-specific functions..
bool juce_dispatchNextMessageOnSystemQueue (bool returnIfNoPendingMessages);
bool juce_postMessageToSystemQueue (void* message);

//==============================================================================
MessageManager* MessageManager::instance = 0;

static const int quitMessageId = 0xfffff321;

MessageManager::MessageManager() throw()
  : quitMessagePosted (false),
    quitMessageReceived (false),
    threadWithLock (0)
{
    messageThreadId = Thread::getCurrentThreadId();
}

MessageManager::~MessageManager() throw()
{
    broadcastListeners = 0;

    doPlatformSpecificShutdown();

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
        delete message;
}

//==============================================================================
CallbackMessage::CallbackMessage() throw()  {}
CallbackMessage::~CallbackMessage() throw()  {}

void CallbackMessage::post()
{
    if (MessageManager::instance != 0)
        MessageManager::instance->postCallbackMessage (this);
}

void MessageManager::postCallbackMessage (Message* const message)
{
    message->messageRecipient = 0;
    postMessageToQueue (message);
}

//==============================================================================
// not for public use..
void MessageManager::deliverMessage (void* message)
{
    const ScopedPointer <Message> m ((Message*) message);
    MessageListener* const recipient = m->messageRecipient;

    JUCE_TRY
    {
        if (messageListeners.contains (recipient))
        {
            recipient->handleMessage (*m);
        }
        else if (recipient == 0)
        {
            if (m->intParameter1 == quitMessageId)
            {
                quitMessageReceived = true;
            }
            else
            {
                CallbackMessage* const cm = dynamic_cast <CallbackMessage*> ((Message*) m);

                if (cm != 0)
                    cm->messageCallback();
            }
        }
    }
    JUCE_CATCH_EXCEPTION
}

//==============================================================================
#if ! (JUCE_MAC || JUCE_IPHONE)
void MessageManager::runDispatchLoop()
{
    jassert (isThisTheMessageThread()); // must only be called by the message thread

    runDispatchLoopUntil (-1);
}

void MessageManager::stopDispatchLoop()
{
    Message* const m = new Message (quitMessageId, 0, 0, 0);
    m->messageRecipient = 0;
    postMessageToQueue (m);

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
bool MessageManager::isThisTheMessageThread() const throw()
{
    return Thread::getCurrentThreadId() == messageThreadId;
}

void MessageManager::setCurrentMessageThread (const Thread::ThreadID threadId) throw()
{
    messageThreadId = threadId;
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
class SharedLockingEvents   : public ReferenceCountedObject
{
public:
    SharedLockingEvents() throw()   {}
    ~SharedLockingEvents()          {}

    /* This class just holds a couple of events to communicate between the MMLockMessage
       and the MessageManagerLock. Because both of these objects may be deleted at any time,
       this shared data must be kept in a separate, ref-counted container. */
    WaitableEvent lockedEvent, releaseEvent;
};

class MMLockMessage   : public CallbackMessage
{
public:
    MMLockMessage (SharedLockingEvents* const events_) throw()
        : events (events_)
    {}

    ~MMLockMessage() throw() {}

    ReferenceCountedObjectPtr <SharedLockingEvents> events;

    void messageCallback()
    {
        events->lockedEvent.signal();
        events->releaseEvent.wait();
    }

    juce_UseDebuggingNewOperator

    MMLockMessage (const MMLockMessage&);
    const MMLockMessage& operator= (const MMLockMessage&);
};

//==============================================================================
MessageManagerLock::MessageManagerLock (Thread* const threadToCheck) throw()
    : locked (false),
      needsUnlocking (false)
{
    init (threadToCheck, 0);
}

MessageManagerLock::MessageManagerLock (ThreadPoolJob* const jobToCheckForExitSignal) throw()
    : locked (false),
      needsUnlocking (false)
{
    init (0, jobToCheckForExitSignal);
}

void MessageManagerLock::init (Thread* const threadToCheck, ThreadPoolJob* const job) throw()
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

            SharedLockingEvents* const events = new SharedLockingEvents();
            sharedEvents = events;
            events->incReferenceCount();

            (new MMLockMessage (events))->post();

            while (! events->lockedEvent.wait (50))
            {
                if ((threadToCheck != 0 && threadToCheck->threadShouldExit())
                      || (job != 0 && job->shouldExit()))
                {
                    events->releaseEvent.signal();
                    events->decReferenceCount();
                    MessageManager::instance->lockingLock.exit();
                    return;
                }
            }

            jassert (MessageManager::instance->threadWithLock == 0);

            MessageManager::instance->threadWithLock = Thread::getCurrentThreadId();
            locked = true;
            needsUnlocking = true;
        }
    }
}


MessageManagerLock::~MessageManagerLock() throw()
{
    if (needsUnlocking && MessageManager::instance != 0)
    {
        jassert (MessageManager::instance->currentThreadHasLockedMessageManager());

        ((SharedLockingEvents*) sharedEvents)->releaseEvent.signal();
        ((SharedLockingEvents*) sharedEvents)->decReferenceCount();
        MessageManager::instance->threadWithLock = 0;
        MessageManager::instance->lockingLock.exit();
    }
}


END_JUCE_NAMESPACE
