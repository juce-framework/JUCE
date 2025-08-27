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

TextLayout::Glyph::Glyph (int glyph, Point<float> anch, float w) noexcept
    : glyphCode (glyph), anchor (anch), width (w)
{
}

//==============================================================================
TextLayout::Run::Run (Range<int> range, int numGlyphsToPreallocate)
    : stringRange (range)
{
    glyphs.ensureStorageAllocated (numGlyphsToPreallocate);
}

Range<float> TextLayout::Run::getRunBoundsX() const noexcept
{
    Range<float> range;
    bool isFirst = true;

    for (auto& glyph : glyphs)
    {
        Range<float> r (glyph.anchor.x, glyph.anchor.x + glyph.width);

        if (isFirst)
        {
            isFirst = false;
            range = r;
        }
        else
        {
            range = range.getUnionWith (r);
        }
    }

    return range;
}

//==============================================================================
TextLayout::Line::Line (Range<int> range, Point<float> o, float asc, float desc,
                        float lead, int numRunsToPreallocate)
    : stringRange (range), lineOrigin (o),
      ascent (asc), descent (desc), leading (lead)
{
    runs.ensureStorageAllocated (numRunsToPreallocate);
}

TextLayout::Line::Line (const Line& other)
    : stringRange (other.stringRange), lineOrigin (other.lineOrigin),
      ascent (other.ascent), descent (other.descent), leading (other.leading)
{
    runs.addCopiesOf (other.runs);
}

TextLayout::Line& TextLayout::Line::operator= (const Line& other)
{
    auto copy = other;
    swap (copy);
    return *this;
}

Range<float> TextLayout::Line::getLineBoundsX() const noexcept
{
    Range<float> range;
    bool isFirst = true;

    for (auto* run : runs)
    {
        auto runRange = run->getRunBoundsX();

        if (isFirst)
        {
            isFirst = false;
            range = runRange;
        }
        else
        {
            range = range.getUnionWith (runRange);
        }
    }

    return range + lineOrigin.x;
}

Range<float> TextLayout::Line::getLineBoundsY() const noexcept
{
    return { lineOrigin.y - ascent,
             lineOrigin.y + descent };
}

Rectangle<float> TextLayout::Line::getLineBounds() const noexcept
{
    auto x = getLineBoundsX();
    auto y = getLineBoundsY();

    return { x.getStart(), y.getStart(), x.getLength(), y.getLength() };
}

void TextLayout::Line::swap (Line& other) noexcept
{
    std::swap (other.runs,          runs);
    std::swap (other.stringRange,   stringRange);
    std::swap (other.lineOrigin,    lineOrigin);
    std::swap (other.ascent,        ascent);
    std::swap (other.descent,       descent);
    std::swap (other.leading,       leading);
}

//==============================================================================
TextLayout::TextLayout()
    : width (0), height (0), justification (Justification::topLeft)
{
}

TextLayout::TextLayout (const TextLayout& other)
    : width (other.width), height (other.height),
      justification (other.justification)
{
    lines.addCopiesOf (other.lines);
}

TextLayout::TextLayout (TextLayout&& other) noexcept
    : lines (std::move (other.lines)),
      width (other.width), height (other.height),
      justification (other.justification)
{
}

TextLayout& TextLayout::operator= (TextLayout&& other) noexcept
{
    lines = std::move (other.lines);
    width = other.width;
    height = other.height;
    justification = other.justification;
    return *this;
}

TextLayout& TextLayout::operator= (const TextLayout& other)
{
    width = other.width;
    height = other.height;
    justification = other.justification;
    lines.clear();
    lines.addCopiesOf (other.lines);
    return *this;
}

TextLayout::~TextLayout()
{
}

TextLayout::Line& TextLayout::getLine (int index) const noexcept
{
    return *lines.getUnchecked (index);
}

void TextLayout::ensureStorageAllocated (int numLinesNeeded)
{
    lines.ensureStorageAllocated (numLinesNeeded);
}

void TextLayout::addLine (std::unique_ptr<Line> line)
{
    lines.add (line.release());
}

void TextLayout::draw (Graphics& g, Rectangle<float> area) const
{
    auto origin = justification.appliedToRectangle (Rectangle<float> (width, getHeight()), area).getPosition();

    auto& context   = g.getInternalContext();
    context.saveState();

    auto clip       = context.getClipBounds();
    auto clipTop    = (float) clip.getY()      - origin.y;
    auto clipBottom = (float) clip.getBottom() - origin.y;

    std::vector<uint16_t> glyphNumbers;
    std::vector<Point<float>> positions;

    for (auto& line : *this)
    {
        auto lineRangeY = line.getLineBoundsY();

        if (lineRangeY.getEnd() < clipTop)
            continue;

        if (lineRangeY.getStart() > clipBottom)
            break;

        auto lineOrigin = origin + line.lineOrigin;

        for (auto* run : line.runs)
        {
            context.setFont (run->font);
            context.setFill (run->colour);

            const auto& glyphs = run->glyphs;

            glyphNumbers.resize ((size_t) glyphs.size());
            std::transform (glyphs.begin(), glyphs.end(), glyphNumbers.begin(), [&] (const Glyph& x)
            {
                return (uint16_t) x.glyphCode;
            });

            positions.resize ((size_t) glyphs.size());
            std::transform (glyphs.begin(), glyphs.end(), positions.begin(), [&] (const Glyph& x)
            {
                return x.anchor;
            });

            context.drawGlyphs (glyphNumbers, positions, AffineTransform::translation (lineOrigin));

            if (run->font.isUnderlined())
            {
                const auto runExtent = run->getRunBoundsX();
                const auto lineThickness = run->font.getDescent() * 0.3f;

                context.fillRect ({ runExtent.getStart() + lineOrigin.x,
                                    lineOrigin.y + lineThickness * 2.0f,
                                    runExtent.getLength(),
                                    lineThickness });
            }
        }
    }

    context.restoreState();
}

void TextLayout::createLayout (const AttributedString& text, float maxWidth)
{
    createLayout (text, maxWidth, 1.0e7f);
}

void TextLayout::createLayout (const AttributedString& text, float maxWidth, float maxHeight)
{
    lines.clear();
    width = maxWidth;
    height = maxHeight;
    justification = text.getJustification();

    createStandardLayout (text);

    recalculateSize();
}

void TextLayout::createLayoutWithBalancedLineLengths (const AttributedString& text, float maxWidth)
{
    createLayoutWithBalancedLineLengths (text, maxWidth, 1.0e7f);
}

void TextLayout::createLayoutWithBalancedLineLengths (const AttributedString& text,
                                                      float maxWidth,
                                                      float maxHeight)
{
    auto minimumWidth = maxWidth / 2.0f;
    auto bestWidth = maxWidth;
    auto bestScore = std::numeric_limits<float>::max();

    auto widthProbeText = text;

    if (widthProbeText.getWordWrap() == AttributedString::byChar)
        widthProbeText.setWordWrap (AttributedString::byWord);

    std::optional<float> advanceWidth;

    while (maxWidth > minimumWidth)
    {
        createLayout (widthProbeText, maxWidth, maxHeight);

        const auto numLines = getNumLines();

        if (numLines < 2)
            return;

        std::vector<float> lineLengths;
        lineLengths.reserve ((size_t) numLines);
        std::transform (lines.begin(),
                        lines.end(),
                        std::back_inserter (lineLengths),
                        [] (const auto& line) { return line->getLineBoundsX().getLength(); });

        const auto longestLineLength = *std::max_element (lineLengths.begin(), lineLengths.end());

        auto accumulateScore = [longestLineLength] (auto scoreSum, auto lineLength)
        {
            const auto unusedSpace = 1.0f - (lineLength / longestLineLength);
            return scoreSum + (unusedSpace * unusedSpace);
        };

        const auto score = std::accumulate (lineLengths.begin(),
                                            lineLengths.end(),
                                            0.0f, accumulateScore) / (float) numLines;

        if (score < bestScore)
        {
            bestScore = score;
            bestWidth = maxWidth;

            if (score < 0.4f)
                break;
        }

        if (! advanceWidth.has_value())
        {
            advanceWidth = std::numeric_limits<float>::max();

            for (const auto& line : lines)
                for (const auto& run : line->runs)
                    for (const auto& glyph : run->glyphs)
                        advanceWidth = jmin (*advanceWidth, glyph.width);
        }

        maxWidth -= *advanceWidth;
    }

    createLayout (text, bestWidth, maxHeight);
}

//==============================================================================
template <typename T, typename U>
static auto castTo (const Range<U>& r)
{
    return Range<T> (static_cast<T> (r.getStart()), static_cast<T> (r.getEnd()));
}

static Range<int64> getInputRange (const detail::ShapedText& st, Range<int64> glyphRange)
{
    if (glyphRange.isEmpty())
    {
        jassertfalse;
        return {};
    }

    const auto startInputRange = st.getTextRange (glyphRange.getStart());
    const auto endInputRange   = st.getTextRange (glyphRange.getEnd() - 1);

    // The glyphRange is always in visual order and could have an opposite direction to the text
    return { std::min (startInputRange.getStart(), endInputRange.getStart()),
             std::max (startInputRange.getEnd(), endInputRange.getEnd()) };
}

static Range<int64> getLineInputRange (const detail::ShapedText& st, int64 lineNumber)
{
    using namespace detail;

    return getInputRange (st, st.getSimpleShapedText()
                                .getLineNumbersForGlyphRanges()
                                .getItem ((size_t) lineNumber).range);
}

struct MaxFontAscentAndDescent
{
    float ascent{}, descent{};
};

static MaxFontAscentAndDescent getMaxFontAscentAndDescentInEnclosingLine (const detail::ShapedText& st,
                                                                          Range<int64> lineChunkRange)
{
    const auto sst = st.getSimpleShapedText();

    const auto lineRange = sst.getLineNumbersForGlyphRanges()
                              .getItemWithEnclosingRange (lineChunkRange.getStart())->range;

    const auto fonts = sst.getResolvedFonts().getIntersectionsWith (lineRange);

    MaxFontAscentAndDescent result;

    for (const auto pair : fonts)
    {
        result.ascent = std::max (result.ascent, pair.value.getAscent());
        result.descent = std::max (result.descent, pair.value.getDescent());
    }

    return result;
}

static std::optional<detail::TextDirection> getTextDirection (const AttributedString& text)
{
    using namespace detail;

    using ReadingDirection = AttributedString::ReadingDirection;

    const auto dir = text.getReadingDirection();

    if (dir == ReadingDirection::leftToRight)
        return TextDirection::ltr;

    if (dir == ReadingDirection::rightToLeft)
        return TextDirection::rtl;

    return std::nullopt;
}

void TextLayout::createStandardLayout (const AttributedString& text)
{
    using namespace detail;

    detail::Ranges::Operations ops;

    RangedValues<Font> fonts;
    RangedValues<Colour> colours;

    for (auto i = 0, iMax = text.getNumAttributes(); i < iMax; ++i)
    {
        const auto& attribute = text.getAttribute (i);
        const auto range = castTo<int64> (attribute.range);
        fonts.set (range, attribute.font, ops);
        colours.set (range, attribute.colour, ops);
        ops.clear();
    }

    auto shapedTextOptions = ShapedTextOptions{}.withFonts (fonts)
                                                .withLanguage (SystemStats::getUserLanguage())
                                                .withTrailingWhitespacesShouldFit (false)
                                                .withJustification (justification)
                                                .withReadingDirection (getTextDirection (text))
                                                .withAdditiveLineSpacing (text.getLineSpacing());

    if (text.getWordWrap() != AttributedString::none)
        shapedTextOptions = shapedTextOptions.withWordWrapWidth (width);

    if (text.getWordWrap() == AttributedString::WordWrap::byChar)
        shapedTextOptions = shapedTextOptions.withAllowBreakingInsideWord (true);

    ShapedText st { text.getText(), shapedTextOptions };

    std::optional<int64> lastLineNumber;
    std::unique_ptr<Line> line;

    st.accessTogetherWith ([&] (Span<const ShapedGlyph> glyphs,
                                Span<const Point<float>> positions,
                                Font font,
                                Range<int64> glyphRange,
                                LineMetrics lineMetrics,
                                Colour colour)
                           {
                               if (std::exchange (lastLineNumber, lineMetrics.lineNumber) != lineMetrics.lineNumber)
                               {
                                   if (line != nullptr)
                                       addLine (std::move (line));

                                   const auto ascentAndDescent = getMaxFontAscentAndDescentInEnclosingLine (st,
                                                                                                            glyphRange);

                                   line = std::make_unique<Line> (castTo<int> (getLineInputRange (st, lineMetrics.lineNumber)),
                                                                  positions[0],
                                                                  ascentAndDescent.ascent,
                                                                  ascentAndDescent.descent,
                                                                  0.0f,
                                                                  0);
                               }

                               auto run = std::make_unique<Run> (castTo<int> (getInputRange (st, glyphRange)), 0);

                               run->font = font;
                               run->colour = colour;

                               const auto beyondLastNonWhitespace = [&]
                               {
                                   auto i = glyphs.size();

                                   for (auto it  = std::reverse_iterator { glyphs.end() },
                                             end = std::reverse_iterator { glyphs.begin() };
                                        it != end && it->isWhitespace();
                                        ++it)
                                   {
                                       --i;
                                   }

                                   return i;
                               }();

                               for (size_t i = 0; i < beyondLastNonWhitespace; ++i)
                               {
                                   if (glyphs[i].isPlaceholderForLigature())
                                       continue;

                                   run->glyphs.add ({ (int) glyphs[i].glyphId,
                                                      positions[i] - line->lineOrigin,
                                                      glyphs[i].advance.x });
                               }

                               line->runs.add (std::move (run));
                           },
                           colours);

    if (line != nullptr)
        addLine (std::move (line));
}

void TextLayout::recalculateSize()
{
    if (! lines.isEmpty())
    {
        auto bounds = lines.getFirst()->getLineBounds();

        for (auto* line : lines)
            bounds = bounds.getUnion (line->getLineBounds());

        for (auto* line : lines)
            line->lineOrigin.x -= bounds.getX();

        width  = bounds.getWidth();
        height = bounds.getHeight();
    }
    else
    {
        width = 0;
        height = 0;
    }
}

} // namespace juce
