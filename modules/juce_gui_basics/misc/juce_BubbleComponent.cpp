/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

BubbleComponent::BubbleComponent()
  : allowablePlacements (above | below | left | right)
{
    setInterceptsMouseClicks (false, false);
    lookAndFeelChanged();
}

BubbleComponent::~BubbleComponent() {}

//==============================================================================
void BubbleComponent::lookAndFeelChanged()
{
    getLookAndFeel().setComponentEffectForBubbleComponent (*this);
}

void BubbleComponent::paint (Graphics& g)
{
    getLookAndFeel().drawBubble (g, *this, arrowTip.toFloat(), content.toFloat());

    g.reduceClipRegion (content);
    g.setOrigin (content.getPosition());

    paintContent (g, content.getWidth(), content.getHeight());
}

void BubbleComponent::setAllowedPlacement (const int newPlacement)
{
    allowablePlacements = newPlacement;
}

//==============================================================================
void BubbleComponent::setPosition (Component* componentToPointTo, int distanceFromTarget, int arrowLength)
{
    jassert (componentToPointTo != nullptr);

    Rectangle<int> target;

    if (Component* p = getParentComponent())
        target = p->getLocalArea (componentToPointTo, componentToPointTo->getLocalBounds());
    else
        target = componentToPointTo->getScreenBounds().transformedBy (getTransform().inverted());

    setPosition (target, distanceFromTarget, arrowLength);
}

void BubbleComponent::setPosition (Point<int> arrowTipPos, int arrowLength)
{
    setPosition (Rectangle<int> (arrowTipPos.x, arrowTipPos.y, 1, 1), arrowLength, arrowLength);
}

void BubbleComponent::setPosition (Rectangle<int> rectangleToPointTo,
                                   int distanceFromTarget, int arrowLength)
{
    {
        int contentW = 150, contentH = 30;
        getContentSize (contentW, contentH);
        content.setBounds (distanceFromTarget, distanceFromTarget, contentW, contentH);
    }

    const int totalW = content.getWidth()  + distanceFromTarget * 2;
    const int totalH = content.getHeight() + distanceFromTarget * 2;

    auto availableSpace = (getParentComponent() != nullptr ? getParentComponent()->getLocalBounds()
                                                           : getParentMonitorArea().transformedBy (getTransform().inverted()));

    int spaceAbove = ((allowablePlacements & above) != 0) ? jmax (0, rectangleToPointTo.getY()  - availableSpace.getY()) : -1;
    int spaceBelow = ((allowablePlacements & below) != 0) ? jmax (0, availableSpace.getBottom() - rectangleToPointTo.getBottom()) : -1;
    int spaceLeft  = ((allowablePlacements & left)  != 0) ? jmax (0, rectangleToPointTo.getX()  - availableSpace.getX()) : -1;
    int spaceRight = ((allowablePlacements & right) != 0) ? jmax (0, availableSpace.getRight()  - rectangleToPointTo.getRight()) : -1;

    // look at whether the component is elongated, and if so, try to position next to its longer dimension.
    if (rectangleToPointTo.getWidth() > rectangleToPointTo.getHeight() * 2
         && (spaceAbove > totalH + 20 || spaceBelow > totalH + 20))
    {
        spaceLeft = spaceRight = 0;
    }
    else if (rectangleToPointTo.getWidth() < rectangleToPointTo.getHeight() / 2
              && (spaceLeft > totalW + 20 || spaceRight > totalW + 20))
    {
        spaceAbove = spaceBelow = 0;
    }

    int targetX, targetY;

    if (jmax (spaceAbove, spaceBelow) >= jmax (spaceLeft, spaceRight))
    {
        targetX = rectangleToPointTo.getCentre().x;
        arrowTip.x = totalW / 2;

        if (spaceAbove >= spaceBelow)
        {
            // above
            targetY = rectangleToPointTo.getY();
            arrowTip.y = content.getBottom() + arrowLength;
        }
        else
        {
            // below
            targetY = rectangleToPointTo.getBottom();
            arrowTip.y = content.getY() - arrowLength;
        }
    }
    else
    {
        targetY = rectangleToPointTo.getCentre().y;
        arrowTip.y = totalH / 2;

        if (spaceLeft > spaceRight)
        {
            // on the left
            targetX = rectangleToPointTo.getX();
            arrowTip.x = content.getRight() + arrowLength;
        }
        else
        {
            // on the right
            targetX = rectangleToPointTo.getRight();
            arrowTip.x = content.getX() - arrowLength;
        }
    }

    setBounds (targetX - arrowTip.x, targetY - arrowTip.y, totalW, totalH);
}

} // namespace juce
