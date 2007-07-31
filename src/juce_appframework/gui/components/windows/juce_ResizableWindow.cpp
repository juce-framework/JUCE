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


#include "juce_ResizableWindow.h"
#include "../juce_Desktop.h"
#include "../lookandfeel/juce_LookAndFeel.h"
#include "../../graphics/geometry/juce_RectangleList.h"


//==============================================================================
ResizableWindow::ResizableWindow (const String& name,
                                  const Colour& backgroundColour_,
                                  const bool addToDesktop_)
    : TopLevelWindow (name, addToDesktop_),
      resizableCorner (0),
      resizableBorder (0),
      contentComponent (0),
      resizeToFitContent (false),
      fullscreen (false),
      constrainer (&defaultConstrainer)
#ifdef JUCE_DEBUG
      , hasBeenResized (false)
#endif
{
    setBackgroundColour (backgroundColour_);

    const Rectangle mainMonArea (Desktop::getInstance().getMainMonitorArea());

    defaultConstrainer.setSizeLimits (200, 200,
                                      mainMonArea.getWidth(),
                                      mainMonArea.getHeight());

    defaultConstrainer.setMinimumOnscreenAmounts (0x10000, 16, 24, 16);

    lastNonFullScreenPos.setBounds (50, 50,
                                    defaultConstrainer.getMinimumWidth(),
                                    defaultConstrainer.getMinimumHeight());

    if (addToDesktop_)
        Component::addToDesktop (getDesktopWindowStyleFlags());
}

ResizableWindow::~ResizableWindow()
{
    deleteAndZero (resizableCorner);
    deleteAndZero (resizableBorder);
    deleteAndZero (contentComponent);

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

    if (contentComponent != newContentComponent)
    {
        if (deleteOldOne)
            delete contentComponent;
        else
            removeChildComponent (contentComponent);

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
            deleteAndZero (resizableBorder);

            if (resizableCorner == 0)
            {
                Component::addChildComponent (resizableCorner = new ResizableCornerComponent (this, constrainer));
                resizableCorner->setAlwaysOnTop (true);
            }
        }
        else
        {
            deleteAndZero (resizableCorner);

            if (resizableBorder == 0)
                Component::addChildComponent (resizableBorder = new ResizableBorderComponent (this, constrainer));
        }
    }
    else
    {
        deleteAndZero (resizableCorner);
        deleteAndZero (resizableBorder);
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
    jassert (constrainer == &defaultConstrainer);

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

        deleteAndZero (resizableCorner);
        deleteAndZero (resizableBorder);

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
    g.fillAll (backgroundColour);

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

void ResizableWindow::setBackgroundColour (const Colour& newColour)
{
    if (Desktop::canUseSemiTransparentWindows())
        backgroundColour = newColour;
    else
        backgroundColour = newColour.withAlpha (1.0f);

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

    const Rectangle r (tokens[n].getIntValue(),
                       tokens[n + 1].getIntValue(),
                       tokens[n + 2].getIntValue(),
                       tokens[n + 3].getIntValue());

    if (r.isEmpty())
        return false;

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
