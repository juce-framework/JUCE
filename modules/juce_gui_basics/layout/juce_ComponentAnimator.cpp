/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

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

class ComponentAnimator::AnimationTask
{
public:
    AnimationTask (Component* const comp)
        : component (comp)
    {
    }

    void reset (const Rectangle<int>& finalBounds,
                float finalAlpha,
                int millisecondsToSpendMoving,
                bool useProxyComponent,
                double startSpeed_, double endSpeed_)
    {
        msElapsed = 0;
        msTotal = jmax (1, millisecondsToSpendMoving);
        lastProgress = 0;
        destination = finalBounds;
        destAlpha = finalAlpha;

        isMoving = (finalBounds != component->getBounds());
        isChangingAlpha = (finalAlpha != component->getAlpha());

        left    = component->getX();
        top     = component->getY();
        right   = component->getRight();
        bottom  = component->getBottom();
        alpha   = component->getAlpha();

        const double invTotalDistance = 4.0 / (startSpeed_ + endSpeed_ + 2.0);
        startSpeed = jmax (0.0, startSpeed_ * invTotalDistance);
        midSpeed = invTotalDistance;
        endSpeed = jmax (0.0, endSpeed_ * invTotalDistance);

        if (useProxyComponent)
            proxy = new ProxyComponent (*component);
        else
            proxy = nullptr;

        component->setVisible (! useProxyComponent);
    }

    bool useTimeslice (const int elapsed)
    {
        if (Component* const c = proxy != nullptr ? static_cast <Component*> (proxy)
                                                  : static_cast <Component*> (component))
        {
            msElapsed += elapsed;
            double newProgress = msElapsed / (double) msTotal;

            if (newProgress >= 0 && newProgress < 1.0)
            {
                newProgress = timeToDistance (newProgress);
                const double delta = (newProgress - lastProgress) / (1.0 - lastProgress);
                jassert (newProgress >= lastProgress);
                lastProgress = newProgress;

                if (delta < 1.0)
                {
                    bool stillBusy = false;

                    if (isMoving)
                    {
                        left   += (destination.getX()      - left)   * delta;
                        top    += (destination.getY()      - top)    * delta;
                        right  += (destination.getRight()  - right)  * delta;
                        bottom += (destination.getBottom() - bottom) * delta;

                        const Rectangle<int> newBounds (roundToInt (left),
                                                        roundToInt (top),
                                                        roundToInt (right - left),
                                                        roundToInt (bottom - top));

                        if (newBounds != destination)
                        {
                            c->setBounds (newBounds);
                            stillBusy = true;
                        }
                    }

                    if (isChangingAlpha)
                    {
                        alpha += (destAlpha - alpha) * delta;
                        c->setAlpha ((float) alpha);
                        stillBusy = true;
                    }

                    if (stillBusy)
                        return true;
                }
            }
        }

        moveToFinalDestination();
        return false;
    }

    void moveToFinalDestination()
    {
        if (component != nullptr)
        {
            component->setAlpha ((float) destAlpha);
            component->setBounds (destination);

            if (proxy != nullptr)
                component->setVisible (destAlpha > 0);
        }
    }

    //==============================================================================
    class ProxyComponent  : public Component
    {
    public:
        ProxyComponent (Component& c)
            : image (c.createComponentSnapshot (c.getLocalBounds()))
        {
            setBounds (c.getBounds());
            setTransform (c.getTransform());
            setAlpha (c.getAlpha());
            setInterceptsMouseClicks (false, false);

            if (Component* const parent = c.getParentComponent())
                parent->addAndMakeVisible (this);
            else if (c.isOnDesktop() && c.getPeer() != nullptr)
                addToDesktop (c.getPeer()->getStyleFlags() | ComponentPeer::windowIgnoresKeyPresses);
            else
                jassertfalse; // seem to be trying to animate a component that's not visible..

            setVisible (true);
            toBehind (&c);
        }

        void paint (Graphics& g)
        {
            g.setOpacity (1.0f);
            g.drawImage (image, 0, 0, getWidth(), getHeight(),
                         0, 0, image.getWidth(), image.getHeight());
        }

    private:
        Image image;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProxyComponent)
    };

    WeakReference<Component> component;
    ScopedPointer<Component> proxy;

    Rectangle<int> destination;
    double destAlpha;

    int msElapsed, msTotal;
    double startSpeed, midSpeed, endSpeed, lastProgress;
    double left, top, right, bottom, alpha;
    bool isMoving, isChangingAlpha;

private:
    double timeToDistance (const double time) const noexcept
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
}

//==============================================================================
ComponentAnimator::AnimationTask* ComponentAnimator::findTaskFor (Component* const component) const noexcept
{
    for (int i = tasks.size(); --i >= 0;)
        if (component == tasks.getUnchecked(i)->component.get())
            return tasks.getUnchecked(i);

    return nullptr;
}

void ComponentAnimator::animateComponent (Component* const component,
                                          const Rectangle<int>& finalBounds,
                                          const float finalAlpha,
                                          const int millisecondsToSpendMoving,
                                          const bool useProxyComponent,
                                          const double startSpeed,
                                          const double endSpeed)
{
    // the speeds must be 0 or greater!
    jassert (startSpeed >= 0 && endSpeed >= 0)

    if (component != nullptr)
    {
        AnimationTask* at = findTaskFor (component);

        if (at == nullptr)
        {
            at = new AnimationTask (component);
            tasks.add (at);
            sendChangeMessage();
        }

        at->reset (finalBounds, finalAlpha, millisecondsToSpendMoving,
                   useProxyComponent, startSpeed, endSpeed);

        if (! isTimerRunning())
        {
            lastTime = Time::getMillisecondCounter();
            startTimer (1000 / 50);
        }
    }
}

void ComponentAnimator::fadeOut (Component* component, int millisecondsToTake)
{
    if (component != nullptr)
    {
        if (component->isShowing() && millisecondsToTake > 0)
            animateComponent (component, component->getBounds(), 0.0f, millisecondsToTake, true, 1.0, 1.0);

        component->setVisible (false);
    }
}

void ComponentAnimator::fadeIn (Component* component, int millisecondsToTake)
{
    if (component != nullptr && ! (component->isVisible() && component->getAlpha() == 1.0f))
    {
        component->setAlpha (0.0f);
        component->setVisible (true);
        animateComponent (component, component->getBounds(), 1.0f, millisecondsToTake, false, 1.0, 1.0);
    }
}

void ComponentAnimator::cancelAllAnimations (const bool moveComponentsToTheirFinalPositions)
{
    if (tasks.size() > 0)
    {
        if (moveComponentsToTheirFinalPositions)
            for (int i = tasks.size(); --i >= 0;)
                tasks.getUnchecked(i)->moveToFinalDestination();

        tasks.clear();
        sendChangeMessage();
    }
}

void ComponentAnimator::cancelAnimation (Component* const component,
                                         const bool moveComponentToItsFinalPosition)
{
    if (AnimationTask* const at = findTaskFor (component))
    {
        if (moveComponentToItsFinalPosition)
            at->moveToFinalDestination();

        tasks.removeObject (at);
        sendChangeMessage();
    }
}

Rectangle<int> ComponentAnimator::getComponentDestination (Component* const component)
{
    jassert (component != nullptr);

    if (AnimationTask* const at = findTaskFor (component))
        return at->destination;

    return component->getBounds();
}

bool ComponentAnimator::isAnimating (Component* component) const noexcept
{
    return findTaskFor (component) != nullptr;
}

bool ComponentAnimator::isAnimating() const noexcept
{
    return tasks.size() != 0;
}

void ComponentAnimator::timerCallback()
{
    const uint32 timeNow = Time::getMillisecondCounter();

    if (lastTime == 0 || lastTime == timeNow)
        lastTime = timeNow;

    const int elapsed = (int) (timeNow - lastTime);

    for (int i = tasks.size(); --i >= 0;)
    {
        if (! tasks.getUnchecked(i)->useTimeslice (elapsed))
        {
            tasks.remove (i);
            sendChangeMessage();
        }
    }

    lastTime = timeNow;

    if (tasks.size() == 0)
        stopTimer();
}
