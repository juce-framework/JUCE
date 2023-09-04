/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

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

//==============================================================================
class InternalMessageQueue
{
public:
    InternalMessageQueue()
    {
        auto err = ::socketpair (AF_LOCAL, SOCK_STREAM, 0, msgpipe);
        jassertquiet (err == 0);

        LinuxEventLoop::registerFdCallback (getReadHandle(),
                                            [this] (int fd)
                                            {
                                                while (auto msg = popNextMessage (fd))
                                                {
                                                    JUCE_TRY
                                                    {
                                                        msg->messageCallback();
                                                    }
                                                    JUCE_CATCH_EXCEPTION
                                                }
                                            });
    }

    ~InternalMessageQueue()
    {
        LinuxEventLoop::unregisterFdCallback (getReadHandle());

        close (getReadHandle());
        close (getWriteHandle());

        clearSingletonInstance();
    }

    //==============================================================================
    void postMessage (MessageManager::MessageBase* const msg) noexcept
    {
        ScopedLock sl (lock);
        queue.add (msg);

        if (bytesInSocket < maxBytesInSocketQueue)
        {
            bytesInSocket++;

            ScopedUnlock ul (lock);
            unsigned char x = 0xff;
            [[maybe_unused]] auto numBytes = write (getWriteHandle(), &x, 1);
        }
    }

    //==============================================================================
    JUCE_DECLARE_SINGLETON (InternalMessageQueue, false)

private:
    CriticalSection lock;
    ReferenceCountedArray <MessageManager::MessageBase> queue;

    int msgpipe[2];
    int bytesInSocket = 0;
    static constexpr int maxBytesInSocketQueue = 128;

    int getWriteHandle() const noexcept  { return msgpipe[0]; }
    int getReadHandle() const noexcept   { return msgpipe[1]; }

    MessageManager::MessageBase::Ptr popNextMessage (int fd) noexcept
    {
        const ScopedLock sl (lock);

        if (bytesInSocket > 0)
        {
            --bytesInSocket;

            ScopedUnlock ul (lock);
            unsigned char x;
            [[maybe_unused]] auto numBytes = read (fd, &x, 1);
        }

        return queue.removeAndReturn (0);
    }
};

JUCE_IMPLEMENT_SINGLETON (InternalMessageQueue)

//==============================================================================
/*
    Stores callbacks associated with file descriptors (FD).

    The callback for a particular FD should be called whenever that file has data to read.

    For standalone apps, the main thread will call poll to wait for new data on any FD, and then
    call the associated callbacks for any FDs that changed.

    For plugins, the host (generally) provides some kind of run loop mechanism instead.
    - In VST2 plugins, the host should call effEditIdle at regular intervals, and plugins can
      dispatch all pending events inside this callback. The host doesn't know about any of the
      plugin's FDs, so it's possible there will be a bit of latency between an FD becoming ready,
      and its associated callback being called.
    - In VST3 plugins, it's possible to register each FD individually with the host. In this case,
      the facilities in LinuxEventLoopInternal can be used to observe added/removed FD callbacks,
      and the host can be notified whenever the set of FDs changes. The host will call onFDIsSet
      whenever a particular FD has data ready. This call should be forwarded through to
      InternalRunLoop::dispatchEvent.
*/
struct InternalRunLoop
{
public:
    InternalRunLoop() = default;

    void registerFdCallback (int fd, std::function<void()>&& cb, short eventMask)
    {
        {
            const ScopedLock sl (lock);

            callbacks.emplace (fd, std::make_shared<std::function<void()>> (std::move (cb)));

            const auto iter = getPollfd (fd);

            if (iter == pfds.end() || iter->fd != fd)
                pfds.insert (iter, { fd, eventMask, 0 });
            else
                jassertfalse;

            jassert (pfdsAreSorted());
        }

        listeners.call ([] (auto& l) { l.fdCallbacksChanged(); });
    }

    void unregisterFdCallback (int fd)
    {
        {
            const ScopedLock sl (lock);

            callbacks.erase (fd);

            const auto iter = getPollfd (fd);

            if (iter != pfds.end() && iter->fd == fd)
                pfds.erase (iter);
            else
                jassertfalse;

            jassert (pfdsAreSorted());
        }

        listeners.call ([] (auto& l) { l.fdCallbacksChanged(); });
    }

    bool dispatchPendingEvents()
    {
        callbackStorage.clear();
        getFunctionsToCallThisTime (callbackStorage);

        // CriticalSection should be available during the callback
        for (auto& fn : callbackStorage)
            (*fn)();

        return ! callbackStorage.empty();
    }

    void dispatchEvent (int fd) const
    {
        const auto fn = [&]
        {
            const ScopedLock sl (lock);
            const auto iter = callbacks.find (fd);
            return iter != callbacks.end() ? iter->second : nullptr;
        }();

        // CriticalSection should be available during the callback
        if (auto* callback = fn.get())
            (*callback)();
    }

    bool sleepUntilNextEvent (int timeoutMs)
    {
        const ScopedLock sl (lock);
        return poll (pfds.data(), static_cast<nfds_t> (pfds.size()), timeoutMs) != 0;
    }

    std::vector<int> getRegisteredFds()
    {
        const ScopedLock sl (lock);
        std::vector<int> result;
        result.reserve (callbacks.size());
        std::transform (callbacks.begin(),
                        callbacks.end(),
                        std::back_inserter (result),
                        [] (const auto& pair) { return pair.first; });
        return result;
    }

    void addListener    (LinuxEventLoopInternal::Listener& listener)         { listeners.add    (&listener); }
    void removeListener (LinuxEventLoopInternal::Listener& listener)         { listeners.remove (&listener); }

    //==============================================================================
    JUCE_DECLARE_SINGLETON (InternalRunLoop, false)

private:
    using SharedCallback = std::shared_ptr<std::function<void()>>;

    /*  Appends any functions that need to be called to the passed-in vector.

        We take a copy of each shared function so that the functions can be called without
        locking or racing in the event that the function attempts to register/deregister a
        new FD callback.
    */
    void getFunctionsToCallThisTime (std::vector<SharedCallback>& functions)
    {
        const ScopedLock sl (lock);

        if (! sleepUntilNextEvent (0))
            return;

        for (auto& pfd : pfds)
        {
            if (std::exchange (pfd.revents, 0) != 0)
            {
                const auto iter = callbacks.find (pfd.fd);

                if (iter != callbacks.end())
                    functions.emplace_back (iter->second);
            }
        }
    }

    std::vector<pollfd>::iterator getPollfd (int fd)
    {
        return std::lower_bound (pfds.begin(), pfds.end(), fd, [] (auto descriptor, auto toFind)
        {
            return descriptor.fd < toFind;
        });
    }

    bool pfdsAreSorted() const
    {
        return std::is_sorted (pfds.begin(), pfds.end(), [] (auto a, auto b) { return a.fd < b.fd; });
    }

    CriticalSection lock;

    std::map<int, SharedCallback> callbacks;
    std::vector<SharedCallback> callbackStorage;
    std::vector<pollfd> pfds;

    ListenerList<LinuxEventLoopInternal::Listener> listeners;
};

JUCE_IMPLEMENT_SINGLETON (InternalRunLoop)

//==============================================================================
namespace LinuxErrorHandling
{
    static bool keyboardBreakOccurred = false;

    static void keyboardBreakSignalHandler (int sig)
    {
        if (sig == SIGINT)
            keyboardBreakOccurred = true;
    }

    static void installKeyboardBreakHandler()
    {
        struct sigaction saction;
        sigset_t maskSet;
        sigemptyset (&maskSet);
        saction.sa_handler = keyboardBreakSignalHandler;
        saction.sa_mask = maskSet;
        saction.sa_flags = 0;
        sigaction (SIGINT, &saction, nullptr);
    }
}

//==============================================================================
void MessageManager::doPlatformSpecificInitialisation()
{
    if (JUCEApplicationBase::isStandaloneApp())
        LinuxErrorHandling::installKeyboardBreakHandler();

    InternalRunLoop::getInstance();
    InternalMessageQueue::getInstance();
}

void MessageManager::doPlatformSpecificShutdown()
{
    InternalMessageQueue::deleteInstance();
    InternalRunLoop::deleteInstance();
}

bool MessageManager::postMessageToSystemQueue (MessageManager::MessageBase* const message)
{
    if (auto* queue = InternalMessageQueue::getInstanceWithoutCreating())
    {
        queue->postMessage (message);
        return true;
    }

    return false;
}

void MessageManager::broadcastMessage (const String&)
{
    // TODO
}

namespace detail
{
// this function expects that it will NEVER be called simultaneously for two concurrent threads
bool dispatchNextMessageOnSystemQueue (bool returnIfNoPendingMessages)
{
    for (;;)
    {
        if (LinuxErrorHandling::keyboardBreakOccurred)
            JUCEApplicationBase::quit();

        if (auto* runLoop = InternalRunLoop::getInstanceWithoutCreating())
        {
            if (runLoop->dispatchPendingEvents())
                break;

            if (returnIfNoPendingMessages)
                return false;

            runLoop->sleepUntilNextEvent (2000);
        }
    }

    return true;
}
} // namespace detail

//==============================================================================
void LinuxEventLoop::registerFdCallback (int fd, std::function<void (int)> readCallback, short eventMask)
{
    if (auto* runLoop = InternalRunLoop::getInstanceWithoutCreating())
        runLoop->registerFdCallback (fd, [cb = std::move (readCallback), fd] { cb (fd); }, eventMask);
}

void LinuxEventLoop::unregisterFdCallback (int fd)
{
    if (auto* runLoop = InternalRunLoop::getInstanceWithoutCreating())
        runLoop->unregisterFdCallback (fd);
}

//==============================================================================
void LinuxEventLoopInternal::registerLinuxEventLoopListener (LinuxEventLoopInternal::Listener& listener)
{
    if (auto* runLoop = InternalRunLoop::getInstanceWithoutCreating())
        runLoop->addListener (listener);
}

void LinuxEventLoopInternal::deregisterLinuxEventLoopListener (LinuxEventLoopInternal::Listener& listener)
{
    if (auto* runLoop = InternalRunLoop::getInstanceWithoutCreating())
        runLoop->removeListener (listener);
}

void LinuxEventLoopInternal::invokeEventLoopCallbackForFd (int fd)
{
    if (auto* runLoop = InternalRunLoop::getInstanceWithoutCreating())
        runLoop->dispatchEvent (fd);
}

std::vector<int> LinuxEventLoopInternal::getRegisteredFds()
{
    if (auto* runLoop = InternalRunLoop::getInstanceWithoutCreating())
        return runLoop->getRegisteredFds();

    return {};
}

} // namespace juce
