/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

MessageManager::MessageManager() noexcept
  : messageThreadId (Thread::getCurrentThreadId())
{
    JUCE_VERSION_ID

    if (JUCEApplicationBase::isStandaloneApp())
        Thread::setCurrentThreadName (SystemStats::getJUCEVersion() + ": Message Thread");
}

MessageManager::~MessageManager() noexcept
{
    broadcaster.reset();

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

    if (mm == nullptr || mm->quitMessagePosted.get() != 0 || ! postMessageToSystemQueue (this))
    {
        Ptr deleter (this); // (this will delete messages that were just created with a 0 ref count)
        return false;
    }

    return true;
}

//==============================================================================
#if ! (JUCE_MAC || JUCE_IOS || JUCE_ANDROID)
// implemented in platform-specific code (juce_Messaging_linux.cpp and juce_Messaging_windows.cpp)
namespace detail
{
bool dispatchNextMessageOnSystemQueue (bool returnIfNoPendingMessages);
} // namespace detail

class MessageManager::QuitMessage final : public MessageManager::MessageBase
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

    while (quitMessageReceived.get() == 0)
    {
        JUCE_TRY
        {
            if (! detail::dispatchNextMessageOnSystemQueue (false))
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

#if JUCE_MODAL_LOOPS_PERMITTED
bool MessageManager::runDispatchLoopUntil (int millisecondsToRunFor)
{
    jassert (isThisTheMessageThread()); // must only be called by the message thread

    auto endTime = Time::currentTimeMillis() + millisecondsToRunFor;

    while (quitMessageReceived.get() == 0)
    {
        JUCE_TRY
        {
            if (! detail::dispatchNextMessageOnSystemQueue (millisecondsToRunFor >= 0))
                Thread::sleep (1);
        }
        JUCE_CATCH_EXCEPTION

        if (millisecondsToRunFor >= 0 && Time::currentTimeMillis() >= endTime)
            break;
    }

    return quitMessageReceived.get() == 0;
}
#endif

#endif

//==============================================================================
void* MessageManager::callFunctionOnMessageThread (MessageCallbackFunction* func, void* parameter)
{
    return callSync ([func, parameter] { return func (parameter); }).value_or (nullptr);
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
        broadcaster.reset (new ActionBroadcaster());

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
    const std::lock_guard<std::mutex> lock { messageThreadIdMutex };

    return Thread::getCurrentThreadId() == messageThreadId;
}

void MessageManager::setCurrentThreadAsMessageThread()
{
    auto thisThread = Thread::getCurrentThreadId();

    const std::lock_guard<std::mutex> lock { messageThreadIdMutex };

    if (std::exchange (messageThreadId, thisThread) != thisThread)
    {
       #if JUCE_WINDOWS
        // This is needed on windows to make sure the message window is created by this thread
        doPlatformSpecificShutdown();
        doPlatformSpecificInitialisation();
       #endif
    }
}

bool MessageManager::currentThreadHasLockedMessageManager() const noexcept
{
    auto thisThread = Thread::getCurrentThreadId();
    return thisThread == messageThreadId || thisThread == threadWithLock.get();
}

bool MessageManager::existsAndIsLockedByCurrentThread() noexcept
{
    if (auto i = getInstanceWithoutCreating())
        return i->currentThreadHasLockedMessageManager();

    return false;
}

bool MessageManager::existsAndIsCurrentThread() noexcept
{
    if (auto i = getInstanceWithoutCreating())
        return i->isThisTheMessageThread();

    return false;
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
struct MessageManager::Lock::BlockingMessage final : public MessageManager::MessageBase
{
    explicit BlockingMessage (const MessageManager::Lock* parent) noexcept
        : owner (parent) {}

    void messageCallback() override
    {
        std::unique_lock lock { mutex };

        if (owner != nullptr)
            owner->setAcquired (true);

        condvar.wait (lock, [&] { return owner == nullptr; });
    }

    void stopWaiting()
    {
        const ScopeGuard scope { [&] { condvar.notify_one(); } };
        const std::scoped_lock lock { mutex };
        owner = nullptr;
    }

private:
    std::mutex mutex;
    std::condition_variable condvar;

    const MessageManager::Lock* owner = nullptr;

    JUCE_DECLARE_NON_COPYABLE (BlockingMessage)
};

//==============================================================================
MessageManager::Lock::Lock()                            {}
MessageManager::Lock::~Lock()                           { exit(); }
void MessageManager::Lock::enter()    const noexcept    {        exclusiveTryAcquire (true); }
bool MessageManager::Lock::tryEnter() const noexcept    { return exclusiveTryAcquire (false); }

bool MessageManager::Lock::exclusiveTryAcquire (bool lockIsMandatory) const noexcept
{
    if (lockIsMandatory)
        entryMutex.enter();
    else if (! entryMutex.tryEnter())
        return false;

    const auto result = tryAcquire (lockIsMandatory);

    if (! result)
        entryMutex.exit();

    return result;
}

bool MessageManager::Lock::tryAcquire (bool lockIsMandatory) const noexcept
{
    auto* mm = MessageManager::instance;

    if (mm == nullptr)
    {
        jassertfalse;
        return false;
    }

    if (! lockIsMandatory && [&]
                             {
                                 const std::scoped_lock lock { mutex };
                                 return std::exchange (abortWait, false);
                             }())
    {
        return false;
    }

    if (mm->currentThreadHasLockedMessageManager())
        return true;

    try
    {
        blockingMessage = *new BlockingMessage (this);
    }
    catch (...)
    {
        jassert (! lockIsMandatory);
        return false;
    }

    if (! blockingMessage->post())
    {
        // post of message failed while trying to get the lock
        jassert (! lockIsMandatory);
        blockingMessage = nullptr;
        return false;
    }

    for (;;)
    {
        {
            std::unique_lock lock { mutex };
            condvar.wait (lock, [&] { return std::exchange (abortWait, false); });
        }

        if (acquired)
        {
            mm->threadWithLock = Thread::getCurrentThreadId();
            return true;
        }

        if (! lockIsMandatory)
            break;
    }

    // we didn't get the lock

    blockingMessage->stopWaiting();
    blockingMessage = nullptr;
    return false;
}

void MessageManager::Lock::exit() const noexcept
{
    const auto wasAcquired = [&]
    {
        const std::scoped_lock lock { mutex };
        return acquired;
    }();

    if (! wasAcquired)
        return;

    const ScopeGuard unlocker { [&] { entryMutex.exit(); } };

    if (blockingMessage == nullptr)
        return;

    if (auto* mm = MessageManager::instance)
    {
        jassert (mm->currentThreadHasLockedMessageManager());
        mm->threadWithLock = {};
    }

    blockingMessage->stopWaiting();
    blockingMessage = nullptr;
    acquired = false;
}

void MessageManager::Lock::abort() const noexcept
{
    setAcquired (false);
}

void MessageManager::Lock::setAcquired (bool x) const noexcept
{
    const ScopeGuard scope { [&] { condvar.notify_one(); } };
    const std::scoped_lock lock { mutex };
    abortWait = true;
    acquired = x;
}

//==============================================================================
MessageManagerLock::MessageManagerLock (Thread* threadToCheck)
    : locked (attemptLock (threadToCheck, nullptr))
{}

MessageManagerLock::MessageManagerLock (ThreadPoolJob* jobToCheck)
    : locked (attemptLock (nullptr, jobToCheck))
{}

bool MessageManagerLock::attemptLock (Thread* threadToCheck, ThreadPoolJob* jobToCheck)
{
    jassert (threadToCheck == nullptr || jobToCheck == nullptr);

    if (threadToCheck != nullptr)
        threadToCheck->addListener (this);

    if (jobToCheck != nullptr)
        jobToCheck->addListener (this);

    // tryEnter may have a spurious abort (return false) so keep checking the condition
    while ((threadToCheck == nullptr || ! threadToCheck->threadShouldExit())
             && (jobToCheck == nullptr || ! jobToCheck->shouldExit()))
    {
        if (mmLock.tryEnter())
            break;
    }

    if (threadToCheck != nullptr)
    {
        threadToCheck->removeListener (this);

        if (threadToCheck->threadShouldExit())
            return false;
    }

    if (jobToCheck != nullptr)
    {
        jobToCheck->removeListener (this);

        if (jobToCheck->shouldExit())
            return false;
    }

    return true;
}

MessageManagerLock::~MessageManagerLock()  { mmLock.exit(); }

void MessageManagerLock::exitSignalSent()
{
    mmLock.abort();
}

//==============================================================================
JUCE_API void JUCE_CALLTYPE initialiseJuce_GUI()
{
    JUCE_AUTORELEASEPOOL
    {
        MessageManager::getInstance();
    }
}

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
