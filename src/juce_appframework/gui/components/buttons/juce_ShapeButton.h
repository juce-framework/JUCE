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

#ifndef __JUCE_SHAPEBUTTON_JUCEHEADER__
#define __JUCE_SHAPEBUTTON_JUCEHEADER__

#include "juce_Button.h"
#include "../../graphics/geometry/juce_Path.h"
#include "../../graphics/effects/juce_DropShadowEffect.h"


//==============================================================================
/**
    A button that contains a filled shape.

    @see Button, ImageButton, TextButton, ArrowButton
*/
class JUCE_API  ShapeButton  : public Button
{
public:
    //==============================================================================
    /** Creates a ShapeButton.

        @param name             a name to give the component - see Component::setName()
        @param normalColour     the colour to fill the shape with when the mouse isn't over
        @param overColour       the colour to use when the mouse is over the shape
        @param downColour       the colour to use when the button is in the pressed-down state
    */
    ShapeButton (const String& name,
                 const Colour& normalColour,
                 const Colour& overColour,
                 const Colour& downColour);

    /** Destructor. */
    ~ShapeButton();

    //==============================================================================
    /** Sets the shape to use.

        @param newShape                 the shape to use
        @param resizeNowToFitThisShape  if true, the button will be resized to fit the shape's bounds
        @param maintainShapeProportions if true, the shape's proportions will be kept fixed when
                                        the button is resized
        @param hasDropShadow            if true, the button will be given a drop-shadow effect
    */
    void setShape (const Path& newShape,
                   const bool resizeNowToFitThisShape,
                   const bool maintainShapeProportions,
                   const bool hasDropShadow);

    /** Set the colours to use for drawing the shape.

        @param normalColour     the colour to fill the shape with when the mouse isn't over
        @param overColour       the colour to use when the mouse is over the shape
        @param downColour       the colour to use when the button is in the pressed-down state
    */
    void setColours (const Colour& normalColour,
                     const Colour& overColour,
                     const Colour& downColour);

    /** Sets up an outline to draw around the shape.

        @param outlineColour        the colour to use
        @param outlineStrokeWidth   the thickness of line to draw
    */
    void setOutline (const Colour& outlineColour,
                     const float outlineStrokeWidth);


    //==============================================================================
    juce_UseDebuggingNewOperator

protected:
    /** @internal */
    void paintButton (Graphics& g,
                      bool isMouseOverButton,
                      bool isButtonDown);

private:
    Colour normalColour, overColour, downColour, outlineColour;
    DropShadowEffect shadow;
    Path shape;
    bool maintainShapeProportions;
    float outlineWidth;

    ShapeButton (const ShapeButton&);
    const ShapeButton& operator= (const ShapeButton&);
};


#endif   // __JUCE_SHAPEBUTTON_JUCEHEADER__
