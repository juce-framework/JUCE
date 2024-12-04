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

template <size_t StartingAt, typename Tuple, size_t... Is>
constexpr auto partiallyUnpackImpl (Tuple&& tuple, std::index_sequence<Is...>)
{
    return std::tie (std::get<StartingAt + Is> (tuple)...);
}

template <size_t StartingAt, size_t NumElems, typename Tuple>
constexpr auto partiallyUnpack (Tuple&& tuple)
{
    return partiallyUnpackImpl<StartingAt> (std::forward<Tuple> (tuple), std::make_index_sequence<NumElems>{});
}

class JustifiedText
{
private:
    enum class DrawType
    {
        normal,
        ellipsis
    };

public:
    JustifiedText (const SimpleShapedText& t, const ShapedTextOptions& options);

    template <typename Callable, typename... RangedValues>
    void accessTogetherWith (Callable&& callback, RangedValues&&... rangedValues) const;

    /* The callback receives (Span<const ShapedGlyph> glyphs,
                              Span<Point<float>> positions,
                              Font font,
                              Range<int64> glyphRange,
                              int64 lineNumber) // So far this has been indexed from 0 per SimpleShapedText
                                                // object, but maybe we'll find we want global text level
                                                // line numbers, so only assume they are increasing by one
    */
    template <typename Callable>
    void access (Callable&& callback) const;

    /*  This is how much cumulative widths glyphs take up in each line. Whether the trailing
        whitespace is included depends on the ShapedTextOptions::getWhitespaceShouldFitInLine()
        setting.
    */
    auto& getMinimumRequiredWidthForLines() const { return minimumRequiredWidthsForLine; }

private:
    const SimpleShapedText& shapedText;
    detail::RangedValues<Point<float>> lineAnchors;
    std::optional<SimpleShapedText> ellipsis;
    detail::RangedValues<DrawType> rangesToDraw;
    detail::RangedValues<float> whitespaceStretch;
    std::vector<float> minimumRequiredWidthsForLine;
};

void drawJustifiedText (const JustifiedText& text, const Graphics& g, AffineTransform);

//==============================================================================
static constexpr auto maxWidthTolerance = 0.005f;

static int64 getNumLeadingWhitespaces (Span<const ShapedGlyph> glyphs)
{
    const auto it = std::find_if_not (glyphs.begin(),
                                      glyphs.end(),
                                      [&] (const auto& g) { return g.whitespace; });

    return (int64) std::distance (glyphs.begin(), it);
}

static int64 getNumTrailingWhitespaces (Span<const ShapedGlyph> glyphs)
{
    if (glyphs.empty())
        return 0;

    int64 trailingWhitespaces = 0;

    for (auto it = glyphs.end(); --it >= glyphs.begin() && it->whitespace;)
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
                                      [] (const auto& g) { return g.whitespace; });

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

    for (auto it = glyphs.end(); --it >= glyphs.begin() && it->whitespace;)
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
    float anchor{}, extraWhitespaceAdvance{};
    Range<int64> stretchableWhitespaces;
};

static MainAxisLineAlignment getMainAxisLineAlignment (Justification justification,
                                                       Span<const ShapedGlyph> glyphs,
                                                       LineLength lineLength,
                                                       float maxWidth,
                                                       bool trailingWhitespacesShouldFit)
{
    const auto effectiveLineLength = (trailingWhitespacesShouldFit ? lineLength.total
                                                                   : lineLength.withoutTrailingWhitespaces);

    const auto tooLong = maxWidth + maxWidthTolerance < effectiveLineLength;

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

            if (approximateIsLeftToRight)
                return 0.0f;

            return maxWidth - effectiveLineLength;
        }

        if (justification.testFlags (Justification::horizontallyCentred))
        {
            return (maxWidth - lineLength.withoutTrailingWhitespaces) / 2.0f;
        }

        if (justification.testFlags (Justification::right))
            return maxWidth - effectiveLineLength;

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

        return numWhitespacesBetweenWords > 0 ? (maxWidth - effectiveLineLength) / (float) numWhitespacesBetweenWords
                                              : 0.0f;
    }();

    return { mainAxisLineOffset, extraWhitespaceAdvance, stretchableWhitespaces };
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

JustifiedText::JustifiedText (const SimpleShapedText& t, const ShapedTextOptions& options)
    : shapedText (t)
{
    const auto leading = options.getLeading() - 1.0f;

    std::vector<LineInfo> lineInfos;

    for (const auto [range, lineNumber] : shapedText.getLineNumbers())
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
            if (! options.getMaxWidth().has_value())
                return MainAxisLineAlignment{};

            return getMainAxisLineAlignment (options.getJustification(),
                                             glyphs,
                                             lineLength,
                                             *options.getMaxWidth(),
                                             options.getTrailingWhitespacesShouldFit());
        }();

        const auto containsHardBreak = shapedText.getCodepoint (range.getEnd() - 1) == 0xa
                                       || shapedText.getCodepoint (range.getStart()) == 0xa;

        if (containsHardBreak || lineNumber == shapedText.getLineNumbers().back().value)
        {
            m.extraWhitespaceAdvance = {};
            m.stretchableWhitespaces = {};
        }

        lineInfos.push_back ({ lineHeight, maxAscent, std::move (m) });
        minimumRequiredWidthsForLine.push_back (options.getTrailingWhitespacesShouldFit() ? lineLength.total
                                                                                          : lineLength.withoutTrailingWhitespaces);
    }

    auto y = options.isBaselineAtZero() ? 0.0f
                                        : getCrossAxisStartingAnchor (options.getJustification(),
                                                                      lineInfos,
                                                                      options.getHeight(),
                                                                      leading);

    for (const auto [lineIndex, lineInfo] : enumerate (lineInfos))
    {
        const auto range = shapedText.getLineNumbers().getItem ((size_t) lineIndex).range;

        lineAnchors.set<detail::MergeEqualItems::no> (range,
                                                      { lineInfo.mainAxisLineAlignment.anchor, y });

        whitespaceStretch.set (range, 0.0f);
        const auto stretchRange = lineInfo.mainAxisLineAlignment.stretchableWhitespaces + range.getStart();

        whitespaceStretch.set (stretchRange,
                               lineInfo.mainAxisLineAlignment.extraWhitespaceAdvance);

        const auto maxDescent = lineInfo.lineHeight - lineInfo.maxAscent;
        const auto nextLineMaxAscent = lineIndex < (int) lineInfos.size() - 1 ? lineInfos[(size_t) lineIndex + 1].maxAscent : 0.0f;

        y += (1.0f + leading) * (maxDescent + nextLineMaxAscent) + options.getAdditiveLineSpacing();
    }

    rangesToDraw.set ({ 0, (int64) shapedText.getGlyphs().size() }, DrawType::normal);

    //==============================================================================
    // Everything above this line should work well given none of the lines were too
    // long. When Options::getMaxNumLines() == 0 this is guaranteed by SimpleShapedText.
    // The remaining logic below is for supporting
    // GlyphArrangement::addFittedText() when the maximum number of lines is
    // constrained.
    if (lineAnchors.isEmpty())
        return;

    const auto lastLineAlignment = lineAnchors.back();
    const auto lastLineGlyphRange = lastLineAlignment.range;
    const auto lastLineGlyphs = shapedText.getGlyphs (lastLineGlyphRange);
    const auto lastLineLengths = getMainAxisLineLength (lastLineGlyphs);

    const auto effectiveLength = options.getTrailingWhitespacesShouldFit() ? lastLineLengths.total
                                                                           : lastLineLengths.withoutTrailingWhitespaces;

    if (! options.getMaxWidth().has_value()
        || effectiveLength <= *options.getMaxWidth() + maxWidthTolerance)
        return;

    const auto cutoffAtFront = lastLineAlignment.value.getX() < 0.0f - maxWidthTolerance;

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
        rangesToDraw.eraseFrom (lastLineGlyphRange.getStart());
    };

    eraseLastLineFromRangesToDraw();
    rangesToDraw.set (lastLineVisibleRangeWithoutEllipsis, DrawType::normal);

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
    rangesToDraw.set (lastLineVisibleRange, DrawType::normal);

    if (cutoffAtFront)
        rangesToDraw.set (Range<int64>::withStartAndLength (lastLineVisibleRange.getStart() - 1, 1), DrawType::ellipsis);
    else
        rangesToDraw.set (Range<int64>::withStartAndLength (lastLineVisibleRange.getEnd(), 1), DrawType::ellipsis);

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
        if (! options.getMaxWidth().has_value())
            return MainAxisLineAlignment{};

        return getMainAxisLineAlignment (options.getJustification(),
                                         lineWithEllipsisGlyphs,
                                         getMainAxisLineLength (lineWithEllipsisGlyphs),
                                         *options.getMaxWidth(),
                                         options.getTrailingWhitespacesShouldFit());
    }();

    lastLineAlignment.value.setX (realign.anchor);
    whitespaceStretch.set (lastLineGlyphRange, 0.0f);
    whitespaceStretch.set (realign.stretchableWhitespaces + lastLineVisibleRange.getStart(),
                           realign.extraWhitespaceAdvance);
}

template <typename Callable, typename... RangedValues>
void JustifiedText::accessTogetherWith (Callable&& callback, RangedValues&&... rangedValues) const
{
    std::optional<int64> lastLine;
    Point<float> anchor{};

    for (const auto item : makeIntersectingRangedValues (&shapedText.getLineNumbers(),
                                                         &shapedText.getResolvedFonts(),
                                                         &lineAnchors,
                                                         &rangesToDraw,
                                                         &whitespaceStretch,
                                                         (&rangedValues)...))
    {
        const auto& [range, line, font, lineAnchor, drawType, stretch] = partiallyUnpack<0, 6> (item);
        const auto& rest = partiallyUnpack<6, std::tuple_size_v<decltype(item)> - 6> (item);

        if (std::exchange (lastLine, line) != line)
            anchor = lineAnchor;

        const auto glyphs = [this, r = range, dt = drawType]() -> Span<const ShapedGlyph>
        {
            if (dt == DrawType::ellipsis)
                return ellipsis->getGlyphs();

            return shapedText.getGlyphs (r);
        }();

        std::vector<Point<float>> positions (glyphs.size());

        std::transform (glyphs.begin(), glyphs.end(), positions.begin(), [&anchor, &s = stretch] (auto& glyph)
                        {
                            auto result = anchor + glyph.offset;

                            anchor += glyph.advance;

                            if (glyph.whitespace)
                                anchor.addXY (s, 0.0f);

                            return result;
                        });

        const auto callbackFont = drawType == DrawType::ellipsis ? ellipsis->getResolvedFonts().front().value : font;
        const auto callbackParameters = std::tuple_cat (std::tie (glyphs, positions, callbackFont, range, line), rest);

        const auto invokeNullChecked = [&] (auto&... params)
        {
            NullCheckedInvocation::invoke (callback, params...);
        };

        std::apply (invokeNullChecked, callbackParameters);
    }
}

template <typename Callable>
void JustifiedText::access (Callable&& callback) const
{
    accessTogetherWith (std::forward<Callable> (callback));
}

void drawJustifiedText (const JustifiedText& text, const Graphics& g, AffineTransform transform)
{
    auto& context = g.getInternalContext();
    context.saveState();
    const ScopeGuard restoreGraphicsContext { [&context] { context.restoreState(); } };

    text.access ([&] (auto glyphs, auto positions, auto font, auto, auto)
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

} // namespace juce
