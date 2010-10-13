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


//==============================================================================
END_JUCE_NAMESPACE

@interface JuceAppStartupDelegate : NSObject <UIApplicationDelegate>
{
}

- (void) applicationDidFinishLaunching: (UIApplication*) application;
- (void) applicationWillTerminate: (UIApplication*) application;

@end

@implementation JuceAppStartupDelegate

- (void) applicationDidFinishLaunching: (UIApplication*) application
{
    initialiseJuce_GUI();

    if (! JUCEApplication::createInstance()->initialiseApp (String::empty))
        exit (0);
}

- (void) applicationWillTerminate: (UIApplication*) application
{
    JUCEApplication::appWillTerminateByForce();
}

@end

BEGIN_JUCE_NAMESPACE

int juce_iOSMain (int argc, const char* argv[])
{
    return UIApplicationMain (argc, const_cast<char**> (argv), nil, @"JuceAppStartupDelegate");
}

//==============================================================================
ScopedAutoReleasePool::ScopedAutoReleasePool()
{
    pool = [[NSAutoreleasePool alloc] init];
}

ScopedAutoReleasePool::~ScopedAutoReleasePool()
{
    [((NSAutoreleasePool*) pool) release];
}

//==============================================================================
void PlatformUtilities::beep()
{
    //xxx
    //AudioServicesPlaySystemSound ();
}

//==============================================================================
void PlatformUtilities::addItemToDock (const File& file)
{
}


//==============================================================================
#if ! JUCE_ONLY_BUILD_CORE_LIBRARY

END_JUCE_NAMESPACE

@interface JuceAlertBoxDelegate  : NSObject
{
@public
    bool clickedOk;
}

- (void) alertView: (UIAlertView*) alertView clickedButtonAtIndex: (NSInteger) buttonIndex;

@end

@implementation JuceAlertBoxDelegate

- (void) alertView: (UIAlertView*) alertView clickedButtonAtIndex: (NSInteger) buttonIndex
{
    clickedOk = (buttonIndex == 0);
    alertView.hidden = true;
}

@end

BEGIN_JUCE_NAMESPACE

// (This function is used directly by other bits of code)
bool juce_iPhoneShowModalAlert (const String& title,
                                const String& bodyText,
                                NSString* okButtonText,
                                NSString* cancelButtonText)
{
    const ScopedAutoReleasePool pool;

    JuceAlertBoxDelegate* callback = [[JuceAlertBoxDelegate alloc] init];

    UIAlertView* alert = [[UIAlertView alloc] initWithTitle: juceStringToNS (title)
                                                    message: juceStringToNS (bodyText)
                                                   delegate: callback
                                          cancelButtonTitle: okButtonText
                                          otherButtonTitles: cancelButtonText, nil];
    [alert retain];
    [alert show];

    while (! alert.hidden && alert.superview != nil)
        [[NSRunLoop mainRunLoop] runUntilDate: [NSDate dateWithTimeIntervalSinceNow: 0.01]];

    const bool result = callback->clickedOk;
    [alert release];
    [callback release];

    return result;
}

bool AlertWindow::showNativeDialogBox (const String& title,
                                       const String& bodyText,
                                       bool isOkCancel)
{
    return juce_iPhoneShowModalAlert (title, bodyText,
                                      @"OK",
                                      isOkCancel ? @"Cancel" : nil);
}

//==============================================================================
bool DragAndDropContainer::performExternalDragDropOfFiles (const StringArray& files, const bool canMoveFiles)
{
    jassertfalse;    // no such thing on the iphone!
    return false;
}

bool DragAndDropContainer::performExternalDragDropOfText (const String& text)
{
    jassertfalse;    // no such thing on the iphone!
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


#endif

#endif
