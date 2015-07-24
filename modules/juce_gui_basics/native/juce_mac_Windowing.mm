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

void LookAndFeel::playAlertSound()
{
    NSBeep();
}

//==============================================================================
class OSXMessageBox  : private AsyncUpdater
{
public:
    OSXMessageBox (AlertWindow::AlertIconType type, const String& t, const String& m,
                   const char* b1, const char* b2, const char* b3,
                   ModalComponentManager::Callback* c, const bool runAsync)
        : iconType (type), title (t), message (m), callback (c),
          button1 (b1), button2 (b2), button3 (b3)
    {
        if (runAsync)
            triggerAsyncUpdate();
    }

    int getResult() const
    {
        switch (getRawResult())
        {
            case NSAlertFirstButtonReturn:  return 1;
            case NSAlertThirdButtonReturn:  return 2;
            default:                        return 0;
        }
    }

    static int show (AlertWindow::AlertIconType iconType, const String& title, const String& message,
                     ModalComponentManager::Callback* callback, const char* b1, const char* b2, const char* b3,
                     bool runAsync)
    {
        ScopedPointer<OSXMessageBox> mb (new OSXMessageBox (iconType, title, message, b1, b2, b3,
                                                            callback, runAsync));
        if (! runAsync)
            return mb->getResult();

        mb.release();
        return 0;
    }

private:
    AlertWindow::AlertIconType iconType;
    String title, message;
    ScopedPointer<ModalComponentManager::Callback> callback;
    const char* button1;
    const char* button2;
    const char* button3;

    void handleAsyncUpdate() override
    {
        const int result = getResult();

        if (callback != nullptr)
            callback->modalStateFinished (result);

        delete this;
    }

    NSInteger getRawResult() const
    {
        NSAlert* alert = [[[NSAlert alloc] init] autorelease];

        [alert setMessageText:     juceStringToNS (title)];
        [alert setInformativeText: juceStringToNS (message)];

        [alert setAlertStyle: iconType == AlertWindow::WarningIcon ? NSCriticalAlertStyle
                                                                   : NSInformationalAlertStyle];
        addButton (alert, button1);
        addButton (alert, button2);
        addButton (alert, button3);

        return [alert runModal];
    }

    static void addButton (NSAlert* alert, const char* button)
    {
        if (button != nullptr)
            [alert addButtonWithTitle: juceStringToNS (TRANS (button))];
    }
};

#if JUCE_MODAL_LOOPS_PERMITTED
void JUCE_CALLTYPE NativeMessageBox::showMessageBox (AlertWindow::AlertIconType iconType,
                                                     const String& title, const String& message,
                                                     Component* /*associatedComponent*/)
{
    OSXMessageBox::show (iconType, title, message, nullptr, "OK", nullptr, nullptr, false);
}
#endif

void JUCE_CALLTYPE NativeMessageBox::showMessageBoxAsync (AlertWindow::AlertIconType iconType,
                                                          const String& title, const String& message,
                                                          Component* /*associatedComponent*/,
                                                          ModalComponentManager::Callback* callback)
{
    OSXMessageBox::show (iconType, title, message, callback, "OK", nullptr, nullptr, true);
}

bool JUCE_CALLTYPE NativeMessageBox::showOkCancelBox (AlertWindow::AlertIconType iconType,
                                                      const String& title, const String& message,
                                                      Component* /*associatedComponent*/,
                                                      ModalComponentManager::Callback* callback)
{
    return OSXMessageBox::show (iconType, title, message, callback,
                                "OK", "Cancel", nullptr, callback != nullptr) == 1;
}

int JUCE_CALLTYPE NativeMessageBox::showYesNoCancelBox (AlertWindow::AlertIconType iconType,
                                                        const String& title, const String& message,
                                                        Component* /*associatedComponent*/,
                                                        ModalComponentManager::Callback* callback)
{
    return OSXMessageBox::show (iconType, title, message, callback,
                                "Yes", "Cancel", "No", callback != nullptr);
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
    {
        if (NSView* view = (NSView*) sourceComp->getWindowHandle())
        {
            if (NSEvent* event = [[view window] currentEvent])
            {
                NSPoint eventPos = [event locationInWindow];
                NSRect dragRect = [view convertRect: NSMakeRect (eventPos.x - 16.0f, eventPos.y - 16.0f, 32.0f, 32.0f)
                                           fromView: nil];

                for (int i = 0; i < files.size(); ++i)
                {
                    if (! [view dragFile: juceStringToNS (files[i])
                                fromRect: dragRect
                               slideBack: YES
                                   event: event])
                        return false;
                }

                return true;
            }
        }
    }

    return false;
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

Point<float> MouseInputSource::getCurrentRawMousePosition()
{
    JUCE_AUTORELEASEPOOL
    {
        const NSPoint p ([NSEvent mouseLocation]);
        return Point<float> ((float) p.x, (float) (getMainScreenHeight() - p.y));
    }
}

void MouseInputSource::setRawMousePosition (Point<float> newPosition)
{
    // this rubbish needs to be done around the warp call, to avoid causing a
    // bizarre glitch..
    CGAssociateMouseAndMouseCursorPosition (false);
    CGWarpMouseCursorPosition (convertToCGPoint (newPosition));
    CGAssociateMouseAndMouseCursorPosition (true);
}

double Desktop::getDefaultMasterScale()
{
    return 1.0;
}

Desktop::DisplayOrientation Desktop::getCurrentOrientation() const
{
    return upright;
}

//==============================================================================
#if defined (MAC_OS_X_VERSION_10_7) && (MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_7)
 #define JUCE_USE_IOPM_SCREENSAVER_DEFEAT 1
#endif

#if ! (defined (JUCE_USE_IOPM_SCREENSAVER_DEFEAT) || defined (__POWER__))
 extern "C"  { extern OSErr UpdateSystemActivity (UInt8); } // Some versions of the SDK omit this function..
#endif

class ScreenSaverDefeater   : public Timer
{
public:
   #if JUCE_USE_IOPM_SCREENSAVER_DEFEAT
    ScreenSaverDefeater()
    {
        startTimer (5000);
        timerCallback();
    }

    void timerCallback() override
    {
        if (Process::isForegroundProcess())
        {
            if (assertion == nullptr)
                assertion = new PMAssertion();
        }
        else
        {
            assertion = nullptr;
        }
    }

    struct PMAssertion
    {
        PMAssertion()  : assertionID (kIOPMNullAssertionID)
        {
            IOReturn res = IOPMAssertionCreateWithName (kIOPMAssertionTypePreventUserIdleDisplaySleep,
                                                        kIOPMAssertionLevelOn,
                                                        CFSTR ("JUCE Playback"),
                                                        &assertionID);
            jassert (res == kIOReturnSuccess); (void) res;
        }

        ~PMAssertion()
        {
            if (assertionID != kIOPMNullAssertionID)
                IOPMAssertionRelease (assertionID);
        }

        IOPMAssertionID assertionID;
    };

    ScopedPointer<PMAssertion> assertion;
   #else
    ScreenSaverDefeater()
    {
        startTimer (10000);
        timerCallback();
    }

    void timerCallback() override
    {
        if (Process::isForegroundProcess())
            UpdateSystemActivity (1 /*UsrActivity*/);
    }
   #endif
};

static ScopedPointer<ScreenSaverDefeater> screenSaverDefeater;

void Desktop::setScreenSaverEnabled (const bool isEnabled)
{
    if (isEnabled)
        screenSaverDefeater = nullptr;
    else if (screenSaverDefeater == nullptr)
        screenSaverDefeater = new ScreenSaverDefeater();
}

bool Desktop::isScreenSaverEnabled()
{
    return screenSaverDefeater == nullptr;
}

//==============================================================================
class DisplaySettingsChangeCallback  : private DeletedAtShutdown
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
        const_cast <Desktop::Displays&> (Desktop::getInstance().getDisplays()).refresh();
    }

    juce_DeclareSingleton_SingleThreaded_Minimal (DisplaySettingsChangeCallback)

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DisplaySettingsChangeCallback)
};

juce_ImplementSingleton_SingleThreaded (DisplaySettingsChangeCallback)

static Rectangle<int> convertDisplayRect (NSRect r, CGFloat mainScreenBottom)
{
    r.origin.y = mainScreenBottom - (r.origin.y + r.size.height);
    return convertToRectInt (r);
}

static Desktop::Displays::Display getDisplayFromScreen (NSScreen* s, CGFloat& mainScreenBottom, const float masterScale)
{
    Desktop::Displays::Display d;

    d.isMain = (mainScreenBottom == 0);

    if (d.isMain)
        mainScreenBottom = [s frame].size.height;

    d.userArea  = convertDisplayRect ([s visibleFrame], mainScreenBottom) / masterScale;
    d.totalArea = convertDisplayRect ([s frame], mainScreenBottom) / masterScale;
    d.scale = masterScale;

   #if defined (MAC_OS_X_VERSION_10_7) && (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_7)
    if ([s respondsToSelector: @selector (backingScaleFactor)])
        d.scale *= s.backingScaleFactor;
   #endif

    NSSize dpi = [[[s deviceDescription] objectForKey: NSDeviceResolution] sizeValue];
    d.dpi = (dpi.width + dpi.height) / 2.0;

    return d;
}

void Desktop::Displays::findDisplays (const float masterScale)
{
    JUCE_AUTORELEASEPOOL
    {
        DisplaySettingsChangeCallback::getInstance();

        CGFloat mainScreenBottom = 0;

        for (NSScreen* s in [NSScreen screens])
            displays.add (getDisplayFromScreen (s, mainScreenBottom, masterScale));
    }
}

//==============================================================================
bool juce_areThereAnyAlwaysOnTopWindows()
{
    for (NSWindow* window in [NSApp windows])
        if ([window level] > NSNormalWindowLevel)
            return true;

    return false;
}

//==============================================================================
Image juce_createIconForFile (const File& file)
{
    JUCE_AUTORELEASEPOOL
    {
        NSImage* image = [[NSWorkspace sharedWorkspace] iconForFile: juceStringToNS (file.getFullPathName())];

        Image result (Image::ARGB, (int) [image size].width, (int) [image size].height, true);

        [NSGraphicsContext saveGraphicsState];
        [NSGraphicsContext setCurrentContext: [NSGraphicsContext graphicsContextWithGraphicsPort: juce_getImageContext (result) flipped: false]];

        [image drawAtPoint: NSMakePoint (0, 0)
                  fromRect: NSMakeRect (0, 0, [image size].width, [image size].height)
                 operation: NSCompositeSourceOver fraction: 1.0f];

        [[NSGraphicsContext currentContext] flushGraphics];
        [NSGraphicsContext restoreGraphicsState];

        return result;
    }
}

//==============================================================================
void SystemClipboard::copyTextToClipboard (const String& text)
{
    NSPasteboard* pb = [NSPasteboard generalPasteboard];

    [pb declareTypes: [NSArray arrayWithObject: NSStringPboardType]
               owner: nil];

    [pb setString: juceStringToNS (text)
          forType: NSStringPboardType];
}

String SystemClipboard::getTextFromClipboard()
{
    NSString* text = [[NSPasteboard generalPasteboard] stringForType: NSStringPboardType];

    return text == nil ? String()
                       : nsStringToJuce (text);
}

void Process::setDockIconVisible (bool isVisible)
{
   #if defined (MAC_OS_X_VERSION_10_6) && (MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_6)
    [NSApp setActivationPolicy: isVisible ? NSApplicationActivationPolicyRegular
                                          : NSApplicationActivationPolicyProhibited];
   #else
    (void) isVisible;
    jassertfalse; // sorry, not available in 10.5!
   #endif
}
