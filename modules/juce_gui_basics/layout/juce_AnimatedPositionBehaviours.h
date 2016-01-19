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

#ifndef JUCE_ANIMATEDPOSITIONBEHAVIOURS_H_INCLUDED
#define JUCE_ANIMATEDPOSITIONBEHAVIOURS_H_INCLUDED


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
    */
    struct ContinuousWithMomentum
    {
        ContinuousWithMomentum() noexcept
            : velocity (0), damping (0.92)
        {
        }

        /** Sets the friction that damps the movement of the value.
            A typical value is 0.08; higher values indicate more friction.
        */
        void setFriction (double newFriction) noexcept
        {
            damping = 1.0 - newFriction;
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

            if (std::abs (velocity) < 0.05)
                velocity = 0;

            return oldPos + velocity * elapsedSeconds;
        }

        /** Called by the AnimatedPosition class to check whether the object
            is now stationary.
        */
        bool isStopped (double /*position*/) const noexcept
        {
            return velocity == 0;
        }

    private:
        double velocity, damping;
    };

    //==============================================================================
    /** A behaviour that gravitates an AnimatedPosition object towards the nearest
        integer position when released.

        This class is intended to be used as a template parameter to the
        AnimatedPosition class. It's handy when using an AnimatedPosition to show a
        series of pages, because it allows the pages can be scrolled smoothly, but when
        released, snaps back to show a whole page.

        @see AnimatedPosition
    */
    struct SnapToPageBoundaries
    {
        SnapToPageBoundaries() noexcept   : targetSnapPosition()
        {
        }

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
        double targetSnapPosition;
    };
}


#endif   // JUCE_ANIMATEDPOSITIONBEHAVIOURS_H_INCLUDED
