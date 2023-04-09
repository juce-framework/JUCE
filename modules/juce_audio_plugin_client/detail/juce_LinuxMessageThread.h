/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#if JUCE_LINUX || JUCE_BSD

namespace juce::detail
{

// Implemented in juce_Messaging_linux.cpp
bool dispatchNextMessageOnSystemQueue (bool returnIfNoPendingMessages);

/** @internal */
class MessageThread : public Thread
{
public:
    MessageThread() : Thread ("JUCE Plugin Message Thread")
    {
        start();
    }

    ~MessageThread() override
    {
        MessageManager::getInstance()->stopDispatchLoop();
        stop();
    }

    void start()
    {
        startThread (Priority::high);

        // Wait for setCurrentThreadAsMessageThread() and getInstance to be executed
        // before leaving this method
        threadInitialised.wait (10000);
    }

    void stop()
    {
        signalThreadShouldExit();
        stopThread (-1);
    }

    bool isRunning() const noexcept  { return isThreadRunning(); }

    void run() override
    {
        MessageManager::getInstance()->setCurrentThreadAsMessageThread();
        XWindowSystem::getInstance();

        threadInitialised.signal();

        while (! threadShouldExit())
        {
            if (! dispatchNextMessageOnSystemQueue (true))
                Thread::sleep (1);
        }
    }

private:
    WaitableEvent threadInitialised;
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

} // namespace juce::detail

#endif
