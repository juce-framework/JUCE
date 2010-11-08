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

#include "juce_MouseEvent.h"
#include "../juce_Component.h"


//==============================================================================
MouseEvent::MouseEvent (MouseInputSource& source_,
                        const Point<int>& position,
                        const ModifierKeys& mods_,
                        Component* const eventComponent_,
                        Component* const originator,
                        const Time& eventTime_,
                        const Point<int> mouseDownPos_,
                        const Time& mouseDownTime_,
                        const int numberOfClicks_,
                        const bool mouseWasDragged) throw()
    : x (position.getX()),
      y (position.getY()),
      mods (mods_),
      eventComponent (eventComponent_),
      originalComponent (originator),
      eventTime (eventTime_),
      source (source_),
      mouseDownPos (mouseDownPos_),
      mouseDownTime (mouseDownTime_),
      numberOfClicks (numberOfClicks_),
      wasMovedSinceMouseDown (mouseWasDragged)
{
}

MouseEvent::~MouseEvent() throw()
{
}

//==============================================================================
const MouseEvent MouseEvent::getEventRelativeTo (Component* const otherComponent) const throw()
{
    if (otherComponent == 0)
    {
        jassertfalse;
        return *this;
    }

    return MouseEvent (source, otherComponent->getLocalPoint (eventComponent, getPosition()),
                       mods, otherComponent, originalComponent, eventTime,
                       otherComponent->getLocalPoint (eventComponent, mouseDownPos),
                       mouseDownTime, numberOfClicks, wasMovedSinceMouseDown);
}

const MouseEvent MouseEvent::withNewPosition (const Point<int>& newPosition) const throw()
{
    return MouseEvent (source, newPosition, mods, eventComponent, originalComponent,
                       eventTime, mouseDownPos, mouseDownTime,
                       numberOfClicks, wasMovedSinceMouseDown);
}

//==============================================================================
bool MouseEvent::mouseWasClicked() const throw()
{
    return ! wasMovedSinceMouseDown;
}

int MouseEvent::getMouseDownX() const throw()
{
    return mouseDownPos.getX();
}

int MouseEvent::getMouseDownY() const throw()
{
    return mouseDownPos.getY();
}

const Point<int> MouseEvent::getMouseDownPosition() const throw()
{
    return mouseDownPos;
}

int MouseEvent::getDistanceFromDragStartX() const throw()
{
    return x - mouseDownPos.getX();
}

int MouseEvent::getDistanceFromDragStartY() const throw()
{
    return y - mouseDownPos.getY();
}

int MouseEvent::getDistanceFromDragStart() const throw()
{
    return mouseDownPos.getDistanceFrom (getPosition());
}

const Point<int> MouseEvent::getOffsetFromDragStart() const throw()
{
    return getPosition() - mouseDownPos;
}

int MouseEvent::getLengthOfMousePress() const throw()
{
    if (mouseDownTime.toMilliseconds() > 0)
        return jmax (0, (int) (eventTime - mouseDownTime).inMilliseconds());

    return 0;
}

//==============================================================================
const Point<int> MouseEvent::getPosition() const throw()
{
    return Point<int> (x, y);
}

int MouseEvent::getScreenX() const
{
    return getScreenPosition().getX();
}

int MouseEvent::getScreenY() const
{
    return getScreenPosition().getY();
}

const Point<int> MouseEvent::getScreenPosition() const
{
    return eventComponent->localPointToGlobal (Point<int> (x, y));
}

int MouseEvent::getMouseDownScreenX() const
{
    return getMouseDownScreenPosition().getX();
}

int MouseEvent::getMouseDownScreenY() const
{
    return getMouseDownScreenPosition().getY();
}

const Point<int> MouseEvent::getMouseDownScreenPosition() const
{
    return eventComponent->localPointToGlobal (mouseDownPos);
}

//==============================================================================
int MouseEvent::doubleClickTimeOutMs = 400;

void MouseEvent::setDoubleClickTimeout (const int newTime) throw()
{
    doubleClickTimeOutMs = newTime;
}

int MouseEvent::getDoubleClickTimeout() throw()
{
    return doubleClickTimeOutMs;
}

END_JUCE_NAMESPACE
