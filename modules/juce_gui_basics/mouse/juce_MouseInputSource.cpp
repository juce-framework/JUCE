/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

class MouseInputSourceInternal   : private AsyncUpdater
{
public:
    //==============================================================================
    MouseInputSourceInternal (const int i, const bool isMouse)
        : index (i), isMouseDevice (isMouse), pressure (0.0f),
          isUnboundedMouseModeOn (false), isCursorVisibleUntilOffscreen (false),
          lastPeer (nullptr), currentCursorHandle (nullptr),
          mouseEventCounter (0), mouseMovedSignificantlySincePressed (false)
    {
    }

    //==============================================================================
    bool isDragging() const noexcept
    {
        return buttonState.isAnyMouseButtonDown();
    }

    Component* getComponentUnderMouse() const noexcept
    {
        return componentUnderMouse.get();
    }

    ModifierKeys getCurrentModifiers() const noexcept
    {
        return ModifierKeys::getCurrentModifiers().withoutMouseButtons().withFlags (buttonState.getRawFlags());
    }

    ComponentPeer* getPeer() noexcept
    {
        if (! ComponentPeer::isValidPeer (lastPeer))
            lastPeer = nullptr;

        return lastPeer;
    }

    static Point<float> screenPosToLocalPos (Component& comp, Point<float> pos)
    {
        if (ComponentPeer* const peer = comp.getPeer())
        {
            pos = peer->globalToLocal (pos);
            Component& peerComp = peer->getComponent();
            return comp.getLocalPoint (&peerComp, ScalingHelpers::unscaledScreenPosToScaled (peerComp, pos));
        }

        return comp.getLocalPoint (nullptr, ScalingHelpers::unscaledScreenPosToScaled (comp, pos));
    }

    Component* findComponentAt (Point<float> screenPos)
    {
        if (ComponentPeer* const peer = getPeer())
        {
            Point<float> relativePos (ScalingHelpers::unscaledScreenPosToScaled (peer->getComponent(),
                                                                                 peer->globalToLocal (screenPos)));
            Component& comp = peer->getComponent();

            const Point<int> pos (relativePos.roundToInt());

            // (the contains() call is needed to test for overlapping desktop windows)
            if (comp.contains (pos))
                return comp.getComponentAt (pos);
        }

        return nullptr;
    }

    Point<float> getScreenPosition() const
    {
        // This needs to return the live position if possible, but it mustn't update the lastScreenPos
        // value, because that can cause continuity problems.
        return ScalingHelpers::unscaledScreenPosToScaled
                    (unboundedMouseOffset + (isMouseDevice ? MouseInputSource::getCurrentRawMousePosition()
                                                           : lastScreenPos));
    }

    void setScreenPosition (Point<float> p)
    {
        MouseInputSource::setRawMousePosition (ScalingHelpers::scaledScreenPosToUnscaled (p));
    }

    bool isPressureValid() const noexcept       { return pressure > 0.0f && pressure < 1.0f; }

    //==============================================================================
   #if JUCE_DUMP_MOUSE_EVENTS
    #define JUCE_MOUSE_EVENT_DBG(desc)   DBG ("Mouse " << desc << " #" << index \
                                                << ": " << screenPosToLocalPos (comp, screenPos).toString() \
                                                << " - Comp: " << String::toHexString ((pointer_sized_int) &comp));
   #else
    #define JUCE_MOUSE_EVENT_DBG(desc)
   #endif

    void sendMouseEnter (Component& comp, Point<float> screenPos, Time time)
    {
        JUCE_MOUSE_EVENT_DBG ("enter")
        comp.internalMouseEnter (MouseInputSource (this), screenPosToLocalPos (comp, screenPos), time);
    }

    void sendMouseExit (Component& comp, Point<float> screenPos, Time time)
    {
        JUCE_MOUSE_EVENT_DBG ("exit")
        comp.internalMouseExit (MouseInputSource (this), screenPosToLocalPos (comp, screenPos), time);
    }

    void sendMouseMove (Component& comp, Point<float> screenPos, Time time)
    {
        JUCE_MOUSE_EVENT_DBG ("move")
        comp.internalMouseMove (MouseInputSource (this), screenPosToLocalPos (comp, screenPos), time);
    }

    void sendMouseDown (Component& comp, Point<float> screenPos, Time time)
    {
        JUCE_MOUSE_EVENT_DBG ("down")
        comp.internalMouseDown (MouseInputSource (this), screenPosToLocalPos (comp, screenPos), time, pressure);
    }

    void sendMouseDrag (Component& comp, Point<float> screenPos, Time time)
    {
        JUCE_MOUSE_EVENT_DBG ("drag")
        comp.internalMouseDrag (MouseInputSource (this), screenPosToLocalPos (comp, screenPos), time, pressure);
    }

    void sendMouseUp (Component& comp, Point<float> screenPos, Time time, const ModifierKeys oldMods)
    {
        JUCE_MOUSE_EVENT_DBG ("up")
        comp.internalMouseUp (MouseInputSource (this), screenPosToLocalPos (comp, screenPos), time, oldMods);
    }

    void sendMouseWheel (Component& comp, Point<float> screenPos, Time time, const MouseWheelDetails& wheel)
    {
        JUCE_MOUSE_EVENT_DBG ("wheel")
        comp.internalMouseWheel (MouseInputSource (this), screenPosToLocalPos (comp, screenPos), time, wheel);
    }

    void sendMagnifyGesture (Component& comp, Point<float> screenPos, Time time, const float amount)
    {
        JUCE_MOUSE_EVENT_DBG ("magnify")
        comp.internalMagnifyGesture (MouseInputSource (this), screenPosToLocalPos (comp, screenPos), time, amount);
    }

    //==============================================================================
    // (returns true if the button change caused a modal event loop)
    bool setButtons (Point<float> screenPos, Time time, const ModifierKeys newButtonState)
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

                sendMouseUp (*current, screenPos + unboundedMouseOffset, time, oldMods);

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
                sendMouseDown (*current, screenPos, time);
            }
        }

        return lastCounter != mouseEventCounter;
    }

    void setComponentUnderMouse (Component* const newComponent, Point<float> screenPos, Time time)
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
                    sendMouseExit (*safeOldComp, screenPos, time);
                }

                buttonState = originalButtonState;
            }

            current = componentUnderMouse = safeNewComp;

            if (current != nullptr)
                sendMouseEnter (*current, screenPos, time);

            revealCursor (false);
            setButtons (screenPos, time, originalButtonState);
        }
    }

    void setPeer (ComponentPeer& newPeer, Point<float> screenPos, Time time)
    {
        ModifierKeys::updateCurrentModifiers();

        if (&newPeer != lastPeer)
        {
            setComponentUnderMouse (nullptr, screenPos, time);
            lastPeer = &newPeer;
            setComponentUnderMouse (findComponentAt (screenPos), screenPos, time);
        }
    }

    void setScreenPos (Point<float> newScreenPos, Time time, const bool forceUpdate)
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
                    sendMouseDrag (*current, newScreenPos + unboundedMouseOffset, time);

                    if (isUnboundedMouseModeOn)
                        handleUnboundedDrag (*current);
                }
                else
                {
                    sendMouseMove (*current, newScreenPos, time);
                }
            }

            revealCursor (false);
        }
    }

    //==============================================================================
    void handleEvent (ComponentPeer& newPeer, Point<float> positionWithinPeer, Time time,
                      const ModifierKeys newMods, float newPressure)
    {
        lastTime = time;
        const bool pressureChanged = (pressure != newPressure);
        pressure = newPressure;
        ++mouseEventCounter;
        const Point<float> screenPos (newPeer.localToGlobal (positionWithinPeer));

        if (isDragging() && newMods.isAnyMouseButtonDown())
        {
            setScreenPos (screenPos, time, pressureChanged);
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
                    setScreenPos (screenPos, time, pressureChanged);
            }
        }
    }

    Component* getTargetForGesture (ComponentPeer& peer, Point<float> positionWithinPeer,
                                    Time time, Point<float>& screenPos)
    {
        lastTime = time;
        ++mouseEventCounter;

        screenPos = peer.localToGlobal (positionWithinPeer);
        setPeer (peer, screenPos, time);
        setScreenPos (screenPos, time, false);
        triggerFakeMove();

        return getComponentUnderMouse();
    }

    void handleWheel (ComponentPeer& peer, Point<float> positionWithinPeer,
                      Time time, const MouseWheelDetails& wheel)
    {
        Desktop::getInstance().incrementMouseWheelCounter();
        Point<float> screenPos;

        // This will make sure that when the wheel spins in its inertial phase, any events
        // continue to be sent to the last component that the mouse was over when it was being
        // actively controlled by the user. This avoids confusion when scrolling through nested
        // scrollable components.
        if (lastNonInertialWheelTarget == nullptr || ! wheel.isInertial)
            lastNonInertialWheelTarget = getTargetForGesture (peer, positionWithinPeer, time, screenPos);

        if (Component* target = lastNonInertialWheelTarget)
            sendMouseWheel (*target, screenPos, time, wheel);
    }

    void handleMagnifyGesture (ComponentPeer& peer, Point<float> positionWithinPeer,
                               Time time, const float scaleFactor)
    {
        Point<float> screenPos;
        if (Component* current = getTargetForGesture (peer, positionWithinPeer, time, screenPos))
            sendMagnifyGesture (*current, screenPos, time, scaleFactor);
    }

    //==============================================================================
    Time getLastMouseDownTime() const noexcept              { return mouseDowns[0].time; }
    Point<float> getLastMouseDownPosition() const noexcept  { return ScalingHelpers::unscaledScreenPosToScaled (mouseDowns[0].position); }

    int getNumberOfMultipleClicks() const noexcept
    {
        int numClicks = 1;

        if (! hasMouseMovedSignificantlySincePressed())
        {
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

    void handleAsyncUpdate() override
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
                    setScreenPosition (current->getScreenBounds().toFloat()
                                          .getConstrainedPoint (ScalingHelpers::unscaledScreenPosToScaled (lastScreenPos)));
            }

            isUnboundedMouseModeOn = enable;
            unboundedMouseOffset = Point<float>();

            revealCursor (true);
        }
    }

    void handleUnboundedDrag (Component& current)
    {
        const Rectangle<float> componentScreenBounds
                = ScalingHelpers::scaledScreenPosToUnscaled (current.getParentMonitorArea().reduced (2, 2).toFloat());

        if (! componentScreenBounds.contains (lastScreenPos))
        {
            const Point<float> componentCentre (current.getScreenBounds().toFloat().getCentre());
            unboundedMouseOffset += (lastScreenPos - ScalingHelpers::scaledScreenPosToUnscaled (componentCentre));
            setScreenPosition (componentCentre);
        }
        else if (isCursorVisibleUntilOffscreen
                  && (! unboundedMouseOffset.isOrigin())
                  && componentScreenBounds.contains (lastScreenPos + unboundedMouseOffset))
        {
            MouseInputSource::setRawMousePosition (lastScreenPos + unboundedMouseOffset);
            unboundedMouseOffset = Point<float>();
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
    Point<float> lastScreenPos, unboundedMouseOffset; // NB: these are unscaled coords
    ModifierKeys buttonState;
    float pressure;

    bool isUnboundedMouseModeOn, isCursorVisibleUntilOffscreen;

private:
    WeakReference<Component> componentUnderMouse, lastNonInertialWheelTarget;
    ComponentPeer* lastPeer;

    void* currentCursorHandle;
    int mouseEventCounter;

    struct RecentMouseDown
    {
        RecentMouseDown() noexcept  : peerID (0) {}

        Point<float> position;
        Time time;
        ModifierKeys buttons;
        uint32 peerID;

        bool canBePartOfMultipleClickWith (const RecentMouseDown& other, const int maxTimeBetweenMs) const
        {
            return time - other.time < RelativeTime::milliseconds (maxTimeBetweenMs)
                    && std::abs (position.x - other.position.x) < 8
                    && std::abs (position.y - other.position.y) < 8
                    && buttons == other.buttons
                    && peerID == other.peerID;
        }
    };

    RecentMouseDown mouseDowns[4];
    Time lastTime;
    bool mouseMovedSignificantlySincePressed;

    void registerMouseDown (Point<float> screenPos, Time time,
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
        lastNonInertialWheelTarget = nullptr;
    }

    void registerMouseDrag (Point<float> screenPos) noexcept
    {
        mouseMovedSignificantlySincePressed = mouseMovedSignificantlySincePressed
               || mouseDowns[0].position.getDistanceFrom (screenPos) >= 4;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MouseInputSourceInternal)
};

//==============================================================================
MouseInputSource::MouseInputSource (MouseInputSourceInternal* s) noexcept   : pimpl (s)  {}
MouseInputSource::MouseInputSource (const MouseInputSource& other) noexcept : pimpl (other.pimpl)  {}
MouseInputSource::~MouseInputSource() noexcept {}

MouseInputSource& MouseInputSource::operator= (const MouseInputSource& other) noexcept
{
    pimpl = other.pimpl;
    return *this;
}

bool MouseInputSource::isMouse() const noexcept                          { return pimpl->isMouseDevice; }
bool MouseInputSource::isTouch() const noexcept                          { return ! isMouse(); }
bool MouseInputSource::canHover() const noexcept                         { return isMouse(); }
bool MouseInputSource::hasMouseWheel() const noexcept                    { return isMouse(); }
int MouseInputSource::getIndex() const noexcept                          { return pimpl->index; }
bool MouseInputSource::isDragging() const noexcept                       { return pimpl->isDragging(); }
Point<float> MouseInputSource::getScreenPosition() const noexcept        { return pimpl->getScreenPosition(); }
ModifierKeys MouseInputSource::getCurrentModifiers() const noexcept      { return pimpl->getCurrentModifiers(); }
float MouseInputSource::getCurrentPressure() const noexcept              { return pimpl->pressure; }
bool MouseInputSource::isPressureValid() const noexcept                  { return pimpl->isPressureValid(); }
Component* MouseInputSource::getComponentUnderMouse() const              { return pimpl->getComponentUnderMouse(); }
void MouseInputSource::triggerFakeMove() const                           { pimpl->triggerFakeMove(); }
int MouseInputSource::getNumberOfMultipleClicks() const noexcept         { return pimpl->getNumberOfMultipleClicks(); }
Time MouseInputSource::getLastMouseDownTime() const noexcept             { return pimpl->getLastMouseDownTime(); }
Point<float> MouseInputSource::getLastMouseDownPosition() const noexcept { return pimpl->getLastMouseDownPosition(); }
bool MouseInputSource::hasMouseMovedSignificantlySincePressed() const noexcept  { return pimpl->hasMouseMovedSignificantlySincePressed(); }
bool MouseInputSource::canDoUnboundedMovement() const noexcept           { return isMouse(); }
void MouseInputSource::enableUnboundedMouseMovement (bool isEnabled, bool keepCursorVisibleUntilOffscreen) const
                                                                         { pimpl->enableUnboundedMouseMovement (isEnabled, keepCursorVisibleUntilOffscreen); }
bool MouseInputSource::isUnboundedMouseMovementEnabled() const           { return pimpl->isUnboundedMouseModeOn; }
bool MouseInputSource::hasMouseCursor() const noexcept                   { return isMouse(); }
void MouseInputSource::showMouseCursor (const MouseCursor& cursor)       { pimpl->showMouseCursor (cursor, false); }
void MouseInputSource::hideCursor()                                      { pimpl->hideCursor(); }
void MouseInputSource::revealCursor()                                    { pimpl->revealCursor (false); }
void MouseInputSource::forceMouseCursorUpdate()                          { pimpl->revealCursor (true); }
void MouseInputSource::setScreenPosition (Point<float> p)                { pimpl->setScreenPosition (p); }

void MouseInputSource::handleEvent (ComponentPeer& peer, Point<float> pos, int64 time, ModifierKeys mods, float pressure)
{
    pimpl->handleEvent (peer, pos, Time (time), mods.withOnlyMouseButtons(), pressure);
}

void MouseInputSource::handleWheel (ComponentPeer& peer, Point<float> pos, int64 time, const MouseWheelDetails& wheel)
{
    pimpl->handleWheel (peer, pos, Time (time), wheel);
}

void MouseInputSource::handleMagnifyGesture (ComponentPeer& peer, Point<float> pos, int64 time, float scaleFactor)
{
    pimpl->handleMagnifyGesture (peer, pos, Time (time), scaleFactor);
}

const float MouseInputSource::invalidPressure = 0.0f;

//==============================================================================
struct MouseInputSource::SourceList  : public Timer
{
    SourceList()
    {
        addSource();
    }

    bool addSource();

    void addSource (int index, bool isMouse)
    {
        MouseInputSourceInternal* s = new MouseInputSourceInternal (index, isMouse);
        sources.add (s);
        sourceArray.add (MouseInputSource (s));
    }

    MouseInputSource* getMouseSource (int index) const noexcept
    {
        return isPositiveAndBelow (index, sourceArray.size()) ? &sourceArray.getReference (index)
                                                              : nullptr;
    }

    MouseInputSource* getOrCreateMouseInputSource (int touchIndex)
    {
        jassert (touchIndex >= 0 && touchIndex < 100); // sanity-check on number of fingers

        for (;;)
        {
            if (MouseInputSource* mouse = getMouseSource (touchIndex))
                return mouse;

            if (! addSource())
            {
                jassertfalse; // not enough mouse sources!
                return nullptr;
            }
        }
    }

    int getNumDraggingMouseSources() const noexcept
    {
        int num = 0;

        for (int i = 0; i < sources.size(); ++i)
            if (sources.getUnchecked(i)->isDragging())
                ++num;

        return num;
    }

    MouseInputSource* getDraggingMouseSource (int index) const noexcept
    {
        int num = 0;

        for (int i = 0; i < sources.size(); ++i)
        {
            MouseInputSource* const mi = &(sourceArray.getReference(i));

            if (mi->isDragging())
            {
                if (index == num)
                    return mi;

                ++num;
            }
        }

        return nullptr;
    }

    void beginDragAutoRepeat (const int interval)
    {
        if (interval > 0)
        {
            if (getTimerInterval() != interval)
                startTimer (interval);
        }
        else
        {
            stopTimer();
        }
    }

    void timerCallback() override
    {
        int numMiceDown = 0;

        for (int i = 0; i < sources.size(); ++i)
        {
            MouseInputSourceInternal* const mi = sources.getUnchecked(i);

            if (mi->isDragging())
            {
                mi->triggerFakeMove();
                ++numMiceDown;
            }
        }

        if (numMiceDown == 0)
            stopTimer();
    }

    OwnedArray<MouseInputSourceInternal> sources;
    Array<MouseInputSource> sourceArray;
};
