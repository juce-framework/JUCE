/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

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

#include "../../../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_MouseInputSource.h"
#include "juce_MouseEvent.h"
#include "../juce_Component.h"
#include "../../../events/juce_AsyncUpdater.h"
#include "../../../events/juce_MessageManager.h"
#include "../lookandfeel/juce_LookAndFeel.h"
#include "../windows/juce_ComponentPeer.h"


//==============================================================================
class MouseInputSourceInternal   : public AsyncUpdater
{
public:
    //==============================================================================
    MouseInputSourceInternal (MouseInputSource& source_, const int index_, const bool isMouseDevice_)
        : index (index_), isMouseDevice (isMouseDevice_), source (source_), lastPeer (0),
          isUnboundedMouseModeOn (false), isCursorVisibleUntilOffscreen (false), currentCursorHandle (0),
          mouseEventCounter (0)
    {
    }

    //==============================================================================
    bool isDragging() const throw()
    {
        return buttonState.isAnyMouseButtonDown();
    }

    Component* getComponentUnderMouse() const
    {
        return static_cast <Component*> (componentUnderMouse);
    }

    const ModifierKeys getCurrentModifiers() const
    {
        return ModifierKeys::getCurrentModifiers().withoutMouseButtons().withFlags (buttonState.getRawFlags());
    }

    ComponentPeer* getPeer()
    {
        if (! ComponentPeer::isValidPeer (lastPeer))
            lastPeer = 0;

        return lastPeer;
    }

    Component* findComponentAt (const Point<int>& screenPos)
    {
        ComponentPeer* const peer = getPeer();

        if (peer != 0)
        {
            Component* const comp = peer->getComponent();
            const Point<int> relativePos (comp->getLocalPoint (0, screenPos));

            // (the contains() call is needed to test for overlapping desktop windows)
            if (comp->contains (relativePos))
                return comp->getComponentAt (relativePos);
        }

        return 0;
    }

    const Point<int> getScreenPosition() const
    {
        // This needs to return the live position if possible, but it mustn't update the lastScreenPos
        // value, because that can cause continuity problems.
        return unboundedMouseOffset + (isMouseDevice ? MouseInputSource::getCurrentMousePosition()
                                                     : lastScreenPos);
    }

    //==============================================================================
    void sendMouseEnter (Component* const comp, const Point<int>& screenPos, const Time& time)
    {
        //DBG ("Mouse " + String (source.getIndex()) + " enter: " + comp->getLocalPoint (0, screenPos).toString() + " - Comp: " + String::toHexString ((int) comp));
        comp->internalMouseEnter (source, comp->getLocalPoint (0, screenPos), time);
    }

    void sendMouseExit (Component* const comp, const Point<int>& screenPos, const Time& time)
    {
        //DBG ("Mouse " + String (source.getIndex()) + " exit: " + comp->getLocalPoint (0, screenPos).toString() + " - Comp: " + String::toHexString ((int) comp));
        comp->internalMouseExit (source, comp->getLocalPoint (0, screenPos), time);
    }

    void sendMouseMove (Component* const comp, const Point<int>& screenPos, const Time& time)
    {
        //DBG ("Mouse " + String (source.getIndex()) + " move: " + comp->getLocalPoint (0, screenPos).toString() + " - Comp: " + String::toHexString ((int) comp));
        comp->internalMouseMove (source, comp->getLocalPoint (0, screenPos), time);
    }

    void sendMouseDown (Component* const comp, const Point<int>& screenPos, const Time& time)
    {
        //DBG ("Mouse " + String (source.getIndex()) + " down: " + comp->getLocalPoint (0, screenPos).toString() + " - Comp: " + String::toHexString ((int) comp));
        comp->internalMouseDown (source, comp->getLocalPoint (0, screenPos), time);
    }

    void sendMouseDrag (Component* const comp, const Point<int>& screenPos, const Time& time)
    {
        //DBG ("Mouse " + String (source.getIndex()) + " drag: " + comp->getLocalPoint (0, screenPos).toString() + " - Comp: " + String::toHexString ((int) comp));
        comp->internalMouseDrag (source, comp->getLocalPoint (0, screenPos), time);
    }

    void sendMouseUp (Component* const comp, const Point<int>& screenPos, const Time& time)
    {
        //DBG ("Mouse " + String (source.getIndex()) + " up: " + comp->getLocalPoint (0, screenPos).toString() + " - Comp: " + String::toHexString ((int) comp));
        comp->internalMouseUp (source, comp->getLocalPoint (0, screenPos), time, getCurrentModifiers());
    }

    void sendMouseWheel (Component* const comp, const Point<int>& screenPos, const Time& time, float x, float y)
    {
        //DBG ("Mouse " + String (source.getIndex()) + " wheel: " + comp->getLocalPoint (0, screenPos).toString() + " - Comp: " + String::toHexString ((int) comp));
        comp->internalMouseWheel (source, comp->getLocalPoint (0, screenPos), time, x, y);
    }

    //==============================================================================
    // (returns true if the button change caused a modal event loop)
    bool setButtons (const Point<int>& screenPos, const Time& time, const ModifierKeys& newButtonState)
    {
        if (buttonState == newButtonState)
            return false;

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
            Component* const current = getComponentUnderMouse();

            if (current != 0)
                sendMouseUp (current, screenPos + unboundedMouseOffset, time);

            enableUnboundedMouseMovement (false, false);
        }

        buttonState = newButtonState;

        if (buttonState.isAnyMouseButtonDown())
        {
            Desktop::getInstance().incrementMouseClickCounter();

            Component* const current = getComponentUnderMouse();

            if (current != 0)
            {
                registerMouseDown (screenPos, time, current, buttonState);
                sendMouseDown (current, screenPos, time);
            }
        }

        return lastCounter != mouseEventCounter;
    }

    void setComponentUnderMouse (Component* const newComponent, const Point<int>& screenPos, const Time& time)
    {
        Component* current = getComponentUnderMouse();

        if (newComponent != current)
        {
            WeakReference<Component> safeNewComp (newComponent);
            const ModifierKeys originalButtonState (buttonState);

            if (current != 0)
            {
                setButtons (screenPos, time, ModifierKeys());
                sendMouseExit (current, screenPos, time);
                buttonState = originalButtonState;
            }

            componentUnderMouse = safeNewComp;
            current = getComponentUnderMouse();

            if (current != 0)
                sendMouseEnter (current, screenPos, time);

            revealCursor (false);
            setButtons (screenPos, time, originalButtonState);
        }
    }

    void setPeer (ComponentPeer* const newPeer, const Point<int>& screenPos, const Time& time)
    {
        ModifierKeys::updateCurrentModifiers();

        if (newPeer != lastPeer)
        {
            setComponentUnderMouse (0, screenPos, time);
            lastPeer = newPeer;
            setComponentUnderMouse (findComponentAt (screenPos), screenPos, time);
        }
    }

    void setScreenPos (const Point<int>& newScreenPos, const Time& time, const bool forceUpdate)
    {
        if (! isDragging())
            setComponentUnderMouse (findComponentAt (newScreenPos), newScreenPos, time);

        if (newScreenPos != lastScreenPos || forceUpdate)
        {
            cancelPendingUpdate();

            lastScreenPos = newScreenPos;
            Component* const current = getComponentUnderMouse();

            if (current != 0)
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
    void handleEvent (ComponentPeer* const newPeer, const Point<int>& positionWithinPeer, const Time& time, const ModifierKeys& newMods)
    {
        jassert (newPeer != 0);
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

            ComponentPeer* peer = getPeer();
            if (peer != 0)
            {
                if (setButtons (screenPos, time, newMods))
                    return; // some modal events have been dispatched, so the current event is now out-of-date

                peer = getPeer();
                if (peer != 0)
                    setScreenPos (screenPos, time, false);
            }
        }
    }

    void handleWheel (ComponentPeer* const peer, const Point<int>& positionWithinPeer, const Time& time, float x, float y)
    {
        jassert (peer != 0);
        lastTime = time;
        ++mouseEventCounter;
        const Point<int> screenPos (peer->localToGlobal (positionWithinPeer));

        setPeer (peer, screenPos, time);
        setScreenPos (screenPos, time, false);
        triggerFakeMove();

        if (! isDragging())
        {
            Component* current = getComponentUnderMouse();
            if (current != 0)
                sendMouseWheel (current, screenPos, time, x, y);
        }
    }

    //==============================================================================
    const Time getLastMouseDownTime() const throw()
    {
        return Time (mouseDowns[0].time);
    }

    const Point<int> getLastMouseDownPosition() const throw()
    {
        return mouseDowns[0].position;
    }

    int getNumberOfMultipleClicks() const throw()
    {
        int numClicks = 0;

        if (mouseDowns[0].time != Time())
        {
            if (! mouseMovedSignificantlySincePressed)
                ++numClicks;

            for (int i = 1; i < numElementsInArray (mouseDowns); ++i)
            {
                if (mouseDowns[0].canBePartOfMultipleClickWith (mouseDowns[i], (int) (MouseEvent::getDoubleClickTimeout() * (1.0 + 0.25 * (i - 1)))))
                    ++numClicks;
                else
                    break;
            }
        }

        return numClicks;
    }

    bool hasMouseMovedSignificantlySincePressed() const throw()
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
                Component* current = getComponentUnderMouse();
                if (current != 0)
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

        Component* current = getComponentUnderMouse();
        if (current != 0)
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
        RecentMouseDown()  : component (0)
        {
        }

        Point<int> position;
        Time time;
        Component* component;
        ModifierKeys buttons;

        bool canBePartOfMultipleClickWith (const RecentMouseDown& other, const int maxTimeBetweenMs) const
        {
            return time - other.time < RelativeTime::milliseconds (maxTimeBetweenMs)
                    && abs (position.getX() - other.position.getX()) < 8
                    && abs (position.getY() - other.position.getY()) < 8
                    && buttons == other.buttons;;
        }
    };

    RecentMouseDown mouseDowns[4];
    bool mouseMovedSignificantlySincePressed;
    Time lastTime;

    void registerMouseDown (const Point<int>& screenPos, const Time& time,
                            Component* const component, const ModifierKeys& modifiers) throw()
    {
        for (int i = numElementsInArray (mouseDowns); --i > 0;)
            mouseDowns[i] = mouseDowns[i - 1];

        mouseDowns[0].position = screenPos;
        mouseDowns[0].time = time;
        mouseDowns[0].component = component;
        mouseDowns[0].buttons = modifiers.withOnlyMouseButtons();
        mouseMovedSignificantlySincePressed = false;
    }

    void registerMouseDrag (const Point<int>& screenPos) throw()
    {
        mouseMovedSignificantlySincePressed = mouseMovedSignificantlySincePressed
               || mouseDowns[0].position.getDistanceFrom (screenPos) >= 4;
    }

    JUCE_DECLARE_NON_COPYABLE (MouseInputSourceInternal);
};

//==============================================================================
MouseInputSource::MouseInputSource (const int index, const bool isMouseDevice)
{
    pimpl = new MouseInputSourceInternal (*this, index, isMouseDevice);
}

MouseInputSource::~MouseInputSource()
{
}

bool MouseInputSource::isMouse() const                                  { return pimpl->isMouseDevice; }
bool MouseInputSource::isTouch() const                                  { return ! isMouse(); }
bool MouseInputSource::canHover() const                                 { return isMouse(); }
bool MouseInputSource::hasMouseWheel() const                            { return isMouse(); }
int MouseInputSource::getIndex() const                                  { return pimpl->index; }
bool MouseInputSource::isDragging() const                               { return pimpl->isDragging(); }
const Point<int> MouseInputSource::getScreenPosition() const            { return pimpl->getScreenPosition(); }
const ModifierKeys MouseInputSource::getCurrentModifiers() const        { return pimpl->getCurrentModifiers(); }
Component* MouseInputSource::getComponentUnderMouse() const             { return pimpl->getComponentUnderMouse(); }
void MouseInputSource::triggerFakeMove() const                          { pimpl->triggerFakeMove(); }
int MouseInputSource::getNumberOfMultipleClicks() const throw()         { return pimpl->getNumberOfMultipleClicks(); }
const Time MouseInputSource::getLastMouseDownTime() const throw()       { return pimpl->getLastMouseDownTime(); }
const Point<int> MouseInputSource::getLastMouseDownPosition() const throw()     { return pimpl->getLastMouseDownPosition(); }
bool MouseInputSource::hasMouseMovedSignificantlySincePressed() const throw()   { return pimpl->hasMouseMovedSignificantlySincePressed(); }
bool MouseInputSource::canDoUnboundedMovement() const throw()           { return isMouse(); }
void MouseInputSource::enableUnboundedMouseMovement (bool isEnabled, bool keepCursorVisibleUntilOffscreen)    { pimpl->enableUnboundedMouseMovement (isEnabled, keepCursorVisibleUntilOffscreen); }
bool MouseInputSource::hasMouseCursor() const throw()                   { return isMouse(); }
void MouseInputSource::showMouseCursor (const MouseCursor& cursor)      { pimpl->showMouseCursor (cursor, false); }
void MouseInputSource::hideCursor()                                     { pimpl->hideCursor(); }
void MouseInputSource::revealCursor()                                   { pimpl->revealCursor (false); }
void MouseInputSource::forceMouseCursorUpdate()                         { pimpl->revealCursor (true); }

void MouseInputSource::handleEvent (ComponentPeer* peer, const Point<int>& positionWithinPeer, const int64 time, const ModifierKeys& mods)
{
    pimpl->handleEvent (peer, positionWithinPeer, Time (time), mods.withOnlyMouseButtons());
}

void MouseInputSource::handleWheel (ComponentPeer* const peer, const Point<int>& positionWithinPeer, const int64 time, const float x, const float y)
{
    pimpl->handleWheel (peer, positionWithinPeer, Time (time), x, y);
}


END_JUCE_NAMESPACE
