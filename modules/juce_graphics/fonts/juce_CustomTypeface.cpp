/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

class CustomTypeface::GlyphInfo
{
public:
    GlyphInfo (juce_wchar c, const Path& p, float w) noexcept
        : character (c), path (p), width (w)
    {
    }

    struct KerningPair
    {
        juce_wchar character2;
        float kerningAmount;
    };

    void addKerningPair (juce_wchar subsequentCharacter, float extraKerningAmount) noexcept
    {
        kerningPairs.add ({ subsequentCharacter, extraKerningAmount });
    }

    float getHorizontalSpacing (juce_wchar subsequentCharacter) const noexcept
    {
        if (subsequentCharacter != 0)
            for (auto& kp : kerningPairs)
                if (kp.character2 == subsequentCharacter)
                    return width + kp.kerningAmount;

        return width;
    }

    const juce_wchar character;
    const Path path;
    float width;
    Array<KerningPair> kerningPairs;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GlyphInfo)
};

//==============================================================================
namespace CustomTypefaceHelpers
{
    static juce_wchar readChar (InputStream& in)
    {
        auto n = (uint32) (uint16) in.readShort();

        if (n >= 0xd800 && n <= 0xdfff)
        {
            auto nextWord = (uint32) (uint16) in.readShort();
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
    : Typeface (String(), String())
{
    clear();
}

CustomTypeface::CustomTypeface (InputStream& serialisedTypefaceStream)
    : Typeface (String(), String())
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

    auto numChars = in.readInt();

    for (int i = 0; i < numChars; ++i)
    {
        auto c = CustomTypefaceHelpers::readChar (in);
        auto width = in.readFloat();

        Path p;
        p.loadPathFromStream (in);
        addGlyph (c, p, width);
    }

    auto numKerningPairs = in.readInt();

    for (int i = 0; i < numKerningPairs; ++i)
    {
        auto char1 = CustomTypefaceHelpers::readChar (in);
        auto char2 = CustomTypefaceHelpers::readChar (in);

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

void CustomTypeface::setCharacteristics (const String& newName, float newAscent, bool isBold,
                                         bool isItalic, juce_wchar newDefaultCharacter) noexcept
{
    name = newName;
    defaultCharacter = newDefaultCharacter;
    ascent = newAscent;
    style = FontStyleHelpers::getStyleName (isBold, isItalic);
}

void CustomTypeface::setCharacteristics (const String& newName, const String& newStyle,
                                         float newAscent, juce_wchar newDefaultCharacter) noexcept
{
    name = newName;
    style = newStyle;
    defaultCharacter = newDefaultCharacter;
    ascent = newAscent;
}

void CustomTypeface::addGlyph (juce_wchar character, const Path& path, float width) noexcept
{
    // Check that you're not trying to add the same character twice..
    jassert (findGlyph (character, false) == nullptr);

    if (isPositiveAndBelow ((int) character, numElementsInArray (lookupTable)))
        lookupTable [character] = (short) glyphs.size();

    glyphs.add (new GlyphInfo (character, path, width));
}

void CustomTypeface::addKerningPair (juce_wchar char1, juce_wchar char2, float extraAmount) noexcept
{
    if (extraAmount != 0.0f)
    {
        if (auto* g = findGlyph (char1, true))
            g->addKerningPair (char2, extraAmount);
        else
            jassertfalse; // can only add kerning pairs for characters that exist!
    }
}

CustomTypeface::GlyphInfo* CustomTypeface::findGlyph (juce_wchar character, bool loadIfNeeded) noexcept
{
    if (isPositiveAndBelow ((int) character, numElementsInArray (lookupTable)) && lookupTable [character] > 0)
        return glyphs [(int) lookupTable [(int) character]];

    for (auto* g : glyphs)
        if (g->character == character)
            return g;

    if (loadIfNeeded && loadGlyphIfPossible (character))
        return findGlyph (character, false);

    return nullptr;
}

bool CustomTypeface::loadGlyphIfPossible (juce_wchar)
{
    return false;
}

void CustomTypeface::addGlyphsFromOtherTypeface (Typeface& typefaceToCopy, juce_wchar characterStartIndex, int numCharacters) noexcept
{
    setCharacteristics (name, style, typefaceToCopy.getAscent(), defaultCharacter);

    for (int i = 0; i < numCharacters; ++i)
    {
        auto c = (juce_wchar) (characterStartIndex + static_cast<juce_wchar> (i));

        Array<int> glyphIndexes;
        Array<float> offsets;
        typefaceToCopy.getGlyphPositions (String::charToString (c), glyphIndexes, offsets);

        const int glyphIndex = glyphIndexes.getFirst();

        if (glyphIndex >= 0 && glyphIndexes.size() > 0)
        {
            auto glyphWidth = offsets[1];

            Path p;
            typefaceToCopy.getOutlineForGlyph (glyphIndex, p);

            addGlyph (c, p, glyphWidth);

            for (int j = glyphs.size() - 1; --j >= 0;)
            {
                auto char2 = glyphs.getUnchecked (j)->character;
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
    GZIPCompressorOutputStream out (outputStream);

    out.writeString (name);
    out.writeBool (FontStyleHelpers::isBold (style));
    out.writeBool (FontStyleHelpers::isItalic (style));
    out.writeFloat (ascent);
    CustomTypefaceHelpers::writeChar (out, defaultCharacter);
    out.writeInt (glyphs.size());

    int numKerningPairs = 0;

    for (auto* g : glyphs)
    {
        CustomTypefaceHelpers::writeChar (out, g->character);
        out.writeFloat (g->width);
        g->path.writePathToStream (out);

        numKerningPairs += g->kerningPairs.size();
    }

    out.writeInt (numKerningPairs);

    for (auto* g : glyphs)
    {
        for (auto& p : g->kerningPairs)
        {
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

    for (auto t = text.getCharPointer(); ! t.isEmpty();)
    {
        auto c = t.getAndAdvance();

        if (auto* glyph = findGlyph (c, true))
        {
            x += glyph->getHorizontalSpacing (*t);
        }
        else
        {
            if (auto fallbackTypeface = Typeface::getFallbackTypeface())
                if (fallbackTypeface.get() != this)
                    x += fallbackTypeface->getStringWidth (String::charToString (c));
        }
    }

    return x;
}

void CustomTypeface::getGlyphPositions (const String& text, Array<int>& resultGlyphs, Array<float>& xOffsets)
{
    xOffsets.add (0);
    float x = 0;

    for (auto t = text.getCharPointer(); ! t.isEmpty();)
    {
        float width = 0.0f;
        int glyphChar = 0;

        auto c = t.getAndAdvance();

        if (auto* glyph = findGlyph (c, true))
        {
            width = glyph->getHorizontalSpacing (*t);
            glyphChar = (int) glyph->character;
        }
        else
        {
            auto fallbackTypeface = getFallbackTypeface();

            if (fallbackTypeface != nullptr && fallbackTypeface.get() != this)
            {
                Array<int> subGlyphs;
                Array<float> subOffsets;
                fallbackTypeface->getGlyphPositions (String::charToString (c), subGlyphs, subOffsets);

                if (subGlyphs.size() > 0)
                {
                    glyphChar = subGlyphs.getFirst();
                    width = subOffsets[1];
                }
            }
        }

        x += width;
        resultGlyphs.add (glyphChar);
        xOffsets.add (x);
    }
}

bool CustomTypeface::getOutlineForGlyph (int glyphNumber, Path& path)
{
    if (auto* glyph = findGlyph ((juce_wchar) glyphNumber, true))
    {
        path = glyph->path;
        return true;
    }

    if (auto fallbackTypeface = getFallbackTypeface())
        if (fallbackTypeface.get() != this)
            return fallbackTypeface->getOutlineForGlyph (glyphNumber, path);

    return false;
}

EdgeTable* CustomTypeface::getEdgeTableForGlyph (int glyphNumber, const AffineTransform& transform, float fontHeight)
{
    if (auto* glyph = findGlyph ((juce_wchar) glyphNumber, true))
    {
        if (! glyph->path.isEmpty())
            return new EdgeTable (glyph->path.getBoundsTransformed (transform)
                                             .getSmallestIntegerContainer().expanded (1, 0),
                                  glyph->path, transform);
    }
    else
    {
        if (auto fallbackTypeface = getFallbackTypeface())
            if (fallbackTypeface.get() != this)
                return fallbackTypeface->getEdgeTableForGlyph (glyphNumber, transform, fontHeight);
    }

    return nullptr;
}

} // namespace juce
