/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

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

// (This file gets included by juce_mac_NativeCode.mm, rather than being
// compiled on its own).
#if JUCE_INCLUDED_FILE

struct CallbackMessagePayload
{
    MessageCallbackFunction* function;
    void* parameter;
    void* volatile result;
    bool volatile hasBeenExecuted;
};

END_JUCE_NAMESPACE

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
        JUCE_NAMESPACE::CallbackMessagePayload* pl = (JUCE_NAMESPACE::CallbackMessagePayload*) [((NSData*) info) bytes];

        if (pl != 0)
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

BEGIN_JUCE_NAMESPACE

//==============================================================================
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
    const ScopedAutoReleasePool pool;
    jassert (isThisTheMessageThread()); // must only be called by the message thread

    uint32 endTime = Time::getMillisecondCounter() + millisecondsToRunFor;
    NSDate* endDate = [NSDate dateWithTimeIntervalSinceNow: millisecondsToRunFor * 0.001];

    while (! quitMessagePosted)
    {
        const ScopedAutoReleasePool pool;

        [[NSRunLoop currentRunLoop] runMode: NSDefaultRunLoopMode
                                 beforeDate: endDate];

        if (millisecondsToRunFor >= 0 && Time::getMillisecondCounter() >= endTime)
            break;
    }

    return ! quitMessagePosted;
}

//==============================================================================
namespace iOSMessageLoopHelpers
{
    static CFRunLoopRef runLoop = 0;
    static CFRunLoopSourceRef runLoopSource = 0;
    static OwnedArray <Message, CriticalSection>* pendingMessages = 0;
    static JuceCustomMessageHandler* juceCustomMessageHandler = 0;

    void runLoopSourceCallback (void*)
    {
        if (pendingMessages != 0)
        {
            int numDispatched = 0;

            do
            {
                Message* const nextMessage = pendingMessages->removeAndReturn (0);

                if (nextMessage == 0)
                    return;

                const ScopedAutoReleasePool pool;
                MessageManager::getInstance()->deliverMessage (nextMessage);

            } while (++numDispatched <= 4);

            CFRunLoopSourceSignal (runLoopSource);
            CFRunLoopWakeUp (runLoop);
        }
    }
}

void MessageManager::doPlatformSpecificInitialisation()
{
    using namespace iOSMessageLoopHelpers;
    pendingMessages = new OwnedArray <Message, CriticalSection>();

    runLoop = CFRunLoopGetCurrent();
    CFRunLoopSourceContext sourceContext;
    zerostruct (sourceContext);
    sourceContext.perform = runLoopSourceCallback;
    runLoopSource = CFRunLoopSourceCreate (kCFAllocatorDefault, 1, &sourceContext);
    CFRunLoopAddSource (runLoop, runLoopSource, kCFRunLoopCommonModes);

    if (juceCustomMessageHandler == 0)
        juceCustomMessageHandler = [[JuceCustomMessageHandler alloc] init];
}

void MessageManager::doPlatformSpecificShutdown()
{
    using namespace iOSMessageLoopHelpers;
    CFRunLoopSourceInvalidate (runLoopSource);
    CFRelease (runLoopSource);
    runLoopSource = 0;
    deleteAndZero (pendingMessages);

    if (juceCustomMessageHandler != 0)
    {
        [[NSRunLoop currentRunLoop] cancelPerformSelectorsWithTarget: juceCustomMessageHandler];
        [juceCustomMessageHandler release];
        juceCustomMessageHandler = 0;
    }
}

bool juce_postMessageToSystemQueue (Message* message)
{
    using namespace iOSMessageLoopHelpers;

    if (pendingMessages != 0)
    {
        pendingMessages->add (message);
        CFRunLoopSourceSignal (runLoopSource);
        CFRunLoopWakeUp (runLoop);
    }

    return true;
}

void MessageManager::broadcastMessage (const String& value)
{
}

void* MessageManager::callFunctionOnMessageThread (MessageCallbackFunction* callback, void* data)
{
    using namespace iOSMessageLoopHelpers;

    if (isThisTheMessageThread())
    {
        return (*callback) (data);
    }
    else
    {
        // If a thread has a MessageManagerLock and then tries to call this method, it'll
        // deadlock because the message manager is blocked from running, so can never
        // call your function..
        jassert (! MessageManager::getInstance()->currentThreadHasLockedMessageManager());

        const ScopedAutoReleasePool pool;

        CallbackMessagePayload cmp;
        cmp.function = callback;
        cmp.parameter = data;
        cmp.result = 0;
        cmp.hasBeenExecuted = false;

        [juceCustomMessageHandler performSelectorOnMainThread: @selector (performCallback:)
                                                   withObject: [NSData dataWithBytesNoCopy: &cmp
                                                                                    length: sizeof (cmp)
                                                                              freeWhenDone: NO]
                                                waitUntilDone: YES];

        return cmp.result;
    }
}

#endif
