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

#ifndef JUCE_HYPERLINKBUTTON_H_INCLUDED
#define JUCE_HYPERLINKBUTTON_H_INCLUDED


//==============================================================================
/**
    A button showing an underlined weblink, that will launch the link
    when it's clicked.

    @see Button
*/
class JUCE_API  HyperlinkButton  : public Button
{
public:
    //==============================================================================
    /** Creates a HyperlinkButton.

        @param linkText     the text that will be displayed in the button - this is
                            also set as the Component's name, but the text can be
                            changed later with the Button::getButtonText() method
        @param linkURL      the URL to launch when the user clicks the button
    */
    HyperlinkButton (const String& linkText,
                     const URL& linkURL);

    /** Creates a HyperlinkButton. */
    HyperlinkButton();

    /** Destructor. */
    ~HyperlinkButton();

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

protected:
    //==============================================================================
    /** @internal */
    void clicked() override;
    /** @internal */
    void colourChanged() override;
    /** @internal */
    void paintButton (Graphics&, bool isMouseOver, bool isButtonDown) override;

private:
    //==============================================================================
    URL url;
    Font font;
    bool resizeFont;
    Justification justification;

    Font getFontToUse() const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HyperlinkButton)
};

#endif   // JUCE_HYPERLINKBUTTON_H_INCLUDED
