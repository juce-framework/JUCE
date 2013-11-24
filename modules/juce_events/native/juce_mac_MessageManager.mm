/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

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

typedef void (*AppFocusChangeCallback)();
AppFocusChangeCallback appFocusChangeCallback = nullptr;

typedef bool (*CheckEventBlockedByModalComps) (NSEvent*);
CheckEventBlockedByModalComps isEventBlockedByModalComps = nullptr;

typedef void (*MenuTrackingChangedCallback)(bool);
MenuTrackingChangedCallback menuTrackingChangedCallback = nullptr;

//==============================================================================
struct AppDelegate
{
public:
    AppDelegate()
    {
        static AppDelegateClass cls;
        delegate = [cls.createInstance() init];

        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];

        [center addObserver: delegate selector: @selector (mainMenuTrackingBegan:)
                       name: NSMenuDidBeginTrackingNotification object: nil];
        [center addObserver: delegate selector: @selector (mainMenuTrackingEnded:)
                       name: NSMenuDidEndTrackingNotification object: nil];

        if (JUCEApplicationBase::isStandaloneApp())
        {
            [NSApp setDelegate: delegate];

            [[NSDistributedNotificationCenter defaultCenter] addObserver: delegate
                                                                selector: @selector (broadcastMessageCallback:)
                                                                    name: getBroacastEventName()
                                                                  object: nil];
        }
        else
        {
            [center addObserver: delegate selector: @selector (applicationDidResignActive:)
                           name: NSApplicationDidResignActiveNotification object: NSApp];

            [center addObserver: delegate selector: @selector (applicationDidBecomeActive:)
                           name: NSApplicationDidBecomeActiveNotification object: NSApp];

            [center addObserver: delegate selector: @selector (applicationWillUnhide:)
                           name: NSApplicationWillUnhideNotification object: NSApp];
        }
    }

    ~AppDelegate()
    {
        [[NSRunLoop currentRunLoop] cancelPerformSelectorsWithTarget: delegate];
        [[NSNotificationCenter defaultCenter] removeObserver: delegate];

        if (JUCEApplicationBase::isStandaloneApp())
        {
            [NSApp setDelegate: nil];

            [[NSDistributedNotificationCenter defaultCenter] removeObserver: delegate
                                                                       name: getBroacastEventName()
                                                                     object: nil];
        }

        [delegate release];
    }

    static NSString* getBroacastEventName()
    {
        return juceStringToNS ("juce_" + String::toHexString (File::getSpecialLocation (File::currentExecutableFile).hashCode64()));
    }

    MessageQueue messageQueue;
    id delegate;

private:
    //==============================================================================
    struct AppDelegateClass   : public ObjCClass <NSObject>
    {
        AppDelegateClass()  : ObjCClass <NSObject> ("JUCEAppDelegate_")
        {
            addMethod (@selector (applicationShouldTerminate:),   applicationShouldTerminate, "I@:@");
            addMethod (@selector (applicationWillTerminate:),     applicationWillTerminate,   "v@:@");
            addMethod (@selector (application:openFile:),         application_openFile,       "c@:@@");
            addMethod (@selector (application:openFiles:),        application_openFiles,      "v@:@@");
            addMethod (@selector (applicationDidBecomeActive:),   applicationDidBecomeActive, "v@:@");
            addMethod (@selector (applicationDidResignActive:),   applicationDidResignActive, "v@:@");
            addMethod (@selector (applicationWillUnhide:),        applicationWillUnhide,      "v@:@");
            addMethod (@selector (broadcastMessageCallback:),     broadcastMessageCallback,   "v@:@");
            addMethod (@selector (mainMenuTrackingBegan:),        mainMenuTrackingBegan,      "v@:@");
            addMethod (@selector (mainMenuTrackingEnded:),        mainMenuTrackingEnded,      "v@:@");
            addMethod (@selector (dummyMethod),                   dummyMethod,                "v@:");

            registerClass();
        }

    private:
        static NSApplicationTerminateReply applicationShouldTerminate (id /*self*/, SEL, NSApplication*)
        {
            if (JUCEApplicationBase* const app = JUCEApplicationBase::getInstance())
            {
                app->systemRequestedQuit();

                if (! MessageManager::getInstance()->hasStopMessageBeenSent())
                    return NSTerminateCancel;
            }

            return NSTerminateNow;
        }

        static void applicationWillTerminate (id /*self*/, SEL, NSNotification*)
        {
            JUCEApplicationBase::appWillTerminateByForce();
        }

        static BOOL application_openFile (id /*self*/, SEL, NSApplication*, NSString* filename)
        {
            if (JUCEApplicationBase* const app = JUCEApplicationBase::getInstance())
            {
                app->anotherInstanceStarted (quotedIfContainsSpaces (filename));
                return YES;
            }

            return NO;
        }

        static void application_openFiles (id /*self*/, SEL, NSApplication*, NSArray* filenames)
        {
            if (JUCEApplicationBase* const app = JUCEApplicationBase::getInstance())
            {
                StringArray files;

                for (NSString* f in filenames)
                    files.add (quotedIfContainsSpaces (f));

                if (files.size() > 0)
                    app->anotherInstanceStarted (files.joinIntoString (" "));
            }
        }

        static void applicationDidBecomeActive (id /*self*/, SEL, NSNotification*)  { focusChanged(); }
        static void applicationDidResignActive (id /*self*/, SEL, NSNotification*)  { focusChanged(); }
        static void applicationWillUnhide      (id /*self*/, SEL, NSNotification*)  { focusChanged(); }

        static void broadcastMessageCallback (id /*self*/, SEL, NSNotification* n)
        {
            NSDictionary* dict = (NSDictionary*) [n userInfo];
            const String messageString (nsStringToJuce ((NSString*) [dict valueForKey: nsStringLiteral ("message")]));
            MessageManager::getInstance()->deliverBroadcastMessage (messageString);
        }

        static void mainMenuTrackingBegan (id /*self*/, SEL, NSNotification*)
        {
            if (menuTrackingChangedCallback != nullptr)
                (*menuTrackingChangedCallback) (true);
        }

        static void mainMenuTrackingEnded (id /*self*/, SEL, NSNotification*)
        {
            if (menuTrackingChangedCallback != nullptr)
                (*menuTrackingChangedCallback) (false);
        }

        static void dummyMethod (id /*self*/, SEL) {}   // (used as a way of running a dummy thread)

    private:
        static void focusChanged()
        {
            if (appFocusChangeCallback != nullptr)
                (*appFocusChangeCallback)();
        }

        static String quotedIfContainsSpaces (NSString* file)
        {
            String s (nsStringToJuce (file));
            if (s.containsChar (' '))
                s = s.quoted ('"');

            return s;
        }
    };
};

//==============================================================================
void MessageManager::runDispatchLoop()
{
    if (! quitMessagePosted) // check that the quit message wasn't already posted..
    {
        JUCE_AUTORELEASEPOOL
        {
            // must only be called by the message thread!
            jassert (isThisTheMessageThread());

          #if JUCE_PROJUCER_LIVE_BUILD
            runDispatchLoopUntil (std::numeric_limits<int>::max());
          #else
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
          #endif
        }
    }
}

void MessageManager::stopDispatchLoop()
{
    jassert (isThisTheMessageThread()); // must only be called by the message thread

    quitMessagePosted = true;
   #if ! JUCE_PROJUCER_LIVE_BUILD
    [NSApp stop: nil];
    [NSApp activateIgnoringOtherApps: YES]; // (if the app is inactive, it sits there and ignores the quit request until the next time it gets activated)
    [NSEvent startPeriodicEventsAfterDelay: 0 withPeriod: 0.1];
   #endif
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
        {
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
    }

    return ! quitMessagePosted;
}
#endif

//==============================================================================
void initialiseNSApplication();
void initialiseNSApplication()
{
    JUCE_AUTORELEASEPOOL
    {
        [NSApplication sharedApplication];
    }
}

static AppDelegate* appDelegate = nullptr;

void MessageManager::doPlatformSpecificInitialisation()
{
    if (appDelegate == nil)
        appDelegate = new AppDelegate();

   #if ! (defined (MAC_OS_X_VERSION_10_5) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_5)
    // This launches a dummy thread, which forces Cocoa to initialise NSThreads correctly (needed prior to 10.5)
    if (! [NSThread isMultiThreaded])
        [NSThread detachNewThreadSelector: @selector (dummyMethod)
                                 toTarget: appDelegate->delegate
                               withObject: nil];
   #endif
}

void MessageManager::doPlatformSpecificShutdown()
{
    delete appDelegate;
    appDelegate = nullptr;
}

bool MessageManager::postMessageToSystemQueue (MessageBase* message)
{
    jassert (appDelegate != nil);
    appDelegate->messageQueue.post (message);
    return true;
}

void MessageManager::broadcastMessage (const String& message)
{
    NSDictionary* info = [NSDictionary dictionaryWithObject: juceStringToNS (message)
                                                     forKey: nsStringLiteral ("message")];

    [[NSDistributedNotificationCenter defaultCenter] postNotificationName: AppDelegate::getBroacastEventName()
                                                                   object: nil
                                                                 userInfo: info];
}

// Special function used by some plugin classes to re-post carbon events
void repostCurrentNSEvent();
void repostCurrentNSEvent()
{
    struct EventReposter  : public CallbackMessage
    {
        EventReposter() : e ([[NSApp currentEvent] retain])  {}
        ~EventReposter()  { [e release]; }

        void messageCallback() override
        {
            [NSApp postEvent: e atStart: YES];
        }

        NSEvent* e;
    };

    (new EventReposter())->post();
}
