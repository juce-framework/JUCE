/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

class UIViewComponentPeer;

// The way rotation works changed in iOS8..
static bool isUsingOldRotationMethod() noexcept
{
    static bool isPreV8 = ([[[UIDevice currentDevice] systemVersion] compare: @"8.0"
                                                                     options: NSNumericSearch] == NSOrderedAscending);
    return isPreV8;
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
            default:                                        jassertfalse; // unknown orientation!
        }

        return UIInterfaceOrientationPortrait;
    }

    static CGAffineTransform getCGTransformFor (const Desktop::DisplayOrientation orientation) noexcept
    {
        if (isUsingOldRotationMethod())
        {
            switch (orientation)
            {
                case Desktop::upsideDown:             return CGAffineTransformMake (-1, 0,  0, -1, 0, 0);
                case Desktop::rotatedClockwise:       return CGAffineTransformMake (0, -1,  1,  0, 0, 0);
                case Desktop::rotatedAntiClockwise:   return CGAffineTransformMake (0,  1, -1,  0, 0, 0);
                default: break;
            }
        }

        return CGAffineTransformIdentity;
    }

    static NSUInteger getSupportedOrientations()
    {
        NSUInteger allowed = 0;
        Desktop& d = Desktop::getInstance();

        if (d.isOrientationEnabled (Desktop::upright))              allowed |= UIInterfaceOrientationMaskPortrait;
        if (d.isOrientationEnabled (Desktop::upsideDown))           allowed |= UIInterfaceOrientationMaskPortraitUpsideDown;
        if (d.isOrientationEnabled (Desktop::rotatedClockwise))     allowed |= UIInterfaceOrientationMaskLandscapeLeft;
        if (d.isOrientationEnabled (Desktop::rotatedAntiClockwise)) allowed |= UIInterfaceOrientationMaskLandscapeRight;

        return allowed;
    }
}

//==============================================================================
} // namespace juce

using namespace juce;


@interface JuceUIView : UIView <UITextViewDelegate>
{
@public
    UIViewComponentPeer* owner;
    UITextView* hiddenTextView;
}

- (JuceUIView*) initWithOwner: (UIViewComponentPeer*) owner withFrame: (CGRect) frame;
- (void) dealloc;

- (void) drawRect: (CGRect) r;

- (void) touchesBegan:     (NSSet*) touches  withEvent: (UIEvent*) event;
- (void) touchesMoved:     (NSSet*) touches  withEvent: (UIEvent*) event;
- (void) touchesEnded:     (NSSet*) touches  withEvent: (UIEvent*) event;
- (void) touchesCancelled: (NSSet*) touches  withEvent: (UIEvent*) event;

- (BOOL) becomeFirstResponder;
- (BOOL) resignFirstResponder;
- (BOOL) canBecomeFirstResponder;

- (BOOL) textView: (UITextView*) textView shouldChangeTextInRange: (NSRange) range replacementText: (NSString*) text;
@end

//==============================================================================
@interface JuceUIViewController : UIViewController
{
}

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
    virtual ~UIViewPeerControllerReceiver();
    virtual void setViewController (UIViewController*) = 0;
};

UIViewPeerControllerReceiver::~UIViewPeerControllerReceiver() {}

class UIViewComponentPeer  : public ComponentPeer,
                             public FocusChangeListener,
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

    Rectangle<int> getBounds() const override               { return getBounds (! isSharedWindow); }
    Rectangle<int> getBounds (bool global) const;
    Point<float> localToGlobal (Point<float> relativePosition) override;
    using ComponentPeer::localToGlobal;
    Point<float> globalToLocal (Point<float> screenPosition) override;
    using ComponentPeer::globalToLocal;
    void setAlpha (float newAlpha) override;
    void setMinimised (bool) override                       {}
    bool isMinimised() const override                       { return false; }
    void setFullScreen (bool shouldBeFullScreen) override;
    bool isFullScreen() const override                      { return fullScreen; }
    bool contains (Point<int> localPos, bool trueIfInAChildWindow) const override;
    BorderSize<int> getFrameSize() const override           { return BorderSize<int>(); }
    bool setAlwaysOnTop (bool alwaysOnTop) override;
    void toFront (bool makeActiveWindow) override;
    void toBehind (ComponentPeer* other) override;
    void setIcon (const Image& newIcon) override;
    StringArray getAvailableRenderingEngines() override     { return StringArray ("CoreGraphics Renderer"); }

    void drawRect (CGRect);
    bool canBecomeKeyWindow();

    //==============================================================================
    void viewFocusGain();
    void viewFocusLoss();
    bool isFocused() const override;
    void grabFocus() override;
    void textInputRequired (Point<int>, TextInputTarget&) override;

    BOOL textViewReplaceCharacters (Range<int>, const String&);
    void updateHiddenTextContent (TextInputTarget*);
    void globalFocusChanged (Component*) override;

    void updateTransformAndScreenBounds();

    void handleTouches (UIEvent*, bool isDown, bool isUp, bool isCancel);

    //==============================================================================
    void repaint (const Rectangle<int>& area) override;
    void performAnyPendingRepaintsNow() override;

    //==============================================================================
    UIWindow* window;
    JuceUIView* view;
    UIViewController* controller;
    bool isSharedWindow, fullScreen, insideDrawRect, isAppex;

    static int64 getMouseTime (UIEvent* e) noexcept
    {
        return (Time::currentTimeMillis() - Time::getMillisecondCounter())
                + (int64) ([e timestamp] * 1000.0);
    }

    static Rectangle<int> rotatedScreenPosToReal (const Rectangle<int>& r)
    {
        if (! SystemStats::isRunningInAppExtensionSandbox() && isUsingOldRotationMethod())
        {
            const Rectangle<int> screen (convertToRectInt ([UIScreen mainScreen].bounds));

            switch ([[UIApplication sharedApplication] statusBarOrientation])
            {
                case UIInterfaceOrientationPortrait:
                    return r;

                case UIInterfaceOrientationPortraitUpsideDown:
                    return Rectangle<int> (screen.getWidth() - r.getRight(), screen.getHeight() - r.getBottom(),
                                           r.getWidth(), r.getHeight());

                case UIInterfaceOrientationLandscapeLeft:
                    return Rectangle<int> (r.getY(), screen.getHeight() - r.getRight(),
                                           r.getHeight(), r.getWidth());

                case UIInterfaceOrientationLandscapeRight:
                    return Rectangle<int> (screen.getWidth() - r.getBottom(), r.getX(),
                                           r.getHeight(), r.getWidth());

                default: jassertfalse; // unknown orientation!
            }
        }

        return r;
    }

    static Rectangle<int> realScreenPosToRotated (const Rectangle<int>& r)
    {
        if (! SystemStats::isRunningInAppExtensionSandbox() && isUsingOldRotationMethod())
        {
            const Rectangle<int> screen (convertToRectInt ([UIScreen mainScreen].bounds));

            switch ([[UIApplication sharedApplication] statusBarOrientation])
            {
                case UIInterfaceOrientationPortrait:
                    return r;

                case UIInterfaceOrientationPortraitUpsideDown:
                    return Rectangle<int> (screen.getWidth() - r.getRight(), screen.getHeight() - r.getBottom(),
                                           r.getWidth(), r.getHeight());

                case UIInterfaceOrientationLandscapeLeft:
                    return Rectangle<int> (screen.getHeight() - r.getBottom(), r.getX(),
                                           r.getHeight(), r.getWidth());

                case UIInterfaceOrientationLandscapeRight:
                    return Rectangle<int> (r.getY(), screen.getWidth() - r.getRight(),
                                           r.getHeight(), r.getWidth());

                default: jassertfalse; // unknown orientation!
            }
        }

        return r;
    }

    static MultiTouchMapper<UITouch*> currentTouches;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (UIViewComponentPeer)

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
};

static void sendScreenBoundsUpdate (JuceUIViewController* c)
{
    JuceUIView* juceView = (JuceUIView*) [c view];

    if (juceView != nil && juceView->owner != nullptr)
        juceView->owner->updateTransformAndScreenBounds();
}

static bool isKioskModeView (JuceUIViewController* c)
{
    JuceUIView* juceView = (JuceUIView*) [c view];
    jassert (juceView != nil && juceView->owner != nullptr);

    return Desktop::getInstance().getKioskModeComponent() == &(juceView->owner->getComponent());
}

MultiTouchMapper<UITouch*> UIViewComponentPeer::currentTouches;

} // namespace juce

//==============================================================================
//==============================================================================
@implementation JuceUIViewController

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
    sendScreenBoundsUpdate (self);

    // On some devices the screen-size isn't yet updated at this point, so also trigger another
    // async update to double-check..
    MessageManager::callAsync ([=] { sendScreenBoundsUpdate (self); });
}

- (BOOL) prefersStatusBarHidden
{
    return isKioskModeView (self);
}

#if defined (__IPHONE_11_0) && __IPHONE_OS_VERSION_MAX_ALLOWED >= __IPHONE_11_0
 - (BOOL) prefersHomeIndicatorAutoHidden
 {
     return isKioskModeView (self);
 }
#endif

- (UIStatusBarStyle) preferredStatusBarStyle
{
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

    hiddenTextView = [[UITextView alloc] initWithFrame: CGRectZero];
    [self addSubview: hiddenTextView];
    hiddenTextView.delegate = self;

    hiddenTextView.autocapitalizationType = UITextAutocapitalizationTypeNone;
    hiddenTextView.autocorrectionType = UITextAutocorrectionTypeNo;

    return self;
}

- (void) dealloc
{
    [hiddenTextView removeFromSuperview];
    [hiddenTextView release];

    [super dealloc];
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
        owner->handleTouches (event, true, false, false);
}

- (void) touchesMoved: (NSSet*) touches withEvent: (UIEvent*) event
{
    ignoreUnused (touches);

    if (owner != nullptr)
        owner->handleTouches (event, false, false, false);
}

- (void) touchesEnded: (NSSet*) touches withEvent: (UIEvent*) event
{
    ignoreUnused (touches);

    if (owner != nullptr)
        owner->handleTouches (event, false, true, false);
}

- (void) touchesCancelled: (NSSet*) touches withEvent: (UIEvent*) event
{
    if (owner != nullptr)
        owner->handleTouches (event, false, true, true);

    [self touchesEnded: touches withEvent: event];
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

- (BOOL) textView: (UITextView*) textView shouldChangeTextInRange: (NSRange) range replacementText: (NSString*) text
{
    ignoreUnused (textView);
    return owner->textViewReplaceCharacters (Range<int> ((int) range.location, (int) (range.location + range.length)),
                                             nsStringToJuce (text));
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
UIViewComponentPeer::UIViewComponentPeer (Component& comp, const int windowStyleFlags, UIView* viewToAttachTo)
    : ComponentPeer (comp, windowStyleFlags),
      window (nil),
      view (nil),
      controller (nil),
      isSharedWindow (viewToAttachTo != nil),
      fullScreen (false),
      insideDrawRect (false),
      isAppex (SystemStats::isRunningInAppExtensionSandbox())
{
    CGRect r = convertToCGRect (component.getBounds());

    view = [[JuceUIView alloc] initWithOwner: this withFrame: r];

    view.multipleTouchEnabled = YES;
    view.hidden = true;
    view.opaque = component.isOpaque();
    view.backgroundColor = [[UIColor blackColor] colorWithAlphaComponent: 0];
    view.transform = CGAffineTransformIdentity;

    if (isSharedWindow)
    {
        window = [viewToAttachTo window];
        [viewToAttachTo addSubview: view];
    }
    else
    {
        r = convertToCGRect (rotatedScreenPosToReal (component.getBounds()));
        r.origin.y = [UIScreen mainScreen].bounds.size.height - (r.origin.y + r.size.height);

        window = [[JuceUIWindow alloc] initWithFrame: r];
        [((JuceUIWindow*) window) setOwner: this];

        controller = [[JuceUIViewController alloc] init];
        controller.view = view;
        window.rootViewController = controller;

        window.hidden = true;
        window.transform = Orientations::getCGTransformFor (Desktop::getInstance().getCurrentOrientation());
        window.opaque = component.isOpaque();
        window.backgroundColor = [[UIColor blackColor] colorWithAlphaComponent: 0];

        if (component.isAlwaysOnTop())
            window.windowLevel = UIWindowLevelAlert;

        view.frame = CGRectMake (0, 0, r.size.width, r.size.height);

        [window addSubview: view];
    }

    setTitle (component.getName());
    setVisible (component.isVisible());

    Desktop::getInstance().addFocusChangeListener (this);
}

UIViewComponentPeer::~UIViewComponentPeer()
{
    currentTouches.deleteAllTouchesForPeer (this);
    Desktop::getInstance().removeFocusChangeListener (this);

    view->owner = nullptr;
    [view removeFromSuperview];
    [view release];
    [controller release];

    if (! isSharedWindow)
    {
        [((JuceUIWindow*) window) setOwner: nil];
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
        window.frame = convertToCGRect (rotatedScreenPosToReal (newBounds));
        view.frame = CGRectMake (0, 0, (CGFloat) newBounds.getWidth(), (CGFloat) newBounds.getHeight());

        handleMovedOrResized();
    }
}

Rectangle<int> UIViewComponentPeer::getBounds (const bool global) const
{
    CGRect r = view.frame;

    if (global && view.window != nil)
    {
        r = [view convertRect: r toView: view.window];
        r = [view.window convertRect: r toWindow: nil];

        return realScreenPosToRotated (convertToRectInt (r));
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
        Rectangle<int> r (shouldBeFullScreen ? Desktop::getInstance().getDisplays().getMainDisplay().userArea
                                             : lastNonFullscreenBounds);

        if ((! shouldBeFullScreen) && r.isEmpty())
            r = getBounds();

        // (can't call the component's setBounds method because that'll reset our fullscreen flag)
        if (! r.isEmpty())
            setBounds (ScalingHelpers::scaledScreenPosToUnscaled (component, r), shouldBeFullScreen);

        component.repaint();
    }
}

void UIViewComponentPeer::updateTransformAndScreenBounds()
{
    Desktop& desktop = Desktop::getInstance();
    const Rectangle<int> oldArea (component.getBounds());
    const Rectangle<int> oldDesktop (desktop.getDisplays().getMainDisplay().userArea);

    const_cast<Displays&> (desktop.getDisplays()).refresh();

    window.transform = Orientations::getCGTransformFor (desktop.getCurrentOrientation());
    view.transform = CGAffineTransformIdentity;

    if (fullScreen)
    {
        fullScreen = false;
        setFullScreen (true);
    }
    else if (! isSharedWindow)
    {
        // this will re-centre the window, but leave its size unchanged
        const float centreRelX = oldArea.getCentreX() / (float) oldDesktop.getWidth();
        const float centreRelY = oldArea.getCentreY() / (float) oldDesktop.getHeight();

        const Rectangle<int> newDesktop (desktop.getDisplays().getMainDisplay().userArea);

        const int x = ((int) (newDesktop.getWidth()  * centreRelX)) - (oldArea.getWidth()  / 2);
        const int y = ((int) (newDesktop.getHeight() * centreRelY)) - (oldArea.getHeight() / 2);

        component.setBounds (oldArea.withPosition (x, y));
    }

    [view setNeedsDisplay];
}

bool UIViewComponentPeer::contains (Point<int> localPos, bool trueIfInAChildWindow) const
{
    {
        Rectangle<int> localBounds =
            ScalingHelpers::scaledScreenPosToUnscaled (component, component.getLocalBounds());

        if (! localBounds.contains (localPos))
            return false;
    }

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
    if (UIViewComponentPeer* const otherPeer = dynamic_cast<UIViewComponentPeer*> (other))
    {
        if (isSharedWindow)
        {
            [[view superview] insertSubview: view belowSubview: otherPeer->view];
        }
        else
        {
            // don't know how to do this
        }
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
   #if defined (__IPHONE_9_0) && __IPHONE_OS_VERSION_MAX_ALLOWED >= __IPHONE_9_0
    if ([touch respondsToSelector: @selector (maximumPossibleForce)])
        return (float) touch.maximumPossibleForce;
   #endif

    ignoreUnused (touch);
    return 0.0f;
}

static float getTouchForce (UITouch* touch) noexcept
{
   #if defined (__IPHONE_9_0) && __IPHONE_OS_VERSION_MAX_ALLOWED >= __IPHONE_9_0
    if ([touch respondsToSelector: @selector (force)])
        return (float) touch.force;
   #endif

    ignoreUnused (touch);
    return 0.0f;
}

void UIViewComponentPeer::handleTouches (UIEvent* event, const bool isDown, const bool isUp, bool isCancel)
{
    NSArray* touches = [[event touchesForView: view] allObjects];

    for (unsigned int i = 0; i < [touches count]; ++i)
    {
        UITouch* touch = [touches objectAtIndex: i];
        const float maximumForce = getMaximumTouchForce (touch);

        if ([touch phase] == UITouchPhaseStationary && maximumForce <= 0)
            continue;

        CGPoint p = [touch locationInView: view];
        const Point<float> pos (static_cast<float> (p.x), static_cast<float> (p.y));
        juce_lastMousePos = pos + getBounds (true).getPosition().toFloat();

        const int64 time = getMouseTime (event);
        const int touchIndex = currentTouches.getIndexOfTouch (this, touch);

        ModifierKeys modsToSend (ModifierKeys::currentModifiers);

        if (isDown)
        {
            if ([touch phase] != UITouchPhaseBegan)
                continue;

            ModifierKeys::currentModifiers = ModifierKeys::currentModifiers.withoutMouseButtons().withFlags (ModifierKeys::leftButtonModifier);
            modsToSend = ModifierKeys::currentModifiers;

            // this forces a mouse-enter/up event, in case for some reason we didn't get a mouse-up before.
            handleMouseEvent (MouseInputSource::InputSourceType::touch, pos, modsToSend.withoutMouseButtons(),
                              MouseInputSource::invalidPressure, MouseInputSource::invalidOrientation, time, {}, touchIndex);

            if (! isValidPeer (this)) // (in case this component was deleted by the event)
                return;
        }
        else if (isUp)
        {
            if (! ([touch phase] == UITouchPhaseEnded || [touch phase] == UITouchPhaseCancelled))
                continue;

            modsToSend = modsToSend.withoutMouseButtons();
            currentTouches.clearTouch (touchIndex);

            if (! currentTouches.areAnyTouchesActive())
                isCancel = true;
        }

        if (isCancel)
        {
            currentTouches.clearTouch (touchIndex);
            modsToSend = ModifierKeys::currentModifiers = ModifierKeys::currentModifiers.withoutMouseButtons();
        }

        // NB: some devices return 0 or 1.0 if pressure is unknown, so we'll clip our value to a believable range:
        float pressure = maximumForce > 0 ? jlimit (0.0001f, 0.9999f, getTouchForce (touch) / maximumForce)
                                          : MouseInputSource::invalidPressure;

        handleMouseEvent (MouseInputSource::InputSourceType::touch, pos, modsToSend, pressure,
                          MouseInputSource::invalidOrientation, time, { }, touchIndex);

        if (! isValidPeer (this)) // (in case this component was deleted by the event)
            return;

        if (isUp || isCancel)
        {
            handleMouseEvent (MouseInputSource::InputSourceType::touch, MouseInputSource::offscreenMousePos, modsToSend,
                              MouseInputSource::invalidPressure, MouseInputSource::invalidOrientation, time, {}, touchIndex);

            if (! isValidPeer (this))
                return;
        }
    }
}

//==============================================================================
static UIViewComponentPeer* currentlyFocusedPeer = nullptr;

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

void UIViewComponentPeer::textInputRequired (Point<int>, TextInputTarget&)
{
}

static bool isIOS4_1() noexcept
{
    return [[[UIDevice currentDevice] systemVersion] doubleValue] >= 4.1;
}

static UIKeyboardType getUIKeyboardType (TextInputTarget::VirtualKeyboardType type) noexcept
{
    switch (type)
    {
        case TextInputTarget::textKeyboard:          return UIKeyboardTypeAlphabet;
        case TextInputTarget::numericKeyboard:       return isIOS4_1() ? UIKeyboardTypeNumberPad  : UIKeyboardTypeNumbersAndPunctuation;
        case TextInputTarget::decimalKeyboard:       return isIOS4_1() ? UIKeyboardTypeDecimalPad : UIKeyboardTypeNumbersAndPunctuation;
        case TextInputTarget::urlKeyboard:           return UIKeyboardTypeURL;
        case TextInputTarget::emailAddressKeyboard:  return UIKeyboardTypeEmailAddress;
        case TextInputTarget::phoneNumberKeyboard:   return UIKeyboardTypePhonePad;
        default:                                     jassertfalse; break;
    }

    return UIKeyboardTypeDefault;
}

void UIViewComponentPeer::updateHiddenTextContent (TextInputTarget* target)
{
    view->hiddenTextView.keyboardType = getUIKeyboardType (target->getKeyboardType());
    view->hiddenTextView.text = juceStringToNS (target->getTextInRange (Range<int> (0, target->getHighlightedRegion().getStart())));
    view->hiddenTextView.selectedRange = NSMakeRange ((NSUInteger) target->getHighlightedRegion().getStart(), 0);
}

BOOL UIViewComponentPeer::textViewReplaceCharacters (Range<int> range, const String& text)
{
    if (TextInputTarget* const target = findCurrentTextInputTarget())
    {
        const Range<int> currentSelection (target->getHighlightedRegion());

        if (range.getLength() == 1 && text.isEmpty()) // (detect backspace)
            if (currentSelection.isEmpty())
                target->setHighlightedRegion (currentSelection.withStart (currentSelection.getStart() - 1));

        if (text == "\r" || text == "\n" || text == "\r\n")
            handleKeyPress (KeyPress::returnKey, text[0]);
        else
            target->insertTextAtCaret (text);

        updateHiddenTextContent (target);
    }

    return NO;
}

void UIViewComponentPeer::globalFocusChanged (Component*)
{
    if (TextInputTarget* const target = findCurrentTextInputTarget())
    {
        Component* comp = dynamic_cast<Component*> (target);

        Point<int> pos (component.getLocalPoint (comp, Point<int>()));
        view->hiddenTextView.frame = CGRectMake (pos.x, pos.y, 0, 0);

        updateHiddenTextContent (target);
        [view->hiddenTextView becomeFirstResponder];
    }
    else
    {
        [view->hiddenTextView resignFirstResponder];
    }
}


//==============================================================================
void UIViewComponentPeer::drawRect (CGRect r)
{
    if (r.size.width < 1.0f || r.size.height < 1.0f)
        return;

    CGContextRef cg = UIGraphicsGetCurrentContext();

    if (! component.isOpaque())
        CGContextClearRect (cg, CGContextGetClipBoundingBox (cg));

    CGContextConcatCTM (cg, CGAffineTransformMake (1, 0, 0, -1, 0, getComponent().getHeight()));

    // NB the CTM on iOS already includes a factor for the display scale, so
    // we'll tell the context that the scale is 1.0 to avoid it using it twice
    CoreGraphicsContext g (cg, getComponent().getHeight(), 1.0f);

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

    if (ComponentPeer* peer = kioskModeComp->getPeer())
    {
        if (UIViewComponentPeer* uiViewPeer = dynamic_cast<UIViewComponentPeer*> (peer))
            [uiViewPeer->controller setNeedsStatusBarAppearanceUpdate];

        peer->setFullScreen (enableOrDisable);
    }
}

void Desktop::allowedOrientationsChanged()
{
    // if the current orientation isn't allowed anymore then switch orientations
    if (! isOrientationEnabled (getCurrentOrientation()))
    {
        DisplayOrientation orientations[] = { upright, upsideDown, rotatedClockwise, rotatedAntiClockwise };

        const int n = sizeof (orientations) / sizeof (DisplayOrientation);
        int i;

        for (i = 0; i < n; ++i)
            if (isOrientationEnabled (orientations[i]))
                break;


        // you need to support at least one orientation
        jassert (i < n);
        i = jmin (n - 1, i);

        NSNumber *value = [NSNumber numberWithInt: (int) Orientations::convertFromJuce (orientations[i])];
        [[UIDevice currentDevice] setValue:value forKey:@"orientation"];
        [value release];
    }
}

//==============================================================================
void UIViewComponentPeer::repaint (const Rectangle<int>& area)
{
    if (insideDrawRect || ! MessageManager::getInstance()->isThisTheMessageThread())
        (new AsyncRepaintMessage (this, area))->post();
    else
        [view setNeedsDisplayInRect: convertToCGRect (area)];
}

void UIViewComponentPeer::performAnyPendingRepaintsNow()
{
}

ComponentPeer* Component::createNewPeer (int styleFlags, void* windowToAttachTo)
{
    return new UIViewComponentPeer (*this, styleFlags, (UIView*) windowToAttachTo);
}

//==============================================================================
const int KeyPress::spaceKey        = ' ';
const int KeyPress::returnKey       = 0x0d;
const int KeyPress::escapeKey       = 0x1b;
const int KeyPress::backspaceKey    = 0x7f;
const int KeyPress::leftKey         = 0x1000;
const int KeyPress::rightKey        = 0x1001;
const int KeyPress::upKey           = 0x1002;
const int KeyPress::downKey         = 0x1003;
const int KeyPress::pageUpKey       = 0x1004;
const int KeyPress::pageDownKey     = 0x1005;
const int KeyPress::endKey          = 0x1006;
const int KeyPress::homeKey         = 0x1007;
const int KeyPress::deleteKey       = 0x1008;
const int KeyPress::insertKey       = -1;
const int KeyPress::tabKey          = 9;
const int KeyPress::F1Key           = 0x2001;
const int KeyPress::F2Key           = 0x2002;
const int KeyPress::F3Key           = 0x2003;
const int KeyPress::F4Key           = 0x2004;
const int KeyPress::F5Key           = 0x2005;
const int KeyPress::F6Key           = 0x2006;
const int KeyPress::F7Key           = 0x2007;
const int KeyPress::F8Key           = 0x2008;
const int KeyPress::F9Key           = 0x2009;
const int KeyPress::F10Key          = 0x200a;
const int KeyPress::F11Key          = 0x200b;
const int KeyPress::F12Key          = 0x200c;
const int KeyPress::F13Key          = 0x200d;
const int KeyPress::F14Key          = 0x200e;
const int KeyPress::F15Key          = 0x200f;
const int KeyPress::F16Key          = 0x2010;
const int KeyPress::F17Key          = 0x2011;
const int KeyPress::F18Key          = 0x2012;
const int KeyPress::F19Key          = 0x2013;
const int KeyPress::F20Key          = 0x2014;
const int KeyPress::F21Key          = 0x2015;
const int KeyPress::F22Key          = 0x2016;
const int KeyPress::F23Key          = 0x2017;
const int KeyPress::F24Key          = 0x2018;
const int KeyPress::F25Key          = 0x2019;
const int KeyPress::F26Key          = 0x201a;
const int KeyPress::F27Key          = 0x201b;
const int KeyPress::F28Key          = 0x201c;
const int KeyPress::F29Key          = 0x201d;
const int KeyPress::F30Key          = 0x201e;
const int KeyPress::F31Key          = 0x201f;
const int KeyPress::F32Key          = 0x2020;
const int KeyPress::F33Key          = 0x2021;
const int KeyPress::F34Key          = 0x2022;
const int KeyPress::F35Key          = 0x2023;
const int KeyPress::numberPad0      = 0x30020;
const int KeyPress::numberPad1      = 0x30021;
const int KeyPress::numberPad2      = 0x30022;
const int KeyPress::numberPad3      = 0x30023;
const int KeyPress::numberPad4      = 0x30024;
const int KeyPress::numberPad5      = 0x30025;
const int KeyPress::numberPad6      = 0x30026;
const int KeyPress::numberPad7      = 0x30027;
const int KeyPress::numberPad8      = 0x30028;
const int KeyPress::numberPad9      = 0x30029;
const int KeyPress::numberPadAdd            = 0x3002a;
const int KeyPress::numberPadSubtract       = 0x3002b;
const int KeyPress::numberPadMultiply       = 0x3002c;
const int KeyPress::numberPadDivide         = 0x3002d;
const int KeyPress::numberPadSeparator      = 0x3002e;
const int KeyPress::numberPadDecimalPoint   = 0x3002f;
const int KeyPress::numberPadEquals         = 0x30030;
const int KeyPress::numberPadDelete         = 0x30031;
const int KeyPress::playKey         = 0x30000;
const int KeyPress::stopKey         = 0x30001;
const int KeyPress::fastForwardKey  = 0x30002;
const int KeyPress::rewindKey       = 0x30003;

} // namespace juce
