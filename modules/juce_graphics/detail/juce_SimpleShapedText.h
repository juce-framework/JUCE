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

/** Types of text direction. This may also be applied to characters. */
enum class TextDirection
{
    ltr, // This text reads left to right.
    rtl  // This text reads right to left.
};

class ShapedTextOptions
{
private:
    auto tie() const
    {
        return std::tie (justification,
                         readingDir,
                         wordWrapWidth,
                         alignmentWidth,
                         height,
                         fontsForRange,
                         language,
                         firstLineIndent,
                         leading,
                         additiveLineSpacing,
                         baselineAtZero,
                         allowBreakingInsideWord,
                         trailingWhitespacesShouldFit,
                         maxNumLines,
                         ellipsis);
    }

public:
    bool operator== (const ShapedTextOptions& other) const { return tie() == other.tie(); }
    bool operator!= (const ShapedTextOptions& other) const { return tie() != other.tie(); }

    //==============================================================================
    [[nodiscard]] ShapedTextOptions withJustification (Justification x) const
    {
        return withMember (*this, &ShapedTextOptions::justification, x);
    }

    /*  This option will use soft wrapping for lines that are longer than the specified value,
        and it will also align each line to this width, using the Justification provided in
        withJustification.

        The alignment width can be overriden using withAlignmentWidth, but currently we only need
        to do this for the TextEditor.
    */
    [[nodiscard]] ShapedTextOptions withWordWrapWidth (float x) const
    {
        return withMember (*this, &ShapedTextOptions::wordWrapWidth, x);
    }

    /*  With this option each line will be aligned only if it's shorter or equal to the alignment
        width. Otherwise, the line's x anchor will be 0.0f. This is in contrast to using
        withWordWrapWidth only, which will modify the x anchor of RTL lines that are too long, to ensure
        that it's the logical end of the text that falls outside the visible bounds.

        The alignment width is also a distinct value from the value used for soft wrapping which is
        specified using withWordWrapWidth.

        This option is specifically meant to support an existing TextEditor behaviour, where text
        can be aligned even when word wrapping is off. You probably don't need to use this function,
        unless you want to reproduce the particular behaviour seen in the TextEditor, and should
        only use withWordWrapWidth, if alignment is required.

        With this option off, text is either not aligned, or aligned to the width specified using
        withWordWrapWidth.

        When this option is in use, it overrides the width specified in withWordWrapWidth for alignment
        purposes, but not for line wrapping purposes.

        It also accommodates the fact that the TextEditor has a scrolling feature and text never
        becomes unreachable, even if the lines are longer than the viewport's width.
    */
    [[nodiscard]] ShapedTextOptions withAlignmentWidth (float x) const
    {
        return withMember (*this, &ShapedTextOptions::alignmentWidth, x);
    }

    [[nodiscard]] ShapedTextOptions withHeight (float x) const
    {
        return withMember (*this, &ShapedTextOptions::height, x);
    }

    [[nodiscard]] ShapedTextOptions withFont (Font x) const
    {
        RangedValues<Font> fonts;
        detail::Ranges::Operations ops;
        fonts.set ({ 0, std::numeric_limits<int64>::max() }, x, ops);

        return withMember (*this, &ShapedTextOptions::fontsForRange, std::move (fonts));
    }

    [[nodiscard]] ShapedTextOptions withFonts (const detail::RangedValues<Font>& x) const
    {
        return withMember (*this, &ShapedTextOptions::fontsForRange, x);
    }

    [[nodiscard]] ShapedTextOptions withLanguage (StringRef x) const
    {
        return withMember (*this, &ShapedTextOptions::language, x);
    }

    [[nodiscard]] ShapedTextOptions withFirstLineIndent (float x) const
    {
        return withMember (*this, &ShapedTextOptions::firstLineIndent, x);
    }

    /*  This controls the space between lines using a proportional value, with a default of 1.0,
        meaning single line spacing i.e. the descender of the current line + ascender of the next
        line. This value is multiplied by the leading provided here.
    */
    [[nodiscard]] ShapedTextOptions withLeading (float x) const
    {
        return withMember (*this, &ShapedTextOptions::leading, x);
    }

    /*  This controls the space between lines using an additive absolute value, with a default of 0.0.
        This value is added to the spacing between each two lines.
    */
    [[nodiscard]] ShapedTextOptions withAdditiveLineSpacing (float x) const
    {
        return withMember (*this, &ShapedTextOptions::additiveLineSpacing, x);
    }

    [[nodiscard]] ShapedTextOptions withBaselineAtZero (bool x = true) const
    {
        return withMember (*this, &ShapedTextOptions::baselineAtZero, x);
    }

    [[nodiscard]] ShapedTextOptions withTrailingWhitespacesShouldFit (bool x = true) const
    {
        return withMember (*this, &ShapedTextOptions::trailingWhitespacesShouldFit, x);
    }

    [[nodiscard]] ShapedTextOptions withMaxNumLines (int64 x) const
    {
        return withMember (*this, &ShapedTextOptions::maxNumLines, x);
    }

    [[nodiscard]] ShapedTextOptions withEllipsis (String x = String::charToString ((juce_wchar) 0x2026)) const
    {
        return withMember (*this, &ShapedTextOptions::ellipsis, std::move (x));
    }

    /*  Draw each line in its entirety even if it goes beyond wordWrapWidth. This means that even
        if configured, an ellipsis will never be inserted.

        This is used by the TextEditor where the Viewport guarantees that all text will be viewable
        even beyond the word wrap width.
    */
    [[nodiscard]] ShapedTextOptions withDrawLinesInFull (bool x = true) const
    {
        return withMember (*this, &ShapedTextOptions::drawLinesInFull, std::move (x));
    }

    [[nodiscard]] ShapedTextOptions withReadingDirection (std::optional<TextDirection> x) const
    {
        return withMember (*this, &ShapedTextOptions::readingDir, x);
    }

    [[nodiscard]] ShapedTextOptions withAllowBreakingInsideWord (bool x = true) const
    {
        return withMember (*this, &ShapedTextOptions::allowBreakingInsideWord, x);
    }

    const auto& getReadingDirection() const             { return readingDir; }
    const auto& getJustification() const                { return justification; }
    const auto& getWordWrapWidth() const                { return wordWrapWidth; }
    const auto& getAlignmentWidth() const               { return alignmentWidth; }
    const auto& getHeight() const                       { return height; }
    const auto& getFontsForRange() const                { return fontsForRange; }
    const auto& getLanguage() const                     { return language; }
    const auto& getFirstLineIndent() const              { return firstLineIndent; }
    const auto& getLeading() const                      { return leading; }
    const auto& getAdditiveLineSpacing() const          { return additiveLineSpacing; }
    const auto& isBaselineAtZero() const                { return baselineAtZero; }
    const auto& getTrailingWhitespacesShouldFit() const { return trailingWhitespacesShouldFit; }
    const auto& getMaxNumLines() const                  { return maxNumLines; }
    const auto& getEllipsis() const                     { return ellipsis; }
    const auto& getDrawLinesInFull() const              { return drawLinesInFull; }
    const auto& getAllowBreakingInsideWord() const      { return allowBreakingInsideWord; }

private:
    Justification justification { Justification::topLeft };
    std::optional<TextDirection> readingDir;
    std::optional<float> wordWrapWidth;
    std::optional<float> alignmentWidth;
    std::optional<float> height;

    detail::RangedValues<Font> fontsForRange = std::invoke ([&]
    {
        detail::RangedValues<Font> result;
        detail::Ranges::Operations ops;
        result.set ({ 0, std::numeric_limits<int64>::max() }, FontOptions { 15.0f }, ops);
        return result;
    });

    String language = SystemStats::getDisplayLanguage();
    float firstLineIndent = 0.0f;
    float leading = 1.0f;
    float additiveLineSpacing = 0.0f;
    bool baselineAtZero = false;
    bool allowBreakingInsideWord = false;
    bool trailingWhitespacesShouldFit = true;
    bool drawLinesInFull = false;
    int64 maxNumLines = std::numeric_limits<int64>::max();
    String ellipsis;
};

struct ShapedGlyph
{
    ShapedGlyph (uint32_t glyphIdIn,
                 int64 clusterIn,
                 bool unsafeToBreakIn,
                 bool whitespaceIn,
                 bool newlineIn,
                 int8_t distanceFromLigatureIn,
                 Point<float> advanceIn,
                 Point<float> offsetIn)
        : advance (advanceIn),
          offset (offsetIn),
          cluster (clusterIn),
          glyphId (glyphIdIn),
          unsafeToBreak (unsafeToBreakIn),
          whitespace (whitespaceIn),
          newline (newlineIn),
          distanceFromLigature (distanceFromLigatureIn) {}

    bool isUnsafeToBreak() const { return unsafeToBreak; }
    bool isWhitespace() const { return whitespace; }
    bool isNewline() const { return newline; }

    bool isNonLigature() const { return distanceFromLigature == 0; }
    bool isLigature() const { return distanceFromLigature < 0; }
    bool isPlaceholderForLigature() const { return distanceFromLigature > 0; }

    int8_t getDistanceFromLigature() const { return distanceFromLigature; }
    int8_t getNumTrailingLigaturePlaceholders() const { return (int8_t) -distanceFromLigature; }

    Point<float> advance;
    Point<float> offset;
    int64 cluster;
    uint32_t glyphId;

private:
    // These are effectively bools, pack into a single int once we have more than four flags.
    int8_t unsafeToBreak;
    int8_t whitespace;
    int8_t newline;
    int8_t distanceFromLigature;
};

struct GlyphLookupEntry
{
    Range<int64> glyphRange;
    bool ltr = true;
};

class SimpleShapedText
{
public:
    /*  Shapes and lays out the first contiguous sequence of ranges specified in the fonts
        parameter.
    */
    SimpleShapedText (const String* data,
                      const ShapedTextOptions& options);

    const auto& getLineNumbersForGlyphRanges() const { return lineNumbersForGlyphRanges; }

    const auto& getLineTextRanges() const { return lineTextRanges; }

    const auto& getResolvedFonts() const { return resolvedFonts; }

    Range<int64> getTextRange (int64 glyphIndex) const;

    /*  Returns true if the specified glyph is inside to an LTR run.
    */
    bool isLtr (int64 glyphIndex) const;

    /*  This function may fail to return an out range, even if the provided textRange falls inside
        the string range used for the creation of the ShapedText object.

        This is because the shaping process could fail due to insufficient glyph resolution to the
        point, where it will produce zero glyphs for the provided string.
    */
    void getGlyphRanges (Range<int64> textRange, std::vector<Range<int64>>& outRanges) const;

    /*  Returns the input codepoint index that follows the glyph in a logical sense. So for LTR text
        this is the cluster number of the glyph to the right. For RTL text it's the one on the left.

        If there is no subsequent glyph, the returned number is the first Unicode codepoint index
        that isn't covered by the cluster to which the selected glyph belongs, so for the glyph 'o'
        in "hello" this would be 5, given there are no ligatures in use.
    */
    int64 getTextIndexAfterGlyph (int64 glyphIndex) const;

    int64 getNumLines() const { return (int64) lineNumbersForGlyphRanges.getRanges().size(); }
    int64 getNumGlyphs() const { return (int64) glyphsInVisualOrder.size(); }

    juce_wchar getCodepoint (int64 glyphIndex) const;

    Span<const ShapedGlyph> getGlyphs (Range<int64> glyphRange) const;

    Span<const ShapedGlyph> getGlyphs() const;

    const auto& getGlyphLookup() const { return glyphLookup; }

private:
    void shape (const String& data,
                const ShapedTextOptions& options);

    const String& string;
    std::vector<ShapedGlyph> glyphsInVisualOrder;
    detail::RangedValues<int64> lineNumbersForGlyphRanges;
    detail::Ranges lineTextRanges;
    detail::RangedValues<Font> resolvedFonts;
    detail::RangedValues<GlyphLookupEntry> glyphLookup;

    JUCE_LEAK_DETECTOR (SimpleShapedText)
};

} // namespace juce::detail
