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
    Describes the layout and colors that should be used to paint a color gradient.

    @see Graphics::setGradientFill

    @tags{Graphics}
*/
class JUCE_API  ColorGradient  final
{
public:
    /** Creates an uninitialized gradient.

        If you use this constructor instead of the other one, be sure to set all the
        object's public member variables before using it!
    */
    ColorGradient() noexcept;

    ColorGradient (const ColorGradient&);
    ColorGradient (ColorGradient&&) noexcept;
    ColorGradient& operator= (const ColorGradient&);
    ColorGradient& operator= (ColorGradient&&) noexcept;

    //==============================================================================
    /** Creates a gradient object.

        (x1, y1) is the location to draw with color1. Likewise (x2, y2) is where
        color2 should be. In between them there's a gradient.

        If isRadial is true, the colors form a circular gradient with (x1, y1) at
        its center.

        The alpha transparencies of the colors are used, so note that
        if you blend from transparent to a solid color, the RGB of the transparent
        color will become visible in parts of the gradient. e.g. blending
        from Color::transparentBlack to Colors::white will produce a
        muddy gray color midway, but Color::transparentWhite to Colors::white
        will be white all the way across.

        @see ColorGradient
    */
    ColorGradient (Color color1, float x1, float y1,
                    Color color2, float x2, float y2,
                    bool isRadial);

    /** Creates a gradient object.

        point1 is the location to draw with color1. Likewise point2 is where
        color2 should be. In between them there's a gradient.

        If isRadial is true, the colors form a circular gradient with point1 at
        its center.

        The alpha transparencies of the colors are used, so note that
        if you blend from transparent to a solid color, the RGB of the transparent
        color will become visible in parts of the gradient. e.g. blending
        from Color::transparentBlack to Colors::white will produce a
        muddy gray color midway, but Color::transparentWhite to Colors::white
        will be white all the way across.

        @see ColorGradient
    */
    ColorGradient (Color color1, Point<float> point1,
                    Color color2, Point<float> point2,
                    bool isRadial);

    //==============================================================================
    /** Creates a vertical linear gradient between two Y coordinates */
    static ColorGradient vertical (Color color1, float y1,
                                    Color color2, float y2);

    /** Creates a horizontal linear gradient between two X coordinates */
    static ColorGradient horizontal (Color color1, float x1,
                                      Color color2, float x2);

    /** Creates a vertical linear gradient from top to bottom in a rectangle */
    template <typename Type>
    static ColorGradient vertical (Color colorTop, Color colorBottom, Rectangle<Type> area)
    {
        return vertical (colorTop, (float) area.getY(), colorBottom, (float) area.getBottom());
    }

    /** Creates a horizontal linear gradient from right to left in a rectangle */
    template <typename Type>
    static ColorGradient horizontal (Color colorLeft, Color colorRight, Rectangle<Type> area)
    {
        return horizontal (colorLeft, (float) area.getX(), colorRight, (float) area.getRight());
    }

    /** Destructor */
    ~ColorGradient();

    //==============================================================================
    /** Removes any colors that have been added.

        This will also remove any start and end colors, so the gradient won't work. You'll
        need to add more colors with addColor().
    */
    void clearColors();

    /** Adds a color at a point along the length of the gradient.

        This allows the gradient to go through a spectrum of colors, instead of just a
        start and end color.

        @param proportionAlongGradient      a value between 0 and 1.0, which is the proportion
                                            of the distance along the line between the two points
                                            at which the color should occur.
        @param color                       the color that should be used at this point
        @returns the index at which the new point was added
    */
    int addColor (double proportionAlongGradient, Color color);

    /** Removes one of the colors from the gradient. */
    void removeColor (int index);

    /** Multiplies the alpha value of all the colors by the given scale factor */
    void multiplyOpacity (float multiplier) noexcept;

    //==============================================================================
    /** Returns the number of color-stops that have been added. */
    int getNumColors() const noexcept;

    /** Returns the position along the length of the gradient of the color with this index.

        The index is from 0 to getNumColors() - 1. The return value will be between 0.0 and 1.0
    */
    double getColorPosition (int index) const noexcept;

    /** Returns the color that was added with a given index.
        The index is from 0 to getNumColors() - 1.
    */
    Color getColor (int index) const noexcept;

    /** Changes the color at a given index.
        The index is from 0 to getNumColors() - 1.
    */
    void setColor (int index, Color newColor) noexcept;

    /** Returns the an interpolated color at any position along the gradient.
        @param position     the position along the gradient, between 0 and 1
    */
    Color getColorAtPosition (double position) const noexcept;

    //==============================================================================
    /** Creates a set of interpolated premultiplied ARGB values.
        This will resize the HeapBlock, fill it with the colors, and will return the number of
        colors that it added.
        When calling this, the ColorGradient must have at least 2 color stops specified.
    */
    int createLookupTable (const AffineTransform& transform, HeapBlock<PixelARGB>& resultLookupTable) const;

    /** Creates a set of interpolated premultiplied ARGB values.
        This will fill an array of a user-specified size with the gradient, interpolating to fit.
        The numEntries argument specifies the size of the array, and this size must be greater than zero.
        When calling this, the ColorGradient must have at least 2 color stops specified.
    */
    void createLookupTable (PixelARGB* resultLookupTable, int numEntries) const noexcept;

    /** Returns true if all colors are opaque. */
    bool isOpaque() const noexcept;

    /** Returns true if all colors are completely transparent. */
    bool isInvisible() const noexcept;

    //==============================================================================
    Point<float> point1, point2;

    /** If true, the gradient should be filled circularly, centered around
        point1, with point2 defining a point on the circumference.

        If false, the gradient is linear between the two points.
    */
    bool isRadial;

    bool operator== (const ColorGradient&) const noexcept;
    bool operator!= (const ColorGradient&) const noexcept;


private:
    //==============================================================================
    struct ColorPoint
    {
        bool operator== (ColorPoint) const noexcept;
        bool operator!= (ColorPoint) const noexcept;

        double position;
        Color color;
    };

    Array<ColorPoint> colors;

    JUCE_LEAK_DETECTOR (ColorGradient)
};

} // namespace juce
