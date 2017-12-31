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

//==============================================================================
/**
    Models a 1-dimensional position that can be dragged around by the user, and which
    will then continue moving with a customisable physics behaviour when released.

    This is useful for things like scrollable views or objects that can be dragged and
    thrown around with the mouse/touch, and by writing your own behaviour class, you can
    customise the trajectory that it follows when released.

    The class uses its own Timer to continuously change its value when a drag ends, and
    Listener objects can be registered to receive callbacks whenever the value changes.

    The value is stored as a double, and can be used to represent whatever units you need.

    The template parameter Behaviour must be a class that implements various methods to
    return the physics of the value's movement - you can use the classes provided for this
    in the AnimatedPositionBehaviours namespace, or write your own custom behaviour.

    @see AnimatedPositionBehaviours::ContinuousWithMomentum,
         AnimatedPositionBehaviours::SnapToPageBoundaries
*/
template <typename Behaviour>
class AnimatedPosition  : private Timer
{
public:
    AnimatedPosition()
        : position(), grabbedPos(), releaseVelocity(),
          range (-std::numeric_limits<double>::max(),
                  std::numeric_limits<double>::max())
    {
    }

    /** Sets a range within which the value will be constrained. */
    void setLimits (Range<double> newRange) noexcept
    {
        range = newRange;
    }

    //==============================================================================
    /** Called to indicate that the object is now being controlled by a
        mouse-drag or similar operation.

        After calling this method, you should make calls to the drag() method
        each time the mouse drags the position around, and always be sure to
        finish with a call to endDrag() when the mouse is released, which allows
        the position to continue moving freely according to the specified behaviour.
    */
    void beginDrag()
    {
        grabbedPos = position;
        releaseVelocity = 0;
        stopTimer();
    }

    /** Called during a mouse-drag operation, to indicate that the mouse has moved.
        The delta is the difference between the position when beginDrag() was called
        and the new position that's required.
    */
    void drag (double deltaFromStartOfDrag)
    {
        moveTo (grabbedPos + deltaFromStartOfDrag);
    }

    /** Called after beginDrag() and drag() to indicate that the drag operation has
        now finished.
    */
    void endDrag()
    {
        startTimerHz (60);
    }

    /** Called outside of a drag operation to cause a nudge in the specified direction.
        This is intended for use by e.g. mouse-wheel events.
    */
    void nudge (double deltaFromCurrentPosition)
    {
        startTimerHz (10);
        moveTo (position + deltaFromCurrentPosition);
    }

    //==============================================================================
    /** Returns the current position. */
    double getPosition() const noexcept
    {
        return position;
    }

    /** Explicitly sets the position and stops any further movement.
        This will cause a synchronous call to any listeners if the position actually
        changes.
    */
    void setPosition (double newPosition)
    {
        stopTimer();
        setPositionAndSendChange (newPosition);
    }

    //==============================================================================
    /** Implement this class if you need to receive callbacks when the value of
        an AnimatedPosition changes.
        @see AnimatedPosition::addListener, AnimatedPosition::removeListener
    */
    class Listener
    {
    public:
        virtual ~Listener() {}

        /** Called synchronously when an AnimatedPosition changes. */
        virtual void positionChanged (AnimatedPosition&, double newPosition) = 0;
    };

    /** Adds a listener to be called when the value changes. */
    void addListener (Listener* listener)       { listeners.add (listener); }

    /** Removes a previously-registered listener. */
    void removeListener (Listener* listener)    { listeners.remove (listener); }

    //==============================================================================
    /** The behaviour object.
        This is public to let you tweak any parameters that it provides.
    */
    Behaviour behaviour;

private:
    //==============================================================================
    double position, grabbedPos, releaseVelocity;
    Range<double> range;
    Time lastUpdate, lastDrag;
    ListenerList<Listener> listeners;

    static double getSpeed (const Time last, double lastPos,
                            const Time now, double newPos)
    {
        auto elapsedSecs = jmax (0.005, (now - last).inSeconds());
        auto v = (newPos - lastPos) / elapsedSecs;
        return std::abs (v) > 0.2 ? v : 0.0;
    }

    void moveTo (double newPos)
    {
        auto now = Time::getCurrentTime();
        releaseVelocity = getSpeed (lastDrag, position, now, newPos);
        behaviour.releasedWithVelocity (newPos, releaseVelocity);
        lastDrag = now;

        setPositionAndSendChange (newPos);
    }

    void setPositionAndSendChange (double newPosition)
    {
        newPosition = range.clipValue (newPosition);

        if (position != newPosition)
        {
            position = newPosition;
            listeners.call ([this, newPosition] (Listener& l) { l.positionChanged (*this, newPosition); });
        }
    }

    void timerCallback() override
    {
        auto now = Time::getCurrentTime();
        auto elapsed = jlimit (0.001, 0.020, (now - lastUpdate).inSeconds());
        lastUpdate = now;
        auto newPos = behaviour.getNextPosition (position, elapsed);

        if (behaviour.isStopped (newPos))
            stopTimer();
        else
            startTimerHz (60);

        setPositionAndSendChange (newPos);
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AnimatedPosition)
};

} // namespace juce
