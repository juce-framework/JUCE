/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

//==============================================================================
/**
    A Viewport is used to contain a larger child component, and allows the child
    to be automatically scrolled around.

    To use a Viewport, just create one and set the component that goes inside it
    using the setViewedComponent() method. When the child component changes size,
    the Viewport will adjust its scrollbars accordingly.

    A subclass of the viewport can be created which will receive calls to its
    visibleAreaChanged() method when the subcomponent changes position or size.


    @tags{GUI}
*/
class JUCE_API  Viewport  : public Component,
                            private ComponentListener,
                            private ScrollBar::Listener
{
public:
    //==============================================================================
    /** Creates a Viewport.

        The viewport is initially empty - use the setViewedComponent() method to
        add a child component for it to manage.
    */
    explicit Viewport (const String& componentName = String());

    /** Destructor. */
    virtual ~Viewport() override;

    //==============================================================================
    /** Sets the component that this viewport will contain and scroll around.

        This will add the given component to this Viewport and position it at (0, 0).

        (Don't add or remove any child components directly using the normal
        Component::addChildComponent() methods).

        @param newViewedComponent   the component to add to this viewport, or null to remove
                                    the current component.
        @param deleteComponentWhenNoLongerNeeded    if true, the component will be deleted
                                    automatically when the viewport is deleted or when a different
                                    component is added. If false, the caller must manage the lifetime
                                    of the component
        @see getViewedComponent
    */
    virtual void setViewedComponent (Component* newViewedComponent,
                             bool deleteComponentWhenNoLongerNeeded = true);

    /** Returns the component that's currently being used inside the Viewport.

        @see setViewedComponent
    */
    virtual Component* getViewedComponent() const noexcept                  { return contentComp.get(); }

    //==============================================================================
    /** Changes the position of the viewed component.

        The inner component will be moved so that the pixel at the top left of
        the viewport will be the pixel at position (xPixelsOffset, yPixelsOffset)
        within the inner component.

        This will update the scrollbars and might cause a call to visibleAreaChanged().

        @see getViewPositionX, getViewPositionY, setViewPositionProportionately
    */
    virtual void setViewPosition (int xPixelsOffset, int yPixelsOffset);

    /** Changes the position of the viewed component.

        The inner component will be moved so that the pixel at the top left of
        the viewport will be the pixel at the specified coordinates within the
        inner component.

        This will update the scrollbars and might cause a call to visibleAreaChanged().

        @see getViewPositionX, getViewPositionY, setViewPositionProportionately
    */
    virtual void setViewPosition (Point<int> newPosition);

    /** Changes the view position as a proportion of the distance it can move.

        The values here are from 0.0 to 1.0 - where (0, 0) would put the
        visible area in the top-left, and (1, 1) would put it as far down and
        to the right as it's possible to go whilst keeping the child component
        on-screen.
    */
    virtual void setViewPositionProportionately (double proportionX, double proportionY);

    /** If the specified position is at the edges of the viewport, this method scrolls
        the viewport to bring that position nearer to the centre.

        Call this if you're dragging an object inside a viewport and want to make it scroll
        when the user approaches an edge. You might also find Component::beginDragAutoRepeat()
        useful when auto-scrolling.

        @param mouseX       the x position, relative to the Viewport's top-left
        @param mouseY       the y position, relative to the Viewport's top-left
        @param distanceFromEdge     specifies how close to an edge the position needs to be
                            before the viewport should scroll in that direction
        @param maximumSpeed the maximum number of pixels that the viewport is allowed
                            to scroll by.
        @returns            true if the viewport was scrolled
    */
    virtual bool autoScroll (int mouseX, int mouseY, int distanceFromEdge, int maximumSpeed);

    /** Returns the position within the child component of the top-left of its visible area. */
    virtual Point<int> getViewPosition() const noexcept             { return lastVisibleArea.getPosition(); }

    /** Returns the visible area of the child component, relative to its top-left */
    virtual Rectangle<int> getViewArea() const noexcept             { return lastVisibleArea; }

    /** Returns the position within the child component of the top-left of its visible area.
        @see getViewWidth, setViewPosition
    */
    virtual int getViewPositionX() const noexcept                   { return lastVisibleArea.getX(); }

    /** Returns the position within the child component of the top-left of its visible area.
        @see getViewHeight, setViewPosition
    */
    virtual int getViewPositionY() const noexcept                   { return lastVisibleArea.getY(); }

    /** Returns the width of the visible area of the child component.

        This may be less than the width of this Viewport if there's a vertical scrollbar
        or if the child component is itself smaller.
    */
    virtual int getViewWidth() const noexcept                       { return lastVisibleArea.getWidth(); }

    /** Returns the height of the visible area of the child component.

        This may be less than the height of this Viewport if there's a horizontal scrollbar
        or if the child component is itself smaller.
    */
    virtual int getViewHeight() const noexcept                      { return lastVisibleArea.getHeight(); }

    /** Returns the width available within this component for the contents.

        This will be the width of the viewport component minus the width of a
        vertical scrollbar (if visible).
    */
    virtual int getMaximumVisibleWidth() const;

    /** Returns the height available within this component for the contents.

        This will be the height of the viewport component minus the space taken up
        by a horizontal scrollbar (if visible).
    */
    virtual int getMaximumVisibleHeight() const;

    //==============================================================================
    /** Callback method that is called when the visible area changes.

        This will be called when the visible area is moved either be scrolling or
        by calls to setViewPosition(), etc.
    */
    virtual void visibleAreaChanged (const Rectangle<int>& newVisibleArea);

    /** Callback method that is called when the viewed component is added, removed or swapped. */
    virtual void viewedComponentChanged (Component* newComponent);

    //==============================================================================
    /** Turns scrollbars on or off.

        If set to false, the scrollbars won't ever appear. When true (the default)
        they will appear only when needed.

        The allowVerticalScrollingWithoutScrollbar parameters allow you to enable
        mouse-wheel scrolling even when there the scrollbars are hidden. When the
        scrollbars are visible, these parameters are ignored.
    */
    virtual void setScrollBarsShown (bool showVerticalScrollbarIfNeeded,
                             bool showHorizontalScrollbarIfNeeded,
                             bool allowVerticalScrollingWithoutScrollbar = false,
                             bool allowHorizontalScrollingWithoutScrollbar = false);

    /** Changes where the scroll bars are positioned

        If verticalScrollbarOnRight is set to true, then the vertical scrollbar will
        appear on the right side of the view port's content (this is the default),
        otherwise it will be on the left side of the content.

        If horizontalScrollbarAtBottom is set to true, then the horizontal scrollbar
        will appear at the bottom of the view port's content (this is the default),
        otherwise it will be at the top.
    */
    virtual void setScrollBarPosition (bool verticalScrollbarOnRight,
                               bool horizontalScrollbarAtBottom);

    /** True if the vertical scrollbar will appear on the right side of the content */
    virtual bool isVerticalScrollbarOnTheRight() const noexcept         { return vScrollbarRight; }

    /** True if the horizontal scrollbar will appear at the bottom of the content */
    virtual bool isHorizontalScrollbarAtBottom() const noexcept         { return hScrollbarBottom; }

    /** True if the vertical scrollbar is enabled.
        @see setScrollBarsShown
    */
    virtual bool isVerticalScrollBarShown() const noexcept              { return showVScrollbar; }

    /** True if the horizontal scrollbar is enabled.
        @see setScrollBarsShown
    */
    virtual bool isHorizontalScrollBarShown() const noexcept            { return showHScrollbar; }

    /** Changes the width of the scrollbars.
        If this isn't specified, the default width from the LookAndFeel class will be used.
        @see LookAndFeel::getDefaultScrollbarWidth
    */
    virtual void setScrollBarThickness (int thickness);

    /** Returns the thickness of the scrollbars.
        @see setScrollBarThickness
    */
    virtual int getScrollBarThickness() const;

    /** Changes the distance that a single-step click on a scrollbar button
        will move the viewport.
    */
    virtual void setSingleStepSizes (int stepX, int stepY);

    /** Returns a reference to the scrollbar component being used.
        Handy if you need to customise the bar somehow.
    */
    virtual ScrollBar& getVerticalScrollBar() noexcept                  { return *verticalScrollBar; }

    /** Returns a reference to the scrollbar component being used.
        Handy if you need to customise the bar somehow.
    */
    virtual ScrollBar& getHorizontalScrollBar() noexcept                { return *horizontalScrollBar; }

    /** Re-instantiates the scrollbars, which is only really useful if you've overridden createScrollBarComponent(). */
    virtual void recreateScrollbars();

    /** True if there's any off-screen content that could be scrolled vertically,
        or false if everything is currently visible.
    */
    virtual bool canScrollVertically() const noexcept;

    /** True if there's any off-screen content that could be scrolled horizontally,
        or false if everything is currently visible.
    */
    virtual bool canScrollHorizontally() const noexcept;

    /** Enables or disables drag-to-scroll functionality for mouse sources in the viewport.

        If your viewport contains a Component that you don't want to receive mouse events when the
        user is drag-scrolling, you can disable this with the Component::setViewportIgnoreDragFlag()
        method.
    */
    [[deprecated ("Use setScrollOnDragMode instead.")]]
    virtual void setScrollOnDragEnabled (bool shouldScrollOnDrag)
    {
        setScrollOnDragMode (shouldScrollOnDrag ? ScrollOnDragMode::all : ScrollOnDragMode::never);
    }

    /** Returns true if drag-to-scroll functionality is enabled for mouse input sources. */
    [[deprecated ("Use getScrollOnDragMode instead.")]]
    virtual bool isScrollOnDragEnabled() const noexcept { return getScrollOnDragMode() == ScrollOnDragMode::all; }

    enum class ScrollOnDragMode
    {
        never,          /**< Dragging will never scroll the viewport. */
        nonHover,       /**< Dragging will only scroll the viewport if the input source cannot hover. */
        all             /**< Dragging will always scroll the viewport. */
    };

    /** Sets the current scroll-on-drag mode. The default is ScrollOnDragMode::nonHover.

        If your viewport contains a Component that you don't want to receive mouse events when the
        user is drag-scrolling, you can disable this with the Component::setViewportIgnoreDragFlag()
        method.
    */
    virtual void setScrollOnDragMode (ScrollOnDragMode scrollOnDragMode);

    /** Returns the current scroll-on-drag mode. */
    virtual ScrollOnDragMode getScrollOnDragMode() const { return scrollOnDragMode; }

    /** Returns true if the user is currently dragging-to-scroll.
        @see setScrollOnDragEnabled
    */
    virtual bool isCurrentlyScrollingOnDrag() const noexcept;

    //==============================================================================
    /** @internal */
    virtual void resized() override;
    /** @internal */
    virtual void scrollBarMoved (ScrollBar*, double newRangeStart) override;
    /** @internal */
    virtual void mouseWheelMove (const MouseEvent&, const MouseWheelDetails&) override;
    /** @internal */
    virtual void mouseDown (const MouseEvent& e) override;
    /** @internal */
    virtual bool keyPressed (const KeyPress&) override;
    /** @internal */
    virtual void componentMovedOrResized (Component&, bool wasMoved, bool wasResized) override;
    /** @internal */
    virtual void lookAndFeelChanged() override;
    /** @internal */
    virtual bool useMouseWheelMoveIfNeeded (const MouseEvent&, const MouseWheelDetails&);
    /** @internal */
    static bool respondsToKey (const KeyPress&);

protected:
    //==============================================================================
    /** Creates the Scrollbar components that will be added to the Viewport.
        Subclasses can override this if they need to customise the scrollbars in some way.
    */
    virtual ScrollBar* createScrollBarComponent (bool isVertical);

private:
    //==============================================================================
    class AccessibilityIgnoredComponent : public Component
    {
    public:
        std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override
        {
            return createIgnoredAccessibilityHandler (*this);
        }
    };

    std::unique_ptr<ScrollBar> verticalScrollBar, horizontalScrollBar;
    AccessibilityIgnoredComponent contentHolder;
    WeakReference<Component> contentComp;
    Rectangle<int> lastVisibleArea;
    int scrollBarThickness = 0;
    int singleStepX = 16, singleStepY = 16;
    ScrollOnDragMode scrollOnDragMode = ScrollOnDragMode::nonHover;
    bool showHScrollbar = true, showVScrollbar = true, deleteContent = true;
    bool customScrollBarThickness = false;
    bool allowScrollingWithoutScrollbarV = false, allowScrollingWithoutScrollbarH = false;
    bool vScrollbarRight = true, hScrollbarBottom = true;

    struct DragToScrollListener;
    std::unique_ptr<DragToScrollListener> dragToScrollListener;

    Point<int> viewportPosToCompPos (Point<int>) const;
    Rectangle<int> getContentBounds() const;

    void updateVisibleArea();
    void deleteOrRemoveContentComp();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Viewport)
};

} // namespace juce
