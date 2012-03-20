/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

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

DialogWindow::DialogWindow (const String& name,
                            const Colour& backgroundColour_,
                            const bool escapeKeyTriggersCloseButton_,
                            const bool addToDesktop_)
    : DocumentWindow (name, backgroundColour_, DocumentWindow::closeButton, addToDesktop_),
      escapeKeyTriggersCloseButton (escapeKeyTriggersCloseButton_)
{
}

DialogWindow::~DialogWindow()
{
}

//==============================================================================
void DialogWindow::resized()
{
    DocumentWindow::resized();

    const KeyPress esc (KeyPress::escapeKey, 0, 0);

    if (escapeKeyTriggersCloseButton
         && getCloseButton() != nullptr
         && ! getCloseButton()->isRegisteredForShortcut (esc))
    {
        getCloseButton()->addShortcut (esc);
    }
}

//==============================================================================
class TempDialogWindow : public DialogWindow
{
public:
    TempDialogWindow (const String& title,
                      Component* contentComponent_,
                      Component* componentToCentreAround,
                      const Colour& colour,
                      const bool escapeKeyTriggersCloseButton_,
                      const bool shouldBeResizable,
                      const bool useBottomRightCornerResizer)
        : DialogWindow (title, colour, escapeKeyTriggersCloseButton_, true)
    {
        if (! JUCEApplication::isStandaloneApp())
            setAlwaysOnTop (true); // for a plugin, make it always-on-top because the host windows are often top-level

        setContentNonOwned (contentComponent_, true);
        centreAroundComponent (componentToCentreAround, getWidth(), getHeight());
        setResizable (shouldBeResizable, useBottomRightCornerResizer);
    }

    void closeButtonPressed()
    {
        setVisible (false);
    }

private:
    JUCE_DECLARE_NON_COPYABLE (TempDialogWindow);
};


//==============================================================================
void DialogWindow::showDialog (const String& dialogTitle,
                               Component* const contentComponent,
                               Component* const componentToCentreAround,
                               const Colour& backgroundColour,
                               const bool escapeKeyTriggersCloseButton,
                               const bool shouldBeResizable,
                               const bool useBottomRightCornerResizer)
{
    TempDialogWindow* dw = new TempDialogWindow (dialogTitle, contentComponent, componentToCentreAround,
                                                 backgroundColour, escapeKeyTriggersCloseButton,
                                                 shouldBeResizable, useBottomRightCornerResizer);

    dw->enterModalState (true, 0, true);
}

#if JUCE_MODAL_LOOPS_PERMITTED
int DialogWindow::showModalDialog (const String& dialogTitle,
                                   Component* const contentComponent,
                                   Component* const componentToCentreAround,
                                   const Colour& backgroundColour,
                                   const bool escapeKeyTriggersCloseButton,
                                   const bool shouldBeResizable,
                                   const bool useBottomRightCornerResizer)
{
    TempDialogWindow dw (dialogTitle, contentComponent, componentToCentreAround,
                         backgroundColour, escapeKeyTriggersCloseButton,
                         shouldBeResizable, useBottomRightCornerResizer);

    return dw.runModalLoop();
}
#endif
