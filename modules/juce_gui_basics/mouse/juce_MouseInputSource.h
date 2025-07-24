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
/**
    Represents a linear source of mouse events from a mouse device or individual finger
    in a multi-touch environment.

    Each MouseEvent object contains a reference to the MouseInputSource that generated
    it. In an environment with a single mouse for input, all events will come from the
    same source, but in a multi-touch system, there may be multiple MouseInputSource
    objects active, each representing a stream of events coming from a particular finger.

    Events coming from a single MouseInputSource are always sent in a fixed and predictable
    order: a mouseMove will never be called without a mouseEnter having been sent beforehand,
    the only events that can happen between a mouseDown and its corresponding mouseUp are
    mouseDrags, etc.
    When there are multiple touches arriving from multiple MouseInputSources, their
    event streams may arrive in an interleaved order, so you should use the getIndex()
    method to find out which finger each event came from.

    @see MouseEvent

    @tags{GUI}
*/
class JUCE_API  MouseInputSource  final
{
public:
    /** Possible mouse input sources. */
    enum InputSourceType
    {
        mouse,
        touch,
        pen
    };

    //==============================================================================
    MouseInputSource (const MouseInputSource&) noexcept;
    MouseInputSource& operator= (const MouseInputSource&) noexcept;
    ~MouseInputSource() noexcept;

    //==============================================================================
    bool operator== (const MouseInputSource& other) const noexcept     { return pimpl == other.pimpl; }
    bool operator!= (const MouseInputSource& other) const noexcept     { return pimpl != other.pimpl; }

    //==============================================================================
    /** Returns the type of input source that this object represents. */
    MouseInputSource::InputSourceType getType() const noexcept;

    /** Returns true if this object represents a normal desk-based mouse device. */
    bool isMouse() const noexcept;

    /** Returns true if this object represents a source of touch events. */
    bool isTouch() const noexcept;

    /** Returns true if this object represents a pen device. */
    bool isPen() const noexcept;

    /** Returns true if this source has an on-screen pointer that can hover over
        items without clicking them.
    */
    bool canHover() const noexcept;

    /** Returns true if this source may have a scroll wheel. */
    bool hasMouseWheel() const noexcept;

    /** Returns this source's index in the global list of possible sources.
        If the system only has a single mouse, there will only be a single MouseInputSource
        with an index of 0.

        If the system supports multi-touch input, then the index will represent a finger
        number, starting from 0. When the first touch event begins, it will have finger
        number 0, and then if a second touch happens while the first is still down, it
        will have index 1, etc.
    */
    int getIndex() const noexcept;

    /** Returns true if this device is currently being pressed. */
    bool isDragging() const noexcept;

    /** Returns the last-known screen position of this source. */
    Point<float> getScreenPosition() const noexcept;

    /** Returns the last-known screen position of this source without any scaling applied. */
    Point<float> getRawScreenPosition() const noexcept;

    /** Returns a set of modifiers that indicate which buttons are currently
        held down on this device.
    */
    ModifierKeys getCurrentModifiers() const noexcept;

    /** Returns the device's current touch or pen pressure.
        The range is 0 (soft) to 1 (hard).
        If the input device doesn't provide any pressure data, it may return a negative
        value here, or 0.0 or 1.0, depending on the platform.
    */
    float getCurrentPressure() const noexcept;

    /** Returns the device's current orientation in radians. 0 indicates a touch pointer
        aligned with the x-axis and pointing from left to right; increasing values indicate
        rotation in the clockwise direction. Only reported by a touch pointer.
    */
    float getCurrentOrientation() const noexcept;

    /** Returns the device's current rotation. Indicates the clockwise rotation, or twist, of the pointer
        in radians. The default is 0. Only reported by a pen pointer.
    */
    float getCurrentRotation() const noexcept;

    /** Returns the angle of tilt of the pointer in a range of -1.0 to 1.0 either in the x- or y-axis. The default is 0.
        If x-axis, a positive value indicates a tilt to the right and if y-axis, a positive value indicates a tilt toward the user.
        Only reported by a pen pointer.
    */
    float getCurrentTilt (bool tiltX) const noexcept;

    /** Returns true if the current pressure value is meaningful. */
    bool isPressureValid() const noexcept;

    /** Returns true if the current orientation value is meaningful. */
    bool isOrientationValid() const noexcept;

    /** Returns true if the current rotation value is meaningful. */
    bool isRotationValid() const noexcept;

    /** Returns true if the current tilt value (either x- or y-axis) is meaningful. */
    bool isTiltValid (bool tiltX) const noexcept;

    /** Returns the component that was last known to be under this pointer. */
    Component* getComponentUnderMouse() const;

    /** Tells the device to dispatch a mouse-move or mouse-drag event.
        This is asynchronous - the event will occur on the message thread.
    */
    void triggerFakeMove() const;

    /** Returns the number of clicks that should be counted as belonging to the
        current mouse event.
        So the mouse is currently down and it's the second click of a double-click, this
        will return 2.
    */
    int getNumberOfMultipleClicks() const noexcept;

    /** Returns the time at which the last mouse-down occurred. */
    Time getLastMouseDownTime() const noexcept;

    /** Returns the screen position at which the last mouse-down occurred. */
    Point<float> getLastMouseDownPosition() const noexcept;

    /** Returns true if this input source represents a long-press or drag interaction i.e. it has been held down for a significant
        amount of time or it has been dragged more than a couple of pixels from the place it was pressed. */
    bool isLongPressOrDrag() const noexcept;

    /** Returns true if this input source has been dragged more than a couple of pixels from the place it was pressed. */
    bool hasMovedSignificantlySincePressed() const noexcept;

    /** Returns true if this input source uses a visible mouse cursor. */
    bool hasMouseCursor() const noexcept;

    /** Changes the mouse cursor, (if there is one). */
    void showMouseCursor (const MouseCursor& cursor);

    /** Hides the mouse cursor (if there is one). */
    void hideCursor();

    /** Un-hides the mouse cursor if it was hidden by hideCursor(). */
    void revealCursor();

    /** Forces an update of the mouse cursor for whatever component it's currently over. */
    void forceMouseCursorUpdate();

    /** Returns true if this mouse can be moved indefinitely in any direction without running out of space. */
    bool canDoUnboundedMovement() const noexcept;

    /** Allows the mouse to move beyond the edges of the screen.

        Calling this method when the mouse button is currently pressed will remove the cursor
        from the screen and allow the mouse to (seem to) move beyond the edges of the screen.

        This means that the coordinates returned to mouseDrag() will be unbounded, and this
        can be used for things like custom slider controls or dragging objects around, where
        movement would be otherwise be limited by the mouse hitting the edges of the screen.

        The unbounded mode is automatically turned off when the mouse button is released, or
        it can be turned off explicitly by calling this method again.

        @param isEnabled                            whether to turn this mode on or off
        @param keepCursorVisibleUntilOffscreen      if set to false, the cursor will immediately be
                                                    hidden; if true, it will only be hidden when it
                                                    is moved beyond the edge of the screen
    */
    void enableUnboundedMouseMovement (bool isEnabled, bool keepCursorVisibleUntilOffscreen = false) const;

    /** Returns true if this source is currently in "unbounded" mode. */
    bool isUnboundedMouseMovementEnabled() const;

    /** Attempts to set this mouse pointer's screen position. */
    void setScreenPosition (Point<float> newPosition);

    /** A default value for pressure, which is used when a device doesn't support it, or for
        mouse-moves, mouse-ups, etc.
    */
    static constexpr float defaultPressure = 0.0f;

    /** A default value for orientation, which is used when a device doesn't support it */
    static constexpr float defaultOrientation = 0.0f;

    /** A default value for rotation, which is used when a device doesn't support it */
    static constexpr float defaultRotation = 0.0f;

    /** Default values for tilt, which are used when a device doesn't support it */
    static constexpr float defaultTiltX = 0.0f;
    static constexpr float defaultTiltY = 0.0f;

    /** A default value for pressure, which is used when a device doesn't support it.

        This is a valid value, returning true when calling isPressureValid() hence the
        deprecation. Use defaultPressure instead.
    */
    [[deprecated ("Use defaultPressure instead.")]]
    static const float invalidPressure;

    /** A default value for orientation, which is used when a device doesn't support it.

        This is a valid value, returning true when calling isOrientationValid() hence the
        deprecation. Use defaultOrientation instead.
    */
    [[deprecated ("Use defaultOrientation instead.")]]
    static const float invalidOrientation;

    /** A default value for rotation, which is used when a device doesn't support it.

        This is a valid value, returning true when calling isRotationValid() hence the
        deprecation. Use defaultRotation instead.
    */
    [[deprecated ("Use defaultRotation instead.")]]
    static const float invalidRotation;

    /** Default values for tilt, which are used when a device doesn't support it

        These are valid values, returning true when calling isTiltValid() hence the
        deprecation. Use defaultTiltX and defaultTiltY instead.
    */
    [[deprecated ("Use defaultTiltX instead.")]]
    static const float invalidTiltX;
    [[deprecated ("Use defaultTiltY instead.")]]
    static const float invalidTiltY;

    /** An offscreen mouse position used when triggering mouse exits where we don't want to move
        the cursor over an existing component.
    */
    static const Point<float> offscreenMousePos;

    //==============================================================================
    /** @cond */
    [[deprecated ("This method has been replaced with the isLongPressOrDrag and hasMovedSignificantlySincePressed "
                 "methods. If you want the same behaviour you should use isLongPressOrDrag which accounts for the "
                 "amount of time that the input source has been held down for, but if you only want to know whether "
                 "it has been moved use hasMovedSignificantlySincePressed instead.")]]
    bool hasMouseMovedSignificantlySincePressed() const noexcept;
    /** @endcond */

private:
    //==============================================================================
    friend class ComponentPeer;
    friend class Desktop;
    friend class detail::MouseInputSourceList;
    friend class detail::MouseInputSourceImpl;
    detail::MouseInputSourceImpl* pimpl;

    explicit MouseInputSource (detail::MouseInputSourceImpl*) noexcept;
    void handleEvent (ComponentPeer&, Point<float>, int64 time, ModifierKeys, float, float, const PenDetails&);
    void handleWheel (ComponentPeer&, Point<float>, int64 time, const MouseWheelDetails&);
    void handleMagnifyGesture (ComponentPeer&, Point<float>, int64 time, float scaleFactor);

    static Point<float> getCurrentRawMousePosition();
    static void setRawMousePosition (Point<float>);

    JUCE_LEAK_DETECTOR (MouseInputSource)
};

} // namespace juce
