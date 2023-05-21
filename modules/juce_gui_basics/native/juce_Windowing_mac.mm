/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
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

class NSDraggingSourceHelper   : public ObjCClass<NSObject<NSDraggingSource>>
{
public:
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

    static NSDraggingSourceHelper& get()
    {
        static NSDraggingSourceHelper draggingSourceHelper;
        return draggingSourceHelper;
    }

private:
    NSDraggingSourceHelper()
        : ObjCClass ("JUCENSDraggingSourceHelper_")
    {
        addIvar<std::function<void()>*> ("callback");
        addIvar<String*> ("text");
        addIvar<NSDragOperation*> ("operation");

        addMethod (@selector (dealloc), [] (id self, SEL)
        {
            delete getIvar<String*> (self, "text");
            delete getIvar<std::function<void()>*> (self, "callback");
            delete getIvar<NSDragOperation*> (self, "operation");

            sendSuperclassMessage<void> (self, @selector (dealloc));
        });

        addMethod (@selector (pasteboard:item:provideDataForType:), [] (id self, SEL, NSPasteboard* sender, NSPasteboardItem*, NSString* type)
        {
            if ([type compare: NSPasteboardTypeString] == NSOrderedSame)
                if (auto* text = getIvar<String*> (self, "text"))
                    [sender setData: [juceStringToNS (*text) dataUsingEncoding: NSUTF8StringEncoding]
                            forType: NSPasteboardTypeString];
        });

        addMethod (@selector (draggingSession:sourceOperationMaskForDraggingContext:), [] (id self, SEL, NSDraggingSession*, NSDraggingContext)
        {
            return *getIvar<NSDragOperation*> (self, "operation");
        });

        addMethod (@selector (draggingSession:endedAtPoint:operation:), [] (id self, SEL, NSDraggingSession*, NSPoint p, NSDragOperation)
        {
            // Our view doesn't receive a mouse up when the drag ends so we need to generate one here and send it...
            if (auto* view = getNSViewForDragEvent (nullptr))
                if (auto* cgEvent = CGEventCreateMouseEvent (nullptr, kCGEventLeftMouseUp, CGPointMake (p.x, p.y), kCGMouseButtonLeft))
                    if (id e = [NSEvent eventWithCGEvent: cgEvent])
                        [view mouseUp: e];

            if (auto* cb = getIvar<std::function<void()>*> (self, "callback"))
                cb->operator()();
        });

        addProtocol (@protocol (NSPasteboardItemDataProvider));

        registerClass();
    }
};

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
                id helper = [NSDraggingSourceHelper::get().createInstance() init];
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

                JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wnullable-to-nonnull-conversion")
                if (auto session = [view beginDraggingSessionWithItems: [NSArray arrayWithObject: dragItem]
                                                                 event: event
                                                                source: helper])
                JUCE_END_IGNORE_WARNINGS_GCC_LIKE
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

                auto helper = [NSDraggingSourceHelper::get().createInstance() autorelease];

                if (callback != nullptr)
                    NSDraggingSourceHelper::setCompletionCallback (helper, callback);

                NSDraggingSourceHelper::setDragOperation (helper, canMoveFiles ? NSDragOperationMove
                                                                               : NSDragOperationCopy);

                JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wnullable-to-nonnull-conversion")
                return [view beginDraggingSessionWithItems: dragItems
                                                     event: event
                                                    source: helper] != nullptr;
                JUCE_END_IGNORE_WARNINGS_GCC_LIKE
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

static ComponentPeer* findPeerContainingPoint (Point<float> globalPos)
{
    for (int i = 0; i < juce::ComponentPeer::getNumPeers(); ++i)
    {
        auto* peer = juce::ComponentPeer::getPeer (i);

        if (peer->contains (peer->globalToLocal (globalPos).toInt(), false))
            return peer;
    }

    return nullptr;
}

void MouseInputSource::setRawMousePosition (Point<float> newPosition)
{
    const auto oldPosition = Desktop::getInstance().getMainMouseSource().getRawScreenPosition();

    // this rubbish needs to be done around the warp call, to avoid causing a
    // bizarre glitch..
    CGAssociateMouseAndMouseCursorPosition (false);
    CGWarpMouseCursorPosition (convertToCGPoint (newPosition));
    CGAssociateMouseAndMouseCursorPosition (true);

    // Mouse enter and exit events seem to be always generated as a consequence of programmatically
    // moving the mouse. However, when the mouse stays within the same peer no mouse move event is
    // generated, and we lose track of the correct Component under the mouse. Hence, we need to
    // generate this missing event here.
    if (auto* peer = findPeerContainingPoint (newPosition); peer != nullptr
                                                            && peer == findPeerContainingPoint (oldPosition))
    {
        peer->handleMouseEvent (MouseInputSource::InputSourceType::mouse,
                                peer->globalToLocal (newPosition),
                                ModifierKeys::currentModifiers,
                                0.0f,
                                0.0f,
                                Time::currentTimeMillis());
    }
}

double Desktop::getDefaultMasterScale()
{
    return 1.0;
}

Desktop::DisplayOrientation Desktop::getCurrentOrientation() const
{
    return upright;
}

bool Desktop::isDarkModeActive() const
{
    return [[[NSUserDefaults standardUserDefaults] stringForKey: nsStringLiteral ("AppleInterfaceStyle")]
                isEqualToString: nsStringLiteral ("Dark")];
}

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wundeclared-selector")
static const auto darkModeSelector = @selector (darkModeChanged:);
static const auto keyboardVisibilitySelector = @selector (keyboardVisiblityChanged:);
JUCE_END_IGNORE_WARNINGS_GCC_LIKE

class Desktop::NativeDarkModeChangeDetectorImpl
{
public:
    NativeDarkModeChangeDetectorImpl()
    {
        static DelegateClass delegateClass;
        delegate.reset ([delegateClass.createInstance() init]);
        observer.emplace (delegate.get(),
                          darkModeSelector,
                          @"AppleInterfaceThemeChangedNotification",
                          nil,
                          [NSDistributedNotificationCenter class]);
    }

private:
    struct DelegateClass  : public ObjCClass<NSObject>
    {
        DelegateClass()  : ObjCClass<NSObject> ("JUCEDelegate_")
        {
            addMethod (darkModeSelector, [] (id, SEL, NSNotification*) { Desktop::getInstance().darkModeChanged(); });
            registerClass();
        }
    };

    NSUniquePtr<NSObject> delegate;
    Optional<ScopedNotificationCenterObserver> observer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NativeDarkModeChangeDetectorImpl)
};

std::unique_ptr<Desktop::NativeDarkModeChangeDetectorImpl> Desktop::createNativeDarkModeChangeDetectorImpl()
{
    return std::make_unique<NativeDarkModeChangeDetectorImpl>();
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
            [[maybe_unused]] IOReturn res = IOPMAssertionCreateWithName (kIOPMAssertionTypePreventUserIdleDisplaySleep,
                                                                         kIOPMAssertionLevelOn,
                                                                         CFSTR ("JUCE Playback"),
                                                                         &assertionID);
            jassert (res == kIOReturnSuccess);
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
        CGDisplayRegisterReconfigurationCallback (displayReconfigurationCallback, this);
    }

    ~DisplaySettingsChangeCallback()
    {
        CGDisplayRemoveReconfigurationCallback (displayReconfigurationCallback, this);
        clearSingletonInstance();
    }

    static void displayReconfigurationCallback (CGDirectDisplayID, CGDisplayChangeSummaryFlags, void* userInfo)
    {
        if (auto* thisPtr = static_cast<DisplaySettingsChangeCallback*> (userInfo))
            if (thisPtr->forceDisplayUpdate != nullptr)
                thisPtr->forceDisplayUpdate();
    }

    std::function<void()> forceDisplayUpdate;

    JUCE_DECLARE_SINGLETON (DisplaySettingsChangeCallback, false)
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

    d.isMain = (approximatelyEqual (mainScreenBottom, 0.0));

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
        if (DisplaySettingsChangeCallback::getInstanceWithoutCreating() == nullptr)
            DisplaySettingsChangeCallback::getInstance()->forceDisplayUpdate = [this] { refresh(); };

        CGFloat mainScreenBottom = 0;

        for (NSScreen* s in [NSScreen screens])
            displays.add (getDisplayFromScreen (s, mainScreenBottom, masterScale));
    }
}

//==============================================================================
static void selectImageForDrawing (const Image& image)
{
    [NSGraphicsContext saveGraphicsState];

    if (@available (macOS 10.10, *))
    {
        [NSGraphicsContext setCurrentContext: [NSGraphicsContext graphicsContextWithCGContext: juce_getImageContext (image)
                                                                                      flipped: false]];
        return;
    }

    JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wdeprecated-declarations")
    [NSGraphicsContext setCurrentContext: [NSGraphicsContext graphicsContextWithGraphicsPort: juce_getImageContext (image)
                                                                                     flipped: false]];
    JUCE_END_IGNORE_WARNINGS_GCC_LIKE
}

static void releaseImageAfterDrawing()
{
    [[NSGraphicsContext currentContext] flushGraphics];
    [NSGraphicsContext restoreGraphicsState];
}

Image detail::WindowingHelpers::createIconForFile (const File& file)
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

    [[maybe_unused]] OSStatus err = TransformProcessType (&psn, isVisible ? kProcessTransformToForegroundApplication
                                                                          : kProcessTransformToUIElementApplication);
    jassert (err == 0);
}

} // namespace juce
