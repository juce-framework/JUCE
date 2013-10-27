/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

#ifndef JUCE_STRETCHABLELAYOUTRESIZERBAR_H_INCLUDED
#define JUCE_STRETCHABLELAYOUTRESIZERBAR_H_INCLUDED


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
    /** This abstract base class is implemented by LookAndFeel classes. */
    struct JUCE_API  LookAndFeelMethods
    {
        virtual ~LookAndFeelMethods() {}

        virtual void drawStretchableLayoutResizerBar (Graphics&, int w, int h,
                                                      bool isVerticalBar, bool isMouseOver, bool isMouseDragging) = 0;
    };

    //==============================================================================
    /** @internal */
    void paint (Graphics&) override;
    /** @internal */
    void mouseDown (const MouseEvent&) override;
    /** @internal */
    void mouseDrag (const MouseEvent&) override;


private:
    //==============================================================================
    StretchableLayoutManager* layout;
    int itemIndex, mouseDownPos;
    bool isVertical;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StretchableLayoutResizerBar)
};


#endif   // JUCE_STRETCHABLELAYOUTRESIZERBAR_H_INCLUDED
