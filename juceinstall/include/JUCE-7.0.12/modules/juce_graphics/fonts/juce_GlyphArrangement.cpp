/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
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

static constexpr bool isNonBreakingSpace (const juce_wchar c)
{
    return c == 0x00a0
        || c == 0x2007
        || c == 0x202f
        || c == 0x2060;
}

PositionedGlyph::PositionedGlyph() noexcept
    : character (0), glyph (0), x (0), y (0), w (0), whitespace (false)
{
}

PositionedGlyph::PositionedGlyph (const Font& font_, juce_wchar character_, int glyphNumber,
                                  float anchorX, float baselineY, float width, bool whitespace_)
    : font (font_), character (character_), glyph (glyphNumber),
      x (anchorX), y (baselineY), w (width), whitespace (whitespace_)
{
}

static void drawGlyphWithFont (Graphics& g, int glyph, const Font& font, AffineTransform t)
{
    auto& context = g.getInternalContext();
    context.setFont (font);
    context.drawGlyph (glyph, t);
}

void PositionedGlyph::draw (Graphics& g) const
{
    if (! isWhitespace())
        drawGlyphWithFont (g, glyph, font, AffineTransform::translation (x, y));
}

void PositionedGlyph::draw (Graphics& g, AffineTransform transform) const
{
    if (! isWhitespace())
        drawGlyphWithFont (g, glyph, font, AffineTransform::translation (x, y).followedBy (transform));
}

void PositionedGlyph::createPath (Path& path) const
{
    if (! isWhitespace())
    {
        if (auto t = font.getTypefacePtr())
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
        if (auto t = font.getTypefacePtr())
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

void PositionedGlyph::moveBy (float deltaX, float deltaY)
{
    x += deltaX;
    y += deltaY;
}


//==============================================================================
GlyphArrangement::GlyphArrangement()
{
    glyphs.ensureStorageAllocated (128);
}

//==============================================================================
void GlyphArrangement::clear()
{
    glyphs.clear();
}

PositionedGlyph& GlyphArrangement::getGlyph (int index) noexcept
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

void GlyphArrangement::removeRangeOfGlyphs (int startIndex, int num)
{
    glyphs.removeRange (startIndex, num < 0 ? glyphs.size() : num);
}

//==============================================================================
void GlyphArrangement::addLineOfText (const Font& font, const String& text, float xOffset, float yOffset)
{
    addCurtailedLineOfText (font, text, xOffset, yOffset, 1.0e10f, false);
}

void GlyphArrangement::addCurtailedLineOfText (const Font& font, const String& text,
                                               float xOffset, float yOffset,
                                               float maxWidthPixels, bool useEllipsis)
{
    if (text.isNotEmpty())
    {
        Array<int> newGlyphs;
        Array<float> xOffsets;
        font.getGlyphPositions (text, newGlyphs, xOffsets);
        auto textLen = newGlyphs.size();
        glyphs.ensureStorageAllocated (glyphs.size() + textLen);

        auto t = text.getCharPointer();

        for (int i = 0; i < textLen; ++i)
        {
            auto nextX = xOffsets.getUnchecked (i + 1);

            if (nextX > maxWidthPixels + 1.0f)
            {
                // curtail the string if it's too wide..
                if (useEllipsis && textLen > 3 && glyphs.size() >= 3)
                    insertEllipsis (font, xOffset + maxWidthPixels, 0, glyphs.size());

                break;
            }

            auto thisX = xOffsets.getUnchecked (i);
            auto isWhitespace = isNonBreakingSpace (*t) || t.isWhitespace();

            glyphs.add (PositionedGlyph (font, t.getAndAdvance(),
                                         newGlyphs.getUnchecked (i),
                                         xOffset + thisX, yOffset,
                                         nextX - thisX, isWhitespace));
        }
    }
}

int GlyphArrangement::insertEllipsis (const Font& font, float maxXPos, int startIndex, int endIndex)
{
    int numDeleted = 0;

    if (! glyphs.isEmpty())
    {
        Array<int> dotGlyphs;
        Array<float> dotXs;
        font.getGlyphPositions ("..", dotGlyphs, dotXs);

        auto dx = dotXs[1];
        float xOffset = 0.0f, yOffset = 0.0f;

        while (endIndex > startIndex)
        {
            auto& pg = glyphs.getReference (--endIndex);
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

void GlyphArrangement::addJustifiedText (const Font& font, const String& text,
                                         float x, float y, float maxLineWidth,
                                         Justification horizontalLayout,
                                         float leading)
{
    auto lineStartIndex = glyphs.size();
    addLineOfText (font, text, x, y);

    auto originalY = y;

    while (lineStartIndex < glyphs.size())
    {
        int i = lineStartIndex;

        if (glyphs.getReference (i).getCharacter() != '\n'
              && glyphs.getReference (i).getCharacter() != '\r')
            ++i;

        auto lineMaxX = glyphs.getReference (lineStartIndex).getLeft() + maxLineWidth;
        int lastWordBreakIndex = -1;

        while (i < glyphs.size())
        {
            auto& pg = glyphs.getReference (i);
            auto c = pg.getCharacter();

            if (c == '\r' || c == '\n')
            {
                ++i;

                if (c == '\r' && i < glyphs.size()
                     && glyphs.getReference (i).getCharacter() == '\n')
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

        auto currentLineStartX = glyphs.getReference (lineStartIndex).getLeft();
        auto currentLineEndX = currentLineStartX;

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

        y += font.getHeight() + leading;
    }
}

void GlyphArrangement::addFittedText (const Font& f, const String& text,
                                      float x, float y, float width, float height,
                                      Justification layout, int maximumLines,
                                      float minimumHorizontalScale)
{
    if (approximatelyEqual (minimumHorizontalScale, 0.0f))
        minimumHorizontalScale = Font::getDefaultMinimumHorizontalScaleFactor();

    // doesn't make much sense if this is outside a sensible range of 0.5 to 1.0
    jassert (minimumHorizontalScale > 0 && minimumHorizontalScale <= 1.0f);

    if (text.containsAnyOf ("\r\n"))
    {
        addLinesWithLineBreaks (text, f, x, y, width, height, layout);
    }
    else
    {
        auto startIndex = glyphs.size();
        auto trimmed = text.trim();
        addLineOfText (f, trimmed, x, y);
        auto numGlyphs = glyphs.size() - startIndex;

        if (numGlyphs > 0)
        {
            auto lineWidth = glyphs.getReference (glyphs.size() - 1).getRight()
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

    if (! approximatelyEqual (dx, 0.0f) || ! approximatelyEqual (dy, 0.0f))
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

    auto bb = ga.getBoundingBox (0, -1, false);
    auto dy = y - bb.getY();

    if (layout.testFlags (Justification::verticallyCentred))   dy += (height - bb.getHeight()) * 0.5f;
    else if (layout.testFlags (Justification::bottom))         dy += (height - bb.getHeight());

    ga.moveRangeOfGlyphs (0, -1, 0.0f, dy);

    glyphs.addArray (ga.glyphs);
}

int GlyphArrangement::fitLineIntoSpace (int start, int numGlyphs, float x, float y, float w, float h, const Font& font,
                                        Justification justification, float minimumHorizontalScale)
{
    int numDeleted = 0;
    auto lineStartX = glyphs.getReference (start).getLeft();
    auto lineWidth  = glyphs.getReference (start + numGlyphs - 1).getRight() - lineStartX;

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

void GlyphArrangement::stretchRangeOfGlyphs (int startIndex, int num, float horizontalScaleFactor)
{
    jassert (startIndex >= 0);

    if (num < 0 || startIndex + num > glyphs.size())
        num = glyphs.size() - startIndex;

    if (num > 0)
    {
        auto xAnchor = glyphs.getReference (startIndex).getLeft();

        while (--num >= 0)
        {
            auto& pg = glyphs.getReference (startIndex++);

            pg.x = xAnchor + (pg.x - xAnchor) * horizontalScaleFactor;
            pg.font.setHorizontalScale (pg.font.getHorizontalScale() * horizontalScaleFactor);
            pg.w *= horizontalScaleFactor;
        }
    }
}

Rectangle<float> GlyphArrangement::getBoundingBox (int startIndex, int num, bool includeWhitespace) const
{
    jassert (startIndex >= 0);

    if (num < 0 || startIndex + num > glyphs.size())
        num = glyphs.size() - startIndex;

    Rectangle<float> result;

    while (--num >= 0)
    {
        auto& pg = glyphs.getReference (startIndex++);

        if (includeWhitespace || ! pg.isWhitespace())
            result = result.getUnion (pg.getBounds());
    }

    return result;
}

void GlyphArrangement::justifyGlyphs (int startIndex, int num,
                                      float x, float y, float width, float height,
                                      Justification justification)
{
    jassert (num >= 0 && startIndex >= 0);

    if (glyphs.size() > 0 && num > 0)
    {
        auto bb = getBoundingBox (startIndex, num, ! justification.testFlags (Justification::horizontallyJustified
                                                                               | Justification::horizontallyCentred));
        float deltaX = x, deltaY = y;

        if (justification.testFlags (Justification::horizontallyJustified))     deltaX -= bb.getX();
        else if (justification.testFlags (Justification::horizontallyCentred))  deltaX += (width - bb.getWidth()) * 0.5f - bb.getX();
        else if (justification.testFlags (Justification::right))                deltaX += width - bb.getRight();
        else                                                                    deltaX -= bb.getX();

        if (justification.testFlags (Justification::top))                       deltaY -= bb.getY();
        else if (justification.testFlags (Justification::bottom))               deltaY += height - bb.getBottom();
        else                                                                    deltaY += (height - bb.getHeight()) * 0.5f - bb.getY();

        moveRangeOfGlyphs (startIndex, num, deltaX, deltaY);

        if (justification.testFlags (Justification::horizontallyJustified))
        {
            int lineStart = 0;
            auto baseY = glyphs.getReference (startIndex).getBaselineY();

            int i;
            for (i = 0; i < num; ++i)
            {
                auto glyphY = glyphs.getReference (startIndex + i).getBaselineY();

                if (! approximatelyEqual (glyphY, baseY))
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

void GlyphArrangement::spreadOutLine (int start, int num, float targetWidth)
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
            auto startX = glyphs.getReference (start).getLeft();
            auto endX   = glyphs.getReference (start + num - 1 - spacesAtEnd).getRight();

            auto extraPaddingBetweenWords = (targetWidth - (endX - startX)) / (float) numSpaces;
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

static bool isBreakableGlyph (const PositionedGlyph& g) noexcept
{
    return ! isNonBreakingSpace (g.getCharacter()) && (g.isWhitespace() || g.getCharacter() == '-');
}

void GlyphArrangement::splitLines (const String& text, Font font, int startIndex,
                                   float x, float y, float width, float height, int maximumLines,
                                   float lineWidth, Justification layout, float minimumHorizontalScale)
{
    auto length = text.length();
    auto originalStartIndex = startIndex;
    int numLines = 1;

    if (length <= 12 && ! text.containsAnyOf (" -\t\r\n"))
        maximumLines = 1;

    maximumLines = jmin (maximumLines, length);

    while (numLines < maximumLines)
    {
        ++numLines;
        auto newFontHeight = height / (float) numLines;

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

        if ((float) numLines > (lineWidth + lineLengthUnevennessAllowance) / width || newFontHeight < 8.0f)
            break;
    }

    if (numLines < 1)
        numLines = 1;

    int lineIndex = 0;
    auto lineY = y;
    auto widthPerLine = jmin (width / minimumHorizontalScale,
                              lineWidth / (float) numLines);

    while (lineY < y + height)
    {
        auto endIndex = startIndex;
        auto lineStartX = glyphs.getReference (startIndex).getLeft();
        auto lineBottomY = lineY + font.getHeight();

        if (lineIndex++ >= numLines - 1
             || lineBottomY >= y + height)
        {
            widthPerLine = width;
            endIndex = glyphs.size();
        }
        else
        {
            while (endIndex < glyphs.size())
            {
                if (glyphs.getReference (endIndex).getRight() - lineStartX > widthPerLine)
                {
                    // got to a point where the line's too long, so skip forward to find a
                    // good place to break it..
                    auto searchStartIndex = endIndex;

                    while (endIndex < glyphs.size())
                    {
                        auto& g = glyphs.getReference (endIndex);

                        if ((g.getRight() - lineStartX) * minimumHorizontalScale < width)
                        {
                            if (isBreakableGlyph (g))
                            {
                                ++endIndex;
                                break;
                            }
                        }
                        else
                        {
                            // can't find a suitable break, so try looking backwards..
                            endIndex = searchStartIndex;

                            for (int back = 1; back < jmin (7, endIndex - startIndex - 1); ++back)
                            {
                                if (isBreakableGlyph (glyphs.getReference (endIndex - back)))
                                {
                                    endIndex -= back - 1;
                                    break;
                                }
                            }

                            break;
                        }

                        ++endIndex;
                    }

                    break;
                }

                ++endIndex;
            }

            auto wsStart = endIndex;
            auto wsEnd   = endIndex;

            while (wsStart > 0 && glyphs.getReference (wsStart - 1).isWhitespace())
                --wsStart;

            while (wsEnd < glyphs.size() && glyphs.getReference (wsEnd).isWhitespace())
                ++wsEnd;

            removeRangeOfGlyphs (wsStart, wsEnd - wsStart);
            endIndex = jmax (wsStart, startIndex + 1);
        }

        endIndex -= fitLineIntoSpace (startIndex, endIndex - startIndex,
                                      x, lineY, width, font.getHeight(), font,
                                      layout.getOnlyHorizontalFlags() | Justification::verticallyCentred,
                                      minimumHorizontalScale);

        startIndex = endIndex;
        lineY = lineBottomY;

        if (startIndex >= glyphs.size())
            break;
    }

    justifyGlyphs (originalStartIndex, glyphs.size() - originalStartIndex,
                   x, y, width, height, layout.getFlags() & ~Justification::horizontallyJustified);
}

//==============================================================================
void GlyphArrangement::drawGlyphUnderline (const Graphics& g, const PositionedGlyph& pg,
                                           int i, AffineTransform transform) const
{
    auto lineThickness = (pg.font.getDescent()) * 0.3f;
    auto nextX = pg.x + pg.w;

    if (i < glyphs.size() - 1 && approximatelyEqual (glyphs.getReference (i + 1).y, pg.y))
        nextX = glyphs.getReference (i + 1).x;

    Path p;
    p.addRectangle (pg.x, pg.y + lineThickness * 2.0f, nextX - pg.x, lineThickness);
    g.fillPath (p, transform);
}

void GlyphArrangement::draw (const Graphics& g) const
{
    draw (g, {});
}

void GlyphArrangement::draw (const Graphics& g, AffineTransform transform) const
{
    auto& context = g.getInternalContext();
    auto lastFont = context.getFont();
    bool needToRestore = false;

    for (int i = 0; i < glyphs.size(); ++i)
    {
        auto& pg = glyphs.getReference (i);

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

            context.drawGlyph (pg.glyph, AffineTransform::translation (pg.x, pg.y)
                                                         .followedBy (transform));
        }
    }

    if (needToRestore)
        context.restoreState();
}

void GlyphArrangement::createPath (Path& path) const
{
    for (auto& g : glyphs)
        g.createPath (path);
}

int GlyphArrangement::findGlyphIndexAt (float x, float y) const
{
    for (int i = 0; i < glyphs.size(); ++i)
        if (glyphs.getReference (i).hitTest (x, y))
            return i;

    return -1;
}

} // namespace juce
