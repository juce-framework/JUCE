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

namespace juce::detail
{

void drawJustifiedText (const JustifiedText& text, const Graphics& g, AffineTransform);

//==============================================================================
static constexpr auto maxWidthTolerance = 0.005f;

static int64 getNumLeadingWhitespaces (Span<const ShapedGlyph> glyphs)
{
    const auto it = std::find_if_not (glyphs.begin(),
                                      glyphs.end(),
                                      [&] (const auto& g) { return g.isWhitespace(); });

    return (int64) std::distance (glyphs.begin(), it);
}

static int64 getNumTrailingWhitespaces (Span<const ShapedGlyph> glyphs)
{
    if (glyphs.empty())
        return 0;

    int64 trailingWhitespaces = 0;

    for (auto it = glyphs.end(); --it >= glyphs.begin() && it->isWhitespace();)
        ++trailingWhitespaces;

    return trailingWhitespaces;
}

struct NumWhitespaces
{
    int64 total{}, leading{}, trailing{};
};

static NumWhitespaces getNumWhitespaces (Span<const ShapedGlyph> glyphs)
{
    const auto total = std::count_if (glyphs.begin(),
                                      glyphs.end(),
                                      [] (const auto& g) { return g.isWhitespace(); });

    return { total, getNumLeadingWhitespaces (glyphs), getNumTrailingWhitespaces (glyphs) };
}

struct LineLength
{
    float total{}, withoutTrailingWhitespaces{};
};

static LineLength getMainAxisLineLength (Span<const ShapedGlyph> glyphs)
{
    const auto total = std::accumulate (glyphs.begin(),
                                        glyphs.end(),
                                        0.0f,
                                        [] (auto acc, const auto& g) { return acc + g.advance.getX(); });

    auto trailingWhitespacesLength = 0.0f;

    if (glyphs.empty())
        return {};

    for (auto it = glyphs.end(); --it >= glyphs.begin() && it->isWhitespace();)
        trailingWhitespacesLength += it->advance.getX();

    return { total, total - trailingWhitespacesLength };
}

static float getMainAxisLineLength (Span<const ShapedGlyph> glyphs, bool trailingWhitespacesShouldFit)
{
    const auto lengths = getMainAxisLineLength (glyphs);

    return trailingWhitespacesShouldFit ? lengths.total : lengths.withoutTrailingWhitespaces;
}

struct MainAxisLineAlignment
{
    float anchor{}, extraWhitespaceAdvance{}, effectiveLineLength{};
    Range<int64> stretchableWhitespaces;
};

static MainAxisLineAlignment getMainAxisLineAlignment (Justification justification,
                                                       Span<const ShapedGlyph> glyphs,
                                                       LineLength lineLength,
                                                       std::optional<float> maxWidthOpt,
                                                       std::optional<float> alignmentWidthOpt,
                                                       bool trailingWhitespacesShouldFit)
{
    const auto effectiveLineLength = (trailingWhitespacesShouldFit ? lineLength.total
                                                                   : lineLength.withoutTrailingWhitespaces);

    const auto alignmentWidth = alignmentWidthOpt.value_or (maxWidthOpt.value_or (0.0f));
    const auto tooLong = alignmentWidth + maxWidthTolerance < effectiveLineLength;

    MainAxisLineAlignment result;
    result.effectiveLineLength = effectiveLineLength;

    // The alignment width opt is supporting the TextEditor use-case where all text remains visible
    // with scrolling, even if longer than the alignment width. We don't need to worry about the
    // front of and RTL text being visually truncated, because nothing is truncated.
    if (tooLong && alignmentWidthOpt.has_value())
        return result;

    const auto mainAxisLineOffset = [&]
    {
        if (tooLong)
        {
            const auto approximateIsLeftToRight = [&]
            {
                if (glyphs.empty())
                    return true;

                return glyphs.front().cluster <= glyphs.back().cluster;
            }();

            // We don't have to align LTR text, but we need to ensure that it's the logical back of
            // RTL text that falls outside the bounds.
            if (approximateIsLeftToRight)
                return 0.0f;

            return alignmentWidth - effectiveLineLength;
        }

        if (justification.testFlags (Justification::horizontallyCentred))
        {
            return (alignmentWidth - lineLength.withoutTrailingWhitespaces) / 2.0f;
        }

        if (justification.testFlags (Justification::right))
            return alignmentWidth - effectiveLineLength;

        return 0.0f;
    }();

    const auto numWhitespaces = getNumWhitespaces (glyphs);

    const auto stretchableWhitespaces = [&]() -> Range<int64>
    {
        if (! justification.testFlags (Justification::horizontallyJustified) || tooLong)
            return {};

        return { numWhitespaces.leading, (int64) glyphs.size() - numWhitespaces.trailing };
    }();

    const auto extraWhitespaceAdvance = [&]
    {
        if (! justification.testFlags (Justification::horizontallyJustified) || tooLong)
            return 0.0f;

        const auto numWhitespacesBetweenWords = numWhitespaces.total
                                                - numWhitespaces.leading
                                                - numWhitespaces.trailing;

        return numWhitespacesBetweenWords > 0 ? (alignmentWidth - effectiveLineLength) / (float) numWhitespacesBetweenWords
                                              : 0.0f;
    }();

    return { mainAxisLineOffset, extraWhitespaceAdvance, effectiveLineLength, stretchableWhitespaces };
}

struct LineInfo
{
    float lineHeight{}, maxAscent{};
    MainAxisLineAlignment mainAxisLineAlignment;
};

static float getCrossAxisStartingAnchor (Justification justification,
                                         Span<const LineInfo> lineInfos,
                                         std::optional<float> height,
                                         float leadingInHeight)
{
    if (lineInfos.empty())
        return 0.0f;

    const auto minimumTop = lineInfos.front().maxAscent + lineInfos.front().lineHeight * leadingInHeight;

    if (! height.has_value())
        return minimumTop;

    const auto textHeight = std::accumulate (lineInfos.begin(),
                                             lineInfos.end(),
                                             0.0f,
                                             [] (auto acc, const auto info) { return acc + info.lineHeight; });

    if (justification.testFlags (Justification::verticallyCentred))
        return (*height - textHeight) / 2.0f + lineInfos.front().maxAscent;

    if (justification.testFlags (Justification::bottom))
    {
        const auto bottomLeading = 0.5f * lineInfos.back().lineHeight * leadingInHeight;
        return *height - textHeight - bottomLeading + lineInfos.front().maxAscent;
    }

    return minimumTop;
}

JustifiedText::JustifiedText (const SimpleShapedText* t, const ShapedTextOptions& options)
    : shapedText (*t)
{
    const auto leading = options.getLeading() - 1.0f;

    std::vector<LineInfo> lineInfos;

    for (const auto [range, lineNumber] : shapedText.getLineNumbersForGlyphRanges())
    {
        // This is guaranteed by the RangedValues implementation. You can't assign a value to an
        // empty range.
        jassert (! range.isEmpty());

        const auto fonts = shapedText.getResolvedFonts().getIntersectionsWith (range);

        const auto lineHeight = std::accumulate (fonts.begin(),
                                                 fonts.end(),
                                                 0.0f,
                                                 [] (auto acc, const auto& rangedFont)
                                                 { return std::max (acc, rangedFont.value.getHeight()); });

        const auto maxAscent = std::accumulate (fonts.begin(),
                                                fonts.end(),
                                                0.0f,
                                                [] (auto acc, const auto& rangedFont)
                                                { return std::max (acc, rangedFont.value.getAscent()); });

        const auto glyphs = shapedText.getGlyphs (range);
        const auto lineLength = getMainAxisLineLength (glyphs);

        auto m = [&]
        {
            return getMainAxisLineAlignment (options.getJustification(),
                                             glyphs,
                                             lineLength,
                                             options.getMaxWidth(),
                                             options.getAlignmentWidth(),
                                             options.getTrailingWhitespacesShouldFit());
        }();

        const auto containsHardBreak = shapedText.getCodepoint (range.getEnd() - 1) == 0xa
                                       || shapedText.getCodepoint (range.getStart()) == 0xa;

        if (containsHardBreak || lineNumber == shapedText.getLineNumbersForGlyphRanges().back().value)
        {
            m.extraWhitespaceAdvance = {};
            m.stretchableWhitespaces = {};
        }

        lineInfos.push_back ({ lineHeight, maxAscent, std::move (m) });
        minimumRequiredWidthsForLine.push_back (options.getTrailingWhitespacesShouldFit() ? lineLength.total
                                                                                          : lineLength.withoutTrailingWhitespaces);
    }

    auto baseline = options.isBaselineAtZero() ? 0.0f
                                               : getCrossAxisStartingAnchor (options.getJustification(),
                                                                             lineInfos,
                                                                             options.getHeight(),
                                                                             leading);

    detail::Ranges::Operations ops;

    std::optional<float> top;

    for (const auto [lineIndex, lineInfo] : enumerate (lineInfos))
    {
        const auto lineNumber = shapedText.getLineNumbersForGlyphRanges().getItem ((size_t) lineIndex);
        const auto range = lineNumber.range;

        const auto maxDescent = lineInfo.lineHeight - lineInfo.maxAscent;
        const auto nextLineTop = baseline + (1.0f + leading) * maxDescent + options.getAdditiveLineSpacing();

        if (! top.has_value())
            top = baseline - (1.0f + leading) * lineInfo.maxAscent;

        lineMetricsForGlyphRange.set (range,
                                      { lineNumber.value,
                                        { lineInfo.mainAxisLineAlignment.anchor, baseline },
                                        lineInfo.maxAscent,
                                        lineInfo.lineHeight - lineInfo.maxAscent,
                                        lineInfo.mainAxisLineAlignment.effectiveLineLength
                                            + lineInfo.mainAxisLineAlignment.extraWhitespaceAdvance,
                                        *top,
                                        nextLineTop },
                                      ops,
                                      MergeEqualItemsNo{});

        whitespaceStretch.set (range, 0.0f, ops);
        const auto stretchRange = lineInfo.mainAxisLineAlignment.stretchableWhitespaces + range.getStart();

        whitespaceStretch.set (stretchRange,
                               lineInfo.mainAxisLineAlignment.extraWhitespaceAdvance,
                               ops);

        ops.clear();

        const auto nextLineMaxAscent = lineIndex < (int) lineInfos.size() - 1 ? lineInfos[(size_t) lineIndex + 1].maxAscent : 0.0f;
        baseline = nextLineTop + (1.0f + leading) * nextLineMaxAscent;
        top = nextLineTop;
    }

    rangesToDraw.set ({ 0, (int64) shapedText.getGlyphs().size() }, DrawType::normal, ops);

    //==============================================================================
    // Everything above this line should work well given none of the lines were too
    // long. When Options::getMaxNumLines() == 0 this is guaranteed by SimpleShapedText.
    // The remaining logic below is for supporting
    // GlyphArrangement::addFittedText() when the maximum number of lines is
    // constrained.
    if (lineMetricsForGlyphRange.isEmpty())
        return;

    const auto lastLineMetrics = lineMetricsForGlyphRange.back();
    const auto lastLineGlyphRange = lastLineMetrics.range;
    const auto lastLineGlyphs = shapedText.getGlyphs (lastLineGlyphRange);
    const auto lastLineLengths = getMainAxisLineLength (lastLineGlyphs);

    const auto effectiveLength = options.getTrailingWhitespacesShouldFit() ? lastLineLengths.total
                                                                           : lastLineLengths.withoutTrailingWhitespaces;

    if (! options.getMaxWidth().has_value()
        || effectiveLength <= *options.getMaxWidth() + maxWidthTolerance)
        return;

    const auto cutoffAtFront = lastLineMetrics.value.anchor.getX() < 0.0f - maxWidthTolerance;

    const auto getLastLineVisibleRange = [&] (float ellipsisLength)
    {
        const auto r = [&]() -> Range<int64>
        {
            if (cutoffAtFront)
            {
                auto length = lastLineLengths.total;

                for (auto it = lastLineGlyphs.begin(); it < lastLineGlyphs.end(); ++it)
                {
                    length -= it->advance.getX();

                    if (! options.getMaxWidth().has_value()
                        || *options.getMaxWidth() >= ellipsisLength + length)
                    {
                        return { (int64) std::distance (lastLineGlyphs.begin(), it) + 1,
                                 (int64) lastLineGlyphs.size() };
                    }
                }
            }
            else
            {
                auto length = lastLineLengths.total;

                for (auto it = lastLineGlyphs.end() - 1; it >= lastLineGlyphs.begin(); --it)
                {
                    length -= it->advance.getX();

                    if (! options.getMaxWidth().has_value()
                        || *options.getMaxWidth() >= ellipsisLength + length)
                    {
                        return { 0, (int64) std::distance (lastLineGlyphs.begin(), it) };
                    }
                }
            }

            return {};
        }();

        return r.movedToStartAt (r.getStart() + lastLineGlyphRange.getStart());
    };

    const auto lastLineVisibleRangeWithoutEllipsis = getLastLineVisibleRange (0.0f);

    const auto eraseLastLineFromRangesToDraw = [&]
    {
        rangesToDraw.eraseFrom (lastLineGlyphRange.getStart(), ops);
        ops.clear();
    };

    eraseLastLineFromRangesToDraw();
    rangesToDraw.set (lastLineVisibleRangeWithoutEllipsis, DrawType::normal, ops);
    ops.clear();

    if (options.getEllipsis().isEmpty())
    {
        return;
    }

    //==============================================================================
    //  More logic supporting using ellipses
    const auto fontForEllipsis = [&]
    {
        const auto lastLineFonts = shapedText.getResolvedFonts().getIntersectionsWith (lastLineGlyphRange);

        if (cutoffAtFront)
            return lastLineFonts.front().value;

        return lastLineFonts.back().value;
    }();

    ellipsis.emplace (&options.getEllipsis(), ShapedTextOptions {}.withFont (fontForEllipsis));

    const auto lastLineVisibleRange = getLastLineVisibleRange (getMainAxisLineLength (ellipsis->getGlyphs(),
                                                                                      options.getTrailingWhitespacesShouldFit()));

    eraseLastLineFromRangesToDraw();
    rangesToDraw.set (lastLineVisibleRange, DrawType::normal, ops);
    ops.clear();

    if (cutoffAtFront)
        rangesToDraw.set (Range<int64>::withStartAndLength (lastLineVisibleRange.getStart() - 1, 1), DrawType::ellipsis, ops);
    else
        rangesToDraw.set (Range<int64>::withStartAndLength (lastLineVisibleRange.getEnd(), 1), DrawType::ellipsis, ops);

    ops.clear();

    const auto lineWithEllipsisGlyphs = [&]
    {
        std::vector<ShapedGlyph> result;

        const auto pushEllipsisGlyphs = [&]
        {
            const auto& range = ellipsis->getGlyphs();
            result.insert (result.begin(), range.begin(), range.end());
        };

        if (cutoffAtFront)
            pushEllipsisGlyphs();

        const auto& range = shapedText.getGlyphs (lastLineVisibleRange);
        result.insert (result.end(), range.begin(), range.end());

        if (! cutoffAtFront)
            pushEllipsisGlyphs();

        return result;
    }();

    const auto realign = [&]
    {
        return getMainAxisLineAlignment (options.getJustification(),
                                         lineWithEllipsisGlyphs,
                                         getMainAxisLineLength (lineWithEllipsisGlyphs),
                                         options.getMaxWidth(),
                                         options.getAlignmentWidth(),
                                         options.getTrailingWhitespacesShouldFit());
    }();

    lastLineMetrics.value.anchor.setX (realign.anchor);
    whitespaceStretch.set (lastLineGlyphRange, 0.0f, ops);
    whitespaceStretch.set (realign.stretchableWhitespaces + lastLineVisibleRange.getStart(),
                           realign.extraWhitespaceAdvance, ops);
}

int64 JustifiedText::getGlyphIndexToTheRightOf (Point<float> p) const
{
    auto lineIt = lineMetricsForGlyphRange.begin();
    float lineTop = 0.0f;

    while (lineIt != lineMetricsForGlyphRange.end())
    {
        const auto nextLineTop = lineIt->value.nextLineTop;

        if (lineTop <= p.getY() && p.getY() < nextLineTop)
            break;

        lineTop = nextLineTop;
        ++lineIt;
    }

    if (lineIt == lineMetricsForGlyphRange.end())
        return 0;

    const auto glyphsInLine = shapedText.getGlyphs (lineIt->range);

    auto glyphIndex = lineIt->range.getStart();
    auto glyphX = lineIt->value.anchor.getX();

    for (const auto& glyph : glyphsInLine)
    {
        if (   p.getX() < glyphX + glyph.advance.getX() / 2.0f
            || glyph.isNewline()
            || (glyphIndex - lineIt->range.getStart() == (int64) glyphsInLine.size() - 1 && glyph.isWhitespace()))
        {
            break;
        }

        ++glyphIndex;
        glyphX += glyph.advance.getX();
    }

    return glyphIndex;
}

GlyphAnchorResult JustifiedText::getGlyphAnchor (int64 index) const
{
    jassert (index >= 0);

    if (lineMetricsForGlyphRange.isEmpty())
        return {};

    const auto lineItem = lineMetricsForGlyphRange.getItemWithEnclosingRange (index)
                                                  .value_or (lineMetricsForGlyphRange.back());

    const auto indexInLine = index - lineItem.range.getStart();

    GlyphAnchorResult anchor { lineItem.value.anchor, lineItem.value.maxAscent, lineItem.value.maxDescent };

    for (auto [i, glyph] : enumerate (shapedText.getGlyphs (lineItem.range), int64{}))
    {
        if (i == indexInLine)
        {
            anchor.anchor += glyph.offset;
            return anchor;
        }

        anchor.anchor += glyph.advance;
    }

    return anchor;
}

RectangleList<float> JustifiedText::getGlyphsBounds (Range<int64> glyphRange) const
{
    RectangleList<float> bounds;

    if (lineMetricsForGlyphRange.isEmpty())
        return bounds;

    const auto getBounds = [&] (const LineMetrics& line, int64 lineStart, int64 boundsStart, int64 boundsEnd) -> Rectangle<float>
    {
        const auto glyphsBefore = shapedText.getGlyphs ({ lineStart, boundsStart });

        const auto xStart = std::accumulate (glyphsBefore.begin(),
                                             glyphsBefore.end(),
                                             line.anchor.getX(),
                                             [] (auto sum, auto glyph)
                                             {
                                                 return sum + glyph.advance.getX();
                                             });

        const auto glyphs = shapedText.getGlyphs ({ boundsStart, boundsEnd });

        const auto xEnd = std::accumulate (glyphs.begin(),
                                           glyphs.end(),
                                           xStart,
                                           [&] (auto sum, auto glyph)
                                           {
                                               return sum + glyph.advance.getX();
                                           });

        return { { xStart, line.top }, { xEnd, line.nextLineTop } };
    };

    for (auto consumeFrom = glyphRange.getStart(); consumeFrom < glyphRange.getEnd();)
    {
        const auto lineItem = lineMetricsForGlyphRange.getItemWithEnclosingRange (consumeFrom);

        if (! lineItem.has_value())
            break;

        const auto consumeTo = std::min (glyphRange.getEnd(), lineItem->range.getEnd());
        bounds.add (getBounds (lineItem->value, lineItem->range.getStart(), consumeFrom, consumeTo));

        consumeFrom = consumeTo;
    }

    return bounds;
}

float JustifiedText::getHeight() const
{
    if (lineMetricsForGlyphRange.isEmpty())
        return 0.0f;

    return lineMetricsForGlyphRange.back().value.nextLineTop;
}

void drawJustifiedText (const JustifiedText& text, const Graphics& g, AffineTransform transform)
{
    auto& context = g.getInternalContext();
    context.saveState();
    const ScopeGuard restoreGraphicsContext { [&context] { context.restoreState(); } };

    text.accessTogetherWith ([&] (auto glyphs, auto positions, auto font, auto, auto)
                             {
                                 if (context.getFont() != font)
                                     context.setFont (font);

                                 std::vector<uint16_t> glyphIds (glyphs.size());

                                 std::transform (glyphs.begin(),
                                                 glyphs.end(),
                                                 glyphIds.begin(),
                                                 [] (auto& glyph) { return (uint16_t) glyph.glyphId; });

                                 context.drawGlyphs (glyphIds, positions, transform);
                             });
}

} // namespace juce::detail
