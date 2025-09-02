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

/*  Class that can visually shape a Unicode string provided a list of Fonts corresponding to
    sub-ranges of the string.
*/
class JUCE_API  ShapedText
{
public:
    using Options = ShapedTextOptions;

    ShapedText();

    explicit ShapedText (String text);

    ShapedText (String text, Options options);

    /*  Returns the text which was used to construct this object. */
    const String& getText() const;

    Span<const ShapedGlyph> getGlyphs() const;

    /*  Returns the text's codepoint range, to which the glyph under the provided index belongs.

        This range will have a length of at least one, and potentially more than one if ligatures
        are enabled.
    */
    Range<int64> getTextRange (int64 glyphIndex) const;

    bool isLtr (int64 glyphIndex) const;

    int64 getTextIndexForCaret (Point<float> p) const;

    void getGlyphRanges (Range<int64> textRange, std::vector<Range<int64>>& outRanges) const;

    RectangleList<float> getGlyphsBounds (Range<int64> glyphRange) const;

    /*  @see JustifiedText::getGlyphAnchor() */
    GlyphAnchorResult getGlyphAnchor (int64 index) const;

    /*  Returns the widths for each line, that the glyphs would require to be rendered without being
        truncated. This will or will not include the space required by trailing whitespaces in the
        line based on the ShapedTextOptions::withTrailingWhitespacesShouldFit() value.

        This value isn't affected by the Justification parameter, it just reports the amount of
        width that would be required to avoid truncation.
     */
    Span<const float> getMinimumRequiredWidthForLines() const;

    /*  @see JustifiedText::accessTogetherWith */
    template <typename Callable, typename... RangedValues>
    void accessTogetherWith (Callable&& callback, RangedValues&&... rangedValues) const
    {
        getJustifiedText().accessTogetherWith (std::forward<Callable> (callback),
                                               std::forward<RangedValues> (rangedValues)...);
    }

    /*  Draws the text. */
    void draw (const Graphics& g, AffineTransform transform) const;

    /*  @see JustifiedText::getHeight
    */
    float getHeight() const;

    int64 getNumGlyphs() const;

    const detail::RangedValues<LineMetrics>& getLineMetricsForGlyphRange() const;

    const detail::Ranges& getLineTextRanges() const;

    /*  @internal */
    const JustifiedText& getJustifiedText() const;

    /*  @internal */
    const SimpleShapedText& getSimpleShapedText() const;

private:
    class Impl;
    std::shared_ptr<Impl> impl;
};

} // namespace juce::detail
