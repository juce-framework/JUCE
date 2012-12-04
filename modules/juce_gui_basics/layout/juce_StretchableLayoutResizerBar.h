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

#ifndef __JUCE_STRETCHABLELAYOUTRESIZERBAR_JUCEHEADER__
#define __JUCE_STRETCHABLELAYOUTRESIZERBAR_JUCEHEADER__

#include "../components/juce_Component.h"
#include "juce_StretchableLayoutManager.h"


//==============================================================================
/**
    A component that acts as one of the vertical or horizontal bars you see being
    used to resize panels in a window.

    One of these acts with a StretchableLayoutManager to resize the other components.

    @see StretchableLayoutManager
*/
class JUCE_API  StretchableLayoutResizerBar  : public Component
{
public:
    //==============================================================================
    /** Creates a resizer bar for use on a specified layout.

        @param layoutToUse          the layout that will be affected when this bar
                                    is dragged
        @param itemIndexInLayout    the item index in the layout that corresponds to
                                    this bar component. You'll need to set up the item
                                    properties in a suitable way for a divider bar, e.g.
                                    for an 8-pixel wide bar which, you could call
                                    myLayout->setItemLayout (barIndex, 8, 8, 8)
        @param isBarVertical        true if it's an upright bar that you drag left and
                                    right; false for a horizontal one that you drag up and
                                    down
    */
    StretchableLayoutResizerBar (StretchableLayoutManager* layoutToUse,
                                 int itemIndexInLayout,
                                 bool isBarVertical);

    /** Destructor. */
    ~StretchableLayoutResizerBar();

    //==============================================================================
    /** This is called when the bar is dragged.

        This method must update the positions of any components whose position is
        determined by the StretchableLayoutManager, because they might have just
        moved.

        The default implementation calls the resized() method of this component's
        parent component, because that's often where you're likely to apply the
        layout, but it can be overridden for more specific needs.
    */
    virtual void hasBeenMoved();

    //==============================================================================
    /** @internal */
    void paint (Graphics& g);
    /** @internal */
    void mouseDown (const MouseEvent& e);
    /** @internal */
    void mouseDrag (const MouseEvent& e);


private:
    //==============================================================================
    StretchableLayoutManager* layout;
    int itemIndex, mouseDownPos;
    bool isVertical;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StretchableLayoutResizerBar)
};


#endif   // __JUCE_STRETCHABLELAYOUTRESIZERBAR_JUCEHEADER__
