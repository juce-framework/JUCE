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
/** Contains classes for different types of physics behaviours - these classes
    are used as template parameters for the AnimatedPosition class.
*/
namespace AnimatedPositionBehaviours
{
    /** A non-snapping behaviour that allows the content to be freely flicked in
        either direction, with momentum based on the velocity at which it was
        released, and variable friction to make it come to a halt.

        This class is intended to be used as a template parameter to the
        AnimatedPosition class.

        @see AnimatedPosition

        @tags{GUI}
    */
    struct ContinuousWithMomentum
    {
        ContinuousWithMomentum() = default;

        /** Sets the friction that damps the movement of the value.
            A typical value is 0.08; higher values indicate more friction.
        */
        void setFriction (double newFriction) noexcept
        {
            damping = 1.0 - newFriction;
        }

        /** Sets the minimum velocity of the movement. Any velocity that's slower than
            this will stop the animation. The default is 0.05. */
        void setMinimumVelocity (double newMinimumVelocityToUse) noexcept
        {
            minimumVelocity = newMinimumVelocityToUse;
        }

        /** Called by the AnimatedPosition class. This tells us the position and
            velocity at which the user is about to release the object.
            The velocity is measured in units/second.
        */
        void releasedWithVelocity (double /*position*/, double releaseVelocity) noexcept
        {
            velocity = releaseVelocity;
        }

        /** Called by the AnimatedPosition class to get the new position, after
            the given time has elapsed.
        */
        double getNextPosition (double oldPos, double elapsedSeconds) noexcept
        {
            velocity *= damping;

            if (std::abs (velocity) < minimumVelocity)
                velocity = 0;

            return oldPos + velocity * elapsedSeconds;
        }

        /** Called by the AnimatedPosition class to check whether the object
            is now stationary.
        */
        bool isStopped (double /*position*/) const noexcept
        {
            return velocity == 0.0;
        }

    private:
        double velocity = 0, damping = 0.92, minimumVelocity = 0.05;
    };

    //==============================================================================
    /** A behaviour that gravitates an AnimatedPosition object towards the nearest
        integer position when released.

        This class is intended to be used as a template parameter to the
        AnimatedPosition class. It's handy when using an AnimatedPosition to show a
        series of pages, because it allows the pages can be scrolled smoothly, but when
        released, snaps back to show a whole page.

        @see AnimatedPosition

        @tags{GUI}
    */
    struct SnapToPageBoundaries
    {
        SnapToPageBoundaries() = default;

        /** Called by the AnimatedPosition class. This tells us the position and
            velocity at which the user is about to release the object.
            The velocity is measured in units/second.
        */
        void releasedWithVelocity (double position, double releaseVelocity) noexcept
        {
            targetSnapPosition = std::floor (position + 0.5);

            if (releaseVelocity >  1.0 && targetSnapPosition < position)  ++targetSnapPosition;
            if (releaseVelocity < -1.0 && targetSnapPosition > position)  --targetSnapPosition;
        }

        /** Called by the AnimatedPosition class to get the new position, after
            the given time has elapsed.
        */
        double getNextPosition (double oldPos, double elapsedSeconds) const noexcept
        {
            if (isStopped (oldPos))
                return targetSnapPosition;

            const double snapSpeed = 10.0;
            const double velocity = (targetSnapPosition - oldPos) * snapSpeed;
            const double newPos = oldPos + velocity * elapsedSeconds;

            return isStopped (newPos) ? targetSnapPosition : newPos;
        }

        /** Called by the AnimatedPosition class to check whether the object
            is now stationary.
        */
        bool isStopped (double position) const noexcept
        {
            return std::abs (targetSnapPosition - position) < 0.001;
        }

    private:
        double targetSnapPosition = 0.0;
    };
}

} // namespace juce
