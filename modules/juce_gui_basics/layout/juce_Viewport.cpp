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

Viewport::Viewport (const String& name)
  : Component (name),
    scrollBarThickness (0),
    singleStepX (16),
    singleStepY (16),
    showHScrollbar (true),
    showVScrollbar (true),
    deleteContent (true),
    allowScrollingWithoutScrollbarV (false),
    allowScrollingWithoutScrollbarH (false),
    verticalScrollBar (true),
    horizontalScrollBar (false)
{
    // content holder is used to clip the contents so they don't overlap the scrollbars
    addAndMakeVisible (contentHolder);
    contentHolder.setInterceptsMouseClicks (false, true);

    addChildComponent (verticalScrollBar);
    addChildComponent (horizontalScrollBar);

    verticalScrollBar.addListener (this);
    horizontalScrollBar.addListener (this);

    setInterceptsMouseClicks (false, true);
    setWantsKeyboardFocus (true);
}

Viewport::~Viewport()
{
    deleteContentComp();
    mouseWheelTimer = nullptr;
}

//==============================================================================
void Viewport::visibleAreaChanged (const Rectangle<int>&) {}
void Viewport::viewedComponentChanged (Component*) {}

//==============================================================================
void Viewport::deleteContentComp()
{
    if (contentComp != nullptr)
        contentComp->removeComponentListener (this);

    if (deleteContent)
    {
        // This sets the content comp to a null pointer before deleting the old one, in case
        // anything tries to use the old one while it's in mid-deletion..
        ScopedPointer<Component> oldCompDeleter (contentComp);
    }
    else
    {
        contentComp = nullptr;
    }
}

void Viewport::setViewedComponent (Component* const newViewedComponent, const bool deleteComponentWhenNoLongerNeeded)
{
    if (contentComp.get() != newViewedComponent)
    {
        deleteContentComp();
        contentComp = newViewedComponent;
        deleteContent = deleteComponentWhenNoLongerNeeded;

        if (contentComp != nullptr)
        {
            contentHolder.addAndMakeVisible (contentComp);
            setViewPosition (Point<int>());
            contentComp->addComponentListener (this);
        }

        viewedComponentChanged (contentComp);
        updateVisibleArea();
    }
}

int Viewport::getMaximumVisibleWidth() const    { return contentHolder.getWidth(); }
int Viewport::getMaximumVisibleHeight() const   { return contentHolder.getHeight(); }

Point<int> Viewport::viewportPosToCompPos (Point<int> pos) const
{
    jassert (contentComp != nullptr);
    return Point<int> (jmax (jmin (0, contentHolder.getWidth()  - contentComp->getWidth()),  jmin (0, -(pos.x))),
                       jmax (jmin (0, contentHolder.getHeight() - contentComp->getHeight()), jmin (0, -(pos.y))));
}

void Viewport::setViewPosition (const int xPixelsOffset, const int yPixelsOffset)
{
    setViewPosition (Point<int> (xPixelsOffset, yPixelsOffset));
}

void Viewport::setViewPosition (Point<int> newPosition)
{
    if (contentComp != nullptr)
        contentComp->setTopLeftPosition (viewportPosToCompPos (newPosition));
}

void Viewport::setViewPositionProportionately (const double x, const double y)
{
    if (contentComp != nullptr)
        setViewPosition (jmax (0, roundToInt (x * (contentComp->getWidth()  - getWidth()))),
                         jmax (0, roundToInt (y * (contentComp->getHeight() - getHeight()))));
}

bool Viewport::autoScroll (const int mouseX, const int mouseY, const int activeBorderThickness, const int maximumSpeed)
{
    if (contentComp != nullptr)
    {
        int dx = 0, dy = 0;

        if (horizontalScrollBar.isVisible() || contentComp->getX() < 0 || contentComp->getRight() > getWidth())
        {
            if (mouseX < activeBorderThickness)
                dx = activeBorderThickness - mouseX;
            else if (mouseX >= contentHolder.getWidth() - activeBorderThickness)
                dx = (contentHolder.getWidth() - activeBorderThickness) - mouseX;

            if (dx < 0)
                dx = jmax (dx, -maximumSpeed, contentHolder.getWidth() - contentComp->getRight());
            else
                dx = jmin (dx, maximumSpeed, -contentComp->getX());
        }

        if (verticalScrollBar.isVisible() || contentComp->getY() < 0 || contentComp->getBottom() > getHeight())
        {
            if (mouseY < activeBorderThickness)
                dy = activeBorderThickness - mouseY;
            else if (mouseY >= contentHolder.getHeight() - activeBorderThickness)
                dy = (contentHolder.getHeight() - activeBorderThickness) - mouseY;

            if (dy < 0)
                dy = jmax (dy, -maximumSpeed, contentHolder.getHeight() - contentComp->getBottom());
            else
                dy = jmin (dy, maximumSpeed, -contentComp->getY());
        }

        if (dx != 0 || dy != 0)
        {
            contentComp->setTopLeftPosition (contentComp->getX() + dx,
                                             contentComp->getY() + dy);

            return true;
        }
    }

    return false;
}

void Viewport::componentMovedOrResized (Component&, bool, bool)
{
    updateVisibleArea();
}

void Viewport::resized()
{
    updateVisibleArea();
}

//==============================================================================
void Viewport::updateVisibleArea()
{
    const int scrollbarWidth = getScrollBarThickness();
    const bool canShowAnyBars = getWidth() > scrollbarWidth && getHeight() > scrollbarWidth;
    const bool canShowHBar = showHScrollbar && canShowAnyBars;
    const bool canShowVBar = showVScrollbar && canShowAnyBars;

    bool hBarVisible = false, vBarVisible = false;
    Rectangle<int> contentArea;

    for (int i = 3; --i >= 0;)
    {
        hBarVisible = canShowHBar && ! horizontalScrollBar.autoHides();
        vBarVisible = canShowVBar && ! verticalScrollBar.autoHides();
        contentArea = getLocalBounds();

        if (contentComp != nullptr && ! contentArea.contains (contentComp->getBounds()))
        {
            hBarVisible = canShowHBar && (hBarVisible || contentComp->getX() < 0 || contentComp->getRight() > contentArea.getWidth());
            vBarVisible = canShowVBar && (vBarVisible || contentComp->getY() < 0 || contentComp->getBottom() > contentArea.getHeight());

            if (vBarVisible)
                contentArea.setWidth (getWidth() - scrollbarWidth);

            if (hBarVisible)
                contentArea.setHeight (getHeight() - scrollbarWidth);

            if (! contentArea.contains (contentComp->getBounds()))
            {
                hBarVisible = canShowHBar && (hBarVisible || contentComp->getRight() > contentArea.getWidth());
                vBarVisible = canShowVBar && (vBarVisible || contentComp->getBottom() > contentArea.getHeight());
            }
        }

        if (vBarVisible)  contentArea.setWidth  (getWidth()  - scrollbarWidth);
        if (hBarVisible)  contentArea.setHeight (getHeight() - scrollbarWidth);

        if (contentComp == nullptr)
        {
            contentHolder.setBounds (contentArea);
            break;
        }

        const Rectangle<int> oldContentBounds (contentComp->getBounds());
        contentHolder.setBounds (contentArea);

        // If the content has changed its size, that might affect our scrollbars, so go round again and re-caclulate..
        if (oldContentBounds == contentComp->getBounds())
            break;
    }

    Rectangle<int> contentBounds;
    if (contentComp != nullptr)
        contentBounds = contentHolder.getLocalArea (contentComp, contentComp->getLocalBounds());

    Point<int> visibleOrigin (-contentBounds.getPosition());

    horizontalScrollBar.setBounds (0, contentArea.getHeight(), contentArea.getWidth(), scrollbarWidth);
    horizontalScrollBar.setRangeLimits (0.0, contentBounds.getWidth());
    horizontalScrollBar.setCurrentRange (visibleOrigin.x, contentArea.getWidth());
    horizontalScrollBar.setSingleStepSize (singleStepX);
    horizontalScrollBar.cancelPendingUpdate();

    if (canShowHBar && ! hBarVisible)
        visibleOrigin.setX (0);

    verticalScrollBar.setBounds (contentArea.getWidth(), 0, scrollbarWidth, contentArea.getHeight());
    verticalScrollBar.setRangeLimits (0.0, contentBounds.getHeight());
    verticalScrollBar.setCurrentRange (visibleOrigin.y, contentArea.getHeight());
    verticalScrollBar.setSingleStepSize (singleStepY);
    verticalScrollBar.cancelPendingUpdate();

    if (canShowVBar && ! vBarVisible)
        visibleOrigin.setY (0);

    // Force the visibility *after* setting the ranges to avoid flicker caused by edge conditions in the numbers.
    horizontalScrollBar.setVisible (hBarVisible);
    verticalScrollBar.setVisible (vBarVisible);

    if (contentComp != nullptr)
    {
        const Point<int> newContentCompPos (viewportPosToCompPos (visibleOrigin));

        if (contentComp->getBounds().getPosition() != newContentCompPos)
        {
            contentComp->setTopLeftPosition (newContentCompPos);  // (this will re-entrantly call updateVisibleArea again)
            return;
        }
    }

    const Rectangle<int> visibleArea (visibleOrigin.x, visibleOrigin.y,
                                      jmin (contentBounds.getWidth()  - visibleOrigin.x, contentArea.getWidth()),
                                      jmin (contentBounds.getHeight() - visibleOrigin.y, contentArea.getHeight()));

    if (lastVisibleArea != visibleArea)
    {
        lastVisibleArea = visibleArea;
        visibleAreaChanged (visibleArea);
    }

    horizontalScrollBar.handleUpdateNowIfNeeded();
    verticalScrollBar.handleUpdateNowIfNeeded();
}

//==============================================================================
void Viewport::setSingleStepSizes (const int stepX, const int stepY)
{
    if (singleStepX != stepX || singleStepY != stepY)
    {
        singleStepX = stepX;
        singleStepY = stepY;
        updateVisibleArea();
    }
}

void Viewport::setScrollBarsShown (const bool showVerticalScrollbarIfNeeded,
                                   const bool showHorizontalScrollbarIfNeeded,
                                   const bool allowVerticalScrollingWithoutScrollbar,
                                   const bool allowHorizontalScrollingWithoutScrollbar)
{
    allowScrollingWithoutScrollbarV = allowVerticalScrollingWithoutScrollbar;
    allowScrollingWithoutScrollbarH = allowHorizontalScrollingWithoutScrollbar;

    if (showVScrollbar != showVerticalScrollbarIfNeeded
         || showHScrollbar != showHorizontalScrollbarIfNeeded)
    {
        showVScrollbar = showVerticalScrollbarIfNeeded;
        showHScrollbar = showHorizontalScrollbarIfNeeded;
        updateVisibleArea();
    }
}

void Viewport::setScrollBarThickness (const int thickness)
{
    if (scrollBarThickness != thickness)
    {
        scrollBarThickness = thickness;
        updateVisibleArea();
    }
}

int Viewport::getScrollBarThickness() const
{
    return scrollBarThickness > 0 ? scrollBarThickness
                                  : getLookAndFeel().getDefaultScrollbarWidth();
}

void Viewport::scrollBarMoved (ScrollBar* scrollBarThatHasMoved, double newRangeStart)
{
    const int newRangeStartInt = roundToInt (newRangeStart);

    if (scrollBarThatHasMoved == &horizontalScrollBar)
    {
        setViewPosition (newRangeStartInt, getViewPositionY());
    }
    else if (scrollBarThatHasMoved == &verticalScrollBar)
    {
        setViewPosition (getViewPositionX(), newRangeStartInt);
    }
}

void Viewport::mouseWheelMove (const MouseEvent& e, const MouseWheelDetails& wheel)
{
    if (! useMouseWheelMoveIfNeeded (e, wheel))
        Component::mouseWheelMove (e, wheel);
}

static int rescaleMouseWheelDistance (float distance, int singleStepSize) noexcept
{
    if (distance == 0)
        return 0;

    distance *= 14.0f * singleStepSize;

    return roundToInt (distance < 0 ? jmin (distance, -1.0f)
                                    : jmax (distance,  1.0f));
}

// This puts a temporary component overlay over the content component, to prevent
// wheel events from reaching components inside it, so that while spinning a wheel
// with momentum, it won't accidentally scroll any subcomponents of the viewport.
struct Viewport::MouseWheelTimer  : public Timer
{
    MouseWheelTimer (Viewport& v) : viewport (v)
    {
        viewport.contentHolder.addAndMakeVisible (dummyOverlay);
        dummyOverlay.setAlwaysOnTop (true);
        dummyOverlay.setPaintingIsUnclipped (true);
        dummyOverlay.setBounds (viewport.contentHolder.getLocalBounds());
    }

    void timerCallback() override
    {
        viewport.mouseWheelTimer = nullptr;
    }

    Component dummyOverlay;
    Viewport& viewport;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MouseWheelTimer)
};

bool Viewport::useMouseWheelMoveIfNeeded (const MouseEvent& e, const MouseWheelDetails& wheel)
{
    if (! (e.mods.isAltDown() || e.mods.isCtrlDown() || e.mods.isCommandDown()))
    {
        const bool canScrollVert = (allowScrollingWithoutScrollbarV || verticalScrollBar.isVisible());
        const bool canScrollHorz = (allowScrollingWithoutScrollbarH || horizontalScrollBar.isVisible());

        if (canScrollHorz || canScrollVert)
        {
            const int deltaX = rescaleMouseWheelDistance (wheel.deltaX, singleStepX);
            const int deltaY = rescaleMouseWheelDistance (wheel.deltaY, singleStepY);

            Point<int> pos (getViewPosition());

            if (deltaX != 0 && deltaY != 0 && canScrollHorz && canScrollVert)
            {
                pos.x -= deltaX;
                pos.y -= deltaY;
            }
            else if (canScrollHorz && (deltaX != 0 || e.mods.isShiftDown() || ! canScrollVert))
            {
                pos.x -= deltaX != 0 ? deltaX : deltaY;
            }
            else if (canScrollVert && deltaY != 0)
            {
                pos.y -= deltaY;
            }

            if (pos != getViewPosition())
            {
                if (mouseWheelTimer == nullptr)
                    mouseWheelTimer = new MouseWheelTimer (*this);

                mouseWheelTimer->startTimer (300);

                setViewPosition (pos);
                return true;
            }
        }
    }

    return false;
}

static bool isUpDownKeyPress (const KeyPress& key)
{
    return key == KeyPress::upKey
        || key == KeyPress::downKey
        || key == KeyPress::pageUpKey
        || key == KeyPress::pageDownKey
        || key == KeyPress::homeKey
        || key == KeyPress::endKey;
}

static bool isLeftRightKeyPress (const KeyPress& key)
{
    return key == KeyPress::leftKey
        || key == KeyPress::rightKey;
}

bool Viewport::keyPressed (const KeyPress& key)
{
    const bool isUpDownKey = isUpDownKeyPress (key);

    if (verticalScrollBar.isVisible() && isUpDownKey)
        return verticalScrollBar.keyPressed (key);

    const bool isLeftRightKey = isLeftRightKeyPress (key);

    if (horizontalScrollBar.isVisible() && (isUpDownKey || isLeftRightKey))
        return horizontalScrollBar.keyPressed (key);

    return false;
}

bool Viewport::respondsToKey (const KeyPress& key)
{
    return isUpDownKeyPress (key) || isLeftRightKeyPress (key);
}
