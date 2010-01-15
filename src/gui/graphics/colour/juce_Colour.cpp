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

#include "../../../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_Colours.h"


//==============================================================================
static uint8 floatAlphaToInt (const float alpha)
{
    return (uint8) jlimit (0, 0xff, roundToInt (alpha * 255.0f));
}

static const float oneOver255 = 1.0f / 255.0f;


//==============================================================================
Colour::Colour() throw()
    : argb (0)
{
}

Colour::Colour (const Colour& other) throw()
    : argb (other.argb)
{
}

const Colour& Colour::operator= (const Colour& other) throw()
{
    argb = other.argb;
    return *this;
}

bool Colour::operator== (const Colour& other) const throw()
{
    return argb.getARGB() == other.argb.getARGB();
}

bool Colour::operator!= (const Colour& other) const throw()
{
    return argb.getARGB() != other.argb.getARGB();
}

//==============================================================================
Colour::Colour (const uint32 argb_) throw()
    : argb (argb_)
{
}

Colour::Colour (const uint8 red,
                const uint8 green,
                const uint8 blue) throw()
{
    argb.setARGB (0xff, red, green, blue);
}

const Colour Colour::fromRGB (const uint8 red,
                              const uint8 green,
                              const uint8 blue) throw()
{
    return Colour (red, green, blue);
}

Colour::Colour (const uint8 red,
                const uint8 green,
                const uint8 blue,
                const uint8 alpha) throw()
{
    argb.setARGB (alpha, red, green, blue);
}

const Colour Colour::fromRGBA (const uint8 red,
                               const uint8 green,
                               const uint8 blue,
                               const uint8 alpha) throw()
{
    return Colour (red, green, blue, alpha);
}

Colour::Colour (const uint8 red,
                const uint8 green,
                const uint8 blue,
                const float alpha) throw()
{
    argb.setARGB (floatAlphaToInt (alpha), red, green, blue);
}

const Colour Colour::fromRGBAFloat (const uint8 red,
                                    const uint8 green,
                                    const uint8 blue,
                                    const float alpha) throw()
{
    return Colour (red, green, blue, alpha);
}

//==============================================================================
static void convertHSBtoRGB (float h, float s, float v,
                             uint8& r, uint8& g, uint8& b) throw()
{
    v = jlimit (0.0f, 1.0f, v);
    v *= 255.0f;
    const uint8 intV = (uint8) roundToInt (v);

    if (s <= 0)
    {
        r = intV;
        g = intV;
        b = intV;
    }
    else
    {
        s = jmin (1.0f, s);
        h = jlimit (0.0f, 1.0f, h);
        h = (h - floorf (h)) * 6.0f + 0.00001f; // need a small adjustment to compensate for rounding errors
        const float f = h - floorf (h);

        const uint8 x = (uint8) roundToInt (v * (1.0f - s));
        const float y = v * (1.0f - s * f);
        const float z = v * (1.0f - (s * (1.0f - f)));

        if (h < 1.0f)
        {
            r = intV;
            g = (uint8) roundToInt (z);
            b = x;
        }
        else if (h < 2.0f)
        {
            r = (uint8) roundToInt (y);
            g = intV;
            b = x;
        }
        else if (h < 3.0f)
        {
            r = x;
            g = intV;
            b = (uint8) roundToInt (z);
        }
        else if (h < 4.0f)
        {
            r = x;
            g = (uint8) roundToInt (y);
            b = intV;
        }
        else if (h < 5.0f)
        {
            r = (uint8) roundToInt (z);
            g = x;
            b = intV;
        }
        else if (h < 6.0f)
        {
            r = intV;
            g = x;
            b = (uint8) roundToInt (y);
        }
        else
        {
            r = 0;
            g = 0;
            b = 0;
        }
    }
}

Colour::Colour (const float hue,
                const float saturation,
                const float brightness,
                const float alpha) throw()
{
    uint8 r = getRed(), g = getGreen(), b = getBlue();
    convertHSBtoRGB (hue, saturation, brightness, r, g, b);

    argb.setARGB (floatAlphaToInt (alpha), r, g, b);
}

const Colour Colour::fromHSV (const float hue,
                              const float saturation,
                              const float brightness,
                              const float alpha) throw()
{
    return Colour (hue, saturation, brightness, alpha);
}

Colour::Colour (const float hue,
                const float saturation,
                const float brightness,
                const uint8 alpha) throw()
{
    uint8 r = getRed(), g = getGreen(), b = getBlue();
    convertHSBtoRGB (hue, saturation, brightness, r, g, b);

    argb.setARGB (alpha, r, g, b);
}

Colour::~Colour() throw()
{
}

//==============================================================================
const PixelARGB Colour::getPixelARGB() const throw()
{
    PixelARGB p (argb);
    p.premultiply();
    return p;
}

uint32 Colour::getARGB() const throw()
{
    return argb.getARGB();
}

//==============================================================================
bool Colour::isTransparent() const throw()
{
    return getAlpha() == 0;
}

bool Colour::isOpaque() const throw()
{
    return getAlpha() == 0xff;
}

const Colour Colour::withAlpha (const uint8 newAlpha) const throw()
{
    PixelARGB newCol (argb);
    newCol.setAlpha (newAlpha);
    return Colour (newCol.getARGB());
}

const Colour Colour::withAlpha (const float newAlpha) const throw()
{
    jassert (newAlpha >= 0 && newAlpha <= 1.0f);

    PixelARGB newCol (argb);
    newCol.setAlpha (floatAlphaToInt (newAlpha));
    return Colour (newCol.getARGB());
}

const Colour Colour::withMultipliedAlpha (const float alphaMultiplier) const throw()
{
    jassert (alphaMultiplier >= 0);

    PixelARGB newCol (argb);
    newCol.setAlpha ((uint8) jmin (0xff, roundToInt (alphaMultiplier * newCol.getAlpha())));
    return Colour (newCol.getARGB());
}

//==============================================================================
const Colour Colour::overlaidWith (const Colour& src) const throw()
{
    const int destAlpha = getAlpha();

    if (destAlpha > 0)
    {
        const int invA = 0xff - (int) src.getAlpha();
        const int resA = 0xff - (((0xff - destAlpha) * invA) >> 8);

        if (resA > 0)
        {
            const int da = (invA * destAlpha) / resA;

            return Colour ((uint8) (src.getRed()   + ((((int) getRed() - src.getRed())     * da) >> 8)),
                           (uint8) (src.getGreen() + ((((int) getGreen() - src.getGreen()) * da) >> 8)),
                           (uint8) (src.getBlue()  + ((((int) getBlue() - src.getBlue())   * da) >> 8)),
                           (uint8) resA);
        }

        return *this;
    }
    else
    {
        return src;
    }
}

const Colour Colour::interpolatedWith (const Colour& other, float proportionOfOther) const throw()
{
    if (proportionOfOther <= 0)
        return *this;

    if (proportionOfOther >= 1.0f)
        return other;

    PixelARGB c1 (getPixelARGB());
    const PixelARGB c2 (other.getPixelARGB());
    c1.tween (c2, roundToInt (proportionOfOther * 255.0f));
    c1.unpremultiply();

    return Colour (c1.getARGB());
}

//==============================================================================
float Colour::getFloatRed() const throw()
{
    return getRed() * oneOver255;
}

float Colour::getFloatGreen() const throw()
{
    return getGreen() * oneOver255;
}

float Colour::getFloatBlue() const throw()
{
    return getBlue() * oneOver255;
}

float Colour::getFloatAlpha() const throw()
{
    return getAlpha() * oneOver255;
}

//==============================================================================
void Colour::getHSB (float& h, float& s, float& v) const throw()
{
    const int r = getRed();
    const int g = getGreen();
    const int b = getBlue();

    const int hi = jmax (r, g, b);
    const int lo = jmin (r, g, b);

    if (hi != 0)
    {
        s = (hi - lo) / (float) hi;

        if (s != 0)
        {
            const float invDiff = 1.0f / (hi - lo);

            const float red   = (hi - r) * invDiff;
            const float green = (hi - g) * invDiff;
            const float blue  = (hi - b) * invDiff;

            if (r == hi)
                h = blue - green;
            else if (g == hi)
                h = 2.0f + red - blue;
            else
                h = 4.0f + green - red;

            h *= 1.0f / 6.0f;

            if (h < 0)
                ++h;
        }
        else
        {
            h = 0;
        }
    }
    else
    {
        s = 0;
        h = 0;
    }

    v = hi * oneOver255;
}

//==============================================================================
float Colour::getHue() const throw()
{
    float h, s, b;
    getHSB (h, s, b);
    return h;
}

const Colour Colour::withHue (const float hue) const throw()
{
    float h, s, b;
    getHSB (h, s, b);

    return Colour (hue, s, b, getAlpha());
}

const Colour Colour::withRotatedHue (const float amountToRotate) const throw()
{
    float h, s, b;
    getHSB (h, s, b);

    h += amountToRotate;
    h -= floorf (h);

    return Colour (h, s, b, getAlpha());
}

//==============================================================================
float Colour::getSaturation() const throw()
{
    float h, s, b;
    getHSB (h, s, b);
    return s;
}

const Colour Colour::withSaturation (const float saturation) const throw()
{
    float h, s, b;
    getHSB (h, s, b);

    return Colour (h, saturation, b, getAlpha());
}

const Colour Colour::withMultipliedSaturation (const float amount) const throw()
{
    float h, s, b;
    getHSB (h, s, b);

    return Colour (h, jmin (1.0f, s * amount), b, getAlpha());
}

//==============================================================================
float Colour::getBrightness() const throw()
{
    float h, s, b;
    getHSB (h, s, b);
    return b;
}

const Colour Colour::withBrightness (const float brightness) const throw()
{
    float h, s, b;
    getHSB (h, s, b);

    return Colour (h, s, brightness, getAlpha());
}

const Colour Colour::withMultipliedBrightness (const float amount) const throw()
{
    float h, s, b;
    getHSB (h, s, b);

    b *= amount;

    if (b > 1.0f)
        b = 1.0f;

    return Colour (h, s, b, getAlpha());
}

//==============================================================================
const Colour Colour::brighter (float amount) const throw()
{
    amount = 1.0f / (1.0f + amount);

    return Colour ((uint8) (255 - (amount * (255 - getRed()))),
                   (uint8) (255 - (amount * (255 - getGreen()))),
                   (uint8) (255 - (amount * (255 - getBlue()))),
                   getAlpha());
}

const Colour Colour::darker (float amount) const throw()
{
    amount = 1.0f / (1.0f + amount);

    return Colour ((uint8) (amount * getRed()),
                   (uint8) (amount * getGreen()),
                   (uint8) (amount * getBlue()),
                   getAlpha());
}

//==============================================================================
const Colour Colour::greyLevel (const float brightness) throw()
{
    const uint8 level
        = (uint8) jlimit (0x00, 0xff, roundToInt (brightness * 255.0f));

    return Colour (level, level, level);
}

//==============================================================================
const Colour Colour::contrasting (const float amount) const throw()
{
    return overlaidWith ((((int) getRed() + (int) getGreen() + (int) getBlue() >= 3 * 128)
                            ? Colours::black
                            : Colours::white).withAlpha (amount));
}

const Colour Colour::contrasting (const Colour& colour1,
                                  const Colour& colour2) throw()
{
    const float b1 = colour1.getBrightness();
    const float b2 = colour2.getBrightness();
    float best = 0.0f;
    float bestDist = 0.0f;

    for (float i = 0.0f; i < 1.0f; i += 0.02f)
    {
        const float d1 = fabsf (i - b1);
        const float d2 = fabsf (i - b2);
        const float dist = jmin (d1, d2, 1.0f - d1, 1.0f - d2);

        if (dist > bestDist)
        {
            best = i;
            bestDist = dist;
        }
    }

    return colour1.overlaidWith (colour2.withMultipliedAlpha (0.5f))
                  .withBrightness (best);
}

//==============================================================================
const String Colour::toString() const throw()
{
    return String::toHexString ((int) argb.getARGB());
}

const Colour Colour::fromString (const String& encodedColourString)
{
    return Colour ((uint32) encodedColourString.getHexValue32());
}


END_JUCE_NAMESPACE
