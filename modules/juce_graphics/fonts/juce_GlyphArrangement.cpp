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

void PositionedGlyph::draw (Graphics& g) const
{
    draw (g, {});
}

void PositionedGlyph::draw (Graphics& g, AffineTransform transform) const
{
    if (isWhitespace())
        return;

    auto& context = g.getInternalContext();
    context.setFont (font);
    const uint16_t glyphs[] { static_cast<uint16_t> (glyph) };
    const Point<float> positions[] { Point { x, y } };
    context.drawGlyphs (glyphs, positions, transform);
}

void PositionedGlyph::createPath (Path& path) const
{
    if (! isWhitespace())
    {
        if (auto t = font.getTypefacePtr())
        {
            Path p;
            t->getOutlineForGlyph (font.getMetricsKind(), glyph, p);

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
            t->getOutlineForGlyph (font.getMetricsKind(), glyph, p);

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

static void addGlyphsFromShapedText (GlyphArrangement& ga, const ShapedText& st, float x, float y)
{
    st.access ([&] (auto shapedGlyphs, auto positions, auto font, auto glyphRange, auto)
               {
                   for (size_t i = 0; i < shapedGlyphs.size(); ++i)
                   {
                       const auto glyphIndex = (int64) i + glyphRange.getStart();

                       auto& glyph = shapedGlyphs[i];
                       auto& position = positions[i];

                       PositionedGlyph pg { font,
                                            st.getText()[(int) st.getTextRange (glyphIndex).getStart()],
                                            (int) glyph.glyphId,
                                            position.getX() + x,
                                            position.getY() + y,
                                            glyph.advance.getX(),
                                            glyph.whitespace };

                       ga.addGlyph (std::move (pg));
                   }
               });
}

void GlyphArrangement::addCurtailedLineOfText (const Font& font, const String& text,
                                               float xOffset, float yOffset,
                                               float maxWidthPixels, bool useEllipsis)
{
    auto options = ShapedText::Options{}.withMaxNumLines (1)
                                        .withMaxWidth (maxWidthPixels)
                                        .withFont (font)
                                        .withBaselineAtZero()
                                        .withTrailingWhitespacesShouldFit (false);

    if (useEllipsis)
        options = options.withEllipsis();

    ShapedText st { text, options };

    addGlyphsFromShapedText (*this, st, xOffset, yOffset);
}

void GlyphArrangement::addJustifiedText (const Font& font, const String& text,
                                         float x, float y, float maxLineWidth,
                                         Justification horizontalLayout,
                                         float leading)
{
    ShapedText st { text, ShapedText::Options{}.withMaxWidth (maxLineWidth)
                                               .withJustification (horizontalLayout)
                                               .withFont (font)
                                               .withLeading (1.0f + leading / font.getHeight())
                                               .withBaselineAtZero()
                                               .withTrailingWhitespacesShouldFit (false) };

    addGlyphsFromShapedText (*this, st, x, y);
}

void GlyphArrangement::addFittedText (const Font& f,
                                      const String& text,
                                      float x,
                                      float y,
                                      float width,
                                      float height,
                                      Justification layout,
                                      int maximumLines,
                                      float minimumHorizontalScale)
{
    if (! layout.testFlags (Justification::bottom | Justification::top))
        layout = layout.getOnlyHorizontalFlags() | Justification::verticallyCentred;

    if (approximatelyEqual (minimumHorizontalScale, 0.0f))
        minimumHorizontalScale = Font::getDefaultMinimumHorizontalScaleFactor();

    // doesn't make much sense if this is outside a sensible range of 0.5 to 1.0
    jassert (minimumHorizontalScale > 0 && minimumHorizontalScale <= 1.0f);

    if (text.containsAnyOf ("\r\n"))
    {
        ShapedText st { text,
                        ShapedText::Options{}
                            .withMaxWidth (width)
                            .withHeight (height)
                            .withJustification (layout)
                            .withFont (f)
                            .withTrailingWhitespacesShouldFit (false) };

        addGlyphsFromShapedText (*this, st, x, y);

        return;
    }

    const auto trimmed = text.trim();

    // First attempt: try to squash the entire text on a single line
    {
        ShapedText st { trimmed, ShapedText::Options{}.withFont (f)
                                                      .withMaxWidth (width)
                                                      .withHeight (height)
                                                      .withMaxNumLines (1)
                                                      .withJustification (layout)
                                                      .withTrailingWhitespacesShouldFit (false) };

        const auto requiredWidths = st.getMinimumRequiredWidthForLines();

        if (requiredWidths.empty() || requiredWidths.front() <= width)
        {
            addGlyphsFromShapedText (*this, st, x, y);
            return;
        }

        // If we can fit the entire line, squash by just enough and insert
        if (requiredWidths.front() * minimumHorizontalScale < width)
        {
            ShapedText squashed { trimmed,
                                  ShapedText::Options{}
                                      .withFont (f.withHorizontalScale (width / (requiredWidths.front() + 0.01f)))
                                      .withMaxWidth (width)
                                      .withHeight (height)
                                      .withJustification (layout)
                                      .withTrailingWhitespacesShouldFit (false)};

            addGlyphsFromShapedText (*this, squashed, x, y);
            return;
        }
    }

    if (maximumLines <= 1)
    {
        ShapedText squashed { trimmed,
                              ShapedText::Options{}
                                  .withFont (f.withHorizontalScale (minimumHorizontalScale))
                                  .withMaxWidth (width)
                                  .withHeight (height)
                                  .withJustification (layout)
                                  .withMaxNumLines (1)
                                  .withEllipsis() };

        addGlyphsFromShapedText (*this, squashed, x, y);
        return;
    }

    // Keep reshaping the text constantly decreasing the fontsize and increasing the number of lines
    // until all text fits.

    auto length  = trimmed.length();
    int numLines = 1;

    if (length <= 12 && ! trimmed.containsAnyOf (" -\t\r\n"))
        maximumLines = 1;

    maximumLines = jmin (maximumLines, length);

    auto font = f;
    auto cumulativeLineLengths = font.getHeight() * 1.4f;

    while (numLines < maximumLines)
    {
        ++numLines;
        auto newFontHeight = height / (float) numLines;

        if (newFontHeight < font.getHeight())
            font.setHeight (jmax (8.0f, newFontHeight));

        ShapedText squashed { trimmed,
                              ShapedText::Options{}
                                  .withFont (font)
                                  .withMaxWidth (width)
                                  .withHeight (height)
                                  .withMaxNumLines (numLines)
                                  .withJustification (layout)
                                  .withTrailingWhitespacesShouldFit (false) };

        const auto lineWidths = squashed.getMinimumRequiredWidthForLines();

        if (lineWidths.empty() || lineWidths.back() <= width)
        {
            addGlyphsFromShapedText (*this, squashed, x, y);
            return;
        }

        // We're trying to guess how much horizontal space the text would need to fit, and we
        // need to have an allowance for line end whitespaces which take up additional room
        // when not falling at the end of lines.
        cumulativeLineLengths = std::accumulate (lineWidths.begin(), lineWidths.end(), 0.0f)
                                + font.getHeight() * (float) numLines * 1.4f;

        if (newFontHeight < 8.0f)
            break;
    }

    // At this point we failed to fit the text by just increasing the number of lines and decreasing
    // the font size. Horizontal squashing is also necessary, for which horizontal justification is
    // enabled.
    layout = layout.getOnlyVerticalFlags() | Justification::horizontallyJustified;

    //==============================================================================
    // We run an iterative interval halving algorithm to find the largest scale that can fit all
    // text
    auto makeShapedText = [&] (float horizontalScale)
    {
        return ShapedText { trimmed,
                            ShapedText::Options{}
                                .withFont (font.withHorizontalScale (horizontalScale))
                                .withMaxWidth (width)
                                .withHeight (height)
                                .withMaxNumLines (numLines)
                                .withJustification (layout)
                                .withTrailingWhitespacesShouldFit (false)
                                .withEllipsis() };
    };

    auto lowerScaleBound = minimumHorizontalScale;
    auto upperScaleBound = std::max (minimumHorizontalScale, ((float) numLines * width) / cumulativeLineLengths);

    const auto isFittingAllText = [width] (auto& shapedText)
    {
        // ShapedText guarantees that all lines maybe except the last - when there is a maximum line
        // limit - will fit the requested width
        return shapedText.getMinimumRequiredWidthForLines().back() <= width;
    };

    if (auto st = makeShapedText (upperScaleBound);
        isFittingAllText (st)
        || approximatelyEqual (upperScaleBound, minimumHorizontalScale))
    {
        addGlyphsFromShapedText (*this, st, x, y);
        return;
    }

    struct Candidate
    {
        float scale{};
        ShapedText shapedText;
    };

    Candidate candidate { minimumHorizontalScale, makeShapedText (minimumHorizontalScale) };

    for (int i = 0, numApproximatingIterations = 3; i < numApproximatingIterations; ++i)
    {
        auto scale = (upperScaleBound - lowerScaleBound) / 2.0f;

        if (auto st = makeShapedText (scale);
            isFittingAllText (st))
        {
            lowerScaleBound = std::max (lowerScaleBound, scale);

            if (candidate.scale < scale)
            {
                candidate.scale = scale;
                candidate.shapedText = std::move (st);
            }
        }
        else
        {
            upperScaleBound = std::min (upperScaleBound, scale);
        }
    }

    if (approximatelyEqual (candidate.scale, minimumHorizontalScale)
        || candidate.shapedText.getMinimumRequiredWidthForLines().back() <= width)
    {
        addGlyphsFromShapedText (*this, candidate.shapedText, x, y);
        return;
    }

    addGlyphsFromShapedText (*this, candidate.shapedText, x, y);
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

//==============================================================================
void GlyphArrangement::drawGlyphUnderline (const Graphics& g,
                                           int i,
                                           AffineTransform transform) const
{
    const auto pg = glyphs.getReference (i);

    if (! pg.font.isUnderlined())
        return;

    const auto lineThickness = (pg.font.getDescent()) * 0.3f;

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
    std::vector<uint16_t> glyphNumbers;
    std::vector<Point<float>> positions;

    glyphNumbers.reserve (static_cast<size_t> (glyphs.size()));
    positions.reserve (static_cast<size_t> (glyphs.size()));

    auto& ctx = g.getInternalContext();
    ctx.saveState();
    const ScopeGuard guard { [&ctx] { ctx.restoreState(); } };

    for (auto it = glyphs.begin(), end = glyphs.end(); it != end;)
    {
        const auto adjacent = std::adjacent_find (it, end, [] (const auto& a, const auto& b)
        {
            return a.font != b.font;
        });

        const auto next = adjacent + (adjacent == end ? 0 : 1);

        glyphNumbers.clear();
        std::transform (it, next, std::back_inserter (glyphNumbers), [] (const PositionedGlyph& x)
        {
            return (uint16_t) x.glyph;
        });

        positions.clear();
        std::transform (it, next, std::back_inserter (positions), [] (const PositionedGlyph& x)
        {
            return Point { x.x, x.y };
        });

        ctx.setFont (it->font);
        ctx.drawGlyphs (glyphNumbers, positions, transform);

        it = next;
    }

    for (const auto pair : enumerate (glyphs))
        drawGlyphUnderline (g, static_cast<int> (pair.index), transform);
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
