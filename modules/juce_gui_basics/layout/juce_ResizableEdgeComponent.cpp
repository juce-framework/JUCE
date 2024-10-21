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

void ResizableEdgeComponent::mouseDown (const MouseEvent& e)
{
    if (component == nullptr)
    {
        jassertfalse; // You've deleted the component that this resizer was supposed to be using!
        return;
    }

    originalBounds = component->getBounds();

    using Zone = ResizableBorderComponent::Zone;

    const Zone zone { [&]
    {
        switch (edge)
        {
            case Edge::leftEdge:    return Zone::left;
            case Edge::rightEdge:   return Zone::right;
            case Edge::topEdge:     return Zone::top;
            case Edge::bottomEdge:  return Zone::bottom;
        }

        return Zone::centre;
    }() };

    if (auto* peer = component->getPeer())
        if (&peer->getComponent() == component)
            peer->startHostManagedResize (peer->globalToLocal (localPointToGlobal (e.getPosition())), zone);

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
