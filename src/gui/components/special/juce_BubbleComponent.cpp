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

#include "juce_BubbleComponent.h"
#include "../lookandfeel/juce_LookAndFeel.h"


//==============================================================================
BubbleComponent::BubbleComponent()
  : side (0),
    allowablePlacements (above | below | left | right),
    arrowTipX (0.0f),
    arrowTipY (0.0f)
{
    setInterceptsMouseClicks (false, false);

    shadow.setShadowProperties (5.0f, 0.35f, 0, 0);
    setComponentEffect (&shadow);
}

BubbleComponent::~BubbleComponent()
{
}

//==============================================================================
void BubbleComponent::paint (Graphics& g)
{
    int x = content.getX();
    int y = content.getY();
    int w = content.getWidth();
    int h = content.getHeight();

    int cw, ch;
    getContentSize (cw, ch);

    if (side == 3)
        x += w - cw;
    else if (side != 1)
        x += (w - cw) / 2;

    w = cw;

    if (side == 2)
        y += h - ch;
    else if (side != 0)
        y += (h - ch) / 2;

    h = ch;

    getLookAndFeel().drawBubble (g, arrowTipX, arrowTipY,
                                 (float) x, (float) y,
                                 (float) w, (float) h);

    const int cx = x + (w - cw) / 2;
    const int cy = y + (h - ch) / 2;

    const int indent = 3;

    g.setOrigin (cx + indent, cy + indent);
    g.reduceClipRegion (0, 0, cw - indent * 2, ch - indent * 2);

    paintContent (g, cw - indent * 2, ch - indent * 2);
}

//==============================================================================
void BubbleComponent::setAllowedPlacement (const int newPlacement)
{
    allowablePlacements = newPlacement;
}

void BubbleComponent::setPosition (Component* componentToPointTo)
{
    jassert (componentToPointTo->isValidComponent());

    int tx = 0;
    int ty = 0;

    if (getParentComponent() != 0)
        componentToPointTo->relativePositionToOtherComponent (getParentComponent(), tx, ty);
    else
        componentToPointTo->relativePositionToGlobal (tx, ty);

    setPosition (Rectangle (tx, ty, componentToPointTo->getWidth(), componentToPointTo->getHeight()));
}

void BubbleComponent::setPosition (const int arrowTipX_,
                                   const int arrowTipY_)
{
    setPosition (Rectangle (arrowTipX_, arrowTipY_, 1, 1));
}

//==============================================================================
void BubbleComponent::setPosition (const Rectangle& rectangleToPointTo)
{
    Rectangle availableSpace;

    if (getParentComponent() != 0)
    {
        availableSpace.setSize (getParentComponent()->getWidth(),
                                getParentComponent()->getHeight());
    }
    else
    {
        availableSpace = getParentMonitorArea();
    }

    int x = 0;
    int y = 0;
    int w = 150;
    int h = 30;

    getContentSize (w, h);
    w += 30;
    h += 30;

    const float edgeIndent = 2.0f;
    const int arrowLength = jmin (10, h / 3, w / 3);

    int spaceAbove = ((allowablePlacements & above) != 0) ? jmax (0, rectangleToPointTo.getY() - availableSpace.getY()) : -1;
    int spaceBelow = ((allowablePlacements & below) != 0) ? jmax (0, availableSpace.getBottom() - rectangleToPointTo.getBottom()) : -1;
    int spaceLeft  = ((allowablePlacements & left)  != 0) ? jmax (0, rectangleToPointTo.getX() - availableSpace.getX()) : -1;
    int spaceRight = ((allowablePlacements & right) != 0) ? jmax (0, availableSpace.getRight() - rectangleToPointTo.getRight()) : -1;

    // look at whether the component is elongated, and if so, try to position next to its longer dimension.
    if (rectangleToPointTo.getWidth() > rectangleToPointTo.getHeight() * 2
         && (spaceAbove > h + 20 || spaceBelow > h + 20))
    {
        spaceLeft = spaceRight = 0;
    }
    else if (rectangleToPointTo.getWidth() < rectangleToPointTo.getHeight() / 2
              && (spaceLeft > w + 20 || spaceRight > w + 20))
    {
        spaceAbove = spaceBelow = 0;
    }

    if (jmax (spaceAbove, spaceBelow) >= jmax (spaceLeft, spaceRight))
    {
        x = rectangleToPointTo.getX() + (rectangleToPointTo.getWidth() - w) / 2;
        arrowTipX = w * 0.5f;
        content.setSize (w, h - arrowLength);

        if (spaceAbove >= spaceBelow)
        {
            // above
            y = rectangleToPointTo.getY() - h;
            content.setPosition (0, 0);
            arrowTipY = h - edgeIndent;
            side = 2;
        }
        else
        {
            // below
            y = rectangleToPointTo.getBottom();
            content.setPosition (0, arrowLength);
            arrowTipY = edgeIndent;
            side = 0;
        }
    }
    else
    {
        y = rectangleToPointTo.getY() + (rectangleToPointTo.getHeight() - h) / 2;
        arrowTipY = h * 0.5f;
        content.setSize (w - arrowLength, h);

        if (spaceLeft > spaceRight)
        {
            // on the left
            x = rectangleToPointTo.getX() - w;
            content.setPosition (0, 0);
            arrowTipX = w - edgeIndent;
            side = 3;
        }
        else
        {
            // on the right
            x = rectangleToPointTo.getRight();
            content.setPosition (arrowLength, 0);
            arrowTipX = edgeIndent;
            side = 1;
        }
    }

    setBounds (x, y, w, h);
}

END_JUCE_NAMESPACE
