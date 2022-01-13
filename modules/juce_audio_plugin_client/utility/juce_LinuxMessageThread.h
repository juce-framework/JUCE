/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

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

} // namespace juce

#endif
