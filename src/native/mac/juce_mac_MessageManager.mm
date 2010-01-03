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
    AppDelegateRedirector()
    {
#if MAC_OS_X_VERSION_MIN_REQUIRED > MAC_OS_X_VERSION_10_4
        runLoop = CFRunLoopGetMain();
#else
        runLoop = CFRunLoopGetCurrent();
#endif
        CFRunLoopSourceContext sourceContext;
        zerostruct (sourceContext);
        sourceContext.info = this;
        sourceContext.perform = runLoopSourceCallback;
        runLoopSource = CFRunLoopSourceCreate (kCFAllocatorDefault, 1, &sourceContext);
        CFRunLoopAddSource (runLoop, runLoopSource, kCFRunLoopCommonModes);
    }

    virtual ~AppDelegateRedirector()
    {
        CFRunLoopRemoveSource (runLoop, runLoopSource, kCFRunLoopCommonModes);
        CFRunLoopSourceInvalidate (runLoopSource);
        CFRelease (runLoopSource);

        while (messages.size() > 0)
            delete ((Message*) messages.remove(0));
    }

    virtual NSApplicationTerminateReply shouldTerminate()
    {
        if (JUCEApplication::getInstance() != 0)
        {
            JUCEApplication::getInstance()->systemRequestedQuit();
            return NSTerminateCancel;
        }

        return NSTerminateNow;
    }

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
        {
            String filename (nsStringToJuce ((NSString*) [filenames objectAtIndex: i]));
            if (filename.containsChar (T(' ')))
                filename = filename.quoted('"');

            files.add (filename);
        }

        if (files.size() > 0 && JUCEApplication::getInstance() != 0)
        {
            JUCEApplication::getInstance()->anotherInstanceStarted (files.joinIntoString (T(" ")));
        }
    }

    virtual void focusChanged()
    {
        juce_HandleProcessFocusChange();
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

    void postMessage (void* m)
    {
        messages.add (m);
        CFRunLoopSourceSignal (runLoopSource);
        CFRunLoopWakeUp (runLoop);
    }

private:
    CFRunLoopRef runLoop;
    CFRunLoopSourceRef runLoopSource;
    Array <void*, CriticalSection> messages;

    void runLoopCallback()
    {
        int numDispatched = 0;

        do
        {
            void* const nextMessage = messages.remove (0);

            if (nextMessage == 0)
                return;

            const ScopedAutoReleasePool pool;
            MessageManager::getInstance()->deliverMessage (nextMessage);

        } while (++numDispatched <= 4);

        CFRunLoopSourceSignal (runLoopSource);
        CFRunLoopWakeUp (runLoop);
    }

    static void runLoopSourceCallback (void* info)
    {
        ((AppDelegateRedirector*) info)->runLoopCallback();
    }
};


END_JUCE_NAMESPACE
using namespace JUCE_NAMESPACE;

#define JuceAppDelegate MakeObjCClassName(JuceAppDelegate)

@interface JuceAppDelegate   : NSObject
{
@private
    id oldDelegate;

@public
    AppDelegateRedirector* redirector;
}

- (JuceAppDelegate*) init;
- (void) dealloc;
- (BOOL) application: (NSApplication*) theApplication openFile: (NSString*) filename;
- (void) application: (NSApplication*) sender openFiles: (NSArray*) filenames;
- (NSApplicationTerminateReply) applicationShouldTerminate: (NSApplication*) app;
- (void) applicationDidBecomeActive: (NSNotification*) aNotification;
- (void) applicationDidResignActive: (NSNotification*) aNotification;
- (void) applicationWillUnhide: (NSNotification*) aNotification;
- (void) performCallback: (id) info;
- (void) dummyMethod;
@end

@implementation JuceAppDelegate

- (JuceAppDelegate*) init
{
    [super init];

    redirector = new AppDelegateRedirector();

    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];

    if (JUCEApplication::getInstance() != 0)
    {
        oldDelegate = [NSApp delegate];
        [NSApp setDelegate: self];
    }
    else
    {
        oldDelegate = 0;
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
    if (oldDelegate != 0)
        [NSApp setDelegate: oldDelegate];

    redirector->deleteSelf();
    [super dealloc];
}

- (NSApplicationTerminateReply) applicationShouldTerminate: (NSApplication*) app
{
    return redirector->shouldTerminate();
}

- (BOOL) application: (NSApplication*) app openFile: (NSString*) filename
{
    return redirector->openFile (filename);
}

- (void) application: (NSApplication*) sender openFiles: (NSArray*) filenames
{
    return redirector->openFiles (filenames);
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
    if (! quitMessagePosted) // check that the quit message wasn't already posted..
    {
        const ScopedAutoReleasePool pool;

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

static bool isEventBlockedByModalComps (NSEvent* e)
{
    if (Component::getNumCurrentlyModalComponents() == 0)
        return false;

    NSWindow* const w = [e window];
    if (w == 0 || [w worksWhenModal])
        return false;

    bool isKey = false, isInputAttempt = false;

    switch ([e type])
    {
        case NSKeyDown:
        case NSKeyUp:
            isKey = isInputAttempt = true;
            break;

        case NSLeftMouseDown:
        case NSRightMouseDown:
        case NSOtherMouseDown:
            isInputAttempt = true;
            break;

        case NSLeftMouseDragged:
        case NSRightMouseDragged:
        case NSLeftMouseUp:
        case NSRightMouseUp:
        case NSOtherMouseUp:
        case NSOtherMouseDragged:
            if (Component::getComponentUnderMouse() != 0)
                return false;
            break;

        case NSMouseMoved:
        case NSMouseEntered:
        case NSMouseExited:
        case NSCursorUpdate:
        case NSScrollWheel:
        case NSTabletPoint:
        case NSTabletProximity:
            break;

        default:
            return false;
    }

    for (int i = ComponentPeer::getNumPeers(); --i >= 0;)
    {
        ComponentPeer* const peer = ComponentPeer::getPeer (i);
        NSView* const compView = (NSView*) peer->getNativeHandle();

        if ([compView window] == w)
        {
            if (isKey)
            {
                if (compView == [w firstResponder])
                    return false;
            }
            else
            {
                if (NSPointInRect ([compView convertPoint: [e locationInWindow] fromView: nil],
                                   [compView bounds]))
                    return false;
            }
        }
    }

    if (isInputAttempt)
    {
        if (! [NSApp isActive])
            [NSApp activateIgnoringOtherApps: YES];

        Component* const modal = Component::getCurrentlyModalComponent (0);
        if (modal != 0)
            modal->inputAttemptWhenModal();
    }

    return true;
}

bool MessageManager::runDispatchLoopUntil (int millisecondsToRunFor)
{
    const ScopedAutoReleasePool pool;
    jassert (isThisTheMessageThread()); // must only be called by the message thread

    uint32 endTime = Time::getMillisecondCounter() + millisecondsToRunFor;

    while (! quitMessagePosted)
    {
        const ScopedAutoReleasePool pool2;

        CFRunLoopRunInMode (kCFRunLoopDefaultMode, 0.001, true);

        NSEvent* e = [NSApp nextEventMatchingMask: NSAnyEventMask
                                        untilDate: [NSDate dateWithTimeIntervalSinceNow: 0.001]
                                           inMode: NSDefaultRunLoopMode
                                          dequeue: YES];

        if (e != 0 && ! isEventBlockedByModalComps (e))
            [NSApp sendEvent: e];

        if (Time::getMillisecondCounter() >= endTime)
            break;
    }

    return ! quitMessagePosted;
}

//==============================================================================
void MessageManager::doPlatformSpecificInitialisation()
{
    if (juceAppDelegate == 0)
        juceAppDelegate = [[JuceAppDelegate alloc] init];

    // This launches a dummy thread, which forces Cocoa to initialise NSThreads
    // correctly (needed prior to 10.5)
    if (! [NSThread isMultiThreaded])
        [NSThread detachNewThreadSelector: @selector (dummyMethod)
                                 toTarget: juceAppDelegate
                               withObject: nil];

    initialiseMainMenu();
}

void MessageManager::doPlatformSpecificShutdown()
{
    if (juceAppDelegate != 0)
    {
        [[NSRunLoop currentRunLoop] cancelPerformSelectorsWithTarget: juceAppDelegate];
        [[NSNotificationCenter defaultCenter] removeObserver: juceAppDelegate];
        [juceAppDelegate release];
        juceAppDelegate = 0;
    }
}

bool juce_postMessageToSystemQueue (void* message)
{
    juceAppDelegate->redirector->postMessage (message);
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
