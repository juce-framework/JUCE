/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

BEGIN_JUCE_NAMESPACE

//==============================================================================
AttributedString::Attribute::Attribute (const Range<int>& range_, const Colour& colour_)
    : range (range_), colour (new Colour (colour_))
{
}

AttributedString::Attribute::Attribute (const Range<int>& range_, const Font& font_)
    : range (range_), font (new Font (font_))
{
}

AttributedString::Attribute::Attribute (const Attribute& other)
    : range (other.range),
      font (other.font.createCopy()),
      colour (other.colour.createCopy())
{
}

AttributedString::Attribute::~Attribute() {}

//==============================================================================
AttributedString::AttributedString()
    : lineSpacing (0.0f),
      justification (Justification::left),
      wordWrap (AttributedString::byWord),
      readingDirection (AttributedString::natural)
{
}

AttributedString::AttributedString (const String& newString)
    : text (newString),
      lineSpacing (0.0f),
      justification (Justification::left),
      wordWrap (AttributedString::byWord),
      readingDirection (AttributedString::natural)
{
}

AttributedString::AttributedString (const AttributedString& other)
    : text (other.text),
      lineSpacing (other.lineSpacing),
      justification (other.justification),
      wordWrap (other.wordWrap),
      readingDirection (other.readingDirection)
{
    attributes.addCopiesOf (other.attributes);
}

AttributedString& AttributedString::operator= (const AttributedString& other)
{
    if (this != &other)
    {
        text = other.text;
        lineSpacing = other.lineSpacing;
        justification = other.justification;
        wordWrap = other.wordWrap;
        readingDirection = other.readingDirection;
        attributes.clear();
        attributes.addCopiesOf (other.attributes);
    }

    return *this;
}

AttributedString::~AttributedString() {}

void AttributedString::setText (const String& other)
{
    text = other;
}

void AttributedString::setJustification (const Justification& newJustification) noexcept
{
    justification = newJustification;
}

void AttributedString::setWordWrap (WordWrap newWordWrap) noexcept
{
    wordWrap = newWordWrap;
}

void AttributedString::setReadingDirection (ReadingDirection newReadingDirection) noexcept
{
    readingDirection = newReadingDirection;
}

void AttributedString::setLineSpacing (const float newLineSpacing) noexcept
{
    lineSpacing = newLineSpacing;
}

void AttributedString::setColour (const Range<int>& range, const Colour& colour)
{
    attributes.add (new Attribute (range, colour));
}

void AttributedString::setFont (const Range<int>& range, const Font& font)
{
    attributes.add (new Attribute (range, font));
}

void AttributedString::draw (Graphics& g, const Rectangle<float>& area) const
{
    if (text.isNotEmpty() && g.clipRegionIntersects (area.getSmallestIntegerContainer()))
    {
        if (! g.getInternalContext()->drawTextLayout (*this, area))
        {
            GlyphLayout layout;
            layout.setText (*this, area.getWidth());
            layout.draw (g, area);
        }
    }
}


//==============================================================================
GlyphLayout::Glyph::Glyph (const int glyphCode_, const Point<float>& anchor_) noexcept
    : glyphCode (glyphCode_), anchor (anchor_)
{
}

GlyphLayout::Glyph::~Glyph() {}

//==============================================================================
GlyphLayout::Run::Run()
    : colour (0xff000000)
{
}

GlyphLayout::Run::Run (const Range<int>& range, const int numGlyphsToPreallocate)
    : stringRange (range), colour (0xff000000)
{
    glyphs.ensureStorageAllocated (numGlyphsToPreallocate);
}

GlyphLayout::Run::~Run() {}

GlyphLayout::Glyph& GlyphLayout::Run::getGlyph (const int index) const
{
    return *glyphs.getUnchecked (index);
}

void GlyphLayout::Run::ensureStorageAllocated (int numGlyphsNeeded)
{
    glyphs.ensureStorageAllocated (numGlyphsNeeded);
}

void GlyphLayout::Run::setStringRange (const Range<int>& newStringRange) noexcept
{
    stringRange = newStringRange;
}

void GlyphLayout::Run::setFont (const Font& newFont)
{
    font = newFont;
}

void GlyphLayout::Run::setColour (const Colour& newColour) noexcept
{
    colour = newColour;
}

void GlyphLayout::Run::addGlyph (Glyph* glyph)
{
    glyphs.add (glyph);
}

//==============================================================================
GlyphLayout::Line::Line() noexcept
    : ascent (0.0f), descent (0.0f), leading (0.0f)
{
}

GlyphLayout::Line::Line (const Range<int>& stringRange_, const Point<float>& lineOrigin_,
                         const float ascent_, const float descent_, const float leading_,
                         const int numRunsToPreallocate)
    : stringRange (stringRange_), lineOrigin (lineOrigin_),
      ascent (ascent_), descent (descent_), leading (leading_)
{
    runs.ensureStorageAllocated (numRunsToPreallocate);
}

GlyphLayout::Line::~Line()
{
}

GlyphLayout::Run& GlyphLayout::Line::getRun (const int index) const noexcept
{
    return *runs.getUnchecked (index);
}

void GlyphLayout::Line::setStringRange (const Range<int>& newStringRange) noexcept
{
    stringRange = newStringRange;
}

void GlyphLayout::Line::setLineOrigin (const Point<float>& newLineOrigin) noexcept
{
    lineOrigin = newLineOrigin;
}

void GlyphLayout::Line::setLeading (float newLeading) noexcept
{
    leading = newLeading;
}

void GlyphLayout::Line::increaseAscentDescent (float newAscent, float newDescent) noexcept
{
    ascent  = jmax (ascent, newAscent);
    descent = jmax (descent, newDescent);
}

void GlyphLayout::Line::addRun (Run* run)
{
    runs.add (run);
}

//==============================================================================
GlyphLayout::GlyphLayout()
    : width (0), justification (Justification::topLeft)
{
}

GlyphLayout::~GlyphLayout()
{
}

void GlyphLayout::setText (const AttributedString& text, float maxWidth)
{
    lines.clear();
    width = maxWidth;
    justification = text.getJustification();

    if (! createNativeLayout (text))
        createStandardLayout (text);
}

float GlyphLayout::getHeight() const noexcept
{
    const Line* const lastLine = lines.getLast();

    return lastLine != nullptr ? lastLine->getLineOrigin().getY() + lastLine->getDescent()
                               : 0;
}

GlyphLayout::Line& GlyphLayout::getLine (const int index) const
{
    return *lines[index];
}

void GlyphLayout::ensureStorageAllocated (int numLinesNeeded)
{
    lines.ensureStorageAllocated (numLinesNeeded);
}

void GlyphLayout::addLine (Line* line)
{
    lines.add (line);
}

void GlyphLayout::draw (Graphics& g, const Rectangle<float>& area) const
{
    const Point<float> origin (justification.appliedToRectangle (Rectangle<float> (0, 0, width, getHeight()), area).getPosition());

    LowLevelGraphicsContext& context = *g.getInternalContext();

    for (int i = 0; i < getNumLines(); ++i)
    {
        const Line& line = getLine (i);
        const Point<float> lineOrigin (origin + line.getLineOrigin());

        for (int j = 0; j < line.getNumRuns(); ++j)
        {
            const Run& run = line.getRun (j);
            context.setFont (run.getFont());
            context.setFill (run.getColour());

            for (int k = 0; k < run.getNumGlyphs(); ++k)
            {
                const Glyph& glyph = run.getGlyph (k);
                context.drawGlyph (glyph.glyphCode, AffineTransform::translation (lineOrigin.x + glyph.anchor.x,
                                                                                  lineOrigin.y + glyph.anchor.y));
            }
        }
    }
}

//==============================================================================
namespace GlyphLayoutHelpers
{
    struct FontAndColour
    {
        FontAndColour (const Font* font_) noexcept   : font (font_), colour (0xff000000) {}

        const Font* font;
        Colour colour;

        bool operator!= (const FontAndColour& other) const noexcept
        {
            return (font != other.font && *font != *other.font) || colour != other.colour;
        }
    };

    struct RunAttribute
    {
        RunAttribute (const FontAndColour& fontAndColour_, const Range<int>& range_) noexcept
            : fontAndColour (fontAndColour_), range (range_)
        {}

        FontAndColour fontAndColour;
        Range<int> range;
    };

    struct Token
    {
        Token (const String& t, const Font& f, const Colour& c, const bool isWhitespace_)
            : text (t), font (f), colour (c),
              area (font.getStringWidth (t), roundToInt (f.getHeight())),
              isWhitespace (isWhitespace_),
              isNewLine (t.containsChar ('\n') || t.containsChar ('\r'))
        {}

        const String text;
        const Font font;
        const Colour colour;
        Rectangle<int> area;
        int line, lineHeight;
        const bool isWhitespace, isNewLine;

    private:
        Token& operator= (const Token&);
    };

    class TokenList
    {
    public:
        TokenList() noexcept  : totalLines (0) {}

        void createLayout (const AttributedString& text, GlyphLayout& glyphLayout)
        {
            tokens.ensureStorageAllocated (64);
            glyphLayout.ensureStorageAllocated (totalLines);

            addTextRuns (text);

            layout ((int) glyphLayout.getWidth());

            int charPosition = 0;
            int lineStartPosition = 0;
            int runStartPosition = 0;

            GlyphLayout::Line* glyphLine = new GlyphLayout::Line();
            GlyphLayout::Run*  glyphRun  = new GlyphLayout::Run();

            for (int i = 0; i < tokens.size(); ++i)
            {
                const Token* const t = tokens.getUnchecked (i);
                const Point<float> tokenPos (t->area.getPosition().toFloat());

                Array <int> newGlyphs;
                Array <float> xOffsets;
                t->font.getGlyphPositions (t->text.trimEnd(), newGlyphs, xOffsets);

                glyphRun->ensureStorageAllocated (glyphRun->getNumGlyphs() + newGlyphs.size());

                for (int j = 0; j < newGlyphs.size(); ++j)
                {
                    if (charPosition == lineStartPosition)
                        glyphLine->setLineOrigin (tokenPos.translated (0, t->font.getAscent()));

                    glyphRun->addGlyph (new GlyphLayout::Glyph (newGlyphs.getUnchecked(j),
                                                                Point<float> (tokenPos.getX() + xOffsets.getUnchecked (j), 0)));
                    ++charPosition;
                }

                if (t->isWhitespace || t->isNewLine)
                    ++charPosition;

                const Token* const nextToken = tokens [i + 1];

                if (nextToken == nullptr) // this is the last token
                {
                    addRun (glyphLine, glyphRun, t, runStartPosition, charPosition);
                    glyphLine->setStringRange (Range<int> (lineStartPosition, charPosition));
                    glyphLayout.addLine (glyphLine);
                }
                else
                {
                    if (t->font != nextToken->font || t->colour != nextToken->colour)
                    {
                        addRun (glyphLine, glyphRun, t, runStartPosition, charPosition);
                        runStartPosition = charPosition;
                        glyphRun = new GlyphLayout::Run();
                    }

                    if (t->line != nextToken->line)
                    {
                        addRun (glyphLine, glyphRun, t, runStartPosition, charPosition);
                        glyphLine->setStringRange (Range<int> (lineStartPosition, charPosition));
                        glyphLayout.addLine (glyphLine);

                        runStartPosition = charPosition;
                        lineStartPosition = charPosition;
                        glyphLine = new GlyphLayout::Line();
                        glyphRun  = new GlyphLayout::Run();
                    }
                }
            }

            if ((text.getJustification().getFlags() & (Justification::right | Justification::horizontallyCentred)) != 0)
            {
                const int totalW = (int) glyphLayout.getWidth();

                for (int i = 0; i < totalLines; ++i)
                {
                    const int lineW = getLineWidth (i);
                    float dx = 0;

                    if ((text.getJustification().getFlags() & Justification::right) != 0)
                        dx = (float) (totalW - lineW);
                    else
                        dx = (totalW - lineW) / 2.0f;

                    GlyphLayout::Line& glyphLine = glyphLayout.getLine (i);
                    glyphLine.setLineOrigin (glyphLine.getLineOrigin().translated (dx, 0));
                }
            }
        }

    private:
        static void addRun (GlyphLayout::Line* glyphLine, GlyphLayout::Run* glyphRun,
                            const Token* const t, const int start, const int end)
        {
            glyphRun->setStringRange (Range<int> (start, end));
            glyphRun->setFont (t->font);
            glyphRun->setColour (t->colour);
            glyphLine->increaseAscentDescent (t->font.getAscent(), t->font.getDescent());
            glyphLine->addRun (glyphRun);
        }

        void appendText (const AttributedString& text, const Range<int>& stringRange,
                         const Font& font, const Colour& colour)
        {
            String stringText (text.getText().substring(stringRange.getStart(), stringRange.getEnd()));
            String::CharPointerType t (stringText.getCharPointer());
            String currentString;
            int lastCharType = 0;

            for (;;)
            {
                const juce_wchar c = t.getAndAdvance();
                if (c == 0)
                    break;

                int charType;
                if (c == '\r' || c == '\n')
                    charType = 0;
                else if (CharacterFunctions::isWhitespace (c))
                    charType = 2;
                else
                    charType = 1;

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

        void layout (const int maxWidth)
        {
            int x = 0, y = 0, h = 0;
            int i;

            for (i = 0; i < tokens.size(); ++i)
            {
                Token* const t = tokens.getUnchecked(i);
                t->area.setPosition (x, y);
                t->line = totalLines;
                x += t->area.getWidth();
                h = jmax (h, t->area.getHeight());

                const Token* nextTok = tokens[i + 1];

                if (nextTok == 0)
                    break;

                if (t->isNewLine || ((! nextTok->isWhitespace) && x + nextTok->area.getWidth() > maxWidth))
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

        void setLastLineHeight (int i, const int height) noexcept
        {
            while (--i >= 0)
            {
                Token* const tok = tokens.getUnchecked (i);

                if (tok->line == totalLines)
                    tok->lineHeight = height;
                else
                    break;
            }
        }

        int getLineWidth (const int lineNumber) const noexcept
        {
            int maxW = 0;

            for (int i = tokens.size(); --i >= 0;)
            {
                const Token* const t = tokens.getUnchecked (i);

                if (t->line == lineNumber && ! t->isWhitespace)
                    maxW = jmax (maxW, t->area.getRight());
            }

            return maxW;
        }

        void addTextRuns (const AttributedString& text)
        {
            Font defaultFont;
            Array<RunAttribute> runAttributes;

            {
                const int stringLength = text.getText().length();
                int rangeStart = 0;
                FontAndColour lastFontAndColour (nullptr);

                // Iterate through every character in the string
                for (int i = 0; i < stringLength; ++i)
                {
                    FontAndColour newFontAndColour (&defaultFont);
                    const int numCharacterAttributes = text.getNumAttributes();

                    for (int j = 0; j < numCharacterAttributes; ++j)
                    {
                        const AttributedString::Attribute* const attr = text.getAttribute (j);

                        // Check if the current character falls within the range of a font attribute
                        if (attr->getFont() != nullptr && (i >= attr->range.getStart()) && (i < attr->range.getEnd()))
                            newFontAndColour.font = attr->getFont();

                        // Check if the current character falls within the range of a foreground colour attribute
                        if (attr->getColour() != nullptr && (i >= attr->range.getStart()) && (i < attr->range.getEnd()))
                            newFontAndColour.colour = *attr->getColour();
                    }

                    if (i > 0 && (newFontAndColour != lastFontAndColour || i == stringLength - 1))
                    {
                        runAttributes.add (RunAttribute (lastFontAndColour,
                                                         Range<int> (rangeStart, (i < stringLength - 1) ? i : (i + 1))));
                        rangeStart = i;
                    }

                    lastFontAndColour = newFontAndColour;
                }
            }

            for (int i = 0; i < runAttributes.size(); ++i)
            {
                const RunAttribute& r = runAttributes.getReference(i);
                appendText (text, r.range, *(r.fontAndColour.font), r.fontAndColour.colour);
            }
        }

        OwnedArray<Token> tokens;
        int totalLines;

        JUCE_DECLARE_NON_COPYABLE (TokenList);
    };
}

//==============================================================================
void GlyphLayout::createStandardLayout (const AttributedString& text)
{
    GlyphLayoutHelpers::TokenList l;
    l.createLayout (text, *this);
}

END_JUCE_NAMESPACE
