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

} // namespace juce
