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

#include "juce_Font.h"
#include "juce_GlyphArrangement.h"
#include "../../components/lookandfeel/juce_LookAndFeel.h"
#include "../../graphics/contexts/juce_LowLevelGraphicsContext.h"
#include "../../../utilities/juce_DeletedAtShutdown.h"
#include "../../../core/juce_Singleton.h"


//==============================================================================
static const float minFontHeight = 0.1f;
static const float maxFontHeight = 10000.0f;
static const float defaultFontHeight = 14.0f;

static const tchar* const juce_defaultFontNameSans = T("<Sans-Serif>");
static const tchar* const juce_defaultFontNameSerif = T("<Serif>");
static const tchar* const juce_defaultFontNameMono = T("<Monospaced>");

void clearUpDefaultFontNames() throw(); // in juce_LookAndFeel.cpp

//==============================================================================
Font::SharedFontInternal::SharedFontInternal (const String& typefaceName_, const float height_, const float horizontalScale_,
                                              const float kerning_, const float ascent_, const int styleFlags_,
                                              Typeface* const typeface_) throw()
    : typefaceName (typefaceName_),
      height (height_),
      horizontalScale (horizontalScale_),
      kerning (kerning_),
      ascent (ascent_),
      styleFlags (styleFlags_),
      typeface (typeface_)
{
}

Font::SharedFontInternal::SharedFontInternal (const SharedFontInternal& other) throw()
    : typefaceName (other.typefaceName),
      height (other.height),
      horizontalScale (other.horizontalScale),
      kerning (other.kerning),
      ascent (other.ascent),
      styleFlags (other.styleFlags),
      typeface (other.typeface)
{
}


//==============================================================================
Font::Font() throw()
    : font (new SharedFontInternal (juce_defaultFontNameSans, defaultFontHeight,
                                    1.0f, 0, 0, Font::plain, 0))
{
}

Font::Font (const float fontHeight, const int styleFlags_) throw()
    : font (new SharedFontInternal (juce_defaultFontNameSans, jlimit (minFontHeight, maxFontHeight, fontHeight),
                                    1.0f, 0, 0, styleFlags_, 0))
{
}

Font::Font (const String& typefaceName_,
            const float fontHeight,
            const int styleFlags_) throw()
    : font (new SharedFontInternal (typefaceName_, jlimit (minFontHeight, maxFontHeight, fontHeight),
                                    1.0f, 0, 0, styleFlags_, 0))
{
}

Font::Font (const Font& other) throw()
    : font (other.font)
{
}

const Font& Font::operator= (const Font& other) throw()
{
    font = other.font;
    return *this;
}

Font::~Font() throw()
{
}

Font::Font (const Typeface::Ptr& typeface) throw()
    : font (new SharedFontInternal (typeface->getName(), defaultFontHeight,
                                    1.0f, 0, 0, Font::plain, typeface))
{
}

bool Font::operator== (const Font& other) const throw()
{
    return font == other.font
            || (font->height == other.font->height
                && font->styleFlags == other.font->styleFlags
                && font->horizontalScale == other.font->horizontalScale
                && font->kerning == other.font->kerning
                && font->typefaceName == other.font->typefaceName);
}

bool Font::operator!= (const Font& other) const throw()
{
    return ! operator== (other);
}

void Font::dupeInternalIfShared() throw()
{
    if (font->getReferenceCount() > 1)
        font = new SharedFontInternal (*font);
}

//==============================================================================
const String Font::getDefaultSansSerifFontName() throw()
{
    return juce_defaultFontNameSans;
}

const String Font::getDefaultSerifFontName() throw()
{
    return juce_defaultFontNameSerif;
}

const String Font::getDefaultMonospacedFontName() throw()
{
    return juce_defaultFontNameMono;
}

void Font::setTypefaceName (const String& faceName) throw()
{
    if (faceName != font->typefaceName)
    {
        dupeInternalIfShared();
        font->typefaceName = faceName;
        font->typeface = 0;
        font->ascent = 0;
    }
}

//==============================================================================
static String fallbackFont;

const String Font::getFallbackFontName() throw()
{
    return fallbackFont;
}

void Font::setFallbackFontName (const String& name) throw()
{
    fallbackFont = name;
}

//==============================================================================
void Font::setHeight (float newHeight) throw()
{
    newHeight = jlimit (minFontHeight, maxFontHeight, newHeight);

    if (font->height != newHeight)
    {
        dupeInternalIfShared();
        font->height = newHeight;
    }
}

void Font::setHeightWithoutChangingWidth (float newHeight) throw()
{
    newHeight = jlimit (minFontHeight, maxFontHeight, newHeight);

    if (font->height != newHeight)
    {
        dupeInternalIfShared();
        font->horizontalScale *= (font->height / newHeight);
        font->height = newHeight;
    }
}

void Font::setStyleFlags (const int newFlags) throw()
{
    if (font->styleFlags != newFlags)
    {
        dupeInternalIfShared();
        font->styleFlags = newFlags;
        font->typeface = 0;
        font->ascent = 0;
    }
}

void Font::setSizeAndStyle (float newHeight,
                            const int newStyleFlags,
                            const float newHorizontalScale,
                            const float newKerningAmount) throw()
{
    newHeight = jlimit (minFontHeight, maxFontHeight, newHeight);

    if (font->height != newHeight
         || font->horizontalScale != newHorizontalScale
         || font->kerning != newKerningAmount)
    {
        dupeInternalIfShared();
        font->height = newHeight;
        font->horizontalScale = newHorizontalScale;
        font->kerning = newKerningAmount;
    }

    setStyleFlags (newStyleFlags);
}

void Font::setHorizontalScale (const float scaleFactor) throw()
{
    dupeInternalIfShared();
    font->horizontalScale = scaleFactor;
}

void Font::setExtraKerningFactor (const float extraKerning) throw()
{
    dupeInternalIfShared();
    font->kerning = extraKerning;
}

void Font::setBold (const bool shouldBeBold) throw()
{
    setStyleFlags (shouldBeBold ? (font->styleFlags | bold)
                                : (font->styleFlags & ~bold));
}

bool Font::isBold() const throw()
{
    return (font->styleFlags & bold) != 0;
}

void Font::setItalic (const bool shouldBeItalic) throw()
{
    setStyleFlags (shouldBeItalic ? (font->styleFlags | italic)
                                  : (font->styleFlags & ~italic));
}

bool Font::isItalic() const throw()
{
    return (font->styleFlags & italic) != 0;
}

void Font::setUnderline (const bool shouldBeUnderlined) throw()
{
    setStyleFlags (shouldBeUnderlined ? (font->styleFlags | underlined)
                                      : (font->styleFlags & ~underlined));
}

bool Font::isUnderlined() const throw()
{
    return (font->styleFlags & underlined) != 0;
}

float Font::getAscent() const throw()
{
    if (font->ascent == 0)
        font->ascent = getTypeface()->getAscent();

    return font->height * font->ascent;
}

float Font::getDescent() const throw()
{
    return font->height - getAscent();
}

int Font::getStringWidth (const String& text) const throw()
{
    return roundFloatToInt (getStringWidthFloat (text));
}

float Font::getStringWidthFloat (const String& text) const throw()
{
    float w = getTypeface()->getStringWidth (text);

    if (font->kerning != 0)
        w += font->kerning * text.length();

    return w * font->height * font->horizontalScale;
}

void Font::getGlyphPositions (const String& text, Array <int>& glyphs, Array <float>& xOffsets) const throw()
{
    getTypeface()->getGlyphPositions (text, glyphs, xOffsets);

    const float scale = font->height * font->horizontalScale;
    const int num = xOffsets.size();
    float* const x = &(xOffsets.getReference(0));

    if (font->kerning != 0)
    {
        for (int i = 0; i < num; ++i)
            x[i] = (x[i] + i * font->kerning) * scale;
    }
    else
    {
        for (int i = 0; i < num; ++i)
            x[i] *= scale;
    }
}

void Font::findFonts (OwnedArray<Font>& destArray) throw()
{
    const StringArray names (findAllTypefaceNames());

    for (int i = 0; i < names.size(); ++i)
        destArray.add (new Font (names[i], defaultFontHeight, Font::plain));
}


//==============================================================================
class TypefaceCache  : public DeletedAtShutdown
{
public:
    TypefaceCache (int numToCache = 10) throw()
        : counter (1),
          faces (2)
    {
        while (--numToCache >= 0)
            faces.add (new CachedFace());
    }

    ~TypefaceCache()
    {
        clearUpDefaultFontNames();
        clearSingletonInstance();
    }

    juce_DeclareSingleton_SingleThreaded_Minimal (TypefaceCache)

    const Typeface::Ptr findTypefaceFor (const Font& font) throw()
    {
        const int flags = font.getStyleFlags() & (Font::bold | Font::italic);
        const String faceName (font.getTypefaceName());

        int i;
        for (i = faces.size(); --i >= 0;)
        {
            CachedFace* const face = faces.getUnchecked(i);

            if (face->flags == flags
                 && face->typefaceName == faceName)
            {
                face->lastUsageCount = ++counter;
                return face->typeFace;
            }
        }

        int replaceIndex = 0;
        int bestLastUsageCount = INT_MAX;

        for (i = faces.size(); --i >= 0;)
        {
            const int lu = faces.getUnchecked(i)->lastUsageCount;

            if (bestLastUsageCount > lu)
            {
                bestLastUsageCount = lu;
                replaceIndex = i;
            }
        }

        CachedFace* const face = faces.getUnchecked (replaceIndex);
        face->typefaceName = faceName;
        face->flags = flags;
        face->lastUsageCount = ++counter;
        face->typeFace = LookAndFeel::getDefaultLookAndFeel().getTypefaceForFont (font);
        jassert (face->typeFace != 0); // the look and feel must return a typeface!

        return face->typeFace;
    }

    juce_UseDebuggingNewOperator

private:
    struct CachedFace
    {
        CachedFace() throw()
            : lastUsageCount (0), flags (-1)
        {
        }

        String typefaceName;
        int lastUsageCount;
        int flags;
        Typeface::Ptr typeFace;
    };

    int counter;
    OwnedArray <CachedFace> faces;

    TypefaceCache (const TypefaceCache&);
    const TypefaceCache& operator= (const TypefaceCache&);
};

juce_ImplementSingleton_SingleThreaded (TypefaceCache)


Typeface* Font::getTypeface() const throw()
{
    if (font->typeface == 0)
        font->typeface = TypefaceCache::getInstance()->findTypefaceFor (*this);

    return font->typeface;
}


//==============================================================================
class FontGlyphAlphaMap
{
public:
    //==============================================================================
    FontGlyphAlphaMap() throw()
        : glyph (0), lastAccessCount (0)
    {
        bitmap[0] = bitmap[1] = 0;
    }

    ~FontGlyphAlphaMap() throw()
    {
        delete bitmap[0];
        delete bitmap[1];
    }

    void draw (LowLevelGraphicsContext& g, float x, const float y) const throw()
    {
        if (bitmap[0] != 0)
        {
            const float xFloor = floorf (x);
            const int bitmapToUse = ((x - xFloor) >= 0.5f && bitmap[1] != 0) ? 1 : 0;

            g.fillAlphaChannel (*bitmap [bitmapToUse],
                                xOrigin [bitmapToUse] + (int) xFloor,
                                yOrigin [bitmapToUse] + (int) floorf (y));
        }
    }

    void generate (const Font& font_, const int glyph_) throw()
    {
        font = font_;
        glyph = glyph_;

        deleteAndZero (bitmap[0]);
        deleteAndZero (bitmap[1]);

        Path glyphPath;
        font.getTypeface()->getOutlineForGlyph (glyph_, glyphPath);

        if (! glyphPath.isEmpty())
        {
            const float fontHeight = font.getHeight();
            const float fontHScale = fontHeight * font.getHorizontalScale();
            AffineTransform transform (AffineTransform::scale (fontHScale, fontHeight));
            Rectangle clip (-2048, -2048, 4096, 4096), pos;

            bitmap[0] = glyphPath.createMaskBitmap (transform, clip, pos);
            xOrigin[0] = pos.getX();
            yOrigin[0] = pos.getY();

            if (fontHScale < 30.0f)
            {
                bitmap[1] = glyphPath.createMaskBitmap (transform.translated (0.5f, 0.0f), clip, pos);
                xOrigin[1] = pos.getX();
                yOrigin[1] = pos.getY();
            }
        }
    }

    int glyph, lastAccessCount;
    Font font;

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    Image* bitmap[2];
    int xOrigin[2], yOrigin[2];

    FontGlyphAlphaMap (const FontGlyphAlphaMap&);
    const FontGlyphAlphaMap& operator= (const FontGlyphAlphaMap&);
};


//==============================================================================
class GlyphCache  : private DeletedAtShutdown
{
public:
    GlyphCache() throw()
        : accessCounter (0)
    {
        setCacheSize (120);
    }

    ~GlyphCache() throw()
    {
        clearSingletonInstance();
    }

    juce_DeclareSingleton_SingleThreaded_Minimal (GlyphCache);

    //==============================================================================
    void drawGlyph (LowLevelGraphicsContext& g, const Font& font, int glyphNumber, float x, float y) throw()
    {
        ++accessCounter;

        int oldestCounter = INT_MAX;
        FontGlyphAlphaMap* oldest = 0;

        for (int i = glyphs.size(); --i >= 0;)
        {
            FontGlyphAlphaMap* const glyph = glyphs.getUnchecked (i);

            if (glyph->glyph == glyphNumber
                 && glyph->font == font)
            {
                ++hits;
                glyph->lastAccessCount = accessCounter;
                glyph->draw (g, x, y);
                return;
            }

            if (glyph->lastAccessCount <= oldestCounter)
            {
                oldestCounter = glyph->lastAccessCount;
                oldest = glyph;
            }
        }

        ++misses;

        if (hits + misses > (glyphs.size() << 4))
        {
            if (misses * 2 > hits)
                setCacheSize (glyphs.size() + 32);

            hits = 0;
            misses = 0;
            oldest = glyphs.getUnchecked (0);
        }

        jassert (oldest != 0);
        oldest->lastAccessCount = accessCounter;
        oldest->generate (font, glyphNumber);
        oldest->draw (g, x, y);
    }

    void setCacheSize (int num) throw()
    {
        if (glyphs.size() != num)
        {
            glyphs.clear();

            while (--num >= 0)
                glyphs.add (new FontGlyphAlphaMap());

            hits = 0;
            misses = 0;
        }
    }

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    OwnedArray <FontGlyphAlphaMap> glyphs;
    int accessCounter, hits, misses;

    GlyphCache (const GlyphCache&);
    const GlyphCache& operator= (const GlyphCache&);
};

juce_ImplementSingleton_SingleThreaded (GlyphCache);


//==============================================================================
void Font::renderGlyphIndirectly (LowLevelGraphicsContext& g, int glyphNumber, float x, float y)
{
    if (font->height < 80.0f)
        GlyphCache::getInstance()->drawGlyph (g, *this, glyphNumber, x, y);
    else
        renderGlyphIndirectly (g, glyphNumber, AffineTransform::translation (x, y));
}

void Font::renderGlyphIndirectly (LowLevelGraphicsContext& g, int glyphNumber, const AffineTransform& transform)
{
    Path p;
    getTypeface()->getOutlineForGlyph (glyphNumber, p);

    g.fillPath (p, AffineTransform::scale (font->height * font->horizontalScale, font->height)
                                   .followedBy (transform));
}


END_JUCE_NAMESPACE
