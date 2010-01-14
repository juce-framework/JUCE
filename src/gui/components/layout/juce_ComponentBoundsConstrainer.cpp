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

#include "../../../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_ComponentBoundsConstrainer.h"
#include "../juce_Desktop.h"


//==============================================================================
ComponentBoundsConstrainer::ComponentBoundsConstrainer() throw()
    : minW (0),
      maxW (0x3fffffff),
      minH (0),
      maxH (0x3fffffff),
      minOffTop (0),
      minOffLeft (0),
      minOffBottom (0),
      minOffRight (0),
      aspectRatio (0.0)
{
}

ComponentBoundsConstrainer::~ComponentBoundsConstrainer()
{
}

//==============================================================================
void ComponentBoundsConstrainer::setMinimumWidth (const int minimumWidth) throw()
{
    minW = minimumWidth;
}

void ComponentBoundsConstrainer::setMaximumWidth (const int maximumWidth) throw()
{
    maxW = maximumWidth;
}

void ComponentBoundsConstrainer::setMinimumHeight (const int minimumHeight) throw()
{
    minH = minimumHeight;
}

void ComponentBoundsConstrainer::setMaximumHeight (const int maximumHeight) throw()
{
    maxH = maximumHeight;
}

void ComponentBoundsConstrainer::setMinimumSize (const int minimumWidth, const int minimumHeight) throw()
{
    jassert (maxW >= minimumWidth);
    jassert (maxH >= minimumHeight);
    jassert (minimumWidth > 0 && minimumHeight > 0);

    minW = minimumWidth;
    minH = minimumHeight;

    if (minW > maxW)
        maxW = minW;

    if (minH > maxH)
        maxH = minH;
}

void ComponentBoundsConstrainer::setMaximumSize (const int maximumWidth, const int maximumHeight) throw()
{
    jassert (maximumWidth >= minW);
    jassert (maximumHeight >= minH);
    jassert (maximumWidth > 0 && maximumHeight > 0);

    maxW = jmax (minW, maximumWidth);
    maxH = jmax (minH, maximumHeight);
}

void ComponentBoundsConstrainer::setSizeLimits (const int minimumWidth,
                                                const int minimumHeight,
                                                const int maximumWidth,
                                                const int maximumHeight) throw()
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

void ComponentBoundsConstrainer::setMinimumOnscreenAmounts (const int minimumWhenOffTheTop,
                                                            const int minimumWhenOffTheLeft,
                                                            const int minimumWhenOffTheBottom,
                                                            const int minimumWhenOffTheRight) throw()
{
    minOffTop = minimumWhenOffTheTop;
    minOffLeft = minimumWhenOffTheLeft;
    minOffBottom = minimumWhenOffTheBottom;
    minOffRight = minimumWhenOffTheRight;
}

void ComponentBoundsConstrainer::setFixedAspectRatio (const double widthOverHeight) throw()
{
    aspectRatio = jmax (0.0, widthOverHeight);
}

double ComponentBoundsConstrainer::getFixedAspectRatio() const throw()
{
    return aspectRatio;
}

void ComponentBoundsConstrainer::setBoundsForComponent (Component* const component,
                                                        int x, int y, int w, int h,
                                                        const bool isStretchingTop,
                                                        const bool isStretchingLeft,
                                                        const bool isStretchingBottom,
                                                        const bool isStretchingRight)
{
    jassert (component != 0);

    Rectangle limits;
    Component* const p = component->getParentComponent();

    if (p == 0)
        limits = Desktop::getInstance().getAllMonitorDisplayAreas().getBounds();
    else
        limits.setSize (p->getWidth(), p->getHeight());

    if (component->isOnDesktop())
    {
        ComponentPeer* const peer = component->getPeer();
        const BorderSize border (peer->getFrameSize());

        x -= border.getLeft();
        y -= border.getTop();
        w += border.getLeftAndRight();
        h += border.getTopAndBottom();

        checkBounds (x, y, w, h,
                     border.addedTo (component->getBounds()), limits,
                     isStretchingTop, isStretchingLeft,
                     isStretchingBottom, isStretchingRight);

        x += border.getLeft();
        y += border.getTop();
        w -= border.getLeftAndRight();
        h -= border.getTopAndBottom();
    }
    else
    {
        checkBounds (x, y, w, h,
                     component->getBounds(), limits,
                     isStretchingTop, isStretchingLeft,
                     isStretchingBottom, isStretchingRight);
    }

    applyBoundsToComponent (component, x, y, w, h);
}

void ComponentBoundsConstrainer::checkComponentBounds (Component* component)
{
    setBoundsForComponent (component,
                           component->getX(), component->getY(),
                           component->getWidth(), component->getHeight(),
                           false, false, false, false);
}

void ComponentBoundsConstrainer::applyBoundsToComponent (Component* component,
                                                         int x, int y, int w, int h)
{
    component->setBounds (x, y, w, h);
}

//==============================================================================
void ComponentBoundsConstrainer::resizeStart()
{
}

void ComponentBoundsConstrainer::resizeEnd()
{
}

//==============================================================================
void ComponentBoundsConstrainer::checkBounds (int& x, int& y, int& w, int& h,
                                              const Rectangle& old,
                                              const Rectangle& limits,
                                              const bool isStretchingTop,
                                              const bool isStretchingLeft,
                                              const bool isStretchingBottom,
                                              const bool isStretchingRight)
{
    // constrain the size if it's being stretched..
    if (isStretchingLeft)
    {
        x = jlimit (old.getRight() - maxW, old.getRight() - minW, x);
        w = old.getRight() - x;
    }

    if (isStretchingRight)
    {
        w = jlimit (minW, maxW, w);
    }

    if (isStretchingTop)
    {
        y = jlimit (old.getBottom() - maxH, old.getBottom() - minH, y);
        h = old.getBottom() - y;
    }

    if (isStretchingBottom)
    {
        h = jlimit (minH, maxH, h);
    }

    // constrain the aspect ratio if one has been specified..
    if (aspectRatio > 0.0 && w > 0 && h > 0)
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
            const double oldRatio = (old.getHeight() > 0) ? fabs (old.getWidth() / (double) old.getHeight()) : 0.0;
            const double newRatio = fabs (w / (double) h);

            adjustWidth = (oldRatio > newRatio);
        }

        if (adjustWidth)
        {
            w = roundToInt (h * aspectRatio);

            if (w > maxW || w < minW)
            {
                w = jlimit (minW, maxW, w);
                h = roundToInt (w / aspectRatio);
            }
        }
        else
        {
            h = roundToInt (w / aspectRatio);

            if (h > maxH || h < minH)
            {
                h = jlimit (minH, maxH, h);
                w = roundToInt (h * aspectRatio);
            }
        }

        if ((isStretchingTop || isStretchingBottom) && ! (isStretchingLeft || isStretchingRight))
        {
            x = old.getX() + (old.getWidth() - w) / 2;
        }
        else if ((isStretchingLeft || isStretchingRight) && ! (isStretchingTop || isStretchingBottom))
        {
            y = old.getY() + (old.getHeight() - h) / 2;
        }
        else
        {
            if (isStretchingLeft)
                x = old.getRight() - w;

            if (isStretchingTop)
                y = old.getBottom() - h;
        }
    }

    // ...and constrain the position if limits have been set for that.
    if (minOffTop > 0 || minOffLeft > 0 || minOffBottom > 0 || minOffRight > 0)
    {
        if (minOffTop > 0)
        {
            const int limit = limits.getY() + jmin (minOffTop - h, 0);

            if (y < limit)
            {
                if (isStretchingTop)
                    h -= (limit - y);

                y = limit;
            }
        }

        if (minOffLeft > 0)
        {
            const int limit = limits.getX() + jmin (minOffLeft - w, 0);

            if (x < limit)
            {
                if (isStretchingLeft)
                    w -= (limit - x);

                x = limit;
            }
        }

        if (minOffBottom > 0)
        {
            const int limit = limits.getBottom() - jmin (minOffBottom, h);

            if (y > limit)
            {
                if (isStretchingBottom)
                    h += (limit - y);
                else
                    y = limit;
            }
        }

        if (minOffRight > 0)
        {
            const int limit = limits.getRight() - jmin (minOffRight, w);

            if (x > limit)
            {
                if (isStretchingRight)
                    w += (limit - x);
                else
                    x = limit;
            }
        }
    }

    jassert (w >= 0 && h >= 0);

}


END_JUCE_NAMESPACE
