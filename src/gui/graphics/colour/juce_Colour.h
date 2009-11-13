/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

#ifndef __JUCE_COLOUR_JUCEHEADER__
#define __JUCE_COLOUR_JUCEHEADER__

#include "../colour/juce_PixelFormats.h"


//==============================================================================
/**
    Represents a colour, also including a transparency value.

    The colour is stored internally as unsigned 8-bit red, green, blue and alpha values.
*/
class JUCE_API  Colour
{
public:
    //==============================================================================
    /** Creates a transparent black colour. */
    Colour() throw();

    /** Creates a copy of another Colour object. */
    Colour (const Colour& other) throw();

    /** Creates a colour from a 32-bit ARGB value.

        The format of this number is:
            ((alpha << 24) | (red << 16) | (green << 8) | blue).

        All components in the range 0x00 to 0xff.
        An alpha of 0x00 is completely transparent, alpha of 0xff is opaque.

        @see getPixelARGB
    */
    explicit Colour (const uint32 argb) throw();

    /** Creates an opaque colour using 8-bit red, green and blue values */
    Colour (const uint8 red,
            const uint8 green,
            const uint8 blue) throw();

    /** Creates an opaque colour using 8-bit red, green and blue values */
    static const Colour fromRGB (const uint8 red,
                                 const uint8 green,
                                 const uint8 blue) throw();

    /** Creates a colour using 8-bit red, green, blue and alpha values. */
    Colour (const uint8 red,
            const uint8 green,
            const uint8 blue,
            const uint8 alpha) throw();

    /** Creates a colour using 8-bit red, green, blue and alpha values. */
    static const Colour fromRGBA (const uint8 red,
                                  const uint8 green,
                                  const uint8 blue,
                                  const uint8 alpha) throw();

    /** Creates a colour from 8-bit red, green, and blue values, and a floating-point alpha.

        Alpha of 0.0 is transparent, alpha of 1.0f is opaque.
        Values outside the valid range will be clipped.
    */
    Colour (const uint8 red,
            const uint8 green,
            const uint8 blue,
            const float alpha) throw();

    /** Creates a colour using 8-bit red, green, blue and float alpha values. */
    static const Colour fromRGBAFloat (const uint8 red,
                                       const uint8 green,
                                       const uint8 blue,
                                       const float alpha) throw();

    /** Creates a colour using floating point hue, saturation and brightness values, and an 8-bit alpha.

        The floating point values must be between 0.0 and 1.0.
        An alpha of 0x00 is completely transparent, alpha of 0xff is opaque.
        Values outside the valid range will be clipped.
    */
    Colour (const float hue,
            const float saturation,
            const float brightness,
            const uint8 alpha) throw();

    /** Creates a colour using floating point hue, saturation, brightness and alpha values.

        All values must be between 0.0 and 1.0.
        Numbers outside the valid range will be clipped.
    */
    Colour (const float hue,
            const float saturation,
            const float brightness,
            const float alpha) throw();

    /** Creates a colour using floating point hue, saturation and brightness values, and an 8-bit alpha.

        The floating point values must be between 0.0 and 1.0.
        An alpha of 0x00 is completely transparent, alpha of 0xff is opaque.
        Values outside the valid range will be clipped.
    */
    static const Colour fromHSV (const float hue,
                                 const float saturation,
                                 const float brightness,
                                 const float alpha) throw();

    /** Destructor. */
    ~Colour() throw();

    /** Copies another Colour object. */
    const Colour& operator= (const Colour& other) throw();

    /** Compares two colours. */
    bool operator== (const Colour& other) const throw();
    /** Compares two colours. */
    bool operator!= (const Colour& other) const throw();

    //==============================================================================
    /** Returns the red component of this colour.

        @returns a value between 0x00 and 0xff.
    */
    uint8 getRed() const throw()                        { return argb.getRed(); }

    /** Returns the green component of this colour.

        @returns a value between 0x00 and 0xff.
    */
    uint8 getGreen() const throw()                      { return argb.getGreen(); }

    /** Returns the blue component of this colour.

        @returns a value between 0x00 and 0xff.
    */
    uint8 getBlue() const throw()                       { return argb.getBlue(); }

    /** Returns the red component of this colour as a floating point value.

        @returns a value between 0.0 and 1.0
    */
    float getFloatRed() const throw();

    /** Returns the green component of this colour as a floating point value.

        @returns a value between 0.0 and 1.0
    */
    float getFloatGreen() const throw();

    /** Returns the blue component of this colour as a floating point value.

        @returns a value between 0.0 and 1.0
    */
    float getFloatBlue() const throw();

    /** Returns a premultiplied ARGB pixel object that represents this colour.
    */
    const PixelARGB getPixelARGB() const throw();

    /** Returns a 32-bit integer that represents this colour.

        The format of this number is:
            ((alpha << 24) | (red << 16) | (green << 16) | blue).
    */
    uint32 getARGB() const throw();

    //==============================================================================
    /** Returns the colour's alpha (opacity).

        Alpha of 0x00 is completely transparent, 0xff is completely opaque.
    */
    uint8 getAlpha() const throw()                      { return argb.getAlpha(); }

    /** Returns the colour's alpha (opacity) as a floating point value.

        Alpha of 0.0 is completely transparent, 1.0 is completely opaque.
    */
    float getFloatAlpha() const throw();

    /** Returns true if this colour is completely opaque.

        Equivalent to (getAlpha() == 0xff).
    */
    bool isOpaque() const throw();

    /** Returns true if this colour is completely transparent.

        Equivalent to (getAlpha() == 0x00).
    */
    bool isTransparent() const throw();

    /** Returns a colour that's the same colour as this one, but with a new alpha value. */
    const Colour withAlpha (const uint8 newAlpha) const throw();

    /** Returns a colour that's the same colour as this one, but with a new alpha value. */
    const Colour withAlpha (const float newAlpha) const throw();

    /** Returns a colour that's the same colour as this one, but with a modified alpha value.

        The new colour's alpha will be this object's alpha multiplied by the value passed-in.
    */
    const Colour withMultipliedAlpha (const float alphaMultiplier) const throw();

    //==============================================================================
    /** Returns a colour that is the result of alpha-compositing a new colour over this one.

        If the foreground colour is semi-transparent, it is blended onto this colour
        accordingly.
    */
    const Colour overlaidWith (const Colour& foregroundColour) const throw();

    /** Returns a colour that lies somewhere between this one and another.

        If amountOfOther is zero, the result is 100% this colour, if amountOfOther
        is 1.0, the result is 100% of the other colour.
    */
    const Colour interpolatedWith (const Colour& other, float proportionOfOther) const throw();

    //==============================================================================
    /** Returns the colour's hue component.
        The value returned is in the range 0.0 to 1.0
    */
    float getHue() const throw();

    /** Returns the colour's saturation component.
        The value returned is in the range 0.0 to 1.0
    */
    float getSaturation() const throw();

    /** Returns the colour's brightness component.
        The value returned is in the range 0.0 to 1.0
    */
    float getBrightness() const throw();

    /** Returns the colour's hue, saturation and brightness components all at once.
        The values returned are in the range 0.0 to 1.0
    */
    void getHSB (float& hue,
                 float& saturation,
                 float& brightness) const throw();

    //==============================================================================
    /** Returns a copy of this colour with a different hue. */
    const Colour withHue (const float newHue) const throw();

    /** Returns a copy of this colour with a different saturation. */
    const Colour withSaturation (const float newSaturation) const throw();

    /** Returns a copy of this colour with a different brightness.
        @see brighter, darker, withMultipliedBrightness
    */
    const Colour withBrightness (const float newBrightness) const throw();

    /** Returns a copy of this colour with it hue rotated.

        The new colour's hue is ((this->getHue() + amountToRotate) % 1.0)

        @see brighter, darker, withMultipliedBrightness
    */
    const Colour withRotatedHue (const float amountToRotate) const throw();

    /** Returns a copy of this colour with its saturation multiplied by the given value.

        The new colour's saturation is (this->getSaturation() * multiplier)
        (the result is clipped to legal limits).
    */
    const Colour withMultipliedSaturation (const float multiplier) const throw();

    /** Returns a copy of this colour with its brightness multiplied by the given value.

        The new colour's saturation is (this->getBrightness() * multiplier)
        (the result is clipped to legal limits).
    */
    const Colour withMultipliedBrightness (const float amount) const throw();

    //==============================================================================
    /** Returns a brighter version of this colour.

        @param amountBrighter   how much brighter to make it - a value from 0 to 1.0 where 0 is
                                unchanged, and higher values make it brighter
        @see withMultipliedBrightness
    */
    const Colour brighter (float amountBrighter = 0.4f) const throw();

    /** Returns a darker version of this colour.

        @param amountDarker     how much darker to make it - a value from 0 to 1.0 where 0 is
                                unchanged, and higher values make it darker
        @see withMultipliedBrightness
    */
    const Colour darker (float amountDarker = 0.4f) const throw();

    //==============================================================================
    /** Returns a colour that will be clearly visible against this colour.

        The amount parameter indicates how contrasting the new colour should
        be, so e.g. Colours::black.contrasting (0.1f) will return a colour
        that's just a little bit lighter; Colours::black.contrasting (1.0f) will
        return white; Colours::white.contrasting (1.0f) will return black, etc.
    */
    const Colour contrasting (const float amount = 1.0f) const throw();

    /** Returns a colour that contrasts against two colours.

        Looks for a colour that contrasts with both of the colours passed-in.

        Handy for things like choosing a highlight colour in text editors, etc.
    */
    static const Colour contrasting (const Colour& colour1,
                                     const Colour& colour2) throw();

    //==============================================================================
    /** Returns an opaque shade of grey.

        @param brightness the level of grey to return - 0 is black, 1.0 is white
    */
    static const Colour greyLevel (const float brightness) throw();

    //==============================================================================
    /** Returns a stringified version of this colour.

        The string can be turned back into a colour using the fromString() method.
    */
    const String toString() const throw();

    /** Reads the colour from a string that was created with toString().
    */
    static const Colour fromString (const String& encodedColourString);

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    PixelARGB argb;
};

#endif   // __JUCE_COLOUR_JUCEHEADER__
