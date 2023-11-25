/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

ResizableBorderComponent::Zone::Zone() noexcept {}
ResizableBorderComponent::Zone::Zone (int zoneFlags) noexcept  : zone (zoneFlags) {}
ResizableBorderComponent::Zone::Zone (const ResizableBorderComponent::Zone& other) noexcept  : zone (other.zone) {}

ResizableBorderComponent::Zone& ResizableBorderComponent::Zone::operator= (const ResizableBorderComponent::Zone& other) noexcept
{
    zone = other.zone;
    return *this;
}

bool ResizableBorderComponent::Zone::operator== (const ResizableBorderComponent::Zone& other) const noexcept      { return zone == other.zone; }
bool ResizableBorderComponent::Zone::operator!= (const ResizableBorderComponent::Zone& other) const noexcept      { return zone != other.zone; }

ResizableBorderComponent::Zone ResizableBorderComponent::Zone::fromPositionOnBorder (Rectangle<int> totalSize,
                                                                                     BorderSize<int> border,
                                                                                     Point<int> position)
{
    int z = 0;

    if (totalSize.contains (position)
         && ! border.subtractedFrom (totalSize).contains (position))
    {
        auto minW = jmax (totalSize.getWidth() / 10, jmin (10, totalSize.getWidth() / 3));

        if (position.x < jmax (border.getLeft(), minW) && border.getLeft() > 0)
            z |= left;
        else if (position.x >= totalSize.getWidth() - jmax (border.getRight(), minW) && border.getRight() > 0)
            z |= right;

        auto minH = jmax (totalSize.getHeight() / 10, jmin (10, totalSize.getHeight() / 3));

        if (position.y < jmax (border.getTop(), minH) && border.getTop() > 0)
            z |= top;
        else if (position.y >= totalSize.getHeight() - jmax (border.getBottom(), minH) && border.getBottom() > 0)
            z |= bottom;
    }

    return Zone (z);
}

MouseCursor ResizableBorderComponent::Zone::getMouseCursor() const noexcept
{
    auto mc = MouseCursor::NormalCursor;

    switch (zone)
    {
        case (left | top):      mc = MouseCursor::TopLeftCornerResizeCursor; break;
        case top:               mc = MouseCursor::TopEdgeResizeCursor; break;
        case (right | top):     mc = MouseCursor::TopRightCornerResizeCursor; break;
        case left:              mc = MouseCursor::LeftEdgeResizeCursor; break;
        case right:             mc = MouseCursor::RightEdgeResizeCursor; break;
        case (left | bottom):   mc = MouseCursor::BottomLeftCornerResizeCursor; break;
        case bottom:            mc = MouseCursor::BottomEdgeResizeCursor; break;
        case (right | bottom):  mc = MouseCursor::BottomRightCornerResizeCursor; break;
        default:                break;
    }

    return mc;
}

//==============================================================================
ResizableBorderComponent::ResizableBorderComponent (Component* componentToResize,
                                                    ComponentBoundsConstrainer* boundsConstrainer)
   : component (componentToResize),
     constrainer (boundsConstrainer),
     borderSize (5)
{
}

ResizableBorderComponent::~ResizableBorderComponent() = default;

//==============================================================================
void ResizableBorderComponent::paint (Graphics& g)
{
    getLookAndFeel().drawResizableFrame (g, getWidth(), getHeight(), borderSize);
}

void ResizableBorderComponent::mouseEnter (const MouseEvent& e)
{
    updateMouseZone (e);
}

void ResizableBorderComponent::mouseMove (const MouseEvent& e)
{
    updateMouseZone (e);
}

void ResizableBorderComponent::mouseDown (const MouseEvent& e)
{
    if (component == nullptr)
    {
        jassertfalse; // You've deleted the component that this resizer was supposed to be using!
        return;
    }

    updateMouseZone (e);

    originalBounds = component->getBounds();

    if (auto* peer = component->getPeer())
        if (&peer->getComponent() == component)
            peer->startHostManagedResize (peer->globalToLocal (localPointToGlobal (e.getPosition())), mouseZone);

    if (constrainer != nullptr)
        constrainer->resizeStart();
}

void ResizableBorderComponent::mouseDrag (const MouseEvent& e)
{
    if (component == nullptr)
    {
        jassertfalse; // You've deleted the component that this resizer was supposed to be using!
        return;
    }

    auto newBounds = mouseZone.resizeRectangleBy (originalBounds, e.getOffsetFromDragStart());

    if (constrainer != nullptr)
    {
        constrainer->setBoundsForComponent (component, newBounds,
                                            mouseZone.isDraggingTopEdge(),
                                            mouseZone.isDraggingLeftEdge(),
                                            mouseZone.isDraggingBottomEdge(),
                                            mouseZone.isDraggingRightEdge());
    }
    else
    {
        if (auto* p = component->getPositioner())
            p->applyNewBounds (newBounds);
        else
            component->setBounds (newBounds);
    }
}

void ResizableBorderComponent::mouseUp (const MouseEvent&)
{
    if (constrainer != nullptr)
        constrainer->resizeEnd();
}

bool ResizableBorderComponent::hitTest (int x, int y)
{
    return ! borderSize.subtractedFrom (getLocalBounds()).contains (x, y);
}

void ResizableBorderComponent::setBorderThickness (BorderSize<int> newBorderSize)
{
    if (borderSize != newBorderSize)
    {
        borderSize = newBorderSize;
        repaint();
    }
}

BorderSize<int> ResizableBorderComponent::getBorderThickness() const
{
    return borderSize;
}

void ResizableBorderComponent::updateMouseZone (const MouseEvent& e)
{
    auto newZone = Zone::fromPositionOnBorder (getLocalBounds(), borderSize, e.getPosition());

    if (mouseZone != newZone)
    {
        mouseZone = newZone;
        setMouseCursor (newZone.getMouseCursor());
    }
}

} // namespace juce
