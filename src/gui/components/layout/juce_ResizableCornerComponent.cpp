/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

#include "juce_ResizableCornerComponent.h"
#include "../juce_Desktop.h"
#include "../lookandfeel/juce_LookAndFeel.h"
#include "../../graphics/geometry/juce_Line.h"


//==============================================================================
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
    if (component->isValidComponent())
    {
        originalX = component->getX();
        originalY = component->getY();
        originalW = component->getWidth();
        originalH = component->getHeight();

        if (constrainer != 0)
            constrainer->resizeStart();
    }
    else
    {
        jassertfalse
    }
}

void ResizableCornerComponent::mouseDrag (const MouseEvent& e)
{
    if (! component->isValidComponent())
    {
        jassertfalse
        return;
    }

    int x = originalX;
    int y = originalY;
    int w = originalW + e.getDistanceFromDragStartX();
    int h = originalH + e.getDistanceFromDragStartY();

    if (constrainer != 0)
        constrainer->setBoundsForComponent (component, x, y, w, h,
                                            false, false, true, true);
    else
        component->setBounds (x, y, w, h);
}

void ResizableCornerComponent::mouseUp (const MouseEvent&)
{
    if (constrainer != 0)
        constrainer->resizeStart();
}

bool ResizableCornerComponent::hitTest (int x, int y)
{
    if (getWidth() <= 0)
        return false;

    const int yAtX = getHeight() - (getHeight() * x / getWidth());

    return y >= yAtX - getHeight() / 4;
}


END_JUCE_NAMESPACE
