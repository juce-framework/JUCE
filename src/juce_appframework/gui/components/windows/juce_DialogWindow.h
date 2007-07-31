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

#ifndef __JUCE_DIALOGWINDOW_JUCEHEADER__
#define __JUCE_DIALOGWINDOW_JUCEHEADER__

#include "juce_DocumentWindow.h"
#include "../buttons/juce_Button.h"


//==============================================================================
/**
    A dialog-box style window.

    This class is a convenient way of creating a DocumentWindow with a close button
    that can be triggered by pressing the escape key.

    Any of the methods available to a DocumentWindow or ResizableWindow are also
    available to this, so it can be made resizable, have a menu bar, etc.

    To add items to the box, see the ResizableWindow::setContentComponent() method.
    Don't add components directly to this class - always put them in a content component!

    You'll need to override the DocumentWindow::closeButtonPressed() method to handle
    the user clicking the close button - for more info, see the DocumentWindow
    help.

    @see DocumentWindow, ResizableWindow
*/
class JUCE_API  DialogWindow   : public DocumentWindow
{
public:
    //==============================================================================
    /** Creates a DialogWindow.

        @param name                 the name to give the component - this is also
                                    the title shown at the top of the window. To change
                                    this later, use setName()
        @param backgroundColour     the colour to use for filling the window's background.
        @param escapeKeyTriggersCloseButton if true, then pressing the escape key will cause the
                                            close button to be triggered
        @param addToDesktop         if true, the window will be automatically added to the
                                    desktop; if false, you can use it as a child component
    */
    DialogWindow (const String& name,
                  const Colour& backgroundColour,
                  const bool escapeKeyTriggersCloseButton,
                  const bool addToDesktop = true);

    /** Destructor.

        If a content component has been set with setContentComponent(), it
        will be deleted.
    */
    ~DialogWindow();

    //==============================================================================
    /** Easy way of quickly showing a dialog box containing a given component.

        This will open and display a DialogWindow containing a given component, returning
        when the user clicks its close button.

        @param dialogTitle          the dialog box's title
        @param contentComponent     the content component for the dialog box. Make sure
                                    that this has been set to the size you want it to
                                    be before calling this method. The component won't
                                    be deleted by this call, so you can re-use it or delete
                                    it afterwards
        @param componentToCentreAround  if this is non-zero, it indicates a component that
                                    you'd like to show this dialog box in front of. See the
                                    DocumentWindow::centreAroundComponent() method for more
                                    info on this parameter
        @param backgroundColour     a colour to use for the dialog box's background colour
        @param escapeKeyTriggersCloseButton if true, then pressing the escape key will cause the
                                            close button to be triggered
        @param shouldBeResizable    if true, the dialog window has either a resizable border, or
                                    a corner resizer
        @param useBottomRightCornerResizer     if shouldBeResizable is true, this indicates whether
                                    to use a border or corner resizer component. See ResizableWindow::setResizable()
    */
    static void showModalDialog (const String& dialogTitle,
                                 Component* contentComponent,
                                 Component* componentToCentreAround,
                                 const Colour& backgroundColour,
                                 const bool escapeKeyTriggersCloseButton,
                                 const bool shouldBeResizable = false,
                                 const bool useBottomRightCornerResizer = false);

    //==============================================================================
    juce_UseDebuggingNewOperator

protected:
    /** @internal */
    void resized();

private:
    bool escapeKeyTriggersCloseButton;

    DialogWindow (const DialogWindow&);
    const DialogWindow& operator= (const DialogWindow&);
};

#endif   // __JUCE_DIALOGWINDOW_JUCEHEADER__
