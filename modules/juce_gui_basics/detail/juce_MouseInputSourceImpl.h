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

namespace juce::detail
{

class MouseInputSourceImpl    : private AsyncUpdater
{
public:
    using SH = ScalingHelpers;

    MouseInputSourceImpl (int i, MouseInputSource::InputSourceType type)
        : index (i),
          inputType (type)
    {}

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
        return ModifierKeys::currentModifiers
                .withoutMouseButtons()
                .withFlags (buttonState.getRawFlags());
    }

    ComponentPeer* getPeer() noexcept
    {
        if (! ComponentPeer::isValidPeer (lastPeer))
            lastPeer = nullptr;

        return lastPeer;
    }

    static Component* findComponentAt (Point<float> screenPos, ComponentPeer* peer)
    {
        if (! ComponentPeer::isValidPeer (peer))
            return nullptr;

        auto relativePos = SH::unscaledScreenPosToScaled (peer->getComponent(),
                                                          peer->globalToLocal (screenPos));
        auto& comp = peer->getComponent();

        // (the contains() call is needed to test for overlapping desktop windows)
        if (comp.contains (relativePos))
            return comp.getComponentAt (relativePos);

        return nullptr;
    }

    Point<float> getScreenPosition() const noexcept
    {
        // This needs to return the live position if possible, but it mustn't update the lastScreenPos
        // value, because that can cause continuity problems.
        return SH::unscaledScreenPosToScaled (getRawScreenPosition());
    }

    Point<float> getRawScreenPosition() const noexcept
    {
        return unboundedMouseOffset + (inputType != MouseInputSource::InputSourceType::touch ? MouseInputSource::getCurrentRawMousePosition()
                                                                                             : lastPointerState.position);
    }

    void setScreenPosition (Point<float> p)
    {
        MouseInputSource::setRawMousePosition (SH::scaledScreenPosToUnscaled (p));
    }

    //==============================================================================
   #if JUCE_DUMP_MOUSE_EVENTS
    #define JUCE_MOUSE_EVENT_DBG(desc, screenPos)   DBG ("Mouse " << desc << " #" << index \
                                                    << ": " << SH::screenPosToLocalPos (comp, screenPos).toString() \
                                                    << " - Comp: " << String::toHexString ((pointer_sized_int) &comp));
   #else
    #define JUCE_MOUSE_EVENT_DBG(desc, screenPos)
   #endif

    void sendMouseEnter (Component& comp, const detail::PointerState& pointerState, Time time)
    {
        JUCE_MOUSE_EVENT_DBG ("enter", pointerState.position)
        Component::internalMouseEnter (&comp,
                                       MouseInputSource (this),
                                       SH::screenPosToLocalPos (comp, pointerState.position),
                                       time);
    }

    void sendMouseExit (Component& comp, const detail::PointerState& pointerState, Time time)
    {
        JUCE_MOUSE_EVENT_DBG ("exit", pointerState.position)
        Component::internalMouseExit (&comp,
                                      MouseInputSource (this),
                                      SH::screenPosToLocalPos (comp, pointerState.position),
                                      time);
    }

    void sendMouseMove (Component& comp, const detail::PointerState& pointerState, Time time)
    {
        JUCE_MOUSE_EVENT_DBG ("move", pointerState.position)
        Component::internalMouseMove (&comp,
                                      MouseInputSource (this),
                                      SH::screenPosToLocalPos (comp, pointerState.position),
                                      time);
    }

    void sendMouseDown (Component& comp, const detail::PointerState& pointerState, Time time)
    {
        JUCE_MOUSE_EVENT_DBG ("down", pointerState.position)
        Component::internalMouseDown (&comp,
                                      MouseInputSource (this),
                                      pointerState.withPosition (SH::screenPosToLocalPos (comp, pointerState.position)),
                                      time);
    }

    void sendMouseDrag (Component& comp, const detail::PointerState& pointerState, Time time)
    {
        JUCE_MOUSE_EVENT_DBG ("drag", pointerState.position)
        Component::internalMouseDrag (&comp,
                                      MouseInputSource (this),
                                      pointerState.withPosition (SH::screenPosToLocalPos (comp, pointerState.position)),
                                      time);
    }

    void sendMouseUp (Component& comp, const detail::PointerState& pointerState, Time time, ModifierKeys oldMods)
    {
        JUCE_MOUSE_EVENT_DBG ("up", pointerState.position)
        Component::internalMouseUp (&comp,
                                    MouseInputSource (this),
                                    pointerState.withPosition (SH::screenPosToLocalPos (comp, pointerState.position)),
                                    time,
                                    oldMods);
    }

    void sendMouseWheel (Component& comp, Point<float> screenPos, Time time, const MouseWheelDetails& wheel)
    {
        JUCE_MOUSE_EVENT_DBG ("wheel", screenPos)
        Component::internalMouseWheel (&comp,
                                       MouseInputSource (this),
                                       SH::screenPosToLocalPos (comp, screenPos),
                                       time,
                                       wheel);
    }

    void sendMagnifyGesture (Component& comp, Point<float> screenPos, Time time, float amount)
    {
        JUCE_MOUSE_EVENT_DBG ("magnify", screenPos)
        Component::internalMagnifyGesture (&comp,
                                           MouseInputSource (this),
                                           SH::screenPosToLocalPos (comp, screenPos),
                                           time,
                                           amount);
    }

    #undef JUCE_MOUSE_EVENT_DBG

    //==============================================================================
    // (returns true if the button change caused a modal event loop)
    bool setButtons (const detail::PointerState& pointerState, Time time, ModifierKeys newButtonState)
    {
        if (buttonState == newButtonState)
            return false;

        // (avoid sending a spurious mouse-drag when we receive a mouse-up)
        if (! (isDragging() && ! newButtonState.isAnyMouseButtonDown()))
            setPointerState (pointerState, time, false);

        // (ignore secondary clicks when there's already a button down)
        if (buttonState.isAnyMouseButtonDown() == newButtonState.isAnyMouseButtonDown())
        {
            buttonState = newButtonState;
            return false;
        }

        auto lastCounter = mouseEventCounter;

        if (buttonState.isAnyMouseButtonDown())
        {
            if (auto* current = getComponentUnderMouse())
            {
                auto oldMods = getCurrentModifiers();
                buttonState = newButtonState; // must change this before calling sendMouseUp, in case it runs a modal loop

                sendMouseUp (*current, pointerState.withPositionOffset (unboundedMouseOffset), time, oldMods);

                if (lastCounter != mouseEventCounter)
                    return true; // if a modal loop happened, then newButtonState is no longer valid.
            }

            enableUnboundedMouseMovement (false, false);
        }

        buttonState = newButtonState;

        if (buttonState.isAnyMouseButtonDown())
        {
            Desktop::getInstance().incrementMouseClickCounter();

            if (auto* current = getComponentUnderMouse())
            {
                registerMouseDown (pointerState.position, time, *current, buttonState,
                                   inputType == MouseInputSource::InputSourceType::touch);
                sendMouseDown (*current, pointerState, time);
            }
        }

        return lastCounter != mouseEventCounter;
    }

    void setComponentUnderMouse (Component* newComponent, const detail::PointerState& pointerState, Time time)
    {
        auto* current = getComponentUnderMouse();

        if (newComponent != current)
        {
            WeakReference<Component> safeNewComp (newComponent);
            auto originalButtonState = buttonState;

            if (current != nullptr)
            {
                WeakReference<Component> safeOldComp (current);
                setButtons (pointerState, time, ModifierKeys());

                if (auto oldComp = safeOldComp.get())
                {
                    componentUnderMouse = safeNewComp;
                    sendMouseExit (*oldComp, pointerState, time);
                }

                buttonState = originalButtonState;
            }

            componentUnderMouse = safeNewComp.get();
            current = safeNewComp.get();

            if (current != nullptr)
                sendMouseEnter (*current, pointerState, time);

            revealCursor (false);
            setButtons (pointerState, time, originalButtonState);
        }
    }

    void setPeer (ComponentPeer& newPeer, const detail::PointerState& pointerState, Time time)
    {
        if (&newPeer != lastPeer && (   findComponentAt (pointerState.position, &newPeer) != nullptr
                                        || findComponentAt (pointerState.position, lastPeer) == nullptr))
        {
            setComponentUnderMouse (nullptr, pointerState, time);
            lastPeer = &newPeer;
            setComponentUnderMouse (findComponentAt (pointerState.position, getPeer()), pointerState, time);
        }
    }

    void setPointerState (const detail::PointerState& newPointerState, Time time, bool forceUpdate)
    {
        const auto& newScreenPos = newPointerState.position;

        if (! isDragging())
            setComponentUnderMouse (findComponentAt (newScreenPos, getPeer()), newPointerState, time);

        if ((newPointerState != lastPointerState) || forceUpdate)
        {
            cancelPendingUpdate();

            lastPointerState = newPointerState;

            if (auto* current = getComponentUnderMouse())
            {
                if (isDragging())
                {
                    registerMouseDrag (newScreenPos);
                    sendMouseDrag (*current, newPointerState.withPositionOffset (unboundedMouseOffset), time);

                    if (isUnboundedMouseModeOn)
                        handleUnboundedDrag (*current);
                }
                else
                {
                    sendMouseMove (*current, newPointerState, time);
                }
            }

            revealCursor (false);
        }
    }

    //==============================================================================
    void handleEvent (ComponentPeer& newPeer, Point<float> positionWithinPeer, Time time,
                      const ModifierKeys newMods, float newPressure, float newOrientation, PenDetails pen)
    {
        lastTime = time;
        ++mouseEventCounter;
        const auto pointerState = detail::PointerState().withPosition (newPeer.localToGlobal (positionWithinPeer))
                                                        .withPressure (newPressure)
                                                        .withOrientation (newOrientation)
                                                        .withRotation (MouseInputSource::defaultRotation)
                                                        .withTiltX (pen.tiltX)
                                                        .withTiltY (pen.tiltY);

        if (isDragging() && newMods.isAnyMouseButtonDown())
        {
            setPointerState (pointerState, time, false);
        }
        else
        {
            setPeer (newPeer, pointerState, time);

            if (auto* peer = getPeer())
            {
                if (setButtons (pointerState, time, newMods))
                    return; // some modal events have been dispatched, so the current event is now out-of-date

                peer = getPeer();

                if (peer != nullptr)
                    setPointerState (pointerState, time, false);
            }
        }
    }

    Component* getTargetForGesture (ComponentPeer& peer, Point<float> positionWithinPeer,
                                    Time time, Point<float>& screenPos)
    {
        lastTime = time;
        ++mouseEventCounter;

        screenPos = peer.localToGlobal (positionWithinPeer);
        const auto pointerState = lastPointerState.withPosition (screenPos);
        setPeer (peer, pointerState, time);
        setPointerState (pointerState, time, false);
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
        else
            screenPos = peer.localToGlobal (positionWithinPeer);

        if (auto target = lastNonInertialWheelTarget.get())
            sendMouseWheel (*target, screenPos, time, wheel);
    }

    void handleMagnifyGesture (ComponentPeer& peer, Point<float> positionWithinPeer,
                               Time time, const float scaleFactor)
    {
        Point<float> screenPos;

        if (auto* current = getTargetForGesture (peer, positionWithinPeer, time, screenPos))
            sendMagnifyGesture (*current, screenPos, time, scaleFactor);
    }

    //==============================================================================
    Time getLastMouseDownTime() const noexcept
    {
        return mouseDowns[0].time;
    }

    Point<float> getLastMouseDownPosition() const noexcept
    {
        return SH::unscaledScreenPosToScaled (mouseDowns[0].position);
    }

    int getNumberOfMultipleClicks() const noexcept
    {
        int numClicks = 1;

        if (! isLongPressOrDrag())
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

    bool isLongPressOrDrag() const noexcept
    {
        return movedSignificantly ||
               lastTime > (mouseDowns[0].time + RelativeTime::milliseconds (300));
    }

    bool hasMovedSignificantlySincePressed() const noexcept
    {
        return movedSignificantly;
    }

    // Deprecated method
    bool hasMouseMovedSignificantlySincePressed() const noexcept
    {
        return isLongPressOrDrag();
    }

    //==============================================================================
    void triggerFakeMove()
    {
        triggerAsyncUpdate();
    }

    void handleAsyncUpdate() override
    {
        setPointerState (lastPointerState,
                         jmax (lastTime, Time::getCurrentTime()), true);
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
                if (auto* current = getComponentUnderMouse())
                    setScreenPosition (current->getScreenBounds().toFloat()
                                              .getConstrainedPoint (SH::unscaledScreenPosToScaled (lastPointerState.position)));
            }

            isUnboundedMouseModeOn = enable;
            unboundedMouseOffset = {};

            revealCursor (true);
        }
    }

    void handleUnboundedDrag (Component& current)
    {
        auto componentScreenBounds = SH::scaledScreenPosToUnscaled (current.getParentMonitorArea()
                                                                           .reduced (2, 2)
                                                                           .toFloat());

        if (! componentScreenBounds.contains (lastPointerState.position))
        {
            auto componentCentre = current.getScreenBounds().toFloat().getCentre();
            unboundedMouseOffset += (lastPointerState.position - SH::scaledScreenPosToUnscaled (componentCentre));
            setScreenPosition (componentCentre);
        }
        else if (isCursorVisibleUntilOffscreen
                 && (! unboundedMouseOffset.isOrigin())
                 && componentScreenBounds.contains (lastPointerState.position + unboundedMouseOffset))
        {
            MouseInputSource::setRawMousePosition (lastPointerState.position + unboundedMouseOffset);
            unboundedMouseOffset = {};
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

        if (auto* current = getComponentUnderMouse())
            mc = current->getLookAndFeel().getMouseCursorFor (*current);

        showMouseCursor (mc, forcedUpdate);
    }

    //==============================================================================
    const int index;
    const MouseInputSource::InputSourceType inputType;
    Point<float> unboundedMouseOffset; // NB: these are unscaled coords
    detail::PointerState lastPointerState;
    ModifierKeys buttonState;

    bool isUnboundedMouseModeOn = false, isCursorVisibleUntilOffscreen = false;

private:
    WeakReference<Component> componentUnderMouse, lastNonInertialWheelTarget;
    ComponentPeer* lastPeer = nullptr;

    void* currentCursorHandle = nullptr;
    int mouseEventCounter = 0;

    struct RecentMouseDown
    {
        RecentMouseDown() = default;

        Point<float> position;
        Time time;
        ModifierKeys buttons;
        uint32 peerID = 0;
        bool isTouch = false;

        bool canBePartOfMultipleClickWith (const RecentMouseDown& other, int maxTimeBetweenMs) const noexcept
        {
            return time - other.time < RelativeTime::milliseconds (maxTimeBetweenMs)
                    && std::abs (position.x - other.position.x) < (float) getPositionToleranceForInputType()
                    && std::abs (position.y - other.position.y) < (float) getPositionToleranceForInputType()
                    && buttons == other.buttons
                    && peerID == other.peerID;
        }

        int getPositionToleranceForInputType() const noexcept    { return isTouch ? 25 : 8;  }
    };

    RecentMouseDown mouseDowns[4];
    Time lastTime;
    bool movedSignificantly = false;

    void registerMouseDown (Point<float> screenPos, Time time, Component& component,
                            const ModifierKeys modifiers, bool isTouchSource) noexcept
    {
        for (int i = numElementsInArray (mouseDowns); --i > 0;)
            mouseDowns[i] = mouseDowns[i - 1];

        mouseDowns[0].position = screenPos;
        mouseDowns[0].time = time;
        mouseDowns[0].buttons = modifiers.withOnlyMouseButtons();
        mouseDowns[0].isTouch = isTouchSource;

        if (auto* peer = component.getPeer())
            mouseDowns[0].peerID = peer->getUniqueID();
        else
            mouseDowns[0].peerID = 0;

        movedSignificantly = false;
        lastNonInertialWheelTarget = nullptr;
    }

    void registerMouseDrag (Point<float> screenPos) noexcept
    {
        movedSignificantly = movedSignificantly || mouseDowns[0].position.getDistanceFrom (screenPos) >= 4;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MouseInputSourceImpl)
};

} // namespace juce::detail
