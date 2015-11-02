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

MouseEvent::MouseEvent (MouseInputSource inputSource,
                        Point<float> pos,
                        ModifierKeys modKeys,
                        float force,
                        Component* const eventComp,
                        Component* const originator,
                        Time time,
                        Point<float> downPos,
                        Time downTime,
                        const int numClicks,
                        const bool mouseWasDragged) noexcept
    : position (pos),
      x (roundToInt (pos.x)),
      y (roundToInt (pos.y)),
      mods (modKeys),
      pressure (force),
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

    return MouseEvent (source, otherComponent->getLocalPoint (eventComponent, position),
                       mods, pressure, otherComponent, originalComponent, eventTime,
                       otherComponent->getLocalPoint (eventComponent, mouseDownPos),
                       mouseDownTime, numberOfClicks, wasMovedSinceMouseDown != 0);
}

MouseEvent MouseEvent::withNewPosition (Point<float> newPosition) const noexcept
{
    return MouseEvent (source, newPosition, mods, pressure, eventComponent,
                       originalComponent, eventTime, mouseDownPos, mouseDownTime,
                       numberOfClicks, wasMovedSinceMouseDown != 0);
}

MouseEvent MouseEvent::withNewPosition (Point<int> newPosition) const noexcept
{
    return MouseEvent (source, newPosition.toFloat(), mods, pressure, eventComponent,
                       originalComponent, eventTime, mouseDownPos, mouseDownTime,
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

Point<int> MouseEvent::getMouseDownPosition() const noexcept    { return mouseDownPos.roundToInt(); }
Point<int> MouseEvent::getMouseDownScreenPosition() const       { return eventComponent->localPointToGlobal (mouseDownPos).roundToInt(); }

Point<int> MouseEvent::getOffsetFromDragStart() const noexcept  { return (position - mouseDownPos).roundToInt(); }
int MouseEvent::getDistanceFromDragStart() const noexcept       { return roundToInt (mouseDownPos.getDistanceFrom (position)); }

int MouseEvent::getMouseDownX() const noexcept                  { return roundToInt (mouseDownPos.x); }
int MouseEvent::getMouseDownY() const noexcept                  { return roundToInt (mouseDownPos.y); }

int MouseEvent::getDistanceFromDragStartX() const noexcept      { return getOffsetFromDragStart().x; }
int MouseEvent::getDistanceFromDragStartY() const noexcept      { return getOffsetFromDragStart().y; }

int MouseEvent::getScreenX() const                              { return getScreenPosition().x; }
int MouseEvent::getScreenY() const                              { return getScreenPosition().y; }

int MouseEvent::getMouseDownScreenX() const                     { return getMouseDownScreenPosition().x; }
int MouseEvent::getMouseDownScreenY() const                     { return getMouseDownScreenPosition().y; }

bool MouseEvent::isPressureValid() const noexcept               { return pressure > 0.0f && pressure < 1.0f; }

//==============================================================================
static int doubleClickTimeOutMs = 400;

int MouseEvent::getDoubleClickTimeout() noexcept                        { return doubleClickTimeOutMs; }
void MouseEvent::setDoubleClickTimeout (const int newTime) noexcept     { doubleClickTimeOutMs = newTime; }
