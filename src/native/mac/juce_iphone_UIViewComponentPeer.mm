/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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
#ifdef JUCE_INCLUDED_FILE

class UIViewComponentPeer;

//==============================================================================
END_JUCE_NAMESPACE

#define JuceUIView MakeObjCClassName(JuceUIView)

@interface JuceUIView : UIView
{
@public
    UIViewComponentPeer* owner;
}

- (JuceUIView*) initWithOwner: (UIViewComponentPeer*) owner withFrame: (CGRect) frame;
- (void) dealloc;

- (void) drawRect: (CGRect) r;

- (void) touchesBegan: (NSSet*) touches withEvent: (UIEvent*) event;
- (void) touchesMoved: (NSSet*) touches withEvent: (UIEvent*) event;
- (void) touchesEnded: (NSSet*) touches withEvent: (UIEvent*) event;
- (void) touchesCancelled: (NSSet*) touches withEvent: (UIEvent*) event;

- (BOOL) becomeFirstResponder;
- (BOOL) resignFirstResponder;
- (BOOL) canBecomeFirstResponder;

@end

//==============================================================================
#define JuceUIWindow MakeObjCClassName(JuceUIWindow)

@interface JuceUIWindow : UIWindow
{
@private
    UIViewComponentPeer* owner;
    bool isZooming;
}

- (void) setOwner: (UIViewComponentPeer*) owner;
- (void) becomeKeyWindow;
@end

BEGIN_JUCE_NAMESPACE

//==============================================================================
class UIViewComponentPeer  : public ComponentPeer
{
public:
    UIViewComponentPeer (Component* const component,
                         const int windowStyleFlags,
                         UIView* viewToAttachTo);

    ~UIViewComponentPeer();

    //==============================================================================
    void* getNativeHandle() const;
    void setVisible (bool shouldBeVisible);
    void setTitle (const String& title);
    void setPosition (int x, int y);
    void setSize (int w, int h);
    void setBounds (int x, int y, int w, int h, const bool isNowFullScreen);
    void getBounds (int& x, int& y, int& w, int& h, const bool global) const;
    void getBounds (int& x, int& y, int& w, int& h) const;
    int getScreenX() const;
    int getScreenY() const;
    void relativePositionToGlobal (int& x, int& y);
    void globalPositionToRelative (int& x, int& y);
    void setMinimised (bool shouldBeMinimised);
    bool isMinimised() const;
    void setFullScreen (bool shouldBeFullScreen);
    bool isFullScreen() const;
    bool contains (int x, int y, bool trueIfInAChildWindow) const;
    const BorderSize getFrameSize() const;
    bool setAlwaysOnTop (bool alwaysOnTop);
    void toFront (bool makeActiveWindow);
    void toBehind (ComponentPeer* other);
    void setIcon (const Image& newIcon);

    virtual void drawRect (CGRect r);

    virtual bool canBecomeKeyWindow();
    virtual bool windowShouldClose();

    virtual void redirectMovedOrResized();
    virtual CGRect constrainRect (CGRect r);

    //==============================================================================
    virtual void viewFocusGain();
    virtual void viewFocusLoss();
    bool isFocused() const;
    void grabFocus();
    void textInputRequired (int x, int y);

    //==============================================================================
    void repaint (int x, int y, int w, int h);
    void performAnyPendingRepaintsNow();

    //==============================================================================
    juce_UseDebuggingNewOperator

    UIWindow* window;
    JuceUIView* view;
    bool isSharedWindow, fullScreen;
};

//==============================================================================
END_JUCE_NAMESPACE

@implementation JuceUIView

- (JuceUIView*) initWithOwner: (UIViewComponentPeer*) owner_
                    withFrame: (CGRect) frame
{
    [super initWithFrame: frame];
    owner = owner_;

    return self;
}

- (void) dealloc
{
    [super dealloc];
}

//==============================================================================
- (void) drawRect: (CGRect) r
{
    if (owner != 0)
        owner->drawRect (r);
}

//==============================================================================
bool KeyPress::isKeyCurrentlyDown (const int keyCode) throw()
{
    return false;
}

static int currentModifiers = 0;

const ModifierKeys ModifierKeys::getCurrentModifiersRealtime() throw()
{
    return ModifierKeys (currentModifiers);
}

void ModifierKeys::updateCurrentModifiers() throw()
{
    currentModifierFlags = currentModifiers;
}

static int getModifierForButtonNumber (const int num) throw()
{
    return num == 0 ? ModifierKeys::leftButtonModifier
                : (num == 1 ? ModifierKeys::rightButtonModifier
                            : (num == 2 ? ModifierKeys::middleButtonModifier : 0));
}

static int64 getMouseTime (UIEvent* e)  { return (int64) [e timestamp] * 1000.0; }

//==============================================================================
- (void) touchesBegan: (NSSet*) touches withEvent: (UIEvent*) event
{
	NSArray* const t = [[event touchesForView: self] allObjects];

	switch ([t count])
	{
		case 1:     // One finger..
		{
            CGPoint p = [[t objectAtIndex: 0] locationInView: self];
            currentModifiers |= getModifierForButtonNumber (0);

            owner->handleMouseDown (p.x, p.y, getMouseTime (event));
        }

        default:
            //xxx multi-touch..
            break;
    }
}

- (void) touchesMoved: (NSSet*) touches withEvent: (UIEvent*) event
{
	NSArray* const t = [[event touchesForView: self] allObjects];

	switch ([t count])
	{
		case 1:     // One finger..
		{
            CGPoint p = [[t objectAtIndex: 0] locationInView: self];
            owner->handleMouseDrag (p.x, p.y, getMouseTime (event));
        }

        default:
            //xxx multi-touch..
            break;
    }
}

- (void) touchesEnded: (NSSet*) touches withEvent: (UIEvent*) event
{
	NSArray* const t = [[event touchesForView: self] allObjects];

	switch ([t count])
	{
		case 1:     // One finger..
		{
            CGPoint p = [[t objectAtIndex: 0] locationInView: self];
            const int oldMods = currentModifiers;
            currentModifiers &= ~getModifierForButtonNumber (0);
            owner->handleMouseUp (oldMods, p.x, p.y, getMouseTime (event));
        }

        default:
            //xxx multi-touch..
            break;
    }
}

- (void) touchesCancelled: (NSSet*) touches withEvent: (UIEvent*) event
{
    [self touchesEnded: touches withEvent: event];
}

//==============================================================================
- (BOOL) becomeFirstResponder
{
    if (owner != 0)
        owner->viewFocusGain();

    return true;
}

- (BOOL) resignFirstResponder
{
    if (owner != 0)
        owner->viewFocusLoss();

    return true;
}

- (BOOL) canBecomeFirstResponder
{
    return owner != 0 && owner->canBecomeKeyWindow();
}


@end

//==============================================================================
@implementation JuceUIWindow

- (void) setOwner: (UIViewComponentPeer*) owner_
{
    owner = owner_;
    isZooming = false;
}

- (void) becomeKeyWindow
{
    [super becomeKeyWindow];

    if (owner != 0)
        owner->grabFocus();
}

@end

//==============================================================================
//==============================================================================
BEGIN_JUCE_NAMESPACE

//==============================================================================
class JuceUIImage
{
public:
    JuceUIImage (const int width, const int height, const bool hasAlpha)
        : juceImage (hasAlpha ? Image::ARGB : Image::RGB,
                     width, height, hasAlpha)
    {
        lineStride = 0;
        pixelStride = 0;
        imageData = juceImage.lockPixelDataReadWrite (0, 0, width, height,
                                                       lineStride, pixelStride);

        CGDataProviderRef provider = CGDataProviderCreateWithData (0, imageData, lineStride * pixelStride, 0);
        
        imageRef = CGImageCreate (width, height, 
                                  8, pixelStride * 8, lineStride, 
                                  CGColorSpaceCreateDeviceRGB(),
                                  hasAlpha ? (kCGImageAlphaFirst | kCGBitmapByteOrder32Little) : kCGBitmapByteOrderDefault,
                                  provider,
                                  0,
                                  true, kCGRenderingIntentDefault);

        juceImage.releasePixelDataReadWrite (imageData);
        
        uiImage = [[UIImage imageWithCGImage: imageRef] retain];
    }

    ~JuceUIImage()
    {
        [uiImage release];
        CFRelease (imageRef);
    }

    Image& getJuceImage() throw()     { return juceImage; }

    void draw (const float x, const float y) const
    {
        [uiImage drawAtPoint: CGPointMake (x, y)
                   blendMode: kCGBlendModeCopy
                       alpha: 1.0f];
    }

    void drawUIImage (UIImage* imageToDraw)
    {
        const ScopedAutoReleasePool pool;

        jassertfalse
        /*[NSGraphicsContext saveGraphicsState];
        [NSGraphicsContext setCurrentContext:
            [NSGraphicsContext graphicsContextWithBitmapImageRep: imageRep]];

        [imageToDraw drawAtPoint: NSZeroPoint
                        fromRect: NSMakeRect (0, 0, [imageToDraw size].width, [imageToDraw size].height)
                       operation: NSCompositeSourceOver
                        fraction: 1.0f];

        [[NSGraphicsContext currentContext] flushGraphics];
        [NSGraphicsContext restoreGraphicsState];

        if (juceImage.hasAlphaChannel())
            swapRGBOrder (0, 0, juceImage.getWidth(), juceImage.getHeight());*/
        
    }

private:
    Image juceImage;
    CGImageRef imageRef;
    CGDataProviderRef provider;
    UIImage* uiImage;
    uint8* imageData;
    int pixelStride, lineStride;

/*    void swapRGBOrder (const int x, const int y, const int w, int h) const
    {
#if JUCE_BIG_ENDIAN
        jassert (pixelStride == 4);
#endif
        jassert (Rectangle (0, 0, juceImage.getWidth(), juceImage.getHeight())
                 .contains (Rectangle (x, y, w, h)));

        uint8* start = imageData + x * pixelStride + y * lineStride;

        while (--h >= 0)
        {
            uint8* p = start;
            start += lineStride;

            for (int i = w; --i >= 0;)
            {
#if JUCE_BIG_ENDIAN
                const uint8 oldp3 = p[3];
                const uint8 oldp1 = p[1];
                p[3] = p[0];
                p[0] = oldp1;
                p[1] = p[2];
                p[2] = oldp3;
#else
                const uint8 oldp0 = p[0];
                p[0] = p[2];
                p[2] = oldp0;
#endif

                p += pixelStride;
            }
        }
    }*/
};

//==============================================================================
UIViewComponentPeer::UIViewComponentPeer (Component* const component,
                                          const int windowStyleFlags,
                                          UIView* viewToAttachTo)
    : ComponentPeer (component, windowStyleFlags),
      window (0),
      view (0),
      isSharedWindow (viewToAttachTo != 0),
      fullScreen (false)
{
    CGRect r;
    r.origin.x = 0;
    r.origin.y = 0;
    r.size.width = (float) component->getWidth();
    r.size.height = (float) component->getHeight();

    view = [[JuceUIView alloc] initWithOwner: this withFrame: r];

    if (isSharedWindow)
    {
        window = [viewToAttachTo window];
        [viewToAttachTo addSubview: view];

        setVisible (component->isVisible());
    }
    else
    {
        r.origin.x = (float) component->getX();
        r.origin.y = (float) component->getY();
        r.origin.y = [[UIScreen mainScreen] bounds].size.height - (r.origin.y + r.size.height);

        window = [[JuceUIWindow alloc] init];
        window.frame = r;
        [((JuceUIWindow*) window) setOwner: this];

        if (component->isAlwaysOnTop())
            window.windowLevel = UIWindowLevelAlert;

        [window addSubview: view];
        view.frame = CGRectMake (0, 0, r.size.width, r.size.height);
    }

    setTitle (component->getName());
}

UIViewComponentPeer::~UIViewComponentPeer()
{
    view->owner = 0;
    [view removeFromSuperview];
    [view release];

    if (! isSharedWindow)
    {
        [((JuceUIWindow*) window) setOwner: 0];
        [window release];
    }
}

//==============================================================================
void* UIViewComponentPeer::getNativeHandle() const
{
    return view;
}

void UIViewComponentPeer::setVisible (bool shouldBeVisible)
{
    view.hidden = ! shouldBeVisible;
}

void UIViewComponentPeer::setTitle (const String& title)
{
    // xxx is this possible?
}

void UIViewComponentPeer::setPosition (int x, int y)
{
    setBounds (x, y, component->getWidth(), component->getHeight(), false);
}

void UIViewComponentPeer::setSize (int w, int h)
{
    setBounds (component->getX(), component->getY(), w, h, false);
}

void UIViewComponentPeer::setBounds (int x, int y, int w, int h, const bool isNowFullScreen)
{
    fullScreen = isNowFullScreen;
    w = jmax (0, w);
    h = jmax (0, h);

    CGRect r;
    r.origin.x = (float) x;
    r.origin.y = (float) y;
    r.size.width = (float) w;
    r.size.height = (float) h;

    if (isSharedWindow)
    {
        //r.origin.y = [[view superview] frame].size.height - (r.origin.y + r.size.height);

        if ([view frame].size.width != r.size.width
             || [view frame].size.height != r.size.height)
            [view setNeedsDisplay];

        view.frame = r;
    }
    else
    {
        //r.origin.y = [[UIScreen mainScreen] bounds].size.height - (r.origin.y + r.size.height);

        window.frame = r;
        view.frame = CGRectMake (0, 0, r.size.width, r.size.height);
    }
}

void UIViewComponentPeer::getBounds (int& x, int& y, int& w, int& h, const bool global) const
{
    CGRect r = [view frame];

    if (global && [view window] != 0)
    {
        r = [view convertRect: r toView: nil];
        CGRect wr = [[view window] frame];
        r.origin.x += wr.origin.x;
        r.origin.y += wr.origin.y;

        y = (int) ([[UIScreen mainScreen] bounds].size.height - r.origin.y - r.size.height);
    }
    else
    {
        y = (int) ([[view superview] frame].size.height - r.origin.y - r.size.height);
    }

    x = (int) r.origin.x;
    w = (int) r.size.width;
    h = (int) r.size.height;
}

void UIViewComponentPeer::getBounds (int& x, int& y, int& w, int& h) const
{
    getBounds (x, y, w, h, ! isSharedWindow);
}

int UIViewComponentPeer::getScreenX() const
{
    int x, y, w, h;
    getBounds (x, y, w, h, true);
    return x;
}

int UIViewComponentPeer::getScreenY() const
{
    int x, y, w, h;
    getBounds (x, y, w, h, true);
    return y;
}

void UIViewComponentPeer::relativePositionToGlobal (int& x, int& y)
{
    int wx, wy, ww, wh;
    getBounds (wx, wy, ww, wh, true);
    x += wx;
    y += wy;
}

void UIViewComponentPeer::globalPositionToRelative (int& x, int& y)
{
    int wx, wy, ww, wh;
    getBounds (wx, wy, ww, wh, true);
    x -= wx;
    y -= wy;
}

CGRect UIViewComponentPeer::constrainRect (CGRect r)
{
    if (constrainer != 0)
    {
        CGRect current = [window frame];
        current.origin.y = [[UIScreen mainScreen] bounds].size.height - current.origin.y - current.size.height;

        r.origin.y = [[UIScreen mainScreen] bounds].size.height - r.origin.y - r.size.height;

        int x = (int) r.origin.x;
        int y = (int) r.origin.y;
        int w = (int) r.size.width;
        int h = (int) r.size.height;

        Rectangle original ((int) current.origin.x, (int) current.origin.y,
                            (int) current.size.width, (int) current.size.height);

        constrainer->checkBounds (x, y, w, h,
                                  original,
                                  Desktop::getInstance().getAllMonitorDisplayAreas().getBounds(),
                                  y != original.getY() && y + h == original.getBottom(),
                                  x != original.getX() && x + w == original.getRight(),
                                  y == original.getY() && y + h != original.getBottom(),
                                  x == original.getX() && x + w != original.getRight());

        r.origin.x = x;
        r.origin.y = [[UIScreen mainScreen] bounds].size.height - r.size.height - y;
        r.size.width = w;
        r.size.height = h;
    }

    return r;
}

void UIViewComponentPeer::setMinimised (bool shouldBeMinimised)
{
    // xxx
}

bool UIViewComponentPeer::isMinimised() const
{
    return false;
}

void UIViewComponentPeer::setFullScreen (bool shouldBeFullScreen)
{
    if (! isSharedWindow)
    {
        Rectangle r (lastNonFullscreenBounds);

        setMinimised (false);

        if (fullScreen != shouldBeFullScreen)
        {
            if (shouldBeFullScreen)
                r = Desktop::getInstance().getMainMonitorArea();

            // (can't call the component's setBounds method because that'll reset our fullscreen flag)
            if (r != getComponent()->getBounds() && ! r.isEmpty())
                setBounds (r.getX(), r.getY(), r.getWidth(), r.getHeight(), shouldBeFullScreen);
        }
    }
}

bool UIViewComponentPeer::isFullScreen() const
{
    return fullScreen;
}

bool UIViewComponentPeer::contains (int x, int y, bool trueIfInAChildWindow) const
{
    if (((unsigned int) x) >= (unsigned int) component->getWidth()
        || ((unsigned int) y) >= (unsigned int) component->getHeight())
        return false;

    CGPoint p;
    p.x = (float) x;
    p.y = (float) y;

    UIView* v = [view hitTest: p withEvent: nil];

    if (trueIfInAChildWindow)
        return v != nil;

    return v == view;
}

const BorderSize UIViewComponentPeer::getFrameSize() const
{
    BorderSize b;

    if (! isSharedWindow)
    {
        CGRect v = [view convertRect: [view frame] toView: nil];
        CGRect w = [window frame];

        b.setTop ((int) (w.size.height - (v.origin.y + v.size.height)));
        b.setBottom ((int) v.origin.y);
        b.setLeft ((int) v.origin.x);
        b.setRight ((int) (w.size.width - (v.origin.x + v.size.width)));
    }

    return b;
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

    if (window != 0 && component->isVisible())
        [window makeKeyAndVisible];
}

void UIViewComponentPeer::toBehind (ComponentPeer* other)
{
    UIViewComponentPeer* o = (UIViewComponentPeer*) other;

    if (isSharedWindow)
    {
        [[view superview] insertSubview: view belowSubview: o->view];
    }
    else
    {
        jassertfalse // don't know how to do this
    }
}

void UIViewComponentPeer::setIcon (const Image& /*newIcon*/)
{
    // to do..
}

//==============================================================================
static UIViewComponentPeer* currentlyFocusedPeer = 0;

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
        currentlyFocusedPeer = 0;
        handleFocusLoss();
    }
}

void juce_HandleProcessFocusChange()
{
    if (UIViewComponentPeer::isValidPeer (currentlyFocusedPeer))
    {
        if (Process::isForegroundProcess())
        {
            currentlyFocusedPeer->handleFocusGain();

            ComponentPeer::bringModalComponentToFront();
        }
        else
        {
            currentlyFocusedPeer->handleFocusLoss();

            // turn kiosk mode off if we lose focus..
            Desktop::getInstance().setKioskModeComponent (0);
        }
    }
}

bool UIViewComponentPeer::isFocused() const
{
    return isSharedWindow ? this == currentlyFocusedPeer
                          : (window != 0 && [window isKeyWindow]);
}

void UIViewComponentPeer::grabFocus()
{
    if (window != 0)
    {
        [window makeKeyWindow];
        viewFocusGain();
    }
}

void UIViewComponentPeer::textInputRequired (int /*x*/, int /*y*/)
{
}


//==============================================================================
void UIViewComponentPeer::drawRect (CGRect r)
{
    if (r.size.width < 1.0f || r.size.height < 1.0f)
        return;

    DBG (Rectangle (r.origin.x, r.origin.y, r.size.width, r.size.height).toString());
    
    const float y = r.origin.y;//[view frame].size.height - (r.origin.y + r.size.height);

    JuceUIImage temp ((int) (r.size.width + 0.5f),
                      (int) (r.size.height + 0.5f),
                      true);//! getComponent()->isOpaque());

    LowLevelGraphicsSoftwareRenderer context (temp.getJuceImage());
    const int originX = -roundFloatToInt (r.origin.x);
    const int originY = -roundFloatToInt (y);
    context.setOrigin (originX, originY);

    handlePaint (context);
    
    //CGContextClipToRect (UIGraphicsGetCurrentContext(), r);
    temp.draw (r.origin.x, r.origin.y);
}

bool UIViewComponentPeer::canBecomeKeyWindow()
{
    return (getStyleFlags() & juce::ComponentPeer::windowIgnoresKeyPresses) == 0;
}

bool UIViewComponentPeer::windowShouldClose()
{
    if (! isValidPeer (this))
        return YES;

    handleUserClosingWindow();
    return NO;
}

void UIViewComponentPeer::redirectMovedOrResized()
{
    handleMovedOrResized();
}

//==============================================================================
void juce_setKioskComponent (Component* kioskModeComponent, bool enableOrDisable, bool allowMenusAndBars)
{
}

//==============================================================================
void UIViewComponentPeer::repaint (int x, int y, int w, int h)
{
    [view setNeedsDisplayInRect:
            CGRectMake ((float) x, (float) y,//([view frame].size.height - (y + h)),
                        (float) w, (float) h)];
}

void UIViewComponentPeer::performAnyPendingRepaintsNow()
{
}

ComponentPeer* Component::createNewPeer (int styleFlags, void* windowToAttachTo)
{
    return new UIViewComponentPeer (this, styleFlags, (UIView*) windowToAttachTo);
}

//==============================================================================
static Image* UIImageToJuceImage (UIImage* image)
{
    JuceUIImage juceIm ((int) [image size].width,
                        (int) [image size].height,
                        true);

    juceIm.drawUIImage (image);
    return juceIm.getJuceImage().createCopy();
}

Image* juce_createIconForFile (const File& file)
{
    return 0;
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

#endif
