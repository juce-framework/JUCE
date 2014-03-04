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

ResizableWindow::ResizableWindow (const String& name,
                                  const bool addToDesktop_)
    : TopLevelWindow (name, addToDesktop_),
      ownsContentComponent (false),
      resizeToFitContent (false),
      fullscreen (false),
      constrainer (nullptr)
     #if JUCE_DEBUG
      , hasBeenResized (false)
     #endif
{
    initialise (addToDesktop_);
}

ResizableWindow::ResizableWindow (const String& name,
                                  Colour backgroundColour_,
                                  const bool addToDesktop_)
    : TopLevelWindow (name, addToDesktop_),
      ownsContentComponent (false),
      resizeToFitContent (false),
      fullscreen (false),
      constrainer (nullptr)
     #if JUCE_DEBUG
      , hasBeenResized (false)
     #endif
{
    setBackgroundColour (backgroundColour_);

    initialise (addToDesktop_);
}

ResizableWindow::~ResizableWindow()
{
    // Don't delete or remove the resizer components yourself! They're managed by the
    // ResizableWindow, and you should leave them alone! You may have deleted them
    // accidentally by careless use of deleteAllChildren()..?
    jassert (resizableCorner == nullptr || getIndexOfChildComponent (resizableCorner) >= 0);
    jassert (resizableBorder == nullptr || getIndexOfChildComponent (resizableBorder) >= 0);

    resizableCorner = nullptr;
    resizableBorder = nullptr;
    clearContentComponent();

    // have you been adding your own components directly to this window..? tut tut tut.
    // Read the instructions for using a ResizableWindow!
    jassert (getNumChildComponents() == 0);
}

void ResizableWindow::initialise (const bool shouldAddToDesktop)
{
    defaultConstrainer.setMinimumOnscreenAmounts (0x10000, 16, 24, 16);

    lastNonFullScreenPos.setBounds (50, 50, 256, 256);

    if (shouldAddToDesktop)
        addToDesktop();
}

int ResizableWindow::getDesktopWindowStyleFlags() const
{
    int styleFlags = TopLevelWindow::getDesktopWindowStyleFlags();

    if (isResizable() && (styleFlags & ComponentPeer::windowHasTitleBar) != 0)
        styleFlags |= ComponentPeer::windowIsResizable;

    return styleFlags;
}

//==============================================================================
void ResizableWindow::clearContentComponent()
{
    if (ownsContentComponent)
    {
        contentComponent.deleteAndZero();
    }
    else
    {
        removeChildComponent (contentComponent);
        contentComponent = nullptr;
    }
}

void ResizableWindow::setContent (Component* newContentComponent,
                                  const bool takeOwnership,
                                  const bool resizeToFitWhenContentChangesSize)
{
    if (newContentComponent != contentComponent)
    {
        clearContentComponent();

        contentComponent = newContentComponent;
        Component::addAndMakeVisible (contentComponent);
    }

    ownsContentComponent = takeOwnership;
    resizeToFitContent = resizeToFitWhenContentChangesSize;

    if (resizeToFitWhenContentChangesSize)
        childBoundsChanged (contentComponent);

    resized(); // must always be called to position the new content comp
}

void ResizableWindow::setContentOwned (Component* newContentComponent, const bool resizeToFitWhenContentChangesSize)
{
    setContent (newContentComponent, true, resizeToFitWhenContentChangesSize);
}

void ResizableWindow::setContentNonOwned (Component* newContentComponent, const bool resizeToFitWhenContentChangesSize)
{
    setContent (newContentComponent, false, resizeToFitWhenContentChangesSize);
}

void ResizableWindow::setContentComponent (Component* const newContentComponent,
                                           const bool deleteOldOne,
                                           const bool resizeToFitWhenContentChangesSize)
{
    if (newContentComponent != contentComponent)
    {
        if (deleteOldOne)
        {
            contentComponent.deleteAndZero();
        }
        else
        {
            removeChildComponent (contentComponent);
            contentComponent = nullptr;
        }
    }

    setContent (newContentComponent, true, resizeToFitWhenContentChangesSize);
}

void ResizableWindow::setContentComponentSize (int width, int height)
{
    jassert (width > 0 && height > 0); // not a great idea to give it a zero size..

    const BorderSize<int> border (getContentComponentBorder());

    setSize (width + border.getLeftAndRight(),
             height + border.getTopAndBottom());
}

BorderSize<int> ResizableWindow::getBorderThickness()
{
    if (isUsingNativeTitleBar() || isKioskMode())
        return BorderSize<int>();

    return BorderSize<int> ((resizableBorder != nullptr && ! isFullScreen()) ? 4 : 1);
}

BorderSize<int> ResizableWindow::getContentComponentBorder()
{
    return getBorderThickness();
}

void ResizableWindow::moved()
{
    updateLastPosIfShowing();
}

void ResizableWindow::visibilityChanged()
{
    TopLevelWindow::visibilityChanged();

    updateLastPosIfShowing();
}

void ResizableWindow::resized()
{
    const bool resizerHidden = isFullScreen() || isKioskMode() || isUsingNativeTitleBar();

    if (resizableBorder != nullptr)
    {
        resizableBorder->setVisible (! resizerHidden);
        resizableBorder->setBorderThickness (getBorderThickness());
        resizableBorder->setSize (getWidth(), getHeight());
        resizableBorder->toBack();
    }

    if (resizableCorner != nullptr)
    {
        resizableCorner->setVisible (! resizerHidden);

        const int resizerSize = 18;
        resizableCorner->setBounds (getWidth() - resizerSize,
                                    getHeight() - resizerSize,
                                    resizerSize, resizerSize);
    }

    if (contentComponent != nullptr)
    {
        // The window expects to be able to be able to manage the size and position
        // of its content component, so you can't arbitrarily add a transform to it!
        jassert (! contentComponent->isTransformed());

        contentComponent->setBoundsInset (getContentComponentBorder());
    }

    updateLastPosIfShowing();

   #if JUCE_DEBUG
    hasBeenResized = true;
   #endif
}

void ResizableWindow::childBoundsChanged (Component* child)
{
    if ((child == contentComponent) && (child != nullptr) && resizeToFitContent)
    {
        // not going to look very good if this component has a zero size..
        jassert (child->getWidth() > 0);
        jassert (child->getHeight() > 0);

        const BorderSize<int> borders (getContentComponentBorder());

        setSize (child->getWidth() + borders.getLeftAndRight(),
                 child->getHeight() + borders.getTopAndBottom());
    }
}


//==============================================================================
void ResizableWindow::activeWindowStatusChanged()
{
    const BorderSize<int> border (getContentComponentBorder());

    Rectangle<int> area (getLocalBounds());
    repaint (area.removeFromTop (border.getTop()));
    repaint (area.removeFromLeft (border.getLeft()));
    repaint (area.removeFromRight (border.getRight()));
    repaint (area.removeFromBottom (border.getBottom()));
}

//==============================================================================
void ResizableWindow::setResizable (const bool shouldBeResizable,
                                    const bool useBottomRightCornerResizer)
{
    if (shouldBeResizable)
    {
        if (useBottomRightCornerResizer)
        {
            resizableBorder = nullptr;

            if (resizableCorner == nullptr)
            {
                Component::addChildComponent (resizableCorner = new ResizableCornerComponent (this, constrainer));
                resizableCorner->setAlwaysOnTop (true);
            }
        }
        else
        {
            resizableCorner = nullptr;

            if (resizableBorder == nullptr)
                Component::addChildComponent (resizableBorder = new ResizableBorderComponent (this, constrainer));
        }
    }
    else
    {
        resizableCorner = nullptr;
        resizableBorder = nullptr;
    }

    if (isUsingNativeTitleBar())
        recreateDesktopWindow();

    childBoundsChanged (contentComponent);
    resized();
}

bool ResizableWindow::isResizable() const noexcept
{
    return resizableCorner != nullptr
        || resizableBorder != nullptr;
}

void ResizableWindow::setResizeLimits (const int newMinimumWidth,
                                       const int newMinimumHeight,
                                       const int newMaximumWidth,
                                       const int newMaximumHeight) noexcept
{
    // if you've set up a custom constrainer then these settings won't have any effect..
    jassert (constrainer == &defaultConstrainer || constrainer == nullptr);

    if (constrainer == nullptr)
        setConstrainer (&defaultConstrainer);

    defaultConstrainer.setSizeLimits (newMinimumWidth, newMinimumHeight,
                                      newMaximumWidth, newMaximumHeight);

    setBoundsConstrained (getBounds());
}

void ResizableWindow::setConstrainer (ComponentBoundsConstrainer* newConstrainer)
{
    if (constrainer != newConstrainer)
    {
        constrainer = newConstrainer;

        const bool useBottomRightCornerResizer = resizableCorner != nullptr;
        const bool shouldBeResizable = useBottomRightCornerResizer || resizableBorder != nullptr;

        resizableCorner = nullptr;
        resizableBorder = nullptr;

        setResizable (shouldBeResizable, useBottomRightCornerResizer);

        if (ComponentPeer* const peer = getPeer())
            peer->setConstrainer (newConstrainer);
    }
}

void ResizableWindow::setBoundsConstrained (const Rectangle<int>& newBounds)
{
    if (constrainer != nullptr)
        constrainer->setBoundsForComponent (this, newBounds, false, false, false, false);
    else
        setBounds (newBounds);
}

//==============================================================================
void ResizableWindow::paint (Graphics& g)
{
    LookAndFeel& lf = getLookAndFeel();

    lf.fillResizableWindowBackground (g, getWidth(), getHeight(),
                                      getBorderThickness(), *this);

    if (! isFullScreen())
        lf.drawResizableWindowBorder (g, getWidth(), getHeight(),
                                      getBorderThickness(), *this);

   #if JUCE_DEBUG
    /* If this fails, then you've probably written a subclass with a resized()
       callback but forgotten to make it call its parent class's resized() method.

       It's important when you override methods like resized(), moved(),
       etc., that you make sure the base class methods also get called.

       Of course you shouldn't really be overriding ResizableWindow::resized() anyway,
       because your content should all be inside the content component - and it's the
       content component's resized() method that you should be using to do your
       layout.
    */
    jassert (hasBeenResized || (getWidth() == 0 && getHeight() == 0));
   #endif
}

void ResizableWindow::lookAndFeelChanged()
{
    resized();

    if (isOnDesktop())
    {
        Component::addToDesktop (getDesktopWindowStyleFlags());

        if (ComponentPeer* const peer = getPeer())
            peer->setConstrainer (constrainer);
    }
}

Colour ResizableWindow::getBackgroundColour() const noexcept
{
    return findColour (backgroundColourId, false);
}

void ResizableWindow::setBackgroundColour (Colour newColour)
{
    Colour backgroundColour (newColour);

    if (! Desktop::canUseSemiTransparentWindows())
        backgroundColour = newColour.withAlpha (1.0f);

    setColour (backgroundColourId, backgroundColour);

    setOpaque (backgroundColour.isOpaque());
    repaint();
}

//==============================================================================
bool ResizableWindow::isFullScreen() const
{
    if (isOnDesktop())
    {
        ComponentPeer* const peer = getPeer();
        return peer != nullptr && peer->isFullScreen();
    }

    return fullscreen;
}

void ResizableWindow::setFullScreen (const bool shouldBeFullScreen)
{
    if (shouldBeFullScreen != isFullScreen())
    {
        updateLastPosIfShowing();
        fullscreen = shouldBeFullScreen;

        if (isOnDesktop())
        {
            if (ComponentPeer* const peer = getPeer())
            {
                // keep a copy of this intact in case the real one gets messed-up while we're un-maximising
                const Rectangle<int> lastPos (lastNonFullScreenPos);

                peer->setFullScreen (shouldBeFullScreen);

                if ((! shouldBeFullScreen) && ! lastPos.isEmpty())
                    setBounds (lastPos);
            }
            else
            {
                jassertfalse;
            }
        }
        else
        {
            if (shouldBeFullScreen)
                setBounds (0, 0, getParentWidth(), getParentHeight());
            else
                setBounds (lastNonFullScreenPos);
        }

        resized();
    }
}

bool ResizableWindow::isMinimised() const
{
    if (ComponentPeer* const peer = getPeer())
        return peer->isMinimised();

    return false;
}

void ResizableWindow::setMinimised (const bool shouldMinimise)
{
    if (shouldMinimise != isMinimised())
    {
        if (ComponentPeer* const peer = getPeer())
        {
            updateLastPosIfShowing();
            peer->setMinimised (shouldMinimise);
        }
        else
        {
            jassertfalse;
        }
    }
}

bool ResizableWindow::isKioskMode() const
{
    if (isOnDesktop())
        if (ComponentPeer* peer = getPeer())
            return peer->isKioskMode();

    return Desktop::getInstance().getKioskModeComponent() == this;
}

void ResizableWindow::updateLastPosIfShowing()
{
    if (isShowing())
        updateLastPosIfNotFullScreen();
}

void ResizableWindow::updateLastPosIfNotFullScreen()
{
    if (! (isFullScreen() || isMinimised() || isKioskMode()))
        lastNonFullScreenPos = getBounds();
}

void ResizableWindow::parentSizeChanged()
{
    if (isFullScreen() && getParentComponent() != nullptr)
        setBounds (getParentComponent()->getLocalBounds());
}

//==============================================================================
String ResizableWindow::getWindowStateAsString()
{
    updateLastPosIfShowing();
    return (isFullScreen() && ! isKioskMode() ? "fs " : "") + lastNonFullScreenPos.toString();
}

bool ResizableWindow::restoreWindowStateFromString (const String& s)
{
    StringArray tokens;
    tokens.addTokens (s, false);
    tokens.removeEmptyStrings();
    tokens.trim();

    const bool fs = tokens[0].startsWithIgnoreCase ("fs");
    const int firstCoord = fs ? 1 : 0;

    if (tokens.size() != firstCoord + 4)
        return false;

    Rectangle<int> newPos (tokens[firstCoord].getIntValue(),
                           tokens[firstCoord + 1].getIntValue(),
                           tokens[firstCoord + 2].getIntValue(),
                           tokens[firstCoord + 3].getIntValue());

    if (newPos.isEmpty())
        return false;

    ComponentPeer* const peer = isOnDesktop() ? getPeer() : nullptr;
    if (peer != nullptr)
        peer->getFrameSize().addTo (newPos);

    {
        Desktop& desktop = Desktop::getInstance();
        RectangleList<int> allMonitors (desktop.getDisplays().getRectangleList (true));
        allMonitors.clipTo (newPos);
        const Rectangle<int> onScreenArea (allMonitors.getBounds());

        if (onScreenArea.getWidth() * onScreenArea.getHeight() < 32 * 32)
        {
            const Rectangle<int> screen (desktop.getDisplays().getDisplayContaining (newPos.getCentre()).userArea);

            newPos.setSize (jmin (newPos.getWidth(),  screen.getWidth()),
                            jmin (newPos.getHeight(), screen.getHeight()));

            newPos.setPosition (jlimit (screen.getX(), screen.getRight()  - newPos.getWidth(),  newPos.getX()),
                                jlimit (screen.getY(), screen.getBottom() - newPos.getHeight(), newPos.getY()));
        }
    }

    if (peer != nullptr)
    {
        peer->getFrameSize().subtractFrom (newPos);
        peer->setNonFullScreenBounds (newPos);
    }

    updateLastPosIfNotFullScreen();
    setFullScreen (fs);

    if (! fs)
        setBoundsConstrained (newPos);

    return true;
}

//==============================================================================
void ResizableWindow::mouseDown (const MouseEvent& e)
{
    if (! isFullScreen())
        dragger.startDraggingComponent (this, e);
}

void ResizableWindow::mouseDrag (const MouseEvent& e)
{
    if (! isFullScreen())
        dragger.dragComponent (this, e, constrainer);
}

//==============================================================================
#if JUCE_DEBUG
void ResizableWindow::addChildComponent (Component* const child, int zOrder)
{
    /* Agh! You shouldn't add components directly to a ResizableWindow - this class
       manages its child components automatically, and if you add your own it'll cause
       trouble. Instead, use setContentComponent() to give it a component which
       will be automatically resized and kept in the right place - then you can add
       subcomponents to the content comp. See the notes for the ResizableWindow class
       for more info.

       If you really know what you're doing and want to avoid this assertion, just call
       Component::addChildComponent directly.
    */
    jassertfalse;

    Component::addChildComponent (child, zOrder);
}

void ResizableWindow::addAndMakeVisible (Component* const child, int zOrder)
{
    /* Agh! You shouldn't add components directly to a ResizableWindow - this class
       manages its child components automatically, and if you add your own it'll cause
       trouble. Instead, use setContentComponent() to give it a component which
       will be automatically resized and kept in the right place - then you can add
       subcomponents to the content comp. See the notes for the ResizableWindow class
       for more info.

       If you really know what you're doing and want to avoid this assertion, just call
       Component::addAndMakeVisible directly.
    */
    jassertfalse;

    Component::addAndMakeVisible (child, zOrder);
}
#endif
