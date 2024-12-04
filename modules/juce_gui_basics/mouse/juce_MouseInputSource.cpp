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

//==============================================================================
MouseInputSource::MouseInputSource (detail::MouseInputSourceImpl* s) noexcept   : pimpl (s)  {}
MouseInputSource::MouseInputSource (const MouseInputSource& other) noexcept : pimpl (other.pimpl)  {}
MouseInputSource::~MouseInputSource() noexcept {}

MouseInputSource& MouseInputSource::operator= (const MouseInputSource& other) noexcept
{
    pimpl = other.pimpl;
    return *this;
}

MouseInputSource::InputSourceType MouseInputSource::getType() const noexcept    { return pimpl->inputType; }
bool MouseInputSource::isMouse() const noexcept                                 { return (getType() == MouseInputSource::InputSourceType::mouse); }
bool MouseInputSource::isTouch() const noexcept                                 { return (getType() == MouseInputSource::InputSourceType::touch); }
bool MouseInputSource::isPen() const noexcept                                   { return (getType() == MouseInputSource::InputSourceType::pen); }
bool MouseInputSource::canHover() const noexcept                                { return ! isTouch(); }
bool MouseInputSource::hasMouseWheel() const noexcept                           { return ! isTouch(); }
int MouseInputSource::getIndex() const noexcept                                 { return pimpl->index; }
bool MouseInputSource::isDragging() const noexcept                              { return pimpl->isDragging(); }
Point<float> MouseInputSource::getScreenPosition() const noexcept               { return pimpl->getScreenPosition(); }
Point<float> MouseInputSource::getRawScreenPosition() const noexcept            { return pimpl->getRawScreenPosition();  }
ModifierKeys MouseInputSource::getCurrentModifiers() const noexcept             { return pimpl->getCurrentModifiers(); }
float MouseInputSource::getCurrentPressure() const noexcept                     { return pimpl->lastPointerState.pressure; }
bool MouseInputSource::isPressureValid() const noexcept                         { return pimpl->lastPointerState.isPressureValid(); }
float MouseInputSource::getCurrentOrientation() const noexcept                  { return pimpl->lastPointerState.orientation; }
bool MouseInputSource::isOrientationValid() const noexcept                      { return pimpl->lastPointerState.isOrientationValid(); }
float MouseInputSource::getCurrentRotation() const noexcept                     { return pimpl->lastPointerState.rotation; }
bool MouseInputSource::isRotationValid() const noexcept                         { return pimpl->lastPointerState.isRotationValid(); }
float MouseInputSource::getCurrentTilt (bool tiltX) const noexcept              { return tiltX ? pimpl->lastPointerState.tiltX : pimpl->lastPointerState.tiltY; }
bool MouseInputSource::isTiltValid (bool isX) const noexcept                    { return pimpl->lastPointerState.isTiltValid (isX); }
Component* MouseInputSource::getComponentUnderMouse() const                     { return pimpl->getComponentUnderMouse(); }
void MouseInputSource::triggerFakeMove() const                                  { pimpl->triggerFakeMove(); }
int MouseInputSource::getNumberOfMultipleClicks() const noexcept                { return pimpl->getNumberOfMultipleClicks(); }
Time MouseInputSource::getLastMouseDownTime() const noexcept                    { return pimpl->getLastMouseDownTime(); }
Point<float> MouseInputSource::getLastMouseDownPosition() const noexcept        { return pimpl->getLastMouseDownPosition(); }
bool MouseInputSource::isLongPressOrDrag() const noexcept                       { return pimpl->isLongPressOrDrag(); }
bool MouseInputSource::hasMovedSignificantlySincePressed() const noexcept       { return pimpl->hasMovedSignificantlySincePressed(); }
bool MouseInputSource::canDoUnboundedMovement() const noexcept                  { return ! isTouch(); }
void MouseInputSource::enableUnboundedMouseMovement (bool isEnabled, bool keepCursorVisibleUntilOffscreen) const
                                                                         { pimpl->enableUnboundedMouseMovement (isEnabled, keepCursorVisibleUntilOffscreen); }
bool MouseInputSource::isUnboundedMouseMovementEnabled() const           { return pimpl->isUnboundedMouseModeOn; }
bool MouseInputSource::hasMouseCursor() const noexcept                   { return ! isTouch(); }
void MouseInputSource::showMouseCursor (const MouseCursor& cursor)       { pimpl->showMouseCursor (cursor, false); }
void MouseInputSource::hideCursor()                                      { pimpl->hideCursor(); }
void MouseInputSource::revealCursor()                                    { pimpl->revealCursor (false); }
void MouseInputSource::forceMouseCursorUpdate()                          { pimpl->revealCursor (true); }
void MouseInputSource::setScreenPosition (Point<float> p)                { pimpl->setScreenPosition (p); }

void MouseInputSource::handleEvent (ComponentPeer& peer, Point<float> pos, int64 time, ModifierKeys mods,
                                    float pressure, float orientation, const PenDetails& penDetails)
{
    pimpl->handleEvent (peer, pos, Time (time), mods.withOnlyMouseButtons(), pressure, orientation, penDetails);
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
const float MouseInputSource::invalidOrientation = 0.0f;
const float MouseInputSource::invalidRotation = 0.0f;

const float MouseInputSource::invalidTiltX = 0.0f;
const float MouseInputSource::invalidTiltY = 0.0f;

const Point<float> MouseInputSource::offscreenMousePos { -10.0f, -10.0f };

// Deprecated method
bool MouseInputSource::hasMouseMovedSignificantlySincePressed() const noexcept  { return pimpl->hasMouseMovedSignificantlySincePressed(); }

} // namespace juce
