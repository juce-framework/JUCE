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

ComponentBoundsConstrainer::ComponentBoundsConstrainer() noexcept {}
ComponentBoundsConstrainer::~ComponentBoundsConstrainer() {}

//==============================================================================
void ComponentBoundsConstrainer::setMinimumWidth  (int minimumWidth) noexcept   { minW = minimumWidth; }
void ComponentBoundsConstrainer::setMaximumWidth  (int maximumWidth) noexcept   { maxW = maximumWidth; }
void ComponentBoundsConstrainer::setMinimumHeight (int minimumHeight) noexcept  { minH = minimumHeight; }
void ComponentBoundsConstrainer::setMaximumHeight (int maximumHeight) noexcept  { maxH = maximumHeight; }

void ComponentBoundsConstrainer::setMinimumSize (int minimumWidth, int minimumHeight) noexcept
{
    jassert (maxW >= minimumWidth);
    jassert (maxH >= minimumHeight);
    jassert (minimumWidth > 0 && minimumHeight > 0);

    minW = minimumWidth;
    minH = minimumHeight;

    if (minW > maxW)  maxW = minW;
    if (minH > maxH)  maxH = minH;
}

void ComponentBoundsConstrainer::setMaximumSize (int maximumWidth, int maximumHeight) noexcept
{
    jassert (maximumWidth >= minW);
    jassert (maximumHeight >= minH);
    jassert (maximumWidth > 0 && maximumHeight > 0);

    maxW = jmax (minW, maximumWidth);
    maxH = jmax (minH, maximumHeight);
}

void ComponentBoundsConstrainer::setSizeLimits (int minimumWidth,
                                                int minimumHeight,
                                                int maximumWidth,
                                                int maximumHeight) noexcept
{
    jassert (maximumWidth >= minimumWidth);
    jassert (maximumHeight >= minimumHeight);
    jassert (maximumWidth > 0 && maximumHeight > 0);
    jassert (minimumWidth > 0 && minimumHeight > 0);

    minW = jmax (0, minimumWidth);
    minH = jmax (0, minimumHeight);
    maxW = jmax (minW, maximumWidth);
    maxH = jmax (minH, maximumHeight);
}

void ComponentBoundsConstrainer::setMinimumOnscreenAmounts (int minimumWhenOffTheTop,
                                                            int minimumWhenOffTheLeft,
                                                            int minimumWhenOffTheBottom,
                                                            int minimumWhenOffTheRight) noexcept
{
    minOffTop    = minimumWhenOffTheTop;
    minOffLeft   = minimumWhenOffTheLeft;
    minOffBottom = minimumWhenOffTheBottom;
    minOffRight  = minimumWhenOffTheRight;
}

void ComponentBoundsConstrainer::setFixedAspectRatio (double widthOverHeight) noexcept
{
    aspectRatio = jmax (0.0, widthOverHeight);
}

double ComponentBoundsConstrainer::getFixedAspectRatio() const noexcept
{
    return aspectRatio;
}

void ComponentBoundsConstrainer::setBoundsForComponent (Component* component,
                                                        Rectangle<int> targetBounds,
                                                        bool isStretchingTop,
                                                        bool isStretchingLeft,
                                                        bool isStretchingBottom,
                                                        bool isStretchingRight)
{
    jassert (component != nullptr);

    Rectangle<int> limits, bounds (targetBounds);
    BorderSize<int> border;

    if (auto* parent = component->getParentComponent())
    {
        limits.setSize (parent->getWidth(), parent->getHeight());
    }
    else
    {
        if (auto* peer = component->getPeer())
            border = peer->getFrameSize();

        auto screenBounds = Desktop::getInstance().getDisplays().findDisplayForPoint (targetBounds.getCentre()).userArea;

        limits = component->getLocalArea (nullptr, screenBounds) + component->getPosition();
    }

    border.addTo (bounds);

    checkBounds (bounds,
                 border.addedTo (component->getBounds()), limits,
                 isStretchingTop, isStretchingLeft,
                 isStretchingBottom, isStretchingRight);

    border.subtractFrom (bounds);

    applyBoundsToComponent (*component, bounds);
}

void ComponentBoundsConstrainer::checkComponentBounds (Component* component)
{
    setBoundsForComponent (component, component->getBounds(),
                           false, false, false, false);
}

void ComponentBoundsConstrainer::applyBoundsToComponent (Component& component, Rectangle<int> bounds)
{
    if (auto* positioner = component.getPositioner())
        positioner->applyNewBounds (bounds);
    else
        component.setBounds (bounds);
}

//==============================================================================
void ComponentBoundsConstrainer::resizeStart()
{
}

void ComponentBoundsConstrainer::resizeEnd()
{
}

//==============================================================================
void ComponentBoundsConstrainer::checkBounds (Rectangle<int>& bounds,
                                              const Rectangle<int>& old,
                                              const Rectangle<int>& limits,
                                              bool isStretchingTop,
                                              bool isStretchingLeft,
                                              bool isStretchingBottom,
                                              bool isStretchingRight)
{
    if (isStretchingLeft)
        bounds.setLeft (jlimit (old.getRight() - maxW, old.getRight() - minW, bounds.getX()));
    else
        bounds.setWidth (jlimit (minW, maxW, bounds.getWidth()));

    if (isStretchingTop)
        bounds.setTop (jlimit (old.getBottom() - maxH, old.getBottom() - minH, bounds.getY()));
    else
        bounds.setHeight (jlimit (minH, maxH, bounds.getHeight()));

    if (bounds.isEmpty())
        return;

    if (minOffTop > 0)
    {
        const int limit = limits.getY() + jmin (minOffTop - bounds.getHeight(), 0);

        if (bounds.getY() < limit)
        {
            if (isStretchingTop)
                bounds.setTop (limits.getY());
            else
                bounds.setY (limit);
        }
    }

    if (minOffLeft > 0)
    {
        const int limit = limits.getX() + jmin (minOffLeft - bounds.getWidth(), 0);

        if (bounds.getX() < limit)
        {
            if (isStretchingLeft)
                bounds.setLeft (limits.getX());
            else
                bounds.setX (limit);
        }
    }

    if (minOffBottom > 0)
    {
        const int limit = limits.getBottom() - jmin (minOffBottom, bounds.getHeight());

        if (bounds.getY() > limit)
        {
            if (isStretchingBottom)
                bounds.setBottom (limits.getBottom());
            else
                bounds.setY (limit);
        }
    }

    if (minOffRight > 0)
    {
        const int limit = limits.getRight() - jmin (minOffRight, bounds.getWidth());

        if (bounds.getX() > limit)
        {
            if (isStretchingRight)
                bounds.setRight (limits.getRight());
            else
                bounds.setX (limit);
        }
    }

    // constrain the aspect ratio if one has been specified..
    if (aspectRatio > 0.0)
    {
        bool adjustWidth;

        if ((isStretchingTop || isStretchingBottom) && ! (isStretchingLeft || isStretchingRight))
        {
            adjustWidth = true;
        }
        else if ((isStretchingLeft || isStretchingRight) && ! (isStretchingTop || isStretchingBottom))
        {
            adjustWidth = false;
        }
        else
        {
            const double oldRatio = (old.getHeight() > 0) ? std::abs (old.getWidth() / (double) old.getHeight()) : 0.0;
            const double newRatio = std::abs (bounds.getWidth() / (double) bounds.getHeight());

            adjustWidth = (oldRatio > newRatio);
        }

        if (adjustWidth)
        {
            bounds.setWidth (roundToInt (bounds.getHeight() * aspectRatio));

            if (bounds.getWidth() > maxW || bounds.getWidth() < minW)
            {
                bounds.setWidth (jlimit (minW, maxW, bounds.getWidth()));
                bounds.setHeight (roundToInt (bounds.getWidth() / aspectRatio));
            }
        }
        else
        {
            bounds.setHeight (roundToInt (bounds.getWidth() / aspectRatio));

            if (bounds.getHeight() > maxH || bounds.getHeight() < minH)
            {
                bounds.setHeight (jlimit (minH, maxH, bounds.getHeight()));
                bounds.setWidth (roundToInt (bounds.getHeight() * aspectRatio));
            }
        }

        if ((isStretchingTop || isStretchingBottom) && ! (isStretchingLeft || isStretchingRight))
        {
            bounds.setX (old.getX() + (old.getWidth() - bounds.getWidth()) / 2);
        }
        else if ((isStretchingLeft || isStretchingRight) && ! (isStretchingTop || isStretchingBottom))
        {
            bounds.setY (old.getY() + (old.getHeight() - bounds.getHeight()) / 2);
        }
        else
        {
            if (isStretchingLeft)
                bounds.setX (old.getRight() - bounds.getWidth());

            if (isStretchingTop)
                bounds.setY (old.getBottom() - bounds.getHeight());
        }
    }

    jassert (! bounds.isEmpty());
}

} // namespace juce
