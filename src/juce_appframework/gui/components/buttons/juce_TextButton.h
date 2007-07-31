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

#ifndef __JUCE_TEXTBUTTON_JUCEHEADER__
#define __JUCE_TEXTBUTTON_JUCEHEADER__

#include "juce_Button.h"


//==============================================================================
/**
    A button that uses the standard lozenge-shaped background with a line of
    text on it.

    @see Button, DrawableButton
*/
class JUCE_API  TextButton  : public Button
{
public:
    //==============================================================================
    /** Creates a TextButton.

        @param buttonName           the text to put in the button (the component's name is also
                                    initially set to this string, but these can be changed later
                                    using the setName() and setButtonText() methods)
        @param toolTip              an optional string to use as a toolip

        @see Button
    */
    TextButton (const String& buttonName,
                const String& toolTip = String::empty);

    /** Destructor. */
    ~TextButton();

    //==============================================================================
    /** A set of colour IDs to use to change the colour of various aspects of the button.

        These constants can be used either via the Component::setColour(), or LookAndFeel::setColour()
        methods.

        @see Component::setColour, Component::findColour, LookAndFeel::setColour, LookAndFeel::findColour
    */
    enum ColourIds
    {
        buttonColourId                  = 0x1000100,  /**< The colour used to fill the button shape (when the button is toggled
                                                           'off'). The look-and-feel class might re-interpret this to add
                                                           effects, etc. */
        buttonOnColourId                = 0x1000101,  /**< The colour used to fill the button shape (when the button is toggled
                                                           'on'). The look-and-feel class might re-interpret this to add
                                                           effects, etc. */
        textColourId                    = 0x1000102   /**< The colour to use for the button's text. */
    };

    //==============================================================================
    /** Resizes the button to fit neatly around its current text.

        If newHeight is >= 0, the button's height will be changed to this
        value. If it's less than zero, its height will be unaffected.
    */
    void changeWidthToFitText (const int newHeight = -1);

    /** This can be overridden to use different fonts than the default one.

        Note that you'll need to set the font's size appropriately, too.
    */
    virtual const Font getFont();


    //==============================================================================
    juce_UseDebuggingNewOperator

protected:
    /** @internal */
    void paintButton (Graphics& g, bool isMouseOverButton, bool isButtonDown);
    /** @internal */
    void colourChanged();

private:
    TextButton (const TextButton&);
    const TextButton& operator= (const TextButton&);
};


#endif   // __JUCE_TEXTBUTTON_JUCEHEADER__
