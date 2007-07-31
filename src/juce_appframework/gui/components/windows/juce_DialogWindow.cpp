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

void DialogWindow::showModalDialog (const String& dialogTitle,
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
    dw.runModalLoop();
    dw.setContentComponent (0, false);
}


END_JUCE_NAMESPACE
