/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#ifndef __JUCE_COMPONENTANIMATOR_JUCEHEADER__
#define __JUCE_COMPONENTANIMATOR_JUCEHEADER__

#include "../juce_Component.h"
#include "../juce_ComponentDeletionWatcher.h"
#include "../../../events/juce_ChangeBroadcaster.h"
#include "../../../events/juce_Timer.h"


//==============================================================================
/**
    Animates a set of components, moving it to a new position.

    To use this, create a ComponentAnimator, and use its animateComponent() method
    to tell it to move components to destination positions. Any number of
    components can be animated by one ComponentAnimator object (if you've got a
    lot of components to move, it's much more efficient to share a single animator
    than to have many animators running at once).

    You'll need to make sure the animator object isn't deleted before it finishes
    moving the components.

    The class is a ChangeBroadcaster and sends a notification when any components
    start or finish being animated.
*/
class JUCE_API  ComponentAnimator  : public ChangeBroadcaster,
                                     private Timer
{
public:
    //==============================================================================
    /** Creates a ComponentAnimator. */
    ComponentAnimator();

    /** Destructor. */
    ~ComponentAnimator();

    //==============================================================================
    /** Starts a component moving from its current position to a specified position.

        If the component is already in the middle of an animation, that will be abandoned,
        and a new animation will begin, moving the component from its current location.

        The start and end speed parameters let you apply some acceleration to the component's
        movement.

        @param component            the component to move
        @param finalPosition        the destination position and size to move it to
        @param millisecondsToSpendMoving    how long, in milliseconds, it should take
                                    to arrive at its destination
        @param startSpeed           a value to indicate the relative start speed of the
                                    animation. If this is 0, the component will start
                                    by accelerating from rest; higher values mean that it
                                    will have an initial speed greater than zero. If the
                                    value if greater than 1, it will decelerate towards the
                                    middle of its journey. To move the component at a constant
                                    rate for its entire animation, set both the start and
                                    end speeds to 1.0
        @param endSpeed             a relative speed at which the component should be moving
                                    when the animation finishes. If this is 0, the component
                                    will decelerate to a standstill at its final position; higher
                                    values mean the component will still be moving when it stops.
                                    To move the component at a constant rate for its entire
                                    animation, set both the start and end speeds to 1.0
    */
    void animateComponent (Component* const component,
                           const Rectangle& finalPosition,
                           const int millisecondsToSpendMoving,
                           const double startSpeed = 1.0,
                           const double endSpeed = 1.0);

    /** Stops a component if it's currently being animated.

        If moveComponentToItsFinalPosition is true, then the component will
        be immediately moved to its destination position and size. If false, it will be
        left in whatever location it currently occupies.
    */
    void cancelAnimation (Component* const component,
                          const bool moveComponentToItsFinalPosition);

    /** Clears all of the active animations.

        If moveComponentsToTheirFinalPositions is true, all the components will
        be immediately set to their final positions. If false, they will be
        left in whatever locations they currently occupy.
    */
    void cancelAllAnimations (const bool moveComponentsToTheirFinalPositions);

    /** Returns the destination position for a component.

        If the component is being animated, this will return the target position that
        was specified when animateComponent() was called.

        If the specified component isn't currently being animated, this method will just
        return its current position.
    */
    const Rectangle getComponentDestination (Component* const component);

    /** Returns true if the specified component is currently being animated.
    */
    bool isAnimating (Component* component) const;

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    VoidArray tasks;
    uint32 lastTime;

    void* findTaskFor (Component* const component) const;
    void timerCallback();
};


#endif   // __JUCE_COMPONENTANIMATOR_JUCEHEADER__
