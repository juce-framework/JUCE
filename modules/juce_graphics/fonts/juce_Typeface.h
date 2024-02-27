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

//==============================================================================
/**
    A typeface represents a size-independent font.

    This base class is abstract, but calling createSystemTypefaceFor() will return
    a platform-specific subclass that can be used.

    Normally you should never need to deal directly with Typeface objects - the Font
    class does everything you typically need for rendering text.

    @see Font

    @tags{Graphics}
*/
class JUCE_API  Typeface  : public ReferenceCountedObject
{
public:
    Typeface (const String& name, const String& newStyle) noexcept;

    //==============================================================================
    /** A handy typedef for a pointer to a typeface. */
    using Ptr = ReferenceCountedObjectPtr<Typeface>;

    //==============================================================================
    /** Returns the font family of the typeface.
        @see Font::getTypefaceName
    */
    const String& getName() const noexcept             { return name; }

    //==============================================================================
    /** Returns the font style of the typeface.
        @see Font::getTypefaceStyle
    */
    const String& getStyle() const noexcept            { return style; }

    //==============================================================================
    /** Creates a new system typeface. */
    static Ptr createSystemTypefaceFor (const Font& font);

    /** Attempts to create a font from some raw font file data (e.g. a TTF or OTF file image).
        The system will take its own internal copy of the data, so you can free the block once
        this method has returned.

        The typeface will remain registered with the system for as long as there is at least one
        owner of the returned Ptr. This allows typefaces registered with createSystemTypefaceFor to
        be created using just a typeface family name, e.g. in font fallback lists.
    */
    static Ptr createSystemTypefaceFor (const void* fontFileData, size_t fontFileDataSize);

    /** Attempts to create a font from some raw font file data (e.g. a TTF or OTF file image).
        The system will take its own internal copy of the data.

        The typeface will remain registered with the system for as long as there is at least one
        owner of the returned Ptr. This allows typefaces registered with createSystemTypefaceFor to
        be created using just a typeface family name, e.g. in font fallback lists.
    */
    static Ptr createSystemTypefaceFor (Span<const std::byte>);

    //==============================================================================
    /** Destructor. */
    ~Typeface() override;

    /** Returns the ascent of the font, as a proportion of its height.
        The height is considered to always be normalised as 1.0, so this will be a
        value less that 1.0, indicating the proportion of the font that lies above
        its baseline.
    */
    float getAscent() const;

    /** Returns the descent of the font, as a proportion of its height.
        The height is considered to always be normalised as 1.0, so this will be a
        value less that 1.0, indicating the proportion of the font that lies below
        its baseline.
    */
    float getDescent() const;

    /** Returns the value by which you should multiply a JUCE font-height value to
        convert it to the equivalent point-size.
    */
    float getHeightToPointsFactor() const;

    /** @deprecated
        This function has several shortcomings:
        - The returned value is based on a font with a normalised JUCE height of 1.0,
          which will normally differ from the value that would be expected for a font
          with a height of 1 pt.
        - This function is unsuitable for measuring text with spacing that doesn't
          scale linearly with point size, which might be the case for fonts that
          implement optical sizing.
        - The result is computed assuming that ligatures and other font features will
          not be used when rendering the string. There's also no way of specifying a
          language used for the string, which may affect the widths of CJK text.
        - If the typeface doesn't contain suitable glyphs for all characters in the
          string, missing characters will be substituted with the notdef/tofu glyph
          instead of attempting to use a different font that contains suitable
          glyphs.

        Measures the width of a line of text.
        The distance returned is based on the font having an normalised height of 1.0.
        You should never need to call this!
    */
    float getStringWidth (const String& text);

    /** @deprecated
        This function has several shortcomings:
        - The returned values are based on a font with a normalised JUCE height of 1.0,
          which will normally differ from the value that would be expected for a font
          with a height of 1 pt.
        - This function is unsuitable for measuring text with spacing that doesn't
          scale linearly with point size, which might be the case for fonts that
          implement optical sizing.
        - Ligatures are deliberately ignored, which will lead to ugly results if the
          positions are used to paint text using latin scripts, and potentially
          illegible results for other scripts. There's also no way of specifying a
          language used for the string, which may affect the rendering of CJK text.
        - If the typeface doesn't contain suitable glyphs for all characters in the
          string, missing characters will be substituted with the notdef/tofu glyph
          instead of attempting to use a different font that contains suitable
          glyphs.

        Converts a line of text into its glyph numbers and their positions.
        The distances returned are based on the font having an normalised height of 1.0.
        You should never need to call this!
    */
    void getGlyphPositions (const String& text, Array<int>& glyphs, Array<float>& xOffsets);

    /** Returns the outline for a glyph.
        The path returned will be normalised to a font height of 1.0.
    */
    void getOutlineForGlyph (int glyphNumber, Path& path);

    /** Returns a new EdgeTable that contains the path for the given glyph, with the specified transform applied. */
    EdgeTable* getEdgeTableForGlyph (int glyphNumber, const AffineTransform& transform, float fontHeight);

    //==============================================================================
    /** Changes the number of fonts that are cached in memory. */
    static void setTypefaceCacheSize (int numFontsToCache);

    /** Clears any fonts that are currently cached in memory. */
    static void clearTypefaceCache();

    /** On some platforms, this allows a specific path to be scanned.
        On macOS you can load .ttf and .otf files, otherwise this is only available when using FreeType.
    */
    static void scanFolderForFonts (const File& folder);

    /** Makes an attempt at performing a good overall distortion that will scale a font of
        the given size to align vertically with the pixel grid. The path should be an unscaled
        (i.e. normalised to height of 1.0) path for a glyph.
    */
    [[deprecated]]
    void applyVerticalHintingTransform (float fontHeight, Path& path);

    /** Returns the glyph index corresponding to the provided codepoint, or nullopt if no
        such glyph is found.
    */
    std::optional<uint32_t> getNominalGlyphForCodepoint (juce_wchar) const;

    /** @internal */
    class Native;

    /** @internal

        At the moment, this is a way to get at the hb_font_t that backs this typeface.
        The typeface's hb_font_t has a size of 1 pt (i.e. 1 pt per em).
        This is only for internal use!
    */
    virtual Native getNativeDetails() const = 0;

private:
    //==============================================================================
    String name;
    String style;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Typeface)
};

} // namespace juce
