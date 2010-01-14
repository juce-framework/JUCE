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
    return roundToInt (getStringWidthFloat (text));
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

    if (num > 0)
    {
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
        : counter (1)
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


END_JUCE_NAMESPACE
