/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#ifndef __JUCE_GLYPHARRANGEMENT_JUCEHEADER__
#define __JUCE_GLYPHARRANGEMENT_JUCEHEADER__

#include "juce_Font.h"
#include "../contexts/juce_Graphics.h"


//==============================================================================
/**
    An glyph from a particular font, with a particular size, style,
    typeface and position.

    @see GlyphArrangement, Font
*/
class JUCE_API  PositionedGlyph
{
public:
    //==============================================================================
    /** Returns the character the glyph represents. */
    juce_wchar getCharacter() const throw()     { return glyphInfo->getCharacter(); }
    /** Checks whether the glyph is actually empty. */
    bool isWhitespace() const throw()           { return CharacterFunctions::isWhitespace (glyphInfo->getCharacter()); }

    /** Returns the position of the glyph's left-hand edge. */
    float getLeft() const throw()               { return x; }
    /** Returns the position of the glyph's right-hand edge. */
    float getRight() const throw()              { return x + w; }
    /** Returns the y position of the glyph's baseline. */
    float getBaselineY() const throw()          { return y; }
    /** Returns the y position of the top of the glyph. */
    float getTop() const throw()                { return y - fontAscent; }
    /** Returns the y position of the bottom of the glyph. */
    float getBottom() const throw()             { return y + fontHeight - fontAscent; }

    //==============================================================================
    /** Shifts the glyph's position by a relative amount. */
    void moveBy (const float deltaX,
                 const float deltaY) throw();

    //==============================================================================
    /** Draws the glyph into a graphics context. */
    void draw (const Graphics& g) const throw();

    /** Draws the glyph into a graphics context, with an extra transform applied to it. */
    void draw (const Graphics& g, const AffineTransform& transform) const throw();

    /** Returns the path for this glyph.

        @param path     the glyph's outline will be appended to this path
    */
    void createPath (Path& path) const throw();

    /** Checks to see if a point lies within this glyph. */
    bool hitTest (float x, float y) const throw();

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    //==============================================================================
    friend class GlyphArrangement;
    float x, y, w;
    float fontHeight, fontAscent, fontHorizontalScale;
    bool isUnderlined;
    const TypefaceGlyphInfo* glyphInfo;

    PositionedGlyph() throw();
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
    GlyphArrangement() throw();

    /** Takes a copy of another arrangement. */
    GlyphArrangement (const GlyphArrangement& other) throw();

    /** Copies another arrangement onto this one.

        To add another arrangement without clearing this one, use addGlyphArrangement().
    */
    const GlyphArrangement& operator= (const GlyphArrangement& other) throw();

    /** Destructor. */
    ~GlyphArrangement() throw();

    //==============================================================================
    /** Returns the total number of glyphs in the arrangement. */
    int getNumGlyphs() const throw()                            { return numGlyphs; }

    /** Returns one of the glyphs from the arrangement.

        @param index    the glyph's index, from 0 to (getNumGlyphs() - 1). Be
                        careful not to pass an out-of-range index here, as it
                        doesn't do any bounds-checking.
    */
    PositionedGlyph& getGlyph (const int index) const throw();

    //==============================================================================
    /** Clears all text from the arrangement and resets it.
    */
    void clear() throw();

    /** Appends a line of text to the arrangement.

        This will add the text as a single line, where x is the left-hand edge of the
        first character, and y is the position for the text's baseline.

        If the text contains new-lines or carriage-returns, this will ignore them - use
        addJustifiedText() to add multi-line arrangements.
    */
    void addLineOfText (const Font& font,
                        const String& text,
                        const float x,
                        const float y) throw();

    /** Adds a line of text, truncating it if it's wider than a specified size.

        This is the same as addLineOfText(), but if the line's width exceeds the value
        specified in maxWidthPixels, it will be truncated using either ellipsis (i.e. dots: "..."),
        if useEllipsis is true, or if this is false, it will just drop any subsequent characters.
    */
    void addCurtailedLineOfText (const Font& font,
                                 const String& text,
                                 float x,
                                 const float y,
                                 const float maxWidthPixels,
                                 const bool useEllipsis) throw();

    /** Adds some multi-line text, breaking lines at word-boundaries if they are too wide.

        This will add text to the arrangement, breaking it into new lines either where there
        is a new-line or carriage-return character in the text, or where a line's width
        exceeds the value set in maxLineWidth.

        Each line that is added will be laid out using the flags set in horizontalLayout, so
        the lines can be left- or right-justified, or centred horizontally in the space
        between x and (x + maxLineWidth).

        The y co-ordinate is the position of the baseline of the first line of text - subsequent
        lines will be placed below it, separated by a distance of font.getHeight().
    */
    void addJustifiedText (const Font& font,
                           const String& text,
                           float x, float y,
                           const float maxLineWidth,
                           const Justification& horizontalLayout) throw();

    /** Tries to fit some text withing a given space.

        This does its best to make the given text readable within the specified rectangle,
        so it useful for labelling things.

        If the text is too big, it'll be squashed horizontally or broken over multiple lines
        if the maximumLinesToUse value allows this. If the text just won't fit into the space,
        it'll cram as much as possible in there, and put some ellipsis at the end to show that
        it's been truncated.

        A Justification parameter lets you specify how the text is laid out within the rectangle,
        both horizontally and vertically.

        @see Graphics::drawFittedText
    */
    void addFittedText (const Font& font,
                        const String& text,
                        float x, float y,
                        float width, float height,
                        const Justification& layout,
                        int maximumLinesToUse,
                        const float minimumHorizontalScale = 0.7f) throw();


    /** Appends another glyph arrangement to this one. */
    void addGlyphArrangement (const GlyphArrangement& other) throw();

    //==============================================================================
    /** Draws this glyph arrangement to a graphics context.

        This uses cached bitmaps so is much faster than the draw (Graphics&, const AffineTransform&)
        method, which renders the glyphs as filled vectors.
    */
    void draw (const Graphics& g) const throw();

    /** Draws this glyph arrangement to a graphics context.

        This renders the paths as filled vectors, so is far slower than the draw (Graphics&)
        method for non-transformed arrangements.
    */
    void draw (const Graphics& g, const AffineTransform& transform) const throw();

    /** Converts the set of glyphs into a path.

        @param path     the glyphs' outlines will be appended to this path
    */
    void createPath (Path& path) const throw();

    /** Looks for a glyph that contains the given co-ordinate.

        @returns the index of the glyph, or -1 if none were found.
    */
    int findGlyphIndexAt (float x, float y) const throw();

    //==============================================================================
    /** Finds the smallest rectangle that will enclose a subset of the glyphs.


        @param startIndex               the first glyph to test
        @param numGlyphs                the number of glyphs to include; if this is < 0, all glyphs after
                                        startIndex will be included
        @param left                     on return, the leftmost co-ordinate of the rectangle
        @param top                      on return, the top co-ordinate of the rectangle
        @param right                    on return, the rightmost co-ordinate of the rectangle
        @param bottom                   on return, the bottom co-ordinate of the rectangle
        @param includeWhitespace        if true, the extent of any whitespace characters will also
                                        be taken into account
    */
    void getBoundingBox (int startIndex,
                         int numGlyphs,
                         float& left,
                         float& top,
                         float& right,
                         float& bottom,
                         const bool includeWhitespace) const throw();

    /** Shifts a set of glyphs by a given amount.

        @param startIndex   the first glyph to transform
        @param numGlyphs    the number of glyphs to move; if this is < 0, all glyphs after
                            startIndex will be used
        @param deltaX       the amount to add to their x-positions
        @param deltaY       the amount to add to their y-positions
    */
    void moveRangeOfGlyphs (int startIndex, int numGlyphs,
                            const float deltaX,
                            const float deltaY) throw();

    /** Removes a set of glyphs from the arrangement.

        @param startIndex   the first glyph to remove
        @param numGlyphs    the number of glyphs to remove; if this is < 0, all glyphs after
                            startIndex will be deleted
    */
    void removeRangeOfGlyphs (int startIndex, int numGlyphs) throw();

    /** Expands or compresses a set of glyphs horizontally.

        @param startIndex               the first glyph to transform
        @param numGlyphs                the number of glyphs to stretch; if this is < 0, all glyphs after
                                        startIndex will be used
        @param horizontalScaleFactor    how much to scale their horizontal width by
    */
    void stretchRangeOfGlyphs (int startIndex, int numGlyphs,
                               const float horizontalScaleFactor) throw();

    /** Justifies a set of glyphs within a given space.

        This moves the glyphs as a block so that the whole thing is located within the
        given rectangle with the specified layout.

        If the Justification::horizontallyJustified flag is specified, each line will
        be stretched out to fill the specified width.
    */
    void justifyGlyphs (const int startIndex, const int numGlyphs,
                        const float x,
                        const float y,
                        const float width,
                        const float height,
                        const Justification& justification) throw();


    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    int numGlyphs, numAllocated;
    PositionedGlyph* glyphs;

    void ensureNumGlyphsAllocated (int minGlyphs) throw();
    void removeLast() throw();
    void appendEllipsis (const Font& font, const float maxXPixels) throw();

    void incGlyphRefCount (const int index) const throw();
    void decGlyphRefCount (const int index) const throw();

    void spreadOutLine (const int start, const int numGlyphs, const float targetWidth) throw();
};


#endif   // __JUCE_GLYPHARRANGEMENT_JUCEHEADER__
