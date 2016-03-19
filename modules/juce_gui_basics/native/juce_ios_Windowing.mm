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

extern bool isIOSAppActive;

struct AppInactivityCallback // NB: careful, this declaration is duplicated in other modules
{
    virtual ~AppInactivityCallback() {}
    virtual void appBecomingInactive() = 0;
};

// This is an internal list of callbacks (but currently used between modules)
Array<AppInactivityCallback*> appBecomingInactiveCallbacks;

} // (juce namespace)

@interface JuceAppStartupDelegate : NSObject <UIApplicationDelegate>
{
}

- (void) applicationDidFinishLaunching: (UIApplication*) application;
- (void) applicationWillTerminate: (UIApplication*) application;
- (void) applicationDidEnterBackground: (UIApplication*) application;
- (void) applicationWillEnterForeground: (UIApplication*) application;
- (void) applicationDidBecomeActive: (UIApplication*) application;
- (void) applicationWillResignActive: (UIApplication*) application;

@end

@implementation JuceAppStartupDelegate

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
    ignoreUnused (application);

    if (JUCEApplicationBase* const app = JUCEApplicationBase::getInstance())
        app->suspended();
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

@end

namespace juce
{

int juce_iOSMain (int argc, const char* argv[]);
int juce_iOSMain (int argc, const char* argv[])
{
    return UIApplicationMain (argc, const_cast<char**> (argv), nil, @"JuceAppStartupDelegate");
}

//==============================================================================
void LookAndFeel::playAlertSound()
{
    //xxx
    //AudioServicesPlaySystemSound ();
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

//==============================================================================
bool DragAndDropContainer::performExternalDragDropOfFiles (const StringArray&, bool)
{
    jassertfalse;    // no such thing on iOS!
    return false;
}

bool DragAndDropContainer::performExternalDragDropOfText (const String&)
{
    jassertfalse;    // no such thing on iOS!
    return false;
}

//==============================================================================
void Desktop::setScreenSaverEnabled (const bool isEnabled)
{
    [[UIApplication sharedApplication] setIdleTimerDisabled: ! isEnabled];
}

bool Desktop::isScreenSaverEnabled()
{
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
    return Image::null;
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
    addSource (sources.size(), false);
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
    return Orientations::convertToJuce ([[UIApplication sharedApplication] statusBarOrientation]);
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
