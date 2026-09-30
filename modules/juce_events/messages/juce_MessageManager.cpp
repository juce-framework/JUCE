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

   JUCE End User Licence Agreement: https://juce.com/legal/juce-9-licence/
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
    // Refuse new posts without calling stopDispatchLoop()
    quitMessagePosted = true;

    JUCE_TRY
    {
        notifyLifetimeStopping();
    }
    JUCE_CATCH_EXCEPTION

    DeletedAtShutdown::deleteAll();

    broadcaster.reset();

    doPlatformSpecificShutdown();

    jassert (instance == this);

    // do this last in case this instance is still needed by any shutdown code
    instance = nullptr;
}

MessageManager* MessageManager::instance = nullptr;

MessageManager* MessageManager::getInstance()
{
    if (instance == nullptr)
    {
        instance = new MessageManager();
        doPlatformSpecificInitialisation();
        notifyLifetimeStarting();
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

namespace
{
    // Namespace-scope so this exists before any LifetimeListener is
    // constructed and remains until after they are destroyed. Do not make
    // this a function-local static because it would be destroyed before some
    // listeners, and calling getLifetimeListeners() from their destructors
    // would be undefined behaviour.
    bool lifetimeListenersDeleted = false;
}

MessageManager::LifetimeListenerList* MessageManager::getLifetimeListeners()
{
    // Check before touching the function-local static: after Holder is
    // destroyed, accessing it is undefined behaviour.
    if (lifetimeListenersDeleted)
        return nullptr;

    struct Holder
    {
        ~Holder() { lifetimeListenersDeleted = true; }
        LifetimeListenerList list;
    };

    static Holder holder;
    return &holder.list;
}

void MessageManager::notifyLifetimeStarting()
{
    if (auto* list = getLifetimeListeners())
        list->call (&LifetimeListener::messageManagerStarting);
}

void MessageManager::notifyLifetimeStopping()
{
    if (auto* list = getLifetimeListeners())
        list->call (&LifetimeListener::messageManagerStopping);
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

    while (! quitMessageReceived)
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

    while (! quitMessageReceived)
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

    return ! quitMessageReceived;
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
    return Thread::getCurrentThreadId() == messageThreadId.load (std::memory_order_relaxed);
}

void MessageManager::setCurrentThreadAsMessageThread()
{
    const auto thisThread = Thread::getCurrentThreadId();

    if (messageThreadId.exchange (thisThread, std::memory_order_release) == thisThread)
        return;

    // If another thread has locked the message manager it means the old thread
    // is blocked and therefore still pumping messages from the queue. Make sure
    // the old thread has completed before assigning a new thread!
    jassert (threadWithLock == Thread::ThreadID{});

   #if JUCE_WINDOWS
    doPlatformSpecificShutdown();
    doPlatformSpecificInitialisation();
   #endif
}

bool MessageManager::currentThreadHasLockedMessageManager() const noexcept
{
    const auto thisThread = Thread::getCurrentThreadId();

    return thisThread == messageThreadId.load (std::memory_order_relaxed)
        || thisThread == threadWithLock.load (std::memory_order_relaxed);
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
/*  The only safe way to lock the message thread while another thread does some
    work is by posting a special message, whose purpose is to tie up the event
    loop until the other thread has finished its business.

    Any other approach can get horribly deadlocked if the OS uses its own hidden
    locks which get locked before making an event callback, because if the same
    OS lock gets indirectly accessed from another thread inside a MM lock,
    you're screwed. (this is exactly what happens in Cocoa).
*/
class MessageManager::Lock::LockingMessage final : public MessageManager::MessageBase
{
public:
    enum class State { pending, locked, aborted, unlocked };

    static ReferenceCountedObjectPtr<LockingMessage> create (bool isAbortable) noexcept
    {
        try
        {
            return *new LockingMessage (isAbortable);
        }
        catch (...)
        {
            jassertfalse;
            return {};
        }
    }

    void messageCallback() final
    {
        std::unique_lock lock { mutex };

        if (state == State::aborted)
            return;

        state = State::locked;
        condition.notify_all();
        condition.wait (lock, [&] { return state != State::locked; });
    }

    State waitUntilLockedOrAborted()
    {
        std::unique_lock lock { mutex };
        condition.wait (lock, [&]
        {
            return state == State::locked
                || state == State::aborted;
        });
        return state;
    }

    void abortIfPending()
    {
        if (! abortable)
            return;

        const std::scoped_lock lock { mutex };

        if (state != State::pending)
            return;

        state = State::aborted;
        condition.notify_all();
    }

    void unlock()
    {
        const std::scoped_lock lock { mutex };
        state = State::unlocked;
        condition.notify_all();
    }

private:
    explicit LockingMessage (bool isAbortableIn)
        : abortable (isAbortableIn)
    {}

    const bool abortable;
    State state = State::pending;
    std::mutex mutex;
    std::condition_variable condition;

    JUCE_DECLARE_NON_COPYABLE (LockingMessage)
};

//==============================================================================
MessageManager::Lock::Lock() = default;

MessageManager::Lock::~Lock()
{
    while (depth > 0)
        exit();
}

void MessageManager::Lock::enter() const noexcept
{
    while (! attemptLock (false))
        Thread::sleep (1);
}

bool MessageManager::Lock::tryEnter() const noexcept
{
    return attemptLock (true);
}

bool MessageManager::Lock::attemptLock (bool canAbort) const noexcept
{
    std::unique_lock lock { mutex };

    if (canAbort && std::exchange (shouldAbort, false))
        return false;

    auto* mm = MessageManager::instance;
    jassert (mm != nullptr);

    if (mm->currentThreadHasLockedMessageManager())
    {
        ++depth;
        ++mm->lockCount;
        return true;
    }

    return attemptLockWithMessage (std::move (lock), *mm, canAbort);
}

bool MessageManager::Lock::attemptLockWithMessage (std::unique_lock<std::mutex> lock,
                                                   MessageManager& mm,
                                                   bool canAbort) const noexcept
{
    auto message = LockingMessage::create (canAbort);

    if (message == nullptr)
        return false;

    messages.push_back (message);

    lock.unlock();

    const auto isLocked = message->post()
                       && message->waitUntilLockedOrAborted() == LockingMessage::State::locked;

    lock.lock();

    messages.erase (std::remove (messages.begin(), messages.end(), message));

    if (! isLocked)
    {
        if (canAbort)
            shouldAbort = false;

        return false;
    }

    jassert (depth == 0
             && mm.lockCount == 0
             && mm.lockingMessage == nullptr
             && mm.threadWithLock == Thread::ThreadID{});

    depth = 1;
    mm.lockCount = 1;
    mm.lockingMessage = message;
    mm.threadWithLock = Thread::getCurrentThreadId();
    return true;
}

void MessageManager::Lock::exit() const noexcept
{
    MessageBase::Ptr messageToUnlock;

    {
        const std::scoped_lock lock { mutex };

        if (depth <= 0)
            return;

        --depth;

        auto* mm = MessageManager::instance;
        jassert (mm != nullptr && mm->lockCount > 0);

        if (--mm->lockCount == 0)
        {
            jassert (mm->currentThreadHasLockedMessageManager());
            mm->threadWithLock = {};
            messageToUnlock = std::exchange (mm->lockingMessage, nullptr);
        }
    }

    if (messageToUnlock != nullptr)
        static_cast<LockingMessage*> (messageToUnlock.get())->unlock();
}

void MessageManager::Lock::abort() const noexcept
{
    const std::scoped_lock lock { mutex };

    shouldAbort = true;

    for (auto& message : messages)
        message->abortIfPending();
}

//==============================================================================
MessageManagerLock::MessageManagerLock (Thread* threadToCheck)
    : locked (attemptLock (threadToCheck))
{}

MessageManagerLock::MessageManagerLock (ThreadPoolJob* jobToCheck)
    : locked (attemptLock (jobToCheck))
{}

template <typename ThreadOrThreadPoolJob>
bool MessageManagerLock::attemptLock (ThreadOrThreadPoolJob* threadOrJobToCheck)
{
    const auto shouldExit = [&]
    {
        if (threadOrJobToCheck == nullptr)
            return false;

        if constexpr (std::is_same_v<ThreadOrThreadPoolJob, Thread>)
            return threadOrJobToCheck->threadShouldExit();
        else
            return threadOrJobToCheck->shouldExit();
    };

    if (threadOrJobToCheck != nullptr)
        threadOrJobToCheck->addListener (this);

    const ScopeGuard removeListener { [&]
    {
        if (threadOrJobToCheck != nullptr)
            threadOrJobToCheck->removeListener (this);
    } };

    while (! shouldExit())
    {
        if (mmLock.tryEnter())
            return true;
    }

    return false;
}

MessageManagerLock::~MessageManagerLock()
{
    mmLock.exit();
}

void MessageManagerLock::exitSignalSent()
{
    mmLock.abort();
}

//==============================================================================
// It's important that this is not marked constexpr, and that its definition
// lives in the source file. This ensures that derived classes cannot have a
// constexpr constructor, which in turn means they will be dynamically
// initialised. Since lifetimeListenersDeleted is statically initialised and a
// listener is dynamically initialised, the order in which they are created and
// destroyed is guaranteed even between different translation units.
MessageManager::LifetimeListener::LifetimeListener() = default;

#if JUCE_ASSERTIONS_ENABLED_OR_LOGGED
MessageManager::LifetimeListener::~LifetimeListener()
{
    // removeLifetimeListener() must be called before or during the
    // destructor of the derived class!
    if (auto* list = getLifetimeListeners())
        jassert (! list->contains (this));
}
#else
MessageManager::LifetimeListener::~LifetimeListener() = default;
#endif

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
        MessageManager::deleteInstance();
    }
}

static int numScopedInitInstances = 0;

ScopedJuceInitialiser_GUI::ScopedJuceInitialiser_GUI()  { if (numScopedInitInstances++ == 0) initialiseJuce_GUI(); }
ScopedJuceInitialiser_GUI::~ScopedJuceInitialiser_GUI() { if (--numScopedInitInstances == 0) shutdownJuce_GUI(); }

} // namespace juce
