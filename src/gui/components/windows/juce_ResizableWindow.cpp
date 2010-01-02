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


#include "juce_ResizableWindow.h"
#include "../juce_Desktop.h"
#include "../lookandfeel/juce_LookAndFeel.h"
#include "../../graphics/geometry/juce_RectangleList.h"


//==============================================================================
ResizableWindow::ResizableWindow (const String& name,
                                  const bool addToDesktop_)
    : TopLevelWindow (name, addToDesktop_),
      resizeToFitContent (false),
      fullscreen (false),
      lastNonFullScreenPos (50, 50, 256, 256),
      constrainer (0)
#ifdef JUCE_DEBUG
      , hasBeenResized (false)
#endif
{
    defaultConstrainer.setMinimumOnscreenAmounts (0x10000, 16, 24, 16);

    lastNonFullScreenPos.setBounds (50, 50, 256, 256);

    if (addToDesktop_)
        Component::addToDesktop (getDesktopWindowStyleFlags());
}

ResizableWindow::ResizableWindow (const String& name,
                                  const Colour& backgroundColour_,
                                  const bool addToDesktop_)
    : TopLevelWindow (name, addToDesktop_),
      resizeToFitContent (false),
      fullscreen (false),
      lastNonFullScreenPos (50, 50, 256, 256),
      constrainer (0)
#ifdef JUCE_DEBUG
      , hasBeenResized (false)
#endif
{
    setBackgroundColour (backgroundColour_);

    defaultConstrainer.setMinimumOnscreenAmounts (0x10000, 16, 24, 16);

    if (addToDesktop_)
        Component::addToDesktop (getDesktopWindowStyleFlags());
}

ResizableWindow::~ResizableWindow()
{
    resizableCorner = 0;
    resizableBorder = 0;
    contentComponent = 0;

    // have you been adding your own components directly to this window..? tut tut tut.
    // Read the instructions for using a ResizableWindow!
    jassert (getNumChildComponents() == 0);
}

int ResizableWindow::getDesktopWindowStyleFlags() const
{
    int flags = TopLevelWindow::getDesktopWindowStyleFlags();

    if (isResizable() && (flags & ComponentPeer::windowHasTitleBar) != 0)
        flags |= ComponentPeer::windowIsResizable;

    return flags;
}

//==============================================================================
void ResizableWindow::setContentComponent (Component* const newContentComponent,
                                           const bool deleteOldOne,
                                           const bool resizeToFit)
{
    resizeToFitContent = resizeToFit;

    if (newContentComponent != (Component*) contentComponent)
    {
        if (! deleteOldOne)
            removeChildComponent (contentComponent.release());

        contentComponent = newContentComponent;

        Component::addAndMakeVisible (contentComponent);
    }

    if (resizeToFit)
        childBoundsChanged (contentComponent);

    resized(); // must always be called to position the new content comp
}

void ResizableWindow::setContentComponentSize (int width, int height)
{
    jassert (width > 0 && height > 0); // not a great idea to give it a zero size..

    const BorderSize border (getContentComponentBorder());

    setSize (width + border.getLeftAndRight(),
             height + border.getTopAndBottom());
}

const BorderSize ResizableWindow::getBorderThickness()
{
    return BorderSize (isUsingNativeTitleBar() ? 0 : ((resizableBorder != 0 && ! isFullScreen()) ? 5 : 3));
}

const BorderSize ResizableWindow::getContentComponentBorder()
{
    return getBorderThickness();
}

void ResizableWindow::moved()
{
    updateLastPos();
}

void ResizableWindow::visibilityChanged()
{
    TopLevelWindow::visibilityChanged();

    updateLastPos();
}

void ResizableWindow::resized()
{
    if (resizableBorder != 0)
    {
        resizableBorder->setVisible (! isFullScreen());
        resizableBorder->setBorderThickness (getBorderThickness());

        resizableBorder->setSize (getWidth(), getHeight());
        resizableBorder->toBack();
    }

    if (resizableCorner != 0)
    {
        resizableCorner->setVisible (! isFullScreen());

        const int resizerSize = 18;
        resizableCorner->setBounds (getWidth() - resizerSize,
                                    getHeight() - resizerSize,
                                    resizerSize, resizerSize);
    }

    if (contentComponent != 0)
        contentComponent->setBoundsInset (getContentComponentBorder());

    updateLastPos();

#ifdef JUCE_DEBUG
    hasBeenResized = true;
#endif
}

void ResizableWindow::childBoundsChanged (Component* child)
{
    if ((child == contentComponent) && (child != 0) && resizeToFitContent)
    {
        // not going to look very good if this component has a zero size..
        jassert (child->getWidth() > 0);
        jassert (child->getHeight() > 0);

        const BorderSize borders (getContentComponentBorder());

        setSize (child->getWidth() + borders.getLeftAndRight(),
                 child->getHeight() + borders.getTopAndBottom());
    }
}


//==============================================================================
void ResizableWindow::activeWindowStatusChanged()
{
    const BorderSize borders (getContentComponentBorder());

    repaint (0, 0, getWidth(), borders.getTop());
    repaint (0, borders.getTop(), borders.getLeft(), getHeight() - borders.getBottom() - borders.getTop());
    repaint (0, getHeight() - borders.getBottom(), getWidth(), borders.getBottom());
    repaint (getWidth() - borders.getRight(), borders.getTop(), borders.getRight(), getHeight() - borders.getBottom() - borders.getTop());
}

//==============================================================================
void ResizableWindow::setResizable (const bool shouldBeResizable,
                                    const bool useBottomRightCornerResizer)
{
    if (shouldBeResizable)
    {
        if (useBottomRightCornerResizer)
        {
            resizableBorder = 0;

            if (resizableCorner == 0)
            {
                Component::addChildComponent (resizableCorner = new ResizableCornerComponent (this, constrainer));
                resizableCorner->setAlwaysOnTop (true);
            }
        }
        else
        {
            resizableCorner = 0;

            if (resizableBorder == 0)
                Component::addChildComponent (resizableBorder = new ResizableBorderComponent (this, constrainer));
        }
    }
    else
    {
        resizableCorner = 0;
        resizableBorder = 0;
    }

    if (isUsingNativeTitleBar())
        recreateDesktopWindow();

    childBoundsChanged (contentComponent);
    resized();
}

bool ResizableWindow::isResizable() const throw()
{
    return resizableCorner != 0
        || resizableBorder != 0;
}

void ResizableWindow::setResizeLimits (const int newMinimumWidth,
                                       const int newMinimumHeight,
                                       const int newMaximumWidth,
                                       const int newMaximumHeight) throw()
{
    // if you've set up a custom constrainer then these settings won't have any effect..
    jassert (constrainer == &defaultConstrainer || constrainer == 0);

    if (constrainer == 0)
        setConstrainer (&defaultConstrainer);

    defaultConstrainer.setSizeLimits (newMinimumWidth, newMinimumHeight,
                                      newMaximumWidth, newMaximumHeight);

    setBoundsConstrained (getX(), getY(), getWidth(), getHeight());
}

void ResizableWindow::setConstrainer (ComponentBoundsConstrainer* newConstrainer)
{
    if (constrainer != newConstrainer)
    {
        constrainer = newConstrainer;

        const bool useBottomRightCornerResizer = resizableCorner != 0;
        const bool shouldBeResizable = useBottomRightCornerResizer || resizableBorder != 0;

        resizableCorner = 0;
        resizableBorder = 0;

        setResizable (shouldBeResizable, useBottomRightCornerResizer);

        ComponentPeer* const peer = getPeer();
        if (peer != 0)
            peer->setConstrainer (newConstrainer);
    }
}

void ResizableWindow::setBoundsConstrained (int x, int y, int w, int h)
{
    if (constrainer != 0)
        constrainer->setBoundsForComponent (this, x, y, w, h, false, false, false, false);
    else
        setBounds (x, y, w, h);
}

//==============================================================================
void ResizableWindow::paint (Graphics& g)
{
    getLookAndFeel().fillResizableWindowBackground (g, getWidth(), getHeight(),
                                                    getBorderThickness(), *this);

    if (! isFullScreen())
    {
        getLookAndFeel().drawResizableWindowBorder (g, getWidth(), getHeight(),
                                                    getBorderThickness(), *this);
    }

#ifdef JUCE_DEBUG
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

        ComponentPeer* const peer = getPeer();
        if (peer != 0)
            peer->setConstrainer (constrainer);
    }
}

const Colour ResizableWindow::getBackgroundColour() const throw()
{
    return findColour (backgroundColourId, false);
}

void ResizableWindow::setBackgroundColour (const Colour& newColour)
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
        return peer != 0 && peer->isFullScreen();
    }

    return fullscreen;
}

void ResizableWindow::setFullScreen (const bool shouldBeFullScreen)
{
    if (shouldBeFullScreen != isFullScreen())
    {
        updateLastPos();
        fullscreen = shouldBeFullScreen;

        if (isOnDesktop())
        {
            ComponentPeer* const peer = getPeer();

            if (peer != 0)
            {
                // keep a copy of this intact in case the real one gets messed-up while we're un-maximising
                const Rectangle lastPos (lastNonFullScreenPos);

                peer->setFullScreen (shouldBeFullScreen);

                if (! shouldBeFullScreen)
                    setBounds (lastPos);
            }
            else
            {
                jassertfalse
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
    ComponentPeer* const peer = getPeer();

    return (peer != 0) && peer->isMinimised();
}

void ResizableWindow::setMinimised (const bool shouldMinimise)
{
    if (shouldMinimise != isMinimised())
    {
        ComponentPeer* const peer = getPeer();

        if (peer != 0)
        {
            updateLastPos();
            peer->setMinimised (shouldMinimise);
        }
        else
        {
            jassertfalse
        }
    }
}

void ResizableWindow::updateLastPos()
{
    if (isShowing() && ! (isFullScreen() || isMinimised()))
    {
        lastNonFullScreenPos = getBounds();
    }
}

void ResizableWindow::parentSizeChanged()
{
    if (isFullScreen() && getParentComponent() != 0)
    {
        setBounds (0, 0, getParentWidth(), getParentHeight());
    }
}

//==============================================================================
const String ResizableWindow::getWindowStateAsString()
{
    updateLastPos();

    String s;

    if (isFullScreen())
        s << "fs ";

    s << lastNonFullScreenPos.getX() << T(' ')
      << lastNonFullScreenPos.getY() << T(' ')
      << lastNonFullScreenPos.getWidth() << T(' ')
      << lastNonFullScreenPos.getHeight();

    return s;
}

bool ResizableWindow::restoreWindowStateFromString (const String& s)
{
    StringArray tokens;
    tokens.addTokens (s, false);
    tokens.removeEmptyStrings();
    tokens.trim();

    const bool fs = tokens[0].startsWithIgnoreCase (T("fs"));
    const int n = fs ? 1 : 0;

    if (tokens.size() != 4 + n)
        return false;

    Rectangle r (tokens[n].getIntValue(),
                 tokens[n + 1].getIntValue(),
                 tokens[n + 2].getIntValue(),
                 tokens[n + 3].getIntValue());

    if (r.isEmpty())
        return false;

    const Rectangle screen (Desktop::getInstance().getMonitorAreaContaining (r.getX(), r.getY()));

    if (! screen.contains (r))
    {
        r.setSize (jmin (r.getWidth(), screen.getWidth()),
                   jmin (r.getHeight(), screen.getHeight()));

        r.setPosition (jlimit (screen.getX(), screen.getRight() - r.getWidth(), r.getX()),
                       jlimit (screen.getY(), screen.getBottom() - r.getHeight(), r.getY()));
    }

    lastNonFullScreenPos = r;

    if (isOnDesktop())
    {
        ComponentPeer* const peer = getPeer();

        if (peer != 0)
            peer->setNonFullScreenBounds (r);
    }

    setFullScreen (fs);

    if (! fs)
        setBoundsConstrained (r.getX(),
                              r.getY(),
                              r.getWidth(),
                              r.getHeight());

    return true;
}

//==============================================================================
void ResizableWindow::mouseDown (const MouseEvent&)
{
    if (! isFullScreen())
        dragger.startDraggingComponent (this, constrainer);
}

void ResizableWindow::mouseDrag (const MouseEvent& e)
{
    if (! isFullScreen())
        dragger.dragComponent (this, e);
}

//==============================================================================
#ifdef JUCE_DEBUG
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
    jassertfalse

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
    jassertfalse

    Component::addAndMakeVisible (child, zOrder);
}
#endif

END_JUCE_NAMESPACE
