/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

struct FontStyleHelpers
{
    static const char* getStyleName (const bool bold,
                                     const bool italic) noexcept
    {
        if (bold && italic) return "Bold Italic";
        if (bold)           return "Bold";
        if (italic)         return "Italic";
        return "Regular";
    }

    static const char* getStyleName (const int styleFlags) noexcept
    {
        return getStyleName ((styleFlags & Font::bold) != 0,
                             (styleFlags & Font::italic) != 0);
    }

    static bool isBold (const String& style) noexcept
    {
        return style.containsWholeWordIgnoreCase ("Bold");
    }

    static bool isItalic (const String& style) noexcept
    {
        return style.containsWholeWordIgnoreCase ("Italic")
            || style.containsWholeWordIgnoreCase ("Oblique");
    }

    static bool isPlaceholderFamilyName (const String& family)
    {
        return family == Font::getDefaultSansSerifFontName()
            || family == Font::getDefaultSerifFontName()
            || family == Font::getDefaultMonospacedFontName();
    }

    struct ConcreteFamilyNames
    {
        ConcreteFamilyNames()
            : sans  (findName (Font::getDefaultSansSerifFontName())),
              serif (findName (Font::getDefaultSerifFontName())),
              mono  (findName (Font::getDefaultMonospacedFontName()))
        {
        }

        String lookUp (const String& placeholder)
        {
            if (placeholder == Font::getDefaultSansSerifFontName())  return sans;
            if (placeholder == Font::getDefaultSerifFontName())      return serif;
            if (placeholder == Font::getDefaultMonospacedFontName()) return mono;

            return findName (placeholder);
        }

    private:
        static String findName (const String& placeholder)
        {
            const Font f (placeholder, Font::getDefaultStyle(), 15.0f);
            return Font::getDefaultTypefaceForFont (f)->getName();
        }

        String sans, serif, mono;
    };

    static String getConcreteFamilyNameFromPlaceholder (const String& placeholder)
    {
        static ConcreteFamilyNames names;
        return names.lookUp (placeholder);
    }

    static String getConcreteFamilyName (const Font& font)
    {
        const String& family = font.getTypefaceName();

        return isPlaceholderFamilyName (family) ? getConcreteFamilyNameFromPlaceholder (family)
                                                : family;
    }
};

//==============================================================================
Typeface::Typeface (const String& faceName, const String& styleName) noexcept
    : name (faceName), style (styleName)
{
}

Typeface::~Typeface() = default;

Typeface::Ptr Typeface::getFallbackTypeface()
{
    const Font fallbackFont (Font::getFallbackFontName(), Font::getFallbackFontStyle(), 10.0f);
    return fallbackFont.getTypefacePtr();
}

EdgeTable* Typeface::getEdgeTableForGlyph (int glyphNumber, const AffineTransform& transform, float fontHeight)
{
    Path path;

    if (getOutlineForGlyph (glyphNumber, path) && ! path.isEmpty())
    {
        applyVerticalHintingTransform (fontHeight, path);

        return new EdgeTable (path.getBoundsTransformed (transform).getSmallestIntegerContainer().expanded (1, 0),
                              path, transform);
    }

    return nullptr;
}

//==============================================================================
struct Typeface::HintingParams
{
    HintingParams (Typeface& t)
    {
        Font font (t);
        font = font.withHeight ((float) standardHeight);

        top = getAverageY (font, "BDEFPRTZOQ", true);
        middle = getAverageY (font, "acegmnopqrsuvwxy", true);
        bottom = getAverageY (font, "BDELZOC", false);
    }

    void applyVerticalHintingTransform (float fontSize, Path& path)
    {
        if (! approximatelyEqual (cachedSize, fontSize))
        {
            cachedSize = fontSize;
            cachedScale = Scaling (top, middle, bottom, fontSize);
        }

        if (bottom < top + 3.0f / fontSize)
            return;

        Path result;

        for (Path::Iterator i (path); i.next();)
        {
            switch (i.elementType)
            {
                case Path::Iterator::startNewSubPath:  result.startNewSubPath (i.x1, cachedScale.apply (i.y1)); break;
                case Path::Iterator::lineTo:           result.lineTo (i.x1, cachedScale.apply (i.y1)); break;
                case Path::Iterator::quadraticTo:      result.quadraticTo (i.x1, cachedScale.apply (i.y1),
                                                                           i.x2, cachedScale.apply (i.y2)); break;
                case Path::Iterator::cubicTo:          result.cubicTo (i.x1, cachedScale.apply (i.y1),
                                                                       i.x2, cachedScale.apply (i.y2),
                                                                       i.x3, cachedScale.apply (i.y3)); break;
                case Path::Iterator::closePath:        result.closeSubPath(); break;
                default:                               jassertfalse; break;
            }
        }

        result.swapWithPath (path);
    }

private:
    struct Scaling
    {
        Scaling() noexcept : middle(), upperScale(), upperOffset(), lowerScale(), lowerOffset() {}

        Scaling (float t, float m, float b, float fontSize) noexcept  : middle (m)
        {
            const float newT = std::floor (fontSize * t + 0.5f) / fontSize;
            const float newB = std::floor (fontSize * b + 0.5f) / fontSize;
            const float newM = std::floor (fontSize * m + 0.3f) / fontSize; // this is slightly biased so that lower-case letters
                                                                            // are more likely to become taller than shorter.
            upperScale  = jlimit (0.9f, 1.1f, (newM - newT) / (m - t));
            lowerScale  = jlimit (0.9f, 1.1f, (newB - newM) / (b - m));

            upperOffset = newM - m * upperScale;
            lowerOffset = newB - b * lowerScale;
        }

        float apply (float y) const noexcept
        {
            return y < middle ? (y * upperScale + upperOffset)
                              : (y * lowerScale + lowerOffset);
        }

        float middle, upperScale, upperOffset, lowerScale, lowerOffset;
    };

    float cachedSize = 0;
    Scaling cachedScale;

    static float getAverageY (const Font& font, const char* chars, bool getTop)
    {
        GlyphArrangement ga;
        ga.addLineOfText (font, chars, 0, 0);

        Array<float> yValues;

        for (auto& glyph : ga)
        {
            Path p;
            glyph.createPath (p);
            auto bounds = p.getBounds();

            if (! p.isEmpty())
                yValues.add (getTop ? bounds.getY() : bounds.getBottom());
        }

        std::sort (yValues.begin(), yValues.end());

        auto median = yValues[yValues.size() / 2];
        float total = 0;
        int num = 0;

        for (auto y : yValues)
        {
            if (std::abs (median - y) < 0.05f * (float) standardHeight)
            {
                total += y;
                ++num;
            }
        }

        return num < 4 ? 0.0f : total / ((float) num * (float) standardHeight);
    }

    enum { standardHeight = 100 };
    float top = 0, middle = 0, bottom = 0;
};

void Typeface::applyVerticalHintingTransform (float fontSize, Path& path)
{
    if (fontSize > 3.0f && fontSize < 25.0f)
    {
        ScopedLock sl (hintingLock);

        if (hintingParams == nullptr)
            hintingParams.reset (new HintingParams (*this));

        return hintingParams->applyVerticalHintingTransform (fontSize, path);
    }
}

} // namespace juce
