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
      mouseDownPosition (downPos),
      eventComponent (eventComp),
      originalComponent (originator),
      eventTime (time),
      mouseDownTime (downTime),
      source (inputSource),
      numberOfClicks ((uint8) numClicks),
      wasMovedSinceMouseDown ((uint8) (mouseWasDragged ? 1 : 0))
{
}

//==============================================================================
MouseEvent MouseEvent::getEventRelativeTo (Component* const otherComponent) const noexcept
{
    jassert (otherComponent != nullptr);

    return MouseEvent (source, otherComponent->getLocalPoint (eventComponent, position),
                       mods, pressure, orientation, rotation, tiltX, tiltY,
                       otherComponent, originalComponent, eventTime,
                       otherComponent->getLocalPoint (eventComponent, mouseDownPosition),
                       mouseDownTime, numberOfClicks, wasMovedSinceMouseDown != 0);
}

MouseEvent MouseEvent::withNewPosition (Point<float> newPosition) const noexcept
{
    return MouseEvent (source, newPosition, mods, pressure, orientation, rotation, tiltX, tiltY,
                       eventComponent, originalComponent, eventTime, mouseDownPosition, mouseDownTime,
                       numberOfClicks, wasMovedSinceMouseDown != 0);
}

MouseEvent MouseEvent::withNewPosition (Point<int> newPosition) const noexcept
{
    return MouseEvent (source, newPosition.toFloat(), mods, pressure, orientation, rotation,
                       tiltX, tiltY, eventComponent,  originalComponent, eventTime, mouseDownPosition,
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

Point<int> MouseEvent::getMouseDownPosition() const noexcept    { return mouseDownPosition.roundToInt(); }
Point<int> MouseEvent::getMouseDownScreenPosition() const       { return eventComponent->localPointToGlobal (mouseDownPosition).roundToInt(); }

Point<int> MouseEvent::getOffsetFromDragStart() const noexcept  { return (position - mouseDownPosition).roundToInt(); }
int MouseEvent::getDistanceFromDragStart() const noexcept       { return roundToInt (mouseDownPosition.getDistanceFrom (position)); }

int MouseEvent::getMouseDownX() const noexcept                  { return roundToInt (mouseDownPosition.x); }
int MouseEvent::getMouseDownY() const noexcept                  { return roundToInt (mouseDownPosition.y); }

int MouseEvent::getDistanceFromDragStartX() const noexcept      { return getOffsetFromDragStart().x; }
int MouseEvent::getDistanceFromDragStartY() const noexcept      { return getOffsetFromDragStart().y; }

int MouseEvent::getScreenX() const                              { return getScreenPosition().x; }
int MouseEvent::getScreenY() const                              { return getScreenPosition().y; }

int MouseEvent::getMouseDownScreenX() const                     { return getMouseDownScreenPosition().x; }
int MouseEvent::getMouseDownScreenY() const                     { return getMouseDownScreenPosition().y; }

bool MouseEvent::isPressureValid() const noexcept               { return pressure > 0.0f && pressure < 1.0f; }
bool MouseEvent::isOrientationValid() const noexcept            { return orientation >= 0.0f && orientation <= MathConstants<float>::twoPi; }
bool MouseEvent::isRotationValid() const noexcept               { return rotation >= 0 && rotation <= MathConstants<float>::twoPi; }
bool MouseEvent::isTiltValid (bool isX) const noexcept          { return isX ? (tiltX >= -1.0f && tiltX <= 1.0f) : (tiltY >= -1.0f && tiltY <= 1.0f); }

//==============================================================================
static int doubleClickTimeOutMs = 400;

int MouseEvent::getDoubleClickTimeout() noexcept                        { return doubleClickTimeOutMs; }
void MouseEvent::setDoubleClickTimeout (const int newTime) noexcept     { doubleClickTimeOutMs = newTime; }

} // namespace juce
