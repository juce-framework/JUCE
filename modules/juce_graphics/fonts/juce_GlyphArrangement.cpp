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

PositionedGlyph::PositionedGlyph() noexcept
    : character (0), glyph (0), x (0), y (0), w (0), whitespace (false)
{
}

PositionedGlyph::PositionedGlyph (const Font& font_, const juce_wchar character_, const int glyph_,
                                  const float x_, const float y_, const float w_, const bool whitespace_)
    : font (font_), character (character_), glyph (glyph_),
      x (x_), y (y_), w (w_), whitespace (whitespace_)
{
}

PositionedGlyph::PositionedGlyph (const PositionedGlyph& other)
    : font (other.font), character (other.character), glyph (other.glyph),
      x (other.x), y (other.y), w (other.w), whitespace (other.whitespace)
{
}

#if JUCE_COMPILER_SUPPORTS_MOVE_SEMANTICS
PositionedGlyph::PositionedGlyph (PositionedGlyph&& other) noexcept
    : font (static_cast<Font&&> (other.font)),
      character (other.character), glyph (other.glyph),
      x (other.x), y (other.y), w (other.w), whitespace (other.whitespace)
{
}

PositionedGlyph& PositionedGlyph::operator= (PositionedGlyph&& other) noexcept
{
    font = static_cast<Font&&> (other.font);
    character = other.character;
    glyph = other.glyph;
    x = other.x;
    y = other.y;
    w = other.w;
    whitespace = other.whitespace;
    return *this;
}
#endif

PositionedGlyph::~PositionedGlyph() {}

PositionedGlyph& PositionedGlyph::operator= (const PositionedGlyph& other)
{
    font = other.font;
    character = other.character;
    glyph = other.glyph;
    x = other.x;
    y = other.y;
    w = other.w;
    whitespace = other.whitespace;
    return *this;
}

static inline void drawGlyphWithFont (Graphics& g, int glyph, const Font& font, const AffineTransform& t)
{
    LowLevelGraphicsContext& context = g.getInternalContext();
    context.setFont (font);
    context.drawGlyph (glyph, t);
}

void PositionedGlyph::draw (Graphics& g) const
{
    if (! isWhitespace())
        drawGlyphWithFont (g, glyph, font, AffineTransform::translation (x, y));
}

void PositionedGlyph::draw (Graphics& g, const AffineTransform& transform) const
{
    if (! isWhitespace())
        drawGlyphWithFont (g, glyph, font, AffineTransform::translation (x, y).followedBy (transform));
}

void PositionedGlyph::createPath (Path& path) const
{
    if (! isWhitespace())
    {
        if (Typeface* const t = font.getTypeface())
        {
            Path p;
            t->getOutlineForGlyph (glyph, p);

            path.addPath (p, AffineTransform::scale (font.getHeight() * font.getHorizontalScale(), font.getHeight())
                                             .translated (x, y));
        }
    }
}

bool PositionedGlyph::hitTest (float px, float py) const
{
    if (getBounds().contains (px, py) && ! isWhitespace())
    {
        if (Typeface* const t = font.getTypeface())
        {
            Path p;
            t->getOutlineForGlyph (glyph, p);

            AffineTransform::translation (-x, -y)
                            .scaled (1.0f / (font.getHeight() * font.getHorizontalScale()), 1.0f / font.getHeight())
                            .transformPoint (px, py);

            return p.contains (px, py);
        }
    }

    return false;
}

void PositionedGlyph::moveBy (const float deltaX,
                              const float deltaY)
{
    x += deltaX;
    y += deltaY;
}


//==============================================================================
GlyphArrangement::GlyphArrangement()
{
    glyphs.ensureStorageAllocated (128);
}

GlyphArrangement::GlyphArrangement (const GlyphArrangement& other)
    : glyphs (other.glyphs)
{
}

GlyphArrangement& GlyphArrangement::operator= (const GlyphArrangement& other)
{
    glyphs = other.glyphs;
    return *this;
}

GlyphArrangement::~GlyphArrangement()
{
}

//==============================================================================
void GlyphArrangement::clear()
{
    glyphs.clear();
}

PositionedGlyph& GlyphArrangement::getGlyph (const int index) const noexcept
{
    return glyphs.getReference (index);
}

//==============================================================================
void GlyphArrangement::addGlyphArrangement (const GlyphArrangement& other)
{
    glyphs.addArray (other.glyphs);
}

void GlyphArrangement::addGlyph (const PositionedGlyph& glyph)
{
    glyphs.add (glyph);
}

void GlyphArrangement::removeRangeOfGlyphs (int startIndex, const int num)
{
    glyphs.removeRange (startIndex, num < 0 ? glyphs.size() : num);
}

//==============================================================================
void GlyphArrangement::addLineOfText (const Font& font,
                                      const String& text,
                                      const float xOffset,
                                      const float yOffset)
{
    addCurtailedLineOfText (font, text, xOffset, yOffset, 1.0e10f, false);
}

void GlyphArrangement::addCurtailedLineOfText (const Font& font,
                                               const String& text,
                                               const float xOffset,
                                               const float yOffset,
                                               const float maxWidthPixels,
                                               const bool useEllipsis)
{
    if (text.isNotEmpty())
    {
        Array<int> newGlyphs;
        Array<float> xOffsets;
        font.getGlyphPositions (text, newGlyphs, xOffsets);
        const int textLen = newGlyphs.size();
        glyphs.ensureStorageAllocated (glyphs.size() + textLen);

        String::CharPointerType t (text.getCharPointer());

        for (int i = 0; i < textLen; ++i)
        {
            const float nextX = xOffsets.getUnchecked (i + 1);

            if (nextX > maxWidthPixels + 1.0f)
            {
                // curtail the string if it's too wide..
                if (useEllipsis && textLen > 3 && glyphs.size() >= 3)
                    insertEllipsis (font, xOffset + maxWidthPixels, 0, glyphs.size());

                break;
            }

            const float thisX = xOffsets.getUnchecked (i);
            const bool isWhitespace = t.isWhitespace();

            glyphs.add (PositionedGlyph (font, t.getAndAdvance(),
                                         newGlyphs.getUnchecked(i),
                                         xOffset + thisX, yOffset,
                                         nextX - thisX, isWhitespace));
        }
    }
}

int GlyphArrangement::insertEllipsis (const Font& font, const float maxXPos,
                                      const int startIndex, int endIndex)
{
    int numDeleted = 0;

    if (glyphs.size() > 0)
    {
        Array<int> dotGlyphs;
        Array<float> dotXs;
        font.getGlyphPositions ("..", dotGlyphs, dotXs);

        const float dx = dotXs[1];
        float xOffset = 0.0f, yOffset = 0.0f;

        while (endIndex > startIndex)
        {
            const PositionedGlyph& pg = glyphs.getReference (--endIndex);
            xOffset = pg.x;
            yOffset = pg.y;

            glyphs.remove (endIndex);
            ++numDeleted;

            if (xOffset + dx * 3 <= maxXPos)
                break;
        }

        for (int i = 3; --i >= 0;)
        {
            glyphs.insert (endIndex++, PositionedGlyph (font, '.', dotGlyphs.getFirst(),
                                                        xOffset, yOffset, dx, false));
            --numDeleted;
            xOffset += dx;

            if (xOffset > maxXPos)
                break;
        }
    }

    return numDeleted;
}

void GlyphArrangement::addJustifiedText (const Font& font,
                                         const String& text,
                                         float x, float y,
                                         const float maxLineWidth,
                                         Justification horizontalLayout)
{
    int lineStartIndex = glyphs.size();
    addLineOfText (font, text, x, y);

    const float originalY = y;

    while (lineStartIndex < glyphs.size())
    {
        int i = lineStartIndex;

        if (glyphs.getReference(i).getCharacter() != '\n'
              && glyphs.getReference(i).getCharacter() != '\r')
            ++i;

        const float lineMaxX = glyphs.getReference (lineStartIndex).getLeft() + maxLineWidth;
        int lastWordBreakIndex = -1;

        while (i < glyphs.size())
        {
            const PositionedGlyph& pg = glyphs.getReference (i);
            const juce_wchar c = pg.getCharacter();

            if (c == '\r' || c == '\n')
            {
                ++i;

                if (c == '\r' && i < glyphs.size()
                     && glyphs.getReference(i).getCharacter() == '\n')
                    ++i;

                break;
            }

            if (pg.isWhitespace())
            {
                lastWordBreakIndex = i + 1;
            }
            else if (pg.getRight() - 0.0001f >= lineMaxX)
            {
                if (lastWordBreakIndex >= 0)
                    i = lastWordBreakIndex;

                break;
            }

            ++i;
        }

        const float currentLineStartX = glyphs.getReference (lineStartIndex).getLeft();
        float currentLineEndX = currentLineStartX;

        for (int j = i; --j >= lineStartIndex;)
        {
            if (! glyphs.getReference (j).isWhitespace())
            {
                currentLineEndX = glyphs.getReference (j).getRight();
                break;
            }
        }

        float deltaX = 0.0f;

        if (horizontalLayout.testFlags (Justification::horizontallyJustified))
            spreadOutLine (lineStartIndex, i - lineStartIndex, maxLineWidth);
        else if (horizontalLayout.testFlags (Justification::horizontallyCentred))
            deltaX = (maxLineWidth - (currentLineEndX - currentLineStartX)) * 0.5f;
        else if (horizontalLayout.testFlags (Justification::right))
            deltaX = maxLineWidth - (currentLineEndX - currentLineStartX);

        moveRangeOfGlyphs (lineStartIndex, i - lineStartIndex,
                           x + deltaX - currentLineStartX, y - originalY);

        lineStartIndex = i;

        y += font.getHeight();
    }
}

void GlyphArrangement::addFittedText (const Font& f,
                                      const String& text,
                                      const float x, const float y,
                                      const float width, const float height,
                                      Justification layout,
                                      int maximumLines,
                                      float minimumHorizontalScale)
{
    if (minimumHorizontalScale == 0.0f)
        minimumHorizontalScale = Font::getDefaultMinimumHorizontalScaleFactor();

    // doesn't make much sense if this is outside a sensible range of 0.5 to 1.0
    jassert (minimumHorizontalScale > 0 && minimumHorizontalScale <= 1.0f);

    if (text.containsAnyOf ("\r\n"))
    {
        addLinesWithLineBreaks (text, f, x, y, width, height, layout);
    }
    else
    {
        const int startIndex = glyphs.size();
        const String trimmed (text.trim());
        addLineOfText (f, trimmed, x, y);
        const int numGlyphs = glyphs.size() - startIndex;

        if (numGlyphs > 0)
        {
            const float lineWidth = glyphs.getReference (glyphs.size() - 1).getRight()
                                      - glyphs.getReference (startIndex).getLeft();

            if (lineWidth > 0)
            {
                if (lineWidth * minimumHorizontalScale < width)
                {
                    if (lineWidth > width)
                        stretchRangeOfGlyphs (startIndex, numGlyphs, width / lineWidth);

                    justifyGlyphs (startIndex, numGlyphs, x, y, width, height, layout);
                }
                else if (maximumLines <= 1)
                {
                    fitLineIntoSpace (startIndex, numGlyphs, x, y, width, height,
                                      f, layout, minimumHorizontalScale);
                }
                else
                {
                    splitLines (trimmed, f, startIndex, x, y, width, height,
                                maximumLines, lineWidth, layout, minimumHorizontalScale);
                }
            }
        }
    }
}

//==============================================================================
void GlyphArrangement::moveRangeOfGlyphs (int startIndex, int num, const float dx, const float dy)
{
    jassert (startIndex >= 0);

    if (dx != 0.0f || dy != 0.0f)
    {
        if (num < 0 || startIndex + num > glyphs.size())
            num = glyphs.size() - startIndex;

        while (--num >= 0)
            glyphs.getReference (startIndex++).moveBy (dx, dy);
    }
}

void GlyphArrangement::addLinesWithLineBreaks (const String& text, const Font& f,
                                               float x, float y, float width, float height, Justification layout)
{
    GlyphArrangement ga;
    ga.addJustifiedText (f, text, x, y, width, layout);

    const Rectangle<float> bb (ga.getBoundingBox (0, -1, false));

    float dy = y - bb.getY();

    if (layout.testFlags (Justification::verticallyCentred))   dy += (height - bb.getHeight()) * 0.5f;
    else if (layout.testFlags (Justification::bottom))         dy += (height - bb.getHeight());

    ga.moveRangeOfGlyphs (0, -1, 0.0f, dy);

    glyphs.addArray (ga.glyphs);
}

int GlyphArrangement::fitLineIntoSpace (int start, int numGlyphs, float x, float y, float w, float h, const Font& font,
                                        Justification justification, float minimumHorizontalScale)
{
    int numDeleted = 0;
    const float lineStartX = glyphs.getReference (start).getLeft();
    float lineWidth = glyphs.getReference (start + numGlyphs - 1).getRight() - lineStartX;

    if (lineWidth > w)
    {
        if (minimumHorizontalScale < 1.0f)
        {
            stretchRangeOfGlyphs (start, numGlyphs, jmax (minimumHorizontalScale, w / lineWidth));
            lineWidth = glyphs.getReference (start + numGlyphs - 1).getRight() - lineStartX - 0.5f;
        }

        if (lineWidth > w)
        {
            numDeleted = insertEllipsis (font, lineStartX + w, start, start + numGlyphs);
            numGlyphs -= numDeleted;
        }
    }

    justifyGlyphs (start, numGlyphs, x, y, w, h, justification);
    return numDeleted;
}

void GlyphArrangement::stretchRangeOfGlyphs (int startIndex, int num,
                                             const float horizontalScaleFactor)
{
    jassert (startIndex >= 0);

    if (num < 0 || startIndex + num > glyphs.size())
        num = glyphs.size() - startIndex;

    if (num > 0)
    {
        const float xAnchor = glyphs.getReference (startIndex).getLeft();

        while (--num >= 0)
        {
            PositionedGlyph& pg = glyphs.getReference (startIndex++);

            pg.x = xAnchor + (pg.x - xAnchor) * horizontalScaleFactor;
            pg.font.setHorizontalScale (pg.font.getHorizontalScale() * horizontalScaleFactor);
            pg.w *= horizontalScaleFactor;
        }
    }
}

Rectangle<float> GlyphArrangement::getBoundingBox (int startIndex, int num, const bool includeWhitespace) const
{
    jassert (startIndex >= 0);

    if (num < 0 || startIndex + num > glyphs.size())
        num = glyphs.size() - startIndex;

    Rectangle<float> result;

    while (--num >= 0)
    {
        const PositionedGlyph& pg = glyphs.getReference (startIndex++);

        if (includeWhitespace || ! pg.isWhitespace())
            result = result.getUnion (pg.getBounds());
    }

    return result;
}

void GlyphArrangement::justifyGlyphs (const int startIndex, const int num,
                                      const float x, const float y, const float width, const float height,
                                      Justification justification)
{
    jassert (num >= 0 && startIndex >= 0);

    if (glyphs.size() > 0 && num > 0)
    {
        const Rectangle<float> bb (getBoundingBox (startIndex, num, ! justification.testFlags (Justification::horizontallyJustified
                                                                                                | Justification::horizontallyCentred)));
        float deltaX = 0.0f, deltaY = 0.0f;

        if (justification.testFlags (Justification::horizontallyJustified))     deltaX = x - bb.getX();
        else if (justification.testFlags (Justification::horizontallyCentred))  deltaX = x + (width - bb.getWidth()) * 0.5f - bb.getX();
        else if (justification.testFlags (Justification::right))                deltaX = x + width - bb.getRight();
        else                                                                    deltaX = x - bb.getX();

        if (justification.testFlags (Justification::top))                       deltaY = y - bb.getY();
        else if (justification.testFlags (Justification::bottom))               deltaY = y + height - bb.getBottom();
        else                                                                    deltaY = y + (height - bb.getHeight()) * 0.5f - bb.getY();

        moveRangeOfGlyphs (startIndex, num, deltaX, deltaY);

        if (justification.testFlags (Justification::horizontallyJustified))
        {
            int lineStart = 0;
            float baseY = glyphs.getReference (startIndex).getBaselineY();

            int i;
            for (i = 0; i < num; ++i)
            {
                const float glyphY = glyphs.getReference (startIndex + i).getBaselineY();

                if (glyphY != baseY)
                {
                    spreadOutLine (startIndex + lineStart, i - lineStart, width);

                    lineStart = i;
                    baseY = glyphY;
                }
            }

            if (i > lineStart)
                spreadOutLine (startIndex + lineStart, i - lineStart, width);
        }
    }
}

void GlyphArrangement::spreadOutLine (const int start, const int num, const float targetWidth)
{
    if (start + num < glyphs.size()
         && glyphs.getReference (start + num - 1).getCharacter() != '\r'
         && glyphs.getReference (start + num - 1).getCharacter() != '\n')
    {
        int numSpaces = 0;
        int spacesAtEnd = 0;

        for (int i = 0; i < num; ++i)
        {
            if (glyphs.getReference (start + i).isWhitespace())
            {
                ++spacesAtEnd;
                ++numSpaces;
            }
            else
            {
                spacesAtEnd = 0;
            }
        }

        numSpaces -= spacesAtEnd;

        if (numSpaces > 0)
        {
            const float startX = glyphs.getReference (start).getLeft();
            const float endX = glyphs.getReference (start + num - 1 - spacesAtEnd).getRight();

            const float extraPaddingBetweenWords
                = (targetWidth - (endX - startX)) / (float) numSpaces;

            float deltaX = 0.0f;

            for (int i = 0; i < num; ++i)
            {
                glyphs.getReference (start + i).moveBy (deltaX, 0.0f);

                if (glyphs.getReference (start + i).isWhitespace())
                    deltaX += extraPaddingBetweenWords;
            }
        }
    }
}


void GlyphArrangement::splitLines (const String& text, Font font, int startIndex,
                                   float x, float y, float width, float height, int maximumLines,
                                   float lineWidth, Justification layout, float minimumHorizontalScale)
{
    const int length = text.length();
    const int originalStartIndex = startIndex;
    int numLines = 1;

    if (length <= 12 && ! text.containsAnyOf (" -\t\r\n"))
        maximumLines = 1;

    maximumLines = jmin (maximumLines, length);

    while (numLines < maximumLines)
    {
        ++numLines;

        const float newFontHeight = height / (float) numLines;

        if (newFontHeight < font.getHeight())
        {
            font.setHeight (jmax (8.0f, newFontHeight));

            removeRangeOfGlyphs (startIndex, -1);
            addLineOfText (font, text, x, y);

            lineWidth = glyphs.getReference (glyphs.size() - 1).getRight()
                            - glyphs.getReference (startIndex).getLeft();
        }

        // Try to estimate the point at which there are enough lines to fit the text,
        // allowing for unevenness in the lengths due to differently sized words.
        const float lineLengthUnevennessAllowance = 80.0f;

        if (numLines > (lineWidth + lineLengthUnevennessAllowance) / width || newFontHeight < 8.0f)
            break;
    }

    if (numLines < 1)
        numLines = 1;

    float lineY = y;
    float widthPerLine = lineWidth / numLines;

    for (int line = 0; line < numLines; ++line)
    {
        int i = startIndex;
        float lineStartX = glyphs.getReference (startIndex).getLeft();

        if (line == numLines - 1)
        {
            widthPerLine = width;
            i = glyphs.size();
        }
        else
        {
            while (i < glyphs.size())
            {
                lineWidth = (glyphs.getReference (i).getRight() - lineStartX);

                if (lineWidth > widthPerLine)
                {
                    // got to a point where the line's too long, so skip forward to find a
                    // good place to break it..
                    const int searchStartIndex = i;

                    while (i < glyphs.size())
                    {
                        if ((glyphs.getReference (i).getRight() - lineStartX) * minimumHorizontalScale < width)
                        {
                            if (glyphs.getReference (i).isWhitespace()
                                 || glyphs.getReference (i).getCharacter() == '-')
                            {
                                ++i;
                                break;
                            }
                        }
                        else
                        {
                            // can't find a suitable break, so try looking backwards..
                            i = searchStartIndex;

                            for (int back = 1; back < jmin (7, i - startIndex - 1); ++back)
                            {
                                if (glyphs.getReference (i - back).isWhitespace()
                                     || glyphs.getReference (i - back).getCharacter() == '-')
                                {
                                    i -= back - 1;
                                    break;
                                }
                            }

                            break;
                        }

                        ++i;
                    }

                    break;
                }

                ++i;
            }

            int wsStart = i;
            while (wsStart > 0 && glyphs.getReference (wsStart - 1).isWhitespace())
                --wsStart;

            int wsEnd = i;
            while (wsEnd < glyphs.size() && glyphs.getReference (wsEnd).isWhitespace())
                ++wsEnd;

            removeRangeOfGlyphs (wsStart, wsEnd - wsStart);
            i = jmax (wsStart, startIndex + 1);
        }

        i -= fitLineIntoSpace (startIndex, i - startIndex,
                               x, lineY, width, font.getHeight(), font,
                               layout.getOnlyHorizontalFlags() | Justification::verticallyCentred,
                               minimumHorizontalScale);

        startIndex = i;
        lineY += font.getHeight();

        if (startIndex >= glyphs.size())
            break;
    }

    justifyGlyphs (originalStartIndex, glyphs.size() - originalStartIndex,
                   x, y, width, height, layout.getFlags() & ~Justification::horizontallyJustified);
}

//==============================================================================
void GlyphArrangement::drawGlyphUnderline (const Graphics& g, const PositionedGlyph& pg,
                                           const int i, const AffineTransform& transform) const
{
    const float lineThickness = (pg.font.getDescent()) * 0.3f;

    float nextX = pg.x + pg.w;

    if (i < glyphs.size() - 1 && glyphs.getReference (i + 1).y == pg.y)
        nextX = glyphs.getReference (i + 1).x;

    Path p;
    p.addRectangle (pg.x, pg.y + lineThickness * 2.0f, nextX - pg.x, lineThickness);
    g.fillPath (p, transform);
}

void GlyphArrangement::draw (const Graphics& g) const
{
    draw (g, AffineTransform());
}

void GlyphArrangement::draw (const Graphics& g, const AffineTransform& transform) const
{
    LowLevelGraphicsContext& context = g.getInternalContext();
    Font lastFont (context.getFont());
    bool needToRestore = false;

    for (int i = 0; i < glyphs.size(); ++i)
    {
        const PositionedGlyph& pg = glyphs.getReference(i);

        if (pg.font.isUnderlined())
            drawGlyphUnderline (g, pg, i, transform);

        if (! pg.isWhitespace())
        {
            if (lastFont != pg.font)
            {
                lastFont = pg.font;

                if (! needToRestore)
                {
                    needToRestore = true;
                    context.saveState();
                }

                context.setFont (lastFont);
            }

            context.drawGlyph (pg.glyph, AffineTransform::translation (pg.x, pg.y).followedBy (transform));
        }
    }

    if (needToRestore)
        context.restoreState();
}

void GlyphArrangement::createPath (Path& path) const
{
    for (int i = 0; i < glyphs.size(); ++i)
        glyphs.getReference (i).createPath (path);
}

int GlyphArrangement::findGlyphIndexAt (const float x, const float y) const
{
    for (int i = 0; i < glyphs.size(); ++i)
        if (glyphs.getReference (i).hitTest (x, y))
            return i;

    return -1;
}
