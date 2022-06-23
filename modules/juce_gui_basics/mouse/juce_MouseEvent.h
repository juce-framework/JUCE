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
/**
    Contains position and status information about a mouse event.

    @see MouseListener, Component::mouseMove, Component::mouseEnter, Component::mouseExit,
         Component::mouseDown, Component::mouseUp, Component::mouseDrag

    @tags{GUI}
*/
class JUCE_API  MouseEvent  final
{
public:
    //==============================================================================
    /** Creates a MouseEvent.

        Normally an application will never need to use this.

        @param source           the source that's invoking the event
        @param position         the position of the mouse, relative to the component that is passed-in
        @param modifiers        the key modifiers at the time of the event
        @param pressure         the pressure of the touch or stylus, in the range 0 to 1. Devices that
                                do not support force information may return 0.0, 1.0, or a negative value,
                                depending on the platform
        @param orientation      the orientation of the touch input for this event in radians. The default is 0
        @param rotation         the rotation of the pen device for this event in radians. The default is 0
        @param tiltX            the tilt of the pen device along the x-axis between -1.0 and 1.0. The default is 0
        @param tiltY            the tilt of the pen device along the y-axis between -1.0 and 1.0. The default is 0
        @param eventComponent   the component that the mouse event applies to
        @param originator       the component that originally received the event
        @param eventTime        the time the event happened
        @param mouseDownPos     the position of the corresponding mouse-down event (relative to the component that is passed-in).
                                If there isn't a corresponding mouse-down (e.g. for a mouse-move), this will just be
                                the same as the current mouse-x position.
        @param mouseDownTime    the time at which the corresponding mouse-down event happened
                                If there isn't a corresponding mouse-down (e.g. for a mouse-move), this will just be
                                the same as the current mouse-event time.
        @param numberOfClicks   how many clicks, e.g. a double-click event will be 2, a triple-click will be 3, etc
        @param mouseWasDragged  whether the mouse has been dragged significantly since the previous mouse-down
    */
    MouseEvent (MouseInputSource source,
                Point<float> position,
                ModifierKeys modifiers,
                float pressure,
                float orientation, float rotation,
                float tiltX, float tiltY,
                Component* eventComponent,
                Component* originator,
                Time eventTime,
                Point<float> mouseDownPos,
                Time mouseDownTime,
                int numberOfClicks,
                bool mouseWasDragged) noexcept;

    MouseEvent (const MouseEvent&) = default;
    MouseEvent& operator= (const MouseEvent&) = delete;

    MouseEvent (MouseEvent&&) = default;
    MouseEvent& operator= (MouseEvent&&) = delete;

    //==============================================================================
    /** The position of the mouse when the event occurred.

        This value is relative to the top-left of the component to which the
        event applies (as indicated by the MouseEvent::eventComponent field).

        This is a more accurate floating-point version of the position returned by
        getPosition() and the integer x and y member variables.
    */
    const Point<float> position;

    /** The x-position of the mouse when the event occurred.

        This value is relative to the top-left of the component to which the
        event applies (as indicated by the MouseEvent::eventComponent field).

        For a floating-point coordinate, see MouseEvent::position
    */
    const int x;

    /** The y-position of the mouse when the event occurred.

        This value is relative to the top-left of the component to which the
        event applies (as indicated by the MouseEvent::eventComponent field).

        For a floating-point coordinate, see MouseEvent::position
    */
    const int y;

    /** The key modifiers associated with the event.

        This will let you find out which mouse buttons were down, as well as which
        modifier keys were held down.

        When used for mouse-up events, this will indicate the state of the mouse buttons
        just before they were released, so that you can tell which button they let go of.
    */
    const ModifierKeys mods;

    /** The pressure of the touch or stylus for this event.
        The range is 0 (soft) to 1 (hard).
        If the input device doesn't provide any pressure data, it may return a negative
        value here, or 0.0 or 1.0, depending on the platform.
    */
    const float pressure;

    /** The orientation of the touch input for this event in radians where 0 indicates a touch aligned with the x-axis
        and pointing from left to right; increasing values indicate rotation in the clockwise direction. The default is 0.
    */
    const float orientation;

    /** The rotation of the pen device for this event in radians. Indicates the clockwise
        rotation, or twist, of the pen. The default is 0.
    */
    const float rotation;

    /** The tilt of the pen device along the x-axis between -1.0 and 1.0. A positive value indicates
        a tilt to the right. The default is 0.
    */
    const float tiltX;

    /** The tilt of the pen device along the y-axis between -1.0 and 1.0. A positive value indicates
        a tilt toward the user. The default is 0.
    */
    const float tiltY;

    /** The coordinates of the last place that a mouse button was pressed.
        The coordinates are relative to the component specified in MouseEvent::component.
        @see getDistanceFromDragStart, getDistanceFromDragStartX, mouseWasDraggedSinceMouseDown
    */
    const Point<float> mouseDownPosition;

    /** The component that this event applies to.

        This is usually the component that the mouse was over at the time, but for mouse-drag
        events the mouse could actually be over a different component and the events are
        still sent to the component that the button was originally pressed on.

        The x and y member variables are relative to this component's position.

        If you use getEventRelativeTo() to retarget this object to be relative to a different
        component, this pointer will be updated, but originalComponent remains unchanged.

        @see originalComponent
    */
    Component* const eventComponent;

    /** The component that the event first occurred on.

        If you use getEventRelativeTo() to retarget this object to be relative to a different
        component, this value remains unchanged to indicate the first component that received it.

        @see eventComponent
    */
    Component* const originalComponent;

    /** The time that this mouse-event occurred. */
    const Time eventTime;

    /** The time that the corresponding mouse-down event occurred. */
    const Time mouseDownTime;

    /** The source device that generated this event. */
    MouseInputSource source;

    //==============================================================================
    /** Returns the x coordinate of the last place that a mouse was pressed.
        The coordinate is relative to the component specified in MouseEvent::component.
        @see getDistanceFromDragStart, getDistanceFromDragStartX, mouseWasDraggedSinceMouseDown
    */
    int getMouseDownX() const noexcept;

    /** Returns the y coordinate of the last place that a mouse was pressed.
        The coordinate is relative to the component specified in MouseEvent::component.
        @see getDistanceFromDragStart, getDistanceFromDragStartX, mouseWasDraggedSinceMouseDown
    */
    int getMouseDownY() const noexcept;

    /** Returns the coordinates of the last place that a mouse was pressed.
        The coordinates are relative to the component specified in MouseEvent::component.
        For a floating point version of this value, see mouseDownPosition.
        @see mouseDownPosition, getDistanceFromDragStart, getDistanceFromDragStartX, mouseWasDraggedSinceMouseDown
    */
    Point<int> getMouseDownPosition() const noexcept;

    /** Returns the straight-line distance between where the mouse is now and where it
        was the last time the button was pressed.

        This is quite handy for things like deciding whether the user has moved far enough
        for it to be considered a drag operation.

        @see getDistanceFromDragStartX
    */
    int getDistanceFromDragStart() const noexcept;

    /** Returns the difference between the mouse's current x position and where it was
        when the button was last pressed.

        @see getDistanceFromDragStart
    */
    int getDistanceFromDragStartX() const noexcept;

    /** Returns the difference between the mouse's current y position and where it was
        when the button was last pressed.

        @see getDistanceFromDragStart
    */
    int getDistanceFromDragStartY() const noexcept;

    /** Returns the difference between the mouse's current position and where it was
        when the button was last pressed.

        @see getDistanceFromDragStart
    */
    Point<int> getOffsetFromDragStart() const noexcept;

    /** Returns true if the user seems to be performing a drag gesture.

        This is only meaningful if called in either a mouseUp() or mouseDrag() method.

        It will return true if the user has dragged the mouse more than a few pixels from the place
        where the mouse-down occurred or the mouse has been held down for a significant amount of time.

        Once they have dragged it far enough for this method to return true, it will continue
        to return true until the mouse-up, even if they move the mouse back to the same
        location at which the mouse-down happened. This means that it's very handy for
        objects that can either be clicked on or dragged, as you can use it in the mouseDrag()
        callback to ignore small movements they might make while trying to click.
    */
    bool mouseWasDraggedSinceMouseDown() const noexcept;

    /** Returns true if the mouse event is part of a click gesture rather than a drag.
        This is effectively the opposite of mouseWasDraggedSinceMouseDown()
    */
    bool mouseWasClicked() const noexcept;

    /** For a click event, the number of times the mouse was clicked in succession.
        So for example a double-click event will return 2, a triple-click 3, etc.
    */
    int getNumberOfClicks() const noexcept                              { return numberOfClicks; }

    /** Returns the time that the mouse button has been held down for.

        If called from a mouseDrag or mouseUp callback, this will return the
        number of milliseconds since the corresponding mouseDown event occurred.
        If called in other contexts, e.g. a mouseMove, then the returned value
        may be 0 or an undefined value.
    */
    int getLengthOfMousePress() const noexcept;

    /** Returns true if the pressure value for this event is meaningful. */
    bool isPressureValid() const noexcept;

    /** Returns true if the orientation value for this event is meaningful. */
    bool isOrientationValid() const noexcept;

    /** Returns true if the rotation value for this event is meaningful. */
    bool isRotationValid() const noexcept;

    /** Returns true if the current tilt value (either x- or y-axis) is meaningful. */
    bool isTiltValid (bool tiltX) const noexcept;

    //==============================================================================
    /** The position of the mouse when the event occurred.

        This position is relative to the top-left of the component to which the
        event applies (as indicated by the MouseEvent::eventComponent field).

        For a floating-point position, see MouseEvent::position
    */
    Point<int> getPosition() const noexcept;

    /** Returns the mouse x position of this event, in global screen coordinates.
        The coordinates are relative to the top-left of the main monitor.
        @see getScreenPosition
    */
    int getScreenX() const;

    /** Returns the mouse y position of this event, in global screen coordinates.
        The coordinates are relative to the top-left of the main monitor.
        @see getScreenPosition
    */
    int getScreenY() const;

    /** Returns the mouse position of this event, in global screen coordinates.
        The coordinates are relative to the top-left of the main monitor.
        @see getMouseDownScreenPosition
    */
    Point<int> getScreenPosition() const;

    /** Returns the x coordinate at which the mouse button was last pressed.
        The coordinates are relative to the top-left of the main monitor.
        @see getMouseDownScreenPosition
    */
    int getMouseDownScreenX() const;

    /** Returns the y coordinate at which the mouse button was last pressed.
        The coordinates are relative to the top-left of the main monitor.
        @see getMouseDownScreenPosition
    */
    int getMouseDownScreenY() const;

    /** Returns the coordinates at which the mouse button was last pressed.
        The coordinates are relative to the top-left of the main monitor.
        @see getScreenPosition
    */
    Point<int> getMouseDownScreenPosition() const;

    //==============================================================================
    /** Creates a version of this event that is relative to a different component.

        The x and y positions of the event that is returned will have been
        adjusted to be relative to the new component.
        The component pointer that is passed-in must not be null.
    */
    MouseEvent getEventRelativeTo (Component* newComponent) const noexcept;

    /** Creates a copy of this event with a different position.
        All other members of the event object are the same, but the x and y are
        replaced with these new values.
    */
    MouseEvent withNewPosition (Point<float> newPosition) const noexcept;

    /** Creates a copy of this event with a different position.
        All other members of the event object are the same, but the x and y are
        replaced with these new values.
    */
    MouseEvent withNewPosition (Point<int> newPosition) const noexcept;

    //==============================================================================
    /** Changes the application-wide setting for the double-click time limit.

        This is the maximum length of time between mouse-clicks for it to be
        considered a double-click. It's used by the Component class.

        @see getDoubleClickTimeout, MouseListener::mouseDoubleClick
    */
    static void setDoubleClickTimeout (int timeOutMilliseconds) noexcept;

    /** Returns the application-wide setting for the double-click time limit.

        This is the maximum length of time between mouse-clicks for it to be
        considered a double-click. It's used by the Component class.

        @see setDoubleClickTimeout, MouseListener::mouseDoubleClick
    */
    static int getDoubleClickTimeout() noexcept;


private:
    //==============================================================================
    const uint8 numberOfClicks, wasMovedSinceMouseDown;
};


//==============================================================================
/**
    Contains status information about a mouse wheel event.

    @see MouseListener, MouseEvent

    @tags{GUI}
*/
struct MouseWheelDetails  final
{
    //==============================================================================
    /** The amount that the wheel has been moved in the X axis.

        If isReversed is true, then a negative deltaX means that the wheel has been
        pushed physically to the left.
        If isReversed is false, then a negative deltaX means that the wheel has been
        pushed physically to the right.
    */
    float deltaX;

    /** The amount that the wheel has been moved in the Y axis.

        If isReversed is true, then a negative deltaY means that the wheel has been
        pushed physically upwards.
        If isReversed is false, then a negative deltaY means that the wheel has been
        pushed physically downwards.
    */
    float deltaY;

    /** Indicates whether the user has reversed the direction of the wheel.
        See deltaX and deltaY for an explanation of the effects of this value.
    */
    bool isReversed;

    /** If true, then the wheel has continuous, un-stepped motion. */
    bool isSmooth;

    /** If true, then this event is part of the inertial momentum phase that follows
        the wheel being released. */
    bool isInertial;
};

//==============================================================================
/**
    Contains status information about a pen event.

    @see MouseListener, MouseEvent

    @tags{GUI}
*/
struct PenDetails  final
{
    /**
        The rotation of the pen device in radians. Indicates the clockwise rotation, or twist,
        of the pen. The default is 0.
    */
    float rotation;

    /**
        Indicates the angle of tilt of the pointer in a range of -1.0 to 1.0 along the x-axis where
        a positive value indicates a tilt to the right. The default is 0.
    */
    float tiltX;

    /**
        Indicates the angle of tilt of the pointer in a range of -1.0 to 1.0 along the y-axis where
        a positive value indicates a tilt toward the user. The default is 0.
    */
    float tiltY;
};

} // namespace juce
