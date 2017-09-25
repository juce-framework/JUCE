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
    A glyph from a particular font, with a particular size, style,
    typeface and position.

    You should rarely need to use this class directly - for most purposes, the
    GlyphArrangement class will do what you need for text layout.

    @see GlyphArrangement, Font
*/
class JUCE_API  PositionedGlyph
{
public:
    //==============================================================================
    PositionedGlyph() noexcept;
    PositionedGlyph (const Font& font, juce_wchar character, int glyphNumber,
                     float anchorX, float baselineY, float width, bool isWhitespace);

    PositionedGlyph (const PositionedGlyph&);
    PositionedGlyph& operator= (const PositionedGlyph&);

    /** Move constructor */
    PositionedGlyph (PositionedGlyph&&) noexcept;

    /** Move assignment operator */
    PositionedGlyph& operator= (PositionedGlyph&&) noexcept;

    ~PositionedGlyph();

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
    Rectangle<float> getBounds() const          { return Rectangle<float> (x, getTop(), w, font.getHeight()); }

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
    void draw (Graphics& g, const AffineTransform& transform) const;

    /** Returns the path for this glyph.
        @param path     the glyph's outline will be appended to this path
    */
    void createPath (Path& path) const;

    /** Checks to see if a point lies within this glyph. */
    bool hitTest (float x, float y) const;

private:
    //==============================================================================
    friend class GlyphArrangement;
    Font font;
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
*/
class JUCE_API  GlyphArrangement
{
public:
    //==============================================================================
    /** Creates an empty arrangement. */
    GlyphArrangement();

    /** Takes a copy of another arrangement. */
    GlyphArrangement (const GlyphArrangement&);

    /** Copies another arrangement onto this one.
        To add another arrangement without clearing this one, use addGlyphArrangement().
    */
    GlyphArrangement& operator= (const GlyphArrangement&);

    /** Destructor. */
    ~GlyphArrangement();

    //==============================================================================
    /** Returns the total number of glyphs in the arrangement. */
    int getNumGlyphs() const noexcept                           { return glyphs.size(); }

    /** Returns one of the glyphs from the arrangement.

        @param index    the glyph's index, from 0 to (getNumGlyphs() - 1). Be
                        careful not to pass an out-of-range index here, as it
                        doesn't do any bounds-checking.
    */
    PositionedGlyph& getGlyph (int index) const noexcept;

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
        lines will be placed below it, separated by a distance of font.getHeight().
    */
    void addJustifiedText (const Font& font,
                           const String& text,
                           float x, float y,
                           float maxLineWidth,
                           Justification horizontalLayout);

    /** Tries to fit some text within a given space.

        This does its best to make the given text readable within the specified rectangle,
        so it useful for labelling things.

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

        This uses cached bitmaps so is much faster than the draw (Graphics&, const AffineTransform&)
        method, which renders the glyphs as filled vectors.
    */
    void draw (const Graphics&) const;

    /** Draws this glyph arrangement to a graphics context.

        This renders the paths as filled vectors, so is far slower than the draw (Graphics&)
        method for non-transformed arrangements.
    */
    void draw (const Graphics&, const AffineTransform&) const;

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


private:
    //==============================================================================
    Array<PositionedGlyph> glyphs;

    int insertEllipsis (const Font&, float maxXPos, int startIndex, int endIndex);
    int fitLineIntoSpace (int start, int numGlyphs, float x, float y, float w, float h, const Font&,
                          Justification, float minimumHorizontalScale);
    void spreadOutLine (int start, int numGlyphs, float targetWidth);
    void splitLines (const String&, Font, int start, float x, float y, float w, float h, int maxLines,
                     float lineWidth, Justification, float minimumHorizontalScale);
    void addLinesWithLineBreaks (const String&, const Font&, float x, float y, float width, float height, Justification);
    void drawGlyphUnderline (const Graphics&, const PositionedGlyph&, int, const AffineTransform&) const;

    JUCE_LEAK_DETECTOR (GlyphArrangement)
};

} // namespace juce
