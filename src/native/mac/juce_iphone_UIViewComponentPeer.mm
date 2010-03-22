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
#if JUCE_INCLUDED_FILE

class UIViewComponentPeer;

//==============================================================================
END_JUCE_NAMESPACE

#define JuceUIView MakeObjCClassName(JuceUIView)

@interface JuceUIView : UIView <UITextFieldDelegate>
{
@public
    UIViewComponentPeer* owner;
    UITextField *hiddenTextField;
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

- (void) asyncRepaint: (id) rect;

- (BOOL) textField: (UITextField*) textField shouldChangeCharactersInRange: (NSRange) range replacementString: (NSString*) string;
- (BOOL) textFieldShouldClear: (UITextField*) textField;
- (BOOL) textFieldShouldReturn: (UITextField*) textField;
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
class UIViewComponentPeer  : public ComponentPeer,
                             public FocusChangeListener
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

    const Rectangle<int> getBounds() const;
    const Rectangle<int> getBounds (const bool global) const;
    const Point<int> getScreenPosition() const;
    const Point<int> relativePositionToGlobal (const Point<int>& relativePosition);
    const Point<int> globalPositionToRelative (const Point<int>& screenPosition);
    void setMinimised (bool shouldBeMinimised);
    bool isMinimised() const;
    void setFullScreen (bool shouldBeFullScreen);
    bool isFullScreen() const;
    bool contains (const Point<int>& position, bool trueIfInAChildWindow) const;
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
    void textInputRequired (const Point<int>& position);

    virtual BOOL textFieldReplaceCharacters (const Range<int>& range, const String& text);
    virtual BOOL textFieldShouldClear();
    virtual BOOL textFieldShouldReturn();
    void updateHiddenTextContent (TextInputTarget* target);
    void globalFocusChanged (Component*);

    void handleTouches (UIEvent* e, bool isDown, bool isUp, bool isCancel);

    //==============================================================================
    void repaint (int x, int y, int w, int h);
    void performAnyPendingRepaintsNow();

    //==============================================================================
    juce_UseDebuggingNewOperator

    UIWindow* window;
    JuceUIView* view;
    bool isSharedWindow, fullScreen, insideDrawRect;
    static ModifierKeys currentModifiers;

    static int64 getMouseTime (UIEvent* e)
    {
        return (Time::currentTimeMillis() - Time::getMillisecondCounter())
                + (int64) ([e timestamp] * 1000.0);
    }

    Array <UITouch*> currentTouches;
};

//==============================================================================
END_JUCE_NAMESPACE

@implementation JuceUIView

- (JuceUIView*) initWithOwner: (UIViewComponentPeer*) owner_
                    withFrame: (CGRect) frame
{
    [super initWithFrame: frame];
    owner = owner_;

    hiddenTextField = [[UITextField alloc] initWithFrame: CGRectMake (0, 0, 0, 0)];
    [self addSubview: hiddenTextField];
    hiddenTextField.delegate = self;

    return self;
}

- (void) dealloc
{
    [hiddenTextField removeFromSuperview];
    [hiddenTextField release];

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

ModifierKeys UIViewComponentPeer::currentModifiers;

const ModifierKeys ModifierKeys::getCurrentModifiersRealtime() throw()
{
    return UIViewComponentPeer::currentModifiers;
}

void ModifierKeys::updateCurrentModifiers() throw()
{
    currentModifiers = UIViewComponentPeer::currentModifiers;
}

JUCE_NAMESPACE::Point<int> juce_lastMousePos;

//==============================================================================
- (void) touchesBegan: (NSSet*) touches withEvent: (UIEvent*) event
{
    if (owner != 0)
        owner->handleTouches (event, true, false, false);
}

- (void) touchesMoved: (NSSet*) touches withEvent: (UIEvent*) event
{
    if (owner != 0)
        owner->handleTouches (event, false, false, false);
}

- (void) touchesEnded: (NSSet*) touches withEvent: (UIEvent*) event
{
    if (owner != 0)
        owner->handleTouches (event, false, true, false);
}

- (void) touchesCancelled: (NSSet*) touches withEvent: (UIEvent*) event
{
    if (owner != 0)
        owner->handleTouches (event, false, true, true);

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

- (void) asyncRepaint: (id) rect
{
    CGRect* r = (CGRect*) [((NSData*) rect) bytes];
    [self setNeedsDisplayInRect: *r];
}

- (BOOL) textField: (UITextField*) textField shouldChangeCharactersInRange: (NSRange) range replacementString: (NSString*) text
{
    return owner->textFieldReplaceCharacters (Range<int> (range.location, range.location + range.length),
                                              nsStringToJuce (text));
}

- (BOOL) textFieldShouldClear: (UITextField*) textField
{
    return owner->textFieldShouldClear();
}

- (BOOL) textFieldShouldReturn: (UITextField*) textField
{
    return owner->textFieldShouldReturn();
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
UIViewComponentPeer::UIViewComponentPeer (Component* const component,
                                          const int windowStyleFlags,
                                          UIView* viewToAttachTo)
    : ComponentPeer (component, windowStyleFlags),
      window (0),
      view (0),
      isSharedWindow (viewToAttachTo != 0),
      fullScreen (false),
      insideDrawRect (false)
{
    CGRect r = CGRectMake (0, 0, (float) component->getWidth(), (float) component->getHeight());

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

        window.opaque = component->isOpaque();
        view.opaque = component->isOpaque();
        window.backgroundColor = [[UIColor blackColor] colorWithAlphaComponent: 0];
        view.backgroundColor = [[UIColor blackColor] colorWithAlphaComponent: 0];

        [((JuceUIWindow*) window) setOwner: this];

        if (component->isAlwaysOnTop())
            window.windowLevel = UIWindowLevelAlert;

        [window addSubview: view];
        view.frame = CGRectMake (0, 0, r.size.width, r.size.height);

        view.hidden = ! component->isVisible();
        window.hidden = ! component->isVisible();

        view.multipleTouchEnabled = YES;
    }

    setTitle (component->getName());

    Desktop::getInstance().addFocusChangeListener (this);
}

UIViewComponentPeer::~UIViewComponentPeer()
{
    Desktop::getInstance().removeFocusChangeListener (this);

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

    if (! isSharedWindow)
        window.hidden = ! shouldBeVisible;
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
        window.frame = r;
        view.frame = CGRectMake (0, 0, r.size.width, r.size.height);
    }
}

const Rectangle<int> UIViewComponentPeer::getBounds (const bool global) const
{
    CGRect r = [view frame];

    if (global && [view window] != 0)
    {
        r = [view convertRect: r toView: nil];
        CGRect wr = [[view window] frame];
        r.origin.x += wr.origin.x;
        r.origin.y += wr.origin.y;
    }

    return Rectangle<int> ((int) r.origin.x, (int) r.origin.y,
                           (int) r.size.width, (int) r.size.height);
}

const Rectangle<int> UIViewComponentPeer::getBounds() const
{
    return getBounds (! isSharedWindow);
}

const Point<int> UIViewComponentPeer::getScreenPosition() const
{
    return getBounds (true).getPosition();
}

const Point<int> UIViewComponentPeer::relativePositionToGlobal (const Point<int>& relativePosition)
{
    return relativePosition + getScreenPosition();
}

const Point<int> UIViewComponentPeer::globalPositionToRelative (const Point<int>& screenPosition)
{
    return screenPosition - getScreenPosition();
}

CGRect UIViewComponentPeer::constrainRect (CGRect r)
{
    if (constrainer != 0)
    {
        CGRect current = [window frame];
        current.origin.y = [[UIScreen mainScreen] bounds].size.height - current.origin.y - current.size.height;

        r.origin.y = [[UIScreen mainScreen] bounds].size.height - r.origin.y - r.size.height;

        Rectangle<int> pos ((int) r.origin.x, (int) r.origin.y,
                            (int) r.size.width, (int) r.size.height);

        Rectangle<int> original ((int) current.origin.x, (int) current.origin.y,
                                 (int) current.size.width, (int) current.size.height);

        constrainer->checkBounds (pos, original,
                                  Desktop::getInstance().getAllMonitorDisplayAreas().getBounds(),
                                  pos.getY() != original.getY() && pos.getBottom() == original.getBottom(),
                                  pos.getX() != original.getX() && pos.getRight() == original.getRight(),
                                  pos.getY() == original.getY() && pos.getBottom() != original.getBottom(),
                                  pos.getX() == original.getX() && pos.getRight() != original.getRight());

        r.origin.x = pos.getX();
        r.origin.y = [[UIScreen mainScreen] bounds].size.height - r.size.height - pos.getY();
        r.size.width = pos.getWidth();
        r.size.height = pos.getHeight();
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
        Rectangle<int> r (lastNonFullscreenBounds);

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

bool UIViewComponentPeer::contains (const Point<int>& position, bool trueIfInAChildWindow) const
{
    if (((unsigned int) position.getX()) >= (unsigned int) component->getWidth()
        || ((unsigned int) position.getY()) >= (unsigned int) component->getHeight())
        return false;

    CGPoint p;
    p.x = (float) position.getX();
    p.y = (float) position.getY();

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
void UIViewComponentPeer::handleTouches (UIEvent* event, const bool isDown, const bool isUp, bool isCancel)
{
    NSArray* touches = [[event touchesForView: view] allObjects];

    for (unsigned int i = 0; i < [touches count]; ++i)
    {
        UITouch* touch = [touches objectAtIndex: i];

        CGPoint p = [touch locationInView: view];
        const Point<int> pos ((int) p.x, (int) p.y);
        juce_lastMousePos = pos + getScreenPosition();

        const int64 time = getMouseTime (event);

        int touchIndex = currentTouches.indexOf (touch);

        if (touchIndex < 0)
        {
            touchIndex = currentTouches.size();
            currentTouches.add (touch);
        }

        if (isDown)
        {
            currentModifiers = currentModifiers.withoutMouseButtons();
            handleMouseEvent (touchIndex, pos, currentModifiers, time);
            currentModifiers = currentModifiers.withoutMouseButtons().withFlags (ModifierKeys::leftButtonModifier);
        }
        else if (isUp)
        {
            currentModifiers = currentModifiers.withoutMouseButtons();
            currentTouches.remove (touchIndex);
        }

        if (isCancel)
            currentTouches.clear();

        handleMouseEvent (touchIndex, pos, currentModifiers, time);
    }
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

void UIViewComponentPeer::textInputRequired (const Point<int>&)
{
}

void UIViewComponentPeer::updateHiddenTextContent (TextInputTarget* target)
{
    view->hiddenTextField.text = juceStringToNS (target->getTextInRange (Range<int> (0, target->getHighlightedRegion().getStart())));
}

BOOL UIViewComponentPeer::textFieldReplaceCharacters (const Range<int>& range, const String& text)
{
    TextInputTarget* const target = findCurrentTextInputTarget();

    if (target != 0)
    {
        const Range<int> currentSelection (target->getHighlightedRegion());

        if (range.getLength() == 1 && text.isEmpty()) // (detect backspace)
            if (currentSelection.isEmpty())
                target->setHighlightedRegion (currentSelection.withStart (currentSelection.getStart() - 1));

        target->insertTextAtCaret (text);
        updateHiddenTextContent (target);
    }

    return NO;
}

BOOL UIViewComponentPeer::textFieldShouldClear()
{
    TextInputTarget* const target = findCurrentTextInputTarget();

    if (target != 0)
    {
        target->setHighlightedRegion (Range<int> (0, std::numeric_limits<int>::max()));
        target->insertTextAtCaret (String::empty);
        updateHiddenTextContent (target);
    }

    return YES;
}

BOOL UIViewComponentPeer::textFieldShouldReturn()
{
    TextInputTarget* const target = findCurrentTextInputTarget();

    if (target != 0)
    {
        target->insertTextAtCaret (T("\n"));
        updateHiddenTextContent (target);
    }

    return YES;
}

void UIViewComponentPeer::globalFocusChanged (Component*)
{
    TextInputTarget* const target = findCurrentTextInputTarget();

    if (target != 0)
    {
        Component* comp = dynamic_cast<Component*> (target);

        Point<int> pos (comp->relativePositionToOtherComponent (component, Point<int>()));
        view->hiddenTextField.frame = CGRectMake (pos.getX(), pos.getY(), 0, 0);

        updateHiddenTextContent (target);
        [view->hiddenTextField becomeFirstResponder];
    }
    else
    {
        [view->hiddenTextField resignFirstResponder];
    }
}


//==============================================================================
void UIViewComponentPeer::drawRect (CGRect r)
{
    if (r.size.width < 1.0f || r.size.height < 1.0f)
        return;

    CGContextRef cg = UIGraphicsGetCurrentContext();

    if (! component->isOpaque())
        CGContextClearRect (cg, CGContextGetClipBoundingBox (cg));

    CGContextConcatCTM (cg, CGAffineTransformMake (1, 0, 0, -1, 0, view.bounds.size.height));
    CoreGraphicsContext g (cg, view.bounds.size.height);

    insideDrawRect = true;
    handlePaint (g);
    insideDrawRect = false;
}

bool UIViewComponentPeer::canBecomeKeyWindow()
{
    return (getStyleFlags() & JUCE_NAMESPACE::ComponentPeer::windowIgnoresKeyPresses) == 0;
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
class AsyncRepaintMessage  : public CallbackMessage
{
public:
    UIViewComponentPeer* const peer;
    const Rectangle<int> rect;

    AsyncRepaintMessage (UIViewComponentPeer* const peer_, const Rectangle<int>& rect_)
        : peer (peer_), rect (rect_)
    {
    }

    void messageCallback()
    {
        if (ComponentPeer::isValidPeer (peer))
            peer->repaint (rect.getX(), rect.getY(), rect.getWidth(), rect.getHeight());
    }
};

void UIViewComponentPeer::repaint (int x, int y, int w, int h)
{
    if (insideDrawRect || ! MessageManager::getInstance()->isThisTheMessageThread())
    {
        (new AsyncRepaintMessage (this, Rectangle<int> (x, y, w, h)))->post();
    }
    else
    {
        [view setNeedsDisplayInRect: CGRectMake ((float) x, (float) y, (float) w, (float) h)];
    }
}

void UIViewComponentPeer::performAnyPendingRepaintsNow()
{
}

ComponentPeer* Component::createNewPeer (int styleFlags, void* windowToAttachTo)
{
    return new UIViewComponentPeer (this, styleFlags, (UIView*) windowToAttachTo);
}

//==============================================================================
Image* juce_createIconForFile (const File& file)
{
    return 0;
}

//==============================================================================
void Desktop::createMouseInputSources()
{
    for (int i = 0; i < 10; ++i)
        mouseSources.add (new MouseInputSource (i, false));
}

bool Desktop::canUseSemiTransparentWindows() throw()
{
    return true;
}

const Point<int> Desktop::getMousePosition()
{
    return juce_lastMousePos;
}

void Desktop::setMousePosition (const Point<int>&)
{
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
