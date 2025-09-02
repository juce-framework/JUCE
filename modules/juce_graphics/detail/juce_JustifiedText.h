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

//==============================================================================
struct LineMetrics
{
    int64 lineNumber;
    Point<float> anchor;
    float maxAscent;
    float maxDescent;

    /*  Effective suggests the length of trailing whitespaces will be included or not depending on
        ShapedTextOptions::getTrailingWhitespacesShouldFit().
    */
    float effectiveLineLength;

    /*  These values seem redundant given the relation between the baseline, ascent and top, but
        we want to ensure exactlyEquals (top, nextLineTop) for subsequent lines.
    */
    float top;

    /*  This will be below the current line's visual bottom if non-default leading or additive
        line spacing is used.
    */
    float nextLineTop;
};

struct GlyphAnchorResult
{
    Point<float> anchor;
    float maxAscent{};
    float maxDescent{};
};

//==============================================================================
class JustifiedText
{
private:
    enum class DrawType
    {
        normal,
        ellipsis
    };

public:
    JustifiedText (const SimpleShapedText* t, const ShapedTextOptions& options);

    /*  Provides access to the data stored in the ShapedText.

        The provided callable will be called multiple times for "uniform glyph runs", for which all
        callback parameters are the same.

        Between each subsequent callback at least one of the provided parameters will be different.

        The callbacks happen in visual order i.e. left to right, which is irrespective of the
        underlying text's writing direction.

        The callback parameters in order are:
        - the glyphs
        - the positions for each glyph in the previous parameter
        - the Font with which these glyphs should be rendered
        - the range in all glyphs this ShapedText object holds, that correspond to the current glyphs
        - a line number which increases by one for each new line
        - followed by any number of ValueType parameters for the supplied RangedValues<ValueType> arguments
    */
    template <typename Callable, typename... RangedValues>
    void accessTogetherWith (Callable&& callback, RangedValues&&... rangedValues) const
    {
        std::optional<int64> lastLine;
        int64 lastGlyph = 0;
        Point<float> anchor {};

        for (const auto item : makeIntersectingRangedValues (&shapedText.getResolvedFonts(),
                                                             &lineMetricsForGlyphRange,
                                                             &rangesToDraw,
                                                             &whitespaceStretch,
                                                             (&rangedValues)...))
        {
            const auto& [range, font, lineMetrics, drawType, stretch] = partiallyUnpack<0, 5> (item);
            const auto& rest = partiallyUnpack<5, std::tuple_size_v<decltype (item)> - 5> (item);

            if (std::exchange (lastLine, lineMetrics.lineNumber) != lineMetrics.lineNumber)
                anchor = lineMetrics.anchor;

            if (range.getStart() != lastGlyph && drawType != DrawType::ellipsis)
            {
                detail::RangedValues<int8_t> glyphMask;
                Ranges::Operations ops;

                const auto firstGlyphInCurrentLine = shapedText.getLineNumbersForGlyphRanges().getItem ((size_t) lineMetrics.lineNumber)
                                                                                              .range
                                                                                              .getStart();

                glyphMask.set ({ std::max (lastGlyph, firstGlyphInCurrentLine), range.getStart() }, 1, ops);

                for (const auto [skippedRange, skippedStretch, _] : makeIntersectingRangedValues (&whitespaceStretch,
                                                                                                  &glyphMask))
                {
                    ignoreUnused (_);

                    for (const auto& skippedGlyph : shapedText.getGlyphs (skippedRange))
                    {
                        anchor += skippedGlyph.advance;

                        if (skippedGlyph.isWhitespace())
                            anchor.addXY (skippedStretch, 0.0f);
                    }
                }
            }

            lastGlyph = range.getEnd();

            const auto glyphs = [this, r = range, dt = drawType]() -> Span<const ShapedGlyph>
            {
                if (dt == DrawType::ellipsis)
                    return ellipsis->getGlyphs();

                return shapedText.getGlyphs (r);
            }();

            std::vector<Point<float>> positions (glyphs.size());

            std::transform (glyphs.begin(),
                            glyphs.end(),
                            positions.begin(),
                            [&anchor, &s = stretch] (auto& glyph)
                            {
                                auto result = anchor + glyph.offset;

                                anchor += glyph.advance;

                                if (glyph.isWhitespace())
                                    anchor.addXY (s, 0.0f);

                                return result;
                            });

            const auto callbackFont =
                drawType == DrawType::ellipsis ? ellipsis->getResolvedFonts().front().value : font;
            const auto callbackParameters =
                std::tuple_cat (std::tie (glyphs, positions, callbackFont, range, lineMetrics), rest);

            const auto invokeNullChecked = [&] (auto&... params)
            { NullCheckedInvocation::invoke (callback, params...); };

            std::apply (invokeNullChecked, callbackParameters);
        }
    }

    /*  This is how much cumulative widths glyphs take up in each line. Whether the trailing
        whitespace is included depends on the ShapedTextOptions::getWhitespaceShouldFitInLine()
        setting.
    */
    auto& getMinimumRequiredWidthForLines() const { return minimumRequiredWidthsForLine; }

    int64 getGlyphIndexToTheRightOf (Point<float> p) const;

    /*  If the passed in index parameter is greater than the index of the last contained glyph,
        then the returned anchor specifies the location where the next glyph would have to be
        placed i.e. lastGlyphAnchor + lastGlyphAdvance.
    */
    GlyphAnchorResult getGlyphAnchor (int64 glyphIndex) const;

    RectangleList<float> getGlyphsBounds (Range<int64> glyphRange) const;

    /*  Returns the vertical distance from the baseline of the first line to the bottom of the last
        plus any additional line spacing that follows from the leading and additiveLineSpacing
        members of the ShapedTextOptions object.

        This guarantees that if ShapedText object1 is drawn at y = 0 and object2 is drawn at
        y = object1.getHeight(), then the two texts will be spaced exactly as if they were a single
        ShapedText object containing both texts.
    */
    float getHeight() const;

    const auto& getLineMetricsForGlyphRange() const { return lineMetricsForGlyphRange; }

private:
    const SimpleShapedText& shapedText;
    detail::RangedValues<LineMetrics> lineMetricsForGlyphRange;
    std::optional<SimpleShapedText> ellipsis;
    detail::RangedValues<DrawType> rangesToDraw;
    detail::RangedValues<float> whitespaceStretch;
    std::vector<float> minimumRequiredWidthsForLine;
};

} // namespace juce::detail
