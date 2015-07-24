/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

#ifndef JUCE_TEXTLAYOUT_H_INCLUDED
#define JUCE_TEXTLAYOUT_H_INCLUDED


//==============================================================================
/**
    A Pre-formatted piece of text, which may contain multiple fonts and colours.

    A TextLayout is created from an AttributedString, and once created can be
    quickly drawn into a Graphics context.

    @see AttributedString
*/
class JUCE_API  TextLayout
{
public:
    /** Creates an empty layout.
        Having created a TextLayout, you can populate it using createLayout() or
        createLayoutWithBalancedLineLengths().
    */
    TextLayout();
    TextLayout (const TextLayout&);
    TextLayout& operator= (const TextLayout&);
   #if JUCE_COMPILER_SUPPORTS_MOVE_SEMANTICS
    TextLayout (TextLayout&&) noexcept;
    TextLayout& operator= (TextLayout&&) noexcept;
   #endif

    /** Destructor. */
    ~TextLayout();

    //==============================================================================
    /** Creates a layout from the given attributed string.
        This will replace any data that is currently stored in the layout.
    */
    void createLayout (const AttributedString&, float maxWidth);

    /** Creates a layout from the given attributed string, given some size constraints.
        This will replace any data that is currently stored in the layout.
    */
    void createLayout (const AttributedString&, float maxWidth, float maxHeight);

    /** Creates a layout, attempting to choose a width which results in lines
        of a similar length.

        This will be slower than the normal createLayout method, but produces a
        tidier result.
    */
    void createLayoutWithBalancedLineLengths (const AttributedString&, float maxWidth);

    /** Creates a layout, attempting to choose a width which results in lines
        of a similar length.

        This will be slower than the normal createLayout method, but produces a
        tidier result.
    */
    void createLayoutWithBalancedLineLengths (const AttributedString&, float maxWidth, float maxHeight);

    /** Draws the layout within the specified area.
        The position of the text within the rectangle is controlled by the justification
        flags set in the original AttributedString that was used to create this layout.
    */
    void draw (Graphics&, const Rectangle<float>& area) const;

    //==============================================================================
    /** A positioned glyph. */
    class JUCE_API  Glyph
    {
    public:
        Glyph (int glyphCode, Point<float> anchor, float width) noexcept;
        Glyph (const Glyph&) noexcept;
        Glyph& operator= (const Glyph&) noexcept;
        ~Glyph() noexcept;

        /** The code number of this glyph. */
        int glyphCode;

        /** The glyph's anchor point - this is relative to the line's origin.
            @see TextLayout::Line::lineOrigin
        */
        Point<float> anchor;

        float width;

    private:
        JUCE_LEAK_DETECTOR (Glyph)
    };

    //==============================================================================
    /** A sequence of glyphs with a common font and colour. */
    class JUCE_API  Run
    {
    public:
        Run() noexcept;
        Run (const Run&);
        Run (Range<int> stringRange, int numGlyphsToPreallocate);
        ~Run() noexcept;

        Font font;              /**< The run's font. */
        Colour colour;          /**< The run's colour. */
        Array<Glyph> glyphs;    /**< The glyphs in this run. */
        Range<int> stringRange; /**< The character range that this run represents in the
                                     original string that was used to create it. */
    private:
        Run& operator= (const Run&);
        JUCE_LEAK_DETECTOR (Run)
    };

    //==============================================================================
    /** A line containing a sequence of glyph-runs. */
    class JUCE_API  Line
    {
    public:
        Line() noexcept;
        Line (const Line&);
        Line (Range<int> stringRange, Point<float> lineOrigin,
              float ascent, float descent, float leading, int numRunsToPreallocate);
        ~Line() noexcept;

        /** Returns the X position range which contains all the glyphs in this line. */
        Range<float> getLineBoundsX() const noexcept;

        /** Returns the Y position range which contains all the glyphs in this line. */
        Range<float> getLineBoundsY() const noexcept;

        /** Returns the smallest rectangle which contains all the glyphs in this line. */
        Rectangle<float> getLineBounds() const noexcept;

        OwnedArray<Run> runs;           /**< The glyph-runs in this line. */
        Range<int> stringRange;         /**< The character range that this line represents in the
                                             original string that was used to create it. */
        Point<float> lineOrigin;        /**< The line's baseline origin. */
        float ascent, descent, leading;

    private:
        Line& operator= (const Line&);
        JUCE_LEAK_DETECTOR (Line)
    };

    //==============================================================================
    /** Returns the maximum width of the content. */
    float getWidth() const noexcept     { return width; }

    /** Returns the maximum height of the content. */
    float getHeight() const noexcept    { return height; }

    /** Returns the number of lines in the layout. */
    int getNumLines() const noexcept    { return lines.size(); }

    /** Returns one of the lines. */
    Line& getLine (int index) const;

    /** Adds a line to the layout. The layout will take ownership of this line object
        and will delete it when it is no longer needed. */
    void addLine (Line*);

    /** Pre-allocates space for the specified number of lines. */
    void ensureStorageAllocated (int numLinesNeeded);

private:
    OwnedArray<Line> lines;
    float width, height;
    Justification justification;

    void createStandardLayout (const AttributedString&);
    bool createNativeLayout (const AttributedString&);
    void recalculateSize (const AttributedString&);

    JUCE_LEAK_DETECTOR (TextLayout)
};

#endif   // JUCE_TEXTLAYOUT_H_INCLUDED
