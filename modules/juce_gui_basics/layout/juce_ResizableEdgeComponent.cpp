/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
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

ResizableEdgeComponent::ResizableEdgeComponent (Component* componentToResize,
                                                ComponentBoundsConstrainer* boundsConstrainer,
                                                Edge e)
   : component (componentToResize),
     constrainer (boundsConstrainer),
     edge (e)
{
    setRepaintsOnMouseActivity (true);
    setMouseCursor (isVertical() ? MouseCursor::LeftRightResizeCursor
                                 : MouseCursor::UpDownResizeCursor);
}

ResizableEdgeComponent::~ResizableEdgeComponent() = default;

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

    auto newBounds = originalBounds;

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
        if (auto* p = component->getPositioner())
            p->applyNewBounds (newBounds);
        else
            component->setBounds (newBounds);
    }
}

void ResizableEdgeComponent::mouseUp (const MouseEvent&)
{
    if (constrainer != nullptr)
        constrainer->resizeEnd();
}

} // namespace juce
