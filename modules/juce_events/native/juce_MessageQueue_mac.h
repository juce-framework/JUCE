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

//==============================================================================
/* An internal message pump class used in OSX and iOS. */
class MessageQueue
{
public:
    MessageQueue()
    {
       #if JUCE_IOS
        runLoop = CFRunLoopGetCurrent();
       #else
        runLoop = CFRunLoopGetMain();
       #endif

        CFRunLoopSourceContext sourceContext;
        zerostruct (sourceContext); // (can't use "= { 0 }" on this object because it's typedef'ed as a C struct)
        sourceContext.info = this;
        sourceContext.perform = runLoopSourceCallback;
        runLoopSource.reset (CFRunLoopSourceCreate (kCFAllocatorDefault, 1, &sourceContext));
        CFRunLoopAddSource (runLoop, runLoopSource.get(), kCFRunLoopCommonModes);
    }

    ~MessageQueue() noexcept
    {
        CFRunLoopRemoveSource (runLoop, runLoopSource.get(), kCFRunLoopCommonModes);
        CFRunLoopSourceInvalidate (runLoopSource.get());
    }

    void post (MessageManager::MessageBase* const message)
    {
        messages.add (message);
        wakeUp();
    }

private:
    ReferenceCountedArray<MessageManager::MessageBase, CriticalSection> messages;
    CFRunLoopRef runLoop;
    CFUniquePtr<CFRunLoopSourceRef> runLoopSource;

    void wakeUp() noexcept
    {
        CFRunLoopSourceSignal (runLoopSource.get());
        CFRunLoopWakeUp (runLoop);
    }

    bool deliverNextMessage()
    {
        const MessageManager::MessageBase::Ptr nextMessage (messages.removeAndReturn (0));

        if (nextMessage == nullptr)
            return false;

        JUCE_AUTORELEASEPOOL
        {
            JUCE_TRY
            {
                nextMessage->messageCallback();
            }
            JUCE_CATCH_EXCEPTION
        }

        return true;
    }

    void runLoopCallback() noexcept
    {
        for (int i = 4; --i >= 0;)
            if (! deliverNextMessage())
                return;

        wakeUp();
    }

    static void runLoopSourceCallback (void* info) noexcept
    {
        static_cast<MessageQueue*> (info)->runLoopCallback();
    }
};

} // namespace juce
