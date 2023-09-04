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

ResizableCornerComponent::ResizableCornerComponent (Component* componentToResize,
                                                    ComponentBoundsConstrainer* boundsConstrainer)
   : component (componentToResize),
     constrainer (boundsConstrainer)
{
    setRepaintsOnMouseActivity (true);
    setMouseCursor (MouseCursor::BottomRightCornerResizeCursor);
}

ResizableCornerComponent::~ResizableCornerComponent() = default;

//==============================================================================
void ResizableCornerComponent::paint (Graphics& g)
{
    getLookAndFeel().drawCornerResizer (g, getWidth(), getHeight(),
                                        isMouseOverOrDragging(),
                                        isMouseButtonDown());
}

void ResizableCornerComponent::mouseDown (const MouseEvent& e)
{
    if (component == nullptr)
    {
        jassertfalse; // You've deleted the component that this resizer is supposed to be controlling!
        return;
    }

    originalBounds = component->getBounds();

    using Zone = ResizableBorderComponent::Zone;
    const Zone zone { Zone::bottom | Zone::right };

    if (auto* peer = component->getPeer())
        if (&peer->getComponent() == component)
            peer->startHostManagedResize (peer->globalToLocal (localPointToGlobal (e.getPosition())), zone);

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

    auto r = originalBounds.withSize (originalBounds.getWidth() + e.getDistanceFromDragStartX(),
                                      originalBounds.getHeight() + e.getDistanceFromDragStartY());

    if (constrainer != nullptr)
        constrainer->setBoundsForComponent (component, r, false, false, true, true);
    else if (auto pos = component->getPositioner())
        pos->applyNewBounds (r);
    else
        component->setBounds (r);
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

    auto yAtX = getHeight() - (getHeight() * x / getWidth());
    return y >= yAtX - getHeight() / 4;
}

} // namespace juce
