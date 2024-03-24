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
    A button showing an underlined weblink, that will launch the link
    when it's clicked.

    @see Button

    @tags{GUI}
*/
class JUCE_API  HyperlinkButton  : public Button
{
public:
    //==============================================================================
    /** Creates a HyperlinkButton.

        @param linkText     the text that will be displayed in the button - this is
                            also set as the Component's name, but the text can be
                            changed later with the Button::setButtonText() method
        @param linkURL      the URL to launch when the user clicks the button
    */
    HyperlinkButton (const String& linkText,
                     const URL& linkURL);

    /** Creates a HyperlinkButton. */
    HyperlinkButton();

    /** Destructor. */
    ~HyperlinkButton() override;

    //==============================================================================
    /** Changes the font to use for the text.

        If resizeToMatchComponentHeight is true, the font's height will be adjusted
        to match the size of the component.
    */
    void setFont (const Font& newFont,
                  bool resizeToMatchComponentHeight,
                  Justification justificationType = Justification::horizontallyCentred);

    //==============================================================================
    /** A set of colour IDs to use to change the colour of various aspects of the link.

        These constants can be used either via the Component::setColour(), or LookAndFeel::setColour()
        methods.

        @see Component::setColour, Component::findColour, LookAndFeel::setColour, LookAndFeel::findColour
    */
    enum ColourIds
    {
        textColourId             = 0x1001f00, /**< The colour to use for the URL text. */
    };

    //==============================================================================
    /** Changes the URL that the button will trigger. */
    void setURL (const URL& newURL) noexcept;

    /** Returns the URL that the button will trigger. */
    const URL& getURL() const noexcept                          { return url; }

    //==============================================================================
    /** Resizes the button horizontally to fit snugly around the text.

        This won't affect the button's height.
    */
    void changeWidthToFitText();

    //==============================================================================
    /** Sets the style of justification to be used for positioning the text.
        (The default is Justification::centred)
    */
    void setJustificationType (Justification justification);

    /** Returns the type of justification, as set in setJustificationType(). */
    Justification getJustificationType() const noexcept         { return justification; }

    /** @internal */
    std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override;

protected:
    //==============================================================================
    /** @internal */
    void clicked() override;
    /** @internal */
    void colourChanged() override;
    /** @internal */
    void paintButton (Graphics&, bool, bool) override;

private:
    //==============================================================================
    using Button::clicked;
    Font getFontToUse() const;

    //==============================================================================
    URL url;
    Font font;
    bool resizeFont;
    Justification justification;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HyperlinkButton)
};

} // namespace juce
