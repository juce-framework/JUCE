/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

MessageManager::MessageManager() noexcept
  : messageThreadId (Thread::getCurrentThreadId())
{
    if (JUCEApplicationBase::isStandaloneApp())
        Thread::setCurrentThreadName ("Juce Message Thread");
}

MessageManager::~MessageManager() noexcept
{
    broadcaster = nullptr;

    doPlatformSpecificShutdown();

    jassert (instance == this);
    instance = nullptr;  // do this last in case this instance is still needed by doPlatformSpecificShutdown()
}

MessageManager* MessageManager::instance = nullptr;

MessageManager* MessageManager::getInstance()
{
    if (instance == nullptr)
    {
        instance = new MessageManager();
        doPlatformSpecificInitialisation();
    }

    return instance;
}

MessageManager* MessageManager::getInstanceWithoutCreating() noexcept
{
    return instance;
}

void MessageManager::deleteInstance()
{
    deleteAndZero (instance);
}

//==============================================================================
bool MessageManager::MessageBase::post()
{
    auto* mm = MessageManager::instance;

    if (mm == nullptr || mm->quitMessagePosted || ! postMessageToSystemQueue (this))
    {
        Ptr deleter (this); // (this will delete messages that were just created with a 0 ref count)
        return false;
    }

    return true;
}

//==============================================================================
#if JUCE_MODAL_LOOPS_PERMITTED && ! (JUCE_MAC || JUCE_IOS)
bool MessageManager::runDispatchLoopUntil (int millisecondsToRunFor)
{
    jassert (isThisTheMessageThread()); // must only be called by the message thread

    const int64 endTime = Time::currentTimeMillis() + millisecondsToRunFor;

    while (! quitMessageReceived)
    {
        JUCE_TRY
        {
            if (! dispatchNextMessageOnSystemQueue (millisecondsToRunFor >= 0))
                Thread::sleep (1);
        }
        JUCE_CATCH_EXCEPTION

        if (millisecondsToRunFor >= 0 && Time::currentTimeMillis() >= endTime)
            break;
    }

    return ! quitMessageReceived;
}
#endif

#if ! (JUCE_MAC || JUCE_IOS || JUCE_ANDROID)
class MessageManager::QuitMessage   : public MessageManager::MessageBase
{
public:
    QuitMessage() {}

    void messageCallback() override
    {
        if (auto* mm = MessageManager::instance)
            mm->quitMessageReceived = true;
    }

    JUCE_DECLARE_NON_COPYABLE (QuitMessage)
};

void MessageManager::runDispatchLoop()
{
    jassert (isThisTheMessageThread()); // must only be called by the message thread

    while (! quitMessageReceived)
    {
        JUCE_TRY
        {
            if (! dispatchNextMessageOnSystemQueue (false))
                Thread::sleep (1);
        }
        JUCE_CATCH_EXCEPTION
    }
}

void MessageManager::stopDispatchLoop()
{
    (new QuitMessage())->post();
    quitMessagePosted = true;
}

#endif

//==============================================================================
class AsyncFunctionCallback   : public MessageManager::MessageBase
{
public:
    AsyncFunctionCallback (MessageCallbackFunction* const f, void* const param)
        : func (f), parameter (param)
    {}

    void messageCallback() override
    {
        result = (*func) (parameter);
        finished.signal();
    }

    WaitableEvent finished;
    void* volatile result = nullptr;

private:
    MessageCallbackFunction* const func;
    void* const parameter;

    JUCE_DECLARE_NON_COPYABLE (AsyncFunctionCallback)
};

void* MessageManager::callFunctionOnMessageThread (MessageCallbackFunction* const func, void* const parameter)
{
    if (isThisTheMessageThread())
        return func (parameter);

    // If this thread has the message manager locked, then this will deadlock!
    jassert (! currentThreadHasLockedMessageManager());

    const ReferenceCountedObjectPtr<AsyncFunctionCallback> message (new AsyncFunctionCallback (func, parameter));

    if (message->post())
    {
        message->finished.wait();
        return message->result;
    }

    jassertfalse; // the OS message queue failed to send the message!
    return nullptr;
}

//==============================================================================
void MessageManager::deliverBroadcastMessage (const String& value)
{
    if (broadcaster != nullptr)
        broadcaster->sendActionMessage (value);
}

void MessageManager::registerBroadcastListener (ActionListener* const listener)
{
    if (broadcaster == nullptr)
        broadcaster = new ActionBroadcaster();

    broadcaster->addActionListener (listener);
}

void MessageManager::deregisterBroadcastListener (ActionListener* const listener)
{
    if (broadcaster != nullptr)
        broadcaster->removeActionListener (listener);
}

//==============================================================================
bool MessageManager::isThisTheMessageThread() const noexcept
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

bool MessageManager::currentThreadHasLockedMessageManager() const noexcept
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
class MessageManagerLock::BlockingMessage   : public MessageManager::MessageBase
{
public:
    BlockingMessage() noexcept {}

    void messageCallback() override
    {
        lockedEvent.signal();
        releaseEvent.wait();
    }

    WaitableEvent lockedEvent, releaseEvent;

    JUCE_DECLARE_NON_COPYABLE (BlockingMessage)
};

//==============================================================================
MessageManagerLock::MessageManagerLock (Thread* const threadToCheck)
    : blockingMessage(), checker (threadToCheck, nullptr),
      locked (attemptLock (threadToCheck != nullptr ? &checker : nullptr))
{
}

MessageManagerLock::MessageManagerLock (ThreadPoolJob* const jobToCheckForExitSignal)
    : blockingMessage(), checker (nullptr, jobToCheckForExitSignal),
      locked (attemptLock (jobToCheckForExitSignal != nullptr ? &checker : nullptr))
{
}

MessageManagerLock::MessageManagerLock (BailOutChecker& bailOutChecker)
    : blockingMessage(), checker (nullptr, nullptr),
      locked (attemptLock (&bailOutChecker))
{
}

bool MessageManagerLock::attemptLock (BailOutChecker* bailOutChecker)
{
    auto* mm = MessageManager::instance;

    if (mm == nullptr)
        return false;

    if (mm->currentThreadHasLockedMessageManager())
        return true;

    if (bailOutChecker == nullptr)
    {
        mm->lockingLock.enter();
    }
    else
    {
        while (! mm->lockingLock.tryEnter())
        {
            if (bailOutChecker->shouldAbortAcquiringLock())
                return false;

            Thread::yield();
        }
    }

    blockingMessage = new BlockingMessage();

    if (! blockingMessage->post())
    {
        blockingMessage = nullptr;
        return false;
    }

    while (! blockingMessage->lockedEvent.wait (20))
    {
        if (bailOutChecker != nullptr && bailOutChecker->shouldAbortAcquiringLock())
        {
            blockingMessage->releaseEvent.signal();
            blockingMessage = nullptr;
            mm->lockingLock.exit();
            return false;
        }
    }

    jassert (mm->threadWithLock == 0);

    mm->threadWithLock = Thread::getCurrentThreadId();
    return true;
}

MessageManagerLock::~MessageManagerLock() noexcept
{
    if (blockingMessage != nullptr)
    {
        auto* mm = MessageManager::instance;

        jassert (mm == nullptr || mm->currentThreadHasLockedMessageManager());

        blockingMessage->releaseEvent.signal();
        blockingMessage = nullptr;

        if (mm != nullptr)
        {
            mm->threadWithLock = 0;
            mm->lockingLock.exit();
        }
    }
}

//==============================================================================
MessageManagerLock::ThreadChecker::ThreadChecker (Thread* const threadToUse,
                                                  ThreadPoolJob* const threadJobToUse)
    : threadToCheck (threadToUse), job (threadJobToUse)
{
}

bool MessageManagerLock::ThreadChecker::shouldAbortAcquiringLock()
{
    return (threadToCheck != nullptr && threadToCheck->threadShouldExit())
        || (job           != nullptr && job->shouldExit());
}

//==============================================================================
JUCE_API void JUCE_CALLTYPE initialiseJuce_GUI();
JUCE_API void JUCE_CALLTYPE initialiseJuce_GUI()
{
    JUCE_AUTORELEASEPOOL
    {
        MessageManager::getInstance();
    }
}

JUCE_API void JUCE_CALLTYPE shutdownJuce_GUI();
JUCE_API void JUCE_CALLTYPE shutdownJuce_GUI()
{
    JUCE_AUTORELEASEPOOL
    {
        DeletedAtShutdown::deleteAll();
        MessageManager::deleteInstance();
    }
}

static int numScopedInitInstances = 0;

ScopedJuceInitialiser_GUI::ScopedJuceInitialiser_GUI()  { if (numScopedInitInstances++ == 0) initialiseJuce_GUI(); }
ScopedJuceInitialiser_GUI::~ScopedJuceInitialiser_GUI() { if (--numScopedInitInstances == 0) shutdownJuce_GUI(); }

} // namespace juce
