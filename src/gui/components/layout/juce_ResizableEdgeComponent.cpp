/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

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

#include "juce_ResizableEdgeComponent.h"
#include "../lookandfeel/juce_LookAndFeel.h"


//==============================================================================
ResizableEdgeComponent::ResizableEdgeComponent (Component* const componentToResize,
                                                ComponentBoundsConstrainer* const constrainer_,
                                                Edge edge_)
   : component (componentToResize),
     constrainer (constrainer_),
     edge (edge_)
{
    setRepaintsOnMouseActivity (true);
    setMouseCursor (MouseCursor (isVertical() ? MouseCursor::LeftRightResizeCursor
                                              : MouseCursor::UpDownResizeCursor));
}

ResizableEdgeComponent::~ResizableEdgeComponent()
{
}

//==============================================================================
bool ResizableEdgeComponent::isVertical() const throw()
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
    if (component == 0)
    {
        jassertfalse; // You've deleted the component that this resizer was supposed to be using!
        return;
    }

    originalBounds = component->getBounds();

    if (constrainer != 0)
        constrainer->resizeStart();
}

void ResizableEdgeComponent::mouseDrag (const MouseEvent& e)
{
    if (component == 0)
    {
        jassertfalse; // You've deleted the component that this resizer was supposed to be using!
        return;
    }

    Rectangle<int> bounds (originalBounds);

    switch (edge)
    {
        case leftEdge:      bounds.setLeft (jmin (bounds.getRight(), bounds.getX() + e.getDistanceFromDragStartX())); break;
        case rightEdge:     bounds.setWidth (jmax (0, bounds.getWidth() + e.getDistanceFromDragStartX())); break;
        case topEdge:       bounds.setTop (jmin (bounds.getBottom(), bounds.getY() + e.getDistanceFromDragStartY())); break;
        case bottomEdge:    bounds.setHeight (jmax (0, bounds.getHeight() + e.getDistanceFromDragStartY())); break;
        default:            jassertfalse; break;
    }

    if (constrainer != 0)
    {
        constrainer->setBoundsForComponent (component, bounds,
                                            edge == topEdge,
                                            edge == leftEdge,
                                            edge == bottomEdge,
                                            edge == rightEdge);
    }
    else
    {
        Component::Positioner* const positioner = component->getPositioner();

        if (positioner != 0)
            positioner->applyNewBounds (bounds);
        else
            component->setBounds (bounds);
    }
}

void ResizableEdgeComponent::mouseUp (const MouseEvent&)
{
    if (constrainer != 0)
        constrainer->resizeEnd();
}

END_JUCE_NAMESPACE
