/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

//==============================================================================
/**
    A button that uses the standard lozenge-shaped background with a line of
    text on it.

    @see Button, DrawableButton

    @tags{GUI}
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
        @param toolTip              an optional string to use as a tooltip
    */
    TextButton (const String& buttonName, const String& toolTip);

    /** Destructor. */
    ~TextButton() override;

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
    void paintButton (Graphics&, bool, bool) override;
    /** @internal */
    void colourChanged() override;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TextButton)
};

} // namespace juce
