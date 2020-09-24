/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

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
        std::unique_ptr<OSXMessageBox> mb (new OSXMessageBox (iconType, title, message, b1, b2, b3,
                                                              callback, runAsync));
        if (! runAsync)
            return mb->getResult();

        mb.release();
        return 0;
    }

private:
    AlertWindow::AlertIconType iconType;
    String title, message;
    std::unique_ptr<ModalComponentManager::Callback> callback;
    const char* button1;
    const char* button2;
    const char* button3;

    void handleAsyncUpdate() override
    {
        auto result = getResult();

        if (callback != nullptr)
            callback->modalStateFinished (result);

        delete this;
    }

    NSInteger getRawResult() const
    {
        NSAlert* alert = [[[NSAlert alloc] init] autorelease];

        [alert setMessageText:     juceStringToNS (title)];
        [alert setInformativeText: juceStringToNS (message)];

        [alert setAlertStyle: iconType == AlertWindow::WarningIcon ? NSAlertStyleCritical
                                                                   : NSAlertStyleInformational];
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

int JUCE_CALLTYPE NativeMessageBox::showYesNoBox (AlertWindow::AlertIconType iconType,
                                                  const String& title, const String& message,
                                                  Component* /*associatedComponent*/,
                                                  ModalComponentManager::Callback* callback)
{
    return OSXMessageBox::show (iconType, title, message, callback,
                                "Yes", "No", nullptr, callback != nullptr);
}


//==============================================================================
static NSRect getDragRect (NSView* view, NSEvent* event)
{
    auto eventPos = [event locationInWindow];

    return [view convertRect: NSMakeRect (eventPos.x - 16.0f, eventPos.y - 16.0f, 32.0f, 32.0f)
                    fromView: nil];
}

static NSView* getNSViewForDragEvent (Component* sourceComp)
{
    if (sourceComp == nullptr)
        if (auto* draggingSource = Desktop::getInstance().getDraggingMouseSource (0))
            sourceComp = draggingSource->getComponentUnderMouse();

    if (sourceComp != nullptr)
        return (NSView*) sourceComp->getWindowHandle();

    jassertfalse;  // This method must be called in response to a component's mouseDown or mouseDrag event!
    return nil;
}

struct NSDraggingSourceHelper   : public ObjCClass<NSObject<NSDraggingSource>>
{
    NSDraggingSourceHelper() : ObjCClass<NSObject<NSDraggingSource>> ("JUCENSDraggingSourceHelper_")
    {
        addIvar<std::function<void()>*> ("callback");
        addIvar<String*> ("text");
        addIvar<NSDragOperation*> ("operation");

        addMethod (@selector (dealloc), dealloc, "v@:");
        addMethod (@selector (pasteboard:item:provideDataForType:), provideDataForType, "v@:@@@");

        addMethod (@selector (draggingSession:sourceOperationMaskForDraggingContext:), sourceOperationMaskForDraggingContext, "c@:@@");
        addMethod (@selector (draggingSession:endedAtPoint:operation:), draggingSessionEnded, "v@:@@@");

        addProtocol (@protocol (NSPasteboardItemDataProvider));

        registerClass();
    }

    static void setText (id self, const String& text)
    {
        object_setInstanceVariable (self, "text", new String (text));
    }

    static void setCompletionCallback (id self, std::function<void()> cb)
    {
        object_setInstanceVariable (self, "callback", new std::function<void()> (cb));
    }

    static void setDragOperation (id self, NSDragOperation op)
    {
        object_setInstanceVariable (self, "operation", new NSDragOperation (op));
    }

private:
    static void dealloc (id self, SEL)
    {
        delete getIvar<String*> (self, "text");
        delete getIvar<std::function<void()>*> (self, "callback");
        delete getIvar<NSDragOperation*> (self, "operation");

        sendSuperclassMessage (self, @selector (dealloc));
    }

    static void provideDataForType (id self, SEL, NSPasteboard* sender, NSPasteboardItem*, NSString* type)
    {
        if ([type compare: NSPasteboardTypeString] == NSOrderedSame)
            if (auto* text = getIvar<String*> (self, "text"))
                [sender setData: [juceStringToNS (*text) dataUsingEncoding: NSUTF8StringEncoding]
                        forType: NSPasteboardTypeString];
    }

    static NSDragOperation sourceOperationMaskForDraggingContext (id self, SEL, NSDraggingSession*, NSDraggingContext)
    {
        return *getIvar<NSDragOperation*> (self, "operation");
    }

    static void draggingSessionEnded (id self, SEL, NSDraggingSession*, NSPoint p, NSDragOperation)
    {
        // Our view doesn't receive a mouse up when the drag ends so we need to generate one here and send it...
        if (auto* view = getNSViewForDragEvent (nullptr))
            if (auto* cgEvent = CGEventCreateMouseEvent (nullptr, kCGEventLeftMouseUp, CGPointMake (p.x, p.y), kCGMouseButtonLeft))
                if (id e = [NSEvent eventWithCGEvent: cgEvent])
                    [view mouseUp: e];

        if (auto* cb = getIvar<std::function<void()>*> (self, "callback"))
            cb->operator()();
    }
};

static NSDraggingSourceHelper draggingSourceHelper;

bool DragAndDropContainer::performExternalDragDropOfText (const String& text, Component* sourceComponent,
                                                          std::function<void()> callback)
{
    if (text.isEmpty())
        return false;

    if (auto* view = getNSViewForDragEvent (sourceComponent))
    {
        JUCE_AUTORELEASEPOOL
        {
            if (auto event = [[view window] currentEvent])
            {
                id helper = [draggingSourceHelper.createInstance() init];
                NSDraggingSourceHelper::setText (helper, text);
                NSDraggingSourceHelper::setDragOperation (helper, NSDragOperationCopy);

                if (callback != nullptr)
                    NSDraggingSourceHelper::setCompletionCallback (helper, callback);

                auto pasteboardItem = [[NSPasteboardItem new] autorelease];
                [pasteboardItem setDataProvider: helper
                                       forTypes: [NSArray arrayWithObjects: NSPasteboardTypeString, nil]];

                auto dragItem = [[[NSDraggingItem alloc] initWithPasteboardWriter: pasteboardItem] autorelease];

                NSImage* image = [[NSWorkspace sharedWorkspace] iconForFile: nsEmptyString()];
                [dragItem setDraggingFrame: getDragRect (view, event) contents: image];

                if (auto session = [view beginDraggingSessionWithItems: [NSArray arrayWithObject: dragItem]
                                                                 event: event
                                                                source: helper])
                {
                    session.animatesToStartingPositionsOnCancelOrFail = YES;
                    session.draggingFormation = NSDraggingFormationNone;

                    return true;
                }
            }
        }
    }

    return false;
}

bool DragAndDropContainer::performExternalDragDropOfFiles (const StringArray& files, bool canMoveFiles,
                                                           Component* sourceComponent, std::function<void()> callback)
{
    if (files.isEmpty())
        return false;

    if (auto* view = getNSViewForDragEvent (sourceComponent))
    {
        JUCE_AUTORELEASEPOOL
        {
            if (auto event = [[view window] currentEvent])
            {
                auto dragItems = [[[NSMutableArray alloc] init] autorelease];

                for (auto& filename : files)
                {
                    auto* nsFilename = juceStringToNS (filename);
                    auto fileURL = [NSURL fileURLWithPath: nsFilename];
                    auto dragItem = [[NSDraggingItem alloc] initWithPasteboardWriter: fileURL];

                    auto eventPos = [event locationInWindow];
                    auto dragRect = [view convertRect: NSMakeRect (eventPos.x - 16.0f, eventPos.y - 16.0f, 32.0f, 32.0f)
                                             fromView: nil];
                    auto dragImage = [[NSWorkspace sharedWorkspace] iconForFile: nsFilename];
                    [dragItem setDraggingFrame: dragRect
                                      contents: dragImage];

                    [dragItems addObject: dragItem];
                    [dragItem release];
                }

                auto helper = [draggingSourceHelper.createInstance() autorelease];

                if (callback != nullptr)
                    NSDraggingSourceHelper::setCompletionCallback (helper, callback);

                NSDraggingSourceHelper::setDragOperation (helper, canMoveFiles ? NSDragOperationMove
                                                                               : NSDragOperationCopy);

                return [view beginDraggingSessionWithItems: dragItems
                                                     event: event
                                                    source: helper] != nullptr;
            }
        }
    }

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
        auto p = [NSEvent mouseLocation];
        return { (float) p.x, (float) (getMainScreenHeight() - p.y) };
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
class ScreenSaverDefeater   : public Timer
{
public:
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
                assertion.reset (new PMAssertion());
        }
        else
        {
            assertion.reset();
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
            jassert (res == kIOReturnSuccess); ignoreUnused (res);
        }

        ~PMAssertion()
        {
            if (assertionID != kIOPMNullAssertionID)
                IOPMAssertionRelease (assertionID);
        }

        IOPMAssertionID assertionID;
    };

    std::unique_ptr<PMAssertion> assertion;
};

static std::unique_ptr<ScreenSaverDefeater> screenSaverDefeater;

void Desktop::setScreenSaverEnabled (const bool isEnabled)
{
    if (isEnabled)
        screenSaverDefeater.reset();
    else if (screenSaverDefeater == nullptr)
        screenSaverDefeater.reset (new ScreenSaverDefeater());
}

bool Desktop::isScreenSaverEnabled()
{
    return screenSaverDefeater == nullptr;
}

//==============================================================================
struct DisplaySettingsChangeCallback  : private DeletedAtShutdown
{
    DisplaySettingsChangeCallback()
    {
        CGDisplayRegisterReconfigurationCallback (displayReconfigurationCallBack, nullptr);
    }

    ~DisplaySettingsChangeCallback()
    {
        CGDisplayRemoveReconfigurationCallback (displayReconfigurationCallBack, nullptr);
        clearSingletonInstance();
    }

    static void displayReconfigurationCallBack (CGDirectDisplayID, CGDisplayChangeSummaryFlags, void*)
    {
        const_cast<Displays&> (Desktop::getInstance().getDisplays()).refresh();
    }

    JUCE_DECLARE_SINGLETON_SINGLETHREADED_MINIMAL (DisplaySettingsChangeCallback)

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DisplaySettingsChangeCallback)
};

JUCE_IMPLEMENT_SINGLETON (DisplaySettingsChangeCallback)

static Rectangle<int> convertDisplayRect (NSRect r, CGFloat mainScreenBottom)
{
    r.origin.y = mainScreenBottom - (r.origin.y + r.size.height);
    return convertToRectInt (r);
}

static Displays::Display getDisplayFromScreen (NSScreen* s, CGFloat& mainScreenBottom, const float masterScale)
{
    Displays::Display d;

    d.isMain = (mainScreenBottom == 0);

    if (d.isMain)
        mainScreenBottom = [s frame].size.height;

    d.userArea  = convertDisplayRect ([s visibleFrame], mainScreenBottom) / masterScale;
    d.totalArea = convertDisplayRect ([s frame], mainScreenBottom) / masterScale;
    d.scale = masterScale;

    if ([s respondsToSelector: @selector (backingScaleFactor)])
        d.scale *= s.backingScaleFactor;

    NSSize dpi = [[[s deviceDescription] objectForKey: NSDeviceResolution] sizeValue];
    d.dpi = (dpi.width + dpi.height) / 2.0;

    return d;
}

void Displays::findDisplays (const float masterScale)
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
static void selectImageForDrawing (const Image& image)
{
    [NSGraphicsContext saveGraphicsState];

   #if (defined (MAC_OS_X_VERSION_10_10) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_10)
    [NSGraphicsContext setCurrentContext: [NSGraphicsContext graphicsContextWithCGContext: juce_getImageContext (image)
                                                                                  flipped: false]];
   #else
    [NSGraphicsContext setCurrentContext: [NSGraphicsContext graphicsContextWithGraphicsPort: juce_getImageContext (image)
                                                                                     flipped: false]];
   #endif
}

static void releaseImageAfterDrawing()
{
    [[NSGraphicsContext currentContext] flushGraphics];
    [NSGraphicsContext restoreGraphicsState];
}

Image juce_createIconForFile (const File& file)
{
    JUCE_AUTORELEASEPOOL
    {
        NSImage* image = [[NSWorkspace sharedWorkspace] iconForFile: juceStringToNS (file.getFullPathName())];

        Image result (Image::ARGB, (int) [image size].width, (int) [image size].height, true);

        selectImageForDrawing (result);
        [image drawAtPoint: NSMakePoint (0, 0)
                  fromRect: NSMakeRect (0, 0, [image size].width, [image size].height)
                 operation: NSCompositingOperationSourceOver fraction: 1.0f];
        releaseImageAfterDrawing();

        return result;
    }
}

static Image createNSWindowSnapshot (NSWindow* nsWindow)
{
    JUCE_AUTORELEASEPOOL
    {
        CGImageRef screenShot = CGWindowListCreateImage (CGRectNull,
                                                         kCGWindowListOptionIncludingWindow,
                                                         (CGWindowID) [nsWindow windowNumber],
                                                         kCGWindowImageBoundsIgnoreFraming);

        NSBitmapImageRep* bitmapRep = [[NSBitmapImageRep alloc] initWithCGImage: screenShot];

        Image result (Image::ARGB, (int) [bitmapRep size].width, (int) [bitmapRep size].height, true);

        selectImageForDrawing (result);
        [bitmapRep drawAtPoint: NSMakePoint (0, 0)];
        releaseImageAfterDrawing();

        [bitmapRep release];
        CGImageRelease (screenShot);

        return result;
    }
}

Image createSnapshotOfNativeWindow (void*);
Image createSnapshotOfNativeWindow (void* nativeWindowHandle)
{
    if (id windowOrView = (id) nativeWindowHandle)
    {
        if ([windowOrView isKindOfClass: [NSWindow class]])
            return createNSWindowSnapshot ((NSWindow*) windowOrView);

        if ([windowOrView isKindOfClass: [NSView class]])
            return createNSWindowSnapshot ([(NSView*) windowOrView window]);
    }

    return {};
}

//==============================================================================
void SystemClipboard::copyTextToClipboard (const String& text)
{
    NSPasteboard* pb = [NSPasteboard generalPasteboard];

    [pb declareTypes: [NSArray arrayWithObject: NSPasteboardTypeString]
               owner: nil];

    [pb setString: juceStringToNS (text)
          forType: NSPasteboardTypeString];
}

String SystemClipboard::getTextFromClipboard()
{
    return nsStringToJuce ([[NSPasteboard generalPasteboard] stringForType: NSPasteboardTypeString]);
}

void Process::setDockIconVisible (bool isVisible)
{
    ProcessSerialNumber psn { 0, kCurrentProcess };

    OSStatus err = TransformProcessType (&psn, isVisible ? kProcessTransformToForegroundApplication
                                                         : kProcessTransformToUIElementApplication);
    jassert (err == 0);
    ignoreUnused (err);
}

bool Desktop::isOSXDarkModeActive()
{
    return [[[NSUserDefaults standardUserDefaults] stringForKey: nsStringLiteral ("AppleInterfaceStyle")]
                isEqualToString: nsStringLiteral ("Dark")];
}

} // namespace juce
