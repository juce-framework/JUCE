/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

#ifndef JUCE_OSX_MESSAGEQUEUE_H_INCLUDED
#define JUCE_OSX_MESSAGEQUEUE_H_INCLUDED

//==============================================================================
/* An internal message pump class used in OSX and iOS. */
class MessageQueue
{
public:
    MessageQueue()
    {
       #if MAC_OS_X_VERSION_MIN_REQUIRED > MAC_OS_X_VERSION_10_4 && ! JUCE_IOS
        runLoop = CFRunLoopGetMain();
       #else
        runLoop = CFRunLoopGetCurrent();
       #endif

        CFRunLoopSourceContext sourceContext;
        zerostruct (sourceContext); // (can't use "= { 0 }" on this object because it's typedef'ed as a C struct)
        sourceContext.info = this;
        sourceContext.perform = runLoopSourceCallback;
        runLoopSource = CFRunLoopSourceCreate (kCFAllocatorDefault, 1, &sourceContext);
        CFRunLoopAddSource (runLoop, runLoopSource, kCFRunLoopCommonModes);
    }

    ~MessageQueue()
    {
        CFRunLoopRemoveSource (runLoop, runLoopSource, kCFRunLoopCommonModes);
        CFRunLoopSourceInvalidate (runLoopSource);
        CFRelease (runLoopSource);
    }

    void post (MessageManager::MessageBase* const message)
    {
        messages.add (message);
        CFRunLoopSourceSignal (runLoopSource);
        CFRunLoopWakeUp (runLoop);
    }

private:
    ReferenceCountedArray <MessageManager::MessageBase, CriticalSection> messages;
    CFRunLoopRef runLoop;
    CFRunLoopSourceRef runLoopSource;

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

    void runLoopCallback()
    {
        for (int i = 4; --i >= 0;)
            if (! deliverNextMessage())
                return;

        CFRunLoopSourceSignal (runLoopSource);
        CFRunLoopWakeUp (runLoop);
    }

    static void runLoopSourceCallback (void* info)
    {
        static_cast <MessageQueue*> (info)->runLoopCallback();
    }
};

#endif   // JUCE_OSX_MESSAGEQUEUE_H_INCLUDED
