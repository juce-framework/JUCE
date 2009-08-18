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

#include "../../../../juce_core/basics/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_Font.h"
#include "juce_GlyphArrangement.h"


//==============================================================================
static const float minFontHeight = 0.1f;
static const float maxFontHeight = 10000.0f;
static const float defaultFontHeight = 14.0f;


//==============================================================================
Font::Font() throw()
    : typefaceName (Typeface::defaultTypefaceNameSans),
      height (defaultFontHeight),
      horizontalScale (1.0f),
      kerning (0),
      ascent (0),
      styleFlags (Font::plain)
{
}

void Font::resetToDefaultState() throw()
{
    typefaceName = Typeface::defaultTypefaceNameSans;
    height = defaultFontHeight;
    horizontalScale = 1.0f;
    kerning = 0;
    ascent = 0;
    styleFlags = Font::plain;
    typeface = 0;
}

Font::Font (const float fontHeight,
            const int styleFlags_) throw()
    : typefaceName (Typeface::defaultTypefaceNameSans),
      height (jlimit (minFontHeight, maxFontHeight, fontHeight)),
      horizontalScale (1.0f),
      kerning (0),
      ascent (0),
      styleFlags (styleFlags_)
{
}

Font::Font (const String& typefaceName_,
            const float fontHeight,
            const int styleFlags_) throw()
    : typefaceName (typefaceName_),
      height (jlimit (minFontHeight, maxFontHeight, fontHeight)),
      horizontalScale (1.0f),
      kerning (0),
      ascent (0),
      styleFlags (styleFlags_)
{
}

Font::Font (const Font& other) throw()
    : typefaceName (other.typefaceName),
      height (other.height),
      horizontalScale (other.horizontalScale),
      kerning (other.kerning),
      ascent (other.ascent),
      styleFlags (other.styleFlags),
      typeface (other.typeface)
{
}

const Font& Font::operator= (const Font& other) throw()
{
    if (this != &other)
    {
        typefaceName = other.typefaceName;
        height = other.height;
        styleFlags = other.styleFlags;
        horizontalScale = other.horizontalScale;
        kerning = other.kerning;
        ascent = other.ascent;
        typeface = other.typeface;
    }

    return *this;
}

Font::~Font() throw()
{
}

Font::Font (const Typeface& face) throw()
    : height (11.0f),
      horizontalScale (1.0f),
      kerning (0),
      ascent (0),
      styleFlags (plain)
{
    typefaceName = face.getName();
    setBold (face.isBold());
    setItalic (face.isItalic());
    typeface = new Typeface (face);
}

bool Font::operator== (const Font& other) const throw()
{
    return height == other.height
            && horizontalScale == other.horizontalScale
            && kerning == other.kerning
            && styleFlags == other.styleFlags
            && typefaceName == other.typefaceName;
}

bool Font::operator!= (const Font& other) const throw()
{
    return ! operator== (other);
}

//==============================================================================
void Font::setTypefaceName (const String& faceName) throw()
{
    typefaceName = faceName;
    typeface = 0;
    ascent = 0;
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
    height = jlimit (minFontHeight, maxFontHeight, newHeight);
}

void Font::setHeightWithoutChangingWidth (float newHeight) throw()
{
    newHeight = jlimit (minFontHeight, maxFontHeight, newHeight);
    horizontalScale *= (height / newHeight);
    height = newHeight;
}

void Font::setStyleFlags (const int newFlags) throw()
{
    if (styleFlags != newFlags)
    {
        styleFlags = newFlags;
        typeface = 0;
        ascent = 0;
    }
}

void Font::setSizeAndStyle (const float newHeight,
                            const int newStyleFlags,
                            const float newHorizontalScale,
                            const float newKerningAmount) throw()
{
    height = jlimit (minFontHeight, maxFontHeight, newHeight);
    horizontalScale = newHorizontalScale;
    kerning = newKerningAmount;

    setStyleFlags (newStyleFlags);
}

void Font::setHorizontalScale (const float scaleFactor) throw()
{
    horizontalScale = scaleFactor;
}

void Font::setExtraKerningFactor (const float extraKerning) throw()
{
    kerning = extraKerning;
}

void Font::setBold (const bool shouldBeBold) throw()
{
    setStyleFlags (shouldBeBold ? (styleFlags | bold)
                                : (styleFlags & ~bold));
}

bool Font::isBold() const throw()
{
    return (styleFlags & bold) != 0;
}

void Font::setItalic (const bool shouldBeItalic) throw()
{
    setStyleFlags (shouldBeItalic ? (styleFlags | italic)
                                  : (styleFlags & ~italic));
}

bool Font::isItalic() const throw()
{
    return (styleFlags & italic) != 0;
}

void Font::setUnderline (const bool shouldBeUnderlined) throw()
{
    setStyleFlags (shouldBeUnderlined ? (styleFlags | underlined)
                                      : (styleFlags & ~underlined));
}

bool Font::isUnderlined() const throw()
{
    return (styleFlags & underlined) != 0;
}

float Font::getAscent() const throw()
{
    if (ascent == 0)
        ascent = getTypeface()->getAscent();

    return height * ascent;
}

float Font::getDescent() const throw()
{
    return height - getAscent();
}

int Font::getStringWidth (const String& text) const throw()
{
    return roundFloatToInt (getStringWidthFloat (text));
}

float Font::getStringWidthFloat (const String& text) const throw()
{
    float x = 0.0f;

    if (text.isNotEmpty())
    {
        Typeface* const typeface = getTypeface();
        const juce_wchar* t = (const juce_wchar*) text;

        do
        {
            const TypefaceGlyphInfo* const glyph = typeface->getGlyph (*t++);

            if (glyph != 0)
                x += kerning + glyph->getHorizontalSpacing (*t);
        }
        while (*t != 0);

        x *= height;
        x *= horizontalScale;
    }

    return x;
}

Typeface* Font::getTypeface() const throw()
{
    if (typeface == 0)
        typeface = Typeface::getTypefaceFor (*this);

    return typeface;
}

void Font::findFonts (OwnedArray<Font>& destArray) throw()
{
    const StringArray names (findAllTypefaceNames());

    for (int i = 0; i < names.size(); ++i)
        destArray.add (new Font (names[i], defaultFontHeight, Font::plain));
}

END_JUCE_NAMESPACE
