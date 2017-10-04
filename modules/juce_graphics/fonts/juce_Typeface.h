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
    A typeface represents a size-independent font.

    This base class is abstract, but calling createSystemTypefaceFor() will return
    a platform-specific subclass that can be used.

    The CustomTypeface subclass allow you to build your own typeface, and to
    load and save it in the Juce typeface format.

    Normally you should never need to deal directly with Typeface objects - the Font
    class does everything you typically need for rendering text.

    @see CustomTypeface, Font
*/
class JUCE_API  Typeface  : public ReferenceCountedObject
{
public:
    //==============================================================================
    /** A handy typedef for a pointer to a typeface. */
    typedef ReferenceCountedObjectPtr<Typeface> Ptr;

    //==============================================================================
    /** Returns the font family of the typeface.
        @see Font::getTypefaceName
    */
    const String& getName() const noexcept      { return name; }

    //==============================================================================
    /** Returns the font style of the typeface.
        @see Font::getTypefaceStyle
    */
    const String& getStyle() const noexcept     { return style; }

    //==============================================================================
    /** Creates a new system typeface. */
    static Ptr createSystemTypefaceFor (const Font& font);

    /** Attempts to create a font from some raw font file data (e.g. a TTF or OTF file image).
        The system will take its own internal copy of the data, so you can free the block once
        this method has returned.
    */
    static Ptr createSystemTypefaceFor (const void* fontFileData, size_t fontFileDataSize);

    //==============================================================================
    /** Destructor. */
    virtual ~Typeface();

    /** Returns true if this typeface can be used to render the specified font.
        When called, the font will already have been checked to make sure that its name and
        style flags match the typeface.
    */
    virtual bool isSuitableForFont (const Font&) const          { return true; }

    /** Returns the ascent of the font, as a proportion of its height.
        The height is considered to always be normalised as 1.0, so this will be a
        value less that 1.0, indicating the proportion of the font that lies above
        its baseline.
    */
    virtual float getAscent() const = 0;

    /** Returns the descent of the font, as a proportion of its height.
        The height is considered to always be normalised as 1.0, so this will be a
        value less that 1.0, indicating the proportion of the font that lies below
        its baseline.
    */
    virtual float getDescent() const = 0;

    /** Returns the value by which you should multiply a juce font-height value to
        convert it to the equivalent point-size.
    */
    virtual float getHeightToPointsFactor() const = 0;

    /** Measures the width of a line of text.
        The distance returned is based on the font having an normalised height of 1.0.
        You should never need to call this directly! Use Font::getStringWidth() instead!
    */
    virtual float getStringWidth (const String& text) = 0;

    /** Converts a line of text into its glyph numbers and their positions.
        The distances returned are based on the font having an normalised height of 1.0.
        You should never need to call this directly! Use Font::getGlyphPositions() instead!
    */
    virtual void getGlyphPositions (const String& text, Array<int>& glyphs, Array<float>& xOffsets) = 0;

    /** Returns the outline for a glyph.
        The path returned will be normalised to a font height of 1.0.
    */
    virtual bool getOutlineForGlyph (int glyphNumber, Path& path) = 0;

    /** Returns a new EdgeTable that contains the path for the givem glyph, with the specified transform applied. */
    virtual EdgeTable* getEdgeTableForGlyph (int glyphNumber, const AffineTransform& transform, float fontHeight);

    /** Returns true if the typeface uses hinting. */
    virtual bool isHinted() const                           { return false; }

    //==============================================================================
    /** Changes the number of fonts that are cached in memory. */
    static void setTypefaceCacheSize (int numFontsToCache);

    /** Clears any fonts that are currently cached in memory. */
    static void clearTypefaceCache();

    /** On some platforms, this allows a specific path to be scanned.
        Currently only available when using FreeType.
    */
    static void scanFolderForFonts (const File& folder);

    /** Makes an attempt at performing a good overall distortion that will scale a font of
        the given size to align vertically with the pixel grid. The path should be an unscaled
        (i.e. normalised to height of 1.0) path for a glyph.
    */
    void applyVerticalHintingTransform (float fontHeight, Path& path);

protected:
    //==============================================================================
    String name, style;

    Typeface (const String& name, const String& style) noexcept;

    static Ptr getFallbackTypeface();

private:
    struct HintingParams;
    friend struct ContainerDeletePolicy<HintingParams>;
    ScopedPointer<HintingParams> hintingParams;
    CriticalSection hintingLock;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Typeface)
};

} // namespace juce
