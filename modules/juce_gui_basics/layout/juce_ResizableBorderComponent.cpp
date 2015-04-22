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

ResizableBorderComponent::Zone::Zone() noexcept
    : zone (0)
{}

ResizableBorderComponent::Zone::Zone (const int zoneFlags) noexcept
    : zone (zoneFlags)
{}

ResizableBorderComponent::Zone::Zone (const ResizableBorderComponent::Zone& other) noexcept
    : zone (other.zone)
{}

ResizableBorderComponent::Zone& ResizableBorderComponent::Zone::operator= (const ResizableBorderComponent::Zone& other) noexcept
{
    zone = other.zone;
    return *this;
}

bool ResizableBorderComponent::Zone::operator== (const ResizableBorderComponent::Zone& other) const noexcept      { return zone == other.zone; }
bool ResizableBorderComponent::Zone::operator!= (const ResizableBorderComponent::Zone& other) const noexcept      { return zone != other.zone; }

ResizableBorderComponent::Zone ResizableBorderComponent::Zone::fromPositionOnBorder (const Rectangle<int>& totalSize,
                                                                                     const BorderSize<int>& border,
                                                                                     Point<int> position)
{
    int z = 0;

    if (totalSize.contains (position)
         && ! border.subtractedFrom (totalSize).contains (position))
    {
        const int minW = jmax (totalSize.getWidth() / 10, jmin (10, totalSize.getWidth() / 3));
        if (position.x < jmax (border.getLeft(), minW) && border.getLeft() > 0)
            z |= left;
        else if (position.x >= totalSize.getWidth() - jmax (border.getRight(), minW) && border.getRight() > 0)
            z |= right;

        const int minH = jmax (totalSize.getHeight() / 10, jmin (10, totalSize.getHeight() / 3));
        if (position.y < jmax (border.getTop(), minH) && border.getTop() > 0)
            z |= top;
        else if (position.y >= totalSize.getHeight() - jmax (border.getBottom(), minH) && border.getBottom() > 0)
            z |= bottom;
    }

    return Zone (z);
}

MouseCursor ResizableBorderComponent::Zone::getMouseCursor() const noexcept
{
    MouseCursor::StandardCursorType mc = MouseCursor::NormalCursor;

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
ResizableBorderComponent::ResizableBorderComponent (Component* const componentToResize,
                                                    ComponentBoundsConstrainer* const constrainer_)
   : component (componentToResize),
     constrainer (constrainer_),
     borderSize (5),
     mouseZone (0)
{
}

ResizableBorderComponent::~ResizableBorderComponent()
{
}

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

    const Rectangle<int> newBounds (mouseZone.resizeRectangleBy (originalBounds, e.getOffsetFromDragStart()));

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
        if (Component::Positioner* const pos = component->getPositioner())
            pos->applyNewBounds (newBounds);
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
    return x < borderSize.getLeft()
            || x >= getWidth() - borderSize.getRight()
            || y < borderSize.getTop()
            || y >= getHeight() - borderSize.getBottom();
}

void ResizableBorderComponent::setBorderThickness (const BorderSize<int>& newBorderSize)
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
    Zone newZone (Zone::fromPositionOnBorder (getLocalBounds(), borderSize, e.getPosition()));

    if (mouseZone != newZone)
    {
        mouseZone = newZone;
        setMouseCursor (newZone.getMouseCursor());
    }
}
