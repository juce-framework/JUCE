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

class NSViewComponentPeer;

//==============================================================================
END_JUCE_NAMESPACE

#define JuceNSView MakeObjCClassName(JuceNSView)

@interface JuceNSView : NSView
{
@public
    NSViewComponentPeer* owner;
    NSNotificationCenter* notificationCenter;
}

- (JuceNSView*) initWithOwner: (NSViewComponentPeer*) owner withFrame: (NSRect) frame;
- (void) dealloc;

- (BOOL) isOpaque;
- (void) drawRect: (NSRect) r;

- (void) mouseDown: (NSEvent*) ev;
- (void) asyncMouseDown: (NSEvent*) ev;
- (void) mouseUp: (NSEvent*) ev;
- (void) asyncMouseUp: (NSEvent*) ev;
- (void) mouseDragged: (NSEvent*) ev;
- (void) mouseMoved: (NSEvent*) ev;
- (void) mouseEntered: (NSEvent*) ev;
- (void) mouseExited: (NSEvent*) ev;
- (void) rightMouseDown: (NSEvent*) ev;
- (void) rightMouseDragged: (NSEvent*) ev;
- (void) rightMouseUp: (NSEvent*) ev;
- (void) otherMouseDown: (NSEvent*) ev;
- (void) otherMouseDragged: (NSEvent*) ev;
- (void) otherMouseUp: (NSEvent*) ev;
- (void) scrollWheel: (NSEvent*) ev;
- (BOOL) acceptsFirstMouse: (NSEvent*) ev;
- (void) frameChanged: (NSNotification*) n;
- (void) viewDidMoveToWindow;

- (void) keyDown: (NSEvent*) ev;
- (void) keyUp: (NSEvent*) ev;
- (void) flagsChanged: (NSEvent*) ev;
#if MACOS_10_4_OR_EARLIER
- (BOOL) performKeyEquivalent: (NSEvent*) ev;
#endif

- (BOOL) becomeFirstResponder;
- (BOOL) resignFirstResponder;
- (BOOL) acceptsFirstResponder;

- (void) asyncRepaint: (id) rect;

- (NSArray*) getSupportedDragTypes;
- (BOOL) sendDragCallback: (int) type sender: (id <NSDraggingInfo>) sender;
- (NSDragOperation) draggingEntered: (id <NSDraggingInfo>) sender;
- (NSDragOperation) draggingUpdated: (id <NSDraggingInfo>) sender;
- (void) draggingEnded: (id <NSDraggingInfo>) sender;
- (void) draggingExited: (id <NSDraggingInfo>) sender;
- (BOOL) prepareForDragOperation: (id <NSDraggingInfo>) sender;
- (BOOL) performDragOperation: (id <NSDraggingInfo>) sender;
- (void) concludeDragOperation: (id <NSDraggingInfo>) sender;

@end

//==============================================================================
#define JuceNSWindow MakeObjCClassName(JuceNSWindow)

#if defined (MAC_OS_X_VERSION_10_6) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_6
@interface JuceNSWindow : NSWindow <NSWindowDelegate>
#else
@interface JuceNSWindow : NSWindow
#endif
{
@private
    NSViewComponentPeer* owner;
    bool isZooming;
}

- (void) setOwner: (NSViewComponentPeer*) owner;
- (BOOL) canBecomeKeyWindow;
- (void) becomeKeyWindow;
- (BOOL) windowShouldClose: (id) window;
- (NSRect) constrainFrameRect: (NSRect) frameRect toScreen: (NSScreen*) screen;
- (NSSize) windowWillResize: (NSWindow*) window toSize: (NSSize) proposedFrameSize;
- (void) zoom: (id) sender;
@end

BEGIN_JUCE_NAMESPACE

//==============================================================================
class NSViewComponentPeer  : public ComponentPeer
{
public:
    NSViewComponentPeer (Component* const component,
                         const int windowStyleFlags,
                         NSView* viewToAttachTo);

    ~NSViewComponentPeer();

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
    const StringArray getAvailableRenderingEngines() throw();
    int getCurrentRenderingEngine() throw();
    void setCurrentRenderingEngine (int index) throw();

    /* When you use multiple DLLs which share similarly-named obj-c classes - like
       for example having more than one juce plugin loaded into a host, then when a
       method is called, the actual code that runs might actually be in a different module
       than the one you expect... So any calls to library functions or statics that are
       made inside obj-c methods will probably end up getting executed in a different DLL's
       memory space. Not a great thing to happen - this obviously leads to bizarre crashes.

       To work around this insanity, I'm only allowing obj-c methods to make calls to
       virtual methods of an object that's known to live inside the right module's space.
    */
    virtual void redirectMouseDown (NSEvent* ev);
    virtual void redirectMouseUp (NSEvent* ev);
    virtual void redirectMouseDrag (NSEvent* ev);
    virtual void redirectMouseMove (NSEvent* ev);
    virtual void redirectMouseEnter (NSEvent* ev);
    virtual void redirectMouseExit (NSEvent* ev);
    virtual void redirectMouseWheel (NSEvent* ev);

    bool handleKeyEvent (NSEvent* ev, bool isKeyDown);
    virtual bool redirectKeyDown (NSEvent* ev);
    virtual bool redirectKeyUp (NSEvent* ev);
    virtual void redirectModKeyChange (NSEvent* ev);
#if MACOS_10_4_OR_EARLIER
    virtual bool redirectPerformKeyEquivalent (NSEvent* ev);
#endif

    virtual BOOL sendDragCallback (int type, id <NSDraggingInfo> sender);

    virtual bool isOpaque();
    virtual void drawRect (NSRect r);

    virtual bool canBecomeKeyWindow();
    virtual bool windowShouldClose();

    virtual void redirectMovedOrResized();
    virtual void viewMovedToWindow();

    virtual NSRect constrainRect (NSRect r);

    static void showArrowCursorIfNeeded();

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

    NSWindow* window;
    JuceNSView* view;
    bool isSharedWindow, fullScreen, insideDrawRect, usingCoreGraphics;
};

//==============================================================================
END_JUCE_NAMESPACE

@implementation JuceNSView

- (JuceNSView*) initWithOwner: (NSViewComponentPeer*) owner_
                    withFrame: (NSRect) frame
{
    [super initWithFrame: frame];
    owner = owner_;

    notificationCenter = [NSNotificationCenter defaultCenter];

    [notificationCenter  addObserver: self
                            selector: @selector (frameChanged:)
                                name: NSViewFrameDidChangeNotification
                              object: self];

    if (! owner_->isSharedWindow)
    {
        [notificationCenter  addObserver: self
                                selector: @selector (frameChanged:)
                                    name: NSWindowDidMoveNotification
                                  object: owner_->window];
    }

    [self registerForDraggedTypes: [self getSupportedDragTypes]];

    return self;
}

- (void) dealloc
{
    [notificationCenter removeObserver: self];
    [super dealloc];
}

//==============================================================================
- (void) drawRect: (NSRect) r
{
    if (owner != 0)
        owner->drawRect (r);
}

- (BOOL) isOpaque
{
    return owner == 0 || owner->isOpaque();
}

//==============================================================================
- (void) mouseDown: (NSEvent*) ev
{
    // In some host situations, the host will stop modal loops from working
    // correctly if they're called from a mouse event, so we'll trigger
    // the event asynchronously..
    if (JUCEApplication::getInstance() == 0)
        [self performSelectorOnMainThread: @selector (asyncMouseDown:)
                               withObject: ev
                            waitUntilDone: NO];
    else
        [self asyncMouseDown: ev];
}

- (void) asyncMouseDown: (NSEvent*) ev
{
    if (owner != 0)
        owner->redirectMouseDown (ev);
}

- (void) mouseUp: (NSEvent*) ev
{
    // In some host situations, the host will stop modal loops from working
    // correctly if they're called from a mouse event, so we'll trigger
    // the event asynchronously..
    if (JUCEApplication::getInstance() == 0)
        [self performSelectorOnMainThread: @selector (asyncMouseUp:)
                               withObject: ev
                            waitUntilDone: NO];
    else
        [self asyncMouseUp: ev];
}

- (void) asyncMouseUp: (NSEvent*) ev
{
    if (owner != 0)
        owner->redirectMouseUp (ev);
}

- (void) mouseDragged: (NSEvent*) ev
{
    if (owner != 0)
        owner->redirectMouseDrag (ev);
}

- (void) mouseMoved: (NSEvent*) ev
{
    if (owner != 0)
        owner->redirectMouseMove (ev);
}

- (void) mouseEntered: (NSEvent*) ev
{
    if (owner != 0)
        owner->redirectMouseEnter (ev);
}

- (void) mouseExited: (NSEvent*) ev
{
    if (owner != 0)
        owner->redirectMouseExit (ev);
}

- (void) rightMouseDown: (NSEvent*) ev
{
    [self mouseDown: ev];
}

- (void) rightMouseDragged: (NSEvent*) ev
{
    [self mouseDragged: ev];
}

- (void) rightMouseUp: (NSEvent*) ev
{
    [self mouseUp: ev];
}

- (void) otherMouseDown: (NSEvent*) ev
{
    [self mouseDown: ev];
}

- (void) otherMouseDragged: (NSEvent*) ev
{
    [self mouseDragged: ev];
}

- (void) otherMouseUp: (NSEvent*) ev
{
    [self mouseUp: ev];
}

- (void) scrollWheel: (NSEvent*) ev
{
    if (owner != 0)
        owner->redirectMouseWheel (ev);
}

- (BOOL) acceptsFirstMouse: (NSEvent*) ev
{
    return YES;
}

- (void) frameChanged: (NSNotification*) n
{
    if (owner != 0)
        owner->redirectMovedOrResized();
}

- (void) viewDidMoveToWindow
{
   if (owner != 0)
       owner->viewMovedToWindow();
}

- (void) asyncRepaint: (id) rect
{
    NSRect* r = (NSRect*) [((NSData*) rect) bytes];
    [self setNeedsDisplayInRect: *r];
}

//==============================================================================
- (void) keyDown: (NSEvent*) ev
{
    if (owner == 0 || ! owner->redirectKeyDown (ev))
        [super keyDown: ev];
}

- (void) keyUp: (NSEvent*) ev
{
    if (owner == 0 || ! owner->redirectKeyUp (ev))
        [super keyUp: ev];
}

- (void) flagsChanged: (NSEvent*) ev
{
    if (owner != 0)
        owner->redirectModKeyChange (ev);
}

#if MACOS_10_4_OR_EARLIER
- (BOOL) performKeyEquivalent: (NSEvent*) ev
{
    if (owner != 0 && owner->redirectPerformKeyEquivalent (ev))
        return true;

    return [super performKeyEquivalent: ev];
}
#endif

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

- (BOOL) acceptsFirstResponder
{
    return owner != 0 && owner->canBecomeKeyWindow();
}

//==============================================================================
- (NSArray*) getSupportedDragTypes
{
    return [NSArray arrayWithObjects: NSFilenamesPboardType, /*NSFilesPromisePboardType, NSStringPboardType,*/ nil];
}

- (BOOL) sendDragCallback: (int) type sender: (id <NSDraggingInfo>) sender
{
    return owner != 0 && owner->sendDragCallback (type, sender);
}

- (NSDragOperation) draggingEntered: (id <NSDraggingInfo>) sender
{
    if ([self sendDragCallback: 0 sender: sender])
        return NSDragOperationCopy | NSDragOperationMove | NSDragOperationGeneric;
    else
        return NSDragOperationNone;
}

- (NSDragOperation) draggingUpdated: (id <NSDraggingInfo>) sender
{
    if ([self sendDragCallback: 0 sender: sender])
        return NSDragOperationCopy | NSDragOperationMove | NSDragOperationGeneric;
    else
        return NSDragOperationNone;
}

- (void) draggingEnded: (id <NSDraggingInfo>) sender
{
    [self sendDragCallback: 1 sender: sender];
}

- (void) draggingExited: (id <NSDraggingInfo>) sender
{
    [self sendDragCallback: 1 sender: sender];
}

- (BOOL) prepareForDragOperation: (id <NSDraggingInfo>) sender
{
    return YES;
}

- (BOOL) performDragOperation: (id <NSDraggingInfo>) sender
{
    return [self sendDragCallback: 2 sender: sender];
}

- (void) concludeDragOperation: (id <NSDraggingInfo>) sender
{
}

@end

//==============================================================================
@implementation JuceNSWindow

- (void) setOwner: (NSViewComponentPeer*) owner_
{
    owner = owner_;
    isZooming = false;
}

- (BOOL) canBecomeKeyWindow
{
    return owner != 0 && owner->canBecomeKeyWindow();
}

- (void) becomeKeyWindow
{
    [super becomeKeyWindow];

    if (owner != 0)
        owner->grabFocus();
}

- (BOOL) windowShouldClose: (id) window
{
    return owner == 0 || owner->windowShouldClose();
}

- (NSRect) constrainFrameRect: (NSRect) frameRect toScreen: (NSScreen*) screen
{
    if (owner != 0)
        frameRect = owner->constrainRect (frameRect);

    return frameRect;
}

- (NSSize) windowWillResize: (NSWindow*) window toSize: (NSSize) proposedFrameSize
{
    if (isZooming)
        return proposedFrameSize;

    NSRect frameRect = [self frame];
    frameRect.origin.y -= proposedFrameSize.height - frameRect.size.height;
    frameRect.size = proposedFrameSize;

    if (owner != 0)
        frameRect = owner->constrainRect (frameRect);

    return frameRect.size;
}

- (void) zoom: (id) sender
{
    isZooming = true;
    [super zoom: sender];
    isZooming = false;
}

- (void) windowWillMove: (NSNotification*) notification
{
    if (JUCE_NAMESPACE::Component::getCurrentlyModalComponent() != 0
          && owner->getComponent()->isCurrentlyBlockedByAnotherModalComponent()
          && (owner->getStyleFlags() & JUCE_NAMESPACE::ComponentPeer::windowHasTitleBar) != 0)
        JUCE_NAMESPACE::Component::getCurrentlyModalComponent()->inputAttemptWhenModal();
}

@end

//==============================================================================
//==============================================================================
BEGIN_JUCE_NAMESPACE

//==============================================================================
static ComponentPeer* currentlyFocusedPeer = 0;
static VoidArray keysCurrentlyDown;

bool KeyPress::isKeyCurrentlyDown (const int keyCode) throw()
{
    if (keysCurrentlyDown.contains ((void*) keyCode))
        return true;

    if (keyCode >= 'A' && keyCode <= 'Z'
        && keysCurrentlyDown.contains ((void*) (int) CharacterFunctions::toLowerCase ((tchar) keyCode)))
        return true;

    if (keyCode >= 'a' && keyCode <= 'z'
        && keysCurrentlyDown.contains ((void*) (int) CharacterFunctions::toUpperCase ((tchar) keyCode)))
        return true;

    return false;
}

static int getKeyCodeFromEvent (NSEvent* ev)
{
    const String unmodified (nsStringToJuce ([ev charactersIgnoringModifiers]));
    int keyCode = unmodified[0];

    if (keyCode == 0x19) // (backwards-tab)
        keyCode = '\t';
    else if (keyCode == 0x03) // (enter)
        keyCode = '\r';

    return keyCode;
}

static int currentModifiers = 0;

static void updateModifiers (NSEvent* e)
{
    int m = currentModifiers & ~(ModifierKeys::shiftModifier | ModifierKeys::ctrlModifier
                                  | ModifierKeys::altModifier | ModifierKeys::commandModifier);

    if (([e modifierFlags] & NSShiftKeyMask) != 0)
        m |= ModifierKeys::shiftModifier;

    if (([e modifierFlags] & NSControlKeyMask) != 0)
        m |= ModifierKeys::ctrlModifier;

    if (([e modifierFlags] & NSAlternateKeyMask) != 0)
        m |= ModifierKeys::altModifier;

    if (([e modifierFlags] & NSCommandKeyMask) != 0)
        m |= ModifierKeys::commandModifier;

    currentModifiers = m;
}

static void updateKeysDown (NSEvent* ev, bool isKeyDown)
{
    updateModifiers (ev);
    int keyCode = getKeyCodeFromEvent (ev);

    if (keyCode != 0)
    {
        if (isKeyDown)
            keysCurrentlyDown.addIfNotAlreadyThere ((void*) keyCode);
        else
            keysCurrentlyDown.removeValue ((void*) keyCode);
    }
}

const ModifierKeys ModifierKeys::getCurrentModifiersRealtime() throw()
{
    return ModifierKeys (currentModifiers);
}

void ModifierKeys::updateCurrentModifiers() throw()
{
    currentModifierFlags = currentModifiers;
}

static int64 getMouseTime (NSEvent* e)
{
    return (Time::currentTimeMillis() - Time::getMillisecondCounter())
            + (int64) ([e timestamp] * 1000.0);
}

static void getMousePos (NSEvent* e, NSView* view, int& x, int& y)
{
    NSPoint p = [view convertPoint: [e locationInWindow] fromView: nil];
    x = roundToInt (p.x);
    y = roundToInt ([view frame].size.height - p.y);
}

static int getModifierForButtonNumber (const NSInteger num)
{
    return num == 0 ? ModifierKeys::leftButtonModifier
                : (num == 1 ? ModifierKeys::rightButtonModifier
                            : (num == 2 ? ModifierKeys::middleButtonModifier : 0));
}

//==============================================================================
NSViewComponentPeer::NSViewComponentPeer (Component* const component_,
                                          const int windowStyleFlags,
                                          NSView* viewToAttachTo)
    : ComponentPeer (component_, windowStyleFlags),
      window (0),
      view (0),
      isSharedWindow (viewToAttachTo != 0),
      fullScreen (false),
      insideDrawRect (false),
#if USE_COREGRAPHICS_RENDERING
      usingCoreGraphics (true)
#else
      usingCoreGraphics (false)
#endif
{
    NSRect r;
    r.origin.x = 0;
    r.origin.y = 0;
    r.size.width = (float) component->getWidth();
    r.size.height = (float) component->getHeight();

    view = [[JuceNSView alloc] initWithOwner: this withFrame: r];
    [view setPostsFrameChangedNotifications: YES];

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
        r.origin.y = [[[NSScreen screens] objectAtIndex: 0] frame].size.height - (r.origin.y + r.size.height);

        unsigned int style = 0;
        if ((windowStyleFlags & windowHasTitleBar) == 0)
            style = NSBorderlessWindowMask;
        else
            style = NSTitledWindowMask;

        if ((windowStyleFlags & windowHasMinimiseButton) != 0)
            style |= NSMiniaturizableWindowMask;

        if ((windowStyleFlags & windowHasCloseButton) != 0)
            style |= NSClosableWindowMask;

        if ((windowStyleFlags & windowIsResizable) != 0)
            style |= NSResizableWindowMask;

        window = [[JuceNSWindow alloc] initWithContentRect: r
                                                 styleMask: style
                                                   backing: NSBackingStoreBuffered
                                                     defer: YES];

        [((JuceNSWindow*) window) setOwner: this];
        [window orderOut: nil];
        [window setDelegate: (JuceNSWindow*) window];
        [window setOpaque: component->isOpaque()];
        [window setHasShadow: ((windowStyleFlags & windowHasDropShadow) != 0)];

        if (component->isAlwaysOnTop())
            [window setLevel: NSFloatingWindowLevel];

        [window setContentView: view];
        [window setAutodisplay: YES];
        [window setAcceptsMouseMovedEvents: YES];

        // We'll both retain and also release this on closing because plugin hosts can unexpectedly
        // close the window for us, and also tend to get cause trouble if setReleasedWhenClosed is NO.
        [window setReleasedWhenClosed: YES];
        [window retain];

        [window setExcludedFromWindowsMenu: (windowStyleFlags & windowIsTemporary) != 0];
        [window setIgnoresMouseEvents: (windowStyleFlags & windowIgnoresMouseClicks) != 0];
    }

    setTitle (component->getName());
}

NSViewComponentPeer::~NSViewComponentPeer()
{
    view->owner = 0;
    [view removeFromSuperview];
    [view release];

    if (! isSharedWindow)
    {
        [((JuceNSWindow*) window) setOwner: 0];
        [window close];
        [window release];
    }
}

//==============================================================================
void* NSViewComponentPeer::getNativeHandle() const
{
    return view;
}

void NSViewComponentPeer::setVisible (bool shouldBeVisible)
{
    if (isSharedWindow)
    {
        [view setHidden: ! shouldBeVisible];
    }
    else
    {
        if (shouldBeVisible)
            [window orderFront: nil];
        else
            [window orderOut: nil];
    }
}

void NSViewComponentPeer::setTitle (const String& title)
{
    const ScopedAutoReleasePool pool;

    if (! isSharedWindow)
        [window setTitle: juceStringToNS (title)];
}

void NSViewComponentPeer::setPosition (int x, int y)
{
    setBounds (x, y, component->getWidth(), component->getHeight(), false);
}

void NSViewComponentPeer::setSize (int w, int h)
{
    setBounds (component->getX(), component->getY(), w, h, false);
}

void NSViewComponentPeer::setBounds (int x, int y, int w, int h, const bool isNowFullScreen)
{
    fullScreen = isNowFullScreen;
    w = jmax (0, w);
    h = jmax (0, h);

    NSRect r;
    r.origin.x = (float) x;
    r.origin.y = (float) y;
    r.size.width = (float) w;
    r.size.height = (float) h;

    if (isSharedWindow)
    {
        r.origin.y = [[view superview] frame].size.height - (r.origin.y + r.size.height);

        if ([view frame].size.width != r.size.width
             || [view frame].size.height != r.size.height)
            [view setNeedsDisplay: true];

        [view setFrame: r];
    }
    else
    {
        r.origin.y = [[[NSScreen screens] objectAtIndex: 0] frame].size.height - (r.origin.y + r.size.height);

        [window setFrame: [window frameRectForContentRect: r]
                 display: true];
    }
}

void NSViewComponentPeer::getBounds (int& x, int& y, int& w, int& h, const bool global) const
{
    NSRect r = [view frame];

    if (global && [view window] != 0)
    {
        r = [view convertRect: r toView: nil];
        NSRect wr = [[view window] frame];
        r.origin.x += wr.origin.x;
        r.origin.y += wr.origin.y;

        y = (int) ([[[NSScreen screens] objectAtIndex:0] frame].size.height - r.origin.y - r.size.height);
    }
    else
    {
        y = (int) ([[view superview] frame].size.height - r.origin.y - r.size.height);
    }

    x = (int) r.origin.x;
    w = (int) r.size.width;
    h = (int) r.size.height;
}

void NSViewComponentPeer::getBounds (int& x, int& y, int& w, int& h) const
{
    getBounds (x, y, w, h, ! isSharedWindow);
}

int NSViewComponentPeer::getScreenX() const
{
    int x, y, w, h;
    getBounds (x, y, w, h, true);
    return x;
}

int NSViewComponentPeer::getScreenY() const
{
    int x, y, w, h;
    getBounds (x, y, w, h, true);
    return y;
}

void NSViewComponentPeer::relativePositionToGlobal (int& x, int& y)
{
    int wx, wy, ww, wh;
    getBounds (wx, wy, ww, wh, true);
    x += wx;
    y += wy;
}

void NSViewComponentPeer::globalPositionToRelative (int& x, int& y)
{
    int wx, wy, ww, wh;
    getBounds (wx, wy, ww, wh, true);
    x -= wx;
    y -= wy;
}

NSRect NSViewComponentPeer::constrainRect (NSRect r)
{
    if (constrainer != 0)
    {
        NSRect current = [window frame];
        current.origin.y = [[[NSScreen screens] objectAtIndex: 0] frame].size.height - current.origin.y - current.size.height;

        r.origin.y = [[[NSScreen screens] objectAtIndex: 0] frame].size.height - r.origin.y - r.size.height;

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
        r.origin.y = [[[NSScreen screens] objectAtIndex: 0] frame].size.height - r.size.height - y;
        r.size.width = w;
        r.size.height = h;
    }

    return r;
}

void NSViewComponentPeer::setMinimised (bool shouldBeMinimised)
{
    if (! isSharedWindow)
    {
        if (shouldBeMinimised)
            [window miniaturize: nil];
        else
            [window deminiaturize: nil];
    }
}

bool NSViewComponentPeer::isMinimised() const
{
    return window != 0 && [window isMiniaturized];
}

void NSViewComponentPeer::setFullScreen (bool shouldBeFullScreen)
{
    if (! isSharedWindow)
    {
        Rectangle r (lastNonFullscreenBounds);

        setMinimised (false);

        if (fullScreen != shouldBeFullScreen)
        {
            if (shouldBeFullScreen && (getStyleFlags() & windowHasTitleBar) != 0)
            {
                fullScreen = true;
                [window performZoom: nil];
            }
            else
            {
                if (shouldBeFullScreen)
                    r = Desktop::getInstance().getMainMonitorArea();

                // (can't call the component's setBounds method because that'll reset our fullscreen flag)
                if (r != getComponent()->getBounds() && ! r.isEmpty())
                    setBounds (r.getX(), r.getY(), r.getWidth(), r.getHeight(), shouldBeFullScreen);
            }
        }
    }
}

bool NSViewComponentPeer::isFullScreen() const
{
    return fullScreen;
}

bool NSViewComponentPeer::contains (int x, int y, bool trueIfInAChildWindow) const
{
    if (((unsigned int) x) >= (unsigned int) component->getWidth()
        || ((unsigned int) y) >= (unsigned int) component->getHeight())
        return false;

    NSPoint p;
    p.x = (float) x;
    p.y = (float) y;

    NSView* v = [view hitTest: p];

    if (trueIfInAChildWindow)
        return v != nil;

    return v == view;
}

const BorderSize NSViewComponentPeer::getFrameSize() const
{
    BorderSize b;

    if (! isSharedWindow)
    {
        NSRect v = [view convertRect: [view frame] toView: nil];
        NSRect w = [window frame];

        b.setTop ((int) (w.size.height - (v.origin.y + v.size.height)));
        b.setBottom ((int) v.origin.y);
        b.setLeft ((int) v.origin.x);
        b.setRight ((int) (w.size.width - (v.origin.x + v.size.width)));
    }

    return b;
}

bool NSViewComponentPeer::setAlwaysOnTop (bool alwaysOnTop)
{
    if (! isSharedWindow)
    {
        [window setLevel: alwaysOnTop ? NSFloatingWindowLevel
                                      : NSNormalWindowLevel];
    }

    return true;
}

void NSViewComponentPeer::toFront (bool makeActiveWindow)
{
    if (isSharedWindow)
    {
        [[view superview] addSubview: view
                          positioned: NSWindowAbove
                          relativeTo: nil];
    }

    if (window != 0 && component->isVisible())
    {
        if (makeActiveWindow)
            [window makeKeyAndOrderFront: nil];
        else
            [window orderFront: nil];
    }
}

void NSViewComponentPeer::toBehind (ComponentPeer* other)
{
    NSViewComponentPeer* o = (NSViewComponentPeer*) other;

    if (isSharedWindow)
    {
        [[view superview] addSubview: view
                          positioned: NSWindowBelow
                          relativeTo: o->view];
    }
    else
    {
        [window orderWindow: NSWindowBelow
                 relativeTo: o->window != 0 ? [o->window windowNumber]
                                            : nil ];
    }
}

void NSViewComponentPeer::setIcon (const Image& /*newIcon*/)
{
    // to do..
}

//==============================================================================
void NSViewComponentPeer::viewFocusGain()
{
    if (currentlyFocusedPeer != this)
    {
        if (ComponentPeer::isValidPeer (currentlyFocusedPeer))
            currentlyFocusedPeer->handleFocusLoss();

        currentlyFocusedPeer = this;

        handleFocusGain();
    }
}

void NSViewComponentPeer::viewFocusLoss()
{
    if (currentlyFocusedPeer == this)
    {
        currentlyFocusedPeer = 0;
        handleFocusLoss();
    }
}

void juce_HandleProcessFocusChange()
{
    keysCurrentlyDown.clear();

    if (NSViewComponentPeer::isValidPeer (currentlyFocusedPeer))
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

bool NSViewComponentPeer::isFocused() const
{
    return isSharedWindow ? this == currentlyFocusedPeer
                          : (window != 0 && [window isKeyWindow]);
}

void NSViewComponentPeer::grabFocus()
{
    if (window != 0)
    {
        [window makeKeyWindow];
        [window makeFirstResponder: view];

        viewFocusGain();
    }
}

void NSViewComponentPeer::textInputRequired (int /*x*/, int /*y*/)
{
}

bool NSViewComponentPeer::handleKeyEvent (NSEvent* ev, bool isKeyDown)
{
    String unicode (nsStringToJuce ([ev characters]));
    String unmodified (nsStringToJuce ([ev charactersIgnoringModifiers]));
    int keyCode = getKeyCodeFromEvent (ev);

    //DBG ("unicode: " + unicode + " " + String::toHexString ((int) unicode[0]));
    //DBG ("unmodified: " + unmodified + " " + String::toHexString ((int) unmodified[0]));

    if (unicode.isNotEmpty() || keyCode != 0)
    {
        if (isKeyDown)
        {
            bool used = false;

            while (unicode.length() > 0)
            {
                juce_wchar textCharacter = unicode[0];
                unicode = unicode.substring (1);

                if (([ev modifierFlags] & NSCommandKeyMask) != 0)
                    textCharacter = 0;

                used = handleKeyUpOrDown (true) || used;
                used = handleKeyPress (keyCode, textCharacter) || used;
            }

            return used;
        }
        else
        {
            if (handleKeyUpOrDown (false))
                return true;
        }
    }

    return false;
}

bool NSViewComponentPeer::redirectKeyDown (NSEvent* ev)
{
    updateKeysDown (ev, true);
    bool used = handleKeyEvent (ev, true);

    if (([ev modifierFlags] & NSCommandKeyMask) != 0)
    {
        // for command keys, the key-up event is thrown away, so simulate one..
        updateKeysDown (ev, false);
        used = (isValidPeer (this) && handleKeyEvent (ev, false)) || used;
    }

    // (If we're running modally, don't allow unused keystrokes to be passed
    // along to other blocked views..)
    if (Component::getCurrentlyModalComponent() != 0)
        used = true;

    return used;
}

bool NSViewComponentPeer::redirectKeyUp (NSEvent* ev)
{
    updateKeysDown (ev, false);
    return handleKeyEvent (ev, false)
            || Component::getCurrentlyModalComponent() != 0;
}

void NSViewComponentPeer::redirectModKeyChange (NSEvent* ev)
{
    updateModifiers (ev);
    handleModifierKeysChange();
}

#if MACOS_10_4_OR_EARLIER
bool NSViewComponentPeer::redirectPerformKeyEquivalent (NSEvent* ev)
{
    if ([ev type] == NSKeyDown)
        return redirectKeyDown (ev);
    else if ([ev type] == NSKeyUp)
        return redirectKeyUp (ev);

    return false;
}
#endif

//==============================================================================
void NSViewComponentPeer::redirectMouseDown (NSEvent* ev)
{
    updateModifiers (ev);
    currentModifiers |= getModifierForButtonNumber ([ev buttonNumber]);
    int x, y;
    getMousePos (ev, view, x, y);

    handleMouseDown (x, y, getMouseTime (ev));
}

void NSViewComponentPeer::redirectMouseUp (NSEvent* ev)
{
    const int oldMods = currentModifiers;
    updateModifiers (ev);
    currentModifiers &= ~getModifierForButtonNumber ([ev buttonNumber]);
    int x, y;
    getMousePos (ev, view, x, y);

    handleMouseUp (oldMods, x, y, getMouseTime (ev));
    showArrowCursorIfNeeded();
}

void NSViewComponentPeer::redirectMouseDrag (NSEvent* ev)
{
    updateModifiers (ev);
    currentModifiers |= getModifierForButtonNumber ([ev buttonNumber]);
    int x, y;
    getMousePos (ev, view, x, y);

    handleMouseDrag (x, y, getMouseTime (ev));
}

void NSViewComponentPeer::redirectMouseMove (NSEvent* ev)
{
    updateModifiers (ev);
    int x, y;
    getMousePos (ev, view, x, y);

    handleMouseMove (x, y, getMouseTime (ev));
    showArrowCursorIfNeeded();
}

void NSViewComponentPeer::redirectMouseEnter (NSEvent* ev)
{
    updateModifiers (ev);
    int x, y;
    getMousePos (ev, view, x, y);

    handleMouseEnter (x, y, getMouseTime (ev));
}

void NSViewComponentPeer::redirectMouseExit (NSEvent* ev)
{
    updateModifiers (ev);
    int x, y;
    getMousePos (ev, view, x, y);

    handleMouseExit (x, y, getMouseTime (ev));
}

void NSViewComponentPeer::redirectMouseWheel (NSEvent* ev)
{
    updateModifiers (ev);

    handleMouseWheel (roundToInt ([ev deltaX] * 10.0f),
                      roundToInt ([ev deltaY] * 10.0f),
                      getMouseTime (ev));
}

void NSViewComponentPeer::showArrowCursorIfNeeded()
{
    if (Component::getComponentUnderMouse() == 0)
    {
        int mx, my;
        Desktop::getInstance().getMousePosition (mx, my);

        if (Desktop::getInstance().findComponentAt (mx, my) == 0)
            [[NSCursor arrowCursor] set];
    }
}

//==============================================================================
BOOL NSViewComponentPeer::sendDragCallback (int type, id <NSDraggingInfo> sender)
{
    NSString* bestType
        = [[sender draggingPasteboard] availableTypeFromArray: [view getSupportedDragTypes]];

    if (bestType == nil)
        return false;

    NSPoint p = [view convertPoint: [sender draggingLocation] fromView: nil];
    int x = (int) p.x;
    int y = (int) ([view frame].size.height - p.y);

    StringArray files;

    id list = [[sender draggingPasteboard] propertyListForType: bestType];
    if (list == nil)
        return false;

    if ([list isKindOfClass: [NSArray class]])
    {
        NSArray* items = (NSArray*) list;

        for (unsigned int i = 0; i < [items count]; ++i)
            files.add (nsStringToJuce ((NSString*) [items objectAtIndex: i]));
    }

    if (files.size() == 0)
        return false;

    if (type == 0)
        handleFileDragMove (files, x, y);
    else if (type == 1)
        handleFileDragExit (files);
    else if (type == 2)
        handleFileDragDrop (files, x, y);

    return true;
}

bool NSViewComponentPeer::isOpaque()
{
    if (! getComponent()->isValidComponent())
        return true;

    return getComponent()->isOpaque();
}

void NSViewComponentPeer::drawRect (NSRect r)
{
    if (r.size.width < 1.0f || r.size.height < 1.0f)
        return;

    CGContextRef cg = (CGContextRef) [[NSGraphicsContext currentContext] graphicsPort];

    if (! component->isOpaque())
        CGContextClearRect (cg, CGContextGetClipBoundingBox (cg));

#if USE_COREGRAPHICS_RENDERING
    if (usingCoreGraphics)
    {
        CoreGraphicsContext context (cg, (float) [view frame].size.height);

        insideDrawRect = true;
        handlePaint (context);
        insideDrawRect = false;
    }
    else
#endif
    {
        Image temp (getComponent()->isOpaque() ? Image::RGB : Image::ARGB,
                    (int) (r.size.width + 0.5f),
                    (int) (r.size.height + 0.5f),
                    ! getComponent()->isOpaque());

        LowLevelGraphicsSoftwareRenderer context (temp);
        context.setOrigin (-roundToInt (r.origin.x),
                           -roundToInt ([view frame].size.height - (r.origin.y + r.size.height)));

        const NSRect* rects = 0;
        NSInteger numRects = 0;
        [view getRectsBeingDrawn: &rects count: &numRects];

        RectangleList clip;
        for (int i = 0; i < numRects; ++i)
        {
            clip.addWithoutMerging (Rectangle (roundToInt (rects[i].origin.x),
                                               roundToInt ([view frame].size.height - (rects[i].origin.y + rects[i].size.height)),
                                               roundToInt (rects[i].size.width),
                                               roundToInt (rects[i].size.height)));
        }

        if (context.clipToRectangleList (clip))
        {
            insideDrawRect = true;
            handlePaint (context);
            insideDrawRect = false;

            CGColorSpaceRef colourSpace = CGColorSpaceCreateDeviceRGB();
            CGImageRef image = CoreGraphicsImage::createImage (temp, false, colourSpace);
            CGColorSpaceRelease (colourSpace);
            CGContextDrawImage (cg, CGRectMake (r.origin.x, r.origin.y, temp.getWidth(), temp.getHeight()), image);
            CGImageRelease (image);
        }
    }
}

const StringArray NSViewComponentPeer::getAvailableRenderingEngines() throw()
{
    StringArray s;
    s.add ("Software Renderer");

#if USE_COREGRAPHICS_RENDERING
    s.add ("CoreGraphics Renderer");
#endif

    return s;
}

int NSViewComponentPeer::getCurrentRenderingEngine() throw()
{
    return usingCoreGraphics ? 1 : 0;
}

void NSViewComponentPeer::setCurrentRenderingEngine (int index) throw()
{
#if USE_COREGRAPHICS_RENDERING
    if (usingCoreGraphics != (index > 0))
    {
        usingCoreGraphics = index > 0;
        [view setNeedsDisplay: true];
    }
#endif
}

bool NSViewComponentPeer::canBecomeKeyWindow()
{
    return (getStyleFlags() & JUCE_NAMESPACE::ComponentPeer::windowIgnoresKeyPresses) == 0;
}

bool NSViewComponentPeer::windowShouldClose()
{
    if (! isValidPeer (this))
        return YES;

    handleUserClosingWindow();
    return NO;
}

void NSViewComponentPeer::redirectMovedOrResized()
{
    handleMovedOrResized();
}

void NSViewComponentPeer::viewMovedToWindow()
{
    if (isSharedWindow)
        window = [view window];
}

//==============================================================================
void juce_setKioskComponent (Component* kioskModeComponent, bool enableOrDisable, bool allowMenusAndBars)
{
    // Very annoyingly, this function has to use the old SetSystemUIMode function,
    // which is in Carbon.framework. But, because there's no Cocoa equivalent, it
    // is apparently still available in 64-bit apps..
    if (enableOrDisable)
    {
        SetSystemUIMode (kUIModeAllSuppressed, allowMenusAndBars ? kUIOptionAutoShowMenuBar : 0);
        kioskModeComponent->setBounds (Desktop::getInstance().getMainMonitorArea (false));
    }
    else
    {
        SetSystemUIMode (kUIModeNormal, 0);
    }
}

//==============================================================================
class AsyncRepaintMessage  : public CallbackMessage
{
public:
    NSViewComponentPeer* const peer;
    const Rectangle rect;

    AsyncRepaintMessage (NSViewComponentPeer* const peer_, const Rectangle& rect_)
        : peer (peer_), rect (rect_)
    {
    }

    void messageCallback()
    {
        if (ComponentPeer::isValidPeer (peer))
            peer->repaint (rect.getX(), rect.getY(), rect.getWidth(), rect.getHeight());
    }
};

void NSViewComponentPeer::repaint (int x, int y, int w, int h)
{
    if (insideDrawRect)
    {
        (new AsyncRepaintMessage (this, Rectangle (x, y, w, h)))->post();
    }
    else
    {
        [view setNeedsDisplayInRect: NSMakeRect ((float) x, (float) ([view frame].size.height - (y + h)),
                                                 (float) w, (float) h)];
    }
}

void NSViewComponentPeer::performAnyPendingRepaintsNow()
{
    [view displayIfNeeded];
}

ComponentPeer* Component::createNewPeer (int styleFlags, void* windowToAttachTo)
{
    return new NSViewComponentPeer (this, styleFlags, (NSView*) windowToAttachTo);
}

//==============================================================================
Image* juce_createIconForFile (const File& file)
{
    const ScopedAutoReleasePool pool;

    NSImage* image = [[NSWorkspace sharedWorkspace] iconForFile: juceStringToNS (file.getFullPathName())];

    CoreGraphicsImage* result = new CoreGraphicsImage (Image::ARGB, (int) [image size].width, (int) [image size].height, true);

    [NSGraphicsContext saveGraphicsState];
    [NSGraphicsContext setCurrentContext: [NSGraphicsContext graphicsContextWithGraphicsPort: result->context flipped: false]];

    [image drawAtPoint: NSMakePoint (0, 0)
              fromRect: NSMakeRect (0, 0, [image size].width, [image size].height)
             operation: NSCompositeSourceOver fraction: 1.0f];

    [[NSGraphicsContext currentContext] flushGraphics];
    [NSGraphicsContext restoreGraphicsState];

    return result;
}

//==============================================================================
const int KeyPress::spaceKey        = ' ';
const int KeyPress::returnKey       = 0x0d;
const int KeyPress::escapeKey       = 0x1b;
const int KeyPress::backspaceKey    = 0x7f;
const int KeyPress::leftKey         = NSLeftArrowFunctionKey;
const int KeyPress::rightKey        = NSRightArrowFunctionKey;
const int KeyPress::upKey           = NSUpArrowFunctionKey;
const int KeyPress::downKey         = NSDownArrowFunctionKey;
const int KeyPress::pageUpKey       = NSPageUpFunctionKey;
const int KeyPress::pageDownKey     = NSPageDownFunctionKey;
const int KeyPress::endKey          = NSEndFunctionKey;
const int KeyPress::homeKey         = NSHomeFunctionKey;
const int KeyPress::deleteKey       = NSDeleteFunctionKey;
const int KeyPress::insertKey       = -1;
const int KeyPress::tabKey          = 9;
const int KeyPress::F1Key           = NSF1FunctionKey;
const int KeyPress::F2Key           = NSF2FunctionKey;
const int KeyPress::F3Key           = NSF3FunctionKey;
const int KeyPress::F4Key           = NSF4FunctionKey;
const int KeyPress::F5Key           = NSF5FunctionKey;
const int KeyPress::F6Key           = NSF6FunctionKey;
const int KeyPress::F7Key           = NSF7FunctionKey;
const int KeyPress::F8Key           = NSF8FunctionKey;
const int KeyPress::F9Key           = NSF9FunctionKey;
const int KeyPress::F10Key          = NSF10FunctionKey;
const int KeyPress::F11Key          = NSF1FunctionKey;
const int KeyPress::F12Key          = NSF12FunctionKey;
const int KeyPress::F13Key          = NSF13FunctionKey;
const int KeyPress::F14Key          = NSF14FunctionKey;
const int KeyPress::F15Key          = NSF15FunctionKey;
const int KeyPress::F16Key          = NSF16FunctionKey;
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
