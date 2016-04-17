/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

TextLayout::Glyph::Glyph (const int glyph, Point<float> anch, float w) noexcept
    : glyphCode (glyph), anchor (anch), width (w)
{
}

TextLayout::Glyph::Glyph (const Glyph& other) noexcept
    : glyphCode (other.glyphCode), anchor (other.anchor), width (other.width)
{
}

TextLayout::Glyph& TextLayout::Glyph::operator= (const Glyph& other) noexcept
{
    glyphCode = other.glyphCode;
    anchor = other.anchor;
    width = other.width;
    return *this;
}

TextLayout::Glyph::~Glyph() noexcept {}

//==============================================================================
TextLayout::Run::Run() noexcept
    : colour (0xff000000)
{
}

TextLayout::Run::Run (Range<int> range, const int numGlyphsToPreallocate)
    : colour (0xff000000), stringRange (range)
{
    glyphs.ensureStorageAllocated (numGlyphsToPreallocate);
}

TextLayout::Run::Run (const Run& other)
    : font (other.font),
      colour (other.colour),
      glyphs (other.glyphs),
      stringRange (other.stringRange)
{
}

TextLayout::Run::~Run() noexcept {}

//==============================================================================
TextLayout::Line::Line() noexcept
    : ascent (0.0f), descent (0.0f), leading (0.0f)
{
}

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

TextLayout::Line::~Line() noexcept
{
}

Range<float> TextLayout::Line::getLineBoundsX() const noexcept
{
    Range<float> range;
    bool isFirst = true;

    for (int i = runs.size(); --i >= 0;)
    {
        const Run& run = *runs.getUnchecked(i);

        if (run.glyphs.size() > 0)
        {
            float minX = run.glyphs.getReference(0).anchor.x;
            float maxX = minX;

            for (int j = run.glyphs.size(); --j >= 0;)
            {
                const Glyph& glyph = run.glyphs.getReference (j);
                const float x = glyph.anchor.x;
                minX = jmin (minX, x);
                maxX = jmax (maxX, x + glyph.width);
            }

            if (isFirst)
            {
                isFirst = false;
                range = Range<float> (minX, maxX);
            }
            else
            {
                range = range.getUnionWith (Range<float> (minX, maxX));
            }
        }
    }

    return range + lineOrigin.x;
}

Range<float> TextLayout::Line::getLineBoundsY() const noexcept
{
    return Range<float> (lineOrigin.y - ascent,
                         lineOrigin.y + descent);
}

Rectangle<float> TextLayout::Line::getLineBounds() const noexcept
{
    const Range<float> x (getLineBoundsX()),
                       y (getLineBoundsY());

    return Rectangle<float> (x.getStart(), y.getStart(), x.getLength(), y.getLength());
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

#if JUCE_COMPILER_SUPPORTS_MOVE_SEMANTICS
TextLayout::TextLayout (TextLayout&& other) noexcept
    : lines (static_cast<OwnedArray<Line>&&> (other.lines)),
      width (other.width), height (other.height),
      justification (other.justification)
{
}

TextLayout& TextLayout::operator= (TextLayout&& other) noexcept
{
    lines = static_cast<OwnedArray<Line>&&> (other.lines);
    width = other.width;
    height = other.height;
    justification = other.justification;
    return *this;
}
#endif

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

TextLayout::Line& TextLayout::getLine (const int index) const
{
    return *lines.getUnchecked (index);
}

void TextLayout::ensureStorageAllocated (int numLinesNeeded)
{
    lines.ensureStorageAllocated (numLinesNeeded);
}

void TextLayout::addLine (Line* line)
{
    lines.add (line);
}

void TextLayout::draw (Graphics& g, const Rectangle<float>& area) const
{
    const Point<float> origin (justification.appliedToRectangle (Rectangle<float> (width, getHeight()), area).getPosition());

    LowLevelGraphicsContext& context = g.getInternalContext();

    for (int i = 0; i < lines.size(); ++i)
    {
        const Line& line = getLine (i);
        const Point<float> lineOrigin (origin + line.lineOrigin);

        for (int j = 0; j < line.runs.size(); ++j)
        {
            const Run& run = *line.runs.getUnchecked (j);
            context.setFont (run.font);
            context.setFill (run.colour);

            for (int k = 0; k < run.glyphs.size(); ++k)
            {
                const Glyph& glyph = run.glyphs.getReference (k);
                context.drawGlyph (glyph.glyphCode, AffineTransform::translation (lineOrigin.x + glyph.anchor.x,
                                                                                  lineOrigin.y + glyph.anchor.y));
            }
        }
    }
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

    if (! createNativeLayout (text))
        createStandardLayout (text);

    recalculateSize();
}

void TextLayout::createLayoutWithBalancedLineLengths (const AttributedString& text, float maxWidth)
{
    createLayoutWithBalancedLineLengths (text, maxWidth, 1.0e7f);
}

void TextLayout::createLayoutWithBalancedLineLengths (const AttributedString& text, float maxWidth, float maxHeight)
{
    const float minimumWidth = maxWidth / 2.0f;
    float bestWidth = maxWidth;
    float bestLineProportion = 0.0f;

    while (maxWidth > minimumWidth)
    {
        createLayout (text, maxWidth, maxHeight);

        if (getNumLines() < 2)
            return;

        const float line1 = lines.getUnchecked (lines.size() - 1)->getLineBoundsX().getLength();
        const float line2 = lines.getUnchecked (lines.size() - 2)->getLineBoundsX().getLength();
        const float shortestLine = jmin (line1, line2);
        const float prop = (shortestLine > 0) ? jmax (line1, line2) / shortestLine : 1.0f;

        if (prop > 0.9f)
            return;

        if (prop > bestLineProportion)
        {
            bestLineProportion = prop;
            bestWidth = maxWidth;
        }

        maxWidth -= 10.0f;
    }

    if (bestWidth != maxWidth)
        createLayout (text, bestWidth, maxHeight);
}

//==============================================================================
namespace TextLayoutHelpers
{
    struct Token
    {
        Token (const String& t, const Font& f, Colour c, const bool whitespace)
            : text (t), font (f), colour (c),
              area (font.getStringWidthFloat (t), f.getHeight()),
              isWhitespace (whitespace),
              isNewLine (t.containsChar ('\n') || t.containsChar ('\r'))
        {}

        const String text;
        const Font font;
        const Colour colour;
        Rectangle<float> area;
        int line;
        float lineHeight;
        const bool isWhitespace, isNewLine;

    private:
        Token& operator= (const Token&);
    };

    struct TokenList
    {
        TokenList() noexcept  : totalLines (0) {}

        void createLayout (const AttributedString& text, TextLayout& layout)
        {
            layout.ensureStorageAllocated (totalLines);

            addTextRuns (text);
            layoutRuns (layout.getWidth(), text.getLineSpacing());

            int charPosition = 0;
            int lineStartPosition = 0;
            int runStartPosition = 0;

            ScopedPointer<TextLayout::Line> currentLine;
            ScopedPointer<TextLayout::Run> currentRun;

            bool needToSetLineOrigin = true;

            for (int i = 0; i < tokens.size(); ++i)
            {
                const Token& t = *tokens.getUnchecked (i);

                Array<int> newGlyphs;
                Array<float> xOffsets;
                t.font.getGlyphPositions (getTrimmedEndIfNotAllWhitespace (t.text), newGlyphs, xOffsets);

                if (currentRun == nullptr)  currentRun  = new TextLayout::Run();
                if (currentLine == nullptr) currentLine = new TextLayout::Line();

                if (newGlyphs.size() > 0)
                {
                    currentRun->glyphs.ensureStorageAllocated (currentRun->glyphs.size() + newGlyphs.size());
                    const Point<float> tokenOrigin (t.area.getPosition().translated (0, t.font.getAscent()));

                    if (needToSetLineOrigin)
                    {
                        needToSetLineOrigin = false;
                        currentLine->lineOrigin = tokenOrigin;
                    }

                    const Point<float> glyphOffset (tokenOrigin - currentLine->lineOrigin);

                    for (int j = 0; j < newGlyphs.size(); ++j)
                    {
                        const float x = xOffsets.getUnchecked (j);
                        currentRun->glyphs.add (TextLayout::Glyph (newGlyphs.getUnchecked(j),
                                                                   glyphOffset.translated (x, 0),
                                                                   xOffsets.getUnchecked (j + 1) - x));
                    }

                    charPosition += newGlyphs.size();
                }

                if (t.isWhitespace || t.isNewLine)
                    ++charPosition;

                const Token* const nextToken = tokens [i + 1];

                if (nextToken == nullptr) // this is the last token
                {
                    addRun (*currentLine, currentRun.release(), t, runStartPosition, charPosition);
                    currentLine->stringRange = Range<int> (lineStartPosition, charPosition);

                    if (! needToSetLineOrigin)
                        layout.addLine (currentLine.release());

                    needToSetLineOrigin = true;
                }
                else
                {
                    if (t.font != nextToken->font || t.colour != nextToken->colour)
                    {
                        addRun (*currentLine, currentRun.release(), t, runStartPosition, charPosition);
                        runStartPosition = charPosition;
                    }

                    if (t.line != nextToken->line)
                    {
                        if (currentRun == nullptr)
                            currentRun = new TextLayout::Run();

                        addRun (*currentLine, currentRun.release(), t, runStartPosition, charPosition);
                        currentLine->stringRange = Range<int> (lineStartPosition, charPosition);

                        if (! needToSetLineOrigin)
                            layout.addLine (currentLine.release());

                        runStartPosition = charPosition;
                        lineStartPosition = charPosition;
                        needToSetLineOrigin = true;
                    }
                }
            }

            if ((text.getJustification().getFlags() & (Justification::right | Justification::horizontallyCentred)) != 0)
            {
                const float totalW = layout.getWidth();
                const bool isCentred = (text.getJustification().getFlags() & Justification::horizontallyCentred) != 0;

                for (int i = 0; i < layout.getNumLines(); ++i)
                {
                    float dx = totalW - layout.getLine(i).getLineBoundsX().getLength();

                    if (isCentred)
                        dx /= 2.0f;

                    layout.getLine(i).lineOrigin.x += dx;
                }
            }
        }

    private:
        static void addRun (TextLayout::Line& glyphLine, TextLayout::Run* glyphRun,
                            const Token& t, const int start, const int end)
        {
            glyphRun->stringRange = Range<int> (start, end);
            glyphRun->font = t.font;
            glyphRun->colour = t.colour;
            glyphLine.ascent  = jmax (glyphLine.ascent,  t.font.getAscent());
            glyphLine.descent = jmax (glyphLine.descent, t.font.getDescent());
            glyphLine.runs.add (glyphRun);
        }

        static int getCharacterType (const juce_wchar c) noexcept
        {
            if (c == '\r' || c == '\n')
                return 0;

            return CharacterFunctions::isWhitespace (c) ? 2 : 1;
        }

        void appendText (const String& stringText, const Font& font, Colour colour)
        {
            String::CharPointerType t (stringText.getCharPointer());
            String currentString;
            int lastCharType = 0;

            for (;;)
            {
                const juce_wchar c = t.getAndAdvance();
                if (c == 0)
                    break;

                const int charType = getCharacterType (c);

                if (charType == 0 || charType != lastCharType)
                {
                    if (currentString.isNotEmpty())
                        tokens.add (new Token (currentString, font, colour,
                                               lastCharType == 2 || lastCharType == 0));

                    currentString = String::charToString (c);

                    if (c == '\r' && *t == '\n')
                        currentString += t.getAndAdvance();
                }
                else
                {
                    currentString += c;
                }

                lastCharType = charType;
            }

            if (currentString.isNotEmpty())
                tokens.add (new Token (currentString, font, colour, lastCharType == 2));
        }

        void layoutRuns (const float maxWidth, const float extraLineSpacing)
        {
            float x = 0, y = 0, h = 0;
            int i;

            for (i = 0; i < tokens.size(); ++i)
            {
                Token& t = *tokens.getUnchecked(i);
                t.area.setPosition (x, y);
                t.line = totalLines;
                x += t.area.getWidth();
                h = jmax (h, t.area.getHeight() + extraLineSpacing);

                const Token* const nextTok = tokens[i + 1];

                if (nextTok == nullptr)
                    break;

                if (t.isNewLine || ((! nextTok->isWhitespace) && x + nextTok->area.getWidth() > maxWidth))
                {
                    setLastLineHeight (i + 1, h);
                    x = 0;
                    y += h;
                    h = 0;
                    ++totalLines;
                }
            }

            setLastLineHeight (jmin (i + 1, tokens.size()), h);
            ++totalLines;
        }

        void setLastLineHeight (int i, const float height) noexcept
        {
            while (--i >= 0)
            {
                Token& tok = *tokens.getUnchecked (i);

                if (tok.line == totalLines)
                    tok.lineHeight = height;
                else
                    break;
            }
        }

        void addTextRuns (const AttributedString& text)
        {
            const int numAttributes = text.getNumAttributes();
            tokens.ensureStorageAllocated (jmax (64, numAttributes));

            for (int i = 0; i < numAttributes; ++i)
            {
                const AttributedString::Attribute& attr = text.getAttribute (i);

                appendText (text.getText().substring (attr.range.getStart(), attr.range.getEnd()),
                            attr.font, attr.colour);
            }
        }

        static String getTrimmedEndIfNotAllWhitespace (const String& s)
        {
            String trimmed (s.trimEnd());
            if (trimmed.isEmpty() && s.isNotEmpty())
                trimmed = s.replaceCharacters ("\r\n\t", "   ");

            return trimmed;
        }

        OwnedArray<Token> tokens;
        int totalLines;

        JUCE_DECLARE_NON_COPYABLE (TokenList)
    };
}

//==============================================================================
void TextLayout::createStandardLayout (const AttributedString& text)
{
    TextLayoutHelpers::TokenList l;
    l.createLayout (text, *this);
}

void TextLayout::recalculateSize()
{
    if (lines.size() > 0)
    {
        Rectangle<float> bounds (lines.getFirst()->getLineBounds());

        for (int i = lines.size(); --i > 0;)
            bounds = bounds.getUnion (lines.getUnchecked(i)->getLineBounds());

        for (int i = lines.size(); --i >= 0;)
            lines.getUnchecked(i)->lineOrigin.x -= bounds.getX();

        width  = bounds.getWidth();
        height = bounds.getHeight();
    }
    else
    {
        width = 0;
        height = 0;
    }
}
