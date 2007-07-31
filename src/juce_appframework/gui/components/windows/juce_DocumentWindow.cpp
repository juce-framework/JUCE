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
      titleBarIcon (0),
      menuBar (0),
      menuBarModel (0)
{
    zeromem (titleBarButtons, sizeof (titleBarButtons));

    lookAndFeelChanged();
}

DocumentWindow::~DocumentWindow()
{
    for (int i = 0; i < 3; ++i)
        delete titleBarButtons[i];

    delete titleBarIcon;
    delete menuBar;
}

//==============================================================================
void DocumentWindow::repaintTitleBar()
{
    const int border = getBorderSize();
    repaint (border, border, getWidth() - border * 2, getTitleBarHeight());
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
    deleteAndZero (titleBarIcon);

    if (imageToUse != 0)
        titleBarIcon = imageToUse->createCopy();

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
        delete menuBar;
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

    if (resizableBorder == 0 && getBorderSize() == 1)
    {
        g.setColour (getBackgroundColour().overlaidWith (Colour (0x80000000)));
        g.drawRect (0, 0, getWidth(), getHeight());
    }

    const int border = getBorderSize();

    g.setOrigin (border, border);
    g.reduceClipRegion (0, 0, getWidth() - border * 2, getTitleBarHeight());

    int titleSpaceX1 = 6;
    int titleSpaceX2 = getWidth() - 6;

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

    getLookAndFeel()
        .drawDocumentWindowTitleBar (*this, g,
                                     getWidth() - border * 2,
                                     getTitleBarHeight(),
                                     titleSpaceX1, jmax (1, titleSpaceX2 - titleSpaceX1),
                                     titleBarIcon, ! drawTitleTextCentred);
}

void DocumentWindow::resized()
{
    ResizableWindow::resized();

    if (titleBarButtons[1] != 0)
        titleBarButtons[1]->setToggleState (isFullScreen(), false);

    const int border = getBorderSize();
    getLookAndFeel()
        .positionDocumentWindowButtons (*this,
                                        border, border,
                                        getWidth() - border * 2, getTitleBarHeight(),
                                        titleBarButtons[0],
                                        titleBarButtons[1],
                                        titleBarButtons[2],
                                        positionTitleBarButtonsOnLeft);

    if (menuBar != 0)
        menuBar->setBounds (border, border + getTitleBarHeight(),
                            getWidth() - border * 2, menuBarHeight);
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
    for (i = 0; i < 3; ++i)
        deleteAndZero (titleBarButtons[i]);

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

void DocumentWindow::activeWindowStatusChanged()
{
    ResizableWindow::activeWindowStatusChanged();

    for (int i = 0; i < 3; ++i)
        if (titleBarButtons[i] != 0)
            titleBarButtons[i]->setEnabled (isActiveWindow());

    if (menuBar != 0)
        menuBar->setEnabled (isActiveWindow());
}

const BorderSize DocumentWindow::getBorderThickness()
{
    return BorderSize (getBorderSize());
}

const BorderSize DocumentWindow::getContentComponentBorder()
{
    const int size = getBorderSize();

    return BorderSize (size
                        + (isUsingNativeTitleBar() ? 0 : titleBarHeight)
                        + (menuBar != 0 ? menuBarHeight : 0),
                       size, size, size);
}

void DocumentWindow::mouseDoubleClick (const MouseEvent& e)
{
    const int border = getBorderSize();

    if (e.x >= border
         && e.y >= border
         && e.x < getWidth() - border
         && e.y < border + getTitleBarHeight()
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
int DocumentWindow::getTitleBarHeight() const
{
    return isUsingNativeTitleBar() ? 0 : jmin (titleBarHeight, getHeight() - 4);
}

int DocumentWindow::getBorderSize() const
{
    return (isFullScreen() || isUsingNativeTitleBar()) ? 0 : (resizableBorder != 0 ? 4 : 1);
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
