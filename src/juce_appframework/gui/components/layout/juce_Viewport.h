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

#ifndef __JUCE_VIEWPORT_JUCEHEADER__
#define __JUCE_VIEWPORT_JUCEHEADER__

#include "juce_ScrollBar.h"


//==============================================================================
/**
    A Viewport is used to contain a larger child component, and allows the child
    to be automatically scrolled around.

    To use a Viewport, just create one and set the component that goes inside it
    using the setViewedComponent() method. When the child component changes size,
    the Viewport will adjust its scrollbars accordingly.

    A subclass of the viewport can be created which will receive calls to its
    visibleAreaChanged() method when the subcomponent changes position or size.

*/
class JUCE_API  Viewport  : public Component,
                            private ComponentListener,
                            private ScrollBarListener
{
public:
    //==============================================================================
    /** Creates a Viewport.

        The viewport is initially empty - use the setViewedComponent() method to
        add a child component for it to manage.
    */
    Viewport (const String& componentName = String::empty);

    /** Destructor. */
    ~Viewport();

    //==============================================================================
    /** Sets the component that this viewport will contain and scroll around.

        This will add the given component to this Viewport and position it at
        (0, 0).

        (Don't add or remove any child components directly using the normal
        Component::addChildComponent() methods).

        @param newViewedComponent       the component to add to this viewport (this pointer
                                        may be null). The component passed in will be deleted
                                        by the Viewport when it's no longer needed
        @see getViewedComponent
    */
    void setViewedComponent (Component* const newViewedComponent);

    /** Returns the component that's currently being used inside the Viewport.

        @see setViewedComponent
    */
    Component* getViewedComponent() const throw()                   { return contentComp; }

    //==============================================================================
    /** Changes the position of the viewed component.

        The inner component will be moved so that the pixel at the top left of
        the viewport will be the pixel at position (xPixelsOffset, yPixelsOffset)
        within the inner component.

        This will update the scrollbars and might cause a call to visibleAreaChanged().

        @see getViewPositionX, getViewPositionY, setViewPositionProportionately
    */
    void setViewPosition (const int xPixelsOffset,
                          const int yPixelsOffset);

    /** Changes the view position as a proportion of the distance it can move.

        The values here are from 0.0 to 1.0 - where (0, 0) would put the
        visible area in the top-left, and (1, 1) would put it as far down and
        to the right as it's possible to go whilst keeping the child component
        on-screen.
    */
    void setViewPositionProportionately (const double proportionX,
                                         const double proportionY);

    /** Returns the position within the child component of the top-left of its visible area.
        @see getViewWidth, setViewPosition
    */
    int getViewPositionX() const throw()                    { return lastVX; }

    /** Returns the position within the child component of the top-left of its visible area.
        @see getViewHeight, setViewPosition
    */
    int getViewPositionY() const throw()                    { return lastVY; }

    /** Returns the width of the visible area of the child component.

        This may be less than the width of this Viewport if there's a vertical scrollbar
        or if the child component is itself smaller.
    */
    int getViewWidth() const throw()                        { return lastVW; }

    /** Returns the height of the visible area of the child component.

        This may be less than the height of this Viewport if there's a horizontal scrollbar
        or if the child component is itself smaller.
    */
    int getViewHeight() const throw()                       { return lastVH; }

    /** Returns the width available within this component for the contents.

        This will be the width of the viewport component minus the width of a
        vertical scrollbar (if visible).
    */
    int getMaximumVisibleWidth() const throw();

    /** Returns the height available within this component for the contents.

        This will be the height of the viewport component minus the space taken up
        by a horizontal scrollbar (if visible).
    */
    int getMaximumVisibleHeight() const throw();

    //==============================================================================
    /** Callback method that is called when the visible area changes.

        This will be called when the visible area is moved either be scrolling or
        by calls to setViewPosition(), etc.
    */
    virtual void visibleAreaChanged (int visibleX, int visibleY,
                                     int visibleW, int visibleH);

    //==============================================================================
    /** Turns scrollbars on or off.

        If set to false, the scrollbars won't ever appear. When true (the default)
        they will appear only when needed.
    */
    void setScrollBarsShown (const bool showVerticalScrollbarIfNeeded,
                             const bool showHorizontalScrollbarIfNeeded);

    /** True if the vertical scrollbar is enabled.
        @see setScrollBarsShown
    */
    bool isVerticalScrollBarShown() const throw()               { return showVScrollbar; }

    /** True if the horizontal scrollbar is enabled.
        @see setScrollBarsShown
    */
    bool isHorizontalScrollBarShown() const throw()             { return showHScrollbar; }

    /** Changes the width of the scrollbars.

        If this isn't specified, the default width from the LookAndFeel class will be used.

        @see LookAndFeel::getDefaultScrollbarWidth
    */
    void setScrollBarThickness (const int thickness);

    /** Returns the thickness of the scrollbars.

        @see setScrollBarThickness
    */
    int getScrollBarThickness() const throw();

    /** Changes the distance that a single-step click on a scrollbar button
        will move the viewport.
    */
    void setSingleStepSizes (const int stepX, const int stepY);

    /** Shows or hides the buttons on any scrollbars that are used.

        @see ScrollBar::setButtonVisibility
    */
    void setScrollBarButtonVisibility (const bool buttonsVisible);

    /** Returns a pointer to the scrollbar component being used.

        Handy if you need to customise the bar somehow.
    */
    ScrollBar* getVerticalScrollBar() const throw()             { return verticalScrollBar; }

    /** Returns a pointer to the scrollbar component being used.

        Handy if you need to customise the bar somehow.
    */
    ScrollBar* getHorizontalScrollBar() const throw()           { return horizontalScrollBar; }


    //==============================================================================
    juce_UseDebuggingNewOperator

    /** @internal */
    void resized();
    /** @internal */
    void scrollBarMoved (ScrollBar* scrollBarThatHasMoved, const double newRangeStart);
    /** @internal */
    void mouseWheelMove (const MouseEvent& e, float wheelIncrementX, float wheelIncrementY);
    /** @internal */
    bool keyPressed (const KeyPress& key);
    /** @internal */
    void componentMovedOrResized (Component& component, bool wasMoved, bool wasResized);
    /** @internal */
    bool useMouseWheelMoveIfNeeded (const MouseEvent& e, float wheelIncrementX, float wheelIncrementY);

private:
    Component* contentComp;
    int lastVX, lastVY, lastVW, lastVH;
    int scrollBarThickness;
    int singleStepX, singleStepY;
    bool showHScrollbar, showVScrollbar;
    Component* contentHolder;
    ScrollBar* verticalScrollBar;
    ScrollBar* horizontalScrollBar;

    void updateVisibleRegion();
    Viewport (const Viewport&);
    const Viewport& operator= (const Viewport&);
};


#endif   // __JUCE_VIEWPORT_JUCEHEADER__
