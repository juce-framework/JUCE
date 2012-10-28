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

StretchableLayoutResizerBar::StretchableLayoutResizerBar (StretchableLayoutManager* layout_,
                                                          const int index,
                                                          const bool vertical)
    : layout (layout_),
      itemIndex (index),
      isVertical (vertical)
{
    setRepaintsOnMouseActivity (true);
    setMouseCursor (vertical ? MouseCursor::LeftRightResizeCursor
                             : MouseCursor::UpDownResizeCursor);
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


    if (layout->getItemCurrentPosition (itemIndex) != desiredPos)
    {
        layout->setItemPosition (itemIndex, desiredPos);
        hasBeenMoved();
    }
}

void StretchableLayoutResizerBar::hasBeenMoved()
{
    if (Component* parent = getParentComponent())
        parent->resized();
}
