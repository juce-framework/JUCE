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


#include "juce_DialogWindow.h"


//==============================================================================
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
         && getCloseButton() != 0
         && ! getCloseButton()->isRegisteredForShortcut (esc))
    {
        getCloseButton()->addShortcut (esc);
    }
}


//==============================================================================
class TempDialogWindow : public DialogWindow
{
public:
    TempDialogWindow (const String& title, const Colour& colour, const bool escapeCloses)
        : DialogWindow (title, colour, escapeCloses, true)
    {
    }

    ~TempDialogWindow()
    {
    }

    void closeButtonPressed()
    {
        setVisible (false);
    }

private:
    TempDialogWindow (const TempDialogWindow&);
    const TempDialogWindow& operator= (const TempDialogWindow&);

};

int DialogWindow::showModalDialog (const String& dialogTitle,
                                   Component* contentComponent,
                                   Component* componentToCentreAround,
                                   const Colour& colour,
                                   const bool escapeKeyTriggersCloseButton,
                                   const bool shouldBeResizable,
                                   const bool useBottomRightCornerResizer)
{
    TempDialogWindow dw (dialogTitle, colour, escapeKeyTriggersCloseButton);

    dw.setContentComponent (contentComponent, true, true);
    dw.centreAroundComponent (componentToCentreAround, dw.getWidth(), dw.getHeight());
    dw.setResizable (shouldBeResizable, useBottomRightCornerResizer);
    const int result = dw.runModalLoop();
    dw.setContentComponent (0, false);
    return result;
}


END_JUCE_NAMESPACE
