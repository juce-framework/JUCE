/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

MouseEvent::MouseEvent (MouseInputSource inputSource,
                        Point<float> pos,
                        ModifierKeys modKeys,
                        float force,
                        float o, float r,
                        float tX, float tY,
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
      orientation (o), rotation (r),
      tiltX (tX), tiltY (tY),
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
                       mods, pressure, orientation, rotation, tiltX, tiltY,
                       otherComponent, originalComponent, eventTime,
                       otherComponent->getLocalPoint (eventComponent, mouseDownPos),
                       mouseDownTime, numberOfClicks, wasMovedSinceMouseDown != 0);
}

MouseEvent MouseEvent::withNewPosition (Point<float> newPosition) const noexcept
{
    return MouseEvent (source, newPosition, mods, pressure, orientation, rotation, tiltX, tiltY,
                       eventComponent, originalComponent, eventTime, mouseDownPos, mouseDownTime,
                       numberOfClicks, wasMovedSinceMouseDown != 0);
}

MouseEvent MouseEvent::withNewPosition (Point<int> newPosition) const noexcept
{
    return MouseEvent (source, newPosition.toFloat(), mods, pressure, orientation, rotation,
                       tiltX, tiltY, eventComponent,  originalComponent, eventTime, mouseDownPos,
                       mouseDownTime, numberOfClicks, wasMovedSinceMouseDown != 0);
}

//==============================================================================
bool MouseEvent::mouseWasDraggedSinceMouseDown() const noexcept
{
    return wasMovedSinceMouseDown != 0;
}

bool MouseEvent::mouseWasClicked() const noexcept
{
    return ! mouseWasDraggedSinceMouseDown();
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
bool MouseEvent::isOrientationValid() const noexcept            { return orientation >= 0.0f && orientation <= 2.0f * float_Pi; }
bool MouseEvent::isRotationValid() const noexcept               { return rotation >= 0 && rotation <= 2.0f * float_Pi; }
bool MouseEvent::isTiltValid (bool isX) const noexcept          { return isX ? (tiltX >= -1.0f && tiltX <= 1.0f) : (tiltY >= -1.0f && tiltY <= 1.0f); }

//==============================================================================
static int doubleClickTimeOutMs = 400;

int MouseEvent::getDoubleClickTimeout() noexcept                        { return doubleClickTimeOutMs; }
void MouseEvent::setDoubleClickTimeout (const int newTime) noexcept     { doubleClickTimeOutMs = newTime; }

} // namespace juce
