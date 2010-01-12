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


//==============================================================================
static JUCEApplication* juce_intialisingApp;

END_JUCE_NAMESPACE

@interface JuceAppStartupDelegate : NSObject <UIApplicationDelegate>
{
}

- (void) applicationDidFinishLaunching: (UIApplication*) application;
- (void) applicationWillResignActive: (UIApplication*) application;

@end

@implementation JuceAppStartupDelegate

- (void) applicationDidFinishLaunching: (UIApplication*) application
{
    String dummy;

    if (! juce_intialisingApp->initialiseApp (dummy))
        exit (0);
}

- (void) applicationWillResignActive: (UIApplication*) application
{
    JUCEApplication::shutdownAppAndClearUp();
}

@end

BEGIN_JUCE_NAMESPACE

int juce_IPhoneMain (int argc, char* argv[], JUCEApplication* app)
{
    juce_intialisingApp = app;
    return UIApplicationMain (argc, argv, nil, @"JuceAppStartupDelegate");
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
    jassertfalse    // no such thing on the iphone!
    return false;
}

bool DragAndDropContainer::performExternalDragDropOfText (const String& text)
{
    jassertfalse    // no such thing on the iphone!
    return false;
}

//==============================================================================
void Desktop::setScreenSaverEnabled (const bool isEnabled) throw()
{
    [[UIApplication sharedApplication] setIdleTimerDisabled: ! isEnabled];
}

bool Desktop::isScreenSaverEnabled() throw()
{
    return ! [[UIApplication sharedApplication] isIdleTimerDisabled];
}


//==============================================================================
void juce_updateMultiMonitorInfo (Array <Rectangle>& monitorCoords, const bool clipToWorkArea)
{
    const ScopedAutoReleasePool pool;
    monitorCoords.clear();

    CGRect r = clipToWorkArea ? [[UIScreen mainScreen] applicationFrame]
                              : [[UIScreen mainScreen] bounds];

    monitorCoords.add (Rectangle ((int) r.origin.x,
                                  (int) r.origin.y,
                                  (int) r.size.width,
                                  (int) r.size.height));
}


#endif

#endif
