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
    A button that contains a filled shape.

    @see Button, ImageButton, TextButton, ArrowButton

    @tags{GUI}
*/
class JUCE_API  ShapeButton  : public Button
{
public:
    //==============================================================================
    /** Creates a ShapeButton.

        @param name             a name to give the component - see Component::setName()
        @param normalColor     the color to fill the shape with when the mouse isn't over
        @param overColor       the color to use when the mouse is over the shape
        @param downColor       the color to use when the button is in the pressed-down state
    */
    ShapeButton (const String& name,
                 Color normalColor,
                 Color overColor,
                 Color downColor);

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

    /** Set the colors to use for drawing the shape.

        @param normalColor     the color to fill the shape with when the mouse isn't over
        @param overColor       the color to use when the mouse is over the shape
        @param downColor       the color to use when the button is in the pressed-down state
    */
    void setColors (Color normalColor,
                     Color overColor,
                     Color downColor);

    /** Sets the colors to use for drawing the shape when the button's toggle state is 'on'. To enable this behavior, use the
        shouldUseOnColors() method.

        @param normalColorOn   the color to fill the shape with when the mouse isn't over and the button's toggle state is 'on'
        @param overColorOn     the color to use when the mouse is over the shape and the button's toggle state is 'on'
        @param downColorOn     the color to use when the button is in the pressed-down state and the button's toggle state is 'on'
     */
    void setOnColors (Color normalColorOn,
                       Color overColorOn,
                       Color downColorOn);

    /** Set whether the button should use the 'on' set of colors when its toggle state is 'on'.
        By default these will be the same as the normal colors but the setOnColors method can be
        used to provide a different set of colors.
    */
    void shouldUseOnColors (bool shouldUse);

    /** Sets up an outline to draw around the shape.

        @param outlineColor        the color to use
        @param outlineStrokeWidth   the thickness of line to draw
    */
    void setOutline (Color outlineColor, float outlineStrokeWidth);

    /** This lets you specify a border to be left around the edge of the button when
        drawing the shape.
    */
    void setBorderSize (BorderSize<int> border);

    /** @internal */
    void paintButton (Graphics&, bool isMouseOverButton, bool isButtonDown) override;

private:
    //==============================================================================
    Color normalColor,   overColor,   downColor,
           normalColorOn, overColorOn, downColorOn, outlineColor;
    bool useOnColors;
    DropShadowEffect shadow;
    Path shape;
    BorderSize<int> border;
    bool maintainShapeProportions;
    float outlineWidth;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ShapeButton)
};

} // namespace juce
