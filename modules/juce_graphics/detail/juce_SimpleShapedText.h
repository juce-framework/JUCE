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
                         maxWidth,
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

    [[nodiscard]] ShapedTextOptions withMaxWidth (float x) const
    {
        return withMember (*this, &ShapedTextOptions::maxWidth, x);
    }

    [[nodiscard]] ShapedTextOptions withHeight (float x) const
    {
        return withMember (*this, &ShapedTextOptions::height, x);
    }

    [[nodiscard]] ShapedTextOptions withFont (Font x) const
    {
        RangedValues<Font> fonts;
        fonts.set ({ 0, std::numeric_limits<int64>::max() }, x);

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
    const auto& getMaxWidth() const                     { return maxWidth; }
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
    const auto& getAllowBreakingInsideWord() const      { return allowBreakingInsideWord; }

private:
    Justification justification { Justification::topLeft };
    std::optional<TextDirection> readingDir;
    std::optional<float> maxWidth;
    std::optional<float> height;

    detail::RangedValues<Font> fontsForRange = std::invoke ([&]
    {
        detail::RangedValues<Font> result;
        result.set ({ 0, std::numeric_limits<int64>::max() }, FontOptions { 15.0f });
        return result;
    });

    String language = SystemStats::getDisplayLanguage();
    float firstLineIndent = 0.0f;
    float leading = 1.0f;
    float additiveLineSpacing = 0.0f;
    bool baselineAtZero = false;
    bool allowBreakingInsideWord = false;
    bool trailingWhitespacesShouldFit = true;
    int64 maxNumLines = std::numeric_limits<int64>::max();
    String ellipsis;
};

struct ShapedGlyph
{
    uint32_t glyphId;
    int64 cluster;
    bool unsafeToBreak;
    bool whitespace;
    bool newline;
    Point<float> advance;
    Point<float> offset;
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

    /*  The returned container associates line numbers with the range of glyphs (not input codepoints)
        that make up the line.
    */
    const auto& getLineNumbers() const { return lineNumbers; }

    const auto& getResolvedFonts() const { return resolvedFonts; }

    Range<int64> getTextRange (int64 glyphIndex) const;

    std::vector<Range<int64>> getGlyphRanges (Range<int64> textRange) const;

    int64 getNumLines() const { return (int64) lineNumbers.getRanges().size(); }
    int64 getNumGlyphs() const { return (int64) glyphsInVisualOrder.size(); }

    juce_wchar getCodepoint (int64 glyphIndex) const;

    Span<const ShapedGlyph> getGlyphs (Range<int64> glyphRange) const;

    Span<const ShapedGlyph> getGlyphs() const;

private:
    void shape (const String& data,
                const ShapedTextOptions& options);

    const String& string;
    std::vector<ShapedGlyph> glyphsInVisualOrder;
    detail::RangedValues<int64> lineNumbers;
    detail::RangedValues<Font> resolvedFonts;
    detail::RangedValues<GlyphLookupEntry> glyphLookup;

    JUCE_LEAK_DETECTOR (SimpleShapedText)
};

} // namespace juce::detail
