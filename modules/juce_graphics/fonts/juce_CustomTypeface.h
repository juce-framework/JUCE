/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
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

//==============================================================================
/**
    A typeface that can be populated with custom glyphs.

    You can create a CustomTypeface if you need one that contains your own glyphs,
    or if you need to load a typeface from a Juce-formatted binary stream.

    If you want to create a copy of a native face, you can use addGlyphsFromOtherTypeface()
    to copy glyphs into this face.

    NOTE! For most people this class is almost certainly NOT the right tool to use!
    If what you want to do is to embed a font into your exe, then your best plan is
    probably to embed your TTF/OTF font file into your binary using the Projucer,
    and then call Typeface::createSystemTypefaceFor() to load it from memory.

    @see Typeface, Font

    @tags{Graphics}
*/
class JUCE_API  CustomTypeface  : public Typeface
{
public:
    //==============================================================================
    /** Creates a new, empty typeface. */
    CustomTypeface();

    /** Loads a typeface from a previously saved stream.
        The stream must have been created by writeToStream().

        NOTE! Since this class was written, support was added for loading real font files from
        memory, so for most people, using Typeface::createSystemTypefaceFor() to load a real font
        is more appropriate than using this class to store it in a proprietary format.

        @see writeToStream
    */
    explicit CustomTypeface (InputStream& serialisedTypefaceStream);

    /** Destructor. */
    ~CustomTypeface() override;

    //==============================================================================
    /** Resets this typeface, deleting all its glyphs and settings. */
    void clear();

    /** Sets the vital statistics for the typeface.
        @param fontFamily the typeface's font family
        @param ascent     the ascent - this is normalised to a height of 1.0 and this is
                          the value that will be returned by Typeface::getAscent(). The
                          descent is assumed to be (1.0 - ascent)
        @param isBold     should be true if the typeface is bold
        @param isItalic   should be true if the typeface is italic
        @param defaultCharacter   the character to be used as a replacement if there's
                          no glyph available for the character that's being drawn
    */
    void setCharacteristics (const String& fontFamily, float ascent,
                             bool isBold, bool isItalic,
                             juce_wchar defaultCharacter) noexcept;

    /** Sets the vital statistics for the typeface.
        @param fontFamily the typeface's font family
        @param fontStyle  the typeface's font style
        @param ascent     the ascent - this is normalised to a height of 1.0 and this is
                          the value that will be returned by Typeface::getAscent(). The
                          descent is assumed to be (1.0 - ascent)
        @param defaultCharacter  the character to be used as a replacement if there's
                          no glyph available for the character that's being drawn
    */
    void setCharacteristics (const String& fontFamily, const String& fontStyle,
                             float ascent, juce_wchar defaultCharacter) noexcept;

    /** Adds a glyph to the typeface.

        The path that is passed in is normalised so that the font height is 1.0, and its
        origin is the anchor point of the character on its baseline.

        The width is the nominal width of the character, and any extra kerning values that
        are specified will be added to this width.
    */
    void addGlyph (juce_wchar character, const Path& path, float width) noexcept;

    /** Specifies an extra kerning amount to be used between a pair of characters.
        The amount will be added to the nominal width of the first character when laying out a string.
    */
    void addKerningPair (juce_wchar char1, juce_wchar char2, float extraAmount) noexcept;

    /** Adds a range of glyphs from another typeface.
        This will attempt to pull in the paths and kerning information from another typeface and
        add it to this one.
    */
    void addGlyphsFromOtherTypeface (Typeface& typefaceToCopy, juce_wchar characterStartIndex, int numCharacters) noexcept;

    /** Saves this typeface as a Juce-formatted font file.
        A CustomTypeface can be created to reload the data that is written - see the CustomTypeface
        constructor.

        NOTE! Since this class was written, support was added for loading real font files from
        memory, so for most people, using Typeface::createSystemTypefaceFor() to load a real font
        is more appropriate than using this class to store it in a proprietary format.
    */
    bool writeToStream (OutputStream& outputStream);

    //==============================================================================
    // The following methods implement the basic Typeface behaviour.
    float getAscent() const override;
    float getDescent() const override;
    float getHeightToPointsFactor() const override;
    float getStringWidth (const String&) override;
    void getGlyphPositions (const String&, Array<int>& glyphs, Array<float>& xOffsets) override;
    bool getOutlineForGlyph (int glyphNumber, Path&) override;
    EdgeTable* getEdgeTableForGlyph (int glyphNumber, const AffineTransform&, float fontHeight) override;

protected:
    //==============================================================================
    juce_wchar defaultCharacter;
    float ascent;

    //==============================================================================
    /** If a subclass overrides this, it can load glyphs into the font on-demand.
        When methods such as getGlyphPositions() or getOutlineForGlyph() are asked for a
        particular character and there's no corresponding glyph, they'll call this
        method so that a subclass can try to add that glyph, returning true if it
        manages to do so.
    */
    virtual bool loadGlyphIfPossible (juce_wchar characterNeeded);

private:
    //==============================================================================
    class GlyphInfo;
    OwnedArray<GlyphInfo> glyphs;
    short lookupTable[128];

    GlyphInfo* findGlyph (const juce_wchar character, bool loadIfNeeded) noexcept;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CustomTypeface)
};

} // namespace juce
