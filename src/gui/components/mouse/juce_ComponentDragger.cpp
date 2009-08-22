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

#include "juce_ComponentDragger.h"
#include "../juce_Component.h"


//==============================================================================
ComponentDragger::ComponentDragger()
    : constrainer (0),
      originalX (0),
      originalY (0)
{
}

ComponentDragger::~ComponentDragger()
{
}

//==============================================================================
void ComponentDragger::startDraggingComponent (Component* const componentToDrag,
                                               ComponentBoundsConstrainer* const constrainer_)
{
    jassert (componentToDrag->isValidComponent());

    if (componentToDrag->isValidComponent())
    {
        constrainer = constrainer_;
        originalX = 0;
        originalY = 0;
        componentToDrag->relativePositionToGlobal (originalX, originalY);
    }
}

void ComponentDragger::dragComponent (Component* const componentToDrag, const MouseEvent& e)
{
    jassert (componentToDrag->isValidComponent());
    jassert (e.mods.isAnyMouseButtonDown()); // (the event has to be a drag event..)

    if (componentToDrag->isValidComponent())
    {
        int x = originalX;
        int y = originalY;
        int w = componentToDrag->getWidth();
        int h = componentToDrag->getHeight();

        const Component* const parentComp = componentToDrag->getParentComponent();
        if (parentComp != 0)
            parentComp->globalPositionToRelative (x, y);

        x += e.getDistanceFromDragStartX();
        y += e.getDistanceFromDragStartY();

        if (constrainer != 0)
            constrainer->setBoundsForComponent (componentToDrag, x, y, w, h,
                                                false, false, false, false);
        else
            componentToDrag->setBounds (x, y, w, h);
    }
}


END_JUCE_NAMESPACE
