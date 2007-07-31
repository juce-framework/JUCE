/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#include "../../../../juce_core/basics/juce_StandardHeader.h"

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
