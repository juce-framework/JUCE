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

ResizableCornerComponent::ResizableCornerComponent (Component* const componentToResize,
                                                    ComponentBoundsConstrainer* const constrainer_)
   : component (componentToResize),
     constrainer (constrainer_)
{
    setRepaintsOnMouseActivity (true);
    setMouseCursor (MouseCursor::BottomRightCornerResizeCursor);
}

ResizableCornerComponent::~ResizableCornerComponent()
{
}

//==============================================================================
void ResizableCornerComponent::paint (Graphics& g)
{
    getLookAndFeel()
        .drawCornerResizer (g, getWidth(), getHeight(),
                            isMouseOverOrDragging(),
                            isMouseButtonDown());
}

void ResizableCornerComponent::mouseDown (const MouseEvent&)
{
    if (component == nullptr)
    {
        jassertfalse; // You've deleted the component that this resizer is supposed to be controlling!
        return;
    }

    originalBounds = component->getBounds();

    if (constrainer != nullptr)
        constrainer->resizeStart();
}

void ResizableCornerComponent::mouseDrag (const MouseEvent& e)
{
    if (component == nullptr)
    {
        jassertfalse; // You've deleted the component that this resizer is supposed to be controlling!
        return;
    }

    Rectangle<int> r (originalBounds.withSize (originalBounds.getWidth() + e.getDistanceFromDragStartX(),
                                               originalBounds.getHeight() + e.getDistanceFromDragStartY()));

    if (constrainer != nullptr)
    {
        constrainer->setBoundsForComponent (component, r, false, false, true, true);
    }
    else
    {
        if (Component::Positioner* const pos = component->getPositioner())
            pos->applyNewBounds (r);
        else
            component->setBounds (r);
    }
}

void ResizableCornerComponent::mouseUp (const MouseEvent&)
{
    if (constrainer != nullptr)
        constrainer->resizeEnd();
}

bool ResizableCornerComponent::hitTest (int x, int y)
{
    if (getWidth() <= 0)
        return false;

    const int yAtX = getHeight() - (getHeight() * x / getWidth());

    return y >= yAtX - getHeight() / 4;
}
