/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

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

class CustomTypeface::GlyphInfo
{
public:
    GlyphInfo (const juce_wchar character_, const Path& path_, const float width_) noexcept
        : character (character_), path (path_), width (width_)
    {
    }

    struct KerningPair
    {
        juce_wchar character2;
        float kerningAmount;
    };

    void addKerningPair (const juce_wchar subsequentCharacter,
                         const float extraKerningAmount) noexcept
    {
        KerningPair kp;
        kp.character2 = subsequentCharacter;
        kp.kerningAmount = extraKerningAmount;
        kerningPairs.add (kp);
    }

    float getHorizontalSpacing (const juce_wchar subsequentCharacter) const noexcept
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
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GlyphInfo)
};

//==============================================================================
namespace CustomTypefaceHelpers
{
    static juce_wchar readChar (InputStream& in)
    {
        uint32 n = (uint32) (uint16) in.readShort();

        if (n >= 0xd800 && n <= 0xdfff)
        {
            const uint32 nextWord = (uint32) (uint16) in.readShort();
            jassert (nextWord >= 0xdc00); // illegal unicode character!

            n = 0x10000 + (((n - 0xd800) << 10) | (nextWord - 0xdc00));
        }

        return (juce_wchar) n;
    }

    static void writeChar (OutputStream& out, juce_wchar charToWrite)
    {
        if (charToWrite >= 0x10000)
        {
            charToWrite -= 0x10000;
            out.writeShort ((short) (uint16) (0xd800 + (charToWrite >> 10)));
            out.writeShort ((short) (uint16) (0xdc00 + (charToWrite & 0x3ff)));
        }
        else
        {
            out.writeShort ((short) (uint16) charToWrite);
        }
    }
}

//==============================================================================
CustomTypeface::CustomTypeface()
    : Typeface (String::empty, String::empty)
{
    clear();
}

CustomTypeface::CustomTypeface (InputStream& serialisedTypefaceStream)
    : Typeface (String::empty, String::empty)
{
    clear();

    GZIPDecompressorInputStream gzin (serialisedTypefaceStream);
    BufferedInputStream in (gzin, 32768);

    name = in.readString();

    const bool isBold   = in.readBool();
    const bool isItalic = in.readBool();
    style = FontStyleHelpers::getStyleName (isBold, isItalic);

    ascent = in.readFloat();
    defaultCharacter = CustomTypefaceHelpers::readChar (in);

    int i, numChars = in.readInt();

    for (i = 0; i < numChars; ++i)
    {
        const juce_wchar c = CustomTypefaceHelpers::readChar (in);
        const float width = in.readFloat();

        Path p;
        p.loadPathFromStream (in);
        addGlyph (c, p, width);
    }

    const int numKerningPairs = in.readInt();

    for (i = 0; i < numKerningPairs; ++i)
    {
        const juce_wchar char1 = CustomTypefaceHelpers::readChar (in);
        const juce_wchar char2 = CustomTypefaceHelpers::readChar (in);

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
    style = "Regular";
    zeromem (lookupTable, sizeof (lookupTable));
    glyphs.clear();
}

void CustomTypeface::setCharacteristics (const String& name_, const float ascent_, const bool isBold,
                                         const bool isItalic, const juce_wchar defaultCharacter_) noexcept
{
    name = name_;
    defaultCharacter = defaultCharacter_;
    ascent = ascent_;
    style = FontStyleHelpers::getStyleName (isBold, isItalic);
}

void CustomTypeface::setCharacteristics (const String& name_, const String& style_, const float ascent_,
                                         const juce_wchar defaultCharacter_) noexcept
{
    name = name_;
    style = style_;
    defaultCharacter = defaultCharacter_;
    ascent = ascent_;
}

void CustomTypeface::addGlyph (const juce_wchar character, const Path& path, const float width) noexcept
{
    // Check that you're not trying to add the same character twice..
    jassert (findGlyph (character, false) == nullptr);

    if (isPositiveAndBelow ((int) character, (int) numElementsInArray (lookupTable)))
        lookupTable [character] = (short) glyphs.size();

    glyphs.add (new GlyphInfo (character, path, width));
}

void CustomTypeface::addKerningPair (const juce_wchar char1, const juce_wchar char2, const float extraAmount) noexcept
{
    if (extraAmount != 0)
    {
        if (GlyphInfo* const g = findGlyph (char1, true))
            g->addKerningPair (char2, extraAmount);
        else
            jassertfalse; // can only add kerning pairs for characters that exist!
    }
}

CustomTypeface::GlyphInfo* CustomTypeface::findGlyph (const juce_wchar character, const bool loadIfNeeded) noexcept
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

    return nullptr;
}

bool CustomTypeface::loadGlyphIfPossible (const juce_wchar /*characterNeeded*/)
{
    return false;
}

void CustomTypeface::addGlyphsFromOtherTypeface (Typeface& typefaceToCopy, juce_wchar characterStartIndex, int numCharacters) noexcept
{
    setCharacteristics (name, style, typefaceToCopy.getAscent(), defaultCharacter);

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
    out.writeBool (FontStyleHelpers::isBold (style));
    out.writeBool (FontStyleHelpers::isItalic (style));
    out.writeFloat (ascent);
    CustomTypefaceHelpers::writeChar (out, defaultCharacter);
    out.writeInt (glyphs.size());

    int i, numKerningPairs = 0;

    for (i = 0; i < glyphs.size(); ++i)
    {
        const GlyphInfo* const g = glyphs.getUnchecked (i);
        CustomTypefaceHelpers::writeChar (out, g->character);
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
            CustomTypefaceHelpers::writeChar (out, g->character);
            CustomTypefaceHelpers::writeChar (out, p.character2);
            out.writeFloat (p.kerningAmount);
        }
    }

    return true;
}

//==============================================================================
float CustomTypeface::getAscent() const                 { return ascent; }
float CustomTypeface::getDescent() const                { return 1.0f - ascent; }
float CustomTypeface::getHeightToPointsFactor() const   { return ascent; }

float CustomTypeface::getStringWidth (const String& text)
{
    float x = 0;
    String::CharPointerType t (text.getCharPointer());

    while (! t.isEmpty())
    {
        const juce_wchar c = t.getAndAdvance();
        const GlyphInfo* const glyph = findGlyph (c, true);

        if (glyph == nullptr)
        {
            const Typeface::Ptr fallbackTypeface (Typeface::getFallbackTypeface());

            if (fallbackTypeface != nullptr && fallbackTypeface != this)
                x += fallbackTypeface->getStringWidth (String::charToString (c));
        }

        if (glyph != nullptr)
            x += glyph->getHorizontalSpacing (*t);
    }

    return x;
}

void CustomTypeface::getGlyphPositions (const String& text, Array <int>& resultGlyphs, Array<float>& xOffsets)
{
    xOffsets.add (0);
    float x = 0;
    String::CharPointerType t (text.getCharPointer());

    while (! t.isEmpty())
    {
        const juce_wchar c = t.getAndAdvance();
        const GlyphInfo* const glyph = findGlyph (c, true);

        if (glyph == nullptr)
        {
            const Typeface::Ptr fallbackTypeface (Typeface::getFallbackTypeface());

            if (fallbackTypeface != nullptr && fallbackTypeface != this)
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

        if (glyph != nullptr)
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

    if (glyph == nullptr)
    {
        const Typeface::Ptr fallbackTypeface (Typeface::getFallbackTypeface());

        if (fallbackTypeface != nullptr && fallbackTypeface != this)
            fallbackTypeface->getOutlineForGlyph (glyphNumber, path);
    }

    if (glyph != nullptr)
    {
        path = glyph->path;
        return true;
    }

    return false;
}

EdgeTable* CustomTypeface::getEdgeTableForGlyph (int glyphNumber, const AffineTransform& transform)
{
    const GlyphInfo* const glyph = findGlyph ((juce_wchar) glyphNumber, true);

    if (glyph == nullptr)
    {
        const Typeface::Ptr fallbackTypeface (Typeface::getFallbackTypeface());

        if (fallbackTypeface != nullptr && fallbackTypeface != this)
            return fallbackTypeface->getEdgeTableForGlyph (glyphNumber, transform);
    }

    if (glyph != nullptr && ! glyph->path.isEmpty())
        return new EdgeTable (glyph->path.getBoundsTransformed (transform).getSmallestIntegerContainer().expanded (1, 0),
                              glyph->path, transform);

    return nullptr;
}
