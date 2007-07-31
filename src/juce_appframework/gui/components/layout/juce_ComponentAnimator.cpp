/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#include "../../../../juce_core/basics/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_ComponentAnimator.h"


//==============================================================================
struct AnimationTask
{
    AnimationTask (Component* const comp)
        : component (comp),
          watcher (comp)
    {
    }

    Component* component;
    ComponentDeletionWatcher watcher;
    Rectangle destination;
    int msElapsed, msTotal;
    double startSpeed, midSpeed, endSpeed, lastProgress;
    double left, top, right, bottom;

    bool useTimeslice (const int elapsed)
    {
        if (watcher.hasBeenDeleted())
            return false;

        msElapsed += elapsed;
        double newProgress = msElapsed / (double) msTotal;

        if (newProgress >= 0 && newProgress < 1.0)
        {
            newProgress = timeToDistance (newProgress);
            const double delta = (newProgress - lastProgress) / (1.0 - lastProgress);
            jassert (newProgress >= lastProgress);
            lastProgress = newProgress;

            left += (destination.getX() - left) * delta;
            top += (destination.getY() - top) * delta;
            right += (destination.getRight() - right) * delta;
            bottom += (destination.getBottom() - bottom) * delta;

            if (delta < 1.0)
            {
                const Rectangle newBounds (roundDoubleToInt (left),
                                           roundDoubleToInt (top),
                                           roundDoubleToInt (right - left),
                                           roundDoubleToInt (bottom - top));

                if (newBounds != destination)
                {
                    component->setBounds (newBounds);
                    return true;
                }
            }
        }

        component->setBounds (destination);
        return false;
    }

    void moveToFinalDestination()
    {
        if (! watcher.hasBeenDeleted())
            component->setBounds (destination);
    }

private:
    inline double timeToDistance (const double time) const
    {
        return (time < 0.5) ? time * (startSpeed + time * (midSpeed - startSpeed))
                            : 0.5 * (startSpeed + 0.5 * (midSpeed - startSpeed))
                                + (time - 0.5) * (midSpeed + (time - 0.5) * (endSpeed - midSpeed));
    }
};

//==============================================================================
ComponentAnimator::ComponentAnimator()
    : lastTime (0)
{
}

ComponentAnimator::~ComponentAnimator()
{
    cancelAllAnimations (false);
    jassert (tasks.size() == 0);
}

//==============================================================================
void* ComponentAnimator::findTaskFor (Component* const component) const
{
    for (int i = tasks.size(); --i >= 0;)
        if (component == ((AnimationTask*) tasks.getUnchecked(i))->component)
            return tasks.getUnchecked(i);

    return 0;
}

void ComponentAnimator::animateComponent (Component* const component,
                                          const Rectangle& finalPosition,
                                          const int millisecondsToSpendMoving,
                                          const double startSpeed,
                                          const double endSpeed)
{
    if (component != 0)
    {
        AnimationTask* at = (AnimationTask*) findTaskFor (component);

        if (at == 0)
        {
            at = new AnimationTask (component);
            tasks.add (at);
        }

        at->msElapsed = 0;
        at->lastProgress = 0;
        at->msTotal = jmax (1, millisecondsToSpendMoving);
        at->destination = finalPosition;

        // the speeds must be 0 or greater!
        jassert (startSpeed >= 0 && endSpeed >= 0)

        const double invTotalDistance = 4.0 / (startSpeed + endSpeed + 2.0);
        at->startSpeed = jmax (0.0, startSpeed * invTotalDistance);
        at->midSpeed = invTotalDistance;
        at->endSpeed = jmax (0.0, endSpeed * invTotalDistance);

        at->left = component->getX();
        at->top = component->getY();
        at->right = component->getRight();
        at->bottom = component->getBottom();

        if (! isTimerRunning())
        {
            lastTime = Time::getMillisecondCounter();
            startTimer (1000 / 50);
        }
    }
}

void ComponentAnimator::cancelAllAnimations (const bool moveComponentsToTheirFinalPositions)
{
    for (int i = tasks.size(); --i >= 0;)
    {
        AnimationTask* const at = (AnimationTask*) tasks.getUnchecked(i);

        if (moveComponentsToTheirFinalPositions)
            at->moveToFinalDestination();

        delete at;
        tasks.remove (i);
    }
}

void ComponentAnimator::cancelAnimation (Component* const component,
                                         const bool moveComponentToItsFinalPosition)
{
    AnimationTask* const at = (AnimationTask*) findTaskFor (component);

    if (at != 0)
    {
        if (moveComponentToItsFinalPosition)
            at->moveToFinalDestination();

        tasks.removeValue (at);
        delete at;
    }
}

const Rectangle ComponentAnimator::getComponentDestination (Component* const component)
{
    AnimationTask* const at = (AnimationTask*) findTaskFor (component);

    if (at != 0)
        return at->destination;
    else if (component != 0)
        return component->getBounds();

    return Rectangle();
}

void ComponentAnimator::timerCallback()
{
    const uint32 timeNow = Time::getMillisecondCounter();

    if (lastTime == 0 || lastTime == timeNow)
        lastTime = timeNow;

    const int elapsed = timeNow - lastTime;

    for (int i = tasks.size(); --i >= 0;)
    {
        AnimationTask* const at = (AnimationTask*) tasks.getUnchecked(i);

        if (! at->useTimeslice (elapsed))
        {
            tasks.remove (i);
            delete at;
        }
    }

    lastTime = timeNow;

    if (tasks.size() == 0)
        stopTimer();
}


END_JUCE_NAMESPACE
