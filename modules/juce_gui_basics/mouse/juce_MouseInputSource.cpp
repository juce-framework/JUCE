/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

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
