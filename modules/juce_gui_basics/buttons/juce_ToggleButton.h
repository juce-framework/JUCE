/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-license
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

//==============================================================================
/**
    A button that can be toggled on/off.

    All buttons can be toggle buttons, but this lets you create one of the
    standard ones which has a tick-box and a text label next to it.

    @see Button, DrawableButton, TextButton

    @tags{GUI}
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
    /** A set of color IDs to use to change the color of various aspects of the button.

        These constants can be used either via the Component::setColor(), or LookAndFeel::setColor()
        methods.

        @see Component::setColor, Component::findColor, LookAndFeel::setColor, LookAndFeel::findColor
    */
    enum ColorIds
    {
        textColorId            = 0x1006501,  /**< The color to use for the button's text. */
        tickColorId            = 0x1006502,  /**< The color to use for the tick mark. */
        tickDisabledColorId    = 0x1006503   /**< The color to use for the disabled tick mark. */
    };

protected:
    //==============================================================================
    /** @internal */
    void paintButton (Graphics&, bool isMouseOverButton, bool isButtonDown) override;
    /** @internal */
    void colorChanged() override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ToggleButton)
};

} // namespace juce
