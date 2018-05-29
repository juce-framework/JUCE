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

BubbleComponent::BubbleComponent()
  : allowablePlacements (above | below | left | right)
{
    setInterceptsMouseClicks (false, false);

    shadow.setShadowProperties (DropShadow (Colours::black.withAlpha (0.35f), 5, Point<int>()));
    setComponentEffect (&shadow);
}

BubbleComponent::~BubbleComponent() {}

//==============================================================================
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
        target = componentToPointTo->getScreenBounds();

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

    const Rectangle<int> availableSpace (getParentComponent() != nullptr ? getParentComponent()->getLocalBounds()
                                                                         : getParentMonitorArea());

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

    auto origin = Point<int> (targetX - arrowTip.x, targetY - arrowTip.y).transformedBy (getTransform().inverted());
    setBounds (origin.getX(), origin.getY(), totalW, totalH);
}

} // namespace juce
