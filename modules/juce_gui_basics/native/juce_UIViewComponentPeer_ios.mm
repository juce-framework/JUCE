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

#if TARGET_OS_SIMULATOR && JUCE_COREGRAPHICS_RENDER_WITH_MULTIPLE_PAINT_CALLS
 #warning JUCE_COREGRAPHICS_RENDER_WITH_MULTIPLE_PAINT_CALLS uses parts of the Metal API that are currently unsupported in the simulator - falling back to JUCE_COREGRAPHICS_RENDER_WITH_MULTIPLE_PAINT_CALLS=0
 #undef JUCE_COREGRAPHICS_RENDER_WITH_MULTIPLE_PAINT_CALLS
#endif

namespace juce
{

//==============================================================================
static NSArray* getContainerAccessibilityElements (AccessibilityHandler& handler)
{
    const auto children = handler.getChildren();

    NSMutableArray* accessibleChildren = [NSMutableArray arrayWithCapacity: (NSUInteger) children.size()];

    for (auto* childHandler : children)
    {
        id accessibleElement = [&childHandler]
        {
            id nativeChild = static_cast<id> (childHandler->getNativeImplementation());

            if (   ! childHandler->getChildren().empty()
                || AccessibilityHandler::getNativeChildForComponent (childHandler->getComponent()) != nullptr)
            {
                return [nativeChild accessibilityContainer];
            }

            return nativeChild;
        }();

        if (accessibleElement != nil)
            [accessibleChildren addObject: accessibleElement];
    }

    id nativeHandler = static_cast<id> (handler.getNativeImplementation());

    if (auto* view = static_cast<UIView*> (AccessibilityHandler::getNativeChildForComponent (handler.getComponent())))
    {
        [static_cast<UIAccessibilityElement*> (view) setAccessibilityContainer: [nativeHandler accessibilityContainer]];

        [accessibleChildren addObject: view];
    }

    [accessibleChildren addObject: nativeHandler];

    return accessibleChildren;
}

class UIViewComponentPeer;

namespace iOSGlobals
{
class KeysCurrentlyDown
{
public:
    bool isDown (int x) const { return down.find (x) != down.cend(); }

    void setDown (int x, bool b)
    {
        if (b)
            down.insert (x);
        else
            down.erase (x);
    }

private:
    std::set<int> down;
};
static KeysCurrentlyDown keysCurrentlyDown;
static UIViewComponentPeer* currentlyFocusedPeer = nullptr;
} // namespace iOSGlobals

static UIInterfaceOrientation getWindowOrientation()
{
    UIApplication* sharedApplication = [UIApplication sharedApplication];

    if (@available (iOS 13.0, *))
    {
        for (UIScene* scene in [sharedApplication connectedScenes])
            if ([scene isKindOfClass: [UIWindowScene class]])
                return [(UIWindowScene*) scene interfaceOrientation];
    }

    JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wdeprecated-declarations")
    return [sharedApplication statusBarOrientation];
    JUCE_END_IGNORE_WARNINGS_GCC_LIKE
}

struct Orientations
{
    static Desktop::DisplayOrientation convertToJuce (UIInterfaceOrientation orientation)
    {
        switch (orientation)
        {
            case UIInterfaceOrientationPortrait:            return Desktop::upright;
            case UIInterfaceOrientationPortraitUpsideDown:  return Desktop::upsideDown;
            case UIInterfaceOrientationLandscapeLeft:       return Desktop::rotatedClockwise;
            case UIInterfaceOrientationLandscapeRight:      return Desktop::rotatedAntiClockwise;
            case UIInterfaceOrientationUnknown:
            default:                                        jassertfalse; // unknown orientation!
        }

        return Desktop::upright;
    }

    static UIInterfaceOrientation convertFromJuce (Desktop::DisplayOrientation orientation)
    {
        switch (orientation)
        {
            case Desktop::upright:                          return UIInterfaceOrientationPortrait;
            case Desktop::upsideDown:                       return UIInterfaceOrientationPortraitUpsideDown;
            case Desktop::rotatedClockwise:                 return UIInterfaceOrientationLandscapeLeft;
            case Desktop::rotatedAntiClockwise:             return UIInterfaceOrientationLandscapeRight;
            case Desktop::allOrientations:
            default:                                        jassertfalse; // unknown orientation!
        }

        return UIInterfaceOrientationPortrait;
    }

    static UIInterfaceOrientationMask getSupportedOrientations()
    {
        NSUInteger allowed = 0;
        auto& d = Desktop::getInstance();

        if (d.isOrientationEnabled (Desktop::upright))              allowed |= UIInterfaceOrientationMaskPortrait;
        if (d.isOrientationEnabled (Desktop::upsideDown))           allowed |= UIInterfaceOrientationMaskPortraitUpsideDown;
        if (d.isOrientationEnabled (Desktop::rotatedClockwise))     allowed |= UIInterfaceOrientationMaskLandscapeLeft;
        if (d.isOrientationEnabled (Desktop::rotatedAntiClockwise)) allowed |= UIInterfaceOrientationMaskLandscapeRight;

        return allowed;
    }
};

enum class MouseEventFlags
{
    none,
    down,
    up,
    upAndCancel,
};

//==============================================================================
} // namespace juce

using namespace juce;

@interface JuceUITextPosition : UITextPosition
{
@public
    int index;
}
@end

@implementation JuceUITextPosition

+ (instancetype) withIndex: (int) indexIn
{
    auto* result = [[JuceUITextPosition alloc] init];
    result->index = indexIn;
    return [result autorelease];
}

@end

//==============================================================================
@interface JuceUITextRange : UITextRange
{
@public
    int from, to;
}
@end

@implementation JuceUITextRange

+ (instancetype) withRange: (juce::Range<int>) range
{
    return [JuceUITextRange from: range.getStart() to: range.getEnd()];
}

+ (instancetype) from: (int) from to: (int) to
{
    auto* result = [[JuceUITextRange alloc] init];
    result->from = from;
    result->to = to;
    return [result autorelease];
}

- (UITextPosition*) start
{
    return [JuceUITextPosition withIndex: from];
}

- (UITextPosition*) end
{
    return [JuceUITextPosition withIndex: to];
}

- (Range<int>) range
{
    return Range<int>::between (from, to);
}

- (BOOL) isEmpty
{
    return from == to;
}

@end

//==============================================================================
// UITextInputStringTokenizer doesn't handle 'line' granularities correctly by default, hence
// this subclass.
@interface JuceTextInputTokenizer : UITextInputStringTokenizer
{
    UIViewComponentPeer* peer;
}

- (instancetype) initWithPeer: (UIViewComponentPeer*) peer;

@end

//==============================================================================
@interface JuceUITextSelectionRect : UITextSelectionRect
{
    CGRect _rect;
}

@end

@implementation JuceUITextSelectionRect

+ (instancetype) withRect: (CGRect) rect
{
    auto* result = [[JuceUITextSelectionRect alloc] init];
    result->_rect = rect;
    return [result autorelease];
}

- (CGRect) rect { return _rect; }
- (NSWritingDirection) writingDirection { return NSWritingDirectionNatural; }
- (BOOL) containsStart { return NO; }
- (BOOL) containsEnd   { return NO; }
- (BOOL) isVertical    { return NO; }

@end

//==============================================================================
struct CADisplayLinkDeleter
{
    void operator() (CADisplayLink* displayLink) const noexcept
    {
        [displayLink invalidate];
        [displayLink release];
    }
};

@interface JuceTextView : UIView <UITextInput>
{
@public
    UIViewComponentPeer* owner;
    id<UITextInputDelegate> delegate;
}

- (instancetype) initWithOwner: (UIViewComponentPeer*) owner;

@end

@interface JuceUIView : UIView<CALayerDelegate>
{
@public
    UIViewComponentPeer* owner;
    std::unique_ptr<CADisplayLink, CADisplayLinkDeleter> displayLink;
}

- (JuceUIView*) initWithOwner: (UIViewComponentPeer*) owner withFrame: (CGRect) frame;
- (void) dealloc;

+ (Class) layerClass;

- (void) displayLinkCallback: (CADisplayLink*) dl;

- (void) drawRect: (CGRect) r;

- (void) touchesBegan:     (NSSet*) touches  withEvent: (UIEvent*) event;
- (void) touchesMoved:     (NSSet*) touches  withEvent: (UIEvent*) event;
- (void) touchesEnded:     (NSSet*) touches  withEvent: (UIEvent*) event;
- (void) touchesCancelled: (NSSet*) touches  withEvent: (UIEvent*) event;

- (void) onHover: (UIHoverGestureRecognizer*) gesture API_AVAILABLE (ios (13.0));
- (void) onScroll: (UIPanGestureRecognizer*) gesture;

- (BOOL) becomeFirstResponder;
- (BOOL) resignFirstResponder;
- (BOOL) canBecomeFirstResponder;

- (void) traitCollectionDidChange: (UITraitCollection*) previousTraitCollection;

- (BOOL) isAccessibilityElement;
- (CGRect) accessibilityFrame;
- (NSArray*) accessibilityElements;
@end

//==============================================================================
@interface JuceUIViewController : UIViewController
{
}

- (JuceUIViewController*) init;

- (NSUInteger) supportedInterfaceOrientations;
- (BOOL) shouldAutorotateToInterfaceOrientation: (UIInterfaceOrientation) interfaceOrientation;
- (void) willRotateToInterfaceOrientation: (UIInterfaceOrientation) toInterfaceOrientation duration: (NSTimeInterval) duration;
- (void) didRotateFromInterfaceOrientation: (UIInterfaceOrientation) fromInterfaceOrientation;
- (void) viewWillTransitionToSize: (CGSize) size withTransitionCoordinator: (id<UIViewControllerTransitionCoordinator>) coordinator;
- (BOOL) prefersStatusBarHidden;
- (UIStatusBarStyle) preferredStatusBarStyle;

- (void) viewDidLoad;
- (void) viewWillAppear: (BOOL) animated;
- (void) viewDidAppear: (BOOL) animated;
- (void) viewWillLayoutSubviews;
- (void) viewDidLayoutSubviews;
@end

//==============================================================================
@interface JuceUIWindow : UIWindow
{
@private
    UIViewComponentPeer* owner;
}

- (void) setOwner: (UIViewComponentPeer*) owner;
- (void) becomeKeyWindow;
@end

//==============================================================================
//==============================================================================
namespace juce
{

struct UIViewPeerControllerReceiver
{
    virtual ~UIViewPeerControllerReceiver() = default;
    virtual void setViewController (UIViewController*) = 0;
};

//==============================================================================
class UIViewComponentPeer final : public ComponentPeer,
                                  public UIViewPeerControllerReceiver
{
public:
    UIViewComponentPeer (Component&, int windowStyleFlags, UIView* viewToAttachTo);
    ~UIViewComponentPeer() override;

    //==============================================================================
    void* getNativeHandle() const override                  { return view; }
    void setVisible (bool shouldBeVisible) override;
    void setTitle (const String& title) override;
    void setBounds (const Rectangle<int>&, bool isNowFullScreen) override;

    void setViewController (UIViewController* newController) override
    {
        jassert (controller == nullptr);
        controller = [newController retain];
    }

    Rectangle<int> getBounds() const override                 { return getBounds (! isSharedWindow); }
    Rectangle<int> getBounds (bool global) const;
    Point<float> localToGlobal (Point<float> relativePosition) override;
    Point<float> globalToLocal (Point<float> screenPosition) override;
    using ComponentPeer::localToGlobal;
    using ComponentPeer::globalToLocal;
    void setAlpha (float newAlpha) override;
    void setMinimised (bool) override                         {}
    bool isMinimised() const override                         { return false; }
    bool isShowing() const override                           { return true; }
    void setFullScreen (bool shouldBeFullScreen) override;
    bool isFullScreen() const override                        { return fullScreen; }
    bool contains (Point<int> localPos, bool trueIfInAChildWindow) const override;
    OptionalBorderSize getFrameSizeIfPresent() const override { return {}; }
    BorderSize<int> getFrameSize() const override             { return BorderSize<int>(); }
    bool setAlwaysOnTop (bool alwaysOnTop) override;
    void toFront (bool makeActiveWindow) override;
    void toBehind (ComponentPeer* other) override;
    void setIcon (const Image& newIcon) override;
    StringArray getAvailableRenderingEngines() override       { return StringArray ("CoreGraphics Renderer"); }

    void displayLinkCallback();

    void drawRect (CGRect);
    void drawRectWithContext (CGContextRef, CGRect);
    bool canBecomeKeyWindow();

    //==============================================================================
    void viewFocusGain();
    void viewFocusLoss();
    bool isFocused() const override;
    void grabFocus() override;
    void textInputRequired (Point<int>, TextInputTarget&) override;
    void dismissPendingTextInput() override;
    void closeInputMethodContext() override;

    void updateScreenBounds();

    void handleTouches (UIEvent*, MouseEventFlags);

    API_AVAILABLE (ios (13.0)) void onHover (UIHoverGestureRecognizer*);
    void onScroll (UIPanGestureRecognizer*);

    Range<int> getMarkedTextRange() const
    {
        return Range<int>::withStartAndLength (startOfMarkedTextInTextInputTarget,
                                               stringBeingComposed.length());
    }

    enum class UnderlineRegion
    {
        none,
        underCompositionRange
    };

    void replaceMarkedRangeWithText (TextInputTarget* target,
                                     const String& text,
                                     UnderlineRegion underline)
    {
        if (stringBeingComposed.isNotEmpty())
            target->setHighlightedRegion (getMarkedTextRange());

        target->insertTextAtCaret (text);

        const auto underlineRanges = underline == UnderlineRegion::underCompositionRange
                                   ? Array { Range<int>::withStartAndLength (startOfMarkedTextInTextInputTarget,
                                                                             text.length()) }
                                   : Array<Range<int>>{};
        target->setTemporaryUnderlining (underlineRanges);

        stringBeingComposed = text;
    }

    //==============================================================================
    void repaint (const Rectangle<int>& area) override;
    void performAnyPendingRepaintsNow() override;

    //==============================================================================
    UIWindow* window = nil;
    JuceUIView* view = nil;
    UIViewController* controller = nil;
    const bool isSharedWindow, isAppex;
    String stringBeingComposed;
    int startOfMarkedTextInTextInputTarget = 0;
    bool fullScreen = false, insideDrawRect = false;
    NSUniquePtr<JuceTextView> hiddenTextInput { [[JuceTextView alloc] initWithOwner: this] };
    NSUniquePtr<JuceTextInputTokenizer> tokenizer { [[JuceTextInputTokenizer alloc] initWithPeer: this] };

    static int64 getMouseTime (NSTimeInterval timestamp) noexcept
    {
        return (Time::currentTimeMillis() - Time::getMillisecondCounter())
             + (int64) (timestamp * 1000.0);
    }

    static int64 getMouseTime (UIEvent* e) noexcept
    {
        return getMouseTime ([e timestamp]);
    }

    static NSString* getDarkModeNotificationName()
    {
        return @"ViewDarkModeChanged";
    }

    static MultiTouchMapper<UITouch*> currentTouches;

    static UIKeyboardType getUIKeyboardType (TextInputTarget::VirtualKeyboardType type) noexcept
    {
        switch (type)
        {
            case TextInputTarget::textKeyboard:          return UIKeyboardTypeDefault;
            case TextInputTarget::numericKeyboard:       return UIKeyboardTypeNumbersAndPunctuation;
            case TextInputTarget::decimalKeyboard:       return UIKeyboardTypeNumbersAndPunctuation;
            case TextInputTarget::urlKeyboard:           return UIKeyboardTypeURL;
            case TextInputTarget::emailAddressKeyboard:  return UIKeyboardTypeEmailAddress;
            case TextInputTarget::phoneNumberKeyboard:   return UIKeyboardTypePhonePad;
            case TextInputTarget::passwordKeyboard:      return UIKeyboardTypeASCIICapable;
        }

        jassertfalse;
        return UIKeyboardTypeDefault;
    }

   #if JUCE_COREGRAPHICS_RENDER_WITH_MULTIPLE_PAINT_CALLS
    std::unique_ptr<CoreGraphicsMetalLayerRenderer> metalRenderer;
   #endif

    RectangleList<float> deferredRepaints;

private:
    void appStyleChanged() override
    {
        [controller setNeedsStatusBarAppearanceUpdate];
    }

    //==============================================================================
    class AsyncRepaintMessage final : public CallbackMessage
    {
    public:
        UIViewComponentPeer* const peer;
        const Rectangle<int> rect;

        AsyncRepaintMessage (UIViewComponentPeer* const p, const Rectangle<int>& r)
            : peer (p), rect (r)
        {
        }

        void messageCallback() override
        {
            if (ComponentPeer::isValidPeer (peer))
                peer->repaint (rect);
        }
    };

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (UIViewComponentPeer)
};

static UIViewComponentPeer* getViewPeer (JuceUIViewController* c)
{
    if (JuceUIView* juceView = (JuceUIView*) [c view])
        return juceView->owner;

    jassertfalse;
    return nullptr;
}

static void sendScreenBoundsUpdate (JuceUIViewController* c)
{
    if (auto* peer = getViewPeer (c))
        peer->updateScreenBounds();
}

static bool isKioskModeView (JuceUIViewController* c)
{
    if (auto* peer = getViewPeer (c))
        return Desktop::getInstance().getKioskModeComponent() == &(peer->getComponent());

    return false;
}

MultiTouchMapper<UITouch*> UIViewComponentPeer::currentTouches;

} // namespace juce

//==============================================================================
//==============================================================================
@implementation JuceUIViewController

- (JuceUIViewController*) init
{
    self = [super init];

    return self;
}

- (NSUInteger) supportedInterfaceOrientations
{
    return Orientations::getSupportedOrientations();
}

- (BOOL) shouldAutorotateToInterfaceOrientation: (UIInterfaceOrientation) interfaceOrientation
{
    return Desktop::getInstance().isOrientationEnabled (Orientations::convertToJuce (interfaceOrientation));
}

- (void) willRotateToInterfaceOrientation: (UIInterfaceOrientation) toInterfaceOrientation
                                 duration: (NSTimeInterval) duration
{
    ignoreUnused (toInterfaceOrientation, duration);

    [UIView setAnimationsEnabled: NO]; // disable this because it goes the wrong way and looks like crap.
}

- (void) didRotateFromInterfaceOrientation: (UIInterfaceOrientation) fromInterfaceOrientation
{
    ignoreUnused (fromInterfaceOrientation);
    sendScreenBoundsUpdate (self);
    [UIView setAnimationsEnabled: YES];
}

- (void) viewWillTransitionToSize: (CGSize) size withTransitionCoordinator: (id<UIViewControllerTransitionCoordinator>) coordinator
{
    [super viewWillTransitionToSize: size withTransitionCoordinator: coordinator];
    [coordinator animateAlongsideTransition: nil completion: ^void (id<UIViewControllerTransitionCoordinatorContext>)
    {
        sendScreenBoundsUpdate (self);
    }];
}

- (BOOL) prefersStatusBarHidden
{
    if (isKioskModeView (self))
        return true;

    return [[[NSBundle mainBundle] objectForInfoDictionaryKey: @"UIStatusBarHidden"] boolValue];
}

 - (BOOL) prefersHomeIndicatorAutoHidden
 {
     return isKioskModeView (self);
 }

- (UIStatusBarStyle) preferredStatusBarStyle
{
    if (@available (iOS 13.0, *))
    {
        if (auto* peer = getViewPeer (self))
        {
            switch (peer->getAppStyle())
            {
                case ComponentPeer::Style::automatic:
                    return UIStatusBarStyleDefault;
                case ComponentPeer::Style::light:
                    return UIStatusBarStyleDarkContent;
                case ComponentPeer::Style::dark:
                    return UIStatusBarStyleLightContent;
            }
        }
    }

    return UIStatusBarStyleDefault;
}

- (void) viewDidLoad
{
    sendScreenBoundsUpdate (self);
    [super viewDidLoad];
}

- (void) viewWillAppear: (BOOL) animated
{
    sendScreenBoundsUpdate (self);
    [super viewWillAppear:animated];
}

- (void) viewDidAppear: (BOOL) animated
{
    sendScreenBoundsUpdate (self);
    [super viewDidAppear:animated];
}

- (void) viewWillLayoutSubviews
{
    sendScreenBoundsUpdate (self);
}

- (void) viewDidLayoutSubviews
{
    sendScreenBoundsUpdate (self);
}

@end

@implementation JuceUIView

- (JuceUIView*) initWithOwner: (UIViewComponentPeer*) peer
                    withFrame: (CGRect) frame
{
    [super initWithFrame: frame];
    owner = peer;

   #if JUCE_COREGRAPHICS_RENDER_WITH_MULTIPLE_PAINT_CALLS
    if (@available (iOS 13.0, *))
    {
        auto* layer = (CAMetalLayer*) [self layer];
        layer.device = MTLCreateSystemDefaultDevice();
        layer.framebufferOnly = NO;
        layer.pixelFormat = MTLPixelFormatBGRA8Unorm_sRGB;

        if (owner != nullptr)
            layer.opaque = owner->getComponent().isOpaque();

        layer.presentsWithTransaction = YES;
        layer.needsDisplayOnBoundsChange = true;
        layer.presentsWithTransaction = true;
        layer.delegate = self;

        layer.allowsNextDrawableTimeout = NO;
    }
   #endif

    displayLink.reset ([CADisplayLink displayLinkWithTarget: self
                                                   selector: @selector (displayLinkCallback:)]);
    [displayLink.get() addToRunLoop: [NSRunLoop mainRunLoop]
                            forMode: NSDefaultRunLoopMode];

    [self addSubview: owner->hiddenTextInput.get()];

    if (@available (iOS 13.4, *))
    {
        auto hoverRecognizer = [[[UIHoverGestureRecognizer alloc] initWithTarget: self action: @selector (onHover:)] autorelease];
        [hoverRecognizer setCancelsTouchesInView: NO];
        [hoverRecognizer setRequiresExclusiveTouchType: YES];
        [self addGestureRecognizer: hoverRecognizer];

        auto panRecognizer = [[[UIPanGestureRecognizer alloc] initWithTarget: self action: @selector (onScroll:)] autorelease];
        [panRecognizer setCancelsTouchesInView: NO];
        [panRecognizer setRequiresExclusiveTouchType: YES];
        [panRecognizer setAllowedScrollTypesMask: UIScrollTypeMaskAll];
        [panRecognizer setMaximumNumberOfTouches: 0];
        [self addGestureRecognizer: panRecognizer];
    }

    return self;
}

- (void) dealloc
{
    [owner->hiddenTextInput.get() removeFromSuperview];
    displayLink = nullptr;

    [super dealloc];
}

//==============================================================================
+ (Class) layerClass
{
   #if JUCE_COREGRAPHICS_RENDER_WITH_MULTIPLE_PAINT_CALLS
    if (@available (iOS 13, *))
        return [CAMetalLayer class];
   #endif

    return [CALayer class];
}

- (void) displayLinkCallback: (CADisplayLink*) dl
{
    if (owner != nullptr)
        owner->displayLinkCallback();
}

#if JUCE_COREGRAPHICS_RENDER_WITH_MULTIPLE_PAINT_CALLS
- (CALayer*) makeBackingLayer
{
    auto* layer = [CAMetalLayer layer];

    layer.device = MTLCreateSystemDefaultDevice();
    layer.framebufferOnly = NO;
    layer.pixelFormat = MTLPixelFormatBGRA8Unorm_sRGB;

    if (owner != nullptr)
        layer.opaque = owner->getComponent().isOpaque();

    layer.presentsWithTransaction = YES;
    layer.needsDisplayOnBoundsChange = true;
    layer.presentsWithTransaction = true;
    layer.delegate = self;

    layer.allowsNextDrawableTimeout = NO;

    return layer;
}

- (void) displayLayer: (CALayer*) layer
{
    if (owner != nullptr)
    {
        owner->deferredRepaints = owner->metalRenderer->drawRectangleList (static_cast<CAMetalLayer*> (layer),
                                                                           (float) [self contentScaleFactor],
                                                                           [self] (auto&&... args) { owner->drawRectWithContext (args...); },
                                                                           std::move (owner->deferredRepaints),
                                                                           false);
    }
}
#endif

//==============================================================================
- (void) drawRect: (CGRect) r
{
    if (owner != nullptr)
        owner->drawRect (r);
}

//==============================================================================
- (void) touchesBegan: (NSSet*) touches withEvent: (UIEvent*) event
{
    ignoreUnused (touches);

    if (owner != nullptr)
        owner->handleTouches (event, MouseEventFlags::down);
}

- (void) touchesMoved: (NSSet*) touches withEvent: (UIEvent*) event
{
    ignoreUnused (touches);

    if (owner != nullptr)
        owner->handleTouches (event, MouseEventFlags::none);
}

- (void) touchesEnded: (NSSet*) touches withEvent: (UIEvent*) event
{
    ignoreUnused (touches);

    if (owner != nullptr)
        owner->handleTouches (event, MouseEventFlags::up);
}

- (void) touchesCancelled: (NSSet*) touches withEvent: (UIEvent*) event
{
    if (owner != nullptr)
        owner->handleTouches (event, MouseEventFlags::upAndCancel);

    [self touchesEnded: touches withEvent: event];
}

- (void) onHover: (UIHoverGestureRecognizer*) gesture
{
    if (owner != nullptr)
        owner->onHover (gesture);
}

- (void) onScroll: (UIPanGestureRecognizer*) gesture
{
    if (owner != nullptr)
        owner->onScroll (gesture);
}

static std::optional<int> getKeyCodeForSpecialCharacterString (StringRef characters)
{
    static const auto map = [&]
    {
        std::map<String, int> result
        {
            { nsStringToJuce (UIKeyInputUpArrow),       KeyPress::upKey },
            { nsStringToJuce (UIKeyInputDownArrow),     KeyPress::downKey },
            { nsStringToJuce (UIKeyInputLeftArrow),     KeyPress::leftKey },
            { nsStringToJuce (UIKeyInputRightArrow),    KeyPress::rightKey },
            { nsStringToJuce (UIKeyInputEscape),        KeyPress::escapeKey },
            { nsStringToJuce (UIKeyInputPageUp),        KeyPress::pageUpKey },
            { nsStringToJuce (UIKeyInputPageDown),      KeyPress::pageDownKey },
        };

        if (@available (iOS 13.4, *))
        {
            result.insert ({ { nsStringToJuce (UIKeyInputHome),          KeyPress::homeKey },
                             { nsStringToJuce (UIKeyInputEnd),           KeyPress::endKey },
                             { nsStringToJuce (UIKeyInputF1),            KeyPress::F1Key },
                             { nsStringToJuce (UIKeyInputF2),            KeyPress::F2Key },
                             { nsStringToJuce (UIKeyInputF3),            KeyPress::F3Key },
                             { nsStringToJuce (UIKeyInputF4),            KeyPress::F4Key },
                             { nsStringToJuce (UIKeyInputF5),            KeyPress::F5Key },
                             { nsStringToJuce (UIKeyInputF6),            KeyPress::F6Key },
                             { nsStringToJuce (UIKeyInputF7),            KeyPress::F7Key },
                             { nsStringToJuce (UIKeyInputF8),            KeyPress::F8Key },
                             { nsStringToJuce (UIKeyInputF9),            KeyPress::F9Key },
                             { nsStringToJuce (UIKeyInputF10),           KeyPress::F10Key },
                             { nsStringToJuce (UIKeyInputF11),           KeyPress::F11Key },
                             { nsStringToJuce (UIKeyInputF12),           KeyPress::F12Key } });
        }

       #if defined (__IPHONE_15_0) && __IPHONE_OS_VERSION_MAX_ALLOWED >= __IPHONE_15_0
        if (@available (iOS 15.0, *))
        {
            result.insert ({ { nsStringToJuce (UIKeyInputDelete),        KeyPress::deleteKey } });
        }
       #endif

        return result;
    }();

    const auto iter = map.find (characters);
    return iter != map.cend() ? std::make_optional (iter->second) : std::nullopt;
}

static int getKeyCodeForCharacters (StringRef unmodified)
{
    return getKeyCodeForSpecialCharacterString (unmodified).value_or (unmodified[0]);
}

static int getKeyCodeForCharacters (NSString* characters)
{
    return getKeyCodeForCharacters (nsStringToJuce (characters));
}

static void updateModifiers (const UIKeyModifierFlags flags)
{
    const auto convert = [&flags] (UIKeyModifierFlags f, int result) { return (flags & f) != 0 ? result : 0; };
    const auto juceFlags = convert (UIKeyModifierAlphaShift, 0) // capslock modifier currently not implemented
                         | convert (UIKeyModifierShift,      ModifierKeys::shiftModifier)
                         | convert (UIKeyModifierControl,    ModifierKeys::ctrlModifier)
                         | convert (UIKeyModifierAlternate,  ModifierKeys::altModifier)
                         | convert (UIKeyModifierCommand,    ModifierKeys::commandModifier)
                         | convert (UIKeyModifierNumericPad, 0); // numpad modifier currently not implemented

    ModifierKeys::currentModifiers = ModifierKeys::currentModifiers.withOnlyMouseButtons().withFlags (juceFlags);
}

API_AVAILABLE (ios(13.4))
static int getKeyCodeForKey (UIKey* key)
{
    return getKeyCodeForCharacters ([key charactersIgnoringModifiers]);
}

API_AVAILABLE (ios(13.4))
static bool attemptToConsumeKeys (JuceUIView* view, NSSet<UIPress*>* presses)
{
    auto used = false;

    for (UIPress* press in presses)
    {
        if (auto* key = [press key])
        {
            const auto code = getKeyCodeForKey (key);
            const auto handleCodepoint = [view, &used, code] (juce_wchar codepoint)
            {
                // These both need to fire; no short-circuiting!
                used |= view->owner->handleKeyUpOrDown (true);
                used |= view->owner->handleKeyPress (code, codepoint);
            };

            if (getKeyCodeForSpecialCharacterString (nsStringToJuce ([key charactersIgnoringModifiers])).has_value())
                handleCodepoint (0);
            else
                for (const auto codepoint : nsStringToJuce ([key characters]))
                    handleCodepoint (codepoint);
        }
    }

    return used;
}

- (void) pressesBegan: (NSSet<UIPress*>*) presses withEvent: (UIPressesEvent*) event
{
    const auto handledEvent = [&]
    {
        if (@available (iOS 13.4, *))
        {
            auto isEscape = false;

            updateModifiers ([event modifierFlags]);

            for (UIPress* press in presses)
            {
                if (auto* key = [press key])
                {
                    const auto code = getKeyCodeForKey (key);
                    isEscape |= code == KeyPress::escapeKey;
                    iOSGlobals::keysCurrentlyDown.setDown (code, true);
                }
            }

            return ((isEscape && owner->stringBeingComposed.isEmpty())
                    || owner->findCurrentTextInputTarget() == nullptr)
                   && attemptToConsumeKeys (self, presses);
        }

        return false;
    }();

    if (! handledEvent)
        [super pressesBegan: presses withEvent: event];
}

/*  Returns true if we handled the event. */
static bool doKeysUp (UIViewComponentPeer* owner, NSSet<UIPress*>* presses, UIPressesEvent* event)
{
    if (@available (iOS 13.4, *))
    {
        updateModifiers ([event modifierFlags]);

        for (UIPress* press in presses)
            if (auto* key = [press key])
                iOSGlobals::keysCurrentlyDown.setDown (getKeyCodeForKey (key), false);

        return owner->findCurrentTextInputTarget() == nullptr && owner->handleKeyUpOrDown (false);
    }

    return false;
}

- (void) pressesEnded: (NSSet<UIPress*>*) presses withEvent: (UIPressesEvent*) event
{
    if (! doKeysUp (owner, presses, event))
        [super pressesEnded: presses withEvent: event];
}

- (void) pressesCancelled: (NSSet<UIPress*>*) presses withEvent: (UIPressesEvent*) event
{
    if (! doKeysUp (owner, presses, event))
        [super pressesCancelled: presses withEvent: event];
}

//==============================================================================
- (BOOL) becomeFirstResponder
{
    if (owner != nullptr)
        owner->viewFocusGain();

    return true;
}

- (BOOL) resignFirstResponder
{
    if (owner != nullptr)
        owner->viewFocusLoss();

    return [super resignFirstResponder];
}

- (BOOL) canBecomeFirstResponder
{
    return owner != nullptr && owner->canBecomeKeyWindow();
}

- (void) traitCollectionDidChange: (UITraitCollection*) previousTraitCollection
{
    [super traitCollectionDidChange: previousTraitCollection];

    const auto wasDarkModeActive = ([previousTraitCollection userInterfaceStyle] == UIUserInterfaceStyleDark);

    if (wasDarkModeActive != Desktop::getInstance().isDarkModeActive())
        [[NSNotificationCenter defaultCenter] postNotificationName: UIViewComponentPeer::getDarkModeNotificationName()
                                                            object: nil];
}

- (BOOL) isAccessibilityElement
{
    return NO;
}

- (CGRect) accessibilityFrame
{
    if (owner != nullptr)
        if (auto* handler = owner->getComponent().getAccessibilityHandler())
            return convertToCGRect (handler->getComponent().getScreenBounds());

    return CGRectZero;
}

- (NSArray*) accessibilityElements
{
    if (owner != nullptr)
        if (auto* handler = owner->getComponent().getAccessibilityHandler())
            return getContainerAccessibilityElements (*handler);

    return nil;
}

@end

//==============================================================================
@implementation JuceUIWindow

- (void) setOwner: (UIViewComponentPeer*) peer
{
    owner = peer;
}

- (void) becomeKeyWindow
{
    [super becomeKeyWindow];

    if (owner != nullptr)
        owner->grabFocus();
}

@end

/** see https://developer.apple.com/library/archive/documentation/StringsTextFonts/Conceptual/TextAndWebiPhoneOS/LowerLevelText-HandlingTechnologies/LowerLevelText-HandlingTechnologies.html */
@implementation JuceTextView

- (TextInputTarget*) getTextInputTarget
{
    if (owner != nullptr)
        return owner->findCurrentTextInputTarget();

    return nullptr;
}

- (instancetype) initWithOwner: (UIViewComponentPeer*) ownerIn
{
    [super init];
    owner = ownerIn;
    delegate = nil;

    // The frame must have a finite size, otherwise some accessibility events will be ignored
    self.frame = CGRectMake (0.0, 0.0, 1.0, 1.0);
    return self;
}

- (BOOL) canPerformAction: (SEL) action withSender: (id) sender
{
    if (auto* target = [self getTextInputTarget])
    {
        if (action == @selector (paste:))
            return [[UIPasteboard generalPasteboard] hasStrings];
    }

    return [super canPerformAction: action withSender: sender];
}

- (void) cut: (id) sender
{
    [self copy: sender];

    if (auto* target = [self getTextInputTarget])
    {
        if (delegate != nil)
            [delegate textWillChange: self];

        target->insertTextAtCaret ("");

        if (delegate != nil)
            [delegate textDidChange: self];
    }
}

- (void) copy: (id) sender
{
    if (auto* target = [self getTextInputTarget])
        [[UIPasteboard generalPasteboard] setString: juceStringToNS (target->getTextInRange (target->getHighlightedRegion()))];
}

- (void) paste: (id) sender
{
    if (auto* target = [self getTextInputTarget])
    {
        if (auto* string = [[UIPasteboard generalPasteboard] string])
        {
            if (delegate != nil)
                [delegate textWillChange: self];

            target->insertTextAtCaret (nsStringToJuce (string));

            if (delegate != nil)
                [delegate textDidChange: self];
        }
    }
}

- (void) selectAll: (id) sender
{
    if (auto* target = [self getTextInputTarget])
        target->setHighlightedRegion ({ 0, target->getTotalNumChars() });
}

- (void) deleteBackward
{
    auto* target = [self getTextInputTarget];

    if (target == nullptr)
        return;

    const auto range = target->getHighlightedRegion();
    const auto rangeToDelete = range.isEmpty() ? range.withStartAndLength (jmax (range.getStart() - 1, 0),
                                                                           range.getStart() != 0 ? 1 : 0)
                                               : range;
    const auto start = rangeToDelete.getStart();

    // This ensures that the cursor is at the beginning, rather than the end, of the selection
    target->setHighlightedRegion ({ start, start });
    target->setHighlightedRegion (rangeToDelete);
    target->insertTextAtCaret ("");
}

- (void) insertText: (NSString*) text
{
    if (owner == nullptr)
        return;

    if (auto* target = owner->findCurrentTextInputTarget())
    {
        // If we're in insertText, it's because there's a focused TextInputTarget,
        // and key presses from pressesBegan and pressesEnded have been composed
        // into a string that is now ready for insertion.
        // Because JUCE has been passing key events to the system for composition, it
        // won't have been processing those key presses itself, so it may not have had
        // a chance to process keys like return/tab/etc.
        // It's not possible to filter out these keys during pressesBegan, because they
        // may form part of a longer composition sequence.
        // e.g. when entering Japanese text, the return key may be used to select an option
        // from the IME menu, and in this situation the return key should not be propagated
        // to the JUCE view.
        // If we receive a special character (return/tab/etc.) in insertText, it can
        // only be because the composition has finished, so we can turn the event into
        // a KeyPress and trust the current TextInputTarget to process it correctly.
        const auto redirectKeyPresses = [&] (juce_wchar codepoint)
        {
            target->setTemporaryUnderlining ({});

            // Simulate a key down
            const auto code = getKeyCodeForCharacters (String::charToString (codepoint));
            iOSGlobals::keysCurrentlyDown.setDown (code, true);
            owner->handleKeyUpOrDown (true);

            owner->handleKeyPress (code, codepoint);

            // Simulate a key up
            iOSGlobals::keysCurrentlyDown.setDown (code, false);
            owner->handleKeyUpOrDown (false);
        };

        using UR = UIViewComponentPeer::UnderlineRegion;

        if ([text isEqual: @"\n"] || [text isEqual: @"\r"])
            redirectKeyPresses ('\r');
        else if ([text isEqual: @"\t"])
            redirectKeyPresses ('\t');
        else
            owner->replaceMarkedRangeWithText (target, nsStringToJuce (text), UR::none);
    }

    owner->stringBeingComposed.clear();
    owner->startOfMarkedTextInTextInputTarget = 0;
}

- (BOOL) hasText
{
    if (auto* target = [self getTextInputTarget])
        return target->getTextInRange ({ 0, 1 }).isNotEmpty();

    return NO;
}

- (BOOL) accessibilityElementsHidden
{
    return NO;
}

- (UITextRange*) selectedTextRangeForTarget: (TextInputTarget*) target
{
    if (target != nullptr)
        return [JuceUITextRange withRange: target->getHighlightedRegion()];

    return nil;
}

- (UITextRange*) selectedTextRange
{
    return [self selectedTextRangeForTarget: [self getTextInputTarget]];
}

- (void) setSelectedTextRange: (JuceUITextRange*) range
{
    if (auto* target = [self getTextInputTarget])
        target->setHighlightedRegion (range != nil ? [range range] : Range<int>());
}

- (UITextRange*) markedTextRange
{
    if (owner != nullptr && owner->stringBeingComposed.isNotEmpty())
        if (auto* target = owner->findCurrentTextInputTarget())
            return [JuceUITextRange withRange: owner->getMarkedTextRange()];

    return nil;
}

- (void) setMarkedText: (NSString*) markedText
         selectedRange: (NSRange) selectedRange
{
    if (owner == nullptr)
        return;

    const auto newMarkedText = nsStringToJuce (markedText);
    const ScopeGuard scope { [&] { owner->stringBeingComposed = newMarkedText; } };

    auto* target = owner->findCurrentTextInputTarget();

    if (target == nullptr)
        return;

    if (owner->stringBeingComposed.isEmpty())
        owner->startOfMarkedTextInTextInputTarget = target->getHighlightedRegion().getStart();

    using UR = UIViewComponentPeer::UnderlineRegion;
    owner->replaceMarkedRangeWithText (target, newMarkedText, UR::underCompositionRange);

    const auto newSelection = nsRangeToJuce (selectedRange) + owner->startOfMarkedTextInTextInputTarget;
    target->setHighlightedRegion (newSelection);
}

- (void) unmarkText
{
    if (owner == nullptr)
        return;

    auto* target = owner->findCurrentTextInputTarget();

    if (target == nullptr)
        return;

    using UR = UIViewComponentPeer::UnderlineRegion;
    owner->replaceMarkedRangeWithText (target, owner->stringBeingComposed, UR::none);
    owner->stringBeingComposed.clear();
    owner->startOfMarkedTextInTextInputTarget = 0;
}

- (NSDictionary<NSAttributedStringKey, id>*) markedTextStyle
{
    return nil;
}

- (void) setMarkedTextStyle: (NSDictionary<NSAttributedStringKey, id>*) dict
{
}

- (UITextPosition*) beginningOfDocument
{
    return [JuceUITextPosition withIndex: 0];
}

- (UITextPosition*) endOfDocument
{
    if (auto* target = [self getTextInputTarget])
        return [JuceUITextPosition withIndex: target->getTotalNumChars()];

    return [JuceUITextPosition withIndex: 0];
}

- (id<UITextInputTokenizer>) tokenizer
{
    return owner->tokenizer.get();
}

- (NSWritingDirection) baseWritingDirectionForPosition: (UITextPosition*) position
                                           inDirection: (UITextStorageDirection) direction
{
    return NSWritingDirectionNatural;
}

- (CGRect) caretRectForPosition: (JuceUITextPosition*) position
{
    if (position == nil)
        return CGRectZero;

    // Currently we ignore the requested position and just return the text editor's caret position
    if (auto* target = [self getTextInputTarget])
    {
        if (auto* comp = dynamic_cast<Component*> (target))
        {
            const auto areaOnDesktop = comp->localAreaToGlobal (target->getCaretRectangle());
            return convertToCGRect (detail::ScalingHelpers::scaledScreenPosToUnscaled (areaOnDesktop));
        }
    }

    return CGRectZero;
}

- (UITextRange*) characterRangeByExtendingPosition: (JuceUITextPosition*) position
                                       inDirection: (UITextLayoutDirection) direction
{
    const auto newPosition = [self indexFromPosition: position inDirection: direction offset: 1];
    return [JuceUITextRange from: position->index to: newPosition];
}

- (int) closestIndexToPoint: (CGPoint) point
{
    if (auto* target = [self getTextInputTarget])
    {
        if (auto* comp = dynamic_cast<Component*> (target))
        {
            const auto pointOnDesktop = detail::ScalingHelpers::unscaledScreenPosToScaled (convertToPointFloat (point));
            return target->getCharIndexForPoint (comp->getLocalPoint (nullptr, pointOnDesktop).roundToInt());
        }
    }

    return -1;
}

- (UITextRange*) characterRangeAtPoint: (CGPoint) point
{
    const auto index = [self closestIndexToPoint: point];
    const auto result = index != -1 ? [JuceUITextRange from: index to: index] : nil;
    jassert (result != nullptr);
    return result;
}

- (UITextPosition*) closestPositionToPoint: (CGPoint) point
{
    const auto index = [self closestIndexToPoint: point];
    const auto result = index != -1 ? [JuceUITextPosition withIndex: index] : nil;
    jassert (result != nullptr);
    return result;
}

- (UITextPosition*) closestPositionToPoint: (CGPoint) point
                               withinRange: (JuceUITextRange*) range
{
    const auto index = [self closestIndexToPoint: point];
    const auto result = index != -1 ? [JuceUITextPosition withIndex: [range range].clipValue (index)] : nil;
    jassert (result != nullptr);
    return result;
}

- (NSComparisonResult) comparePosition: (JuceUITextPosition*) position
                            toPosition: (JuceUITextPosition*) other
{
    const auto a = position != nil ? makeOptional (position->index) : nullopt;
    const auto b = other    != nil ? makeOptional (other   ->index) : nullopt;

    if (a < b)
        return NSOrderedAscending;

    if (b < a)
        return NSOrderedDescending;

    return NSOrderedSame;
}

- (NSInteger) offsetFromPosition: (JuceUITextPosition*) from
                      toPosition: (JuceUITextPosition*) toPosition
{
    if (from != nil && toPosition != nil)
        return toPosition->index - from->index;

    return 0;
}

- (int) indexFromPosition: (JuceUITextPosition*) position
              inDirection: (UITextLayoutDirection) direction
                   offset: (NSInteger) offset
{
    switch (direction)
    {
        case UITextLayoutDirectionLeft:
        case UITextLayoutDirectionRight:
            return position->index + (int) (offset * (direction == UITextLayoutDirectionLeft ? -1 : 1));

        case UITextLayoutDirectionUp:
        case UITextLayoutDirectionDown:
        {
            if (auto* target = [self getTextInputTarget])
            {
                const auto originalRectangle = target->getCaretRectangleForCharIndex (position->index);

                auto testIndex = position->index;

                for (auto lineOffset = 0; lineOffset < offset; ++lineOffset)
                {
                    const auto caretRect = target->getCaretRectangleForCharIndex (testIndex);
                    const auto newY = direction == UITextLayoutDirectionUp ? caretRect.getY() - 1
                                                                           : caretRect.getBottom() + 1;
                    testIndex = target->getCharIndexForPoint ({ originalRectangle.getX(), newY });
                }

                return testIndex;
            }
        }
    }

    return position->index;
}

- (UITextPosition*) positionFromPosition: (JuceUITextPosition*) position
                             inDirection: (UITextLayoutDirection) direction
                                  offset: (NSInteger) offset
{
    return [JuceUITextPosition withIndex: [self indexFromPosition: position inDirection: direction offset: offset]];
}

- (UITextPosition*) positionFromPosition: (JuceUITextPosition*) position
                                  offset: (NSInteger) offset
{
    if (position != nil)
    {
        if (auto* target = [self getTextInputTarget])
        {
            const auto newIndex = position->index + (int) offset;

            if (isPositiveAndBelow (newIndex, target->getTotalNumChars() + 1))
                return [JuceUITextPosition withIndex: newIndex];
        }
    }

   return nil;
}

- (CGRect) firstRectForRange: (JuceUITextRange*) range
{
    if (auto* target = [self getTextInputTarget])
    {
        if (auto* comp = dynamic_cast<Component*> (target))
        {
            const auto list = target->getTextBounds ([range range]);

            if (! list.isEmpty())
            {
                const auto areaOnDesktop = comp->localAreaToGlobal (list.getRectangle (0));
                return convertToCGRect (detail::ScalingHelpers::scaledScreenPosToUnscaled (areaOnDesktop));
            }
        }
    }

    return {};
}

- (NSArray<UITextSelectionRect*>*) selectionRectsForRange: (JuceUITextRange*) range
{
    if (auto* target = [self getTextInputTarget])
    {
        if (auto* comp = dynamic_cast<Component*> (target))
        {
            const auto list = target->getTextBounds ([range range]);

            auto* result = [NSMutableArray arrayWithCapacity: (NSUInteger) list.getNumRectangles()];

            for (const auto& rect : list)
            {
                const auto areaOnDesktop = comp->localAreaToGlobal (rect);
                const auto nativeArea = convertToCGRect (detail::ScalingHelpers::scaledScreenPosToUnscaled (areaOnDesktop));

                [result addObject: [JuceUITextSelectionRect withRect: nativeArea]];
            }

            return result;
        }
    }

    return nil;
}

- (UITextPosition*) positionWithinRange: (JuceUITextRange*) range
                    farthestInDirection: (UITextLayoutDirection) direction
{
    return direction == UITextLayoutDirectionUp || direction == UITextLayoutDirectionLeft
         ? [range start]
         : [range end];
}

- (void) replaceRange: (JuceUITextRange*) range
             withText: (NSString*) text
{
    if (owner == nullptr)
        return;

    owner->stringBeingComposed.clear();

    if (auto* target = owner->findCurrentTextInputTarget())
    {
        target->setHighlightedRegion ([range range]);
        target->insertTextAtCaret (nsStringToJuce (text));
    }
}

- (void) setBaseWritingDirection: (NSWritingDirection) writingDirection
                        forRange: (UITextRange*) range
{
}

- (NSString*) textInRange: (JuceUITextRange*) range
{
    if (auto* target = [self getTextInputTarget])
        return juceStringToNS (target->getTextInRange ([range range]));

    return nil;
}

- (UITextRange*) textRangeFromPosition: (JuceUITextPosition*) fromPosition
                            toPosition: (JuceUITextPosition*) toPosition
{
    const auto from = fromPosition != nil ? fromPosition->index : 0;
    const auto to   = toPosition   != nil ? toPosition  ->index : 0;

    return [JuceUITextRange withRange: Range<int>::between (from, to)];
}

- (void) setInputDelegate: (id<UITextInputDelegate>) delegateIn
{
    delegate = delegateIn;
}

- (id<UITextInputDelegate>) inputDelegate
{
    return delegate;
}

- (UIKeyboardType) keyboardType
{
    if (auto* target = [self getTextInputTarget])
        return UIViewComponentPeer::getUIKeyboardType (target->getKeyboardType());

    return UIKeyboardTypeDefault;
}

- (UITextAutocapitalizationType) autocapitalizationType
{
    return UITextAutocapitalizationTypeNone;
}

- (UITextAutocorrectionType) autocorrectionType
{
    return UITextAutocorrectionTypeNo;
}

- (UITextSpellCheckingType) spellCheckingType
{
    return UITextSpellCheckingTypeNo;
}

- (BOOL) canBecomeFirstResponder
{
    return YES;
}

@end

//==============================================================================
@implementation JuceTextInputTokenizer

- (instancetype) initWithPeer: (UIViewComponentPeer*) peerIn
{
    [super initWithTextInput: peerIn->hiddenTextInput.get()];
    peer = peerIn;
    return self;
}

- (UITextRange*) rangeEnclosingPosition: (JuceUITextPosition*) position
                        withGranularity: (UITextGranularity) granularity
                            inDirection: (UITextDirection) direction
{
    if (granularity != UITextGranularityLine)
        return [super rangeEnclosingPosition: position withGranularity: granularity inDirection: direction];

    auto* target = peer->findCurrentTextInputTarget();

    if (target == nullptr)
        return nullptr;

    const auto numChars = target->getTotalNumChars();

    if (! isPositiveAndBelow (position->index, numChars))
        return nullptr;

    const auto allText = target->getTextInRange ({ 0, numChars });

    const auto begin = AccessibilityTextHelpers::makeCharPtrIteratorAdapter (allText.begin());
    const auto end   = AccessibilityTextHelpers::makeCharPtrIteratorAdapter (allText.end());
    const auto positionIter = begin + position->index;

    const auto nextNewlineIter = std::find (positionIter, end, '\n');
    const auto lastNewlineIter = std::find (std::make_reverse_iterator (positionIter),
                                            std::make_reverse_iterator (begin),
                                            '\n').base();

    const auto from = std::distance (begin, lastNewlineIter);
    const auto to   = std::distance (begin, nextNewlineIter);

    return [JuceUITextRange from: from to: to];
}

@end

//==============================================================================
//==============================================================================
namespace juce
{

bool KeyPress::isKeyCurrentlyDown (int keyCode)
{
    return iOSGlobals::keysCurrentlyDown.isDown (keyCode)
        || ('A' <= keyCode && keyCode <= 'Z' && iOSGlobals::keysCurrentlyDown.isDown ((int) CharacterFunctions::toLowerCase ((juce_wchar) keyCode)))
        || ('a' <= keyCode && keyCode <= 'z' && iOSGlobals::keysCurrentlyDown.isDown ((int) CharacterFunctions::toUpperCase ((juce_wchar) keyCode)));
}

Point<float> juce_lastMousePos;

//==============================================================================
UIViewComponentPeer::UIViewComponentPeer (Component& comp, int windowStyleFlags, UIView* viewToAttachTo)
    : ComponentPeer (comp, windowStyleFlags),
      isSharedWindow (viewToAttachTo != nil),
      isAppex (SystemStats::isRunningInAppExtensionSandbox())
{
    CGRect r = convertToCGRect (component.getBounds());

    view = [[JuceUIView alloc] initWithOwner: this withFrame: r];

    view.multipleTouchEnabled = YES;
    view.hidden = true;
    view.opaque = component.isOpaque();
    view.backgroundColor = [[UIColor blackColor] colorWithAlphaComponent: 0];

   #if JUCE_COREGRAPHICS_RENDER_WITH_MULTIPLE_PAINT_CALLS
    if (@available (iOS 13, *))
    {
        metalRenderer = CoreGraphicsMetalLayerRenderer::create();
        jassert (metalRenderer != nullptr);
    }
   #endif

    if ((windowStyleFlags & ComponentPeer::windowRequiresSynchronousCoreGraphicsRendering) == 0)
        [[view layer] setDrawsAsynchronously: YES];

    if (isSharedWindow)
    {
        window = [viewToAttachTo window];
        [viewToAttachTo addSubview: view];
    }
    else
    {
        r = convertToCGRect (component.getBounds());
        r.origin.y = [UIScreen mainScreen].bounds.size.height - (r.origin.y + r.size.height);

        window = [[JuceUIWindow alloc] initWithFrame: r];
        [((JuceUIWindow*) window) setOwner: this];

        controller = [[JuceUIViewController alloc] init];
        controller.view = view;
        window.rootViewController = controller;

        window.hidden = true;
        window.opaque = component.isOpaque();
        window.backgroundColor = [[UIColor blackColor] colorWithAlphaComponent: 0];

        if (component.isAlwaysOnTop())
            window.windowLevel = UIWindowLevelAlert;

        view.frame = CGRectMake (0, 0, r.size.width, r.size.height);
    }

    setTitle (component.getName());
    setVisible (component.isVisible());
}

UIViewComponentPeer::~UIViewComponentPeer()
{
    if (iOSGlobals::currentlyFocusedPeer == this)
        iOSGlobals::currentlyFocusedPeer = nullptr;

    currentTouches.deleteAllTouchesForPeer (this);

    view->owner = nullptr;
    [view removeFromSuperview];
    [view release];
    [controller release];

    if (! isSharedWindow)
    {
        [((JuceUIWindow*) window) setOwner: nil];

        if (@available (iOS 13.0, *))
            window.windowScene = nil;

        [window release];
    }
}

//==============================================================================
void UIViewComponentPeer::setVisible (bool shouldBeVisible)
{
    if (! isSharedWindow)
        window.hidden = ! shouldBeVisible;

    view.hidden = ! shouldBeVisible;
}

void UIViewComponentPeer::setTitle (const String&)
{
    // xxx is this possible?
}

void UIViewComponentPeer::setBounds (const Rectangle<int>& newBounds, const bool isNowFullScreen)
{
    fullScreen = isNowFullScreen;

    if (isSharedWindow)
    {
        CGRect r = convertToCGRect (newBounds);

        if (! approximatelyEqual (view.frame.size.width, r.size.width)
            || ! approximatelyEqual (view.frame.size.height, r.size.height))
        {
            [view setNeedsDisplay];
        }

        view.frame = r;
    }
    else
    {
        window.frame = convertToCGRect (newBounds);
        view.frame = CGRectMake (0, 0, (CGFloat) newBounds.getWidth(), (CGFloat) newBounds.getHeight());

        handleMovedOrResized();
    }
}

Rectangle<int> UIViewComponentPeer::getBounds (const bool global) const
{
    auto r = view.frame;

    if (global)
    {
        if (view.window != nil)
        {
            r = [view convertRect: r toView: view.window];
            r = [view.window convertRect: r toWindow: nil];
        }
        else if (window != nil)
        {
            r.origin.x += window.frame.origin.x;
            r.origin.y += window.frame.origin.y;
        }
    }

    return convertToRectInt (r);
}

Point<float> UIViewComponentPeer::localToGlobal (Point<float> relativePosition)
{
    return relativePosition + getBounds (true).getPosition().toFloat();
}

Point<float> UIViewComponentPeer::globalToLocal (Point<float> screenPosition)
{
    return screenPosition - getBounds (true).getPosition().toFloat();
}

void UIViewComponentPeer::setAlpha (float newAlpha)
{
    [view.window setAlpha: (CGFloat) newAlpha];
}

void UIViewComponentPeer::setFullScreen (bool shouldBeFullScreen)
{
    if (! isSharedWindow)
    {
        auto r = shouldBeFullScreen ? Desktop::getInstance().getDisplays().getPrimaryDisplay()->userArea
                                    : lastNonFullscreenBounds;

        if ((! shouldBeFullScreen) && r.isEmpty())
            r = getBounds();

        // (can't call the component's setBounds method because that'll reset our fullscreen flag)
        if (! r.isEmpty())
            setBounds (detail::ScalingHelpers::scaledScreenPosToUnscaled (component, r), shouldBeFullScreen);

        component.repaint();
    }
}

void UIViewComponentPeer::updateScreenBounds()
{
    auto& desktop = Desktop::getInstance();

    auto oldArea = component.getBounds();
    auto oldDesktop = desktop.getDisplays().getPrimaryDisplay()->userArea;

    forceDisplayUpdate();

    if (fullScreen)
    {
        fullScreen = false;
        setFullScreen (true);
    }
    else if (! isSharedWindow)
    {
        auto newDesktop = desktop.getDisplays().getPrimaryDisplay()->userArea;

        if (newDesktop != oldDesktop)
        {
            // this will re-centre the window, but leave its size unchanged

            auto centreRelX = (float) oldArea.getCentreX() / (float) oldDesktop.getWidth();
            auto centreRelY = (float) oldArea.getCentreY() / (float) oldDesktop.getHeight();

            auto x = ((int) ((float) newDesktop.getWidth()  * centreRelX)) - (oldArea.getWidth()  / 2);
            auto y = ((int) ((float) newDesktop.getHeight() * centreRelY)) - (oldArea.getHeight() / 2);

            component.setBounds (oldArea.withPosition (x, y));
        }
    }

    [view setNeedsDisplay];
}

bool UIViewComponentPeer::contains (Point<int> localPos, bool trueIfInAChildWindow) const
{
    if (! detail::ScalingHelpers::scaledScreenPosToUnscaled (component, component.getLocalBounds()).contains (localPos))
        return false;

    UIView* v = [view hitTest: convertToCGPoint (localPos)
                    withEvent: nil];

    if (trueIfInAChildWindow)
        return v != nil;

    return v == view;
}

bool UIViewComponentPeer::setAlwaysOnTop (bool alwaysOnTop)
{
    if (! isSharedWindow)
        window.windowLevel = alwaysOnTop ? UIWindowLevelAlert : UIWindowLevelNormal;

    return true;
}

void UIViewComponentPeer::toFront (bool makeActiveWindow)
{
    if (isSharedWindow)
        [[view superview] bringSubviewToFront: view];

    if (makeActiveWindow && window != nil && component.isVisible())
        [window makeKeyAndVisible];
}

void UIViewComponentPeer::toBehind (ComponentPeer* other)
{
    if (auto* otherPeer = dynamic_cast<UIViewComponentPeer*> (other))
    {
        if (isSharedWindow)
            [[view superview] insertSubview: view belowSubview: otherPeer->view];
    }
    else
    {
        jassertfalse; // wrong type of window?
    }
}

void UIViewComponentPeer::setIcon (const Image& /*newIcon*/)
{
    // to do..
}

//==============================================================================
static float getMaximumTouchForce (UITouch* touch) noexcept
{
    if ([touch respondsToSelector: @selector (maximumPossibleForce)])
        return (float) touch.maximumPossibleForce;

    return 0.0f;
}

static float getTouchForce (UITouch* touch) noexcept
{
    if ([touch respondsToSelector: @selector (force)])
        return (float) touch.force;

    return 0.0f;
}

void UIViewComponentPeer::handleTouches (UIEvent* event, MouseEventFlags mouseEventFlags)
{
    if (event == nullptr)
        return;

    if (@available (iOS 13.4, *))
    {
        updateModifiers ([event modifierFlags]);
    }

    NSArray* touches = [[event touchesForView: view] allObjects];

    for (unsigned int i = 0; i < [touches count]; ++i)
    {
        UITouch* touch = [touches objectAtIndex: i];
        auto maximumForce = getMaximumTouchForce (touch);

        if ([touch phase] == UITouchPhaseStationary && maximumForce <= 0)
            continue;

        auto pos = convertToPointFloat ([touch locationInView: view]);
        juce_lastMousePos = pos + getBounds (true).getPosition().toFloat();

        auto time = getMouseTime (event);
        auto touchIndex = currentTouches.getIndexOfTouch (this, touch);

        auto modsToSend = ModifierKeys::currentModifiers;

        auto isUp = [] (MouseEventFlags m)
        {
            return m == MouseEventFlags::up || m == MouseEventFlags::upAndCancel;
        };

        if (mouseEventFlags == MouseEventFlags::down)
        {
            if ([touch phase] != UITouchPhaseBegan)
                continue;

            ModifierKeys::currentModifiers = ModifierKeys::currentModifiers.withoutMouseButtons().withFlags (ModifierKeys::leftButtonModifier);
            modsToSend = ModifierKeys::currentModifiers;

            // this forces a mouse-enter/up event, in case for some reason we didn't get a mouse-up before.
            handleMouseEvent (MouseInputSource::InputSourceType::touch, pos, modsToSend.withoutMouseButtons(),
                              MouseInputSource::defaultPressure, MouseInputSource::defaultOrientation, time, {}, touchIndex);

            if (! isValidPeer (this)) // (in case this component was deleted by the event)
                return;
        }
        else if (isUp (mouseEventFlags))
        {
            if (! ([touch phase] == UITouchPhaseEnded || [touch phase] == UITouchPhaseCancelled))
                continue;

            modsToSend = modsToSend.withoutMouseButtons();
            currentTouches.clearTouch (touchIndex);

            if (! currentTouches.areAnyTouchesActive())
                mouseEventFlags = MouseEventFlags::upAndCancel;
        }

        if (mouseEventFlags == MouseEventFlags::upAndCancel)
        {
            currentTouches.clearTouch (touchIndex);
            modsToSend = ModifierKeys::currentModifiers = ModifierKeys::currentModifiers.withoutMouseButtons();
        }

        // NB: some devices return 0 or 1.0 if pressure is unknown, so we'll clip our value to a believable range:
        auto pressure = maximumForce > 0 ? jlimit (0.0001f, 0.9999f, getTouchForce (touch) / maximumForce)
                                         : MouseInputSource::defaultPressure;

        handleMouseEvent (MouseInputSource::InputSourceType::touch,
                          pos, modsToSend, pressure, MouseInputSource::defaultOrientation, time, { }, touchIndex);

        if (! isValidPeer (this)) // (in case this component was deleted by the event)
            return;

        if (isUp (mouseEventFlags))
        {
            handleMouseEvent (MouseInputSource::InputSourceType::touch, MouseInputSource::offscreenMousePos, modsToSend,
                              MouseInputSource::defaultPressure, MouseInputSource::defaultOrientation, time, {}, touchIndex);

            if (! isValidPeer (this))
                return;
        }
    }
}

void UIViewComponentPeer::onHover (UIHoverGestureRecognizer* gesture)
{
    auto pos = convertToPointFloat ([gesture locationInView: view]);
    juce_lastMousePos = pos + getBounds (true).getPosition().toFloat();

    handleMouseEvent (MouseInputSource::InputSourceType::touch,
                      pos,
                      ModifierKeys::currentModifiers,
                      MouseInputSource::defaultPressure, MouseInputSource::defaultOrientation,
                      UIViewComponentPeer::getMouseTime ([[NSProcessInfo processInfo] systemUptime]),
                      {});
}

void UIViewComponentPeer::onScroll (UIPanGestureRecognizer* gesture)
{
    const auto offset = [gesture translationInView: view];
    const auto scale = 0.5f / 256.0f;

    MouseWheelDetails details;
    details.deltaX = scale * (float) offset.x;
    details.deltaY = scale * (float) offset.y;
    details.isReversed = false;
    details.isSmooth = true;
    details.isInertial = false;

    const auto reconstructedMousePosition = convertToPointFloat ([gesture locationInView: view]) - convertToPointFloat (offset);

    handleMouseWheel (MouseInputSource::InputSourceType::touch,
                      reconstructedMousePosition,
                      UIViewComponentPeer::getMouseTime ([[NSProcessInfo processInfo] systemUptime]),
                      details);
}

//==============================================================================
void UIViewComponentPeer::viewFocusGain()
{
    if (iOSGlobals::currentlyFocusedPeer != this)
    {
        if (ComponentPeer::isValidPeer (iOSGlobals::currentlyFocusedPeer))
            iOSGlobals::currentlyFocusedPeer->handleFocusLoss();

        iOSGlobals::currentlyFocusedPeer = this;

        handleFocusGain();
    }
}

void UIViewComponentPeer::viewFocusLoss()
{
    if (iOSGlobals::currentlyFocusedPeer == this)
    {
        iOSGlobals::currentlyFocusedPeer = nullptr;
        handleFocusLoss();
    }
}

bool UIViewComponentPeer::isFocused() const
{
    if (isAppex)
        return true;

    return isSharedWindow ? this == iOSGlobals::currentlyFocusedPeer
                          : (window != nil && [window isKeyWindow]);
}

void UIViewComponentPeer::grabFocus()
{
    if (window != nil)
    {
        [window makeKeyWindow];
        viewFocusGain();
    }
}

void UIViewComponentPeer::textInputRequired (Point<int>, TextInputTarget&)
{
    // We need to restart the text input session so that the keyboard can change types if necessary.
    if ([hiddenTextInput.get() isFirstResponder])
        [hiddenTextInput.get() resignFirstResponder];

    [hiddenTextInput.get() becomeFirstResponder];
}

void UIViewComponentPeer::closeInputMethodContext()
{
    if (auto* input = hiddenTextInput.get())
    {
        if (auto* delegate = [input inputDelegate])
        {
            [delegate selectionWillChange: input];
            [delegate selectionDidChange:  input];
        }
    }
}

void UIViewComponentPeer::dismissPendingTextInput()
{
    closeInputMethodContext();
    [hiddenTextInput.get() resignFirstResponder];
}

//==============================================================================
void UIViewComponentPeer::displayLinkCallback()
{
    vBlankListeners.call ([] (auto& l) { l.onVBlank(); });

    if (deferredRepaints.isEmpty())
        return;

    for (const auto& r : deferredRepaints)
        [view setNeedsDisplayInRect: convertToCGRect (r)];

   #if JUCE_COREGRAPHICS_RENDER_WITH_MULTIPLE_PAINT_CALLS
    if (metalRenderer == nullptr)
   #endif
        deferredRepaints.clear();
}

//==============================================================================
void UIViewComponentPeer::drawRect (CGRect r)
{
    if (r.size.width < 1.0f || r.size.height < 1.0f)
        return;

    drawRectWithContext (UIGraphicsGetCurrentContext(), r);
}

void UIViewComponentPeer::drawRectWithContext (CGContextRef cg, CGRect)
{
    if (! component.isOpaque())
        CGContextClearRect (cg, CGContextGetClipBoundingBox (cg));

    CGContextConcatCTM (cg, CGAffineTransformMake (1, 0, 0, -1, 0, getComponent().getHeight()));
    CoreGraphicsContext g (cg, (float) getComponent().getHeight());

    insideDrawRect = true;
    handlePaint (g);
    insideDrawRect = false;
}

bool UIViewComponentPeer::canBecomeKeyWindow()
{
    return (getStyleFlags() & juce::ComponentPeer::windowIgnoresKeyPresses) == 0;
}

//==============================================================================
void Desktop::setKioskComponent (Component* kioskModeComp, bool enableOrDisable, bool /*allowMenusAndBars*/)
{
    displays->refresh();

    if (auto* peer = kioskModeComp->getPeer())
    {
        if (auto* uiViewPeer = dynamic_cast<UIViewComponentPeer*> (peer))
            [uiViewPeer->controller setNeedsStatusBarAppearanceUpdate];

        peer->setFullScreen (enableOrDisable);
    }
}

void Desktop::allowedOrientationsChanged()
{
   #if defined (__IPHONE_16_0) && __IPHONE_OS_VERSION_MAX_ALLOWED >= __IPHONE_16_0
    if (@available (iOS 16.0, *))
    {
        UIApplication* sharedApplication = [UIApplication sharedApplication];

        const NSUniquePtr<UIWindowSceneGeometryPreferencesIOS> preferences { [UIWindowSceneGeometryPreferencesIOS alloc] };
        [preferences.get() initWithInterfaceOrientations: Orientations::getSupportedOrientations()];

        for (UIScene* scene in [sharedApplication connectedScenes])
        {
            if ([scene isKindOfClass: [UIWindowScene class]])
            {
                auto* windowScene = static_cast<UIWindowScene*> (scene);

                for (UIWindow* window in [windowScene windows])
                    if (auto* vc = [window rootViewController])
                        [vc setNeedsUpdateOfSupportedInterfaceOrientations];

                [windowScene requestGeometryUpdateWithPreferences: preferences.get()
                                                     errorHandler: ^([[maybe_unused]] NSError* error)
                 {
                    // Failed to set the new set of supported orientations.
                    // You may have hit this assertion because you're trying to restrict the supported orientations
                    // of an app that allows multitasking (i.e. the app does not require fullscreen, and supports
                    // all orientations).
                    // iPadOS apps that allow multitasking must support all interface orientations,
                    // so attempting to change the set of supported orientations will fail.
                    // If you hit this assertion in an application that requires fullscreen, it may be because the
                    // set of supported orientations declared in the app's plist doesn't have any entries in common
                    // with the orientations passed to Desktop::setOrientationsEnabled.
                    DBG (nsStringToJuce ([error localizedDescription]));
                    jassertfalse;
                }];
            }
        }

        return;
    }
   #endif

    // if the current orientation isn't allowed anymore then switch orientations
    if (! isOrientationEnabled (getCurrentOrientation()))
    {
        auto newOrientation = [this]
        {
            for (auto orientation : { upright, upsideDown, rotatedClockwise, rotatedAntiClockwise })
                if (isOrientationEnabled (orientation))
                    return orientation;

            // you need to support at least one orientation
            jassertfalse;
            return upright;
        }();

        NSNumber* value = [NSNumber numberWithInt: (int) Orientations::convertFromJuce (newOrientation)];
        [[UIDevice currentDevice] setValue:value forKey:@"orientation"];
        [value release];
    }
}

//==============================================================================
void UIViewComponentPeer::repaint (const Rectangle<int>& area)
{
    if (insideDrawRect || ! MessageManager::getInstance()->isThisTheMessageThread())
    {
        (new AsyncRepaintMessage (this, area))->post();
        return;
    }

    deferredRepaints.add (area.toFloat());
}

void UIViewComponentPeer::performAnyPendingRepaintsNow()
{
}

ComponentPeer* Component::createNewPeer (int styleFlags, void* windowToAttachTo)
{
    return new UIViewComponentPeer (*this, styleFlags, (UIView*) windowToAttachTo);
}

//==============================================================================
const int KeyPress::spaceKey              = ' ';
const int KeyPress::returnKey             = 0x0d;
const int KeyPress::escapeKey             = 0x1b;
const int KeyPress::backspaceKey          = 0x7f;
const int KeyPress::leftKey               = 0x1000;
const int KeyPress::rightKey              = 0x1001;
const int KeyPress::upKey                 = 0x1002;
const int KeyPress::downKey               = 0x1003;
const int KeyPress::pageUpKey             = 0x1004;
const int KeyPress::pageDownKey           = 0x1005;
const int KeyPress::endKey                = 0x1006;
const int KeyPress::homeKey               = 0x1007;
const int KeyPress::deleteKey             = 0x1008;
const int KeyPress::insertKey             = -1;
const int KeyPress::tabKey                = 9;
const int KeyPress::F1Key                 = 0x2001;
const int KeyPress::F2Key                 = 0x2002;
const int KeyPress::F3Key                 = 0x2003;
const int KeyPress::F4Key                 = 0x2004;
const int KeyPress::F5Key                 = 0x2005;
const int KeyPress::F6Key                 = 0x2006;
const int KeyPress::F7Key                 = 0x2007;
const int KeyPress::F8Key                 = 0x2008;
const int KeyPress::F9Key                 = 0x2009;
const int KeyPress::F10Key                = 0x200a;
const int KeyPress::F11Key                = 0x200b;
const int KeyPress::F12Key                = 0x200c;
const int KeyPress::F13Key                = 0x200d;
const int KeyPress::F14Key                = 0x200e;
const int KeyPress::F15Key                = 0x200f;
const int KeyPress::F16Key                = 0x2010;
const int KeyPress::F17Key                = 0x2011;
const int KeyPress::F18Key                = 0x2012;
const int KeyPress::F19Key                = 0x2013;
const int KeyPress::F20Key                = 0x2014;
const int KeyPress::F21Key                = 0x2015;
const int KeyPress::F22Key                = 0x2016;
const int KeyPress::F23Key                = 0x2017;
const int KeyPress::F24Key                = 0x2018;
const int KeyPress::F25Key                = 0x2019;
const int KeyPress::F26Key                = 0x201a;
const int KeyPress::F27Key                = 0x201b;
const int KeyPress::F28Key                = 0x201c;
const int KeyPress::F29Key                = 0x201d;
const int KeyPress::F30Key                = 0x201e;
const int KeyPress::F31Key                = 0x201f;
const int KeyPress::F32Key                = 0x2020;
const int KeyPress::F33Key                = 0x2021;
const int KeyPress::F34Key                = 0x2022;
const int KeyPress::F35Key                = 0x2023;
const int KeyPress::numberPad0            = 0x30020;
const int KeyPress::numberPad1            = 0x30021;
const int KeyPress::numberPad2            = 0x30022;
const int KeyPress::numberPad3            = 0x30023;
const int KeyPress::numberPad4            = 0x30024;
const int KeyPress::numberPad5            = 0x30025;
const int KeyPress::numberPad6            = 0x30026;
const int KeyPress::numberPad7            = 0x30027;
const int KeyPress::numberPad8            = 0x30028;
const int KeyPress::numberPad9            = 0x30029;
const int KeyPress::numberPadAdd          = 0x3002a;
const int KeyPress::numberPadSubtract     = 0x3002b;
const int KeyPress::numberPadMultiply     = 0x3002c;
const int KeyPress::numberPadDivide       = 0x3002d;
const int KeyPress::numberPadSeparator    = 0x3002e;
const int KeyPress::numberPadDecimalPoint = 0x3002f;
const int KeyPress::numberPadEquals       = 0x30030;
const int KeyPress::numberPadDelete       = 0x30031;
const int KeyPress::playKey               = 0x30000;
const int KeyPress::stopKey               = 0x30001;
const int KeyPress::fastForwardKey        = 0x30002;
const int KeyPress::rewindKey             = 0x30003;

} // namespace juce
