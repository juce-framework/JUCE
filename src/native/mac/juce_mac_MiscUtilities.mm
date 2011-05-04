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

// (This file gets included by juce_mac_NativeCode.mm, rather than being
// compiled on its own).
#if JUCE_INCLUDED_FILE


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
    NSBeep();
}

//==============================================================================
void PlatformUtilities::addItemToDock (const File& file)
{
    // check that it's not already there...
    if (! juce_getOutputFromCommand ("defaults read com.apple.dock persistent-apps")
            .containsIgnoreCase (file.getFullPathName()))
    {
        juce_runSystemCommand ("defaults write com.apple.dock persistent-apps -array-add \"<dict><key>tile-data</key><dict><key>file-data</key><dict><key>_CFURLString</key><string>"
                                 + file.getFullPathName() + "</string><key>_CFURLStringType</key><integer>0</integer></dict></dict></dict>\"");

        juce_runSystemCommand ("osascript -e \"tell application \\\"Dock\\\" to quit\"");
    }
}


//==============================================================================
#if ! JUCE_ONLY_BUILD_CORE_LIBRARY

//==============================================================================
class OSXMessageBox  : public AsyncUpdater
{
public:
    OSXMessageBox (AlertWindow::AlertIconType iconType_,
                   const String& title_, const String& message_,
                   NSString* button1_, NSString* button2_, NSString* button3_,
                   ModalComponentManager::Callback* callback_,
                   const bool runAsync)
        : iconType (iconType_), title (title_),
          message (message_), callback (callback_),
          button1 ([button1_ retain]),
          button2 ([button2_ retain]),
          button3 ([button3_ retain])
    {
        if (runAsync)
            triggerAsyncUpdate();
    }

    ~OSXMessageBox()
    {
        [button1 release];
        [button2 release];
        [button3 release];
    }

    int getResult() const
    {
        JUCE_AUTORELEASEPOOL
        NSInteger r = getRawResult();
        return r == NSAlertDefaultReturn ? 1 : (r == NSAlertOtherReturn ? 2 : 0);
    }

    void handleAsyncUpdate()
    {
        const int result = getResult();

        if (callback != nullptr)
            callback->modalStateFinished (result);

        delete this;
    }

private:
    AlertWindow::AlertIconType iconType;
    String title, message;
    ModalComponentManager::Callback* callback;
    NSString* button1;
    NSString* button2;
    NSString* button3;

    NSInteger getRawResult() const
    {
        NSString* messageString = juceStringToNS (message);
        NSString* titleString = juceStringToNS (title);

        switch (iconType)
        {
            case AlertWindow::InfoIcon:     return NSRunInformationalAlertPanel (titleString, messageString, button1, button2, button3);
            case AlertWindow::WarningIcon:  return NSRunCriticalAlertPanel      (titleString, messageString, button1, button2, button3);
            default:                        return NSRunAlertPanel              (titleString, messageString, button1, button2, button3);
        }
    }
};


void JUCE_CALLTYPE NativeMessageBox::showMessageBox (AlertWindow::AlertIconType iconType,
                                                     const String& title, const String& message,
                                                     Component* associatedComponent)
{
    OSXMessageBox box (iconType, title, message, @"OK", nil, nil, 0, false);
    (void) box.getResult();
}

void JUCE_CALLTYPE NativeMessageBox::showMessageBoxAsync (AlertWindow::AlertIconType iconType,
                                                          const String& title, const String& message,
                                                          Component* associatedComponent)
{
    new OSXMessageBox (iconType, title, message, @"OK", nil, nil, 0, true);
}

bool JUCE_CALLTYPE NativeMessageBox::showOkCancelBox (AlertWindow::AlertIconType iconType,
                                                      const String& title, const String& message,
                                                      Component* associatedComponent,
                                                      ModalComponentManager::Callback* callback)
{
    ScopedPointer<OSXMessageBox> mb (new OSXMessageBox (iconType, title, message,
                                                        @"OK", @"Cancel", nil, callback, callback != nullptr));
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
    ScopedPointer<OSXMessageBox> mb (new OSXMessageBox (iconType, title, message,
                                                        @"Yes", @"Cancel", @"No", callback, callback != nullptr));
    if (callback == nullptr)
        return mb->getResult();

    mb.release();
    return 0;
}


//==============================================================================
bool DragAndDropContainer::performExternalDragDropOfFiles (const StringArray& files, const bool /*canMoveFiles*/)
{
    if (files.size() == 0)
        return false;

    MouseInputSource* draggingSource = Desktop::getInstance().getDraggingMouseSource(0);

    if (draggingSource == nullptr)
    {
        jassertfalse;  // This method must be called in response to a component's mouseDown or mouseDrag event!
        return false;
    }

    Component* sourceComp = draggingSource->getComponentUnderMouse();

    if (sourceComp == nullptr)
    {
        jassertfalse;  // This method must be called in response to a component's mouseDown or mouseDrag event!
        return false;
    }

    JUCE_AUTORELEASEPOOL

    NSView* view = (NSView*) sourceComp->getWindowHandle();

    if (view == nil)
        return false;

    NSPasteboard* pboard = [NSPasteboard pasteboardWithName: NSDragPboard];
    [pboard declareTypes: [NSArray arrayWithObject: NSFilenamesPboardType]
                   owner: nil];

    NSMutableArray* filesArray = [NSMutableArray arrayWithCapacity: 4];
    for (int i = 0; i < files.size(); ++i)
        [filesArray addObject: juceStringToNS (files[i])];

    [pboard setPropertyList: filesArray
                    forType: NSFilenamesPboardType];

    NSPoint dragPosition = [view convertPoint: [[[view window] currentEvent] locationInWindow]
                                     fromView: nil];
    dragPosition.x -= 16;
    dragPosition.y -= 16;

    [view dragImage: [[NSWorkspace sharedWorkspace] iconForFile: juceStringToNS (files[0])]
                 at: dragPosition
             offset: NSMakeSize (0, 0)
              event: [[view window] currentEvent]
         pasteboard: pboard
             source: view
          slideBack: YES];

    return true;
}

bool DragAndDropContainer::performExternalDragDropOfText (const String& /*text*/)
{
    jassertfalse;    // not implemented!
    return false;
}

//==============================================================================
bool Desktop::canUseSemiTransparentWindows() noexcept
{
    return true;
}

const Point<int> MouseInputSource::getCurrentMousePosition()
{
    JUCE_AUTORELEASEPOOL
    const NSPoint p ([NSEvent mouseLocation]);
    return Point<int> (roundToInt (p.x), roundToInt ([[[NSScreen screens] objectAtIndex: 0] frame].size.height - p.y));
}

void Desktop::setMousePosition (const Point<int>& newPosition)
{
    // this rubbish needs to be done around the warp call, to avoid causing a
    // bizarre glitch..
    CGAssociateMouseAndMouseCursorPosition (false);
    CGWarpMouseCursorPosition (CGPointMake (newPosition.getX(), newPosition.getY()));
    CGAssociateMouseAndMouseCursorPosition (true);
}

Desktop::DisplayOrientation Desktop::getCurrentOrientation() const
{
    return upright;
}

//==============================================================================
#if MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_5
class ScreenSaverDefeater   : public Timer,
                              public DeletedAtShutdown
{
public:
    ScreenSaverDefeater()
    {
        startTimer (10000);
        timerCallback();
    }

    void timerCallback()
    {
        if (Process::isForegroundProcess())
            UpdateSystemActivity (UsrActivity);
    }
};

static ScreenSaverDefeater* screenSaverDefeater = nullptr;

void Desktop::setScreenSaverEnabled (const bool isEnabled)
{
    if (isEnabled)
        deleteAndZero (screenSaverDefeater);
    else if (screenSaverDefeater == nullptr)
        screenSaverDefeater = new ScreenSaverDefeater();
}

bool Desktop::isScreenSaverEnabled()
{
    return screenSaverDefeater == 0;
}

#else
//==============================================================================
static IOPMAssertionID screenSaverDisablerID = 0;

void Desktop::setScreenSaverEnabled (const bool isEnabled)
{
    if (isEnabled)
    {
        if (screenSaverDisablerID != 0)
        {
            IOPMAssertionRelease (screenSaverDisablerID);
            screenSaverDisablerID = 0;
        }
    }
    else
    {
        if (screenSaverDisablerID == 0)
        {
           #if defined (MAC_OS_X_VERSION_10_6) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_6
            IOPMAssertionCreateWithName (kIOPMAssertionTypeNoIdleSleep, kIOPMAssertionLevelOn,
                                         CFSTR ("Juce"), &screenSaverDisablerID);
           #else
            IOPMAssertionCreate (kIOPMAssertionTypeNoIdleSleep, kIOPMAssertionLevelOn,
                                 &screenSaverDisablerID);
           #endif
        }
    }
}

bool Desktop::isScreenSaverEnabled()
{
    return screenSaverDisablerID == 0;
}

#endif

//==============================================================================
class DisplaySettingsChangeCallback  : public DeletedAtShutdown
{
public:
    DisplaySettingsChangeCallback()
    {
        CGDisplayRegisterReconfigurationCallback (displayReconfigurationCallBack, 0);
    }

    ~DisplaySettingsChangeCallback()
    {
        CGDisplayRemoveReconfigurationCallback (displayReconfigurationCallBack, 0);
        clearSingletonInstance();
    }

    static void displayReconfigurationCallBack (CGDirectDisplayID, CGDisplayChangeSummaryFlags, void*)
    {
        Desktop::getInstance().refreshMonitorSizes();
    }

    juce_DeclareSingleton_SingleThreaded_Minimal (DisplaySettingsChangeCallback);

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DisplaySettingsChangeCallback);
};

juce_ImplementSingleton_SingleThreaded (DisplaySettingsChangeCallback);

void Desktop::getCurrentMonitorPositions (Array <Rectangle<int> >& monitorCoords, const bool clipToWorkArea)
{
    JUCE_AUTORELEASEPOOL

    DisplaySettingsChangeCallback::getInstance();

    monitorCoords.clear();
    NSArray* screens = [NSScreen screens];
    const CGFloat mainScreenBottom = [[[NSScreen screens] objectAtIndex: 0] frame].size.height;

    for (unsigned int i = 0; i < [screens count]; ++i)
    {
        NSScreen* s = (NSScreen*) [screens objectAtIndex: i];

        NSRect r = clipToWorkArea ? [s visibleFrame]
                                  : [s frame];

        r.origin.y = mainScreenBottom - (r.origin.y + r.size.height);

        monitorCoords.add (convertToRectInt (r));
    }

    jassert (monitorCoords.size() > 0);
}

#endif
#endif
