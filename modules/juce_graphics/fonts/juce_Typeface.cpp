/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

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

Typeface::~Typeface()
{
}

Typeface::Ptr Typeface::getFallbackTypeface()
{
    const Font fallbackFont (Font::getFallbackFontName(), Font::getFallbackFontStyle(), 10.0f);
    return fallbackFont.getTypeface();
}

EdgeTable* Typeface::getEdgeTableForGlyph (int glyphNumber, const AffineTransform& transform)
{
    Path path;

    if (getOutlineForGlyph (glyphNumber, path) && ! path.isEmpty())
        return new EdgeTable (path.getBoundsTransformed (transform).getSmallestIntegerContainer().expanded (1, 0),
                              path, transform);

    return nullptr;
}

//==============================================================================
struct Typeface::HintingParams
{
    HintingParams (Typeface& t)
        : top (0), middle (0), bottom (0)
    {
        Font font (&t);
        font = font.withHeight ((float) standardHeight);

        top = getAverageY (font, "BDEFPRTZOQC", true);
        middle = getAverageY (font, "acegmnopqrsuvwxy", true);
        bottom = getAverageY (font, "BDELZOC", false);
    }

    AffineTransform getVerticalHintingTransform (float fontSize) noexcept
    {
        if (cachedSize == fontSize)
            return cachedTransform;

        const float t = fontSize * top;
        const float m = fontSize * middle;
        const float b = fontSize * bottom;

        if (b < t + 2.0f)
            return AffineTransform();

        Scaling s[] = { Scaling (t, m, b, 0.0f, 0.0f),
                        Scaling (t, m, b, 1.0f, 0.0f),
                        Scaling (t, m, b, 0.0f, 1.0f),
                        Scaling (t, m, b, 1.0f, 1.0f) };

        int best = 0;

        for (int i = 1; i < numElementsInArray (s); ++i)
            if (s[i].drift < s[best].drift)
                best = i;

        cachedSize = fontSize;

        AffineTransform result (s[best].getTransform());
        cachedTransform = result;
        return result;
    }

private:
    float cachedSize;
    AffineTransform cachedTransform;

    struct Scaling
    {
        Scaling (float t, float m, float b, float direction1, float direction2) noexcept
        {
            float newT = std::floor (t) + direction1;
            float newB = std::floor (b) + direction2;
            float newM = newT + (newB - newT) * (m - t) / (b - t);

            float middleOffset = newM - std::floor (newM);
            float nudge = middleOffset < 0.5f ? (middleOffset * -0.2f) : ((1.0f - middleOffset) * 0.2f);
            newT += nudge;
            newB += nudge;

            scale = (newB - newT) / (b - t);
            offset = (newB / scale) - b;

            drift = getDrift (t) + getDrift (m) + getDrift (b) + offset + 20.0f * std::abs (scale - 1.0f);
        }

        AffineTransform getTransform() const noexcept
        {
            return AffineTransform::translation (0, offset).scaled (1.0f, scale);
        }

        float getDrift (float n) const noexcept
        {
            n = (n + offset) * scale;
            const float diff = n - std::floor (n);
            return jmin (diff, 1.0f - diff);
        }

        float offset, scale, drift;
    };

    static float getAverageY (const Font& font, const char* chars, bool getTop)
    {
        GlyphArrangement ga;
        ga.addLineOfText (font, chars, 0, 0);

        Array<float> y;
        DefaultElementComparator<float> sorter;

        for (int i = 0; i < ga.getNumGlyphs(); ++i)
        {
            Path p;
            ga.getGlyph (i).createPath (p);
            Rectangle<float> bounds (p.getBounds());

            if (! p.isEmpty())
                y.addSorted (sorter, getTop ? bounds.getY() : bounds.getBottom());
        }

        float median = y[y.size() / 2];

        int total = 0;
        int num = 0;

        for (int i = 0; i < y.size(); ++i)
        {
            if (std::abs (median - y.getUnchecked(i)) < 0.05f * (float) standardHeight)
            {
                total += y.getUnchecked(i);
                ++num;
            }
        }

        return num < 4 ? 0.0f : total / (num * (float) standardHeight);
    }

    enum { standardHeight = 100 };
    float top, middle, bottom;
};

AffineTransform Typeface::getVerticalHintingTransform (float fontSize)
{
    ScopedLock sl (hintingLock);

    if (hintingParams == nullptr)
        hintingParams = new HintingParams (*this);

    return hintingParams->getVerticalHintingTransform (fontSize);
}
