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

#include "juce_StretchableLayoutResizerBar.h"
#include "../lookandfeel/juce_LookAndFeel.h"


//==============================================================================
StretchableLayoutResizerBar::StretchableLayoutResizerBar (StretchableLayoutManager* layout_,
                                                          const int itemIndex_,
                                                          const bool isVertical_)
    : layout (layout_),
      itemIndex (itemIndex_),
      isVertical (isVertical_)
{
    setRepaintsOnMouseActivity (true);
    setMouseCursor (MouseCursor (isVertical_ ? MouseCursor::LeftRightResizeCursor
                                                : MouseCursor::UpDownResizeCursor));
}

StretchableLayoutResizerBar::~StretchableLayoutResizerBar()
{
}

//==============================================================================
void StretchableLayoutResizerBar::paint (Graphics& g)
{
    getLookAndFeel().drawStretchableLayoutResizerBar (g,
                                                      getWidth(), getHeight(),
                                                      isVertical,
                                                      isMouseOver(),
                                                      isMouseButtonDown());
}

void StretchableLayoutResizerBar::mouseDown (const MouseEvent&)
{
    mouseDownPos = layout->getItemCurrentPosition (itemIndex);
}

void StretchableLayoutResizerBar::mouseDrag (const MouseEvent& e)
{
    const int desiredPos = mouseDownPos + (isVertical ? e.getDistanceFromDragStartX()
                                                      : e.getDistanceFromDragStartY());

    layout->setItemPosition (itemIndex, desiredPos);

    hasBeenMoved();
}

void StretchableLayoutResizerBar::hasBeenMoved()
{
    if (getParentComponent() != 0)
        getParentComponent()->resized();
}

END_JUCE_NAMESPACE
