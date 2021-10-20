/*
  ==============================================================================

   This file is part of the JUCE 7 technical preview.
   Copyright (c) 2022 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For the technical preview this file cannot be licensed commercially.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#if JUCE_LINUX || JUCE_BSD

#include <thread>

namespace juce
{

// Implemented in juce_linux_Messaging.cpp
bool dispatchNextMessageOnSystemQueue (bool returnIfNoPendingMessages);

/** @internal */
class MessageThread
{
public:
    MessageThread()
    {
        start();
    }

    ~MessageThread()
    {
        MessageManager::getInstance()->stopDispatchLoop();
        stop();
    }

    void start()
    {
        if (isRunning())
            stop();

        shouldExit = false;

        thread = std::thread { [this]
        {
            Thread::setCurrentThreadPriority (7);
            Thread::setCurrentThreadName ("JUCE Plugin Message Thread");

            MessageManager::getInstance()->setCurrentThreadAsMessageThread();
            XWindowSystem::getInstance();

            threadInitialised.signal();

            for (;;)
            {
                if (! dispatchNextMessageOnSystemQueue (true))
                    Thread::sleep (1);

                if (shouldExit)
                    break;
            }
        } };

        threadInitialised.wait();
    }

    void stop()
    {
        if (! isRunning())
            return;

        shouldExit = true;
        thread.join();
    }

    bool isRunning() const noexcept  { return thread.joinable(); }

private:
    WaitableEvent threadInitialised;
    std::thread thread;

    std::atomic<bool> shouldExit { false };

    JUCE_DECLARE_NON_MOVEABLE (MessageThread)
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MessageThread)
};

//==============================================================================
/** @internal */
class HostDrivenEventLoop
{
public:
    HostDrivenEventLoop()
    {
        messageThread->stop();
        MessageManager::getInstance()->setCurrentThreadAsMessageThread();
    }

    void processPendingEvents()
    {
        MessageManager::getInstance()->setCurrentThreadAsMessageThread();

        for (;;)
            if (! dispatchNextMessageOnSystemQueue (true))
                return;
    }

    ~HostDrivenEventLoop()
    {
        messageThread->start();
    }

private:
    SharedResourcePointer<MessageThread> messageThread;
};

} // namespace juce

#endif
