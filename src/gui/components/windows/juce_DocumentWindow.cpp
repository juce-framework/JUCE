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


#include "juce_DocumentWindow.h"
#include "../lookandfeel/juce_LookAndFeel.h"
#include "../../graphics/imaging/juce_Image.h"


//==============================================================================
DocumentWindow::DocumentWindow (const String& title,
                                const Colour& backgroundColour,
                                const int requiredButtons_,
                                const bool addToDesktop_)
    : ResizableWindow (title, backgroundColour, addToDesktop_),
      titleBarHeight (26),
      menuBarHeight (24),
      requiredButtons (requiredButtons_),
#if JUCE_MAC
      positionTitleBarButtonsOnLeft (true),
#else
      positionTitleBarButtonsOnLeft (false),
#endif
      drawTitleTextCentred (true),
      menuBarModel (0)
{
    setResizeLimits (128, 128, 32768, 32768);

    lookAndFeelChanged();
}

DocumentWindow::~DocumentWindow()
{
    for (int i = numElementsInArray (titleBarButtons); --i >= 0;)
        titleBarButtons[i] = 0;

    menuBar = 0;
}

//==============================================================================
void DocumentWindow::repaintTitleBar()
{
    const Rectangle titleBarArea (getTitleBarArea());
    repaint (titleBarArea.getX(), titleBarArea.getY(),
             titleBarArea.getWidth(), titleBarArea.getHeight());
}

void DocumentWindow::setName (const String& newName)
{
    if (newName != getName())
    {
        Component::setName (newName);
        repaintTitleBar();
    }
}

void DocumentWindow::setIcon (const Image* imageToUse)
{
    titleBarIcon = imageToUse != 0 ? imageToUse->createCopy() : 0;
    repaintTitleBar();
}

void DocumentWindow::setTitleBarHeight (const int newHeight)
{
    titleBarHeight = newHeight;
    resized();
    repaintTitleBar();
}

void DocumentWindow::setTitleBarButtonsRequired (const int requiredButtons_,
                                                 const bool positionTitleBarButtonsOnLeft_)
{
    requiredButtons = requiredButtons_;
    positionTitleBarButtonsOnLeft = positionTitleBarButtonsOnLeft_;
    lookAndFeelChanged();
}

void DocumentWindow::setTitleBarTextCentred (const bool textShouldBeCentred)
{
    drawTitleTextCentred = textShouldBeCentred;
    repaintTitleBar();
}

void DocumentWindow::setMenuBar (MenuBarModel* menuBarModel_,
                                 const int menuBarHeight_)
{
    if (menuBarModel != menuBarModel_)
    {
        menuBar = 0;

        menuBarModel = menuBarModel_;
        menuBarHeight = (menuBarHeight_ > 0) ? menuBarHeight_
                                             : getLookAndFeel().getDefaultMenuBarHeight();

        if (menuBarModel != 0)
        {
            // (call the Component method directly to avoid the assertion in ResizableWindow)
            Component::addAndMakeVisible (menuBar = new MenuBarComponent (menuBarModel));
            menuBar->setEnabled (isActiveWindow());
        }

        resized();
    }
}


//==============================================================================
void DocumentWindow::closeButtonPressed()
{
    /*  If you've got a close button, you have to override this method to get
        rid of your window!

        If the window is just a pop-up, you should override this method and make
        it delete the window in whatever way is appropriate for your app. E.g. you
        might just want to call "delete this".

        If your app is centred around this window such that the whole app should quit when
        the window is closed, then you will probably want to use this method as an opportunity
        to call JUCEApplication::quit(), and leave the window to be deleted later by your
        JUCEApplication::shutdown() method. (Doing it this way means that your window will
        still get cleaned-up if the app is quit by some other means (e.g. a cmd-Q on the mac
        or closing it via the taskbar icon on Windows).
    */
    jassertfalse
}

void DocumentWindow::minimiseButtonPressed()
{
    setMinimised (true);
}

void DocumentWindow::maximiseButtonPressed()
{
    setFullScreen (! isFullScreen());
}

//==============================================================================
void DocumentWindow::paint (Graphics& g)
{
    ResizableWindow::paint (g);

    if (resizableBorder == 0)
    {
        g.setColour (getBackgroundColour().overlaidWith (Colour (0x80000000)));

        const BorderSize border (getBorderThickness());

        g.fillRect (0, 0, getWidth(), border.getTop());
        g.fillRect (0, border.getTop(), border.getLeft(), getHeight() - border.getTopAndBottom());
        g.fillRect (getWidth() - border.getRight(), border.getTop(), border.getRight(), getHeight() - border.getTopAndBottom());
        g.fillRect (0, getHeight() - border.getBottom(), getWidth(), border.getBottom());
    }

    const Rectangle titleBarArea (getTitleBarArea());
    g.setOrigin (titleBarArea.getX(), titleBarArea.getY());
    g.reduceClipRegion (0, 0, titleBarArea.getWidth(), titleBarArea.getHeight());

    int titleSpaceX1 = 6;
    int titleSpaceX2 = titleBarArea.getWidth() - 6;

    for (int i = 0; i < 3; ++i)
    {
        if (titleBarButtons[i] != 0)
        {
            if (positionTitleBarButtonsOnLeft)
                titleSpaceX1 = jmax (titleSpaceX1, titleBarButtons[i]->getRight() + (getWidth() - titleBarButtons[i]->getRight()) / 8);
            else
                titleSpaceX2 = jmin (titleSpaceX2, titleBarButtons[i]->getX() - (titleBarButtons[i]->getX() / 8));
        }
    }

    getLookAndFeel().drawDocumentWindowTitleBar (*this, g,
                                                 titleBarArea.getWidth(),
                                                 titleBarArea.getHeight(),
                                                 titleSpaceX1,
                                                 jmax (1, titleSpaceX2 - titleSpaceX1),
                                                 titleBarIcon,
                                                 ! drawTitleTextCentred);
}

void DocumentWindow::resized()
{
    ResizableWindow::resized();

    if (titleBarButtons[1] != 0)
        titleBarButtons[1]->setToggleState (isFullScreen(), false);

    const Rectangle titleBarArea (getTitleBarArea());

    getLookAndFeel()
        .positionDocumentWindowButtons (*this,
                                        titleBarArea.getX(), titleBarArea.getY(),
                                        titleBarArea.getWidth(), titleBarArea.getHeight(),
                                        titleBarButtons[0],
                                        titleBarButtons[1],
                                        titleBarButtons[2],
                                        positionTitleBarButtonsOnLeft);

    if (menuBar != 0)
        menuBar->setBounds (titleBarArea.getX(), titleBarArea.getBottom(),
                            titleBarArea.getWidth(), menuBarHeight);
}

const BorderSize DocumentWindow::getBorderThickness()
{
    return BorderSize ((isFullScreen() || isUsingNativeTitleBar())
                            ? 0 : (resizableBorder != 0 ? 4 : 1));
}

const BorderSize DocumentWindow::getContentComponentBorder()
{
    BorderSize border (getBorderThickness());

    border.setTop (border.getTop()
                        + (isUsingNativeTitleBar() ? 0 : titleBarHeight)
                        + (menuBar != 0 ? menuBarHeight : 0));

    return border;
}

int DocumentWindow::getTitleBarHeight() const
{
    return isUsingNativeTitleBar() ? 0 : jmin (titleBarHeight, getHeight() - 4);
}

const Rectangle DocumentWindow::getTitleBarArea()
{
    const BorderSize border (getBorderThickness());

    return Rectangle (border.getLeft(), border.getTop(),
                      getWidth() - border.getLeftAndRight(),
                      getTitleBarHeight());
}

Button* DocumentWindow::getCloseButton() const throw()
{
    return titleBarButtons[2];
}

Button* DocumentWindow::getMinimiseButton() const throw()
{
    return titleBarButtons[0];
}

Button* DocumentWindow::getMaximiseButton() const throw()
{
    return titleBarButtons[1];
}

int DocumentWindow::getDesktopWindowStyleFlags() const
{
    int flags = ResizableWindow::getDesktopWindowStyleFlags();

    if ((requiredButtons & minimiseButton) != 0)
        flags |= ComponentPeer::windowHasMinimiseButton;

    if ((requiredButtons & maximiseButton) != 0)
        flags |= ComponentPeer::windowHasMaximiseButton;

    if ((requiredButtons & closeButton) != 0)
        flags |= ComponentPeer::windowHasCloseButton;

    return flags;
}

void DocumentWindow::lookAndFeelChanged()
{
    int i;
    for (i = numElementsInArray (titleBarButtons); --i >= 0;)
        titleBarButtons[i] = 0;

    if (! isUsingNativeTitleBar())
    {
        titleBarButtons[0] = ((requiredButtons & minimiseButton) != 0)
                                ? getLookAndFeel().createDocumentWindowButton (minimiseButton) : 0;

        titleBarButtons[1] = ((requiredButtons & maximiseButton) != 0)
                                ? getLookAndFeel().createDocumentWindowButton (maximiseButton) : 0;

        titleBarButtons[2] = ((requiredButtons & closeButton) != 0)
                                ? getLookAndFeel().createDocumentWindowButton (closeButton) : 0;

        for (i = 0; i < 3; ++i)
        {
            if (titleBarButtons[i] != 0)
            {
                buttonListener.owner = this;
                titleBarButtons[i]->addButtonListener (&buttonListener);
                titleBarButtons[i]->setWantsKeyboardFocus (false);

                // (call the Component method directly to avoid the assertion in ResizableWindow)
                Component::addAndMakeVisible (titleBarButtons[i]);
            }
        }

        if (getCloseButton() != 0)
        {
#if JUCE_MAC
            getCloseButton()->addShortcut (KeyPress (T('w'), ModifierKeys::commandModifier, 0));
#else
            getCloseButton()->addShortcut (KeyPress (KeyPress::F4Key, ModifierKeys::altModifier, 0));
#endif
        }
    }

    activeWindowStatusChanged();

    ResizableWindow::lookAndFeelChanged();
}

void DocumentWindow::parentHierarchyChanged()
{
    lookAndFeelChanged();
}

void DocumentWindow::activeWindowStatusChanged()
{
    ResizableWindow::activeWindowStatusChanged();

    for (int i = numElementsInArray (titleBarButtons); --i >= 0;)
        if (titleBarButtons[i] != 0)
            titleBarButtons[i]->setEnabled (isActiveWindow());

    if (menuBar != 0)
        menuBar->setEnabled (isActiveWindow());
}

void DocumentWindow::mouseDoubleClick (const MouseEvent& e)
{
    if (getTitleBarArea().contains (e.x, e.y)
         && getMaximiseButton() != 0)
    {
        getMaximiseButton()->triggerClick();
    }
}

void DocumentWindow::userTriedToCloseWindow()
{
    closeButtonPressed();
}

//==============================================================================
DocumentWindow::ButtonListenerProxy::ButtonListenerProxy()
{
}

void DocumentWindow::ButtonListenerProxy::buttonClicked (Button* button)
{
    if (button == owner->getMinimiseButton())
    {
        owner->minimiseButtonPressed();
    }
    else if (button == owner->getMaximiseButton())
    {
        owner->maximiseButtonPressed();
    }
    else if (button == owner->getCloseButton())
    {
        owner->closeButtonPressed();
    }
}


END_JUCE_NAMESPACE
