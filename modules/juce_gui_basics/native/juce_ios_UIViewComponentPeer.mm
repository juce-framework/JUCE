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

#include "juce_mac_CGMetalLayerRenderer.h"

#if TARGET_OS_SIMULATOR && JUCE_COREGRAPHICS_RENDER_WITH_MULTIPLE_PAINT_CALLS
 #warning JUCE_COREGRAPHICS_RENDER_WITH_MULTIPLE_PAINT_CALLS uses parts of the Metal API that are currently unsupported in the simulator - falling back to JUCE_COREGRAPHICS_RENDER_WITH_MULTIPLE_PAINT_CALLS=0
 #undef JUCE_COREGRAPHICS_RENDER_WITH_MULTIPLE_PAINT_CALLS
#endif

#if defined (__IPHONE_13_0) && __IPHONE_OS_VERSION_MAX_ALLOWED >= __IPHONE_13_0
 #define JUCE_HAS_IOS_POINTER_SUPPORT 1
#else
 #define JUCE_HAS_IOS_POINTER_SUPPORT 0
#endif

namespace juce
{

class UIViewComponentPeer;

static UIInterfaceOrientation getWindowOrientation()
{
    UIApplication* sharedApplication = [UIApplication sharedApplication];

   #if defined (__IPHONE_13_0) && __IPHONE_OS_VERSION_MAX_ALLOWED >= __IPHONE_13_0
    if (@available (iOS 13.0, *))
    {
        for (UIScene* scene in [sharedApplication connectedScenes])
            if ([scene isKindOfClass: [UIWindowScene class]])
                return [(UIWindowScene*) scene interfaceOrientation];
    }
   #endif

    JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wdeprecated-declarations")
    return [sharedApplication statusBarOrientation];
    JUCE_END_IGNORE_WARNINGS_GCC_LIKE
}

namespace Orientations
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


    static NSUInteger getSupportedOrientations()
    {
        NSUInteger allowed = 0;
        auto& d = Desktop::getInstance();

        if (d.isOrientationEnabled (Desktop::upright))              allowed |= UIInterfaceOrientationMaskPortrait;
        if (d.isOrientationEnabled (Desktop::upsideDown))           allowed |= UIInterfaceOrientationMaskPortraitUpsideDown;
        if (d.isOrientationEnabled (Desktop::rotatedClockwise))     allowed |= UIInterfaceOrientationMaskLandscapeLeft;
        if (d.isOrientationEnabled (Desktop::rotatedAntiClockwise)) allowed |= UIInterfaceOrientationMaskLandscapeRight;

        return allowed;
    }
}

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

struct CADisplayLinkDeleter
{
    void operator() (CADisplayLink* displayLink) const noexcept
    {
        [displayLink invalidate];
        [displayLink release];
    }
};

@interface JuceUIView : UIView <UITextViewDelegate>
{
@public
    UIViewComponentPeer* owner;
    UITextView* hiddenTextView;
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

#if JUCE_HAS_IOS_POINTER_SUPPORT
- (void) onHover: (UIHoverGestureRecognizer*) gesture API_AVAILABLE (ios (13.0));
- (void) onScroll: (UIPanGestureRecognizer*) gesture;
#endif

- (BOOL) becomeFirstResponder;
- (BOOL) resignFirstResponder;
- (BOOL) canBecomeFirstResponder;

- (BOOL) textView: (UITextView*) textView shouldChangeTextInRange: (NSRange) range replacementText: (NSString*) text;

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

class UIViewComponentPeer  : public ComponentPeer,
                             private UIViewPeerControllerReceiver
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

    BOOL textViewReplaceCharacters (Range<int>, const String&);
    void updateHiddenTextContent (TextInputTarget&);

    void updateScreenBounds();

    void handleTouches (UIEvent*, MouseEventFlags);

   #if JUCE_HAS_IOS_POINTER_SUPPORT
    API_AVAILABLE (ios (13.0)) void onHover (UIHoverGestureRecognizer*);
    void onScroll (UIPanGestureRecognizer*);
   #endif

    //==============================================================================
    void repaint (const Rectangle<int>& area) override;
    void performAnyPendingRepaintsNow() override;

    //==============================================================================
    UIWindow* window = nil;
    JuceUIView* view = nil;
    UIViewController* controller = nil;
    const bool isSharedWindow, isAppex;
    bool fullScreen = false, insideDrawRect = false;

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

private:
    void appStyleChanged() override
    {
        [controller setNeedsStatusBarAppearanceUpdate];
    }

    //==============================================================================
    class AsyncRepaintMessage  : public CallbackMessage
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

    std::unique_ptr<CoreGraphicsMetalLayerRenderer> metalRenderer;
    RectangleList<float> deferredRepaints;

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

#if defined (__IPHONE_11_0) && __IPHONE_OS_VERSION_MAX_ALLOWED >= __IPHONE_11_0
 - (BOOL) prefersHomeIndicatorAutoHidden
 {
     return isKioskModeView (self);
 }
#endif

- (UIStatusBarStyle) preferredStatusBarStyle
{
   #if defined (__IPHONE_13_0) && __IPHONE_OS_VERSION_MAX_ALLOWED >= __IPHONE_13_0
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
   #endif

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

    displayLink.reset ([CADisplayLink displayLinkWithTarget: self
                                                   selector: @selector (displayLinkCallback:)]);
    [displayLink.get() addToRunLoop: [NSRunLoop mainRunLoop]
                            forMode: NSDefaultRunLoopMode];

    hiddenTextView = [[UITextView alloc] initWithFrame: CGRectZero];
    [self addSubview: hiddenTextView];
    hiddenTextView.delegate = self;

    hiddenTextView.autocapitalizationType = UITextAutocapitalizationTypeNone;
    hiddenTextView.autocorrectionType = UITextAutocorrectionTypeNo;
    hiddenTextView.inputAssistantItem.leadingBarButtonGroups = @[];
    hiddenTextView.inputAssistantItem.trailingBarButtonGroups = @[];

   #if JUCE_HAS_IOS_POINTER_SUPPORT
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
   #endif

    return self;
}

- (void) dealloc
{
    [hiddenTextView removeFromSuperview];
    [hiddenTextView release];

    displayLink = nullptr;

    [super dealloc];
}

//==============================================================================
+ (Class) layerClass
{
   #if JUCE_COREGRAPHICS_RENDER_WITH_MULTIPLE_PAINT_CALLS
    if (@available (iOS 12, *))
        return [CAMetalLayer class];
   #endif

    return [CALayer class];
}

- (void) displayLinkCallback: (CADisplayLink*) dl
{
    if (owner != nullptr)
        owner->displayLinkCallback();
}

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

#if JUCE_HAS_IOS_POINTER_SUPPORT
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
#endif

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

- (BOOL) textView: (UITextView*) textView shouldChangeTextInRange: (NSRange) range replacementText: (NSString*) text
{
    ignoreUnused (textView);
    return owner->textViewReplaceCharacters (Range<int> ((int) range.location, (int) (range.location + range.length)),
                                             nsStringToJuce (text));
}

- (void) traitCollectionDidChange: (UITraitCollection*) previousTraitCollection
{
    [super traitCollectionDidChange: previousTraitCollection];

   #if defined (__IPHONE_12_0) && __IPHONE_OS_VERSION_MAX_ALLOWED >= __IPHONE_12_0
    if (@available (iOS 12.0, *))
    {
        const auto wasDarkModeActive = ([previousTraitCollection userInterfaceStyle] == UIUserInterfaceStyleDark);

        if (wasDarkModeActive != Desktop::getInstance().isDarkModeActive())
            [[NSNotificationCenter defaultCenter] postNotificationName: UIViewComponentPeer::getDarkModeNotificationName()
                                                                object: nil];
    }
   #endif
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

//==============================================================================
//==============================================================================
namespace juce
{

bool KeyPress::isKeyCurrentlyDown (int)
{
    return false;
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
    if (@available (iOS 12, *))
        metalRenderer = std::make_unique<CoreGraphicsMetalLayerRenderer> ((CAMetalLayer*) view.layer, comp);
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

static UIViewComponentPeer* currentlyFocusedPeer = nullptr;

UIViewComponentPeer::~UIViewComponentPeer()
{
    if (currentlyFocusedPeer == this)
        currentlyFocusedPeer = nullptr;

    currentTouches.deleteAllTouchesForPeer (this);

    view->owner = nullptr;
    [view removeFromSuperview];
    [view release];
    [controller release];

    if (! isSharedWindow)
    {
        [((JuceUIWindow*) window) setOwner: nil];

      #if defined (__IPHONE_13_0)
        if (@available (iOS 13.0, *))
            window.windowScene = nil;
      #endif

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

        if (view.frame.size.width != r.size.width || view.frame.size.height != r.size.height)
            [view setNeedsDisplay];

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
            setBounds (ScalingHelpers::scaledScreenPosToUnscaled (component, r), shouldBeFullScreen);

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

            auto centreRelX = oldArea.getCentreX() / (float) oldDesktop.getWidth();
            auto centreRelY = oldArea.getCentreY() / (float) oldDesktop.getHeight();

            auto x = ((int) (newDesktop.getWidth()  * centreRelX)) - (oldArea.getWidth()  / 2);
            auto y = ((int) (newDesktop.getHeight() * centreRelY)) - (oldArea.getHeight() / 2);

            component.setBounds (oldArea.withPosition (x, y));
        }
    }

    [view setNeedsDisplay];
}

bool UIViewComponentPeer::contains (Point<int> localPos, bool trueIfInAChildWindow) const
{
    if (! ScalingHelpers::scaledScreenPosToUnscaled (component, component.getLocalBounds()).contains (localPos))
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

#if JUCE_HAS_IOS_POINTER_SUPPORT
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

    handleMouseWheel (MouseInputSource::InputSourceType::touch,
                      convertToPointFloat ([gesture locationInView: view]),
                      UIViewComponentPeer::getMouseTime ([[NSProcessInfo processInfo] systemUptime]),
                      details);
}
#endif

//==============================================================================
void UIViewComponentPeer::viewFocusGain()
{
    if (currentlyFocusedPeer != this)
    {
        if (ComponentPeer::isValidPeer (currentlyFocusedPeer))
            currentlyFocusedPeer->handleFocusLoss();

        currentlyFocusedPeer = this;

        handleFocusGain();
    }
}

void UIViewComponentPeer::viewFocusLoss()
{
    if (currentlyFocusedPeer == this)
    {
        currentlyFocusedPeer = nullptr;
        handleFocusLoss();
    }
}

bool UIViewComponentPeer::isFocused() const
{
    if (isAppex)
        return true;

    return isSharedWindow ? this == currentlyFocusedPeer
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

void UIViewComponentPeer::textInputRequired (Point<int> pos, TextInputTarget& target)
{
    view->hiddenTextView.frame = CGRectMake (pos.x, pos.y, 0, 0);

    updateHiddenTextContent (target);
    [view->hiddenTextView becomeFirstResponder];
}

void UIViewComponentPeer::dismissPendingTextInput()
{
    closeInputMethodContext();
    [view->hiddenTextView resignFirstResponder];
}

static UIKeyboardType getUIKeyboardType (TextInputTarget::VirtualKeyboardType type) noexcept
{
    switch (type)
    {
        case TextInputTarget::textKeyboard:          return UIKeyboardTypeAlphabet;
        case TextInputTarget::numericKeyboard:       return UIKeyboardTypeNumbersAndPunctuation;
        case TextInputTarget::decimalKeyboard:       return UIKeyboardTypeNumbersAndPunctuation;
        case TextInputTarget::urlKeyboard:           return UIKeyboardTypeURL;
        case TextInputTarget::emailAddressKeyboard:  return UIKeyboardTypeEmailAddress;
        case TextInputTarget::phoneNumberKeyboard:   return UIKeyboardTypePhonePad;
        default:                                     jassertfalse; break;
    }

    return UIKeyboardTypeDefault;
}

void UIViewComponentPeer::updateHiddenTextContent (TextInputTarget& target)
{
    view->hiddenTextView.keyboardType = getUIKeyboardType (target.getKeyboardType());
    view->hiddenTextView.text = juceStringToNS (target.getTextInRange (Range<int> (0, target.getHighlightedRegion().getStart())));
    view->hiddenTextView.selectedRange = NSMakeRange ((NSUInteger) target.getHighlightedRegion().getStart(), 0);
}

BOOL UIViewComponentPeer::textViewReplaceCharacters (Range<int> range, const String& text)
{
    if (auto* target = findCurrentTextInputTarget())
    {
        auto currentSelection = target->getHighlightedRegion();

        if (range.getLength() == 1 && text.isEmpty()) // (detect backspace)
            if (currentSelection.isEmpty())
                target->setHighlightedRegion (currentSelection.withStart (currentSelection.getStart() - 1));

        WeakReference<Component> deletionChecker (dynamic_cast<Component*> (target));

        if (text == "\r" || text == "\n" || text == "\r\n")
            handleKeyPress (KeyPress::returnKey, text[0]);
        else
            target->insertTextAtCaret (text);

        if (deletionChecker != nullptr)
            updateHiddenTextContent (*target);
    }

    return NO;
}

//==============================================================================
void UIViewComponentPeer::displayLinkCallback()
{
    if (deferredRepaints.isEmpty())
        return;

    auto dispatchRectangles = [this] ()
    {
        // We shouldn't need this preprocessor guard, but when running in the simulator
        // CAMetalLayer is flagged as requiring iOS 13
       #if JUCE_COREGRAPHICS_RENDER_WITH_MULTIPLE_PAINT_CALLS
        if (metalRenderer != nullptr)
        {
            if (@available (iOS 12, *))
            {
                return metalRenderer->drawRectangleList ((CAMetalLayer*) view.layer,
                                                         (float) view.contentScaleFactor,
                                                         view.frame,
                                                         component,
                                                         [this] (CGContextRef ctx, CGRect r) { drawRectWithContext (ctx, r); },
                                                         deferredRepaints);
            }

            // The creation of metalRenderer should already be guarded with @available (iOS 12, *).
            jassertfalse;
            return false;
        }
       #endif

        for (const auto& r : deferredRepaints)
            [view setNeedsDisplayInRect: convertToCGRect (r)];

        return true;
    };

    if (dispatchRectangles())
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
    CoreGraphicsContext g (cg, getComponent().getHeight());

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
