/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

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

#ifndef __JUCE_HYPERLINKBUTTON_JUCEHEADER__
#define __JUCE_HYPERLINKBUTTON_JUCEHEADER__

#include "juce_Button.h"


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
                  const Justification& justificationType = Justification::horizontallyCentred);

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
    void clicked();
    /** @internal */
    void colourChanged();
    /** @internal */
    void paintButton (Graphics& g,
                      bool isMouseOverButton,
                      bool isButtonDown);

private:
    //==============================================================================
    URL url;
    Font font;
    bool resizeFont;
    Justification justification;

    Font getFontToUse() const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HyperlinkButton)
};

#endif   // __JUCE_HYPERLINKBUTTON_JUCEHEADER__
