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

/* When you use multiple DLLs which share similarly-named obj-c classes - like
   for example having more than one juce plugin loaded into a host, then when a
   method is called, the actual code that runs might actually be in a different module
   than the one you expect... So any calls to library functions or statics that are
   made inside obj-c methods will probably end up getting executed in a different DLL's
   memory space. Not a great thing to happen - this obviously leads to bizarre crashes.

   To work around this insanity, I'm only allowing obj-c methods to make calls to
   virtual methods of an object that's known to live inside the right module's space.
*/
class AppDelegateRedirector
{
public:
    AppDelegateRedirector() {}
    virtual ~AppDelegateRedirector() {}

    virtual BOOL openFile (const NSString* filename)
    {
        if (JUCEApplication::getInstance() != 0)
        {
            JUCEApplication::getInstance()->anotherInstanceStarted (nsStringToJuce (filename));
            return YES;
        }

        return NO;
    }

    virtual void openFiles (NSArray* filenames)
    {
        StringArray files;
        for (unsigned int i = 0; i < [filenames count]; ++i)
            files.add (nsStringToJuce ((NSString*) [filenames objectAtIndex: i]));

        if (files.size() > 0 && JUCEApplication::getInstance() != 0)
        {
            JUCEApplication::getInstance()->anotherInstanceStarted (files.joinIntoString (T(" ")));
        }
    }

    virtual void focusChanged()
    {
        juce_HandleProcessFocusChange();
    }

    virtual void deliverMessage (void* message)
    {
        // no need for an mm lock here - deliverMessage locks it
        MessageManager::getInstance()->deliverMessage (message);
    }

    virtual void performCallback (CallbackMessagePayload* pl)
    {
        pl->result = (*pl->function) (pl->parameter);
        pl->hasBeenExecuted = true;
    }

    virtual void deleteSelf()
    {
        delete this;
    }
};

END_JUCE_NAMESPACE
using namespace JUCE_NAMESPACE;

#define JuceAppDelegate MakeObjCClassName(JuceAppDelegate)

static int numPendingMessages = 0;

@interface JuceAppDelegate   : NSObject <UIApplicationDelegate>
{
@private
    id oldDelegate;
    AppDelegateRedirector* redirector;

@public
    bool flushingMessages;
}

- (JuceAppDelegate*) init;
- (void) dealloc;
- (BOOL) application: (UIApplication*) application handleOpenURL: (NSURL*) url;
- (void) applicationDidBecomeActive: (NSNotification*) aNotification;
- (void) applicationDidResignActive: (NSNotification*) aNotification;
- (void) applicationWillUnhide: (NSNotification*) aNotification;
- (void) customEvent: (id) data;
- (void) performCallback: (id) info;
- (void) dummyMethod;
@end

@implementation JuceAppDelegate

- (JuceAppDelegate*) init
{
    [super init];

    redirector = new AppDelegateRedirector();
    numPendingMessages = 0;
    flushingMessages = false;

    oldDelegate = [[UIApplication sharedApplication] delegate];
    [[UIApplication sharedApplication] setDelegate: self];

    return self;
}

- (void) dealloc
{
    if (oldDelegate != 0)
        [[UIApplication sharedApplication] setDelegate: oldDelegate];

    redirector->deleteSelf();
    [super dealloc];
}

- (BOOL) application: (UIApplication*) application handleOpenURL: (NSURL*) url
{
    return redirector->openFile ([url absoluteString]);
}

- (void) applicationDidBecomeActive: (NSNotification*) aNotification
{
    redirector->focusChanged();
}

- (void) applicationDidResignActive: (NSNotification*) aNotification
{
    redirector->focusChanged();
}

- (void) applicationWillUnhide: (NSNotification*) aNotification
{
    redirector->focusChanged();
}

- (void) customEvent: (id) n
{
    atomicDecrement (numPendingMessages);

    NSData* data = (NSData*) n;
    void* message = 0;
    [data getBytes: &message length: sizeof (message)];
    [data release];

    if (message != 0 && ! flushingMessages)
        redirector->deliverMessage (message);
}

- (void) performCallback: (id) info
{
    if ([info isKindOfClass: [NSData class]])
    {
        CallbackMessagePayload* pl = (CallbackMessagePayload*) [((NSData*) info) bytes];

        if (pl != 0)
            redirector->performCallback (pl);
    }
    else
    {
        jassertfalse // should never get here!
    }
}

- (void) dummyMethod  {}   // (used as a way of running a dummy thread)

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
    Message* const m = new Message (quitMessageId, 0, 0, 0);
    m->messageRecipient = 0;
    postMessageToQueue (m);

    quitMessagePosted = true;
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
void MessageManager::doPlatformSpecificInitialisation()
{
    if (juceAppDelegate == 0)
        juceAppDelegate = [[JuceAppDelegate alloc] init];
}

void MessageManager::doPlatformSpecificShutdown()
{
    if (juceAppDelegate != 0)
    {
        [[NSRunLoop currentRunLoop] cancelPerformSelectorsWithTarget: juceAppDelegate];
        [[NSNotificationCenter defaultCenter] removeObserver: juceAppDelegate];

        // Annoyingly, cancelPerformSelectorsWithTarget can't actually cancel the messages
        // sent by performSelectorOnMainThread, so need to manually flush these before quitting..
        juceAppDelegate->flushingMessages = true;

        for (int i = 100; --i >= 0 && numPendingMessages > 0;)
        {
            const ScopedAutoReleasePool pool;
            [[NSRunLoop currentRunLoop] runMode: NSDefaultRunLoopMode
                                     beforeDate: [NSDate dateWithTimeIntervalSinceNow: 5 * 0.001]];
        }

        [juceAppDelegate release];
        juceAppDelegate = 0;
    }
}

bool juce_postMessageToSystemQueue (void* message)
{
    atomicIncrement (numPendingMessages);

    [juceAppDelegate performSelectorOnMainThread: @selector (customEvent:)
                     withObject: (id) [[NSData alloc] initWithBytes: &message length: (int) sizeof (message)]
                     waitUntilDone: NO];
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
