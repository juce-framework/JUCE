/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

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

ResizableEdgeComponent::ResizableEdgeComponent (Component* const componentToResize,
                                                ComponentBoundsConstrainer* const constrainer_,
                                                Edge edge_)
   : component (componentToResize),
     constrainer (constrainer_),
     edge (edge_)
{
    setRepaintsOnMouseActivity (true);
    setMouseCursor (isVertical() ? MouseCursor::LeftRightResizeCursor
                                 : MouseCursor::UpDownResizeCursor);
}

ResizableEdgeComponent::~ResizableEdgeComponent()
{
}

//==============================================================================
bool ResizableEdgeComponent::isVertical() const noexcept
{
    return edge == leftEdge || edge == rightEdge;
}

void ResizableEdgeComponent::paint (Graphics& g)
{
    getLookAndFeel().drawStretchableLayoutResizerBar (g, getWidth(), getHeight(), isVertical(),
                                                      isMouseOver(), isMouseButtonDown());
}

void ResizableEdgeComponent::mouseDown (const MouseEvent&)
{
    if (component == nullptr)
    {
        jassertfalse; // You've deleted the component that this resizer was supposed to be using!
        return;
    }

    originalBounds = component->getBounds();

    if (constrainer != nullptr)
        constrainer->resizeStart();
}

void ResizableEdgeComponent::mouseDrag (const MouseEvent& e)
{
    if (component == nullptr)
    {
        jassertfalse; // You've deleted the component that this resizer was supposed to be using!
        return;
    }

    Rectangle<int> newBounds (originalBounds);

    switch (edge)
    {
        case leftEdge:      newBounds.setLeft (jmin (newBounds.getRight(), newBounds.getX() + e.getDistanceFromDragStartX())); break;
        case rightEdge:     newBounds.setWidth (jmax (0, newBounds.getWidth() + e.getDistanceFromDragStartX())); break;
        case topEdge:       newBounds.setTop (jmin (newBounds.getBottom(), newBounds.getY() + e.getDistanceFromDragStartY())); break;
        case bottomEdge:    newBounds.setHeight (jmax (0, newBounds.getHeight() + e.getDistanceFromDragStartY())); break;
        default:            jassertfalse; break;
    }

    if (constrainer != nullptr)
    {
        constrainer->setBoundsForComponent (component, newBounds,
                                            edge == topEdge,
                                            edge == leftEdge,
                                            edge == bottomEdge,
                                            edge == rightEdge);
    }
    else
    {
        if (Component::Positioner* const pos = component->getPositioner())
            pos->applyNewBounds (newBounds);
        else
            component->setBounds (newBounds);
    }
}

void ResizableEdgeComponent::mouseUp (const MouseEvent&)
{
    if (constrainer != nullptr)
        constrainer->resizeEnd();
}
