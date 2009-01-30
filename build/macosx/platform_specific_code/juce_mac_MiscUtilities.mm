/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

// (This file gets included by juce_mac_NativeCode.mm, rather than being
// compiled on its own).
#ifdef JUCE_INCLUDED_FILE


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
bool AlertWindow::showNativeDialogBox (const String& title,
                                       const String& bodyText,
                                       bool isOkCancel)
{
    const ScopedAutoReleasePool pool;
    return NSRunAlertPanel (juceStringToNS (title),
                            juceStringToNS (bodyText),
                            @"Ok",
                            isOkCancel ? @"Cancel" : nil,
                            nil) == 0;
}

//==============================================================================
bool DragAndDropContainer::performExternalDragDropOfFiles (const StringArray& files, const bool canMoveFiles)
{
    if (files.size() == 0)
        return false;

    Component* sourceComp = Component::getComponentUnderMouse();

    if (sourceComp == 0)
    {
        jassertfalse  // this method must be called in response to a
                      // component's mouseDrag event!
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

bool DragAndDropContainer::performExternalDragDropOfText (const String& text)
{
    jassertfalse    // not implemented!
    return false;
}

//==============================================================================
bool Desktop::canUseSemiTransparentWindows() throw()
{
    return true;
}

void Desktop::getMousePosition (int& x, int& y) throw()
{
    const ScopedAutoReleasePool pool;
    const NSPoint p ([NSEvent mouseLocation]);
    x = roundFloatToInt (p.x);
    y = roundFloatToInt ([[[NSScreen screens] objectAtIndex: 0] frame].size.height - p.y);
}

void Desktop::setMousePosition (int x, int y) throw()
{
    // this rubbish needs to be done around the warp call, to avoid causing a
    // bizarre glitch..
    CGAssociateMouseAndMouseCursorPosition (false);
    CGSetLocalEventsSuppressionInterval (0);

    CGPoint pos = { x, y };
    CGWarpMouseCursorPosition (pos);

    CGAssociateMouseAndMouseCursorPosition (true);
}

//==============================================================================
#if MACOS_10_4_OR_EARLIER
class ScreenSaverDefeater   : public Timer,
                              public DeletedAtShutdown
{
public:
    ScreenSaverDefeater() throw()
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

void Desktop::setScreenSaverEnabled (const bool isEnabled) throw()
{
    if (isEnabled)
    {
        deleteAndZero (screenSaverDefeater);
    }
    else if (screenSaverDefeater == 0)
    {
        screenSaverDefeater = new ScreenSaverDefeater();
    }
}

bool Desktop::isScreenSaverEnabled() throw()
{
    return screenSaverDefeater == 0;
}

#else
//==============================================================================
static IOPMAssertionID screenSaverDisablerID = 0;

void Desktop::setScreenSaverEnabled (const bool isEnabled) throw()
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
            IOPMAssertionCreate (kIOPMAssertionTypeNoIdleSleep,
                                 kIOPMAssertionLevelOn, &screenSaverDisablerID);
        }
    }
}

bool Desktop::isScreenSaverEnabled() throw()
{
    return screenSaverDisablerID == 0;
}

#endif

//==============================================================================
void juce_updateMultiMonitorInfo (Array <Rectangle>& monitorCoords, const bool clipToWorkArea) throw()
{
    const ScopedAutoReleasePool pool;
    monitorCoords.clear();
    NSArray* screens = [NSScreen screens];
    const float mainScreenBottom = [[[NSScreen screens] objectAtIndex: 0] frame].size.height;

    for (unsigned int i = 0; i < [screens count]; ++i)
    {
        NSScreen* s = (NSScreen*) [screens objectAtIndex: i];

        NSRect r = clipToWorkArea ? [s visibleFrame]
                                  : [s frame];

        monitorCoords.add (Rectangle ((int) r.origin.x,
                                      (int) (mainScreenBottom - (r.origin.y + r.size.height)),
                                      (int) r.size.width,
                                      (int) r.size.height));
    }

    jassert (monitorCoords.size() > 0);
}

#endif
