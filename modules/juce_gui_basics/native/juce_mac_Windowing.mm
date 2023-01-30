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
class OSXMessageBox  : private AsyncUpdater
{
public:
    OSXMessageBox (const MessageBoxOptions& opts,
                   std::unique_ptr<ModalComponentManager::Callback>&& c)
        : options (opts), callback (std::move (c))
    {
    }

    int getResult() const
    {
        return convertResult ([getAlert() runModal]);
    }

    using AsyncUpdater::triggerAsyncUpdate;

private:
    static int convertResult (NSModalResponse response)
    {
        switch (response)
        {
            case NSAlertFirstButtonReturn:   return 0;
            case NSAlertSecondButtonReturn:  return 1;
            case NSAlertThirdButtonReturn:   return 2;
            default:                         break;
        }

        jassertfalse;
        return 0;
    }

    void handleAsyncUpdate() override
    {
        if (auto* comp = options.getAssociatedComponent())
        {
            if (auto* peer = comp->getPeer())
            {
                if (auto* view = static_cast<NSView*> (peer->getNativeHandle()))
                {
                    if (auto* window = [view window])
                    {
                        if (@available (macOS 10.9, *))
                        {
                            [getAlert() beginSheetModalForWindow: window completionHandler: ^(NSModalResponse result)
                            {
                                handleModalFinished (result);
                            }];

                            return;
                        }
                    }
                }
            }
        }

        handleModalFinished ([getAlert() runModal]);
    }

    void handleModalFinished (NSModalResponse result)
    {
        if (callback != nullptr)
            callback->modalStateFinished (convertResult (result));

        delete this;
    }

    static void addButton (NSAlert* alert, const String& button)
    {
        if (! button.isEmpty())
            [alert addButtonWithTitle: juceStringToNS (button)];
    }

    NSAlert* getAlert() const
    {
        NSAlert* alert = [[[NSAlert alloc] init] autorelease];

        [alert setMessageText:     juceStringToNS (options.getTitle())];
        [alert setInformativeText: juceStringToNS (options.getMessage())];

        [alert setAlertStyle: options.getIconType() == MessageBoxIconType::WarningIcon ? NSAlertStyleCritical
                                                                                       : NSAlertStyleInformational];

        const auto button1Text = options.getButtonText (0);

        addButton (alert, button1Text.isEmpty() ? "OK" : button1Text);
        addButton (alert, options.getButtonText (1));
        addButton (alert, options.getButtonText (2));

        return alert;
    }

    MessageBoxOptions options;
    std::unique_ptr<ModalComponentManager::Callback> callback;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OSXMessageBox)
};

static int showDialog (const MessageBoxOptions& options,
                       ModalComponentManager::Callback* callbackIn,
                       AlertWindowMappings::MapFn mapFn)
{
   #if JUCE_MODAL_LOOPS_PERMITTED
    if (callbackIn == nullptr)
    {
        jassert (mapFn != nullptr);

        OSXMessageBox messageBox (options, nullptr);
        return mapFn (messageBox.getResult());
    }
   #endif

    auto messageBox = std::make_unique<OSXMessageBox> (options,
                                                       AlertWindowMappings::getWrappedCallback (callbackIn, mapFn));

    messageBox->triggerAsyncUpdate();
    messageBox.release();

    return 0;
}

#if JUCE_MODAL_LOOPS_PERMITTED
void JUCE_CALLTYPE NativeMessageBox::showMessageBox (MessageBoxIconType iconType,
                                                     const String& title, const String& message,
                                                     Component* associatedComponent)
{
    showDialog (MessageBoxOptions()
                 .withIconType (iconType)
                 .withTitle (title)
                 .withMessage (message)
                 .withButton (TRANS("OK"))
                 .withAssociatedComponent (associatedComponent),
                nullptr, AlertWindowMappings::messageBox);
}

int JUCE_CALLTYPE NativeMessageBox::show (const MessageBoxOptions& options)
{
    return showDialog (options, nullptr, AlertWindowMappings::noMapping);
}
#endif

void JUCE_CALLTYPE NativeMessageBox::showMessageBoxAsync (MessageBoxIconType iconType,
                                                          const String& title, const String& message,
                                                          Component* associatedComponent,
                                                          ModalComponentManager::Callback* callback)
{
    showDialog (MessageBoxOptions()
                  .withIconType (iconType)
                  .withTitle (title)
                  .withMessage (message)
                  .withButton (TRANS("OK"))
                  .withAssociatedComponent (associatedComponent),
                callback, AlertWindowMappings::messageBox);
}

bool JUCE_CALLTYPE NativeMessageBox::showOkCancelBox (MessageBoxIconType iconType,
                                                      const String& title, const String& message,
                                                      Component* associatedComponent,
                                                      ModalComponentManager::Callback* callback)
{
    return showDialog (MessageBoxOptions()
                         .withIconType (iconType)
                         .withTitle (title)
                         .withMessage (message)
                         .withButton (TRANS("OK"))
                         .withButton (TRANS("Cancel"))
                         .withAssociatedComponent (associatedComponent),
                       callback, AlertWindowMappings::okCancel) != 0;
}

int JUCE_CALLTYPE NativeMessageBox::showYesNoCancelBox (MessageBoxIconType iconType,
                                                        const String& title, const String& message,
                                                        Component* associatedComponent,
                                                        ModalComponentManager::Callback* callback)
{
    return showDialog (MessageBoxOptions()
                         .withIconType (iconType)
                         .withTitle (title)
                         .withMessage (message)
                         .withButton (TRANS("Yes"))
                         .withButton (TRANS("No"))
                         .withButton (TRANS("Cancel"))
                         .withAssociatedComponent (associatedComponent),
                       callback, AlertWindowMappings::yesNoCancel);
}

int JUCE_CALLTYPE NativeMessageBox::showYesNoBox (MessageBoxIconType iconType,
                                                  const String& title, const String& message,
                                                  Component* associatedComponent,
                                                  ModalComponentManager::Callback* callback)
{
    return showDialog (MessageBoxOptions()
                         .withIconType (iconType)
                         .withTitle (title)
                         .withMessage (message)
                         .withButton (TRANS("Yes"))
                         .withButton (TRANS("No"))
                         .withAssociatedComponent (associatedComponent),
                       callback, AlertWindowMappings::okCancel);
}

void JUCE_CALLTYPE NativeMessageBox::showAsync (const MessageBoxOptions& options,
                                                ModalComponentManager::Callback* callback)
{
    showDialog (options, callback, AlertWindowMappings::noMapping);
}

void JUCE_CALLTYPE NativeMessageBox::showAsync (const MessageBoxOptions& options,
                                                std::function<void (int)> callback)
{
    showAsync (options, ModalCallbackFunction::create (callback));
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

        addMethod (@selector (dealloc), dealloc);
        addMethod (@selector (pasteboard:item:provideDataForType:), provideDataForType);

        addMethod (@selector (draggingSession:sourceOperationMaskForDraggingContext:), sourceOperationMaskForDraggingContext);
        addMethod (@selector (draggingSession:endedAtPoint:operation:), draggingSessionEnded);

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

        sendSuperclassMessage<void> (self, @selector (dealloc));
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
        if (DisplaySettingsChangeCallback::getInstanceWithoutCreating() == nullptr)
            DisplaySettingsChangeCallback::getInstance()->forceDisplayUpdate = [this] { refresh(); };

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
