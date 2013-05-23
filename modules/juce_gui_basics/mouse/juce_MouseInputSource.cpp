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

class MouseInputSourceInternal   : private AsyncUpdater
{
public:
    //==============================================================================
    MouseInputSourceInternal (MouseInputSource& source_, const int index_, const bool isMouseDevice_)
        : index (index_), isMouseDevice (isMouseDevice_), source (source_), lastPeer (nullptr),
          isUnboundedMouseModeOn (false), isCursorVisibleUntilOffscreen (false), currentCursorHandle (nullptr),
          mouseEventCounter (0), mouseMovedSignificantlySincePressed (false)
    {
    }

    //==============================================================================
    bool isDragging() const noexcept
    {
        return buttonState.isAnyMouseButtonDown();
    }

    Component* getComponentUnderMouse() const
    {
        return componentUnderMouse.get();
    }

    ModifierKeys getCurrentModifiers() const
    {
        return ModifierKeys::getCurrentModifiers().withoutMouseButtons().withFlags (buttonState.getRawFlags());
    }

    ComponentPeer* getPeer()
    {
        if (! ComponentPeer::isValidPeer (lastPeer))
            lastPeer = nullptr;

        return lastPeer;
    }

    Component* findComponentAt (Point<int> screenPos)
    {
        if (ComponentPeer* const peer = getPeer())
        {
            Component& comp = peer->getComponent();
            const Point<int> relativePos (comp.getLocalPoint (nullptr, screenPos));

            // (the contains() call is needed to test for overlapping desktop windows)
            if (comp.contains (relativePos))
                return comp.getComponentAt (relativePos);
        }

        return nullptr;
    }

    Point<int> getScreenPosition() const
    {
        // This needs to return the live position if possible, but it mustn't update the lastScreenPos
        // value, because that can cause continuity problems.
        return unboundedMouseOffset + (isMouseDevice ? MouseInputSource::getCurrentMousePosition()
                                                     : lastScreenPos);
    }

    //==============================================================================
   #if JUCE_DUMP_MOUSE_EVENTS
    #define JUCE_MOUSE_EVENT_DBG(desc)   DBG ("Mouse " desc << " #" << source.getIndex() \
                                                << ": " << comp->getLocalPoint (nullptr, screenPos).toString() \
                                                << " - Comp: " << String::toHexString ((int) comp));
   #else
    #define JUCE_MOUSE_EVENT_DBG(desc)
   #endif

    void sendMouseEnter (Component* const comp, Point<int> screenPos, Time time)
    {
        JUCE_MOUSE_EVENT_DBG ("enter")
        comp->internalMouseEnter (source, comp->getLocalPoint (nullptr, screenPos), time);
    }

    void sendMouseExit (Component* const comp, Point<int> screenPos, Time time)
    {
        JUCE_MOUSE_EVENT_DBG ("exit")
        comp->internalMouseExit (source, comp->getLocalPoint (nullptr, screenPos), time);
    }

    void sendMouseMove (Component* const comp, Point<int> screenPos, Time time)
    {
        JUCE_MOUSE_EVENT_DBG ("move")
        comp->internalMouseMove (source, comp->getLocalPoint (nullptr, screenPos), time);
    }

    void sendMouseDown (Component* const comp, Point<int> screenPos, Time time)
    {
        JUCE_MOUSE_EVENT_DBG ("down")
        comp->internalMouseDown (source, comp->getLocalPoint (nullptr, screenPos), time);
    }

    void sendMouseDrag (Component* const comp, Point<int> screenPos, Time time)
    {
        JUCE_MOUSE_EVENT_DBG ("drag")
        comp->internalMouseDrag (source, comp->getLocalPoint (nullptr, screenPos), time);
    }

    void sendMouseUp (Component* const comp, Point<int> screenPos, Time time, const ModifierKeys oldMods)
    {
        JUCE_MOUSE_EVENT_DBG ("up")
        comp->internalMouseUp (source, comp->getLocalPoint (nullptr, screenPos), time, oldMods);
    }

    void sendMouseWheel (Component* const comp, Point<int> screenPos, Time time, const MouseWheelDetails& wheel)
    {
        JUCE_MOUSE_EVENT_DBG ("wheel")
        comp->internalMouseWheel (source, comp->getLocalPoint (nullptr, screenPos), time, wheel);
    }

    void sendMagnifyGesture (Component* const comp, Point<int> screenPos, Time time, const float amount)
    {
        JUCE_MOUSE_EVENT_DBG ("magnify")
        comp->internalMagnifyGesture (source, comp->getLocalPoint (nullptr, screenPos), time, amount);
    }

    //==============================================================================
    // (returns true if the button change caused a modal event loop)
    bool setButtons (Point<int> screenPos, Time time, const ModifierKeys newButtonState)
    {
        if (buttonState == newButtonState)
            return false;

        // (avoid sending a spurious mouse-drag when we receive a mouse-up)
        if (! (isDragging() && ! newButtonState.isAnyMouseButtonDown()))
            setScreenPos (screenPos, time, false);

        // (ignore secondary clicks when there's already a button down)
        if (buttonState.isAnyMouseButtonDown() == newButtonState.isAnyMouseButtonDown())
        {
            buttonState = newButtonState;
            return false;
        }

        const int lastCounter = mouseEventCounter;

        if (buttonState.isAnyMouseButtonDown())
        {
            if (Component* const current = getComponentUnderMouse())
            {
                const ModifierKeys oldMods (getCurrentModifiers());
                buttonState = newButtonState; // must change this before calling sendMouseUp, in case it runs a modal loop

                sendMouseUp (current, screenPos + unboundedMouseOffset, time, oldMods);

                if (lastCounter != mouseEventCounter)
                    return true; // if a modal loop happened, then newButtonState is no longer valid.
            }

            enableUnboundedMouseMovement (false, false);
        }

        buttonState = newButtonState;

        if (buttonState.isAnyMouseButtonDown())
        {
            Desktop::getInstance().incrementMouseClickCounter();

            if (Component* const current = getComponentUnderMouse())
            {
                registerMouseDown (screenPos, time, *current, buttonState);
                sendMouseDown (current, screenPos, time);
            }
        }

        return lastCounter != mouseEventCounter;
    }

    void setComponentUnderMouse (Component* const newComponent, Point<int> screenPos, Time time)
    {
        Component* current = getComponentUnderMouse();

        if (newComponent != current)
        {
            WeakReference<Component> safeNewComp (newComponent);
            const ModifierKeys originalButtonState (buttonState);

            if (current != nullptr)
            {
                WeakReference<Component> safeOldComp (current);
                setButtons (screenPos, time, ModifierKeys());

                if (safeOldComp != nullptr)
                {
                    componentUnderMouse = safeNewComp;
                    sendMouseExit (safeOldComp, screenPos, time);
                }

                buttonState = originalButtonState;
            }

            current = componentUnderMouse = safeNewComp;

            if (current != nullptr)
                sendMouseEnter (current, screenPos, time);

            revealCursor (false);
            setButtons (screenPos, time, originalButtonState);
        }
    }

    void setPeer (ComponentPeer* const newPeer, Point<int> screenPos, Time time)
    {
        ModifierKeys::updateCurrentModifiers();

        if (newPeer != lastPeer)
        {
            setComponentUnderMouse (nullptr, screenPos, time);
            lastPeer = newPeer;
            setComponentUnderMouse (findComponentAt (screenPos), screenPos, time);
        }
    }

    void setScreenPos (Point<int> newScreenPos, Time time, const bool forceUpdate)
    {
        if (! isDragging())
            setComponentUnderMouse (findComponentAt (newScreenPos), newScreenPos, time);

        if (newScreenPos != lastScreenPos || forceUpdate)
        {
            cancelPendingUpdate();
            lastScreenPos = newScreenPos;

            if (Component* const current = getComponentUnderMouse())
            {
                if (isDragging())
                {
                    registerMouseDrag (newScreenPos);
                    sendMouseDrag (current, newScreenPos + unboundedMouseOffset, time);

                    if (isUnboundedMouseModeOn)
                        handleUnboundedDrag (current);
                }
                else
                {
                    sendMouseMove (current, newScreenPos, time);
                }
            }

            revealCursor (false);
        }
    }

    //==============================================================================
    void handleEvent (ComponentPeer* const newPeer, Point<int> positionWithinPeer, Time time, const ModifierKeys newMods)
    {
        jassert (newPeer != nullptr);
        lastTime = time;
        ++mouseEventCounter;
        const Point<int> screenPos (newPeer->localToGlobal (positionWithinPeer));

        if (isDragging() && newMods.isAnyMouseButtonDown())
        {
            setScreenPos (screenPos, time, false);
        }
        else
        {
            setPeer (newPeer, screenPos, time);

            if (ComponentPeer* peer = getPeer())
            {
                if (setButtons (screenPos, time, newMods))
                    return; // some modal events have been dispatched, so the current event is now out-of-date

                peer = getPeer();
                if (peer != nullptr)
                    setScreenPos (screenPos, time, false);
            }
        }
    }

    Component* getTargetForGesture (ComponentPeer* const peer, Point<int> positionWithinPeer,
                                    Time time, Point<int>& screenPos)
    {
        jassert (peer != nullptr);
        lastTime = time;
        ++mouseEventCounter;

        screenPos = peer->localToGlobal (positionWithinPeer);
        setPeer (peer, screenPos, time);
        setScreenPos (screenPos, time, false);
        triggerFakeMove();

        return isDragging() ? nullptr : getComponentUnderMouse();
    }

    void handleWheel (ComponentPeer* const peer, Point<int> positionWithinPeer,
                      Time time, const MouseWheelDetails& wheel)
    {
        Desktop::getInstance().incrementMouseWheelCounter();

        Point<int> screenPos;
        if (Component* current = getTargetForGesture (peer, positionWithinPeer, time, screenPos))
            sendMouseWheel (current, screenPos, time, wheel);
    }

    void handleMagnifyGesture (ComponentPeer* const peer, Point<int> positionWithinPeer,
                               Time time, const float scaleFactor)
    {
        Point<int> screenPos;
        if (Component* current = getTargetForGesture (peer, positionWithinPeer, time, screenPos))
            sendMagnifyGesture (current, screenPos, time, scaleFactor);
    }

    //==============================================================================
    Time getLastMouseDownTime() const noexcept              { return mouseDowns[0].time; }
    Point<int> getLastMouseDownPosition() const noexcept    { return mouseDowns[0].position; }

    int getNumberOfMultipleClicks() const noexcept
    {
        int numClicks = 0;

        if (mouseDowns[0].time != Time())
        {
            if (! mouseMovedSignificantlySincePressed)
                ++numClicks;

            for (int i = 1; i < numElementsInArray (mouseDowns); ++i)
            {
                if (mouseDowns[0].canBePartOfMultipleClickWith (mouseDowns[i], MouseEvent::getDoubleClickTimeout() * jmin (i, 2)))
                    ++numClicks;
                else
                    break;
            }
        }

        return numClicks;
    }

    bool hasMouseMovedSignificantlySincePressed() const noexcept
    {
        return mouseMovedSignificantlySincePressed
                || lastTime > mouseDowns[0].time + RelativeTime::milliseconds (300);
    }

    //==============================================================================
    void triggerFakeMove()
    {
        triggerAsyncUpdate();
    }

    void handleAsyncUpdate()
    {
        setScreenPos (lastScreenPos, jmax (lastTime, Time::getCurrentTime()), true);
    }

    //==============================================================================
    void enableUnboundedMouseMovement (bool enable, bool keepCursorVisibleUntilOffscreen)
    {
        enable = enable && isDragging();
        isCursorVisibleUntilOffscreen = keepCursorVisibleUntilOffscreen;

        if (enable != isUnboundedMouseModeOn)
        {
            if ((! enable) && ((! isCursorVisibleUntilOffscreen) || ! unboundedMouseOffset.isOrigin()))
            {
                // when released, return the mouse to within the component's bounds
                if (Component* current = getComponentUnderMouse())
                    Desktop::setMousePosition (current->getScreenBounds()
                                                 .getConstrainedPoint (lastScreenPos));
            }

            isUnboundedMouseModeOn = enable;
            unboundedMouseOffset = Point<int>();

            revealCursor (true);
        }
    }

    void handleUnboundedDrag (Component* current)
    {
        const Rectangle<int> screenArea (current->getParentMonitorArea().expanded (-2, -2));

        if (! screenArea.contains (lastScreenPos))
        {
            const Point<int> componentCentre (current->getScreenBounds().getCentre());
            unboundedMouseOffset += (lastScreenPos - componentCentre);
            Desktop::setMousePosition (componentCentre);
        }
        else if (isCursorVisibleUntilOffscreen
                  && (! unboundedMouseOffset.isOrigin())
                  && screenArea.contains (lastScreenPos + unboundedMouseOffset))
        {
            Desktop::setMousePosition (lastScreenPos + unboundedMouseOffset);
            unboundedMouseOffset = Point<int>();
        }
    }

    //==============================================================================
    void showMouseCursor (MouseCursor cursor, bool forcedUpdate)
    {
        if (isUnboundedMouseModeOn && ((! unboundedMouseOffset.isOrigin()) || ! isCursorVisibleUntilOffscreen))
        {
            cursor = MouseCursor::NoCursor;
            forcedUpdate = true;
        }

        if (forcedUpdate || cursor.getHandle() != currentCursorHandle)
        {
            currentCursorHandle = cursor.getHandle();
            cursor.showInWindow (getPeer());
        }
    }

    void hideCursor()
    {
        showMouseCursor (MouseCursor::NoCursor, true);
    }

    void revealCursor (bool forcedUpdate)
    {
        MouseCursor mc (MouseCursor::NormalCursor);

        if (Component* current = getComponentUnderMouse())
            mc = current->getLookAndFeel().getMouseCursorFor (*current);

        showMouseCursor (mc, forcedUpdate);
    }

    //==============================================================================
    const int index;
    const bool isMouseDevice;
    Point<int> lastScreenPos;
    ModifierKeys buttonState;

private:
    MouseInputSource& source;
    WeakReference<Component> componentUnderMouse;
    ComponentPeer* lastPeer;

    Point<int> unboundedMouseOffset;
    bool isUnboundedMouseModeOn, isCursorVisibleUntilOffscreen;
    void* currentCursorHandle;
    int mouseEventCounter;

    struct RecentMouseDown
    {
        RecentMouseDown() noexcept  : peerID (0) {}

        Point<int> position;
        Time time;
        ModifierKeys buttons;
        uint32 peerID;

        bool canBePartOfMultipleClickWith (const RecentMouseDown& other, const int maxTimeBetweenMs) const
        {
            return time - other.time < RelativeTime::milliseconds (maxTimeBetweenMs)
                    && abs (position.x - other.position.x) < 8
                    && abs (position.y - other.position.y) < 8
                    && buttons == other.buttons
                    && peerID == other.peerID;
        }
    };

    RecentMouseDown mouseDowns[4];
    Time lastTime;
    bool mouseMovedSignificantlySincePressed;

    void registerMouseDown (Point<int> screenPos, Time time,
                            Component& component, const ModifierKeys modifiers) noexcept
    {
        for (int i = numElementsInArray (mouseDowns); --i > 0;)
            mouseDowns[i] = mouseDowns[i - 1];

        mouseDowns[0].position = screenPos;
        mouseDowns[0].time = time;
        mouseDowns[0].buttons = modifiers.withOnlyMouseButtons();

        if (ComponentPeer* const peer = component.getPeer())
            mouseDowns[0].peerID = peer->getUniqueID();
        else
            mouseDowns[0].peerID = 0;

        mouseMovedSignificantlySincePressed = false;
    }

    void registerMouseDrag (Point<int> screenPos) noexcept
    {
        mouseMovedSignificantlySincePressed = mouseMovedSignificantlySincePressed
               || mouseDowns[0].position.getDistanceFrom (screenPos) >= 4;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MouseInputSourceInternal)
};

//==============================================================================
MouseInputSource::MouseInputSource (const int index, const bool isMouseDevice)
{
    pimpl = new MouseInputSourceInternal (*this, index, isMouseDevice);
}

MouseInputSource::~MouseInputSource() {}

bool MouseInputSource::isMouse() const                                  { return pimpl->isMouseDevice; }
bool MouseInputSource::isTouch() const                                  { return ! isMouse(); }
bool MouseInputSource::canHover() const                                 { return isMouse(); }
bool MouseInputSource::hasMouseWheel() const                            { return isMouse(); }
int MouseInputSource::getIndex() const                                  { return pimpl->index; }
bool MouseInputSource::isDragging() const                               { return pimpl->isDragging(); }
Point<int> MouseInputSource::getScreenPosition() const                  { return pimpl->getScreenPosition(); }
ModifierKeys MouseInputSource::getCurrentModifiers() const              { return pimpl->getCurrentModifiers(); }
Component* MouseInputSource::getComponentUnderMouse() const             { return pimpl->getComponentUnderMouse(); }
void MouseInputSource::triggerFakeMove() const                          { pimpl->triggerFakeMove(); }
int MouseInputSource::getNumberOfMultipleClicks() const noexcept        { return pimpl->getNumberOfMultipleClicks(); }
Time MouseInputSource::getLastMouseDownTime() const noexcept            { return pimpl->getLastMouseDownTime(); }
Point<int> MouseInputSource::getLastMouseDownPosition() const noexcept  { return pimpl->getLastMouseDownPosition(); }
bool MouseInputSource::hasMouseMovedSignificantlySincePressed() const noexcept  { return pimpl->hasMouseMovedSignificantlySincePressed(); }
bool MouseInputSource::canDoUnboundedMovement() const noexcept          { return isMouse(); }
void MouseInputSource::enableUnboundedMouseMovement (bool isEnabled, bool keepCursorVisibleUntilOffscreen)    { pimpl->enableUnboundedMouseMovement (isEnabled, keepCursorVisibleUntilOffscreen); }
bool MouseInputSource::hasMouseCursor() const noexcept                  { return isMouse(); }
void MouseInputSource::showMouseCursor (const MouseCursor& cursor)      { pimpl->showMouseCursor (cursor, false); }
void MouseInputSource::hideCursor()                                     { pimpl->hideCursor(); }
void MouseInputSource::revealCursor()                                   { pimpl->revealCursor (false); }
void MouseInputSource::forceMouseCursorUpdate()                         { pimpl->revealCursor (true); }

void MouseInputSource::handleEvent (ComponentPeer* peer, Point<int> positionWithinPeer,
                                    const int64 time, const ModifierKeys mods)
{
    pimpl->handleEvent (peer, positionWithinPeer, Time (time), mods.withOnlyMouseButtons());
}

void MouseInputSource::handleWheel (ComponentPeer* const peer, Point<int> positionWithinPeer,
                                    const int64 time, const MouseWheelDetails& wheel)
{
    pimpl->handleWheel (peer, positionWithinPeer, Time (time), wheel);
}

void MouseInputSource::handleMagnifyGesture (ComponentPeer* const peer, Point<int> positionWithinPeer,
                                             const int64 time, const float scaleFactor)
{
    pimpl->handleMagnifyGesture (peer, positionWithinPeer, Time (time), scaleFactor);
}
