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

struct CallbackMessagePayload
{
    MessageCallbackFunction* function;
    void* parameter;
    void* volatile result;
    bool volatile hasBeenExecuted;
};

} // (juce namespace)

//==============================================================================
@interface JuceCustomMessageHandler   : NSObject
{
}

- (void) performCallback: (id) info;

@end

//==============================================================================
@implementation JuceCustomMessageHandler

- (void) performCallback: (id) info
{
    if ([info isKindOfClass: [NSData class]])
    {
        juce::CallbackMessagePayload* pl = (juce::CallbackMessagePayload*) [((NSData*) info) bytes];

        if (pl != nullptr)
        {
            pl->result = (*pl->function) (pl->parameter);
            pl->hasBeenExecuted = true;
        }
    }
    else
    {
        jassertfalse; // should never get here!
    }
}

@end

//==============================================================================
namespace juce
{

void MessageManager::runDispatchLoop()
{
    jassert (isThisTheMessageThread()); // must only be called by the message thread
    runDispatchLoopUntil (-1);
}

void MessageManager::stopDispatchLoop()
{
    [[[UIApplication sharedApplication] delegate] applicationWillTerminate: [UIApplication sharedApplication]];
    exit (0); // iPhone apps get no mercy..
}

bool MessageManager::runDispatchLoopUntil (int millisecondsToRunFor)
{
    JUCE_AUTORELEASEPOOL
    jassert (isThisTheMessageThread()); // must only be called by the message thread

    uint32 endTime = Time::getMillisecondCounter() + millisecondsToRunFor;
    NSDate* endDate = [NSDate dateWithTimeIntervalSinceNow: millisecondsToRunFor * 0.001];

    while (! quitMessagePosted)
    {
        JUCE_AUTORELEASEPOOL

        [[NSRunLoop currentRunLoop] runMode: NSDefaultRunLoopMode
                                 beforeDate: endDate];

        if (millisecondsToRunFor >= 0 && Time::getMillisecondCounter() >= endTime)
            break;
    }

    return ! quitMessagePosted;
}

//==============================================================================
struct MessageDispatchSystem
{
    MessageDispatchSystem()
        : juceCustomMessageHandler (nil)
    {
        juceCustomMessageHandler = [[JuceCustomMessageHandler alloc] init];
    }

    ~MessageDispatchSystem()
    {
        [[NSRunLoop currentRunLoop] cancelPerformSelectorsWithTarget: juceCustomMessageHandler];
        [juceCustomMessageHandler release];
    }

    JuceCustomMessageHandler* juceCustomMessageHandler;
    MessageQueue messageQueue;
};

static ScopedPointer<MessageDispatchSystem> dispatcher;

void MessageManager::doPlatformSpecificInitialisation()
{
    if (dispatcher == nullptr)
        dispatcher = new MessageDispatchSystem();
}

void MessageManager::doPlatformSpecificShutdown()
{
    dispatcher = nullptr;
}

bool MessageManager::postMessageToSystemQueue (Message* message)
{
    if (dispatcher != nullptr)
        dispatcher->messageQueue.post (message);

    return true;
}

void MessageManager::broadcastMessage (const String& value)
{
}

void* MessageManager::callFunctionOnMessageThread (MessageCallbackFunction* callback, void* data)
{
    if (isThisTheMessageThread())
    {
        return (*callback) (data);
    }
    else
    {
        jassert (dispatcher != nullptr); // trying to call this when the juce system isn't initialised..

        // If a thread has a MessageManagerLock and then tries to call this method, it'll
        // deadlock because the message manager is blocked from running, so can never
        // call your function..
        jassert (! MessageManager::getInstance()->currentThreadHasLockedMessageManager());

        JUCE_AUTORELEASEPOOL

        CallbackMessagePayload cmp;
        cmp.function = callback;
        cmp.parameter = data;
        cmp.result = 0;
        cmp.hasBeenExecuted = false;

        [dispatcher->juceCustomMessageHandler performSelectorOnMainThread: @selector (performCallback:)
                                                               withObject: [NSData dataWithBytesNoCopy: &cmp
                                                                                                length: sizeof (cmp)
                                                                                          freeWhenDone: NO]
                                                            waitUntilDone: YES];

        return cmp.result;
    }
}
