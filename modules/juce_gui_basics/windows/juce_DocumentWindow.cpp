/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

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

class DocumentWindow::ButtonListenerProxy  : public ButtonListener // (can't use Button::Listener due to idiotic VC2005 bug)
{
public:
    ButtonListenerProxy (DocumentWindow& w) : owner (w) {}

    void buttonClicked (Button* button) override
    {
        if      (button == owner.getMinimiseButton())  owner.minimiseButtonPressed();
        else if (button == owner.getMaximiseButton())  owner.maximiseButtonPressed();
        else if (button == owner.getCloseButton())     owner.closeButtonPressed();
    }

private:
    DocumentWindow& owner;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ButtonListenerProxy)
};

//==============================================================================
DocumentWindow::DocumentWindow (const String& title,
                                Colour backgroundColour,
                                int requiredButtons_,
                                bool addToDesktop_)
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
      menuBarModel (nullptr)
{
    setResizeLimits (128, 128, 32768, 32768);

    DocumentWindow::lookAndFeelChanged();
}

DocumentWindow::~DocumentWindow()
{
    // Don't delete or remove the resizer components yourself! They're managed by the
    // DocumentWindow, and you should leave them alone! You may have deleted them
    // accidentally by careless use of deleteAllChildren()..?
    jassert (menuBar == nullptr || getIndexOfChildComponent (menuBar) >= 0);
    jassert (titleBarButtons[0] == nullptr || getIndexOfChildComponent (titleBarButtons[0]) >= 0);
    jassert (titleBarButtons[1] == nullptr || getIndexOfChildComponent (titleBarButtons[1]) >= 0);
    jassert (titleBarButtons[2] == nullptr || getIndexOfChildComponent (titleBarButtons[2]) >= 0);

    for (int i = numElementsInArray (titleBarButtons); --i >= 0;)
        titleBarButtons[i] = nullptr;

    menuBar = nullptr;
}

//==============================================================================
void DocumentWindow::repaintTitleBar()
{
    repaint (getTitleBarArea());
}

void DocumentWindow::setName (const String& newName)
{
    if (newName != getName())
    {
        Component::setName (newName);
        repaintTitleBar();
    }
}

void DocumentWindow::setIcon (const Image& imageToUse)
{
    titleBarIcon = imageToUse;
    repaintTitleBar();
}

void DocumentWindow::setTitleBarHeight (const int newHeight)
{
    titleBarHeight = newHeight;
    resized();
    repaintTitleBar();
}

void DocumentWindow::setTitleBarButtonsRequired (const int buttons, const bool onLeft)
{
    requiredButtons = buttons;
    positionTitleBarButtonsOnLeft = onLeft;
    lookAndFeelChanged();
}

void DocumentWindow::setTitleBarTextCentred (const bool textShouldBeCentred)
{
    drawTitleTextCentred = textShouldBeCentred;
    repaintTitleBar();
}

//==============================================================================
void DocumentWindow::setMenuBar (MenuBarModel* newMenuBarModel, const int newMenuBarHeight)
{
    if (menuBarModel != newMenuBarModel)
    {
        menuBar = nullptr;

        menuBarModel = newMenuBarModel;
        menuBarHeight = newMenuBarHeight > 0 ? newMenuBarHeight
                                             : getLookAndFeel().getDefaultMenuBarHeight();

        if (menuBarModel != nullptr)
            setMenuBarComponent (new MenuBarComponent (menuBarModel));

        resized();
    }
}

Component* DocumentWindow::getMenuBarComponent() const noexcept
{
    return menuBar;
}

void DocumentWindow::setMenuBarComponent (Component* newMenuBarComponent)
{
    // (call the Component method directly to avoid the assertion in ResizableWindow)
    Component::addAndMakeVisible (menuBar = newMenuBarComponent);

    if (menuBar != nullptr)
        menuBar->setEnabled (isActiveWindow());

    resized();
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
        to call JUCEApplicationBase::quit(), and leave the window to be deleted later by your
        JUCEApplicationBase::shutdown() method. (Doing it this way means that your window will
        still get cleaned-up if the app is quit by some other means (e.g. a cmd-Q on the mac
        or closing it via the taskbar icon on Windows).
    */
    jassertfalse;
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

    const Rectangle<int> titleBarArea (getTitleBarArea());
    g.reduceClipRegion (titleBarArea);
    g.setOrigin (titleBarArea.getPosition());

    int titleSpaceX1 = 6;
    int titleSpaceX2 = titleBarArea.getWidth() - 6;

    for (int i = 0; i < 3; ++i)
    {
        if (Button* const b = titleBarButtons[i])
        {
            if (positionTitleBarButtonsOnLeft)
                titleSpaceX1 = jmax (titleSpaceX1, b->getRight() + (getWidth() - b->getRight()) / 8);
            else
                titleSpaceX2 = jmin (titleSpaceX2, b->getX() - (b->getX() / 8));
        }
    }

    getLookAndFeel().drawDocumentWindowTitleBar (*this, g,
                                                 titleBarArea.getWidth(),
                                                 titleBarArea.getHeight(),
                                                 titleSpaceX1,
                                                 jmax (1, titleSpaceX2 - titleSpaceX1),
                                                 titleBarIcon.isValid() ? &titleBarIcon : 0,
                                                 ! drawTitleTextCentred);
}

void DocumentWindow::resized()
{
    ResizableWindow::resized();

    if (Button* const b = getMaximiseButton())
        b->setToggleState (isFullScreen(), dontSendNotification);

    const Rectangle<int> titleBarArea (getTitleBarArea());

    getLookAndFeel()
        .positionDocumentWindowButtons (*this,
                                        titleBarArea.getX(), titleBarArea.getY(),
                                        titleBarArea.getWidth(), titleBarArea.getHeight(),
                                        titleBarButtons[0],
                                        titleBarButtons[1],
                                        titleBarButtons[2],
                                        positionTitleBarButtonsOnLeft);

    if (menuBar != nullptr)
        menuBar->setBounds (titleBarArea.getX(), titleBarArea.getBottom(),
                            titleBarArea.getWidth(), menuBarHeight);
}

BorderSize<int> DocumentWindow::getBorderThickness()
{
    return ResizableWindow::getBorderThickness();
}

BorderSize<int> DocumentWindow::getContentComponentBorder()
{
    BorderSize<int> border (getBorderThickness());

    if (! isKioskMode())
        border.setTop (border.getTop()
                        + (isUsingNativeTitleBar() ? 0 : titleBarHeight)
                        + (menuBar != nullptr ? menuBarHeight : 0));

    return border;
}

int DocumentWindow::getTitleBarHeight() const
{
    return isUsingNativeTitleBar() ? 0 : jmin (titleBarHeight, getHeight() - 4);
}

Rectangle<int> DocumentWindow::getTitleBarArea()
{
    const BorderSize<int> border (getBorderThickness());

    if (isKioskMode())
        return Rectangle<int>();

    return Rectangle<int> (border.getLeft(), border.getTop(),
                           getWidth() - border.getLeftAndRight(), getTitleBarHeight());
}

Button* DocumentWindow::getCloseButton()    const noexcept  { return titleBarButtons[2]; }
Button* DocumentWindow::getMinimiseButton() const noexcept  { return titleBarButtons[0]; }
Button* DocumentWindow::getMaximiseButton() const noexcept  { return titleBarButtons[1]; }

int DocumentWindow::getDesktopWindowStyleFlags() const
{
    int styleFlags = ResizableWindow::getDesktopWindowStyleFlags();

    if ((requiredButtons & minimiseButton) != 0)  styleFlags |= ComponentPeer::windowHasMinimiseButton;
    if ((requiredButtons & maximiseButton) != 0)  styleFlags |= ComponentPeer::windowHasMaximiseButton;
    if ((requiredButtons & closeButton)    != 0)  styleFlags |= ComponentPeer::windowHasCloseButton;

    return styleFlags;
}

void DocumentWindow::lookAndFeelChanged()
{
    for (int i = numElementsInArray (titleBarButtons); --i >= 0;)
        titleBarButtons[i] = nullptr;

    if (! isUsingNativeTitleBar())
    {
        LookAndFeel& lf = getLookAndFeel();

        if ((requiredButtons & minimiseButton) != 0)  titleBarButtons[0] = lf.createDocumentWindowButton (minimiseButton);
        if ((requiredButtons & maximiseButton) != 0)  titleBarButtons[1] = lf.createDocumentWindowButton (maximiseButton);
        if ((requiredButtons & closeButton)    != 0)  titleBarButtons[2] = lf.createDocumentWindowButton (closeButton);

        for (int i = 0; i < 3; ++i)
        {
            if (Button* const b = titleBarButtons[i])
            {
                if (buttonListener == nullptr)
                    buttonListener = new ButtonListenerProxy (*this);

                b->addListener (buttonListener);
                b->setWantsKeyboardFocus (false);

                // (call the Component method directly to avoid the assertion in ResizableWindow)
                Component::addAndMakeVisible (b);
            }
        }

        if (Button* const b = getCloseButton())
        {
           #if JUCE_MAC
            b->addShortcut (KeyPress ('w', ModifierKeys::commandModifier, 0));
           #else
            b->addShortcut (KeyPress (KeyPress::F4Key, ModifierKeys::altModifier, 0));
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
        if (Button* const b = titleBarButtons[i])
            b->setEnabled (isActiveWindow());

    if (menuBar != nullptr)
        menuBar->setEnabled (isActiveWindow());
}

void DocumentWindow::mouseDoubleClick (const MouseEvent& e)
{
    Button* const maximise = getMaximiseButton();

    if (maximise != nullptr && getTitleBarArea().contains (e.x, e.y))
        maximise->triggerClick();
}

void DocumentWindow::userTriedToCloseWindow()
{
    closeButtonPressed();
}
