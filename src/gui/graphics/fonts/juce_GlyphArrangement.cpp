/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#include "../../../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_GlyphArrangement.h"
#include "../contexts/juce_LowLevelGraphicsSoftwareRenderer.h"
#include "../imaging/juce_Image.h"
#include "../../../utilities/juce_DeletedAtShutdown.h"


//==============================================================================
PositionedGlyph::PositionedGlyph()
{
}

void PositionedGlyph::draw (const Graphics& g) const
{
    if (! isWhitespace())
    {
        g.getInternalContext()->setFont (font);
        g.getInternalContext()->drawGlyph (glyph, AffineTransform::translation (x, y));
    }
}

void PositionedGlyph::draw (const Graphics& g,
                            const AffineTransform& transform) const
{
    if (! isWhitespace())
    {
        g.getInternalContext()->setFont (font);
        g.getInternalContext()->drawGlyph (glyph, AffineTransform::translation (x, y)
                                                                  .followedBy (transform));
    }
}

void PositionedGlyph::createPath (Path& path) const
{
    if (! isWhitespace())
    {
        Typeface* const t = font.getTypeface();

        if (t != 0)
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
    if (px >= getLeft() && px < getRight()
        && py >= getTop() && py < getBottom()
        && ! isWhitespace())
    {
        Typeface* const t = font.getTypeface();

        if (t != 0)
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
{
    addGlyphArrangement (other);
}

const GlyphArrangement& GlyphArrangement::operator= (const GlyphArrangement& other)
{
    if (this != &other)
    {
        clear();
        addGlyphArrangement (other);
    }

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

PositionedGlyph& GlyphArrangement::getGlyph (const int index) const
{
    jassert (((unsigned int) index) < (unsigned int) glyphs.size());

    return *glyphs [index];
}

//==============================================================================
void GlyphArrangement::addGlyphArrangement (const GlyphArrangement& other)
{
    glyphs.ensureStorageAllocated (glyphs.size() + other.glyphs.size());

    for (int i = 0; i < other.glyphs.size(); ++i)
        glyphs.add (new PositionedGlyph (*other.glyphs.getUnchecked (i)));
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
    addCurtailedLineOfText (font, text,
                            xOffset, yOffset,
                            1.0e10f, false);
}

void GlyphArrangement::addCurtailedLineOfText (const Font& font,
                                               const String& text,
                                               float xOffset,
                                               const float yOffset,
                                               const float maxWidthPixels,
                                               const bool useEllipsis)
{
    int textLen = text.length();

    if (textLen > 0)
    {
        Array <int> newGlyphs;
        Array <float> xOffsets;
        font.getGlyphPositions (text, newGlyphs, xOffsets);

        const juce_wchar* const unicodeText = (const juce_wchar*) text;
        textLen = jmin (textLen, newGlyphs.size());

        for (int i = 0; i < textLen; ++i)
        {
            const float thisX = xOffsets.getUnchecked (i);
            const float nextX = xOffsets.getUnchecked (i + 1);

            if (nextX > maxWidthPixels + 1.0f)
            {
                // curtail the string if it's too wide..
                if (useEllipsis && textLen > 3 && glyphs.size() >= 3)
                    insertEllipsis (font, xOffset + maxWidthPixels, 0, glyphs.size());

                break;
            }
            else
            {
                PositionedGlyph* const pg = new PositionedGlyph();
                pg->x = xOffset + thisX;
                pg->y = yOffset;
                pg->w = nextX - thisX;
                pg->font = font;
                pg->glyph = newGlyphs.getUnchecked(i);
                pg->character = unicodeText[i];

                glyphs.add (pg);
            }
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
        font.getGlyphPositions (T(".."), dotGlyphs, dotXs);

        const float dx = dotXs[1];
        float xOffset = 0.0f, yOffset = 0.0f;

        while (endIndex > startIndex)
        {
            const PositionedGlyph* pg = glyphs.getUnchecked (--endIndex);
            xOffset = pg->x;
            yOffset = pg->y;

            glyphs.remove (endIndex);
            ++numDeleted;

            if (xOffset + dx * 3 <= maxXPos)
                break;
        }

        for (int i = 3; --i >= 0;)
        {
            PositionedGlyph* const pg = new PositionedGlyph();
            pg->x = xOffset;
            pg->y = yOffset;
            pg->w = dx;
            pg->font = font;
            pg->character = '.';
            pg->glyph = dotGlyphs.getFirst();
            glyphs.insert (endIndex++, pg);
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
                                         const Justification& horizontalLayout)
{
    int lineStartIndex = glyphs.size();
    addLineOfText (font, text, x, y);

    const float originalY = y;

    while (lineStartIndex < glyphs.size())
    {
        int i = lineStartIndex;

        if (glyphs.getUnchecked(i)->getCharacter() != T('\n')
              && glyphs.getUnchecked(i)->getCharacter() != T('\r'))
            ++i;

        const float lineMaxX = glyphs.getUnchecked (lineStartIndex)->getLeft() + maxLineWidth;
        int lastWordBreakIndex = -1;

        while (i < glyphs.size())
        {
            const PositionedGlyph* pg = glyphs.getUnchecked (i);
            const juce_wchar c = pg->getCharacter();

            if (c == T('\r') || c == T('\n'))
            {
                ++i;

                if (c == T('\r') && i < glyphs.size()
                     && glyphs.getUnchecked(i)->getCharacter() == T('\n'))
                    ++i;

                break;
            }
            else if (pg->isWhitespace())
            {
                lastWordBreakIndex = i + 1;
            }
            else if (pg->getRight() - 0.0001f >= lineMaxX)
            {
                if (lastWordBreakIndex >= 0)
                    i = lastWordBreakIndex;

                break;
            }

            ++i;
        }

        const float currentLineStartX = glyphs.getUnchecked (lineStartIndex)->getLeft();
        float currentLineEndX = currentLineStartX;

        for (int j = i; --j >= lineStartIndex;)
        {
            if (! glyphs.getUnchecked (j)->isWhitespace())
            {
                currentLineEndX = glyphs.getUnchecked (j)->getRight();
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
                                      const Justification& layout,
                                      int maximumLines,
                                      const float minimumHorizontalScale)
{
    // doesn't make much sense if this is outside a sensible range of 0.5 to 1.0
    jassert (minimumHorizontalScale > 0 && minimumHorizontalScale <= 1.0f);

    if (text.containsAnyOf (T("\r\n")))
    {
        GlyphArrangement ga;
        ga.addJustifiedText (f, text, x, y, width, layout);

        float l, t, r, b;
        ga.getBoundingBox (0, -1, l, t, r, b, false);

        float dy = y - t;

        if (layout.testFlags (Justification::verticallyCentred))
            dy += (height - (b - t)) * 0.5f;
        else if (layout.testFlags (Justification::bottom))
            dy += height - (b - t);

        ga.moveRangeOfGlyphs (0, -1, 0.0f, dy);

        glyphs.ensureStorageAllocated (glyphs.size() + ga.glyphs.size());

        for (int i = 0; i < ga.glyphs.size(); ++i)
            glyphs.add (ga.glyphs.getUnchecked (i));

        ga.glyphs.clear (false);
        return;
    }

    int startIndex = glyphs.size();
    addLineOfText (f, text.trim(), x, y);

    if (glyphs.size() > startIndex)
    {
        float lineWidth = glyphs.getUnchecked (glyphs.size() - 1)->getRight()
                            - glyphs.getUnchecked (startIndex)->getLeft();

        if (lineWidth <= 0)
            return;

        if (lineWidth * minimumHorizontalScale < width)
        {
            if (lineWidth > width)
                stretchRangeOfGlyphs (startIndex, glyphs.size() - startIndex,
                                      width / lineWidth);

            justifyGlyphs (startIndex, glyphs.size() - startIndex,
                           x, y, width, height, layout);
        }
        else if (maximumLines <= 1)
        {
            fitLineIntoSpace (startIndex, glyphs.size() - startIndex,
                              x, y, width, height, f, layout, minimumHorizontalScale);
        }
        else
        {
            Font font (f);
            String txt (text.trim());
            const int length = txt.length();
            const int originalStartIndex = startIndex;
            int numLines = 1;

            if (length <= 12 && ! txt.containsAnyOf (T(" -\t\r\n")))
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
                    addLineOfText (font, txt, x, y);

                    lineWidth = glyphs.getUnchecked (glyphs.size() - 1)->getRight()
                                    - glyphs.getUnchecked (startIndex)->getLeft();
                }

                if (numLines > lineWidth / width || newFontHeight < 8.0f)
                    break;
            }

            if (numLines < 1)
                numLines = 1;

            float lineY = y;
            float widthPerLine = lineWidth / numLines;
            int lastLineStartIndex = 0;

            for (int line = 0; line < numLines; ++line)
            {
                int i = startIndex;
                lastLineStartIndex = i;
                float lineStartX = glyphs.getUnchecked (startIndex)->getLeft();

                if (line == numLines - 1)
                {
                    widthPerLine = width;
                    i = glyphs.size();
                }
                else
                {
                    while (i < glyphs.size())
                    {
                        lineWidth = (glyphs.getUnchecked (i)->getRight() - lineStartX);

                        if (lineWidth > widthPerLine)
                        {
                            // got to a point where the line's too long, so skip forward to find a
                            // good place to break it..
                            const int searchStartIndex = i;

                            while (i < glyphs.size())
                            {
                                if ((glyphs.getUnchecked (i)->getRight() - lineStartX) * minimumHorizontalScale < width)
                                {
                                    if (glyphs.getUnchecked (i)->isWhitespace()
                                         || glyphs.getUnchecked (i)->getCharacter() == T('-'))
                                    {
                                        ++i;
                                        break;
                                    }
                                }
                                else
                                {
                                    // can't find a suitable break, so try looking backwards..
                                    i = searchStartIndex;

                                    for (int back = 1; back < jmin (5, i - startIndex - 1); ++back)
                                    {
                                        if (glyphs.getUnchecked (i - back)->isWhitespace()
                                             || glyphs.getUnchecked (i - back)->getCharacter() == T('-'))
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
                    while (wsStart > 0 && glyphs.getUnchecked (wsStart - 1)->isWhitespace())
                        --wsStart;

                    int wsEnd = i;

                    while (wsEnd < glyphs.size() && glyphs.getUnchecked (wsEnd)->isWhitespace())
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
    }
}

//==============================================================================
void GlyphArrangement::moveRangeOfGlyphs (int startIndex, int num,
                                          const float dx, const float dy)
{
    jassert (startIndex >= 0);

    if (dx != 0.0f || dy != 0.0f)
    {
        if (num < 0 || startIndex + num > glyphs.size())
            num = glyphs.size() - startIndex;

        while (--num >= 0)
            glyphs.getUnchecked (startIndex++)->moveBy (dx, dy);
    }
}

int GlyphArrangement::fitLineIntoSpace (int start, int numGlyphs, float x, float y, float w, float h, const Font& font,
                                        const Justification& justification, float minimumHorizontalScale)
{
    int numDeleted = 0;
    const float lineStartX = glyphs.getUnchecked (start)->getLeft();
    float lineWidth = glyphs.getUnchecked (start + numGlyphs - 1)->getRight() - lineStartX;

    if (lineWidth > w)
    {
        if (minimumHorizontalScale < 1.0f)
        {
            stretchRangeOfGlyphs (start, numGlyphs, jmax (minimumHorizontalScale, w / lineWidth));
            lineWidth = glyphs.getUnchecked (start + numGlyphs - 1)->getRight() - lineStartX - 0.5f;
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
        const float xAnchor = glyphs.getUnchecked (startIndex)->getLeft();

        while (--num >= 0)
        {
            PositionedGlyph* const pg = glyphs.getUnchecked (startIndex++);

            pg->x = xAnchor + (pg->x - xAnchor) * horizontalScaleFactor;
            pg->font.setHorizontalScale (pg->font.getHorizontalScale() * horizontalScaleFactor);
            pg->w *= horizontalScaleFactor;
        }
    }
}

void GlyphArrangement::getBoundingBox (int startIndex, int num,
                                       float& left,
                                       float& top,
                                       float& right,
                                       float& bottom,
                                       const bool includeWhitespace) const
{
    jassert (startIndex >= 0);

    if (num < 0 || startIndex + num > glyphs.size())
        num = glyphs.size() - startIndex;

    left = 0.0f;
    top = 0.0f;
    right = 0.0f;
    bottom = 0.0f;
    bool isFirst = true;

    while (--num >= 0)
    {
        const PositionedGlyph* const pg = glyphs.getUnchecked (startIndex++);

        if (includeWhitespace || ! pg->isWhitespace())
        {
            if (isFirst)
            {
                isFirst = false;
                left    = pg->getLeft();
                top     = pg->getTop();
                right   = pg->getRight();
                bottom  = pg->getBottom();
            }
            else
            {
                left    = jmin (left, pg->getLeft());
                top     = jmin (top, pg->getTop());
                right   = jmax (right, pg->getRight());
                bottom  = jmax (bottom, pg->getBottom());
            }
        }
    }
}

void GlyphArrangement::justifyGlyphs (const int startIndex,
                                      const int num,
                                      const float x, const float y,
                                      const float width, const float height,
                                      const Justification& justification)
{
    jassert (num >= 0 && startIndex >= 0);

    if (glyphs.size() > 0 && num > 0)
    {
        float left, top, right, bottom;
        getBoundingBox (startIndex, num, left, top, right, bottom,
                        ! justification.testFlags (Justification::horizontallyJustified
                                                    | Justification::horizontallyCentred));

        float deltaX = 0.0f;

        if (justification.testFlags (Justification::horizontallyJustified))
            deltaX = x - left;
        else if (justification.testFlags (Justification::horizontallyCentred))
            deltaX = x + (width - (right - left)) * 0.5f - left;
        else if (justification.testFlags (Justification::right))
            deltaX = (x + width) - right;
        else
            deltaX = x - left;

        float deltaY = 0.0f;

        if (justification.testFlags (Justification::top))
            deltaY = y - top;
        else if (justification.testFlags (Justification::bottom))
            deltaY = (y + height) - bottom;
        else
            deltaY = y + (height - (bottom - top)) * 0.5f - top;

        moveRangeOfGlyphs (startIndex, num, deltaX, deltaY);

        if (justification.testFlags (Justification::horizontallyJustified))
        {
            int lineStart = 0;
            float baseY = glyphs.getUnchecked (startIndex)->getBaselineY();

            int i;
            for (i = 0; i < num; ++i)
            {
                const float glyphY = glyphs.getUnchecked (startIndex + i)->getBaselineY();

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
         && glyphs.getUnchecked (start + num - 1)->getCharacter() != T('\r')
         && glyphs.getUnchecked (start + num - 1)->getCharacter() != T('\n'))
    {
        int numSpaces = 0;
        int spacesAtEnd = 0;

        for (int i = 0; i < num; ++i)
        {
            if (glyphs.getUnchecked (start + i)->isWhitespace())
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
            const float startX = glyphs.getUnchecked (start)->getLeft();
            const float endX = glyphs.getUnchecked (start + num - 1 - spacesAtEnd)->getRight();

            const float extraPaddingBetweenWords
                = (targetWidth - (endX - startX)) / (float) numSpaces;

            float deltaX = 0.0f;

            for (int i = 0; i < num; ++i)
            {
                glyphs.getUnchecked (start + i)->moveBy (deltaX, 0.0f);

                if (glyphs.getUnchecked (start + i)->isWhitespace())
                    deltaX += extraPaddingBetweenWords;
            }
        }
    }
}

//==============================================================================
void GlyphArrangement::draw (const Graphics& g) const
{
    for (int i = 0; i < glyphs.size(); ++i)
    {
        const PositionedGlyph* const pg = glyphs.getUnchecked(i);

        if (pg->font.isUnderlined())
        {
            const float lineThickness = (pg->font.getDescent()) * 0.3f;

            float nextX = pg->x + pg->w;

            if (i < glyphs.size() - 1 && glyphs.getUnchecked (i + 1)->y == pg->y)
                nextX = glyphs.getUnchecked (i + 1)->x;

            g.fillRect (pg->x, pg->y + lineThickness * 2.0f,
                        nextX - pg->x, lineThickness);
        }

        pg->draw (g);
    }
}

void GlyphArrangement::draw (const Graphics& g, const AffineTransform& transform) const
{
    for (int i = 0; i < glyphs.size(); ++i)
    {
        const PositionedGlyph* const pg = glyphs.getUnchecked(i);

        if (pg->font.isUnderlined())
        {
            const float lineThickness = (pg->font.getDescent()) * 0.3f;

            float nextX = pg->x + pg->w;

            if (i < glyphs.size() - 1 && glyphs.getUnchecked (i + 1)->y == pg->y)
                nextX = glyphs.getUnchecked (i + 1)->x;

            Path p;
            p.addLineSegment (pg->x, pg->y + lineThickness * 2.0f,
                              nextX, pg->y + lineThickness * 2.0f,
                              lineThickness);

            g.fillPath (p, transform);
        }

        pg->draw (g, transform);
    }
}

void GlyphArrangement::createPath (Path& path) const
{
    for (int i = 0; i < glyphs.size(); ++i)
        glyphs.getUnchecked (i)->createPath (path);
}

int GlyphArrangement::findGlyphIndexAt (float x, float y) const
{
    for (int i = 0; i < glyphs.size(); ++i)
        if (glyphs.getUnchecked (i)->hitTest (x, y))
            return i;

    return -1;
}

END_JUCE_NAMESPACE
