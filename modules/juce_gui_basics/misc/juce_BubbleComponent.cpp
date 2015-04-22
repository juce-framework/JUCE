/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

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

void BubbleComponent::setPosition (Component* componentToPointTo)
{
    jassert (componentToPointTo != nullptr);

    if (Component* p = getParentComponent())
        setPosition (p->getLocalArea (componentToPointTo, componentToPointTo->getLocalBounds()));
    else
        setPosition (componentToPointTo->getScreenBounds());
}

void BubbleComponent::setPosition (Point<int> pos)
{
    setPosition (Rectangle<int> (pos.x, pos.y, 1, 1));
}

//==============================================================================
void BubbleComponent::setPosition (const Rectangle<int>& rectangleToPointTo)
{
    const int edgeSpace = 15;
    const int arrowLength = 10;

    {
        int contentW = 150, contentH = 30;
        getContentSize (contentW, contentH);
        content.setBounds (edgeSpace, edgeSpace, contentW, contentH);
    }

    const int totalW = content.getWidth()  + edgeSpace * 2;
    const int totalH = content.getHeight() + edgeSpace * 2;

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

    setBounds (targetX - arrowTip.x, targetY - arrowTip.y, totalW, totalH);
}
