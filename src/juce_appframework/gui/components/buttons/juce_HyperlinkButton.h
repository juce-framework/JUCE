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

#ifndef __JUCE_HYPERLINKBUTTON_JUCEHEADER__
#define __JUCE_HYPERLINKBUTTON_JUCEHEADER__

#include "juce_Button.h"
#include "../../../../juce_core/io/network/juce_URL.h"


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

    /** Destructor. */
    ~HyperlinkButton();

    //==============================================================================
    /** Changes the font to use for the text.

        If resizeToMatchComponentHeight is true, the font's height will be adjusted
        to match the size of the component.
    */
    void setFont (const Font& newFont,
                  const bool resizeToMatchComponentHeight,
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
    void setURL (const URL& newURL) throw();

    /** Returns the URL that the button will trigger. */
    const URL& getURL() const throw()                           { return url; }

    //==============================================================================
    /** Resizes the button horizontally to fit snugly around the text.

        This won't affect the button's height.
    */
    void changeWidthToFitText();


    //==============================================================================
    juce_UseDebuggingNewOperator

protected:
    /** @internal */
    void clicked();
    /** @internal */
    void colourChanged();
    /** @internal */
    void paintButton (Graphics& g,
                      bool isMouseOverButton,
                      bool isButtonDown);

private:
    URL url;
    Font font;
    bool resizeFont;
    Justification justification;

    const Font getFontToUse() const;

    HyperlinkButton (const HyperlinkButton&);
    const HyperlinkButton& operator= (const HyperlinkButton&);
};

#endif   // __JUCE_HYPERLINKBUTTON_JUCEHEADER__
