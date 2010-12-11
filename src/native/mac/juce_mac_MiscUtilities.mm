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

bool AlertWindow::showNativeDialogBox (const String& title,
                                       const String& bodyText,
                                       bool isOkCancel)
{
    const ScopedAutoReleasePool pool;
    return NSRunAlertPanel (juceStringToNS (title),
                            juceStringToNS (bodyText),
                            @"Ok",
                            isOkCancel ? @"Cancel" : nil,
                            nil) == NSAlertDefaultReturn;
}

//==============================================================================
bool DragAndDropContainer::performExternalDragDropOfFiles (const StringArray& files, const bool /*canMoveFiles*/)
{
    if (files.size() == 0)
        return false;

    MouseInputSource* draggingSource = Desktop::getInstance().getDraggingMouseSource(0);

    if (draggingSource == 0)
    {
        jassertfalse;  // This method must be called in response to a component's mouseDown or mouseDrag event!
        return false;
    }

    Component* sourceComp = draggingSource->getComponentUnderMouse();

    if (sourceComp == 0)
    {
        jassertfalse;  // This method must be called in response to a component's mouseDown or mouseDrag event!
        return false;
    }

    const ScopedAutoReleasePool pool;

    NSView* view = (NSView*) sourceComp->getWindowHandle();

    if (view == 0)
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
bool Desktop::canUseSemiTransparentWindows() throw()
{
    return true;
}

const Point<int> MouseInputSource::getCurrentMousePosition()
{
    const ScopedAutoReleasePool pool;
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

    ~ScreenSaverDefeater() {}

    void timerCallback()
    {
        if (Process::isForegroundProcess())
            UpdateSystemActivity (UsrActivity);
    }
};

static ScreenSaverDefeater* screenSaverDefeater = 0;

void Desktop::setScreenSaverEnabled (const bool isEnabled)
{
    if (isEnabled)
        deleteAndZero (screenSaverDefeater);
    else if (screenSaverDefeater == 0)
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
void juce_updateMultiMonitorInfo (Array <Rectangle<int> >& monitorCoords, const bool clipToWorkArea)
{
    const ScopedAutoReleasePool pool;
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
