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
    A glyph from a particular font, with a particular size, style,
    typeface and position.

    You should rarely need to use this class directly - for most purposes, the
    GlyphArrangement class will do what you need for text layout.

    @see GlyphArrangement, Font

    @tags{Graphics}
*/
class JUCE_API  PositionedGlyph  final
{
public:
    //==============================================================================
    PositionedGlyph() noexcept;

    PositionedGlyph (const Font& font, juce_wchar character, int glyphNumber,
                     float anchorX, float baselineY, float width, bool isWhitespace);

    /** Returns the character the glyph represents. */
    juce_wchar getCharacter() const noexcept    { return character; }
    /** Checks whether the glyph is actually empty. */
    bool isWhitespace() const noexcept          { return whitespace; }

    /** Returns the position of the glyph's left-hand edge. */
    float getLeft() const noexcept              { return x; }
    /** Returns the position of the glyph's right-hand edge. */
    float getRight() const noexcept             { return x + w; }
    /** Returns the y position of the glyph's baseline. */
    float getBaselineY() const noexcept         { return y; }
    /** Returns the y position of the top of the glyph. */
    float getTop() const                        { return y - font.getAscent(); }
    /** Returns the y position of the bottom of the glyph. */
    float getBottom() const                     { return y + font.getDescent(); }
    /** Returns the bounds of the glyph. */
    Rectangle<float> getBounds() const          { return { x, getTop(), w, font.getHeight() }; }

    //==============================================================================
    /** Shifts the glyph's position by a relative amount. */
    void moveBy (float deltaX, float deltaY);

    //==============================================================================
    /** Draws the glyph into a graphics context.
        (Note that this may change the context's currently selected font).
    */
    void draw (Graphics& g) const;

    /** Draws the glyph into a graphics context, with an extra transform applied to it.
        (Note that this may change the context's currently selected font).
    */
    void draw (Graphics& g, AffineTransform transform) const;

    /** Returns the path for this glyph.
        @param path     the glyph's outline will be appended to this path
    */
    void createPath (Path& path) const;

    /** Checks to see if a point lies within this glyph. */
    bool hitTest (float x, float y) const;

private:
    //==============================================================================
    friend class GlyphArrangement;
    Font font { FontOptions{} };
    juce_wchar character;
    int glyph;
    float x, y, w;
    bool whitespace;

    JUCE_LEAK_DETECTOR (PositionedGlyph)
};


//==============================================================================
/**
    A set of glyphs, each with a position.

    You can create a GlyphArrangement, text to it and then draw it onto a
    graphics context. It's used internally by the text methods in the
    Graphics class, but can be used directly if more control is needed.

    @see Font, PositionedGlyph

    @tags{Graphics}
*/
class JUCE_API  GlyphArrangement  final
{
public:
    //==============================================================================
    /** Creates an empty arrangement. */
    GlyphArrangement();

    GlyphArrangement (const GlyphArrangement&) = default;
    GlyphArrangement& operator= (const GlyphArrangement&) = default;
    GlyphArrangement (GlyphArrangement&&) = default;
    GlyphArrangement& operator= (GlyphArrangement&&) = default;

    /** Destructor. */
    ~GlyphArrangement() = default;

    //==============================================================================
    /** Returns the total number of glyphs in the arrangement. */
    int getNumGlyphs() const noexcept                           { return glyphs.size(); }

    /** Returns one of the glyphs from the arrangement.

        @param index    the glyph's index, from 0 to (getNumGlyphs() - 1). Be
                        careful not to pass an out-of-range index here, as it
                        doesn't do any bounds-checking.
    */
    PositionedGlyph& getGlyph (int index) noexcept;

    const PositionedGlyph* begin() const                        { return glyphs.begin(); }
    const PositionedGlyph* end() const                          { return glyphs.end(); }

    //==============================================================================
    /** Clears all text from the arrangement and resets it. */
    void clear();

    /** Appends a line of text to the arrangement.

        This will add the text as a single line, where x is the left-hand edge of the
        first character, and y is the position for the text's baseline.

        If the text contains new-lines or carriage-returns, this will ignore them - use
        addJustifiedText() to add multi-line arrangements.
    */
    void addLineOfText (const Font& font,
                        const String& text,
                        float x, float y);

    /** Adds a line of text, truncating it if it's wider than a specified size.

        This is the same as addLineOfText(), but if the line's width exceeds the value
        specified in maxWidthPixels, it will be truncated using either ellipsis (i.e. dots: "..."),
        if useEllipsis is true, or if this is false, it will just drop any subsequent characters.
    */
    void addCurtailedLineOfText (const Font& font,
                                 const String& text,
                                 float x, float y,
                                 float maxWidthPixels,
                                 bool useEllipsis);

    /** Adds some multi-line text, breaking lines at word-boundaries if they are too wide.

        This will add text to the arrangement, breaking it into new lines either where there
        is a new-line or carriage-return character in the text, or where a line's width
        exceeds the value set in maxLineWidth.

        Each line that is added will be laid out using the flags set in horizontalLayout, so
        the lines can be left- or right-justified, or centred horizontally in the space
        between x and (x + maxLineWidth).

        The y coordinate is the position of the baseline of the first line of text - subsequent
        lines will be placed below it, separated by a distance of font.getHeight() + leading.
    */
    void addJustifiedText (const Font& font,
                           const String& text,
                           float x, float y,
                           float maxLineWidth,
                           Justification horizontalLayout,
                           float leading = 0.0f);

    /** Tries to fit some text within a given space.

        This does its best to make the given text readable within the specified rectangle,
        so it's useful for labelling things.

        If the text is too big, it'll be squashed horizontally or broken over multiple lines
        if the maximumLinesToUse value allows this. If the text just won't fit into the space,
        it'll cram as much as possible in there, and put some ellipsis at the end to show that
        it's been truncated.

        A Justification parameter lets you specify how the text is laid out within the rectangle,
        both horizontally and vertically.

        The minimumHorizontalScale parameter specifies how much the text can be squashed horizontally
        to try to squeeze it into the space. If you don't want any horizontal scaling to occur, you
        can set this value to 1.0f. Pass 0 if you want it to use the default value.

        @see Graphics::drawFittedText
    */
    void addFittedText (const Font& font,
                        const String& text,
                        float x, float y, float width, float height,
                        Justification layout,
                        int maximumLinesToUse,
                        float minimumHorizontalScale = 0.0f);

    /** Appends another glyph arrangement to this one. */
    void addGlyphArrangement (const GlyphArrangement&);

    /** Appends a custom glyph to the arrangement. */
    void addGlyph (const PositionedGlyph&);

    //==============================================================================
    /** Draws this glyph arrangement to a graphics context.

        This uses cached bitmaps so is much faster than the draw (Graphics&, AffineTransform)
        method, which renders the glyphs as filled vectors.
    */
    void draw (const Graphics&) const;

    /** Draws this glyph arrangement to a graphics context.

        This renders the paths as filled vectors, so is far slower than the draw (Graphics&)
        method for non-transformed arrangements.
    */
    void draw (const Graphics&, AffineTransform) const;

    /** Converts the set of glyphs into a path.
        @param path     the glyphs' outlines will be appended to this path
    */
    void createPath (Path& path) const;

    /** Looks for a glyph that contains the given coordinate.
        @returns the index of the glyph, or -1 if none were found.
    */
    int findGlyphIndexAt (float x, float y) const;

    //==============================================================================
    /** Finds the smallest rectangle that will enclose a subset of the glyphs.


        @param startIndex               the first glyph to test
        @param numGlyphs                the number of glyphs to include; if this is < 0, all glyphs after
                                        startIndex will be included
        @param includeWhitespace        if true, the extent of any whitespace characters will also
                                        be taken into account
    */
    Rectangle<float> getBoundingBox (int startIndex, int numGlyphs, bool includeWhitespace) const;

    /** Shifts a set of glyphs by a given amount.

        @param startIndex   the first glyph to transform
        @param numGlyphs    the number of glyphs to move; if this is < 0, all glyphs after
                            startIndex will be used
        @param deltaX       the amount to add to their x-positions
        @param deltaY       the amount to add to their y-positions
    */
    void moveRangeOfGlyphs (int startIndex, int numGlyphs,
                            float deltaX, float deltaY);

    /** Removes a set of glyphs from the arrangement.

        @param startIndex   the first glyph to remove
        @param numGlyphs    the number of glyphs to remove; if this is < 0, all glyphs after
                            startIndex will be deleted
    */
    void removeRangeOfGlyphs (int startIndex, int numGlyphs);

    /** Expands or compresses a set of glyphs horizontally.

        @param startIndex               the first glyph to transform
        @param numGlyphs                the number of glyphs to stretch; if this is < 0, all glyphs after
                                        startIndex will be used
        @param horizontalScaleFactor    how much to scale their horizontal width by
    */
    void stretchRangeOfGlyphs (int startIndex, int numGlyphs,
                               float horizontalScaleFactor);

    /** Justifies a set of glyphs within a given space.

        This moves the glyphs as a block so that the whole thing is located within the
        given rectangle with the specified layout.

        If the Justification::horizontallyJustified flag is specified, each line will
        be stretched out to fill the specified width.
    */
    void justifyGlyphs (int startIndex, int numGlyphs,
                        float x, float y, float width, float height,
                        Justification justification);

    /** This convenience function adds text to a GlyphArrangement using the specified font
        and returns the bounding box of the text after shaping.

        The returned bounding box is positioned with its origin at the left end of the text's
        baseline.
    */
    static Rectangle<float> getStringBounds (const Font& font, StringRef text)
    {
        GlyphArrangement arrangement;
        arrangement.addLineOfText (font, text, 0.0f, 0.0f);
        return arrangement.getBoundingBox (0, arrangement.getNumGlyphs(), true);
    }

    /** This convenience function adds text to a GlyphArrangement using the specified font
        and returns the width of the bounding box of the text after shaping.
    */
    static float getStringWidth (const Font& font, StringRef text)
    {
        return getStringBounds (font, text).getWidth();
    }

    /** This convenience function adds text to a GlyphArrangement using the specified font
        and returns the width of the bounding box of the text after shaping, rounded up to the
        next integer.
    */
    static int getStringWidthInt (const Font& font, StringRef text)
    {
        return (int) std::ceil (getStringWidth (font, text));
    }

private:
    //==============================================================================
    Array<PositionedGlyph> glyphs;

    void spreadOutLine (int start, int numGlyphs, float targetWidth);
    void drawGlyphUnderline (const Graphics&, int, AffineTransform) const;

    JUCE_LEAK_DETECTOR (GlyphArrangement)
};

} // namespace juce
