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

} // (juce namespace)

@interface JuceAppStartupDelegate : NSObject <UIApplicationDelegate>
{
}

- (void) applicationDidFinishLaunching: (UIApplication*) application;
- (void) applicationWillTerminate: (UIApplication*) application;
- (void) applicationDidEnterBackground: (UIApplication*) application;
- (void) applicationWillEnterForeground: (UIApplication*) application;

@end

@implementation JuceAppStartupDelegate

- (void) applicationDidFinishLaunching: (UIApplication*) application
{
    initialiseJuce_GUI();

    JUCEApplication* app = dynamic_cast <JUCEApplication*> (JUCEApplicationBase::createInstance());
    if (! app->initialiseApp())
        exit (0);
}

- (void) applicationWillTerminate: (UIApplication*) application
{
    JUCEApplicationBase::appWillTerminateByForce();
}

- (void) applicationDidEnterBackground: (UIApplication*) application
{
    JUCEApplicationBase* const app = JUCEApplicationBase::getInstance();
    if (app != nullptr)
        app->suspended();
}

- (void) applicationWillEnterForeground: (UIApplication*) application
{
    JUCEApplicationBase* const app = JUCEApplicationBase::getInstance();
    if (app != nullptr)
        app->resumed();
}

@end

namespace juce
{

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

} // (juce namespace)

@interface JuceAlertBoxDelegate  : NSObject
{
@public
    iOSMessageBox* owner;
}

- (void) alertView: (UIAlertView*) alertView clickedButtonAtIndex: (NSInteger) buttonIndex;

@end

namespace juce
{

class iOSMessageBox
{
public:
    iOSMessageBox (const String& title, const String& message,
                   NSString* button1, NSString* button2, NSString* button3,
                   ModalComponentManager::Callback* cb, const bool async)
        : result (0), delegate (nil), alert (nil),
          callback (cb), isYesNo (button3 != nil), isAsync (async)
    {
        delegate = [[JuceAlertBoxDelegate alloc] init];
        delegate->owner = this;

        alert = [[UIAlertView alloc] initWithTitle: juceStringToNS (title)
                                           message: juceStringToNS (message)
                                          delegate: delegate
                                 cancelButtonTitle: button1
                                 otherButtonTitles: button2, button3, nil];
        [alert retain];
        [alert show];
    }

    ~iOSMessageBox()
    {
        [alert release];
        [delegate release];
    }

    int getResult()
    {
        jassert (callback == nullptr);

        JUCE_AUTORELEASEPOOL
        {
            while (! alert.hidden && alert.superview != nil)
                [[NSRunLoop mainRunLoop] runUntilDate: [NSDate dateWithTimeIntervalSinceNow: 0.01]];
        }

        return result;
    }

    void buttonClicked (const int buttonIndex) noexcept
    {
        result = buttonIndex;

        if (callback != nullptr)
            callback->modalStateFinished (result);

        if (isAsync)
            delete this;
    }

private:
    int result;
    JuceAlertBoxDelegate* delegate;
    UIAlertView* alert;
    ModalComponentManager::Callback* callback;
    const bool isYesNo, isAsync;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (iOSMessageBox)
};

} // (juce namespace)

@implementation JuceAlertBoxDelegate

- (void) alertView: (UIAlertView*) alertView clickedButtonAtIndex: (NSInteger) buttonIndex
{
    owner->buttonClicked (buttonIndex);
    alertView.hidden = true;
}

@end

namespace juce
{

//==============================================================================
void JUCE_CALLTYPE NativeMessageBox::showMessageBox (AlertWindow::AlertIconType iconType,
                                                     const String& title, const String& message,
                                                     Component* associatedComponent)
{
    JUCE_AUTORELEASEPOOL
    {
        iOSMessageBox mb (title, message, @"OK", nil, nil, nullptr, false);
        (void) mb.getResult();
    }
}

void JUCE_CALLTYPE NativeMessageBox::showMessageBoxAsync (AlertWindow::AlertIconType iconType,
                                                          const String& title, const String& message,
                                                          Component* associatedComponent,
                                                          ModalComponentManager::Callback* callback)
{
    new iOSMessageBox (title, message, @"OK", nil, nil, callback, true);
}

bool JUCE_CALLTYPE NativeMessageBox::showOkCancelBox (AlertWindow::AlertIconType iconType,
                                                      const String& title, const String& message,
                                                      Component* associatedComponent,
                                                      ModalComponentManager::Callback* callback)
{
    ScopedPointer<iOSMessageBox> mb (new iOSMessageBox (title, message, @"Cancel", @"OK",
                                                        nil, callback, callback != nullptr));

    if (callback == nullptr)
        return mb->getResult() == 1;

    mb.release();
    return false;
}

int JUCE_CALLTYPE NativeMessageBox::showYesNoCancelBox (AlertWindow::AlertIconType iconType,
                                                        const String& title, const String& message,
                                                        Component* associatedComponent,
                                                        ModalComponentManager::Callback* callback)
{
    ScopedPointer<iOSMessageBox> mb (new iOSMessageBox (title, message, @"Cancel", @"Yes", @"No", callback, callback != nullptr));

    if (callback == nullptr)
        return mb->getResult();

    mb.release();
    return 0;
}

//==============================================================================
bool DragAndDropContainer::performExternalDragDropOfFiles (const StringArray& files, const bool canMoveFiles)
{
    jassertfalse;    // no such thing on iOS!
    return false;
}

bool DragAndDropContainer::performExternalDragDropOfText (const String& text)
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
Image juce_createIconForFile (const File& file)
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
    NSString* text = [[UIPasteboard generalPasteboard] valueForPasteboardType: @"public.text"];

    return text == nil ? String::empty
                       : nsStringToJuce (text);
}

//==============================================================================
bool Desktop::addMouseInputSource()
{
    mouseSources.add (new MouseInputSource (mouseSources.size(), false));
    return true;
}

bool Desktop::canUseSemiTransparentWindows() noexcept
{
    return true;
}

Point<int> MouseInputSource::getCurrentMousePosition()
{
    return juce_lastMousePos;
}

void Desktop::setMousePosition (const Point<int>&)
{
}

Desktop::DisplayOrientation Desktop::getCurrentOrientation() const
{
    return Orientations::convertToJuce ([[UIApplication sharedApplication] statusBarOrientation]);
}

void Desktop::Displays::findDisplays()
{
    JUCE_AUTORELEASEPOOL
    {
        UIScreen* s = [UIScreen mainScreen];

        Display d;
        d.userArea  = UIViewComponentPeer::realScreenPosToRotated (convertToRectInt ([s applicationFrame]));
        d.totalArea = UIViewComponentPeer::realScreenPosToRotated (convertToRectInt ([s bounds]));
        d.isMain = true;

        if ([s respondsToSelector: @selector (scale)])
            d.scale = s.scale;
        else
            d.scale = 1.0;

        displays.add (d);
    }
}
