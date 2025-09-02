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

static String portableTrim (String toTrim)
{
    if (toTrim.isEmpty())
        return toTrim;

    const auto b = toTrim.begin();
    const auto e = toTrim.end();

    const auto shouldTrim = [] (auto ptr)
    {
        return SBCodepointGetBidiType ((SBCodepoint) *ptr) == SBBidiTypeWS;
    };

    const auto trimmedBegin = CharacterFunctions::trimBegin (b, e, shouldTrim);
    const auto trimmedEnd = CharacterFunctions::trimEnd (trimmedBegin, e, shouldTrim);

    if (trimmedBegin == b && trimmedEnd == e)
        return toTrim;

    return String (trimmedBegin, trimmedEnd);
}

static bool areAllRequiredWidthsSmallerThanMax (const detail::ShapedText& shapedText, float width)
{
    const auto lineWidths = shapedText.getMinimumRequiredWidthForLines();
    return std::all_of (lineWidths.begin(), lineWidths.end(), [width] (auto& w) { return w <= width; });
}

// ShapedText truncates the last line by default, even if it requires larger width than the maximum
// allowed.
static bool areAllRequiredWidthsExceptTheLastSmallerThanMax (const detail::ShapedText& shapedText, float width)
{
    const auto lineWidths = shapedText.getMinimumRequiredWidthForLines();

    if (lineWidths.empty())
        return true;

    return std::all_of (lineWidths.begin(), std::prev (lineWidths.end()), [width] (auto& w) { return w <= width; });
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

static void addGlyphsFromShapedText (GlyphArrangement& ga, const detail::ShapedText& st, float x, float y)
{
    st.accessTogetherWith ([&] (auto shapedGlyphs, auto positions, auto font, auto glyphRange, auto)
    {
        for (auto it = shapedGlyphs.begin(); it != shapedGlyphs.end();)
        {
            const auto& glyph = *it;
            const auto isNotPlaceholder = [] (auto& shapedGlyph)
            {
                return ! shapedGlyph.isPlaceholderForLigature();
            };

            const auto next = std::find_if (std::next (it),
                                            shapedGlyphs.end(),
                                            isNotPlaceholder);

            const auto addWidth = [] (auto acc, auto& shapedGlyph)
            {
                return acc + shapedGlyph.advance.x;
            };

            const auto width = std::accumulate (it, next, 0.0f, addWidth);
            const auto index = (size_t) std::distance (shapedGlyphs.begin(), it);
            const auto position = positions[index];
            const auto glyphIndex = (int64) index + glyphRange.getStart();

            ga.addGlyph ({ font,
                           st.getText()[(int) st.getTextRange (glyphIndex).getStart()],
                           (int) glyph.glyphId,
                           position.getX() + x,
                           position.getY() + y,
                           width,
                           glyph.isWhitespace() });

            it = next;
        }
    });
}

void GlyphArrangement::addCurtailedLineOfText (const Font& font, const String& text,
                                               float xOffset, float yOffset,
                                               float maxWidthPixels, bool useEllipsis)
{
    using namespace detail;

    auto options = ShapedText::Options{}.withMaxNumLines (1)
                                        .withWordWrapWidth (maxWidthPixels)
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
    using namespace detail;

    ShapedText st { text, ShapedText::Options{}.withWordWrapWidth (maxLineWidth)
                                               .withJustification (horizontalLayout)
                                               .withFont (font)
                                               .withAdditiveLineSpacing (leading)
                                               .withBaselineAtZero()
                                               .withTrailingWhitespacesShouldFit (false) };

    addGlyphsFromShapedText (*this, st, x, y);
}

static auto createFittedText (const Font& f,
                              const String& text,
                              float width,
                              float height,
                              Justification layout,
                              int maximumLines,
                              float minimumRelativeHorizontalScale,
                              detail::ShapedText::Options baseOptions)
{
    using namespace detail;

    if (! layout.testFlags (Justification::bottom | Justification::top))
        layout = layout.getOnlyHorizontalFlags() | Justification::verticallyCentred;

    if (approximatelyEqual (minimumRelativeHorizontalScale, 0.0f))
        minimumRelativeHorizontalScale = Font::getDefaultMinimumHorizontalScaleFactor();

    // doesn't make much sense if this is outside a sensible range of 0.5 to 1.0
    jassert (0 < minimumRelativeHorizontalScale && minimumRelativeHorizontalScale <= 1.0f);

    if (text.containsAnyOf ("\r\n"))
    {
        ShapedText st { text,
                        baseOptions
                            .withWordWrapWidth (width)
                            .withHeight (height)
                            .withJustification (layout)
                            .withFont (f)
                            .withTrailingWhitespacesShouldFit (false) };

        return st;
    }

    const auto trimmed = portableTrim (text);

    constexpr auto widthFittingTolerance = 0.01f;

    // First attempt: try to squash the entire text on a single line
    {
        ShapedText st { trimmed, baseOptions.withFont (f)
                                            .withWordWrapWidth (width)
                                            .withHeight (height)
                                            .withMaxNumLines (1)
                                            .withJustification (layout)
                                            .withTrailingWhitespacesShouldFit (false) };

        const auto requiredWidths = st.getMinimumRequiredWidthForLines();

        if (requiredWidths.empty() || requiredWidths.front() <= width)
            return st;

        // If we can fit the entire line, squash by just enough and insert
        if (requiredWidths.front() * minimumRelativeHorizontalScale < width)
        {
            const auto requiredRelativeScale = width / (requiredWidths.front() + widthFittingTolerance);

            ShapedText squashed { trimmed,
                                  baseOptions
                                      .withFont (f.withHorizontalScale (f.getHorizontalScale() * requiredRelativeScale))
                                      .withWordWrapWidth (width)
                                      .withHeight (height)
                                      .withJustification (layout)
                                      .withTrailingWhitespacesShouldFit (false)};

            return squashed;
        }
    }

    const auto minimumHorizontalScale = minimumRelativeHorizontalScale * f.getHorizontalScale();

    if (maximumLines <= 1)
    {
        ShapedText squashed { trimmed,
                              baseOptions
                                  .withFont (f.withHorizontalScale (minimumHorizontalScale))
                                  .withWordWrapWidth (width)
                                  .withHeight (height)
                                  .withJustification (layout)
                                  .withMaxNumLines (1)
                                  .withEllipsis() };

        return squashed;
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

        const auto a = baseOptions.getAdditiveLineSpacing();
        auto newFontHeight = ((height + a) / (float) numLines - a)
                             / baseOptions.getLeading();

        if (newFontHeight < font.getHeight())
            font.setHeight (jmax (8.0f, newFontHeight));

        ShapedText squashed { trimmed,
                              baseOptions
                                  .withFont (font)
                                  .withWordWrapWidth (width)
                                  .withHeight (height)
                                  .withMaxNumLines (numLines)
                                  .withJustification (layout)
                                  .withTrailingWhitespacesShouldFit (false) };

        if (areAllRequiredWidthsSmallerThanMax (squashed, width))
            return squashed;

        const auto lineWidths = squashed.getMinimumRequiredWidthForLines();

        // We're trying to guess how much horizontal space the text would need to fit, and we
        // need to have an allowance for line end whitespaces which take up additional room
        // when not falling at the end of lines.
        cumulativeLineLengths = std::accumulate (lineWidths.begin(), lineWidths.end(), 0.0f)
                                + font.getHeight() * (float) numLines * 1.4f;

        if (newFontHeight < 8.0f)
            break;
    }

    //==============================================================================
    // We run an iterative interval halving algorithm to find the largest scale that can fit all
    // text
    auto makeShapedText = [&] (float horizontalScale)
    {
        return ShapedText { trimmed,
                            baseOptions
                                .withFont (font.withHorizontalScale (horizontalScale))
                                .withWordWrapWidth (width)
                                .withHeight (height)
                                .withMaxNumLines (numLines)
                                .withJustification (layout)
                                .withTrailingWhitespacesShouldFit (false)
                                .withEllipsis() };
    };

    auto lowerScaleBound = minimumHorizontalScale;
    auto upperScaleBound = jlimit (minimumHorizontalScale,
                                   1.0f,
                                   (float) numLines * width / cumulativeLineLengths);

    if (auto st = makeShapedText (upperScaleBound);
        areAllRequiredWidthsSmallerThanMax (st, width)
        || approximatelyEqual (upperScaleBound, minimumHorizontalScale))
    {
        return st;
    }

    struct Candidate
    {
        float scale{};
        ShapedText shapedText;
    };

    Candidate candidate { minimumHorizontalScale, makeShapedText (minimumHorizontalScale) };

    for (int i = 0, numApproximatingIterations = 3; i < numApproximatingIterations; ++i)
    {
        auto scale = jmap (0.5f, lowerScaleBound, upperScaleBound);

        if (auto st = makeShapedText (scale);
            areAllRequiredWidthsSmallerThanMax (st, width))
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

    const auto scalePerfectlyFittingTheLongestLine = [&]
    {
        const auto lineWidths = candidate.shapedText.getMinimumRequiredWidthForLines();
        const auto greatestLineWidth = std::accumulate (lineWidths.begin(),
                                                        lineWidths.end(),
                                                        0.0f,
                                                        [] (auto acc, auto w) { return std::max (acc, w); });

        if (exactlyEqual (greatestLineWidth, 0.0f))
            return candidate.scale;

        return jlimit (candidate.scale,
                       1.0f,
                       candidate.scale * width / (greatestLineWidth + widthFittingTolerance));
    }();

    if (candidate.scale < scalePerfectlyFittingTheLongestLine)
    {
        if (auto st = makeShapedText (scalePerfectlyFittingTheLongestLine);
            areAllRequiredWidthsSmallerThanMax (st, width))
        {
            candidate.scale = scalePerfectlyFittingTheLongestLine;
            candidate.shapedText = std::move (st);
        }
    }

    return candidate.shapedText;
}

static detail::ShapedText::Options withGlyphArrangementOptions (const detail::ShapedText::Options& opts,
                                                                const GlyphArrangementOptions& gaOpts)
{
    return opts.withAdditiveLineSpacing (gaOpts.getLineSpacing())
               .withLeading (gaOpts.getLineHeightMultiple());
}

void GlyphArrangement::addFittedText (const Font& f,
                                      const String& text,
                                      float x,
                                      float y,
                                      float width,
                                      float height,
                                      Justification layout,
                                      int maximumLines,
                                      float minimumHorizontalScale,
                                      GlyphArrangementOptions options)
{
    using namespace detail;

    const auto st = createFittedText (f,
                                      text,
                                      width,
                                      height,
                                      layout,
                                      maximumLines,
                                      minimumHorizontalScale,
                                      withGlyphArrangementOptions (ShapedText::Options{}, options));

    // ShapedText has the feature for visually truncating the last line, and createFittedText() uses
    // it. Hence if it's only the last line that requires a larger width, ShapedText will take care
    // of it. If lines other than the last one require more width than the minimum, it means they
    // contain a single unbreakable word, and the shaping needs to be redone with breaks inside
    // words allowed.
    if (areAllRequiredWidthsExceptTheLastSmallerThanMax (st, width))
    {
        addGlyphsFromShapedText (*this, st, x, y);
        return;
    }

    const auto stWithWordBreaks = createFittedText (f,
                                                    text,
                                                    width,
                                                    height,
                                                    layout,
                                                    maximumLines,
                                                    minimumHorizontalScale,
                                                    withGlyphArrangementOptions (ShapedText::Options{}.withAllowBreakingInsideWord(),
                                                                                 options));

    addGlyphsFromShapedText (*this, stWithWordBreaks, x, y);
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
