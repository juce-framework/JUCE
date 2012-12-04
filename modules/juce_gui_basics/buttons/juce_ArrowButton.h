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

#ifndef __JUCE_ARROWBUTTON_JUCEHEADER__
#define __JUCE_ARROWBUTTON_JUCEHEADER__

#include "juce_Button.h"


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

    /** @internal */
    void paintButton (Graphics&, bool isMouseOverButton, bool isButtonDown);

private:
    Colour colour;
    Path path;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ArrowButton)
};


#endif   // __JUCE_ARROWBUTTON_JUCEHEADER__
