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

#if JUCE_LINUX || JUCE_BSD

namespace juce::detail
{

// Implemented in juce_Messaging_linux.cpp
bool dispatchNextMessageOnSystemQueue (bool returnIfNoPendingMessages);

class MessageThread : public Thread
{
public:
    MessageThread() : Thread (SystemStats::getJUCEVersion() + ": Plugin Message Thread")
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
