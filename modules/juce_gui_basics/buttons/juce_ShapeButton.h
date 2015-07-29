/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

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

#ifndef JUCE_SHAPEBUTTON_H_INCLUDED
#define JUCE_SHAPEBUTTON_H_INCLUDED


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
                 Colour normalColour,
                 Colour overColour,
                 Colour downColour);

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
                   bool resizeNowToFitThisShape,
                   bool maintainShapeProportions,
                   bool hasDropShadow);

    /** Set the colours to use for drawing the shape.

        @param normalColour     the colour to fill the shape with when the mouse isn't over
        @param overColour       the colour to use when the mouse is over the shape
        @param downColour       the colour to use when the button is in the pressed-down state
    */
    void setColours (Colour normalColour,
                     Colour overColour,
                     Colour downColour);

    /** Sets up an outline to draw around the shape.

        @param outlineColour        the colour to use
        @param outlineStrokeWidth   the thickness of line to draw
    */
    void setOutline (Colour outlineColour, float outlineStrokeWidth);

    /** This lets you specify a border to be left around the edge of the button when
        drawing the shape.
    */
    void setBorderSize (BorderSize<int> border);

    /** @internal */
    void paintButton (Graphics&, bool isMouseOverButton, bool isButtonDown) override;

private:
    //==============================================================================
    Colour normalColour, overColour, downColour, outlineColour;
    DropShadowEffect shadow;
    Path shape;
    BorderSize<int> border;
    bool maintainShapeProportions;
    float outlineWidth;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ShapeButton)
};


#endif   // JUCE_SHAPEBUTTON_H_INCLUDED
