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

#include "juce_Typeface.h"
#include "juce_Font.h"
#include "../../../io/streams/juce_GZIPDecompressorInputStream.h"
#include "../../../io/streams/juce_GZIPCompressorOutputStream.h"
#include "../../../io/streams/juce_BufferedInputStream.h"


//==============================================================================
Typeface::Typeface (const String& name_) throw()
    : name (name_), isFallbackFont (false)
{
}

Typeface::~Typeface()
{
}

const Typeface::Ptr Typeface::getFallbackTypeface()
{
    const Font fallbackFont (Font::getFallbackFontName(), 10, 0);
    Typeface* t = fallbackFont.getTypeface();
    t->isFallbackFont = true;
    return t;
}

//==============================================================================
class CustomTypeface::GlyphInfo
{
public:
    GlyphInfo (const juce_wchar character_, const Path& path_, const float width_) throw()
        : character (character_), path (path_), width (width_)
    {
    }

    struct KerningPair
    {
        juce_wchar character2;
        float kerningAmount;
    };

    void addKerningPair (const juce_wchar subsequentCharacter,
                         const float extraKerningAmount) throw()
    {
        KerningPair kp;
        kp.character2 = subsequentCharacter;
        kp.kerningAmount = extraKerningAmount;
        kerningPairs.add (kp);
    }

    float getHorizontalSpacing (const juce_wchar subsequentCharacter) const throw()
    {
        if (subsequentCharacter != 0)
        {
            for (int i = kerningPairs.size(); --i >= 0;)
                if (kerningPairs.getReference(i).character2 == subsequentCharacter)
                    return width + kerningPairs.getReference(i).kerningAmount;
        }

        return width;
    }

    const juce_wchar character;
    const Path path;
    float width;
    Array <KerningPair> kerningPairs;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GlyphInfo);
};

//==============================================================================
CustomTypeface::CustomTypeface()
    : Typeface (String::empty)
{
    clear();
}

CustomTypeface::CustomTypeface (InputStream& serialisedTypefaceStream)
    : Typeface (String::empty)
{
    clear();

    GZIPDecompressorInputStream gzin (serialisedTypefaceStream);
    BufferedInputStream in (gzin, 32768);

    name = in.readString();
    isBold = in.readBool();
    isItalic = in.readBool();
    ascent = in.readFloat();
    defaultCharacter = (juce_wchar) in.readShort();

    int i, numChars = in.readInt();

    for (i = 0; i < numChars; ++i)
    {
        const juce_wchar c = (juce_wchar) in.readShort();
        const float width = in.readFloat();

        Path p;
        p.loadPathFromStream (in);
        addGlyph (c, p, width);
    }

    const int numKerningPairs = in.readInt();

    for (i = 0; i < numKerningPairs; ++i)
    {
        const juce_wchar char1 = (juce_wchar) in.readShort();
        const juce_wchar char2 = (juce_wchar) in.readShort();

        addKerningPair (char1, char2, in.readFloat());
    }
}

CustomTypeface::~CustomTypeface()
{
}

//==============================================================================
void CustomTypeface::clear()
{
    defaultCharacter = 0;
    ascent = 1.0f;
    isBold = isItalic = false;
    zeromem (lookupTable, sizeof (lookupTable));
    glyphs.clear();
}

void CustomTypeface::setCharacteristics (const String& name_, const float ascent_, const bool isBold_,
                                         const bool isItalic_, const juce_wchar defaultCharacter_) throw()
{
    name = name_;
    defaultCharacter = defaultCharacter_;
    ascent = ascent_;
    isBold = isBold_;
    isItalic = isItalic_;
}

void CustomTypeface::addGlyph (const juce_wchar character, const Path& path, const float width) throw()
{
    // Check that you're not trying to add the same character twice..
    jassert (findGlyph (character, false) == 0);

    if (isPositiveAndBelow ((int) character, (int) numElementsInArray (lookupTable)))
        lookupTable [character] = (short) glyphs.size();

    glyphs.add (new GlyphInfo (character, path, width));
}

void CustomTypeface::addKerningPair (const juce_wchar char1, const juce_wchar char2, const float extraAmount) throw()
{
    if (extraAmount != 0)
    {
        GlyphInfo* const g = findGlyph (char1, true);
        jassert (g != 0); // can only add kerning pairs for characters that exist!

        if (g != 0)
            g->addKerningPair (char2, extraAmount);
    }
}

CustomTypeface::GlyphInfo* CustomTypeface::findGlyph (const juce_wchar character, const bool loadIfNeeded) throw()
{
    if (isPositiveAndBelow ((int) character, (int) numElementsInArray (lookupTable)) && lookupTable [character] > 0)
        return glyphs [(int) lookupTable [(int) character]];

    for (int i = 0; i < glyphs.size(); ++i)
    {
        GlyphInfo* const g = glyphs.getUnchecked(i);
        if (g->character == character)
            return g;
    }

    if (loadIfNeeded && loadGlyphIfPossible (character))
        return findGlyph (character, false);

    return 0;
}

CustomTypeface::GlyphInfo* CustomTypeface::findGlyphSubstituting (const juce_wchar character) throw()
{
    GlyphInfo* glyph = findGlyph (character, true);

    if (glyph == 0)
    {
        if (CharacterFunctions::isWhitespace (character) && character != L' ')
            glyph = findGlyph (L' ', true);

        if (glyph == 0)
        {
            const Font fallbackFont (Font::getFallbackFontName(), 10, 0);
            Typeface* const fallbackTypeface = fallbackFont.getTypeface();
            if (fallbackTypeface != 0 && fallbackTypeface != this)
            {
                Path path;
                fallbackTypeface->getOutlineForGlyph (character, path);
                addGlyph (character, path, fallbackTypeface->getStringWidth (String::charToString (character)));
            }

            if (glyph == 0)
                glyph = findGlyph (defaultCharacter, true);
        }
    }

    return glyph;
}

bool CustomTypeface::loadGlyphIfPossible (const juce_wchar /*characterNeeded*/)
{
    return false;
}

void CustomTypeface::addGlyphsFromOtherTypeface (Typeface& typefaceToCopy, juce_wchar characterStartIndex, int numCharacters) throw()
{
    setCharacteristics (name, typefaceToCopy.getAscent(), isBold, isItalic, defaultCharacter);

    for (int i = 0; i < numCharacters; ++i)
    {
        const juce_wchar c = (juce_wchar) (characterStartIndex + i);

        Array <int> glyphIndexes;
        Array <float> offsets;
        typefaceToCopy.getGlyphPositions (String::charToString (c), glyphIndexes, offsets);

        const int glyphIndex = glyphIndexes.getFirst();

        if (glyphIndex >= 0 && glyphIndexes.size() > 0)
        {
            const float glyphWidth = offsets[1];

            Path p;
            typefaceToCopy.getOutlineForGlyph (glyphIndex, p);

            addGlyph (c, p, glyphWidth);

            for (int j = glyphs.size() - 1; --j >= 0;)
            {
                const juce_wchar char2 = glyphs.getUnchecked (j)->character;
                glyphIndexes.clearQuick();
                offsets.clearQuick();
                typefaceToCopy.getGlyphPositions (String::charToString (c) + String::charToString (char2), glyphIndexes, offsets);

                if (offsets.size() > 1)
                    addKerningPair (c, char2, offsets[1] - glyphWidth);
            }
        }
    }
}

bool CustomTypeface::writeToStream (OutputStream& outputStream)
{
    GZIPCompressorOutputStream out (&outputStream);

    out.writeString (name);
    out.writeBool (isBold);
    out.writeBool (isItalic);
    out.writeFloat (ascent);
    out.writeShort ((short) (unsigned short) defaultCharacter);
    out.writeInt (glyphs.size());

    int i, numKerningPairs = 0;

    for (i = 0; i < glyphs.size(); ++i)
    {
        const GlyphInfo* const g = glyphs.getUnchecked (i);
        out.writeShort ((short) (unsigned short) g->character);
        out.writeFloat (g->width);
        g->path.writePathToStream (out);

        numKerningPairs += g->kerningPairs.size();
    }

    out.writeInt (numKerningPairs);

    for (i = 0; i < glyphs.size(); ++i)
    {
        const GlyphInfo* const g = glyphs.getUnchecked (i);

        for (int j = 0; j < g->kerningPairs.size(); ++j)
        {
            const GlyphInfo::KerningPair& p = g->kerningPairs.getReference (j);
            out.writeShort ((short) (unsigned short) g->character);
            out.writeShort ((short) (unsigned short) p.character2);
            out.writeFloat (p.kerningAmount);
        }
    }

    return true;
}

//==============================================================================
float CustomTypeface::getAscent() const
{
    return ascent;
}

float CustomTypeface::getDescent() const
{
    return 1.0f - ascent;
}

float CustomTypeface::getStringWidth (const String& text)
{
    float x = 0;
    const juce_wchar* t = text;

    while (*t != 0)
    {
        const GlyphInfo* const glyph = findGlyphSubstituting (*t++);

        if (glyph == 0 && ! isFallbackFont)
        {
            const Typeface::Ptr fallbackTypeface (Typeface::getFallbackTypeface());

            if (fallbackTypeface != 0)
                x += fallbackTypeface->getStringWidth (String::charToString (*t));
        }

        if (glyph != 0)
            x += glyph->getHorizontalSpacing (*t);
    }

    return x;
}

void CustomTypeface::getGlyphPositions (const String& text, Array <int>& resultGlyphs, Array<float>& xOffsets)
{
    xOffsets.add (0);
    float x = 0;
    const juce_wchar* t = text;

    while (*t != 0)
    {
        const juce_wchar c = *t++;
        const GlyphInfo* const glyph = findGlyph (c, true);

        if (glyph == 0 && ! isFallbackFont)
        {
            const Typeface::Ptr fallbackTypeface (Typeface::getFallbackTypeface());

            if (fallbackTypeface != 0)
            {
                Array <int> subGlyphs;
                Array <float> subOffsets;
                fallbackTypeface->getGlyphPositions (String::charToString (c), subGlyphs, subOffsets);

                if (subGlyphs.size() > 0)
                {
                    resultGlyphs.add (subGlyphs.getFirst());
                    x += subOffsets[1];
                    xOffsets.add (x);
                }
            }
        }

        if (glyph != 0)
        {
            x += glyph->getHorizontalSpacing (*t);
            resultGlyphs.add ((int) glyph->character);
            xOffsets.add (x);
        }
    }
}

bool CustomTypeface::getOutlineForGlyph (int glyphNumber, Path& path)
{
    const GlyphInfo* const glyph = findGlyph ((juce_wchar) glyphNumber, true);

    if (glyph == 0 && ! isFallbackFont)
    {
        const Typeface::Ptr fallbackTypeface (Typeface::getFallbackTypeface());

        if (fallbackTypeface != 0)
            fallbackTypeface->getOutlineForGlyph (glyphNumber, path);
    }

    if (glyph != 0)
    {
        path = glyph->path;
        return true;
    }

    return false;
}

END_JUCE_NAMESPACE
