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

#ifndef __JUCE_ARROWBUTTON_JUCEHEADER__
#define __JUCE_ARROWBUTTON_JUCEHEADER__

#include "juce_Button.h"
#include "../../graphics/effects/juce_DropShadowEffect.h"


//==============================================================================
/**
    A button with an arrow in it.

    @see Button
*/
class JUCE_API  ArrowButton  : public Button
{
public:
    //==============================================================================
    /** Creates an ArrowButton.

        @param buttonName       the name to give the button
        @param arrowDirection   the direction the arrow should point in, where 0.0 is
                                pointing right, 0.25 is down, 0.5 is left, 0.75 is up
        @param arrowColour      the colour to use for the arrow
    */
    ArrowButton (const String& buttonName,
                 float arrowDirection,
                 const Colour& arrowColour);

    /** Destructor. */
    ~ArrowButton();


    //==============================================================================
    juce_UseDebuggingNewOperator

protected:
    /** @internal */
    void paintButton (Graphics& g,
                      bool isMouseOverButton,
                      bool isButtonDown);

    /** @internal */
    void buttonStateChanged();

private:
    //==============================================================================
    Colour colour;
    DropShadowEffect shadow;
    Path path;
    int offset;

    ArrowButton (const ArrowButton&);
    const ArrowButton& operator= (const ArrowButton&);
};


#endif   // __JUCE_ARROWBUTTON_JUCEHEADER__
