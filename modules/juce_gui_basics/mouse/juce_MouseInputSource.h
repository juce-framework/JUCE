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

#ifndef JUCE_MOUSEINPUTSOURCE_H_INCLUDED
#define JUCE_MOUSEINPUTSOURCE_H_INCLUDED


//==============================================================================
/**
    Represents a linear source of mouse events from a mouse device or individual finger
    in a multi-touch environment.

    Each MouseEvent object contains a reference to the MouseInputSource that generated
    it. In an environment with a single mouse for input, all events will come from the
    same source, but in a multi-touch system, there may be multiple MouseInputSource
    obects active, each representing a stream of events coming from a particular finger.

    Events coming from a single MouseInputSource are always sent in a fixed and predictable
    order: a mouseMove will never be called without a mouseEnter having been sent beforehand,
    the only events that can happen between a mouseDown and its corresponding mouseUp are
    mouseDrags, etc.
    When there are multiple touches arriving from multiple MouseInputSources, their
    event streams may arrive in an interleaved order, so you should use the getIndex()
    method to find out which finger each event came from.

    @see MouseEvent
*/
class JUCE_API  MouseInputSource
{
public:
    //==============================================================================
    MouseInputSource (const MouseInputSource&) noexcept;
    MouseInputSource& operator= (const MouseInputSource&) noexcept;
    ~MouseInputSource() noexcept;

    //==============================================================================
    bool operator== (const MouseInputSource& other) const noexcept     { return pimpl == other.pimpl; }
    bool operator!= (const MouseInputSource& other) const noexcept     { return pimpl != other.pimpl; }

    //==============================================================================
    /** Returns true if this object represents a normal desk-based mouse device. */
    bool isMouse() const noexcept;

    /** Returns true if this object represents a source of touch events - i.e. a finger or stylus. */
    bool isTouch() const noexcept;

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

    /** Returns true if the current pressure value is meaningful. */
    bool isPressureValid() const noexcept;

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

    /** Returns true if this mouse is currently down, and if it has been dragged more
        than a couple of pixels from the place it was pressed.
    */
    bool hasMouseMovedSignificantlySincePressed() const noexcept;

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
    static const float invalidPressure;

private:
    //==============================================================================
    friend class ComponentPeer;
    friend class Desktop;
    friend class MouseInputSourceInternal;
    MouseInputSourceInternal* pimpl;

    struct SourceList;

    explicit MouseInputSource (MouseInputSourceInternal*) noexcept;
    void handleEvent (ComponentPeer&, Point<float>, int64 time, ModifierKeys, float);
    void handleWheel (ComponentPeer&, Point<float>, int64 time, const MouseWheelDetails&);
    void handleMagnifyGesture (ComponentPeer&, Point<float>, int64 time, float scaleFactor);

    static Point<float> getCurrentRawMousePosition();
    static void setRawMousePosition (Point<float>);

    JUCE_LEAK_DETECTOR (MouseInputSource)
};


#endif   // JUCE_MOUSEINPUTSOURCE_H_INCLUDED
