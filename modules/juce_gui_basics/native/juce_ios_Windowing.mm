/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{
    extern bool isIOSAppActive;

    struct AppInactivityCallback // NB: careful, this declaration is duplicated in other modules
    {
        virtual ~AppInactivityCallback() {}
        virtual void appBecomingInactive() = 0;
    };

    // This is an internal list of callbacks (but currently used between modules)
    Array<AppInactivityCallback*> appBecomingInactiveCallbacks;
}

@interface JuceAppStartupDelegate : NSObject <UIApplicationDelegate>
{
    UIBackgroundTaskIdentifier appSuspendTask;
}

@property (strong, nonatomic) UIWindow *window;
- (id)init;
- (void) applicationDidFinishLaunching: (UIApplication*) application;
- (void) applicationWillTerminate: (UIApplication*) application;
- (void) applicationDidEnterBackground: (UIApplication*) application;
- (void) applicationWillEnterForeground: (UIApplication*) application;
- (void) applicationDidBecomeActive: (UIApplication*) application;
- (void) applicationWillResignActive: (UIApplication*) application;
- (void) application: (UIApplication*) application handleEventsForBackgroundURLSession: (NSString*)identifier
   completionHandler: (void (^)(void))completionHandler;
@end

@implementation JuceAppStartupDelegate

- (id)init
{
    self = [super init];
    appSuspendTask = UIBackgroundTaskInvalid;

    return self;
}

- (void) applicationDidFinishLaunching: (UIApplication*) application
{
    ignoreUnused (application);
    initialiseJuce_GUI();

    if (JUCEApplicationBase* app = JUCEApplicationBase::createInstance())
    {
        if (! app->initialiseApp())
            exit (app->shutdownApp());
    }
    else
    {
        jassertfalse; // you must supply an application object for an iOS app!
    }
}

- (void) applicationWillTerminate: (UIApplication*) application
{
    ignoreUnused (application);
    JUCEApplicationBase::appWillTerminateByForce();
}

- (void) applicationDidEnterBackground: (UIApplication*) application
{
    if (JUCEApplicationBase* const app = JUCEApplicationBase::getInstance())
    {
       #if JUCE_EXECUTE_APP_SUSPEND_ON_BACKGROUND_TASK
        appSuspendTask = [application beginBackgroundTaskWithName:@"JUCE Suspend Task" expirationHandler:^{
            if (appSuspendTask != UIBackgroundTaskInvalid)
            {
                [application endBackgroundTask:appSuspendTask];
                appSuspendTask = UIBackgroundTaskInvalid;
            }
        }];

        MessageManager::callAsync ([self,application,app] ()  { app->suspended(); });
       #else
        ignoreUnused (application);
        app->suspended();
       #endif
    }
}

- (void) applicationWillEnterForeground: (UIApplication*) application
{
    ignoreUnused (application);

    if (JUCEApplicationBase* const app = JUCEApplicationBase::getInstance())
        app->resumed();
}

- (void) applicationDidBecomeActive: (UIApplication*) application
{
    ignoreUnused (application);
    isIOSAppActive = true;
}

- (void) applicationWillResignActive: (UIApplication*) application
{
    ignoreUnused (application);
    isIOSAppActive = false;

    for (int i = appBecomingInactiveCallbacks.size(); --i >= 0;)
        appBecomingInactiveCallbacks.getReference(i)->appBecomingInactive();
}

- (void) application: (UIApplication*) application handleEventsForBackgroundURLSession: (NSString*)identifier
   completionHandler: (void (^)(void))completionHandler
{
    ignoreUnused (application);
    URL::DownloadTask::juce_iosURLSessionNotify (nsStringToJuce (identifier));
    completionHandler();
}

@end

namespace juce
{

int juce_iOSMain (int argc, const char* argv[], void* customDelgatePtr);
int juce_iOSMain (int argc, const char* argv[], void* customDelagetPtr)
{
    Class delegateClass = (customDelagetPtr != nullptr ? reinterpret_cast<Class> (customDelagetPtr) : [JuceAppStartupDelegate class]);

    return UIApplicationMain (argc, const_cast<char**> (argv), nil, NSStringFromClass (delegateClass));
}

//==============================================================================
void LookAndFeel::playAlertSound()
{
    // TODO
}

//==============================================================================
class iOSMessageBox;

#if defined (__IPHONE_8_0) && __IPHONE_OS_VERSION_MAX_ALLOWED >= __IPHONE_8_0
 #define JUCE_USE_NEW_IOS_ALERTWINDOW 1
#endif

#if ! JUCE_USE_NEW_IOS_ALERTWINDOW
    } // (juce namespace)

    @interface JuceAlertBoxDelegate  : NSObject <UIAlertViewDelegate>
    {
    @public
        iOSMessageBox* owner;
    }

    - (void) alertView: (UIAlertView*) alertView clickedButtonAtIndex: (NSInteger) buttonIndex;

    @end

    namespace juce
    {
#endif


class iOSMessageBox
{
public:
    iOSMessageBox (const String& title, const String& message,
                   NSString* button1, NSString* button2, NSString* button3,
                   ModalComponentManager::Callback* cb, const bool async)
        : result (0), resultReceived (false), callback (cb), isAsync (async)
    {
       #if JUCE_USE_NEW_IOS_ALERTWINDOW
        if (currentlyFocusedPeer != nullptr)
        {
            UIAlertController* alert = [UIAlertController alertControllerWithTitle: juceStringToNS (title)
                                                                           message: juceStringToNS (message)
                                                                    preferredStyle: UIAlertControllerStyleAlert];
            addButton (alert, button1, 0);
            addButton (alert, button2, 1);
            addButton (alert, button3, 2);

            [currentlyFocusedPeer->controller presentViewController: alert
                                                           animated: YES
                                                         completion: nil];
        }
        else
        {
            // Since iOS8, alert windows need to be associated with a window, so you need to
            // have at least one window on screen when you use this
            jassertfalse;
        }

       #else
        delegate = [[JuceAlertBoxDelegate alloc] init];
        delegate->owner = this;

        alert = [[UIAlertView alloc] initWithTitle: juceStringToNS (title)
                                           message: juceStringToNS (message)
                                          delegate: delegate
                                 cancelButtonTitle: button1
                                 otherButtonTitles: button2, button3, nil];
        [alert retain];
        [alert show];
       #endif
    }

    ~iOSMessageBox()
    {
       #if ! JUCE_USE_NEW_IOS_ALERTWINDOW
        [alert release];
        [delegate release];
       #endif
    }

    int getResult()
    {
        jassert (callback == nullptr);

        JUCE_AUTORELEASEPOOL
        {
           #if JUCE_USE_NEW_IOS_ALERTWINDOW
            while (! resultReceived)
           #else
            while (! (alert.hidden || resultReceived))
           #endif
                [[NSRunLoop mainRunLoop] runUntilDate: [NSDate dateWithTimeIntervalSinceNow: 0.01]];
        }

        return result;
    }

    void buttonClicked (const int buttonIndex) noexcept
    {
        result = buttonIndex;
        resultReceived = true;

        if (callback != nullptr)
            callback->modalStateFinished (result);

        if (isAsync)
            delete this;
    }

private:
    int result;
    bool resultReceived;
    ScopedPointer<ModalComponentManager::Callback> callback;
    const bool isAsync;

   #if JUCE_USE_NEW_IOS_ALERTWINDOW
    void addButton (UIAlertController* alert, NSString* text, int index)
    {
        if (text != nil)
            [alert addAction: [UIAlertAction actionWithTitle: text
                                                       style: UIAlertActionStyleDefault
                                                     handler: ^(UIAlertAction*) { this->buttonClicked (index); }]];
    }
   #else
    UIAlertView* alert;
    JuceAlertBoxDelegate* delegate;
   #endif

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (iOSMessageBox)
};


#if ! JUCE_USE_NEW_IOS_ALERTWINDOW
    } // (juce namespace)

    @implementation JuceAlertBoxDelegate

    - (void) alertView: (UIAlertView*) alertView clickedButtonAtIndex: (NSInteger) buttonIndex
    {
        owner->buttonClicked ((int) buttonIndex);
        alertView.hidden = true;
    }

    @end

    namespace juce
    {
#endif


//==============================================================================
#if JUCE_MODAL_LOOPS_PERMITTED
void JUCE_CALLTYPE NativeMessageBox::showMessageBox (AlertWindow::AlertIconType /*iconType*/,
                                                     const String& title, const String& message,
                                                     Component* /*associatedComponent*/)
{
    JUCE_AUTORELEASEPOOL
    {
        iOSMessageBox mb (title, message, @"OK", nil, nil, nullptr, false);
        ignoreUnused (mb.getResult());
    }
}
#endif

void JUCE_CALLTYPE NativeMessageBox::showMessageBoxAsync (AlertWindow::AlertIconType /*iconType*/,
                                                          const String& title, const String& message,
                                                          Component* /*associatedComponent*/,
                                                          ModalComponentManager::Callback* callback)
{
    new iOSMessageBox (title, message, @"OK", nil, nil, callback, true);
}

bool JUCE_CALLTYPE NativeMessageBox::showOkCancelBox (AlertWindow::AlertIconType /*iconType*/,
                                                      const String& title, const String& message,
                                                      Component* /*associatedComponent*/,
                                                      ModalComponentManager::Callback* callback)
{
    ScopedPointer<iOSMessageBox> mb (new iOSMessageBox (title, message, @"Cancel", @"OK",
                                                        nil, callback, callback != nullptr));

    if (callback == nullptr)
        return mb->getResult() == 1;

    mb.release();
    return false;
}

int JUCE_CALLTYPE NativeMessageBox::showYesNoCancelBox (AlertWindow::AlertIconType /*iconType*/,
                                                        const String& title, const String& message,
                                                        Component* /*associatedComponent*/,
                                                        ModalComponentManager::Callback* callback)
{
    ScopedPointer<iOSMessageBox> mb (new iOSMessageBox (title, message, @"Cancel", @"Yes", @"No", callback, callback != nullptr));

    if (callback == nullptr)
        return mb->getResult();

    mb.release();
    return 0;
}

int JUCE_CALLTYPE NativeMessageBox::showYesNoBox (AlertWindow::AlertIconType /*iconType*/,
                                                  const String& title, const String& message,
                                                  Component* /*associatedComponent*/,
                                                  ModalComponentManager::Callback* callback)
{
    ScopedPointer<iOSMessageBox> mb (new iOSMessageBox (title, message, @"No", @"Yes", nil, callback, callback != nullptr));

    if (callback == nullptr)
        return mb->getResult();

    mb.release();
    return 0;
}

//==============================================================================
bool DragAndDropContainer::performExternalDragDropOfFiles (const StringArray&, bool, Component*)
{
    jassertfalse;    // no such thing on iOS!
    return false;
}

bool DragAndDropContainer::performExternalDragDropOfText (const String&, Component*)
{
    jassertfalse;    // no such thing on iOS!
    return false;
}

//==============================================================================
void Desktop::setScreenSaverEnabled (const bool isEnabled)
{
    if (! SystemStats::isRunningInAppExtensionSandbox())
        [[UIApplication sharedApplication] setIdleTimerDisabled: ! isEnabled];
}

bool Desktop::isScreenSaverEnabled()
{
    if (SystemStats::isRunningInAppExtensionSandbox())
        return true;

    return ! [[UIApplication sharedApplication] isIdleTimerDisabled];
}

//==============================================================================
bool juce_areThereAnyAlwaysOnTopWindows()
{
    return false;
}

//==============================================================================
Image juce_createIconForFile (const File&)
{
    return Image();
}

//==============================================================================
void SystemClipboard::copyTextToClipboard (const String& text)
{
    [[UIPasteboard generalPasteboard] setValue: juceStringToNS (text)
                             forPasteboardType: @"public.text"];
}

String SystemClipboard::getTextFromClipboard()
{
    return nsStringToJuce ([[UIPasteboard generalPasteboard] valueForPasteboardType: @"public.text"]);
}

//==============================================================================
bool MouseInputSource::SourceList::addSource()
{
    addSource (sources.size(), MouseInputSource::InputSourceType::touch);
    return true;
}

bool MouseInputSource::SourceList::canUseTouch()
{
    return true;
}

bool Desktop::canUseSemiTransparentWindows() noexcept
{
    return true;
}

Point<float> MouseInputSource::getCurrentRawMousePosition()
{
    return juce_lastMousePos;
}

void MouseInputSource::setRawMousePosition (Point<float>)
{
}

double Desktop::getDefaultMasterScale()
{
    return 1.0;
}

Desktop::DisplayOrientation Desktop::getCurrentOrientation() const
{
    UIInterfaceOrientation orientation = SystemStats::isRunningInAppExtensionSandbox() ? UIInterfaceOrientationPortrait
                                                                                       : [[UIApplication sharedApplication] statusBarOrientation];

    return Orientations::convertToJuce (orientation);
}

void Desktop::Displays::findDisplays (float masterScale)
{
    JUCE_AUTORELEASEPOOL
    {
        UIScreen* s = [UIScreen mainScreen];

        Display d;
        d.userArea = d.totalArea = UIViewComponentPeer::realScreenPosToRotated (convertToRectInt ([s bounds])) / masterScale;
        d.isMain = true;
        d.scale = masterScale;

        if ([s respondsToSelector: @selector (scale)])
            d.scale *= s.scale;

        d.dpi = 160 * d.scale;

        displays.add (d);
    }
}

} // namespace juce
