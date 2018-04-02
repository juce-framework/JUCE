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

namespace ColorHelpers
{
    static uint8 floatToUInt8 (const float n) noexcept
    {
        return n <= 0.0f ? 0 : (n >= 1.0f ? 255 : static_cast<uint8> (n * 255.996f));
    }

    //==============================================================================
    struct HSB
    {
        HSB (Color col) noexcept
        {
            const int r = col.getRed();
            const int g = col.getGreen();
            const int b = col.getBlue();

            const int hi = jmax (r, g, b);
            const int lo = jmin (r, g, b);

            if (hi != 0)
            {
                saturation = (hi - lo) / (float) hi;

                if (saturation > 0)
                {
                    const float invDiff = 1.0f / (hi - lo);

                    const float red   = (hi - r) * invDiff;
                    const float green = (hi - g) * invDiff;
                    const float blue  = (hi - b) * invDiff;

                    if (r == hi)
                        hue = blue - green;
                    else if (g == hi)
                        hue = 2.0f + red - blue;
                    else
                        hue = 4.0f + green - red;

                    hue *= 1.0f / 6.0f;

                    if (hue < 0)
                        ++hue;
                }
                else
                {
                    hue = 0;
                }
            }
            else
            {
                saturation = hue = 0;
            }

            brightness = hi / 255.0f;
        }

        Color toColor (Color original) const noexcept
        {
            return Color (hue, saturation, brightness, original.getAlpha());
        }

        static PixelARGB toRGB (float h, float s, float v, const uint8 alpha) noexcept
        {
            v = jlimit (0.0f, 255.0f, v * 255.0f);
            const uint8 intV = (uint8) roundToInt (v);

            if (s <= 0)
                return PixelARGB (alpha, intV, intV, intV);

            s = jmin (1.0f, s);
            h = (h - std::floor (h)) * 6.0f + 0.00001f; // need a small adjustment to compensate for rounding errors
            const float f = h - std::floor (h);
            const uint8 x = (uint8) roundToInt (v * (1.0f - s));

            if (h < 1.0f)   return PixelARGB (alpha, intV,    (uint8) roundToInt (v * (1.0f - (s * (1.0f - f)))), x);
            if (h < 2.0f)   return PixelARGB (alpha,          (uint8) roundToInt (v * (1.0f - s * f)), intV, x);
            if (h < 3.0f)   return PixelARGB (alpha, x, intV, (uint8) roundToInt (v * (1.0f - (s * (1.0f - f)))));
            if (h < 4.0f)   return PixelARGB (alpha, x,       (uint8) roundToInt (v * (1.0f - s * f)), intV);
            if (h < 5.0f)   return PixelARGB (alpha,          (uint8) roundToInt (v * (1.0f - (s * (1.0f - f)))), x, intV);
            return                 PixelARGB (alpha, intV, x, (uint8) roundToInt (v * (1.0f - s * f)));
        }

        float hue, saturation, brightness;
    };

    //==============================================================================
    struct YIQ
    {
        YIQ (Color c) noexcept
        {
            const float r = c.getFloatRed();
            const float g = c.getFloatGreen();
            const float b = c.getFloatBlue();

            y = 0.2999f * r + 0.5870f * g + 0.1140f * b;
            i = 0.5957f * r - 0.2744f * g - 0.3212f * b;
            q = 0.2114f * r - 0.5225f * g - 0.3113f * b;
            alpha = c.getFloatAlpha();
        }

        Color toColor() const noexcept
        {
            return Color::fromFloatRGBA (y + 0.9563f * i + 0.6210f * q,
                                          y - 0.2721f * i - 0.6474f * q,
                                          y - 1.1070f * i + 1.7046f * q,
                                          alpha);
        }

        float y, i, q, alpha;
    };
}

//==============================================================================
Color::Color() noexcept
    : argb (0, 0, 0, 0)
{
}

Color::Color (const Color& other) noexcept
    : argb (other.argb)
{
}

Color& Color::operator= (const Color& other) noexcept
{
    argb = other.argb;
    return *this;
}

bool Color::operator== (const Color& other) const noexcept    { return argb.getNativeARGB() == other.argb.getNativeARGB(); }
bool Color::operator!= (const Color& other) const noexcept    { return argb.getNativeARGB() != other.argb.getNativeARGB(); }

//==============================================================================
Color::Color (const uint32 col) noexcept
    : argb ((col >> 24) & 0xff, (col >> 16) & 0xff, (col >> 8) & 0xff, col & 0xff)
{
}

Color::Color (const uint8 red, const uint8 green, const uint8 blue) noexcept
{
    argb.setARGB (0xff, red, green, blue);
}

Color Color::fromRGB (const uint8 red, const uint8 green, const uint8 blue) noexcept
{
    return Color (red, green, blue);
}

Color::Color (const uint8 red, const uint8 green, const uint8 blue, const uint8 alpha) noexcept
{
    argb.setARGB (alpha, red, green, blue);
}

Color Color::fromRGBA (const uint8 red, const uint8 green, const uint8 blue, const uint8 alpha) noexcept
{
    return Color (red, green, blue, alpha);
}

Color::Color (const uint8 red, const uint8 green, const uint8 blue, const float alpha) noexcept
{
    argb.setARGB (ColorHelpers::floatToUInt8 (alpha), red, green, blue);
}

Color Color::fromFloatRGBA (const float red, const float green, const float blue, const float alpha) noexcept
{
    return Color (ColorHelpers::floatToUInt8 (red),
                   ColorHelpers::floatToUInt8 (green),
                   ColorHelpers::floatToUInt8 (blue), alpha);
}

Color::Color (const float hue, const float saturation, const float brightness, const float alpha) noexcept
    : argb (ColorHelpers::HSB::toRGB (hue, saturation, brightness, ColorHelpers::floatToUInt8 (alpha)))
{
}

Color Color::fromHSV (const float hue, const float saturation, const float brightness, const float alpha) noexcept
{
    return Color (hue, saturation, brightness, alpha);
}

Color::Color (const float hue, const float saturation, const float brightness, const uint8 alpha) noexcept
    : argb (ColorHelpers::HSB::toRGB (hue, saturation, brightness, alpha))
{
}

Color::Color (PixelARGB argb_) noexcept
    : argb (argb_)
{
}

Color::Color (PixelRGB rgb) noexcept
    : argb (Color (rgb.getInARGBMaskOrder()).argb)
{
}

Color::Color (PixelAlpha alpha) noexcept
    : argb (Color (alpha.getInARGBMaskOrder()).argb)
{
}

Color::~Color() noexcept
{
}


//==============================================================================
const PixelARGB Color::getPixelARGB() const noexcept
{
    PixelARGB p (argb);
    p.premultiply();
    return p;
}

uint32 Color::getARGB() const noexcept
{
    return argb.getInARGBMaskOrder();
}

//==============================================================================
bool Color::isTransparent() const noexcept
{
    return getAlpha() == 0;
}

bool Color::isOpaque() const noexcept
{
    return getAlpha() == 0xff;
}

Color Color::withAlpha (const uint8 newAlpha) const noexcept
{
    PixelARGB newCol (argb);
    newCol.setAlpha (newAlpha);
    return Color (newCol);
}

Color Color::withAlpha (const float newAlpha) const noexcept
{
    jassert (newAlpha >= 0 && newAlpha <= 1.0f);

    PixelARGB newCol (argb);
    newCol.setAlpha (ColorHelpers::floatToUInt8 (newAlpha));
    return Color (newCol);
}

Color Color::withMultipliedAlpha (const float alphaMultiplier) const noexcept
{
    jassert (alphaMultiplier >= 0);

    PixelARGB newCol (argb);
    newCol.setAlpha ((uint8) jmin (0xff, roundToInt (alphaMultiplier * newCol.getAlpha())));
    return Color (newCol);
}

//==============================================================================
Color Color::overlaidWith (Color src) const noexcept
{
    const int destAlpha = getAlpha();

    if (destAlpha <= 0)
        return src;

    const int invA = 0xff - (int) src.getAlpha();
    const int resA = 0xff - (((0xff - destAlpha) * invA) >> 8);

    if (resA <= 0)
        return *this;

    const int da = (invA * destAlpha) / resA;

    return Color ((uint8) (src.getRed()   + ((((int) getRed()   - src.getRed())   * da) >> 8)),
                   (uint8) (src.getGreen() + ((((int) getGreen() - src.getGreen()) * da) >> 8)),
                   (uint8) (src.getBlue()  + ((((int) getBlue()  - src.getBlue())  * da) >> 8)),
                   (uint8) resA);
}

Color Color::interpolatedWith (Color other, float proportionOfOther) const noexcept
{
    if (proportionOfOther <= 0)
        return *this;

    if (proportionOfOther >= 1.0f)
        return other;

    PixelARGB c1 (getPixelARGB());
    const PixelARGB c2 (other.getPixelARGB());
    c1.tween (c2, (uint32) roundToInt (proportionOfOther * 255.0f));
    c1.unpremultiply();

    return Color (c1);
}

//==============================================================================
float Color::getFloatRed() const noexcept      { return getRed()   / 255.0f; }
float Color::getFloatGreen() const noexcept    { return getGreen() / 255.0f; }
float Color::getFloatBlue() const noexcept     { return getBlue()  / 255.0f; }
float Color::getFloatAlpha() const noexcept    { return getAlpha() / 255.0f; }

//==============================================================================
void Color::getHSB (float& h, float& s, float& v) const noexcept
{
    const ColorHelpers::HSB hsb (*this);
    h = hsb.hue;
    s = hsb.saturation;
    v = hsb.brightness;
}

float Color::getHue() const noexcept           { return ColorHelpers::HSB (*this).hue; }
float Color::getSaturation() const noexcept    { return ColorHelpers::HSB (*this).saturation; }
float Color::getBrightness() const noexcept    { return ColorHelpers::HSB (*this).brightness; }

Color Color::withHue (float h) const noexcept          { ColorHelpers::HSB hsb (*this); hsb.hue = h;        return hsb.toColor (*this); }
Color Color::withSaturation (float s) const noexcept   { ColorHelpers::HSB hsb (*this); hsb.saturation = s; return hsb.toColor (*this); }
Color Color::withBrightness (float v) const noexcept   { ColorHelpers::HSB hsb (*this); hsb.brightness = v; return hsb.toColor (*this); }

float Color::getPerceivedBrightness() const noexcept
{
    return std::sqrt (0.241f * square (getFloatRed())
                    + 0.691f * square (getFloatGreen())
                    + 0.068f * square (getFloatBlue()));
}

//==============================================================================
Color Color::withRotatedHue (const float amountToRotate) const noexcept
{
    ColorHelpers::HSB hsb (*this);
    hsb.hue += amountToRotate;
    return hsb.toColor (*this);
}

Color Color::withMultipliedSaturation (const float amount) const noexcept
{
    ColorHelpers::HSB hsb (*this);
    hsb.saturation = jmin (1.0f, hsb.saturation * amount);
    return hsb.toColor (*this);
}

Color Color::withMultipliedBrightness (const float amount) const noexcept
{
    ColorHelpers::HSB hsb (*this);
    hsb.brightness = jmin (1.0f, hsb.brightness * amount);
    return hsb.toColor (*this);
}

//==============================================================================
Color Color::brighter (float amount) const noexcept
{
    amount = 1.0f / (1.0f + amount);

    return Color ((uint8) (255 - (amount * (255 - getRed()))),
                   (uint8) (255 - (amount * (255 - getGreen()))),
                   (uint8) (255 - (amount * (255 - getBlue()))),
                   getAlpha());
}

Color Color::darker (float amount) const noexcept
{
    amount = 1.0f / (1.0f + amount);

    return Color ((uint8) (amount * getRed()),
                   (uint8) (amount * getGreen()),
                   (uint8) (amount * getBlue()),
                   getAlpha());
}

//==============================================================================
Color Color::grayLevel (const float brightness) noexcept
{
    const uint8 level = ColorHelpers::floatToUInt8 (brightness);
    return Color (level, level, level);
}

//==============================================================================
Color Color::contrasting (const float amount) const noexcept
{
   return overlaidWith ((getPerceivedBrightness() >= 0.5f
                           ? Colors::black
                           : Colors::white).withAlpha (amount));
}

Color Color::contrasting (Color target, float minContrast) const noexcept
{
    const ColorHelpers::YIQ bg (*this);
    ColorHelpers::YIQ fg (target);

    if (std::abs (bg.y - fg.y) >= minContrast)
        return target;

    const float y1 = jmax (0.0f, bg.y - minContrast);
    const float y2 = jmin (1.0f, bg.y + minContrast);
    fg.y = (std::abs (y1 - bg.y) > std::abs (y2 - bg.y)) ? y1 : y2;

    return fg.toColor();
}

Color Color::contrasting (Color color1,
                            Color color2) noexcept
{
    const float b1 = color1.getPerceivedBrightness();
    const float b2 = color2.getPerceivedBrightness();
    float best = 0.0f;
    float bestDist = 0.0f;

    for (float i = 0.0f; i < 1.0f; i += 0.02f)
    {
        const float d1 = std::abs (i - b1);
        const float d2 = std::abs (i - b2);
        const float dist = jmin (d1, d2, 1.0f - d1, 1.0f - d2);

        if (dist > bestDist)
        {
            best = i;
            bestDist = dist;
        }
    }

    return color1.overlaidWith (color2.withMultipliedAlpha (0.5f))
                  .withBrightness (best);
}

//==============================================================================
String Color::toString() const
{
    return String::toHexString ((int) argb.getInARGBMaskOrder());
}

Color Color::fromString (StringRef encodedColorString)
{
    return Color ((uint32) CharacterFunctions::HexParser<int>::parse (encodedColorString.text));
}

String Color::toDisplayString (const bool includeAlphaValue) const
{
    return String::toHexString ((int) (argb.getInARGBMaskOrder() & (includeAlphaValue ? 0xffffffff : 0xffffff)))
                  .paddedLeft ('0', includeAlphaValue ? 8 : 6)
                  .toUpperCase();
}

} // namespace juce
