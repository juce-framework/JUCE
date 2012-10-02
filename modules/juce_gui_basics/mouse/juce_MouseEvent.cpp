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

MouseEvent::MouseEvent (MouseInputSource& inputSource,
                        const Point<int>& position,
                        const ModifierKeys& modKeys,
                        Component* const eventComp,
                        Component* const originator,
                        const Time& time,
                        const Point<int>& downPos,
                        const Time& downTime,
                        const int numClicks,
                        const bool mouseWasDragged) noexcept
    : x (position.x),
      y (position.y),
      mods (modKeys),
      eventComponent (eventComp),
      originalComponent (originator),
      eventTime (time),
      mouseDownTime (downTime),
      source (inputSource),
      mouseDownPos (downPos),
      numberOfClicks ((uint8) numClicks),
      wasMovedSinceMouseDown ((uint8) (mouseWasDragged ? 1 : 0))
{
}

MouseEvent::~MouseEvent() noexcept
{
}

//==============================================================================
MouseEvent MouseEvent::getEventRelativeTo (Component* const otherComponent) const noexcept
{
    jassert (otherComponent != nullptr);

    return MouseEvent (source, otherComponent->getLocalPoint (eventComponent, getPosition()),
                       mods, otherComponent, originalComponent, eventTime,
                       otherComponent->getLocalPoint (eventComponent, mouseDownPos),
                       mouseDownTime, numberOfClicks, wasMovedSinceMouseDown != 0);
}

MouseEvent MouseEvent::withNewPosition (const Point<int>& newPosition) const noexcept
{
    return MouseEvent (source, newPosition, mods, eventComponent, originalComponent,
                       eventTime, mouseDownPos, mouseDownTime,
                       numberOfClicks, wasMovedSinceMouseDown != 0);
}

//==============================================================================
bool MouseEvent::mouseWasClicked() const noexcept
{
    return wasMovedSinceMouseDown == 0;
}

int MouseEvent::getLengthOfMousePress() const noexcept
{
    if (mouseDownTime.toMilliseconds() > 0)
        return jmax (0, (int) (eventTime - mouseDownTime).inMilliseconds());

    return 0;
}

//==============================================================================
Point<int> MouseEvent::getPosition() const noexcept             { return Point<int> (x, y); }
Point<int> MouseEvent::getScreenPosition() const                { return eventComponent->localPointToGlobal (getPosition()); }

Point<int> MouseEvent::getMouseDownPosition() const noexcept    { return mouseDownPos; }
Point<int> MouseEvent::getMouseDownScreenPosition() const       { return eventComponent->localPointToGlobal (mouseDownPos); }

Point<int> MouseEvent::getOffsetFromDragStart() const noexcept  { return getPosition() - mouseDownPos; }
int MouseEvent::getDistanceFromDragStart() const noexcept       { return mouseDownPos.getDistanceFrom (getPosition()); }

int MouseEvent::getMouseDownX() const noexcept                  { return mouseDownPos.x; }
int MouseEvent::getMouseDownY() const noexcept                  { return mouseDownPos.y; }

int MouseEvent::getDistanceFromDragStartX() const noexcept      { return x - mouseDownPos.x; }
int MouseEvent::getDistanceFromDragStartY() const noexcept      { return y - mouseDownPos.y; }

int MouseEvent::getScreenX() const                              { return getScreenPosition().x; }
int MouseEvent::getScreenY() const                              { return getScreenPosition().y; }

int MouseEvent::getMouseDownScreenX() const                     { return getMouseDownScreenPosition().x; }
int MouseEvent::getMouseDownScreenY() const                     { return getMouseDownScreenPosition().y; }

//==============================================================================
static int doubleClickTimeOutMs = 400;

int MouseEvent::getDoubleClickTimeout() noexcept                        { return doubleClickTimeOutMs; }
void MouseEvent::setDoubleClickTimeout (const int newTime) noexcept     { doubleClickTimeOutMs = newTime; }
