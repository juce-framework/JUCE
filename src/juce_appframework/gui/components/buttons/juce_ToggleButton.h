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

#ifndef __JUCE_TOGGLEBUTTON_JUCEHEADER__
#define __JUCE_TOGGLEBUTTON_JUCEHEADER__

#include "juce_Button.h"


//==============================================================================
/**
    A button that can be toggled on/off.

    All buttons can be toggle buttons, but this lets you create one of the
    standard ones which has a tick-box and a text label next to it.

    @see Button, DrawableButton, TextButton
*/
class JUCE_API  ToggleButton  : public Button
{
public:
    //==============================================================================
    /** Creates a ToggleButton.

        @param buttonText   the text to put in the button (the component's name is also
                            initially set to this string, but these can be changed later
                            using the setName() and setButtonText() methods)
    */
    ToggleButton (const String& buttonText);

    /** Destructor. */
    ~ToggleButton();

    //==============================================================================
    /** Resizes the button to fit neatly around its current text.

        The button's height won't be affected, only its width.
    */
    void changeWidthToFitText();

    //==============================================================================
    /** A set of colour IDs to use to change the colour of various aspects of the button.

        These constants can be used either via the Component::setColour(), or LookAndFeel::setColour()
        methods.

        @see Component::setColour, Component::findColour, LookAndFeel::setColour, LookAndFeel::findColour
    */
    enum ColourIds
    {
        textColourId                    = 0x1006501   /**< The colour to use for the button's text. */
    };

    //==============================================================================
    juce_UseDebuggingNewOperator

protected:
    /** @internal */
    void paintButton (Graphics& g,
                      bool isMouseOverButton,
                      bool isButtonDown);

    /** @internal */
    void colourChanged();

private:
    //==============================================================================
    ToggleButton (const ToggleButton&);
    const ToggleButton& operator= (const ToggleButton&);
};


#endif   // __JUCE_TOGGLEBUTTON_JUCEHEADER__
