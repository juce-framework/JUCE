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

typedef void (*AppFocusChangeCallback)();
AppFocusChangeCallback appFocusChangeCallback = nullptr;

typedef bool (*CheckEventBlockedByModalComps) (NSEvent*);
CheckEventBlockedByModalComps isEventBlockedByModalComps = nullptr;

//==============================================================================
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

    virtual NSApplicationTerminateReply shouldTerminate()
    {
        if (JUCEApplicationBase::getInstance() != nullptr)
        {
            JUCEApplicationBase::getInstance()->systemRequestedQuit();

            if (! MessageManager::getInstance()->hasStopMessageBeenSent())
                return NSTerminateCancel;
        }

        return NSTerminateNow;
    }

    virtual void willTerminate()
    {
        JUCEApplicationBase::appWillTerminateByForce();
    }

    virtual BOOL openFile (NSString* filename)
    {
        if (JUCEApplicationBase::getInstance() != nullptr)
        {
            JUCEApplicationBase::getInstance()->anotherInstanceStarted (quotedIfContainsSpaces (filename));
            return YES;
        }

        return NO;
    }

    virtual void openFiles (NSArray* filenames)
    {
        StringArray files;
        for (unsigned int i = 0; i < [filenames count]; ++i)
            files.add (quotedIfContainsSpaces ((NSString*) [filenames objectAtIndex: i]));

        if (files.size() > 0 && JUCEApplicationBase::getInstance() != nullptr)
            JUCEApplicationBase::getInstance()->anotherInstanceStarted (files.joinIntoString (" "));
    }

    virtual void focusChanged()
    {
        if (appFocusChangeCallback != nullptr)
            (*appFocusChangeCallback)();
    }

    virtual void deleteSelf()
    {
        delete this;
    }

    void postMessage (MessageManager::MessageBase* const m)
    {
        messageQueue.post (m);
    }

    static NSString* getBroacastEventName()
    {
        return juceStringToNS ("juce_" + String::toHexString (File::getSpecialLocation (File::currentExecutableFile).hashCode64()));
    }

private:
    CFRunLoopRef runLoop;
    CFRunLoopSourceRef runLoopSource;
    MessageQueue messageQueue;

    static String quotedIfContainsSpaces (NSString* file)
    {
        String s (nsStringToJuce (file));
        if (s.containsChar (' '))
            s = s.quoted ('"');

        return s;
    }
};


} // (juce namespace)

using namespace juce;

#define JuceAppDelegate MakeObjCClassName(JuceAppDelegate)

//==============================================================================
@interface JuceAppDelegate   : NSObject
{
@public
    AppDelegateRedirector* redirector;
}

- (JuceAppDelegate*) init;
- (void) dealloc;
- (void) unregisterObservers;
- (BOOL) application: (NSApplication*) theApplication openFile: (NSString*) filename;
- (void) application: (NSApplication*) sender openFiles: (NSArray*) filenames;
- (NSApplicationTerminateReply) applicationShouldTerminate: (NSApplication*) app;
- (void) applicationWillTerminate: (NSNotification*) aNotification;
- (void) applicationDidBecomeActive: (NSNotification*) aNotification;
- (void) applicationDidResignActive: (NSNotification*) aNotification;
- (void) applicationWillUnhide: (NSNotification*) aNotification;
- (void) broadcastMessageCallback: (NSNotification*) info;
- (void) dummyMethod;
@end

@implementation JuceAppDelegate

- (JuceAppDelegate*) init
{
    [super init];

    redirector = new AppDelegateRedirector();

    if (JUCEApplicationBase::isStandaloneApp())
    {
        [NSApp setDelegate: self];

        [[NSDistributedNotificationCenter defaultCenter] addObserver: self
                                                            selector: @selector (broadcastMessageCallback:)
                                                                name: AppDelegateRedirector::getBroacastEventName()
                                                              object: nil];
    }
    else
    {
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];

        [center addObserver: self selector: @selector (applicationDidResignActive:)
                       name: NSApplicationDidResignActiveNotification object: NSApp];

        [center addObserver: self selector: @selector (applicationDidBecomeActive:)
                       name: NSApplicationDidBecomeActiveNotification object: NSApp];

        [center addObserver: self selector: @selector (applicationWillUnhide:)
                       name: NSApplicationWillUnhideNotification object: NSApp];
    }

    return self;
}

- (void) dealloc
{
    redirector->deleteSelf();
    [super dealloc];
}

- (void) unregisterObservers
{
    [[NSRunLoop currentRunLoop] cancelPerformSelectorsWithTarget: self];
    [[NSNotificationCenter defaultCenter] removeObserver: self];

    if (JUCEApplicationBase::isStandaloneApp())
    {
        [NSApp setDelegate: nil];

        [[NSDistributedNotificationCenter defaultCenter] removeObserver: self
                                                                   name: AppDelegateRedirector::getBroacastEventName()
                                                                 object: nil];
    }
}

- (NSApplicationTerminateReply) applicationShouldTerminate: (NSApplication*) app
{
    (void) app;
    return redirector->shouldTerminate();
}

- (void) applicationWillTerminate: (NSNotification*) aNotification
{
    (void) aNotification;
    redirector->willTerminate();
}

- (BOOL) application: (NSApplication*) app openFile: (NSString*) filename
{
    (void) app;
    return redirector->openFile (filename);
}

- (void) application: (NSApplication*) sender openFiles: (NSArray*) filenames
{
    (void) sender;
    return redirector->openFiles (filenames);
}

- (void) applicationDidBecomeActive: (NSNotification*) notification
{
    (void) notification;
    redirector->focusChanged();
}

- (void) applicationDidResignActive: (NSNotification*) notification
{
    (void) notification;
    redirector->focusChanged();
}

- (void) applicationWillUnhide: (NSNotification*) notification
{
    (void) notification;
    redirector->focusChanged();
}

- (void) broadcastMessageCallback: (NSNotification*) n
{
    NSDictionary* dict = (NSDictionary*) [n userInfo];
    const String messageString (nsStringToJuce ((NSString*) [dict valueForKey: nsStringLiteral ("message")]));
    MessageManager::getInstance()->deliverBroadcastMessage (messageString);
}

- (void) dummyMethod  {}   // (used as a way of running a dummy thread)

@end

//==============================================================================
namespace juce
{

static JuceAppDelegate* juceAppDelegate = nil;

void MessageManager::runDispatchLoop()
{
    if (! quitMessagePosted) // check that the quit message wasn't already posted..
    {
        JUCE_AUTORELEASEPOOL

        // must only be called by the message thread!
        jassert (isThisTheMessageThread());

      #if JUCE_CATCH_UNHANDLED_EXCEPTIONS
        @try
        {
            [NSApp run];
        }
        @catch (NSException* e)
        {
            // An AppKit exception will kill the app, but at least this provides a chance to log it.,
            std::runtime_error ex (std::string ("NSException: ") + [[e name] UTF8String] + ", Reason:" + [[e reason] UTF8String]);
            JUCEApplication::sendUnhandledException (&ex, __FILE__, __LINE__);
        }
        @finally
        {
        }
       #else
        [NSApp run];
       #endif
    }
}

void MessageManager::stopDispatchLoop()
{
    quitMessagePosted = true;
    [NSApp stop: nil];
    [NSApp activateIgnoringOtherApps: YES]; // (if the app is inactive, it sits there and ignores the quit request until the next time it gets activated)
    [NSEvent startPeriodicEventsAfterDelay: 0 withPeriod: 0.1];
}

#if JUCE_MODAL_LOOPS_PERMITTED
bool MessageManager::runDispatchLoopUntil (int millisecondsToRunFor)
{
    jassert (millisecondsToRunFor >= 0);
    jassert (isThisTheMessageThread()); // must only be called by the message thread

    uint32 endTime = Time::getMillisecondCounter() + (uint32) millisecondsToRunFor;

    while (! quitMessagePosted)
    {
        JUCE_AUTORELEASEPOOL

        CFRunLoopRunInMode (kCFRunLoopDefaultMode, 0.001, true);

        NSEvent* e = [NSApp nextEventMatchingMask: NSAnyEventMask
                                        untilDate: [NSDate dateWithTimeIntervalSinceNow: 0.001]
                                           inMode: NSDefaultRunLoopMode
                                          dequeue: YES];

        if (e != nil && (isEventBlockedByModalComps == nullptr || ! (*isEventBlockedByModalComps) (e)))
            [NSApp sendEvent: e];

        if (Time::getMillisecondCounter() >= endTime)
            break;
    }

    return ! quitMessagePosted;
}
#endif

//==============================================================================
void initialiseNSApplication();
void initialiseNSApplication()
{
    JUCE_AUTORELEASEPOOL
    [NSApplication sharedApplication];
}

void MessageManager::doPlatformSpecificInitialisation()
{
    if (juceAppDelegate == nil)
        juceAppDelegate = [[JuceAppDelegate alloc] init];

   #if ! (defined (MAC_OS_X_VERSION_10_5) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_5)
    // This launches a dummy thread, which forces Cocoa to initialise NSThreads correctly (needed prior to 10.5)
    if (! [NSThread isMultiThreaded])
        [NSThread detachNewThreadSelector: @selector (dummyMethod)
                                 toTarget: juceAppDelegate
                               withObject: nil];
   #endif
}

void MessageManager::doPlatformSpecificShutdown()
{
    if (juceAppDelegate != nil)
    {
        [juceAppDelegate unregisterObservers];
        [juceAppDelegate release];
        juceAppDelegate = nil;
    }
}

bool MessageManager::postMessageToSystemQueue (MessageBase* message)
{
    juceAppDelegate->redirector->postMessage (message);
    return true;
}

void MessageManager::broadcastMessage (const String& message)
{
    NSDictionary* info = [NSDictionary dictionaryWithObject: juceStringToNS (message)
                                                     forKey: nsStringLiteral ("message")];

    [[NSDistributedNotificationCenter defaultCenter] postNotificationName: AppDelegateRedirector::getBroacastEventName()
                                                                   object: nil
                                                                 userInfo: info];
}
