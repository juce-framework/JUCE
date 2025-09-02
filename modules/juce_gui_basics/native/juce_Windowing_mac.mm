/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

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

class NSDraggingSourceHelper final : public ObjCClass<NSObject<NSDraggingSource, NSPasteboardItemDataProvider>>
{
public:
    struct DataMembers
    {
        std::function<void()> callback;
        String text;
        NSDragOperation operation;
        Component::SafePointer<Component> originator;
    };

    static auto* create (DataMembers members)
    {
        static NSDraggingSourceHelper draggingSourceHelper;
        auto* result = [[draggingSourceHelper.createInstance() init] autorelease];
        object_setInstanceVariable (result, "members", new DataMembers (std::move (members)));
        return result;
    }

private:
    static DataMembers* getMembers (id self)
    {
        return getIvar<DataMembers*> (self, "members");
    }

    NSDraggingSourceHelper()
        : ObjCClass ("JUCENSDraggingSourceHelper_")
    {
        addIvar<DataMembers*> ("members");

        addMethod (@selector (dealloc), [] (id self, SEL)
        {
            delete getMembers (self);
            sendSuperclassMessage<void> (self, @selector (dealloc));
        });

        addMethod (@selector (pasteboard:item:provideDataForType:),
                   [] (id self, SEL, NSPasteboard* sender, NSPasteboardItem*, NSString* type)
        {
            if ([type compare: NSPasteboardTypeString] == NSOrderedSame)
                if (auto* members = getMembers (self))
                    [sender setData: [juceStringToNS (members->text) dataUsingEncoding: NSUTF8StringEncoding]
                            forType: NSPasteboardTypeString];
        });

        addMethod (@selector (draggingSession:sourceOperationMaskForDraggingContext:),
                   [] (id self, SEL, NSDraggingSession*, NSDraggingContext) -> NSDragOperation
        {
            if (auto* members = getMembers (self))
                return members->operation;

            return {};
        });

        addMethod (@selector (draggingSession:endedAtPoint:operation:),
                   [] (id self, SEL, NSDraggingSession*, NSPoint p, NSDragOperation)
        {
            // Our view doesn't receive a mouse up when the drag ends so we need to generate one here and send it...
            auto* members = getMembers (self);

            if (members == nullptr)
                return;

            auto* cgEvent = CGEventCreateMouseEvent (nullptr,
                                                     kCGEventLeftMouseUp,
                                                     CGPointMake (p.x, p.y),
                                                     kCGMouseButtonLeft);

            if (cgEvent != nullptr)
                if (id e = [NSEvent eventWithCGEvent: cgEvent])
                    if (auto* view = getNSViewForDragEvent (members->originator))
                        [view mouseUp: e];

            NullCheckedInvocation::invoke (members->callback);
        });

        addProtocol (@protocol (NSPasteboardItemDataProvider));
        addProtocol (@protocol (NSDraggingSource));

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
                auto* helper = NSDraggingSourceHelper::create ({ std::move (callback),
                                                                 text,
                                                                 NSDragOperationCopy,
                                                                 sourceComponent });

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

                auto* helper = NSDraggingSourceHelper::create ({ std::move (callback),
                                                                 "",
                                                                 canMoveFiles ? NSDragOperationMove : NSDragOperationCopy,
                                                                 sourceComponent });

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
    struct DelegateClass final : public ObjCClass<NSObject>
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
class ScreenSaverDefeater final : public Timer
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
struct DisplaySettingsChangeCallback final : private DeletedAtShutdown
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
            NullCheckedInvocation::invoke (thisPtr->forceDisplayUpdate);
    }

    std::function<void()> forceDisplayUpdate;

    JUCE_DECLARE_SINGLETON_INLINE (DisplaySettingsChangeCallback, false)
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DisplaySettingsChangeCallback)
};

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

   #if JUCE_MAC_API_VERSION_CAN_BE_BUILT (12, 0)
    if (@available (macOS 12.0, *))
    {
        const auto safeInsets = [s safeAreaInsets];
        d.safeAreaInsets = detail::WindowingHelpers::roundToInt (BorderSize<double> { safeInsets.top,
                                                                                      safeInsets.left,
                                                                                      safeInsets.bottom,
                                                                                      safeInsets.right }.multipliedBy (1.0 / (double) masterScale));
    }
   #endif

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
    [NSGraphicsContext setCurrentContext: [NSGraphicsContext graphicsContextWithCGContext: juce_getImageContext (image)
                                                                                  flipped: false]];
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
        const auto createImageFromCGImage = [&] (CGImageRef cgImage)
        {
            jassert (cgImage != nullptr);

            const auto width = CGImageGetWidth (cgImage);
            const auto height = CGImageGetHeight (cgImage);
            const auto cgRect = CGRectMake (0, 0, (CGFloat) width, (CGFloat) height);
            const Image image (Image::ARGB, (int) width, (int) height, true);

            CGContextDrawImage (juce_getImageContext (image), cgRect, cgImage);

            return image;
        };

       #if JUCE_MAC_API_VERSION_MIN_REQUIRED_AT_LEAST (14, 4)

        if (dlopen ("/System/Library/Frameworks/ScreenCaptureKit.framework/ScreenCaptureKit", RTLD_LAZY) == nullptr)
        {
            DBG (dlerror());
            jassertfalse;
            return {};
        }

        std::promise<Image> result;

        const auto windowId = nsWindow.windowNumber;
        const auto windowRect = [nsWindow.screen convertRectToBacking: nsWindow.frame].size;

        const auto onSharableContent = [&] (SCShareableContent* content, NSError* contentError)
        {
            if (contentError != nullptr)
            {
                jassertfalse;
                result.set_value (Image{});
                return;
            }

            const auto window = [&]() -> SCWindow*
            {
                for (SCWindow* w in content.windows)
                    if (w.windowID == windowId)
                        return w;

                return nullptr;
            }();

            if (window == nullptr)
            {
                jassertfalse;
                result.set_value (Image{});
                return;
            }

            Class contentFilterClass = NSClassFromString (@"SCContentFilter");
            SCContentFilter* filter = [[[contentFilterClass alloc] initWithDesktopIndependentWindow: window] autorelease];

            Class streamConfigurationClass = NSClassFromString (@"SCStreamConfiguration");
            SCStreamConfiguration* config = [[[streamConfigurationClass alloc] init] autorelease];
            config.colorSpaceName = kCGColorSpaceSRGB;
            config.showsCursor = NO;
            config.ignoreShadowsSingleWindow = YES;
            config.captureResolution = SCCaptureResolutionBest;
            config.ignoreGlobalClipSingleWindow = YES;
            config.includeChildWindows = NO;
            config.width = (size_t) windowRect.width;
            config.height = (size_t) windowRect.height;

            const auto onScreenshot = [&] (CGImageRef screenshot, NSError* screenshotError)
            {
                jassert (screenshotError == nullptr);
                result.set_value (screenshotError == nullptr ? createImageFromCGImage (screenshot) : Image{});
            };

            Class screenshotManagerClass = NSClassFromString (@"SCScreenshotManager");
            [screenshotManagerClass captureImageWithFilter: filter
                                             configuration: config
                                         completionHandler: onScreenshot];
        };

        Class shareableContentClass = NSClassFromString (@"SCShareableContent");
        [shareableContentClass getCurrentProcessShareableContentWithCompletionHandler: onSharableContent];

        return result.get_future().get();

       #else

        JUCE_BEGIN_IGNORE_DEPRECATION_WARNINGS
        return createImageFromCGImage ((CGImageRef) CFAutorelease (CGWindowListCreateImage (CGRectNull,
                                                                                            kCGWindowListOptionIncludingWindow,
                                                                                            (CGWindowID) [nsWindow windowNumber],
                                                                                            kCGWindowImageBoundsIgnoreFraming)));
        JUCE_END_IGNORE_DEPRECATION_WARNINGS

       #endif
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

} // namespace juce
