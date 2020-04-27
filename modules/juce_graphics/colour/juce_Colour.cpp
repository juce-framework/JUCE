/*
  ==============================================================================

   This file is part of the JUCE 6 technical preview.
   Copyright (c) 2020 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For this technical preview, this file is not subject to commercial licensing.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

namespace ColourHelpers
{
    static uint8 floatToUInt8 (float n) noexcept
    {
        return n <= 0.0f ? 0 : (n >= 1.0f ? 255 : (uint8) roundToInt (n * 255.0f));
    }

    static float getHue (Colour col)
    {
        auto r = (int) col.getRed();
        auto g = (int) col.getGreen();
        auto b = (int) col.getBlue();

        auto hi = jmax (r, g, b);
        auto lo = jmin (r, g, b);

        float hue = 0.0f;

        if (hi > 0)
        {
            auto invDiff = 1.0f / (hi - lo);

            auto red   = (hi - r) * invDiff;
            auto green = (hi - g) * invDiff;
            auto blue  = (hi - b) * invDiff;

            if      (r == hi)  hue = blue - green;
            else if (g == hi)  hue = 2.0f + red - blue;
            else               hue = 4.0f + green - red;

            hue *= 1.0f / 6.0f;

            if (hue < 0.0f)
                hue += 1.0f;
        }

        return hue;
    }

    //==============================================================================
    struct HSL
    {
        HSL (Colour col) noexcept
        {
            auto r = (int) col.getRed();
            auto g = (int) col.getGreen();
            auto b = (int) col.getBlue();

            auto hi = jmax (r, g, b);
            auto lo = jmin (r, g, b);

            if (hi > 0)
            {
                lightness = ((hi + lo) / 2.0f) / 255.0f;

                if (lightness > 0.0f)
                    hue = getHue (col);

                saturation = (hi - lo) / (1.0f - std::abs ((2.0f * lightness) - 1.0f));
            }
        }

        Colour toColour (Colour original) const noexcept
        {
            return Colour::fromHSL (hue, saturation, lightness, original.getAlpha());
        }

        static PixelARGB toRGB (float h, float s, float l, uint8 alpha) noexcept
        {
            auto v = l < 0.5f ? l * (1.0f + s) : l + s - (l * s);

            if (approximatelyEqual (v, 0.0f))
                return PixelARGB (alpha, 0, 0, 0);

            auto min = (2.0f * l) - v;
            auto sv = (v - min) / v;

            h = jlimit (0.0f, 360.0f, h * 360.0f) / 60.0f;
            auto f = h - std::floor (h);
            auto vsf = v * sv * f;
            auto mid1 = min + vsf;
            auto mid2 = v - vsf;

            if      (h < 1.0f)  return PixelARGB (alpha, floatToUInt8 (v),    floatToUInt8 (mid1), floatToUInt8 (min));
            else if (h < 2.0f)  return PixelARGB (alpha, floatToUInt8 (mid2), floatToUInt8 (v),    floatToUInt8 (min));
            else if (h < 3.0f)  return PixelARGB (alpha, floatToUInt8 (min),  floatToUInt8 (v),    floatToUInt8 (mid1));
            else if (h < 4.0f)  return PixelARGB (alpha, floatToUInt8 (min),  floatToUInt8 (mid2), floatToUInt8 (v));
            else if (h < 5.0f)  return PixelARGB (alpha, floatToUInt8 (mid1), floatToUInt8 (min),  floatToUInt8 (v));
            else if (h < 6.0f)  return PixelARGB (alpha, floatToUInt8 (v),    floatToUInt8 (min),  floatToUInt8 (mid2));

            return PixelARGB (alpha, 0, 0, 0);
        }

        float hue = 0.0f, saturation = 0.0f, lightness = 0.0f;
    };

    //==============================================================================
    struct HSB
    {
        HSB (Colour col) noexcept
        {
            auto r = (int) col.getRed();
            auto g = (int) col.getGreen();
            auto b = (int) col.getBlue();

            auto hi = jmax (r, g, b);
            auto lo = jmin (r, g, b);

            if (hi > 0)
            {
                saturation = (hi - lo) / (float) hi;

                if (saturation > 0.0f)
                    hue = getHue (col);

                brightness = hi / 255.0f;
            }
        }

        Colour toColour (Colour original) const noexcept
        {
            return Colour (hue, saturation, brightness, original.getAlpha());
        }

        static PixelARGB toRGB (float h, float s, float v, uint8 alpha) noexcept
        {
            v = jlimit (0.0f, 255.0f, v * 255.0f);
            auto intV = (uint8) roundToInt (v);

            if (s <= 0)
                return PixelARGB (alpha, intV, intV, intV);

            s = jmin (1.0f, s);
            h = jlimit (0.0f, 360.0f, h * 360.0f) / 60.0f;
            auto f = h - std::floor (h);
            auto x = (uint8) roundToInt (v * (1.0f - s));

            if (h < 1.0f)   return PixelARGB (alpha, intV,    (uint8) roundToInt (v * (1.0f - (s * (1.0f - f)))), x);
            if (h < 2.0f)   return PixelARGB (alpha,          (uint8) roundToInt (v * (1.0f - s * f)), intV, x);
            if (h < 3.0f)   return PixelARGB (alpha, x, intV, (uint8) roundToInt (v * (1.0f - (s * (1.0f - f)))));
            if (h < 4.0f)   return PixelARGB (alpha, x,       (uint8) roundToInt (v * (1.0f - s * f)), intV);
            if (h < 5.0f)   return PixelARGB (alpha,          (uint8) roundToInt (v * (1.0f - (s * (1.0f - f)))), x, intV);
            return                 PixelARGB (alpha, intV, x, (uint8) roundToInt (v * (1.0f - s * f)));
        }

        float hue = 0.0f, saturation = 0.0f, brightness = 0.0f;
    };

    //==============================================================================
    struct YIQ
    {
        YIQ (Colour c) noexcept
        {
            auto r = c.getFloatRed();
            auto g = c.getFloatGreen();
            auto b = c.getFloatBlue();

            y = 0.2999f * r + 0.5870f * g + 0.1140f * b;
            i = 0.5957f * r - 0.2744f * g - 0.3212f * b;
            q = 0.2114f * r - 0.5225f * g - 0.3113f * b;
            alpha = c.getFloatAlpha();
        }

        Colour toColour() const noexcept
        {
            return Colour::fromFloatRGBA (y + 0.9563f * i + 0.6210f * q,
                                          y - 0.2721f * i - 0.6474f * q,
                                          y - 1.1070f * i + 1.7046f * q,
                                          alpha);
        }

        float y = 0.0f, i = 0.0f, q = 0.0f, alpha = 0.0f;
    };
}

//==============================================================================
bool Colour::operator== (const Colour& other) const noexcept    { return argb.getNativeARGB() == other.argb.getNativeARGB(); }
bool Colour::operator!= (const Colour& other) const noexcept    { return argb.getNativeARGB() != other.argb.getNativeARGB(); }

//==============================================================================
Colour::Colour (uint32 col) noexcept
    : argb (static_cast<uint8> ((col >> 24) & 0xff),
            static_cast<uint8> ((col >> 16) & 0xff),
            static_cast<uint8> ((col >> 8) & 0xff),
            static_cast<uint8> (col & 0xff))
{
}

Colour::Colour (uint8 red, uint8 green, uint8 blue) noexcept
{
    argb.setARGB (0xff, red, green, blue);
}

Colour Colour::fromRGB (uint8 red, uint8 green, uint8 blue) noexcept
{
    return Colour (red, green, blue);
}

Colour::Colour (uint8 red, uint8 green, uint8 blue, uint8 alpha) noexcept
{
    argb.setARGB (alpha, red, green, blue);
}

Colour Colour::fromRGBA (uint8 red, uint8 green, uint8 blue, uint8 alpha) noexcept
{
    return Colour (red, green, blue, alpha);
}

Colour::Colour (uint8 red, uint8 green, uint8 blue, float alpha) noexcept
{
    argb.setARGB (ColourHelpers::floatToUInt8 (alpha), red, green, blue);
}

Colour Colour::fromFloatRGBA (float red, float green, float blue, float alpha) noexcept
{
    return Colour (ColourHelpers::floatToUInt8 (red),
                   ColourHelpers::floatToUInt8 (green),
                   ColourHelpers::floatToUInt8 (blue), alpha);
}

Colour::Colour (float hue, float saturation, float brightness, float alpha) noexcept
    : argb (ColourHelpers::HSB::toRGB (hue, saturation, brightness, ColourHelpers::floatToUInt8 (alpha)))
{
}

Colour Colour::fromHSV (float hue, float saturation, float brightness, float alpha) noexcept
{
    return Colour (hue, saturation, brightness, alpha);
}

Colour Colour::fromHSL (float hue, float saturation, float lightness, float alpha) noexcept
{
    Colour hslColour;
    hslColour.argb = ColourHelpers::HSL::toRGB (hue, saturation, lightness, ColourHelpers::floatToUInt8 (alpha));

    return hslColour;
}

Colour::Colour (float hue, float saturation, float brightness, uint8 alpha) noexcept
    : argb (ColourHelpers::HSB::toRGB (hue, saturation, brightness, alpha))
{
}

Colour::Colour (PixelARGB argb_) noexcept
    : argb (argb_)
{
}

Colour::Colour (PixelRGB rgb) noexcept
    : argb (Colour (rgb.getInARGBMaskOrder()).argb)
{
}

Colour::Colour (PixelAlpha alpha) noexcept
    : argb (Colour (alpha.getInARGBMaskOrder()).argb)
{
}

//==============================================================================
const PixelARGB Colour::getPixelARGB() const noexcept
{
    PixelARGB p (argb);
    p.premultiply();
    return p;
}

uint32 Colour::getARGB() const noexcept
{
    return argb.getInARGBMaskOrder();
}

//==============================================================================
bool Colour::isTransparent() const noexcept
{
    return getAlpha() == 0;
}

bool Colour::isOpaque() const noexcept
{
    return getAlpha() == 0xff;
}

Colour Colour::withAlpha (const uint8 newAlpha) const noexcept
{
    PixelARGB newCol (argb);
    newCol.setAlpha (newAlpha);
    return Colour (newCol);
}

Colour Colour::withAlpha (const float newAlpha) const noexcept
{
    jassert (newAlpha >= 0 && newAlpha <= 1.0f);

    PixelARGB newCol (argb);
    newCol.setAlpha (ColourHelpers::floatToUInt8 (newAlpha));
    return Colour (newCol);
}

Colour Colour::withMultipliedAlpha (const float alphaMultiplier) const noexcept
{
    jassert (alphaMultiplier >= 0);

    PixelARGB newCol (argb);
    newCol.setAlpha ((uint8) jmin (0xff, roundToInt (alphaMultiplier * newCol.getAlpha())));
    return Colour (newCol);
}

//==============================================================================
Colour Colour::overlaidWith (Colour src) const noexcept
{
    auto destAlpha = getAlpha();

    if (destAlpha <= 0)
        return src;

    auto invA = 0xff - (int) src.getAlpha();
    auto resA = 0xff - (((0xff - destAlpha) * invA) >> 8);

    if (resA <= 0)
        return *this;

    auto da = (invA * destAlpha) / resA;

    return Colour ((uint8) (src.getRed()   + ((((int) getRed()   - src.getRed())   * da) >> 8)),
                   (uint8) (src.getGreen() + ((((int) getGreen() - src.getGreen()) * da) >> 8)),
                   (uint8) (src.getBlue()  + ((((int) getBlue()  - src.getBlue())  * da) >> 8)),
                   (uint8) resA);
}

Colour Colour::interpolatedWith (Colour other, float proportionOfOther) const noexcept
{
    if (proportionOfOther <= 0)
        return *this;

    if (proportionOfOther >= 1.0f)
        return other;

    PixelARGB c1 (getPixelARGB());
    PixelARGB c2 (other.getPixelARGB());
    c1.tween (c2, (uint32) roundToInt (proportionOfOther * 255.0f));
    c1.unpremultiply();

    return Colour (c1);
}

//==============================================================================
float Colour::getFloatRed() const noexcept      { return getRed()   / 255.0f; }
float Colour::getFloatGreen() const noexcept    { return getGreen() / 255.0f; }
float Colour::getFloatBlue() const noexcept     { return getBlue()  / 255.0f; }
float Colour::getFloatAlpha() const noexcept    { return getAlpha() / 255.0f; }

//==============================================================================
void Colour::getHSB (float& h, float& s, float& v) const noexcept
{
    ColourHelpers::HSB hsb (*this);
    h = hsb.hue;
    s = hsb.saturation;
    v = hsb.brightness;
}

void Colour::getHSL (float& h, float& s, float& l) const noexcept
{
    ColourHelpers::HSL hsl (*this);
    h = hsl.hue;
    s = hsl.saturation;
    l = hsl.lightness;
}

float Colour::getHue() const noexcept           { return ColourHelpers::HSB (*this).hue; }
float Colour::getSaturation() const noexcept    { return ColourHelpers::HSB (*this).saturation; }
float Colour::getBrightness() const noexcept    { return ColourHelpers::HSB (*this).brightness; }

float Colour::getLightness() const noexcept     { return ColourHelpers::HSL (*this).lightness; }

Colour Colour::withHue (float h) const noexcept          { ColourHelpers::HSB hsb (*this); hsb.hue = h;        return hsb.toColour (*this); }
Colour Colour::withSaturation (float s) const noexcept   { ColourHelpers::HSB hsb (*this); hsb.saturation = s; return hsb.toColour (*this); }
Colour Colour::withBrightness (float v) const noexcept   { ColourHelpers::HSB hsb (*this); hsb.brightness = v; return hsb.toColour (*this); }

Colour Colour::withLightness (float l) const noexcept    { ColourHelpers::HSL hsl (*this); hsl.lightness = l; return hsl.toColour (*this); }

float Colour::getPerceivedBrightness() const noexcept
{
    return std::sqrt (0.241f * square (getFloatRed())
                    + 0.691f * square (getFloatGreen())
                    + 0.068f * square (getFloatBlue()));
}

//==============================================================================
Colour Colour::withRotatedHue (const float amountToRotate) const noexcept
{
    ColourHelpers::HSB hsb (*this);
    hsb.hue += amountToRotate;
    return hsb.toColour (*this);
}

Colour Colour::withMultipliedSaturation (const float amount) const noexcept
{
    ColourHelpers::HSB hsb (*this);
    hsb.saturation = jmin (1.0f, hsb.saturation * amount);
    return hsb.toColour (*this);
}

Colour Colour::withMultipliedBrightness (const float amount) const noexcept
{
    ColourHelpers::HSB hsb (*this);
    hsb.brightness = jmin (1.0f, hsb.brightness * amount);
    return hsb.toColour (*this);
}

Colour Colour::withMultipliedLightness (const float amount) const noexcept
{
    ColourHelpers::HSL hsl (*this);
    hsl.lightness = jmin (1.0f, hsl.lightness * amount);
    return hsl.toColour (*this);
}

//==============================================================================
Colour Colour::brighter (float amount) const noexcept
{
    amount = 1.0f / (1.0f + amount);

    return Colour ((uint8) (255 - (amount * (255 - getRed()))),
                   (uint8) (255 - (amount * (255 - getGreen()))),
                   (uint8) (255 - (amount * (255 - getBlue()))),
                   getAlpha());
}

Colour Colour::darker (float amount) const noexcept
{
    amount = 1.0f / (1.0f + amount);

    return Colour ((uint8) (amount * getRed()),
                   (uint8) (amount * getGreen()),
                   (uint8) (amount * getBlue()),
                   getAlpha());
}

//==============================================================================
Colour Colour::greyLevel (const float brightness) noexcept
{
    auto level = ColourHelpers::floatToUInt8 (brightness);
    return Colour (level, level, level);
}

//==============================================================================
Colour Colour::contrasting (const float amount) const noexcept
{
   return overlaidWith ((getPerceivedBrightness() >= 0.5f
                           ? Colours::black
                           : Colours::white).withAlpha (amount));
}

Colour Colour::contrasting (Colour target, float minContrast) const noexcept
{
    ColourHelpers::YIQ bg (*this);
    ColourHelpers::YIQ fg (target);

    if (std::abs (bg.y - fg.y) >= minContrast)
        return target;

    auto y1 = jmax (0.0f, bg.y - minContrast);
    auto y2 = jmin (1.0f, bg.y + minContrast);
    fg.y = (std::abs (y1 - bg.y) > std::abs (y2 - bg.y)) ? y1 : y2;

    return fg.toColour();
}

Colour Colour::contrasting (Colour colour1,
                            Colour colour2) noexcept
{
    auto b1 = colour1.getPerceivedBrightness();
    auto b2 = colour2.getPerceivedBrightness();
    float best = 0.0f, bestDist = 0.0f;

    for (float i = 0.0f; i < 1.0f; i += 0.02f)
    {
        auto d1 = std::abs (i - b1);
        auto d2 = std::abs (i - b2);
        auto dist = jmin (d1, d2, 1.0f - d1, 1.0f - d2);

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
String Colour::toString() const
{
    return String::toHexString ((int) argb.getInARGBMaskOrder());
}

Colour Colour::fromString (StringRef encodedColourString)
{
    return Colour ((uint32) CharacterFunctions::HexParser<int>::parse (encodedColourString.text));
}

String Colour::toDisplayString (const bool includeAlphaValue) const
{
    return String::toHexString ((int) (argb.getInARGBMaskOrder() & (includeAlphaValue ? 0xffffffff : 0xffffff)))
                  .paddedLeft ('0', includeAlphaValue ? 8 : 6)
                  .toUpperCase();
}


//==============================================================================
//==============================================================================
#if JUCE_UNIT_TESTS

class ColourTests  : public UnitTest
{
public:
    ColourTests()
        : UnitTest ("Colour", UnitTestCategories::graphics)
    {}

    void runTest() override
    {
        beginTest ("Constructors");
        {
            Colour c1;
            expectEquals (c1.getRed(),        (uint8) 0);
            expectEquals (c1.getGreen(),      (uint8) 0);
            expectEquals (c1.getBlue(),       (uint8) 0);
            expectEquals (c1.getAlpha(),      (uint8) 0);
            expectEquals (c1.getFloatAlpha(), 0.0f);

            Colour c2 ((uint32) 0);
            expectEquals (c2.getRed(),        (uint8) 0);
            expectEquals (c2.getGreen(),      (uint8) 0);
            expectEquals (c2.getBlue(),       (uint8) 0);
            expectEquals (c2.getAlpha(),      (uint8) 0);
            expectEquals (c2.getFloatAlpha(), 0.0f);

            Colour c3 ((uint32) 0xffffffff);
            expectEquals (c3.getRed(),        (uint8) 255);
            expectEquals (c3.getGreen(),      (uint8) 255);
            expectEquals (c3.getBlue(),       (uint8) 255);
            expectEquals (c3.getAlpha(),      (uint8) 255);
            expectEquals (c3.getFloatAlpha(), 1.0f);

            Colour c4 (0, 0, 0);
            expectEquals (c4.getRed(),        (uint8) 0);
            expectEquals (c4.getGreen(),      (uint8) 0);
            expectEquals (c4.getBlue(),       (uint8) 0);
            expectEquals (c4.getAlpha(),      (uint8) 255);
            expectEquals (c4.getFloatAlpha(), 1.0f);

            Colour c5 (255, 255, 255);
            expectEquals (c5.getRed(),        (uint8) 255);
            expectEquals (c5.getGreen(),      (uint8) 255);
            expectEquals (c5.getBlue(),       (uint8) 255);
            expectEquals (c5.getAlpha(),      (uint8) 255);
            expectEquals (c5.getFloatAlpha(), 1.0f);

            Colour c6 ((uint8) 0, (uint8) 0, (uint8) 0, (uint8) 0);
            expectEquals (c6.getRed(),        (uint8) 0);
            expectEquals (c6.getGreen(),      (uint8) 0);
            expectEquals (c6.getBlue(),       (uint8) 0);
            expectEquals (c6.getAlpha(),      (uint8) 0);
            expectEquals (c6.getFloatAlpha(), 0.0f);

            Colour c7 ((uint8) 255, (uint8) 255, (uint8) 255, (uint8) 255);
            expectEquals (c7.getRed(),        (uint8) 255);
            expectEquals (c7.getGreen(),      (uint8) 255);
            expectEquals (c7.getBlue(),       (uint8) 255);
            expectEquals (c7.getAlpha(),      (uint8) 255);
            expectEquals (c7.getFloatAlpha(), 1.0f);

            Colour c8 ((uint8) 0, (uint8) 0, (uint8) 0, 0.0f);
            expectEquals (c8.getRed(),        (uint8) 0);
            expectEquals (c8.getGreen(),      (uint8) 0);
            expectEquals (c8.getBlue(),       (uint8) 0);
            expectEquals (c8.getAlpha(),      (uint8) 0);
            expectEquals (c8.getFloatAlpha(), 0.0f);

            Colour c9 ((uint8) 255, (uint8) 255, (uint8) 255, 1.0f);
            expectEquals (c9.getRed(),        (uint8) 255);
            expectEquals (c9.getGreen(),      (uint8) 255);
            expectEquals (c9.getBlue(),       (uint8) 255);
            expectEquals (c9.getAlpha(),      (uint8) 255);
            expectEquals (c9.getFloatAlpha(), 1.0f);
        }

        beginTest ("HSV");
        {
            auto testHSV = [this] (int hueDegrees, int saturationPercentage, int brightnessPercentage,
                                   uint8 expectedRed, uint8 expectedGreen, uint8 expectedBlue)
            {
                auto testColour = Colour::fromHSV (hueDegrees / 360.0f,
                                                   saturationPercentage / 100.0f,
                                                   brightnessPercentage / 100.0f,
                                                   1.0f);

                expectEquals (testColour.getRed(),   expectedRed);
                expectEquals (testColour.getGreen(), expectedGreen);
                expectEquals (testColour.getBlue(),  expectedBlue);
            };

            // black
            testHSV (0, 0, 0, 0, 0, 0);
            // white
            testHSV (0, 0, 100, 255, 255, 255);
            // red
            testHSV (0, 100, 100, 255, 0, 0);
            // lime
            testHSV (120, 100, 100, 0, 255, 0);
            // blue
            testHSV (240, 100, 100, 0, 0, 255);
            // yellow
            testHSV (60, 100, 100, 255, 255, 0);
            // cyan
            testHSV (180, 100, 100, 0, 255, 255);
            // magenta
            testHSV (300, 100, 100, 255, 0, 255);
            // silver
            testHSV (0, 0, 75, 191, 191, 191);
            // grey
            testHSV (0, 0, 50, 128, 128, 128);
            // maroon
            testHSV (0, 100, 50, 128, 0, 0);
            // olive
            testHSV (60, 100, 50, 128, 128, 0);
            // green
            testHSV (120, 100, 50, 0, 128, 0);
            // purple
            testHSV (300, 100, 50, 128, 0, 128);
            // teal
            testHSV (180, 100, 50, 0, 128, 128);
            // navy
            testHSV (240, 100, 50, 0, 0, 128);
        }

        beginTest ("HSL");
        {
            auto testHSL = [this] (int hueDegrees, int saturationPercentage, int lightnessPercentage,
                                   uint8 expectedRed, uint8 expectedGreen, uint8 expectedBlue)
            {
                auto testColour = Colour::fromHSL (hueDegrees / 360.0f,
                                                   saturationPercentage / 100.0f,
                                                   lightnessPercentage / 100.0f,
                                                   1.0f);

                expectEquals (testColour.getRed(),   expectedRed);
                expectEquals (testColour.getGreen(), expectedGreen);
                expectEquals (testColour.getBlue(),  expectedBlue);
            };

            // black
            testHSL (0, 0, 0, 0, 0, 0);
            // white
            testHSL (0, 0, 100, 255, 255, 255);
            // red
            testHSL (0, 100, 50, 255, 0, 0);
            // lime
            testHSL (120, 100, 50, 0, 255, 0);
            // blue
            testHSL (240, 100, 50, 0, 0, 255);
            // yellow
            testHSL (60, 100, 50, 255, 255, 0);
            // cyan
            testHSL (180, 100, 50, 0, 255, 255);
            // magenta
            testHSL (300, 100, 50, 255, 0, 255);
            // silver
            testHSL (0, 0, 75, 191, 191, 191);
            // grey
            testHSL (0, 0, 50, 128, 128, 128);
            // maroon
            testHSL (0, 100, 25, 128, 0, 0);
            // olive
            testHSL (60, 100, 25, 128, 128, 0);
            // green
            testHSL (120, 100, 25, 0, 128, 0);
            // purple
            testHSL (300, 100, 25, 128, 0, 128);
            // teal
            testHSL (180, 100, 25, 0, 128, 128);
            // navy
            testHSL (240, 100, 25, 0, 0, 128);
        }
    }
};

static ColourTests colourTests;

#endif

} // namespace juce
