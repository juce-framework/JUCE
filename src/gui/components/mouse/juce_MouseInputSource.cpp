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

#include "../../../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_MouseInputSource.h"
#include "juce_MouseEvent.h"
#include "../juce_Component.h"
#include "../juce_ComponentDeletionWatcher.h"
#include "../../../events/juce_AsyncUpdater.h"


//==============================================================================
class MouseInputSourceInternal   : public AsyncUpdater
{
public:
    MouseInputSourceInternal (MouseInputSource& source_, const int index_, const bool isMouseDevice_)
        : index (index_), isMouseDevice (isMouseDevice_), source (source_), lastPeer (0), lastTime (0)
    {
        zerostruct (mouseDowns);
    }

    ~MouseInputSourceInternal()
    {
    }

    bool isDragging() const throw()
    {
        return buttonState.isAnyMouseButtonDown();
    }

    Component* getComponentUnderMouse() const
    {
        return componentUnderMouse != 0 ? const_cast<Component*> (componentUnderMouse->getComponent()) : 0;
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
            const Point<int> relativePos (comp->globalPositionToRelative (screenPos));

            // (the contains() call is needed to test for overlapping desktop windows)
            if (comp->contains (relativePos.getX(), relativePos.getY()))
                return comp->getComponentAt (relativePos);
        }

        return 0;
    }

    void setButtons (const Point<int>& screenPos, const int64 time, const ModifierKeys& newButtonState)
    {
        if (buttonState != newButtonState)
        {
            // (ignore secondary clicks when there's already a button down)
            if (buttonState.isAnyMouseButtonDown() == newButtonState.isAnyMouseButtonDown())
            {
                buttonState = newButtonState;
                return;
            }

            if (buttonState.isAnyMouseButtonDown())
            {
                Component* const current = getComponentUnderMouse();

                if (current != 0)
                    current->internalMouseUp (source, current->globalPositionToRelative (screenPos),
                                              time, getCurrentModifiers());
            }

            buttonState = newButtonState;

            if (buttonState.isAnyMouseButtonDown())
            {
                Desktop::getInstance().incrementMouseClickCounter();

                Component* const current = getComponentUnderMouse();

                if (current != 0)
                {
                    registerMouseDown (screenPos, time, current);

                    current->internalMouseDown (source, current->globalPositionToRelative (screenPos), time);
                }
            }
        }
    }

    void setComponentUnderMouse (Component* const newComponent, const Point<int>& screenPos, const int64 time)
    {
        Component* current = getComponentUnderMouse();

        if (newComponent != current)
        {
            ScopedPointer<ComponentDeletionWatcher> newCompWatcher (newComponent != 0 ? new ComponentDeletionWatcher (newComponent) : 0);
            const ModifierKeys originalButtonState (buttonState);

            if (current != 0)
            {
                setButtons (screenPos, time, ModifierKeys());
                current->internalMouseExit (source, current->globalPositionToRelative (screenPos), time);
                buttonState = originalButtonState;
            }

            componentUnderMouse = newCompWatcher;
            current = getComponentUnderMouse();
            Component::componentUnderMouse = current;

            if (current != 0)
                current->internalMouseEnter (source, current->globalPositionToRelative (screenPos), time);

            setButtons (screenPos, time, originalButtonState);
        }
    }

    void setPeer (ComponentPeer* const newPeer, const Point<int>& screenPos, const int64 time)
    {
        ModifierKeys::updateCurrentModifiers();

        if (newPeer != lastPeer)
        {
            setComponentUnderMouse (0, screenPos, time);
            lastPeer = newPeer;
            setComponentUnderMouse (findComponentAt (screenPos), screenPos, time);
        }
    }

    void setScreenPos (const Point<int>& newScreenPos, const int64 time, const bool forceUpdate)
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
                const Point<int> pos (current->globalPositionToRelative (lastScreenPos));

                if (isDragging())
                {
                    registerMouseDrag (newScreenPos);
                    current->internalMouseDrag (source, pos, time);
                }
                else
                {
                    current->internalMouseMove (source, pos, time);
                }
            }
        }
    }

    void handleEvent (ComponentPeer* const newPeer, const Point<int>& positionWithinPeer, const int64 time, const ModifierKeys& newMods)
    {
        jassert (newPeer != 0);
        lastTime = time;
        const Point<int> screenPos (newPeer->relativePositionToGlobal (positionWithinPeer));

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
                setButtons (screenPos, time, newMods);

                peer = getPeer();
                if (peer != 0)
                    setScreenPos (peer->relativePositionToGlobal (positionWithinPeer), time, false);
            }
        }
    }

    void handleWheel (ComponentPeer* const peer, const Point<int>& positionWithinPeer, int64 time, float x, float y)
    {
        jassert (peer != 0);
        lastTime = time;
        const Point<int> screenPos (peer->relativePositionToGlobal (positionWithinPeer));

        setPeer (peer, screenPos, time);
        setScreenPos (screenPos, time, false);
        triggerFakeMove();

        if (! isDragging())
        {
            Component* current = getComponentUnderMouse();
            if (current != 0)
                current->internalMouseWheel (source, current->globalPositionToRelative (screenPos), time, x, y);
        }
    }

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

        if (mouseDowns[0].time != 0)
        {
            if (! mouseMovedSignificantlySincePressed)
                ++numClicks;

            for (int i = 1; i < numElementsInArray (mouseDowns); ++i)
            {
                if (mouseDowns[0].time - mouseDowns[i].time < (int) (MouseEvent::getDoubleClickTimeout() * (1.0 + 0.25 * (i - 1)))
                    && abs (mouseDowns[0].position.getX() - mouseDowns[i].position.getX()) < 8
                    && abs (mouseDowns[0].position.getY() - mouseDowns[i].position.getY()) < 8
                    && mouseDowns[0].component == mouseDowns[i].component)
                {
                    ++numClicks;
                }
                else
                {
                    break;
                }
            }
        }

        return numClicks;
    }

    bool hasMouseMovedSignificantlySincePressed() const throw()
    {
        return mouseMovedSignificantlySincePressed
                || lastTime > mouseDowns[0].time + 300;
    }

    void triggerFakeMove()
    {
        triggerAsyncUpdate();
    }

    void handleAsyncUpdate()
    {
        setScreenPos (Desktop::getMousePosition(), Time::currentTimeMillis(), true);
    }

    int index;
    bool isMouseDevice;
    Point<int> lastScreenPos;
    ModifierKeys buttonState;

private:
    MouseInputSource& source;
    ScopedPointer<ComponentDeletionWatcher> componentUnderMouse;
    ComponentPeer* lastPeer;

    struct RecentMouseDown
    {
        Point<int> position;
        int64 time;
        Component* component;
    };

    RecentMouseDown mouseDowns[4];
    bool mouseMovedSignificantlySincePressed;
    int64 lastTime;

    void registerMouseDown (const Point<int>& screenPos, const int64 time, Component* const component) throw()
    {
        for (int i = numElementsInArray (mouseDowns); --i > 0;)
            mouseDowns[i] = mouseDowns[i - 1];

        mouseDowns[0].position = screenPos;
        mouseDowns[0].time = time;
        mouseDowns[0].component = component;
        mouseMovedSignificantlySincePressed = false;
    }

    void registerMouseDrag (const Point<int>& screenPos) throw()
    {
        mouseMovedSignificantlySincePressed = mouseMovedSignificantlySincePressed
               || mouseDowns[0].position.getDistanceFrom (screenPos) >= 4;
    }
};

//==============================================================================
MouseInputSource::MouseInputSource (const int index, const bool isMouseDevice)
{
    pimpl = new MouseInputSourceInternal (*this, index, isMouseDevice);
}

MouseInputSource::~MouseInputSource()
{
}

bool MouseInputSource::isMouse() const                              { return pimpl->isMouseDevice; }
bool MouseInputSource::isTouch() const                              { return ! pimpl->isMouseDevice; }
bool MouseInputSource::canHover() const                             { return pimpl->isMouseDevice; }
bool MouseInputSource::hasMouseWheel() const                        { return pimpl->isMouseDevice; }
int MouseInputSource::getIndex() const                              { return pimpl->index; }
bool MouseInputSource::isDragging() const                           { return pimpl->isDragging(); }
const Point<int> MouseInputSource::getScreenPosition() const        { return pimpl->lastScreenPos; }
const ModifierKeys MouseInputSource::getCurrentModifiers() const    { return pimpl->getCurrentModifiers(); }
Component* MouseInputSource::getComponentUnderMouse() const         { return pimpl->getComponentUnderMouse(); }
void MouseInputSource::triggerFakeMove() const                      { pimpl->triggerFakeMove(); }
int MouseInputSource::getNumberOfMultipleClicks() const throw()     { return pimpl->getNumberOfMultipleClicks(); }
const Time MouseInputSource::getLastMouseDownTime() const throw()   { return pimpl->getLastMouseDownTime(); }
const Point<int> MouseInputSource::getLastMouseDownPosition() const throw()     { return pimpl->getLastMouseDownPosition(); }
bool MouseInputSource::hasMouseMovedSignificantlySincePressed() const throw()   { return pimpl->hasMouseMovedSignificantlySincePressed(); }

void MouseInputSource::handleEvent (ComponentPeer* peer, const Point<int>& positionWithinPeer, const int64 time, const ModifierKeys& mods)
{
    pimpl->handleEvent (peer, positionWithinPeer, time, mods.withOnlyMouseButtons());
}

void MouseInputSource::handleWheel (ComponentPeer* const peer, const Point<int>& positionWithinPeer, const int64 time, const float x, const float y)
{
    pimpl->handleWheel (peer, positionWithinPeer, time, x, y);
}


END_JUCE_NAMESPACE
