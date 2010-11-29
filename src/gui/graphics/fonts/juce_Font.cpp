/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

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
namespace FontValues
{
    float limitFontHeight (const float height) throw()
    {
        return jlimit (0.1f, 10000.0f, height);
    }

    const float defaultFontHeight = 14.0f;
    String fallbackFont;
}

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
Font::Font()
    : font (new SharedFontInternal (getDefaultSansSerifFontName(), FontValues::defaultFontHeight,
                                    1.0f, 0, 0, Font::plain, 0))
{
}

Font::Font (const float fontHeight, const int styleFlags_)
    : font (new SharedFontInternal (getDefaultSansSerifFontName(), FontValues::limitFontHeight (fontHeight),
                                    1.0f, 0, 0, styleFlags_, 0))
{
}

Font::Font (const String& typefaceName_,
            const float fontHeight,
            const int styleFlags_)
    : font (new SharedFontInternal (typefaceName_, FontValues::limitFontHeight (fontHeight),
                                    1.0f, 0, 0, styleFlags_, 0))
{
}

Font::Font (const Font& other) throw()
    : font (other.font)
{
}

Font& Font::operator= (const Font& other) throw()
{
    font = other.font;
    return *this;
}

Font::~Font() throw()
{
}

Font::Font (const Typeface::Ptr& typeface)
    : font (new SharedFontInternal (typeface->getName(), FontValues::defaultFontHeight,
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

void Font::dupeInternalIfShared()
{
    if (font->getReferenceCount() > 1)
        font = new SharedFontInternal (*font);
}

//==============================================================================
const String Font::getDefaultSansSerifFontName()
{
    static const String name ("<Sans-Serif>");
    return name;
}

const String Font::getDefaultSerifFontName()
{
    static const String name ("<Serif>");
    return name;
}

const String Font::getDefaultMonospacedFontName()
{
    static const String name ("<Monospaced>");
    return name;
}

void Font::setTypefaceName (const String& faceName)
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
const String Font::getFallbackFontName()
{
    return FontValues::fallbackFont;
}

void Font::setFallbackFontName (const String& name)
{
    FontValues::fallbackFont = name;
}

//==============================================================================
void Font::setHeight (float newHeight)
{
    newHeight = FontValues::limitFontHeight (newHeight);

    if (font->height != newHeight)
    {
        dupeInternalIfShared();
        font->height = newHeight;
    }
}

void Font::setHeightWithoutChangingWidth (float newHeight)
{
    newHeight = FontValues::limitFontHeight (newHeight);

    if (font->height != newHeight)
    {
        dupeInternalIfShared();
        font->horizontalScale *= (font->height / newHeight);
        font->height = newHeight;
    }
}

void Font::setStyleFlags (const int newFlags)
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
                            const float newKerningAmount)
{
    newHeight = FontValues::limitFontHeight (newHeight);

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

void Font::setHorizontalScale (const float scaleFactor)
{
    dupeInternalIfShared();
    font->horizontalScale = scaleFactor;
}

void Font::setExtraKerningFactor (const float extraKerning)
{
    dupeInternalIfShared();
    font->kerning = extraKerning;
}

void Font::setBold (const bool shouldBeBold)
{
    setStyleFlags (shouldBeBold ? (font->styleFlags | bold)
                                : (font->styleFlags & ~bold));
}

const Font Font::boldened() const
{
    Font f (*this);
    f.setBold (true);
    return f;
}

bool Font::isBold() const throw()
{
    return (font->styleFlags & bold) != 0;
}

void Font::setItalic (const bool shouldBeItalic)
{
    setStyleFlags (shouldBeItalic ? (font->styleFlags | italic)
                                  : (font->styleFlags & ~italic));
}

const Font Font::italicised() const
{
    Font f (*this);
    f.setItalic (true);
    return f;
}

bool Font::isItalic() const throw()
{
    return (font->styleFlags & italic) != 0;
}

void Font::setUnderline (const bool shouldBeUnderlined)
{
    setStyleFlags (shouldBeUnderlined ? (font->styleFlags | underlined)
                                      : (font->styleFlags & ~underlined));
}

bool Font::isUnderlined() const throw()
{
    return (font->styleFlags & underlined) != 0;
}

float Font::getAscent() const
{
    if (font->ascent == 0)
        font->ascent = getTypeface()->getAscent();

    return font->height * font->ascent;
}

float Font::getDescent() const
{
    return font->height - getAscent();
}

int Font::getStringWidth (const String& text) const
{
    return roundToInt (getStringWidthFloat (text));
}

float Font::getStringWidthFloat (const String& text) const
{
    float w = getTypeface()->getStringWidth (text);

    if (font->kerning != 0)
        w += font->kerning * text.length();

    return w * font->height * font->horizontalScale;
}

void Font::getGlyphPositions (const String& text, Array <int>& glyphs, Array <float>& xOffsets) const
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

void Font::findFonts (Array<Font>& destArray)
{
    const StringArray names (findAllTypefaceNames());

    for (int i = 0; i < names.size(); ++i)
        destArray.add (Font (names[i], FontValues::defaultFontHeight, Font::plain));
}

//==============================================================================
const String Font::toString() const
{
    String s (getTypefaceName());

    if (s == getDefaultSansSerifFontName())
        s = String::empty;
    else
        s += "; ";

    s += String (getHeight(), 1);

    if (isBold())
        s += " bold";

    if (isItalic())
        s += " italic";

    return s;
}

const Font Font::fromString (const String& fontDescription)
{
    String name;

    const int separator = fontDescription.indexOfChar (';');

    if (separator > 0)
        name = fontDescription.substring (0, separator).trim();

    if (name.isEmpty())
        name = getDefaultSansSerifFontName();

    String sizeAndStyle (fontDescription.substring (separator + 1));

    float height = sizeAndStyle.getFloatValue();
    if (height <= 0)
        height = 10.0f;

    int flags = Font::plain;
    if (sizeAndStyle.containsIgnoreCase ("bold"))
        flags |= Font::bold;
    if (sizeAndStyle.containsIgnoreCase ("italic"))
        flags |= Font::italic;

    return Font (name, height, flags);
}

//==============================================================================
class TypefaceCache  : public DeletedAtShutdown
{
public:
    TypefaceCache (int numToCache = 10)
        : counter (1)
    {
        while (--numToCache >= 0)
            faces.add (new CachedFace());
    }

    ~TypefaceCache()
    {
        clearSingletonInstance();
    }

    juce_DeclareSingleton_SingleThreaded_Minimal (TypefaceCache)

    const Typeface::Ptr findTypefaceFor (const Font& font)
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
        int bestLastUsageCount = std::numeric_limits<int>::max();

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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TypefaceCache);
};

juce_ImplementSingleton_SingleThreaded (TypefaceCache)


Typeface* Font::getTypeface() const
{
    if (font->typeface == 0)
        font->typeface = TypefaceCache::getInstance()->findTypefaceFor (*this);

    return font->typeface;
}


END_JUCE_NAMESPACE
