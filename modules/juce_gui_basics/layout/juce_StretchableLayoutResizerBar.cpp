/*
  ==============================================================================

   This file is part of the JUCE 7 technical preview.
   Copyright (c) 2022 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For the technical preview this file cannot be licensed commercially.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

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

} // namespace juce
