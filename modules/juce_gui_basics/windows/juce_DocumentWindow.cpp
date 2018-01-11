/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

class DocumentWindow::ButtonListenerProxy  : public Button::Listener
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
      requiredButtons (requiredButtons_),
     #if JUCE_MAC
      positionTitleBarButtonsOnLeft (true)
     #else
      positionTitleBarButtonsOnLeft (false)
     #endif
{
    setResizeLimits (128, 128, 32768, 32768);

    DocumentWindow::lookAndFeelChanged();
}

DocumentWindow::~DocumentWindow()
{
    // Don't delete or remove the resizer components yourself! They're managed by the
    // DocumentWindow, and you should leave them alone! You may have deleted them
    // accidentally by careless use of deleteAllChildren()..?
    jassert (menuBar == nullptr || getIndexOfChildComponent (menuBar.get()) >= 0);
    jassert (titleBarButtons[0] == nullptr || getIndexOfChildComponent (titleBarButtons[0].get()) >= 0);
    jassert (titleBarButtons[1] == nullptr || getIndexOfChildComponent (titleBarButtons[1].get()) >= 0);
    jassert (titleBarButtons[2] == nullptr || getIndexOfChildComponent (titleBarButtons[2].get()) >= 0);

    for (auto& b : titleBarButtons)
        b.reset();

    menuBar.reset();
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
        menuBar.reset();

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
    return menuBar.get();
}

void DocumentWindow::setMenuBarComponent (Component* newMenuBarComponent)
{
    menuBar.reset (newMenuBarComponent);
    Component::addAndMakeVisible (menuBar.get()); // (call the superclass method directly to avoid the assertion in ResizableWindow)

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

    auto titleBarArea = getTitleBarArea();
    g.reduceClipRegion (titleBarArea);
    g.setOrigin (titleBarArea.getPosition());

    int titleSpaceX1 = 6;
    int titleSpaceX2 = titleBarArea.getWidth() - 6;

    for (auto& b : titleBarButtons)
    {
        if (b != nullptr)
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

    if (auto* b = getMaximiseButton())
        b->setToggleState (isFullScreen(), dontSendNotification);

    auto titleBarArea = getTitleBarArea();

    getLookAndFeel()
        .positionDocumentWindowButtons (*this,
                                        titleBarArea.getX(), titleBarArea.getY(),
                                        titleBarArea.getWidth(), titleBarArea.getHeight(),
                                        titleBarButtons[0].get(),
                                        titleBarButtons[1].get(),
                                        titleBarButtons[2].get(),
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
    auto border = getBorderThickness();

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
    if (isKioskMode())
        return {};

    auto border = getBorderThickness();
    return { border.getLeft(), border.getTop(), getWidth() - border.getLeftAndRight(), getTitleBarHeight() };
}

Button* DocumentWindow::getCloseButton()    const noexcept  { return titleBarButtons[2].get(); }
Button* DocumentWindow::getMinimiseButton() const noexcept  { return titleBarButtons[0].get(); }
Button* DocumentWindow::getMaximiseButton() const noexcept  { return titleBarButtons[1].get(); }

int DocumentWindow::getDesktopWindowStyleFlags() const
{
    auto styleFlags = ResizableWindow::getDesktopWindowStyleFlags();

    if ((requiredButtons & minimiseButton) != 0)  styleFlags |= ComponentPeer::windowHasMinimiseButton;
    if ((requiredButtons & maximiseButton) != 0)  styleFlags |= ComponentPeer::windowHasMaximiseButton;
    if ((requiredButtons & closeButton)    != 0)  styleFlags |= ComponentPeer::windowHasCloseButton;

    return styleFlags;
}

void DocumentWindow::lookAndFeelChanged()
{
    for (auto& b : titleBarButtons)
        b.reset();

    if (! isUsingNativeTitleBar())
    {
        auto& lf = getLookAndFeel();

        if ((requiredButtons & minimiseButton) != 0)  titleBarButtons[0].reset (lf.createDocumentWindowButton (minimiseButton));
        if ((requiredButtons & maximiseButton) != 0)  titleBarButtons[1].reset (lf.createDocumentWindowButton (maximiseButton));
        if ((requiredButtons & closeButton)    != 0)  titleBarButtons[2].reset (lf.createDocumentWindowButton (closeButton));

        for (auto& b : titleBarButtons)
        {
            if (b != nullptr)
            {
                if (buttonListener == nullptr)
                    buttonListener.reset (new ButtonListenerProxy (*this));

                b->addListener (buttonListener.get());
                b->setWantsKeyboardFocus (false);

                // (call the Component method directly to avoid the assertion in ResizableWindow)
                Component::addAndMakeVisible (b.get());
            }
        }

        if (auto* b = getCloseButton())
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
    bool isActive = isActiveWindow();

    for (auto& b : titleBarButtons)
        if (b != nullptr)
            b->setEnabled (isActive);

    if (menuBar != nullptr)
        menuBar->setEnabled (isActive);
}

void DocumentWindow::mouseDoubleClick (const MouseEvent& e)
{
    if (getTitleBarArea().contains (e.x, e.y))
        if (auto* maximise = getMaximiseButton())
            maximise->triggerClick();
}

void DocumentWindow::userTriedToCloseWindow()
{
    closeButtonPressed();
}

} // namespace juce
