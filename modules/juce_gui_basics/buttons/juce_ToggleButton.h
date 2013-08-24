/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

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

#ifndef JUCE_TOGGLEBUTTON_H_INCLUDED
#define JUCE_TOGGLEBUTTON_H_INCLUDED


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
    /** Creates a ToggleButton. */
    ToggleButton();

    /** Creates a ToggleButton.

        @param buttonText   the text to put in the button (the component's name is also
                            initially set to this string, but these can be changed later
                            using the setName() and setButtonText() methods)
    */
    explicit ToggleButton (const String& buttonText);

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

protected:
    //==============================================================================
    /** @internal */
    void paintButton (Graphics&, bool isMouseOverButton, bool isButtonDown) override;
    /** @internal */
    void colourChanged() override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ToggleButton)
};


#endif   // JUCE_TOGGLEBUTTON_H_INCLUDED
