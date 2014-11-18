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

#ifndef JUCE_TEXTBUTTON_H_INCLUDED
#define JUCE_TEXTBUTTON_H_INCLUDED


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
    /** Creates a TextButton. */
    TextButton();

    /** Creates a TextButton.
        @param buttonName           the text to put in the button (the component's name is also
                                    initially set to this string, but these can be changed later
                                    using the setName() and setButtonText() methods)
    */
    explicit TextButton (const String& buttonName);

    /** Creates a TextButton.
        @param buttonName           the text to put in the button (the component's name is also
                                    initially set to this string, but these can be changed later
                                    using the setName() and setButtonText() methods)
        @param toolTip              an optional string to use as a toolip
    */
    TextButton (const String& buttonName, const String& toolTip);

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
        textColourOffId                 = 0x1000102,  /**< The colour to use for the button's text when the button's toggle state is "off". */
        textColourOnId                  = 0x1000103   /**< The colour to use for the button's text.when the button's toggle state is "on". */
    };

    //==============================================================================
    /** Changes this button's width to fit neatly around its current text, without
        changing its height.
    */
    void changeWidthToFitText();

    /** Resizes the button's width to fit neatly around its current text, and gives it
        the specified height.
    */
    void changeWidthToFitText (int newHeight);

    /** Returns the width that the LookAndFeel suggests would be best for this button if it
        had the given height.
    */
    int getBestWidthForHeight (int buttonHeight);

    //==============================================================================
    /** @internal */
    void paintButton (Graphics&, bool isMouseOverButton, bool isButtonDown) override;
    /** @internal */
    void colourChanged() override;

private:
   #if JUCE_CATCH_DEPRECATED_CODE_MISUSE
    // Note that this method has been removed - instead, see LookAndFeel::getTextButtonFont()
    virtual int getFont() { return 0; }
   #endif

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TextButton)
};


#endif   // JUCE_TEXTBUTTON_H_INCLUDED
