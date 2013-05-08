/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#ifndef __JUCE_OSX_MESSAGEQUEUE_JUCEHEADER__
#define __JUCE_OSX_MESSAGEQUEUE_JUCEHEADER__

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

#endif   // __JUCE_OSX_MESSAGEQUEUE_JUCEHEADER__
