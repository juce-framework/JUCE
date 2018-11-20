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

Displays::Displays (Desktop& desktop)
{
    init (desktop);
}

void Displays::init (Desktop& desktop)
{
    findDisplays (desktop.getGlobalScaleFactor());
}

const Displays::Display& Displays::findDisplayForRect (Rectangle<int> rect, bool isPhysical) const noexcept
{
    int maxArea = -1;
    Display* retVal = nullptr;

    for (auto& display : displays)
    {
        auto displayArea = display.totalArea;

        if (isPhysical)
            displayArea = (displayArea.withZeroOrigin() * display.scale) + display.topLeftPhysical;

        displayArea = displayArea.getIntersection (rect);
        auto area = displayArea.getWidth() * displayArea.getHeight();

        if (area >= maxArea)
        {
            maxArea = area;
            retVal = &display;
        }
    }

    return *retVal;
}

const Displays::Display& Displays::findDisplayForPoint (Point<int> point, bool isPhysical) const noexcept
{
    auto minDistance = std::numeric_limits<int>::max();
    Display* retVal = nullptr;

    for (auto& display : displays)
    {
        auto displayArea = display.totalArea;

        if (isPhysical)
            displayArea = (displayArea.withZeroOrigin() * display.scale) + display.topLeftPhysical;

        if (displayArea.contains (point))
            return display;

        auto distance = displayArea.getCentre().getDistanceFrom (point);
        if (distance <= minDistance)
        {
            minDistance = distance;
            retVal = &display;
        }
    }

    return *retVal;
}

Rectangle<int> Displays::physicalToLogical (Rectangle<int> rect, const Display* useScaleFactorOfDisplay) const noexcept
{
    auto& display = useScaleFactorOfDisplay != nullptr ? *useScaleFactorOfDisplay
                                                       : findDisplayForRect (rect, true);

    auto globalScale = Desktop::getInstance().getGlobalScaleFactor();

    return ((rect.toFloat() - display.topLeftPhysical.toFloat()) / (display.scale / globalScale)).toNearestInt() + (display.totalArea.getTopLeft() * globalScale);
}

Rectangle<int> Displays::logicalToPhysical (Rectangle<int> rect, const Display* useScaleFactorOfDisplay) const noexcept
{
    auto& display = useScaleFactorOfDisplay != nullptr ? *useScaleFactorOfDisplay
                                                       : findDisplayForRect (rect, false);

    auto globalScale = Desktop::getInstance().getGlobalScaleFactor();

    return ((rect.toFloat() - (display.totalArea.getTopLeft().toFloat() * globalScale)) * (display.scale / globalScale)).toNearestInt() + display.topLeftPhysical;
}

template <typename ValueType>
Point<ValueType> Displays::physicalToLogical (Point<ValueType> point, const Display* useScaleFactorOfDisplay) const noexcept
{
    auto& display = useScaleFactorOfDisplay != nullptr ? *useScaleFactorOfDisplay
                                                       : findDisplayForPoint (point.roundToInt(), true);

    auto globalScale = Desktop::getInstance().getGlobalScaleFactor();

    Point<ValueType> logicalTopLeft  (display.totalArea.getX(),       display.totalArea.getY());
    Point<ValueType> physicalTopLeft (display.topLeftPhysical.getX(), display.topLeftPhysical.getY());

    return ((point - physicalTopLeft) / (display.scale / globalScale)) + (logicalTopLeft * globalScale);
}

template <typename ValueType>
Point<ValueType> Displays::logicalToPhysical (Point<ValueType> point, const Display* useScaleFactorOfDisplay)  const noexcept
{
    auto& display = useScaleFactorOfDisplay != nullptr ? *useScaleFactorOfDisplay
                                                       : findDisplayForPoint (point.roundToInt(), false);

    auto globalScale = Desktop::getInstance().getGlobalScaleFactor();

    Point<ValueType> logicalTopLeft  (display.totalArea.getX(),       display.totalArea.getY());
    Point<ValueType> physicalTopLeft (display.topLeftPhysical.getX(), display.topLeftPhysical.getY());

    return ((point - (logicalTopLeft * globalScale)) * (display.scale / globalScale)) + physicalTopLeft;
}

const Displays::Display& Displays::getMainDisplay() const noexcept
{
    JUCE_ASSERT_MESSAGE_MANAGER_IS_LOCKED

    for (auto& d : displays)
        if (d.isMain)
            return d;

    // no main display!
    jassertfalse;
    return displays.getReference (0);
}

RectangleList<int> Displays::getRectangleList (bool userAreasOnly) const
{
    JUCE_ASSERT_MESSAGE_MANAGER_IS_LOCKED
    RectangleList<int> rl;

    for (auto& d : displays)
        rl.addWithoutMerging (userAreasOnly ? d.userArea : d.totalArea);

    return rl;
}

Rectangle<int> Displays::getTotalBounds (bool userAreasOnly) const
{
    return getRectangleList (userAreasOnly).getBounds();
}

void Displays::refresh()
{
    Array<Display> oldDisplays;
    oldDisplays.swapWith (displays);

    init (Desktop::getInstance());

    if (oldDisplays != displays)
    {
        for (auto i = ComponentPeer::getNumPeers(); --i >= 0;)
            if (auto* peer = ComponentPeer::getPeer (i))
                peer->handleScreenSizeChange();
    }
}

bool operator== (const Displays::Display& d1, const Displays::Display& d2) noexcept;
bool operator== (const Displays::Display& d1, const Displays::Display& d2) noexcept
{
    return d1.isMain          == d2.isMain
        && d1.totalArea       == d2.totalArea
        && d1.userArea        == d2.userArea
        && d1.topLeftPhysical == d2.topLeftPhysical
        && d1.scale           == d2.scale
        && d1.dpi             == d2.dpi;
}

bool operator!= (const Displays::Display& d1, const Displays::Display& d2) noexcept;
bool operator!= (const Displays::Display& d1, const Displays::Display& d2) noexcept    { return ! (d1 == d2); }

// Deprecated method
const Displays::Display& Displays::getDisplayContaining (Point<int> position) const noexcept
{
    JUCE_ASSERT_MESSAGE_MANAGER_IS_LOCKED
    auto* best = &displays.getReference (0);
    auto bestDistance = std::numeric_limits<int>::max();

    for (auto& d : displays)
    {
        if (d.totalArea.contains (position))
        {
            best = &d;
            break;
        }

        auto distance = d.totalArea.getCentre().getDistanceFrom (position);

        if (distance < bestDistance)
        {
            bestDistance = distance;
            best = &d;
        }
    }

    return *best;
}

//==============================================================================
// These methods are used for converting the totalArea and userArea Rectangles in Display from physical to logical
// pixels. We do this by constructing a graph of connected displays where the root node has position (0, 0); this can be
// safely converted to logical pixels using its scale factor and we can then traverse the graph and work out the logical pixels
// for all the other connected displays. We need to do this as the logical bounds of a display depend not only on its scale
// factor but also the scale factor of the displays connected to it.

/**
    Represents a node in our graph of displays.
*/
struct DisplayNode
{
    /** The Display object that this represents. */
    Displays::Display* display;

    /** True if this represents the 'root' display with position (0, 0). */
    bool isRoot = false;

    /** The parent node of this node in our display graph. This will have a correct logicalArea. */
    DisplayNode* parent = nullptr;

    /** The logical area to be calculated. This will be valid after processDisplay() has
        been called on this node.
    */
    Rectangle<double> logicalArea;
};

/** Recursive - will calculate and set the logicalArea member of current. */
static void processDisplay (DisplayNode* currentNode, const Array<DisplayNode>& allNodes)
{
    const auto physicalArea = currentNode->display->totalArea.toDouble();
    const auto scale = currentNode->display->scale;

    if (! currentNode->isRoot)
    {
        const auto logicalWidth  = physicalArea.getWidth() / scale;
        const auto logicalHeight = physicalArea.getHeight() / scale;

        const auto physicalParentArea = currentNode->parent->display->totalArea.toDouble();
        const auto logicalParentArea  = currentNode->parent->logicalArea; // logical area of parent has already been calculated
        const auto parentScale        = currentNode->parent->display->scale;

        Rectangle<double> logicalArea (0.0, 0.0, logicalWidth, logicalHeight);

        if      (physicalArea.getRight() == physicalParentArea.getX())     logicalArea.setPosition ({ logicalParentArea.getX() - logicalWidth, physicalArea.getY() / parentScale });  // on left
        else if (physicalArea.getX() == physicalParentArea.getRight())     logicalArea.setPosition ({ logicalParentArea.getRight(),  physicalArea.getY() / parentScale });            // on right
        else if (physicalArea.getBottom() == physicalParentArea.getY())    logicalArea.setPosition ({ physicalArea.getX() / parentScale, logicalParentArea.getY() - logicalHeight }); // on top
        else if (physicalArea.getY() == physicalParentArea.getBottom())    logicalArea.setPosition ({ physicalArea.getX() / parentScale, logicalParentArea.getBottom() });            // on bottom
        else                                                               jassertfalse;

        currentNode->logicalArea = logicalArea;
    }
    else
    {
        // If currentNode is the root (position (0, 0)) then we can just scale the physical area
        currentNode->logicalArea = physicalArea / scale;
        currentNode->parent = currentNode;
    }

    // Find child nodes
    Array<DisplayNode*> children;
    for (auto& node : allNodes)
    {
        // Already calculated
        if (node.parent != nullptr)
            continue;

        const auto otherPhysicalArea = node.display->totalArea.toDouble();

        // If the displays are touching on any side
        if (otherPhysicalArea.getX() == physicalArea.getRight()  || otherPhysicalArea.getRight() == physicalArea.getX()
            || otherPhysicalArea.getY() == physicalArea.getBottom() || otherPhysicalArea.getBottom() == physicalArea.getY())
        {
            node.parent = currentNode;
            children.add (&node);
        }
    }

    // Recursively process all child nodes
    for (auto child : children)
        processDisplay (child, allNodes);
}

/** This is called when the displays Array has been filled out with the info for all connected displays and the
    totalArea and userArea Rectangles need to be converted from physical to logical coordinates.
*/
void Displays::updateToLogical()
{
    if (displays.size() == 1)
    {
        auto& display = displays.getReference (0);

        display.totalArea = (display.totalArea.toDouble() / display.scale).toNearestInt();
        display.userArea  = (display.userArea.toDouble()  / display.scale).toNearestInt();

        return;
    }

    Array<DisplayNode> displayNodes;

    for (auto& d : displays)
    {
        DisplayNode node;

        node.display = &d;
        displayNodes.add (node);
    }

    DisplayNode* root = nullptr;
    for (auto& node : displayNodes)
    {
        if (node.display->totalArea.getTopLeft() == Point<int>())
        {
            root = &node;
            root->isRoot = true;
            break;
        }
    }

    // Must have a root node!
    jassert (root != nullptr);

    // Recursively traverse the display graph from the root and work out logical bounds
    processDisplay (root, displayNodes);

    for (auto& node : displayNodes)
    {
        // All of the nodes should have a parent
        jassert (node.parent != nullptr);

        auto relativeUserArea = (node.display->userArea.toDouble() - node.display->totalArea.toDouble().getTopLeft()) / node.display->scale;

        // Now set Display::totalArea and ::userArea using the logical area that we have calculated
        node.display->topLeftPhysical = node.display->totalArea.getTopLeft();
        node.display->totalArea       = node.logicalArea.toNearestInt();
        node.display->userArea        = (relativeUserArea + node.logicalArea.getTopLeft()).toNearestInt();
    }
}

} // namespace juce
