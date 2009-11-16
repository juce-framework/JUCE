/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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
using namespace JUCE_NAMESPACE;

@interface JuceAppDelegate   : NSObject <UIApplicationDelegate>
{
}

- (JuceAppDelegate*) init;
- (void) dealloc;
- (BOOL) application: (UIApplication*) application handleOpenURL: (NSURL*) url;
- (void) applicationDidBecomeActive: (NSNotification*) aNotification;
- (void) applicationDidResignActive: (NSNotification*) aNotification;
- (void) applicationWillUnhide: (NSNotification*) aNotification;
- (void) customEvent: (id) data;
- (void) performCallback: (id) info;
@end

@implementation JuceAppDelegate

- (JuceAppDelegate*) init
{
    [super init];

    [[UIApplication sharedApplication] setDelegate: self];

    return self;
}

- (void) dealloc
{
    [[UIApplication sharedApplication] setDelegate: nil];
    [super dealloc];
}

- (BOOL) application: (UIApplication*) application handleOpenURL: (NSURL*) url
{
    if (JUCEApplication::getInstance() != 0)
    {
        JUCEApplication::getInstance()->anotherInstanceStarted (nsStringToJuce ([url absoluteString]));
        return YES;
    }

    return NO;
}

- (void) applicationDidBecomeActive: (NSNotification*) aNotification
{
    juce_HandleProcessFocusChange();
}

- (void) applicationDidResignActive: (NSNotification*) aNotification
{
    juce_HandleProcessFocusChange();
}

- (void) applicationWillUnhide: (NSNotification*) aNotification
{
    juce_HandleProcessFocusChange();
}

- (void) customEvent: (id) n
{
    NSData* data = (NSData*) n;
    void* message = 0;
    [data getBytes: &message length: sizeof (message)];
    [data release];

    if (message != 0)
        MessageManager::getInstance()->deliverMessage (message);
}

- (void) performCallback: (id) info
{
    if ([info isKindOfClass: [NSData class]])
    {
        CallbackMessagePayload* pl = (CallbackMessagePayload*) [((NSData*) info) bytes];

        if (pl != 0)
        {
            pl->result = (*pl->function) (pl->parameter);
            pl->hasBeenExecuted = true;
        }
    }
    else
    {
        jassertfalse // should never get here!
    }
}

@end

BEGIN_JUCE_NAMESPACE

static JuceAppDelegate* juceAppDelegate = 0;

void MessageManager::runDispatchLoop()
{
    jassert (isThisTheMessageThread()); // must only be called by the message thread
    runDispatchLoopUntil (-1);
}

static const int quitMessageId = 0xfffff321;

void MessageManager::stopDispatchLoop()
{
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
static CFRunLoopTimerRef messageTimer = 0;
static Array <void*, CriticalSection>* pendingMessages = 0;

static void timerCallback (CFRunLoopTimerRef, void*)
{
    if (pendingMessages != 0)
    {
        const int numToDispatch = jmin (4, pendingMessages->size());

        for (int i = 0; i < numToDispatch; ++i)
        {
            void* const nextMessage = (*pendingMessages)[i];

            if (nextMessage != 0)
                MessageManager::getInstance()->deliverMessage (nextMessage);
        }

        pendingMessages->removeRange (0, numToDispatch);

        if (pendingMessages->size() > 0)
            CFRunLoopTimerSetNextFireDate (messageTimer, CFAbsoluteTimeGetCurrent() - 0.5);
    }
}

void MessageManager::doPlatformSpecificInitialisation()
{
    pendingMessages = new Array <void*, CriticalSection>();

    messageTimer = CFRunLoopTimerCreate (kCFAllocatorDefault, CFAbsoluteTimeGetCurrent() + 60.0, 60.0,
                                         0, 0, timerCallback, 0);

    CFRunLoopAddTimer (CFRunLoopGetMain(), messageTimer, kCFRunLoopCommonModes);

    if (juceAppDelegate == 0)
        juceAppDelegate = [[JuceAppDelegate alloc] init];
}

void MessageManager::doPlatformSpecificShutdown()
{
    CFRunLoopTimerInvalidate (messageTimer);
    CFRunLoopRemoveTimer (CFRunLoopGetMain(), messageTimer, kCFRunLoopCommonModes);
    CFRelease (messageTimer);
    messageTimer = 0;

    if (pendingMessages != 0)
    {
        while (pendingMessages->size() > 0)
            delete ((Message*) pendingMessages->remove(0));

        deleteAndZero (pendingMessages);
    }

    if (juceAppDelegate != 0)
    {
        [[NSRunLoop currentRunLoop] cancelPerformSelectorsWithTarget: juceAppDelegate];
        [juceAppDelegate release];
        juceAppDelegate = 0;
    }
}

bool juce_postMessageToSystemQueue (void* message)
{
    if (pendingMessages != 0)
    {
        pendingMessages->add (message);
        CFRunLoopTimerSetNextFireDate (messageTimer, CFAbsoluteTimeGetCurrent() - 1.0);
    }

    return true;
}

void MessageManager::broadcastMessage (const String& value) throw()
{
}

void* MessageManager::callFunctionOnMessageThread (MessageCallbackFunction* callback,
                                                   void* data)
{
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

        [juceAppDelegate performSelectorOnMainThread: @selector (performCallback:)
                                          withObject: [NSData dataWithBytesNoCopy: &cmp
                                                                           length: sizeof (cmp)
                                                                     freeWhenDone: NO]
                                       waitUntilDone: YES];

        return cmp.result;
    }
}

#endif
