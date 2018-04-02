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
    Represents a color, also including a transparency value.

    The color is stored internally as unsigned 8-bit red, green, blue and alpha values.

    @tags{Graphics}
*/
class JUCE_API  Color  final
{
public:
    //==============================================================================
    /** Creates a transparent black color. */
    Color() noexcept;

    /** Creates a copy of another Color object. */
    Color (const Color& other) noexcept;

    /** Creates a color from a 32-bit ARGB value.

        The format of this number is:
            ((alpha << 24) | (red << 16) | (green << 8) | blue).

        All components in the range 0x00 to 0xff.
        An alpha of 0x00 is completely transparent, alpha of 0xff is opaque.

        @see getPixelARGB
    */
    explicit Color (uint32 argb) noexcept;

    /** Creates an opaque color using 8-bit red, green and blue values */
    Color (uint8 red,
            uint8 green,
            uint8 blue) noexcept;

    /** Creates an opaque color using 8-bit red, green and blue values */
    static Color fromRGB (uint8 red,
                           uint8 green,
                           uint8 blue) noexcept;

    /** Creates a color using 8-bit red, green, blue and alpha values. */
    Color (uint8 red,
            uint8 green,
            uint8 blue,
            uint8 alpha) noexcept;

    /** Creates a color using 8-bit red, green, blue and alpha values. */
    static Color fromRGBA (uint8 red,
                            uint8 green,
                            uint8 blue,
                            uint8 alpha) noexcept;

    /** Creates a color from 8-bit red, green, and blue values, and a floating-point alpha.

        Alpha of 0.0 is transparent, alpha of 1.0f is opaque.
        Values outside the valid range will be clipped.
    */
    Color (uint8 red,
            uint8 green,
            uint8 blue,
            float alpha) noexcept;

    /** Creates a color using floating point red, green, blue and alpha values.
        Numbers outside the range 0..1 will be clipped.
    */
    static Color fromFloatRGBA (float red,
                                 float green,
                                 float blue,
                                 float alpha) noexcept;

    /** Creates a color using floating point hue, saturation and brightness values, and an 8-bit alpha.

        The floating point values must be between 0.0 and 1.0.
        An alpha of 0x00 is completely transparent, alpha of 0xff is opaque.
        Values outside the valid range will be clipped.
    */
    Color (float hue,
            float saturation,
            float brightness,
            uint8 alpha) noexcept;

    /** Creates a color using floating point hue, saturation, brightness and alpha values.

        All values must be between 0.0 and 1.0.
        Numbers outside the valid range will be clipped.
    */
    Color (float hue,
            float saturation,
            float brightness,
            float alpha) noexcept;

    /** Creates a color using a PixelARGB object. This function assumes that the argb pixel is
        not premultiplied.
     */
    Color (PixelARGB argb) noexcept;

    /** Creates a color using a PixelRGB object.
     */
    Color (PixelRGB rgb) noexcept;

    /** Creates a color using a PixelAlpha object.
     */
    Color (PixelAlpha alpha) noexcept;

    /** Creates a color using floating point hue, saturation and brightness values, and an 8-bit alpha.

        The floating point values must be between 0.0 and 1.0.
        An alpha of 0x00 is completely transparent, alpha of 0xff is opaque.
        Values outside the valid range will be clipped.
    */
    static Color fromHSV (float hue,
                           float saturation,
                           float brightness,
                           float alpha) noexcept;

    /** Destructor. */
    ~Color() noexcept;

    /** Copies another Color object. */
    Color& operator= (const Color& other) noexcept;

    /** Compares two colors. */
    bool operator== (const Color& other) const noexcept;
    /** Compares two colors. */
    bool operator!= (const Color& other) const noexcept;

    //==============================================================================
    /** Returns the red component of this color.
        @returns a value between 0x00 and 0xff.
    */
    uint8 getRed() const noexcept                       { return argb.getRed(); }

    /** Returns the green component of this color.
        @returns a value between 0x00 and 0xff.
    */
    uint8 getGreen() const noexcept                     { return argb.getGreen(); }

    /** Returns the blue component of this color.
        @returns a value between 0x00 and 0xff.
    */
    uint8 getBlue() const noexcept                      { return argb.getBlue(); }

    /** Returns the red component of this color as a floating point value.
        @returns a value between 0.0 and 1.0
    */
    float getFloatRed() const noexcept;

    /** Returns the green component of this color as a floating point value.
        @returns a value between 0.0 and 1.0
    */
    float getFloatGreen() const noexcept;

    /** Returns the blue component of this color as a floating point value.
        @returns a value between 0.0 and 1.0
    */
    float getFloatBlue() const noexcept;

    /** Returns a premultiplied ARGB pixel object that represents this color.
    */
    const PixelARGB getPixelARGB() const noexcept;

    /** Returns a 32-bit integer that represents this color.

        The format of this number is:
            ((alpha << 24) | (red << 16) | (green << 16) | blue).
    */
    uint32 getARGB() const noexcept;

    //==============================================================================
    /** Returns the color's alpha (opacity).

        Alpha of 0x00 is completely transparent, 0xff is completely opaque.
    */
    uint8 getAlpha() const noexcept                     { return argb.getAlpha(); }

    /** Returns the color's alpha (opacity) as a floating point value.

        Alpha of 0.0 is completely transparent, 1.0 is completely opaque.
    */
    float getFloatAlpha() const noexcept;

    /** Returns true if this color is completely opaque.

        Equivalent to (getAlpha() == 0xff).
    */
    bool isOpaque() const noexcept;

    /** Returns true if this color is completely transparent.

        Equivalent to (getAlpha() == 0x00).
    */
    bool isTransparent() const noexcept;

    /** Returns a color that's the same color as this one, but with a new alpha value. */
    Color withAlpha (uint8 newAlpha) const noexcept;

    /** Returns a color that's the same color as this one, but with a new alpha value. */
    Color withAlpha (float newAlpha) const noexcept;

    /** Returns a color that's the same color as this one, but with a modified alpha value.
        The new color's alpha will be this object's alpha multiplied by the value passed-in.
    */
    Color withMultipliedAlpha (float alphaMultiplier) const noexcept;

    //==============================================================================
    /** Returns a color that is the result of alpha-compositing a new color over this one.
        If the foreground color is semi-transparent, it is blended onto this color accordingly.
    */
    Color overlaidWith (Color foregroundColor) const noexcept;

    /** Returns a color that lies somewhere between this one and another.
        If amountOfOther is zero, the result is 100% this color, if amountOfOther
        is 1.0, the result is 100% of the other color.
    */
    Color interpolatedWith (Color other, float proportionOfOther) const noexcept;

    //==============================================================================
    /** Returns the color's hue component.
        The value returned is in the range 0.0 to 1.0
    */
    float getHue() const noexcept;

    /** Returns the color's saturation component.
        The value returned is in the range 0.0 to 1.0
    */
    float getSaturation() const noexcept;

    /** Returns the color's brightness component.
        The value returned is in the range 0.0 to 1.0
    */
    float getBrightness() const noexcept;

    /** Returns a skewed brightness value, adjusted to better reflect the way the human
        eye responds to different color channels. This makes it better than getBrightness()
        for comparing differences in brightness.
    */
    float getPerceivedBrightness() const noexcept;

    /** Returns the color's hue, saturation and brightness components all at once.
        The values returned are in the range 0.0 to 1.0
    */
    void getHSB (float& hue,
                 float& saturation,
                 float& brightness) const noexcept;

    //==============================================================================
    /** Returns a copy of this color with a different hue. */
    Color withHue (float newHue) const noexcept;

    /** Returns a copy of this color with a different saturation. */
    Color withSaturation (float newSaturation) const noexcept;

    /** Returns a copy of this color with a different brightness.
        @see brighter, darker, withMultipliedBrightness
    */
    Color withBrightness (float newBrightness) const noexcept;

    /** Returns a copy of this color with it hue rotated.
        The new color's hue is ((this->getHue() + amountToRotate) % 1.0)
        @see brighter, darker, withMultipliedBrightness
    */
    Color withRotatedHue (float amountToRotate) const noexcept;

    /** Returns a copy of this color with its saturation multiplied by the given value.
        The new color's saturation is (this->getSaturation() * multiplier)
        (the result is clipped to legal limits).
    */
    Color withMultipliedSaturation (float multiplier) const noexcept;

    /** Returns a copy of this color with its brightness multiplied by the given value.
        The new color's brightness is (this->getBrightness() * multiplier)
        (the result is clipped to legal limits).
    */
    Color withMultipliedBrightness (float amount) const noexcept;

    //==============================================================================
    /** Returns a brighter version of this color.
        @param amountBrighter   how much brighter to make it - a value from 0 to 1.0 where 0 is
                                unchanged, and higher values make it brighter
        @see withMultipliedBrightness
    */
    Color brighter (float amountBrighter = 0.4f) const noexcept;

    /** Returns a darker version of this color.
        @param amountDarker     how much darker to make it - a value from 0 to 1.0 where 0 is
                                unchanged, and higher values make it darker
        @see withMultipliedBrightness
    */
    Color darker (float amountDarker = 0.4f) const noexcept;

    //==============================================================================
    /** Returns a color that will be clearly visible against this color.

        The amount parameter indicates how contrasting the new color should
        be, so e.g. Colors::black.contrasting (0.1f) will return a color
        that's just a little bit lighter; Colors::black.contrasting (1.0f) will
        return white; Colors::white.contrasting (1.0f) will return black, etc.
    */
    Color contrasting (float amount = 1.0f) const noexcept;

    /** Returns a color that is as close as possible to a target color whilst
        still being in contrast to this one.

        The color that is returned will be the targetColor, but with its luminosity
        nudged up or down so that it differs from the luminosity of this color
        by at least the amount specified by minLuminosityDiff.
    */
    Color contrasting (Color targetColor, float minLuminosityDiff) const noexcept;

    /** Returns a color that contrasts against two colors.
        Looks for a color that contrasts with both of the colors passed-in.
        Handy for things like choosing a highlight color in text editors, etc.
    */
    static Color contrasting (Color color1,
                               Color color2) noexcept;

    //==============================================================================
    /** Returns an opaque shade of gray.
        @param brightness the level of gray to return - 0 is black, 1.0 is white
    */
    static Color grayLevel (float brightness) noexcept;

    //==============================================================================
    /** Returns a stringified version of this color.
        The string can be turned back into a color using the fromString() method.
    */
    String toString() const;

    /** Reads the color from a string that was created with toString(). */
    static Color fromString (StringRef encodedColorString);

    /** Returns the color as a hex string in the form RRGGBB or AARRGGBB. */
    String toDisplayString (bool includeAlphaValue) const;

private:
    //==============================================================================
    PixelARGB argb;
};

} // namespace juce
