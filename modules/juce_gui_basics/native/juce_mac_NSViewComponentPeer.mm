/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

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

class NSViewComponentPeer;

typedef void (*AppFocusChangeCallback)();
extern AppFocusChangeCallback appFocusChangeCallback;
typedef bool (*CheckEventBlockedByModalComps) (NSEvent*);
extern CheckEventBlockedByModalComps isEventBlockedByModalComps;

} // (juce namespace)

@interface NSEvent (JuceDeviceDelta)
 - (CGFloat) deviceDeltaX;
 - (CGFloat) deviceDeltaY;

#if ! (defined (MAC_OS_X_VERSION_10_7) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_7)
 - (CGFloat) scrollingDeltaX;
 - (CGFloat) scrollingDeltaX;
 - (BOOL) hasPreciseScrollingDeltas;
#endif
@end

#define JuceNSView MakeObjCClassName(JuceNSView)

@interface JuceNSView : NSView<NSTextInput>
{
@public
    NSViewComponentPeer* owner;
    NSNotificationCenter* notificationCenter;
    String* stringBeingComposed;
    bool textWasInserted;
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

// NSTextInput Methods
- (void) insertText: (id) aString;
- (void) doCommandBySelector: (SEL) aSelector;
- (void) setMarkedText: (id) aString selectedRange: (NSRange) selRange;
- (void) unmarkText;
- (BOOL) hasMarkedText;
- (long) conversationIdentifier;
- (NSAttributedString*) attributedSubstringFromRange: (NSRange) theRange;
- (NSRange) markedRange;
- (NSRange) selectedRange;
- (NSRect) firstRectForCharacterRange: (NSRange) theRange;
- (NSUInteger) characterIndexForPoint: (NSPoint) thePoint;
- (NSArray*) validAttributesForMarkedText;

- (void) flagsChanged: (NSEvent*) ev;
#if MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_5
- (BOOL) performKeyEquivalent: (NSEvent*) ev;
#endif

- (BOOL) becomeFirstResponder;
- (BOOL) resignFirstResponder;
- (BOOL) acceptsFirstResponder;

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

namespace juce
{

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
    Rectangle<int> getBounds (const bool global) const;
    Rectangle<int> getBounds() const;
    Point<int> getScreenPosition() const;
    Point<int> localToGlobal (const Point<int>& relativePosition);
    Point<int> globalToLocal (const Point<int>& screenPosition);
    void setAlpha (float newAlpha);
    void setMinimised (bool shouldBeMinimised);
    bool isMinimised() const;
    void setFullScreen (bool shouldBeFullScreen);
    bool isFullScreen() const;
    void updateFullscreenStatus();
    bool contains (const Point<int>& position, bool trueIfInAChildWindow) const;
    bool hasNativeTitleBar() const        { return (getStyleFlags() & windowHasTitleBar) != 0; }
    BorderSize<int> getFrameSize() const;
    bool setAlwaysOnTop (bool alwaysOnTop);
    void toFront (bool makeActiveWindow);
    void toBehind (ComponentPeer* other);
    void setIcon (const Image& newIcon);
    StringArray getAvailableRenderingEngines();
    int getCurrentRenderingEngine() const;
    void setCurrentRenderingEngine (int index);

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
    void sendMouseEvent (NSEvent* ev);

    bool handleKeyEvent (NSEvent* ev, bool isKeyDown);
    virtual bool redirectKeyDown (NSEvent* ev);
    virtual bool redirectKeyUp (NSEvent* ev);
    virtual void redirectModKeyChange (NSEvent* ev);
   #if MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_5
    virtual bool redirectPerformKeyEquivalent (NSEvent* ev);
   #endif

    virtual BOOL sendDragCallback (int type, id <NSDraggingInfo> sender);

    virtual bool isOpaque();
    virtual void drawRect (NSRect r);

    virtual bool canBecomeKeyWindow();
    virtual void becomeKeyWindow();
    virtual bool windowShouldClose();

    virtual void redirectMovedOrResized();
    virtual void viewMovedToWindow();

    virtual NSRect constrainRect (NSRect r);

    static void showArrowCursorIfNeeded();
    static void updateModifiers (NSEvent* e);
    static void updateModifiers (NSUInteger);
    static void updateKeysDown (NSEvent* ev, bool isKeyDown);

    static int getKeyCodeFromEvent (NSEvent* ev)
    {
        const String unmodified (nsStringToJuce ([ev charactersIgnoringModifiers]));
        int keyCode = unmodified[0];

        if (keyCode == 0x19) // (backwards-tab)
            keyCode = '\t';
        else if (keyCode == 0x03) // (enter)
            keyCode = '\r';
        else
            keyCode = (int) CharacterFunctions::toUpperCase ((juce_wchar) keyCode);

        if (([ev modifierFlags] & NSNumericPadKeyMask) != 0)
        {
            const int numPadConversions[] = { '0', KeyPress::numberPad0, '1', KeyPress::numberPad1,
                                              '2', KeyPress::numberPad2, '3', KeyPress::numberPad3,
                                              '4', KeyPress::numberPad4, '5', KeyPress::numberPad5,
                                              '6', KeyPress::numberPad6, '7', KeyPress::numberPad7,
                                              '8', KeyPress::numberPad8, '9', KeyPress::numberPad9,
                                              '+', KeyPress::numberPadAdd,  '-', KeyPress::numberPadSubtract,
                                              '*', KeyPress::numberPadMultiply, '/', KeyPress::numberPadDivide,
                                              '.', KeyPress::numberPadDecimalPoint, '=', KeyPress::numberPadEquals };

            for (int i = 0; i < numElementsInArray (numPadConversions); i += 2)
                if (keyCode == numPadConversions [i])
                    keyCode = numPadConversions [i + 1];
        }

        return keyCode;
    }

    static int64 getMouseTime (NSEvent* e)
    {
        return (Time::currentTimeMillis() - Time::getMillisecondCounter())
                + (int64) ([e timestamp] * 1000.0);
    }

    static Point<int> getMousePos (NSEvent* e, NSView* view)
    {
        NSPoint p = [view convertPoint: [e locationInWindow] fromView: nil];
        return Point<int> (roundToInt (p.x), roundToInt ([view frame].size.height - p.y));
    }

    static int getModifierForButtonNumber (const NSInteger num)
    {
        return num == 0 ? ModifierKeys::leftButtonModifier
                    : (num == 1 ? ModifierKeys::rightButtonModifier
                                : (num == 2 ? ModifierKeys::middleButtonModifier : 0));
    }

    static unsigned int getNSWindowStyleMask (const int flags) noexcept
    {
        unsigned int style = (flags & windowHasTitleBar) != 0 ? NSTitledWindowMask
                                                              : NSBorderlessWindowMask;

        if ((flags & windowHasMinimiseButton) != 0)  style |= NSMiniaturizableWindowMask;
        if ((flags & windowHasCloseButton) != 0)     style |= NSClosableWindowMask;
        if ((flags & windowIsResizable) != 0)        style |= NSResizableWindowMask;
        return style;
    }

    //==============================================================================
    virtual void viewFocusGain();
    virtual void viewFocusLoss();
    bool isFocused() const;
    void grabFocus();
    void textInputRequired (const Point<int>& position);

    //==============================================================================
    void repaint (const Rectangle<int>& area);
    void performAnyPendingRepaintsNow();

    //==============================================================================
    NSWindow* window;
    JuceNSView* view;
    bool isSharedWindow, fullScreen, insideDrawRect, usingCoreGraphics, recursiveToFrontCall;

    static ModifierKeys currentModifiers;
    static ComponentPeer* currentlyFocusedPeer;
    static Array<int> keysCurrentlyDown;

private:
    static void appFocusChanged()
    {
        keysCurrentlyDown.clear();

        if (isValidPeer (currentlyFocusedPeer))
        {
            if (Process::isForegroundProcess())
            {
                currentlyFocusedPeer->handleFocusGain();
                ModalComponentManager::getInstance()->bringModalComponentsToFront();
            }
            else
            {
                currentlyFocusedPeer->handleFocusLoss();
            }
        }
    }

    static bool checkEventBlockedByModalComps (NSEvent* e)
    {
        if (Component::getNumCurrentlyModalComponents() == 0)
            return false;

        NSWindow* const w = [e window];
        if (w == nil || [w worksWhenModal])
            return false;

        bool isKey = false, isInputAttempt = false;

        switch ([e type])
        {
            case NSKeyDown:
            case NSKeyUp:
                isKey = isInputAttempt = true;
                break;

            case NSLeftMouseDown:
            case NSRightMouseDown:
            case NSOtherMouseDown:
                isInputAttempt = true;
                break;

            case NSLeftMouseDragged:
            case NSRightMouseDragged:
            case NSLeftMouseUp:
            case NSRightMouseUp:
            case NSOtherMouseUp:
            case NSOtherMouseDragged:
                if (Desktop::getInstance().getDraggingMouseSource(0) != nullptr)
                    return false;
                break;

            case NSMouseMoved:
            case NSMouseEntered:
            case NSMouseExited:
            case NSCursorUpdate:
            case NSScrollWheel:
            case NSTabletPoint:
            case NSTabletProximity:
                break;

            default:
                return false;
        }

        for (int i = ComponentPeer::getNumPeers(); --i >= 0;)
        {
            ComponentPeer* const peer = ComponentPeer::getPeer (i);
            NSView* const compView = (NSView*) peer->getNativeHandle();

            if ([compView window] == w)
            {
                if (isKey)
                {
                    if (compView == [w firstResponder])
                        return false;
                }
                else
                {
                    NSViewComponentPeer* nsViewPeer = dynamic_cast<NSViewComponentPeer*> (peer);

                    if ((nsViewPeer == nullptr || ! nsViewPeer->isSharedWindow)
                            ? NSPointInRect ([e locationInWindow], NSMakeRect (0, 0, [w frame].size.width, [w frame].size.height))
                            : NSPointInRect ([compView convertPoint: [e locationInWindow] fromView: nil], [compView bounds]))
                        return false;
                }
            }
        }

        if (isInputAttempt)
        {
            if (! [NSApp isActive])
                [NSApp activateIgnoringOtherApps: YES];

            Component* const modal = Component::getCurrentlyModalComponent (0);
            if (modal != nullptr)
                modal->inputAttemptWhenModal();
        }

        return true;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NSViewComponentPeer);
};

} // (juce namespace)

//==============================================================================
@implementation JuceNSView

- (JuceNSView*) initWithOwner: (NSViewComponentPeer*) owner_
                    withFrame: (NSRect) frame
{
    [super initWithFrame: frame];
    owner = owner_;
    stringBeingComposed = nullptr;
    textWasInserted = false;

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
    delete stringBeingComposed;
    [super dealloc];
}

//==============================================================================
- (void) drawRect: (NSRect) r
{
    if (owner != nullptr)
        owner->drawRect (r);
}

- (BOOL) isOpaque
{
    return owner == nullptr || owner->isOpaque();
}

//==============================================================================
- (void) mouseDown: (NSEvent*) ev
{
    if (JUCEApplication::isStandaloneApp())
        [self asyncMouseDown: ev];
    else
        // In some host situations, the host will stop modal loops from working
        // correctly if they're called from a mouse event, so we'll trigger
        // the event asynchronously..
        [self performSelectorOnMainThread: @selector (asyncMouseDown:)
                               withObject: ev
                            waitUntilDone: NO];
}

- (void) asyncMouseDown: (NSEvent*) ev
{
    if (owner != nullptr)
        owner->redirectMouseDown (ev);
}

- (void) mouseUp: (NSEvent*) ev
{
    if (! JUCEApplication::isStandaloneApp())
        [self asyncMouseUp: ev];
    else
        // In some host situations, the host will stop modal loops from working
        // correctly if they're called from a mouse event, so we'll trigger
        // the event asynchronously..
        [self performSelectorOnMainThread: @selector (asyncMouseUp:)
                               withObject: ev
                            waitUntilDone: NO];
}

- (void) asyncMouseUp: (NSEvent*) ev    { if (owner != nullptr) owner->redirectMouseUp    (ev); }
- (void) mouseDragged: (NSEvent*) ev    { if (owner != nullptr) owner->redirectMouseDrag  (ev); }
- (void) mouseMoved:   (NSEvent*) ev    { if (owner != nullptr) owner->redirectMouseMove  (ev); }
- (void) mouseEntered: (NSEvent*) ev    { if (owner != nullptr) owner->redirectMouseEnter (ev); }
- (void) mouseExited:  (NSEvent*) ev    { if (owner != nullptr) owner->redirectMouseExit  (ev); }
- (void) scrollWheel:  (NSEvent*) ev    { if (owner != nullptr) owner->redirectMouseWheel (ev); }

- (void) rightMouseDown:    (NSEvent*) ev   { [self mouseDown:    ev]; }
- (void) rightMouseDragged: (NSEvent*) ev   { [self mouseDragged: ev]; }
- (void) rightMouseUp:      (NSEvent*) ev   { [self mouseUp:      ev]; }
- (void) otherMouseDown:    (NSEvent*) ev   { [self mouseDown:    ev]; }
- (void) otherMouseDragged: (NSEvent*) ev   { [self mouseDragged: ev]; }
- (void) otherMouseUp:      (NSEvent*) ev   { [self mouseUp:      ev]; }

- (BOOL) acceptsFirstMouse: (NSEvent*) ev
{
    (void) ev;
    return YES;
}

- (void) frameChanged: (NSNotification*) n
{
    (void) n;
    if (owner != nullptr)
        owner->redirectMovedOrResized();
}

- (void) viewDidMoveToWindow
{
   if (owner != nullptr)
       owner->viewMovedToWindow();
}

//==============================================================================
- (void) keyDown: (NSEvent*) ev
{
    TextInputTarget* const target = owner->findCurrentTextInputTarget();
    textWasInserted = false;

    if (target != nullptr)
        [self interpretKeyEvents: [NSArray arrayWithObject: ev]];
    else
        deleteAndZero (stringBeingComposed);

    if ((! textWasInserted) && (owner == nullptr || ! owner->redirectKeyDown (ev)))
        [super keyDown: ev];
}

- (void) keyUp: (NSEvent*) ev
{
    if (owner == nullptr || ! owner->redirectKeyUp (ev))
        [super keyUp: ev];
}

//==============================================================================
- (void) insertText: (id) aString
{
    // This commits multi-byte text when return is pressed, or after every keypress for western keyboards
    NSString* newText = [aString isKindOfClass: [NSAttributedString class]] ? [aString string] : aString;

    if ([newText length] > 0)
    {
        TextInputTarget* const target = owner->findCurrentTextInputTarget();

        if (target != nullptr)
        {
            target->insertTextAtCaret (nsStringToJuce (newText));
            textWasInserted = true;
        }
    }

    deleteAndZero (stringBeingComposed);
}

- (void) doCommandBySelector: (SEL) aSelector
{
    (void) aSelector;
}

- (void) setMarkedText: (id) aString selectedRange: (NSRange) selectionRange
{
    (void) selectionRange;

    if (stringBeingComposed == 0)
        stringBeingComposed = new String();

    *stringBeingComposed = nsStringToJuce ([aString isKindOfClass:[NSAttributedString class]] ? [aString string] : aString);

    TextInputTarget* const target = owner->findCurrentTextInputTarget();

    if (target != nullptr)
    {
        const Range<int> currentHighlight (target->getHighlightedRegion());
        target->insertTextAtCaret (*stringBeingComposed);
        target->setHighlightedRegion (currentHighlight.withLength (stringBeingComposed->length()));
        textWasInserted = true;
    }
}

- (void) unmarkText
{
    if (stringBeingComposed != nullptr)
    {
        TextInputTarget* const target = owner->findCurrentTextInputTarget();

        if (target != nullptr)
        {
            target->insertTextAtCaret (*stringBeingComposed);
            textWasInserted = true;
        }
    }

    deleteAndZero (stringBeingComposed);
}

- (BOOL) hasMarkedText
{
    return stringBeingComposed != nullptr;
}

- (long) conversationIdentifier
{
    return (long) (pointer_sized_int) self;
}

- (NSAttributedString*) attributedSubstringFromRange: (NSRange) theRange
{
    TextInputTarget* const target = owner->findCurrentTextInputTarget();

    if (target != nullptr)
    {
        const Range<int> r ((int) theRange.location,
                            (int) (theRange.location + theRange.length));

        return [[[NSAttributedString alloc] initWithString: juceStringToNS (target->getTextInRange (r))] autorelease];
    }

    return nil;
}

- (NSRange) markedRange
{
    return stringBeingComposed != nullptr ? NSMakeRange (0, stringBeingComposed->length())
                                          : NSMakeRange (NSNotFound, 0);
}

- (NSRange) selectedRange
{
    TextInputTarget* const target = owner->findCurrentTextInputTarget();

    if (target != nullptr)
    {
        const Range<int> highlight (target->getHighlightedRegion());

        if (! highlight.isEmpty())
            return NSMakeRange (highlight.getStart(), highlight.getLength());
    }

    return NSMakeRange (NSNotFound, 0);
}

- (NSRect) firstRectForCharacterRange: (NSRange) theRange
{
    (void) theRange;
    juce::Component* const comp = dynamic_cast <juce::Component*> (owner->findCurrentTextInputTarget());

    if (comp == 0)
        return NSMakeRect (0, 0, 0, 0);

    const Rectangle<int> bounds (comp->getScreenBounds());

    return NSMakeRect (bounds.getX(),
                       [[[NSScreen screens] objectAtIndex: 0] frame].size.height - bounds.getY(),
                       bounds.getWidth(),
                       bounds.getHeight());
}

- (NSUInteger) characterIndexForPoint: (NSPoint) thePoint
{
    (void) thePoint;
    return NSNotFound;
}

- (NSArray*) validAttributesForMarkedText
{
    return [NSArray array];
}

//==============================================================================
- (void) flagsChanged: (NSEvent*) ev
{
    if (owner != nullptr)
        owner->redirectModKeyChange (ev);
}

#if MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_5
- (BOOL) performKeyEquivalent: (NSEvent*) ev
{
    if (owner != nullptr && owner->redirectPerformKeyEquivalent (ev))
        return true;

    return [super performKeyEquivalent: ev];
}
#endif

- (BOOL) becomeFirstResponder   { if (owner != nullptr) owner->viewFocusGain(); return YES; }
- (BOOL) resignFirstResponder   { if (owner != nullptr) owner->viewFocusLoss(); return YES; }

- (BOOL) acceptsFirstResponder  { return owner != nullptr && owner->canBecomeKeyWindow(); }

//==============================================================================
- (NSArray*) getSupportedDragTypes
{
    return [NSArray arrayWithObjects: NSFilenamesPboardType, NSFilesPromisePboardType, /* NSStringPboardType,*/ nil];
}

- (BOOL) sendDragCallback: (int) type sender: (id <NSDraggingInfo>) sender
{
    return owner != nullptr && owner->sendDragCallback (type, sender);
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
    (void) sender;
    return YES;
}

- (BOOL) performDragOperation: (id <NSDraggingInfo>) sender
{
    return [self sendDragCallback: 2 sender: sender];
}

- (void) concludeDragOperation: (id <NSDraggingInfo>) sender
{
    (void) sender;
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
    return owner != nullptr && owner->canBecomeKeyWindow();
}

- (void) becomeKeyWindow
{
    [super becomeKeyWindow];

    if (owner != nullptr)
        owner->becomeKeyWindow();
}

- (BOOL) windowShouldClose: (id) window
{
    (void) window;
    return owner == nullptr || owner->windowShouldClose();
}

- (NSRect) constrainFrameRect: (NSRect) frameRect toScreen: (NSScreen*) screen
{
    (void) screen;
    if (owner != nullptr)
        frameRect = owner->constrainRect (frameRect);

    return frameRect;
}

- (NSSize) windowWillResize: (NSWindow*) window toSize: (NSSize) proposedFrameSize
{
    (void) window;
    if (isZooming)
        return proposedFrameSize;

    NSRect frameRect = [self frame];
    frameRect.origin.y -= proposedFrameSize.height - frameRect.size.height;
    frameRect.size = proposedFrameSize;

    if (owner != nullptr)
        frameRect = owner->constrainRect (frameRect);

    if (juce::Component::getCurrentlyModalComponent() != nullptr
          && owner->getComponent()->isCurrentlyBlockedByAnotherModalComponent()
          && owner->hasNativeTitleBar())
        juce::Component::getCurrentlyModalComponent()->inputAttemptWhenModal();

    return frameRect.size;
}

- (void) zoom: (id) sender
{
    isZooming = true;
    [super zoom: sender];
    isZooming = false;

    owner->redirectMovedOrResized();
}

- (void) windowWillMove: (NSNotification*) notification
{
    (void) notification;

    if (juce::Component::getCurrentlyModalComponent() != nullptr
          && owner->getComponent()->isCurrentlyBlockedByAnotherModalComponent()
          && owner->hasNativeTitleBar())
        juce::Component::getCurrentlyModalComponent()->inputAttemptWhenModal();
}

@end


//==============================================================================
//==============================================================================
namespace juce
{

//==============================================================================
ModifierKeys NSViewComponentPeer::currentModifiers;
ComponentPeer* NSViewComponentPeer::currentlyFocusedPeer = nullptr;
Array<int> NSViewComponentPeer::keysCurrentlyDown;

//==============================================================================
bool KeyPress::isKeyCurrentlyDown (const int keyCode)
{
    if (NSViewComponentPeer::keysCurrentlyDown.contains (keyCode))
        return true;

    if (keyCode >= 'A' && keyCode <= 'Z'
         && NSViewComponentPeer::keysCurrentlyDown.contains ((int) CharacterFunctions::toLowerCase ((juce_wchar) keyCode)))
        return true;

    if (keyCode >= 'a' && keyCode <= 'z'
         && NSViewComponentPeer::keysCurrentlyDown.contains ((int) CharacterFunctions::toUpperCase ((juce_wchar) keyCode)))
        return true;

    return false;
}

void NSViewComponentPeer::updateModifiers (const NSUInteger flags)
{
    int m = 0;

    if ((flags & NSShiftKeyMask) != 0)        m |= ModifierKeys::shiftModifier;
    if ((flags & NSControlKeyMask) != 0)      m |= ModifierKeys::ctrlModifier;
    if ((flags & NSAlternateKeyMask) != 0)    m |= ModifierKeys::altModifier;
    if ((flags & NSCommandKeyMask) != 0)      m |= ModifierKeys::commandModifier;

    currentModifiers = currentModifiers.withOnlyMouseButtons().withFlags (m);
}

void NSViewComponentPeer::updateModifiers (NSEvent* e)
{
    updateModifiers ([e modifierFlags]);
}

void NSViewComponentPeer::updateKeysDown (NSEvent* ev, bool isKeyDown)
{
    updateModifiers (ev);
    int keyCode = getKeyCodeFromEvent (ev);

    if (keyCode != 0)
    {
        if (isKeyDown)
            keysCurrentlyDown.addIfNotAlreadyThere (keyCode);
        else
            keysCurrentlyDown.removeValue (keyCode);
    }
}

ModifierKeys ModifierKeys::getCurrentModifiersRealtime() noexcept
{
    if ([NSEvent respondsToSelector: @selector (modifierFlags)])
        NSViewComponentPeer::updateModifiers ([NSEvent modifierFlags]);

    return NSViewComponentPeer::currentModifiers;
}

void ModifierKeys::updateCurrentModifiers() noexcept
{
    currentModifiers = NSViewComponentPeer::currentModifiers;
}

//==============================================================================
NSViewComponentPeer::NSViewComponentPeer (Component* const component_,
                                          const int windowStyleFlags,
                                          NSView* viewToAttachTo)
    : ComponentPeer (component_, windowStyleFlags),
      window (nil),
      view (nil),
      isSharedWindow (viewToAttachTo != nil),
      fullScreen (false),
      insideDrawRect (false),
     #if USE_COREGRAPHICS_RENDERING
      usingCoreGraphics (true),
     #else
      usingCoreGraphics (false),
     #endif
      recursiveToFrontCall (false)
{
    appFocusChangeCallback = appFocusChanged;
    isEventBlockedByModalComps = checkEventBlockedByModalComps;

    NSRect r = NSMakeRect (0, 0, (CGFloat) component->getWidth(), (CGFloat) component->getHeight());

    view = [[JuceNSView alloc] initWithOwner: this withFrame: r];
    [view setPostsFrameChangedNotifications: YES];

    if (isSharedWindow)
    {
        window = [viewToAttachTo window];
        [viewToAttachTo addSubview: view];
    }
    else
    {
        r.origin.x = (CGFloat) component->getX();
        r.origin.y = (CGFloat) component->getY();
        r.origin.y = [[[NSScreen screens] objectAtIndex: 0] frame].size.height - (r.origin.y + r.size.height);

        window = [[JuceNSWindow alloc] initWithContentRect: r
                                                 styleMask: getNSWindowStyleMask (windowStyleFlags)
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

       #if defined (MAC_OS_X_VERSION_10_7) && (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_7)
        if ((windowStyleFlags & (windowHasMaximiseButton | windowHasTitleBar)) == (windowHasMaximiseButton | windowHasTitleBar))
            [window setCollectionBehavior: NSWindowCollectionBehaviorFullScreenPrimary];
       #endif
    }

    const float alpha = component->getAlpha();
    if (alpha < 1.0f)
        setAlpha (alpha);

    setTitle (component->getName());
}

NSViewComponentPeer::~NSViewComponentPeer()
{
    view->owner = nullptr;
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
        {
            [window orderFront: nil];
            handleBroughtToFront();
        }
        else
        {
            [window orderOut: nil];
        }
    }
}

void NSViewComponentPeer::setTitle (const String& title)
{
    JUCE_AUTORELEASEPOOL

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

void NSViewComponentPeer::setBounds (int x, int y, int w, int h, bool isNowFullScreen)
{
    fullScreen = isNowFullScreen;

    NSRect r = NSMakeRect ((CGFloat) x, (CGFloat) y, (CGFloat) jmax (0, w), (CGFloat) jmax (0, h));

    if (isSharedWindow)
    {
        r.origin.y = [[view superview] frame].size.height - (r.origin.y + r.size.height);

        if ([view frame].size.width != r.size.width
             || [view frame].size.height != r.size.height)
        {
            [view setNeedsDisplay: true];
        }

        [view setFrame: r];
    }
    else
    {
        r.origin.y = [[[NSScreen screens] objectAtIndex: 0] frame].size.height - (r.origin.y + r.size.height);

        [window setFrame: [window frameRectForContentRect: r]
                 display: true];
    }
}

Rectangle<int> NSViewComponentPeer::getBounds (const bool global) const
{
    NSRect r = [view frame];

    if (global && [view window] != nil)
    {
        r = [view convertRect: r toView: nil];
        NSRect wr = [[view window] frame];
        r.origin.x += wr.origin.x;
        r.origin.y += wr.origin.y;
        r.origin.y = [[[NSScreen screens] objectAtIndex: 0] frame].size.height - r.origin.y - r.size.height;
    }
    else
    {
        r.origin.y = [[view superview] frame].size.height - r.origin.y - r.size.height;
    }

    return Rectangle<int> (convertToRectInt (r));
}

Rectangle<int> NSViewComponentPeer::getBounds() const
{
    return getBounds (! isSharedWindow);
}

Point<int> NSViewComponentPeer::getScreenPosition() const
{
    return getBounds (true).getPosition();
}

Point<int> NSViewComponentPeer::localToGlobal (const Point<int>& relativePosition)
{
    return relativePosition + getScreenPosition();
}

Point<int> NSViewComponentPeer::globalToLocal (const Point<int>& screenPosition)
{
    return screenPosition - getScreenPosition();
}

NSRect NSViewComponentPeer::constrainRect (NSRect r)
{
    if (constrainer != nullptr
        #if defined (MAC_OS_X_VERSION_10_7) && (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_7)
         && ([window styleMask] & NSFullScreenWindowMask) == 0
        #endif
        )
    {
        NSRect current = [window frame];
        current.origin.y = [[[NSScreen screens] objectAtIndex: 0] frame].size.height - current.origin.y - current.size.height;

        r.origin.y = [[[NSScreen screens] objectAtIndex: 0] frame].size.height - r.origin.y - r.size.height;

        Rectangle<int> pos (convertToRectInt (r));
        Rectangle<int> original (convertToRectInt (current));

       #if defined (MAC_OS_X_VERSION_10_6) && MAC_OS_X_VERSION_MIN_ALLOWED >= MAC_OS_X_VERSION_10_6
        if ([window inLiveResize])
       #else
        if ([window respondsToSelector: @selector (inLiveResize)]
             && [window performSelector: @selector (inLiveResize)])
       #endif
        {
            constrainer->checkBounds (pos, original,
                                      Desktop::getInstance().getAllMonitorDisplayAreas().getBounds(),
                                      false, false, true, true);
        }
        else
        {
            constrainer->checkBounds (pos, original,
                                      Desktop::getInstance().getAllMonitorDisplayAreas().getBounds(),
                                      pos.getY() != original.getY() && pos.getBottom() == original.getBottom(),
                                      pos.getX() != original.getX() && pos.getRight() == original.getRight(),
                                      pos.getY() == original.getY() && pos.getBottom() != original.getBottom(),
                                      pos.getX() == original.getX() && pos.getRight() != original.getRight());
        }

        r.origin.x = pos.getX();
        r.origin.y = [[[NSScreen screens] objectAtIndex: 0] frame].size.height - r.size.height - pos.getY();
        r.size.width = pos.getWidth();
        r.size.height = pos.getHeight();
    }

    return r;
}

void NSViewComponentPeer::setAlpha (float newAlpha)
{
    if (! isSharedWindow)
    {
        [window setAlphaValue: (CGFloat) newAlpha];
    }
    else
    {
       #if defined (MAC_OS_X_VERSION_10_5) && MAC_OS_X_VERSION_MIN_ALLOWED >= MAC_OS_X_VERSION_10_5
        [view setAlphaValue: (CGFloat) newAlpha];
       #else
        if ([view respondsToSelector: @selector (setAlphaValue:)])
        {
            // PITA dynamic invocation for 10.4 builds..
            NSInvocation* inv = [NSInvocation invocationWithMethodSignature: [view methodSignatureForSelector: @selector (setAlphaValue:)]];
            [inv setSelector: @selector (setAlphaValue:)];
            [inv setTarget: view];
            CGFloat cgNewAlpha = (CGFloat) newAlpha;
            [inv setArgument: &cgNewAlpha atIndex: 2];
            [inv invoke];
        }
       #endif
    }
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
    return [window isMiniaturized];
}

void NSViewComponentPeer::setFullScreen (bool shouldBeFullScreen)
{
    if (! isSharedWindow)
    {
        Rectangle<int> r (lastNonFullscreenBounds);

        if (isMinimised())
            setMinimised (false);

        if (fullScreen != shouldBeFullScreen)
        {
            if (shouldBeFullScreen && hasNativeTitleBar())
            {
                fullScreen = true;
                [window performZoom: nil];
            }
            else
            {
                if (shouldBeFullScreen)
                    r = component->getParentMonitorArea();

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

bool NSViewComponentPeer::contains (const Point<int>& position, bool trueIfInAChildWindow) const
{
    if (! (isPositiveAndBelow (position.getX(), component->getWidth())
            && isPositiveAndBelow (position.getY(), component->getHeight())))
        return false;

    NSRect frameRect = [view frame];

    NSView* v = [view hitTest: NSMakePoint (frameRect.origin.x + position.getX(),
                                            frameRect.origin.y + frameRect.size.height - position.getY())];

    if (trueIfInAChildWindow)
        return v != nil;

    return v == view;
}

BorderSize<int> NSViewComponentPeer::getFrameSize() const
{
    BorderSize<int> b;

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
        [window setLevel: alwaysOnTop ? NSFloatingWindowLevel
                                      : NSNormalWindowLevel];
    return true;
}

void NSViewComponentPeer::toFront (bool makeActiveWindow)
{
    if (isSharedWindow)
        [[view superview] addSubview: view
                          positioned: NSWindowAbove
                          relativeTo: nil];

    if (window != nil && component->isVisible())
    {
        if (makeActiveWindow)
            [window makeKeyAndOrderFront: nil];
        else
            [window orderFront: nil];

        if (! recursiveToFrontCall)
        {
            recursiveToFrontCall = true;
            Desktop::getInstance().getMainMouseSource().forceMouseCursorUpdate();
            handleBroughtToFront();
            recursiveToFrontCall = false;
        }
    }
}

void NSViewComponentPeer::toBehind (ComponentPeer* other)
{
    NSViewComponentPeer* const otherPeer = dynamic_cast <NSViewComponentPeer*> (other);
    jassert (otherPeer != nullptr); // wrong type of window?

    if (otherPeer != nullptr)
    {
        if (isSharedWindow)
        {
            [[view superview] addSubview: view
                              positioned: NSWindowBelow
                              relativeTo: otherPeer->view];
        }
        else
        {
            [window orderWindow: NSWindowBelow
                     relativeTo: [otherPeer->window windowNumber]];
        }
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
        currentlyFocusedPeer = nullptr;
        handleFocusLoss();
    }
}

bool NSViewComponentPeer::isFocused() const
{
    return isSharedWindow ? this == currentlyFocusedPeer
                          : [window isKeyWindow];
}

void NSViewComponentPeer::grabFocus()
{
    if (window != nil)
    {
        [window makeKeyWindow];
        [window makeFirstResponder: view];

        viewFocusGain();
    }
}

void NSViewComponentPeer::textInputRequired (const Point<int>&)
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
    if (Component::getCurrentlyModalComponent() != nullptr)
        used = true;

    return used;
}

bool NSViewComponentPeer::redirectKeyUp (NSEvent* ev)
{
    updateKeysDown (ev, false);
    return handleKeyEvent (ev, false)
            || Component::getCurrentlyModalComponent() != nullptr;
}

void NSViewComponentPeer::redirectModKeyChange (NSEvent* ev)
{
    keysCurrentlyDown.clear();
    handleKeyUpOrDown (true);

    updateModifiers (ev);
    handleModifierKeysChange();
}

#if MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_5
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
void NSViewComponentPeer::sendMouseEvent (NSEvent* ev)
{
    updateModifiers (ev);
    handleMouseEvent (0, getMousePos (ev, view), currentModifiers, getMouseTime (ev));
}

void NSViewComponentPeer::redirectMouseDown (NSEvent* ev)
{
    currentModifiers = currentModifiers.withFlags (getModifierForButtonNumber ([ev buttonNumber]));
    sendMouseEvent (ev);
}

void NSViewComponentPeer::redirectMouseUp (NSEvent* ev)
{
    currentModifiers = currentModifiers.withoutFlags (getModifierForButtonNumber ([ev buttonNumber]));
    sendMouseEvent (ev);
    showArrowCursorIfNeeded();
}

void NSViewComponentPeer::redirectMouseDrag (NSEvent* ev)
{
    currentModifiers = currentModifiers.withFlags (getModifierForButtonNumber ([ev buttonNumber]));
    sendMouseEvent (ev);
}

void NSViewComponentPeer::redirectMouseMove (NSEvent* ev)
{
    currentModifiers = currentModifiers.withoutMouseButtons();
    sendMouseEvent (ev);
    showArrowCursorIfNeeded();
}

void NSViewComponentPeer::redirectMouseEnter (NSEvent* ev)
{
    Desktop::getInstance().getMainMouseSource().forceMouseCursorUpdate();
    currentModifiers = currentModifiers.withoutMouseButtons();
    sendMouseEvent (ev);
}

void NSViewComponentPeer::redirectMouseExit (NSEvent* ev)
{
    currentModifiers = currentModifiers.withoutMouseButtons();
    sendMouseEvent (ev);
}

void NSViewComponentPeer::redirectMouseWheel (NSEvent* ev)
{
    updateModifiers (ev);

    float x = 0, y = 0;

    @try
    {
       #if defined (MAC_OS_X_VERSION_10_7) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_7
        if ([ev respondsToSelector: @selector (hasPreciseScrollingDeltas)])
        {
            if ([ev hasPreciseScrollingDeltas])
            {
                x = [ev scrollingDeltaX] * 0.5f;
                y = [ev scrollingDeltaY] * 0.5f;
            }
        }
        else
       #endif
        {
            x = [ev deviceDeltaX] * 0.5f;
            y = [ev deviceDeltaY] * 0.5f;
        }
    }
    @catch (...)
    {}

    if (x == 0 && y == 0)
    {
        x = [ev deltaX] * 10.0f;
        y = [ev deltaY] * 10.0f;
    }

    handleMouseWheel (0, getMousePos (ev, view), getMouseTime (ev), x, y);
}

void NSViewComponentPeer::showArrowCursorIfNeeded()
{
    MouseInputSource& mouse = Desktop::getInstance().getMainMouseSource();

    if (mouse.getComponentUnderMouse() == nullptr
         && Desktop::getInstance().findComponentAt (mouse.getScreenPosition()) == nullptr)
    {
        [[NSCursor arrowCursor] set];
    }
}

//==============================================================================
BOOL NSViewComponentPeer::sendDragCallback (const int type, id <NSDraggingInfo> sender)
{
    NSString* bestType
        = [[sender draggingPasteboard] availableTypeFromArray: [view getSupportedDragTypes]];

    if (bestType == nil)
        return false;

    NSPoint p = [view convertPoint: [sender draggingLocation] fromView: nil];
    const Point<int> pos ((int) p.x, (int) ([view frame].size.height - p.y));

    NSPasteboard* pasteBoard = [sender draggingPasteboard];
    StringArray files;

    NSString* iTunesPasteboardType = nsStringLiteral ("CorePasteboardFlavorType 0x6974756E"); // 'itun'

    if (bestType == NSFilesPromisePboardType
         && [[pasteBoard types] containsObject: iTunesPasteboardType])
    {
        id list = [pasteBoard propertyListForType: iTunesPasteboardType];

        if ([list isKindOfClass: [NSDictionary class]])
        {
            NSDictionary* iTunesDictionary = (NSDictionary*) list;
            NSArray* tracks = [iTunesDictionary valueForKey: nsStringLiteral ("Tracks")];
            NSEnumerator* enumerator = [tracks objectEnumerator];
            NSDictionary* track;

            while ((track = [enumerator nextObject]) != nil)
            {
                NSURL* url = [NSURL URLWithString: [track valueForKey: nsStringLiteral ("Location")]];

                if ([url isFileURL])
                    files.add (nsStringToJuce ([url path]));
            }
        }
    }
    else
    {
        id list = [pasteBoard propertyListForType: NSFilenamesPboardType];

        if ([list isKindOfClass: [NSArray class]])
        {
            NSArray* items = (NSArray*) [pasteBoard propertyListForType: NSFilenamesPboardType];

            for (unsigned int i = 0; i < [items count]; ++i)
                files.add (nsStringToJuce ((NSString*) [items objectAtIndex: i]));
        }
    }

    if (files.size() > 0)
    {
        switch (type)
        {
            case 0:   return handleFileDragMove (files, pos);
            case 1:   return handleFileDragExit (files);
            case 2:   return handleFileDragDrop (files, pos);
            default:  jassertfalse; break;
        }
    }

    return false;
}

bool NSViewComponentPeer::isOpaque()
{
    return component == nullptr || component->isOpaque();
}

static void getClipRects (RectangleList& clip, NSView* view,
                          const int xOffset, const int yOffset, const int clipW, const int clipH)
{
    const NSRect* rects = nullptr;
    NSInteger numRects = 0;
    [view getRectsBeingDrawn: &rects count: &numRects];

    const Rectangle<int> clipBounds (clipW, clipH);
    const CGFloat viewH = [view frame].size.height;

    for (int i = 0; i < numRects; ++i)
        clip.addWithoutMerging (clipBounds.getIntersection (Rectangle<int> (roundToInt (rects[i].origin.x) + xOffset,
                                                                            roundToInt (viewH - (rects[i].origin.y + rects[i].size.height)) + yOffset,
                                                                            roundToInt (rects[i].size.width),
                                                                            roundToInt (rects[i].size.height))));
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
        const int xOffset = -roundToInt (r.origin.x);
        const int yOffset = -roundToInt ([view frame].size.height - (r.origin.y + r.size.height));
        const int clipW = (int) (r.size.width  + 0.5f);
        const int clipH = (int) (r.size.height + 0.5f);

        RectangleList clip;
        getClipRects (clip, view, xOffset, yOffset, clipW, clipH);

        if (! clip.isEmpty())
        {
            Image temp (getComponent()->isOpaque() ? Image::RGB : Image::ARGB,
                        clipW, clipH, ! getComponent()->isOpaque());

            {
                ScopedPointer<LowLevelGraphicsContext> context (component->getLookAndFeel()
                                                                    .createGraphicsContext (temp, Point<int> (xOffset, yOffset), clip));

                insideDrawRect = true;
                handlePaint (*context);
                insideDrawRect = false;
            }

            CGColorSpaceRef colourSpace = CGColorSpaceCreateDeviceRGB();
            CGImageRef image = juce_createCoreGraphicsImage (temp, false, colourSpace, false);
            CGColorSpaceRelease (colourSpace);
            CGContextDrawImage (cg, CGRectMake (r.origin.x, r.origin.y, clipW, clipH), image);
            CGImageRelease (image);
        }
    }
}

StringArray NSViewComponentPeer::getAvailableRenderingEngines()
{
    StringArray s (ComponentPeer::getAvailableRenderingEngines());

   #if USE_COREGRAPHICS_RENDERING
    s.add ("CoreGraphics Renderer");
   #endif

    return s;
}

int NSViewComponentPeer::getCurrentRenderingEngine() const
{
    return usingCoreGraphics ? 1 : 0;
}

void NSViewComponentPeer::setCurrentRenderingEngine (int index)
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
    return (getStyleFlags() & juce::ComponentPeer::windowIgnoresKeyPresses) == 0;
}

void NSViewComponentPeer::becomeKeyWindow()
{
    handleBroughtToFront();
    grabFocus();
}

bool NSViewComponentPeer::windowShouldClose()
{
    if (! isValidPeer (this))
        return YES;

    handleUserClosingWindow();
    return NO;
}

void NSViewComponentPeer::updateFullscreenStatus()
{
    if (hasNativeTitleBar())
    {
        const Rectangle<int> screen (getFrameSize().subtractedFrom (component->getParentMonitorArea()));
        const Rectangle<int> window (component->getScreenBounds());

        fullScreen = std::abs (screen.getX() - window.getX()) <= 2
                  && std::abs (screen.getY() - window.getY()) <= 2
                  && std::abs (screen.getRight() - window.getRight()) <= 2
                  && std::abs (screen.getBottom() - window.getBottom()) <= 2;
    }
}

void NSViewComponentPeer::redirectMovedOrResized()
{
    updateFullscreenStatus();
    handleMovedOrResized();
}

void NSViewComponentPeer::viewMovedToWindow()
{
    if (isSharedWindow)
        window = [view window];
}

//==============================================================================
void Desktop::createMouseInputSources()
{
    mouseSources.add (new MouseInputSource (0, true));
}

//==============================================================================
void Desktop::setKioskComponent (Component* kioskModeComponent, bool enableOrDisable, bool allowMenusAndBars)
{
   #if defined (MAC_OS_X_VERSION_10_6) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_6

    NSViewComponentPeer* const peer = dynamic_cast<NSViewComponentPeer*> (kioskModeComponent->getPeer());

   #if defined (MAC_OS_X_VERSION_10_7) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_7
    if (peer != nullptr
         && peer->hasNativeTitleBar()
         && [peer->window respondsToSelector: @selector (toggleFullScreen:)])
    {
        [peer->window performSelector: @selector (toggleFullScreen:)
                           withObject: [NSNumber numberWithBool: (BOOL) enableOrDisable]];
    }
    else
   #endif
    {
        if (enableOrDisable)
        {
            if (peer->hasNativeTitleBar())
                [peer->window setStyleMask: NSBorderlessWindowMask];

            [NSApp setPresentationOptions: (allowMenusAndBars ? (NSApplicationPresentationAutoHideDock | NSApplicationPresentationAutoHideMenuBar)
                                                              : (NSApplicationPresentationHideDock | NSApplicationPresentationHideMenuBar))];
            kioskModeComponent->setBounds (Desktop::getInstance().getMainMonitorArea (false));
        }
        else
        {
            if (peer->hasNativeTitleBar())
            {
                [peer->window setStyleMask: (NSViewComponentPeer::getNSWindowStyleMask (peer->getStyleFlags()))];
                peer->setTitle (peer->component->getName()); // required to force the OS to update the title
            }

            [NSApp setPresentationOptions: NSApplicationPresentationDefault];
        }
    }
   #elif JUCE_SUPPORT_CARBON
    if (enableOrDisable)
    {
        SetSystemUIMode (kUIModeAllSuppressed, allowMenusAndBars ? kUIOptionAutoShowMenuBar : 0);
        kioskModeComponent->setBounds (Desktop::getInstance().getMainMonitorArea (false));
    }
    else
    {
        SetSystemUIMode (kUIModeNormal, 0);
    }
   #else
    // If you're targeting OSes earlier than 10.6 and want to use this feature,
    // you'll need to enable JUCE_SUPPORT_CARBON.
    jassertfalse;
   #endif
}

//==============================================================================
void NSViewComponentPeer::repaint (const Rectangle<int>& area)
{
    if (insideDrawRect)
    {
        class AsyncRepaintMessage  : public CallbackMessage
        {
        public:
            AsyncRepaintMessage (NSViewComponentPeer* const peer_, const Rectangle<int>& rect_)
                : peer (peer_), rect (rect_)
            {
            }

            void messageCallback()
            {
                if (ComponentPeer::isValidPeer (peer))
                    peer->repaint (rect);
            }

        private:
            NSViewComponentPeer* const peer;
            const Rectangle<int> rect;
        };

        (new AsyncRepaintMessage (this, area))->post();
    }
    else
    {
        [view setNeedsDisplayInRect: NSMakeRect ((CGFloat) area.getX(), [view frame].size.height - (CGFloat) area.getBottom(),
                                                 (CGFloat) area.getWidth(), (CGFloat) area.getHeight())];
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
