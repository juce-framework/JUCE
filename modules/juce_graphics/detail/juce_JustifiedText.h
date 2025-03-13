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
    void accessTogetherWith (Callable&& callback, RangedValues&&... rangedValues) const
    {
        std::optional<int64> lastLine;
        Point<float> anchor {};

        for (const auto item : makeIntersectingRangedValues (&shapedText.getLineNumbers(),
                                                             &shapedText.getResolvedFonts(),
                                                             &lineAnchors,
                                                             &rangesToDraw,
                                                             &whitespaceStretch,
                                                             (&rangedValues)...))
        {
            const auto& [range, line, font, lineAnchor, drawType, stretch] = partiallyUnpack<0, 6> (item);
            const auto& rest = partiallyUnpack<6, std::tuple_size_v<decltype (item)> - 6> (item);

            if (std::exchange (lastLine, line) != line)
                anchor = lineAnchor;

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

                                if (glyph.whitespace)
                                    anchor.addXY (s, 0.0f);

                                return result;
                            });

            const auto callbackFont =
                drawType == DrawType::ellipsis ? ellipsis->getResolvedFonts().front().value : font;
            const auto callbackParameters =
                std::tuple_cat (std::tie (glyphs, positions, callbackFont, range, line), rest);

            const auto invokeNullChecked = [&] (auto&... params)
            { NullCheckedInvocation::invoke (callback, params...); };

            std::apply (invokeNullChecked, callbackParameters);
        }
    }

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

} // namespace juce::detail
