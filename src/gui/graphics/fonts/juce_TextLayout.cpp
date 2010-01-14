/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

#include "../../../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_TextLayout.h"
#include "../contexts/juce_Graphics.h"


//==============================================================================
class TextLayout::Token
{
public:
    String text;
    Font font;
    int x, y, w, h;
    int line, lineHeight;
    bool isWhitespace, isNewLine;

    Token (const String& t,
           const Font& f,
           const bool isWhitespace_) throw()
        : text (t),
          font (f),
          x(0),
          y(0),
          isWhitespace (isWhitespace_)
    {
        w = font.getStringWidth (t);
        h = roundToInt (f.getHeight());
        isNewLine = t.containsAnyOf (T("\r\n"));
    }

    Token (const Token& other) throw()
        : text (other.text),
          font (other.font),
          x (other.x),
          y (other.y),
          w (other.w),
          h (other.h),
          line (other.line),
          lineHeight (other.lineHeight),
          isWhitespace (other.isWhitespace),
          isNewLine (other.isNewLine)
    {
    }

    ~Token() throw()
    {
    }

    void draw (Graphics& g,
               const int xOffset,
               const int yOffset) throw()
    {
        if (! isWhitespace)
        {
            g.setFont (font);
            g.drawSingleLineText (text.trimEnd(),
                                  xOffset + x,
                                  yOffset + y + (lineHeight - h)
                                    + roundToInt (font.getAscent()));
        }
    }

    juce_UseDebuggingNewOperator
};


//==============================================================================
TextLayout::TextLayout() throw()
    : totalLines (0)
{
    tokens.ensureStorageAllocated (64);
}

TextLayout::TextLayout (const String& text,
                        const Font& font) throw()
    : totalLines (0)
{
    tokens.ensureStorageAllocated (64);
    appendText (text, font);
}

TextLayout::TextLayout (const TextLayout& other) throw()
    : totalLines (0)
{
    *this = other;
}

const TextLayout& TextLayout::operator= (const TextLayout& other) throw()
{
    if (this != &other)
    {
        clear();

        totalLines = other.totalLines;

        for (int i = 0; i < other.tokens.size(); ++i)
            tokens.add (new Token (*other.tokens.getUnchecked(i)));
    }

    return *this;
}

TextLayout::~TextLayout() throw()
{
    clear();
}

//==============================================================================
void TextLayout::clear() throw()
{
    tokens.clear();
    totalLines = 0;
}

void TextLayout::appendText (const String& text,
                             const Font& font) throw()
{
    const tchar* t = text;
    String currentString;
    int lastCharType = 0;

    for (;;)
    {
        const tchar c = *t++;
        if (c == 0)
            break;

        int charType;
        if (c == T('\r') || c == T('\n'))
        {
            charType = 0;
        }
        else if (CharacterFunctions::isWhitespace (c))
        {
            charType = 2;
        }
        else
        {
            charType = 1;
        }

        if (charType == 0 || charType != lastCharType)
        {
            if (currentString.isNotEmpty())
            {
                tokens.add (new Token (currentString, font,
                                       lastCharType == 2 || lastCharType == 0));
            }

            currentString = String::charToString (c);

            if (c == T('\r') && *t == T('\n'))
                currentString += *t++;
        }
        else
        {
            currentString += c;
        }

        lastCharType = charType;
    }

    if (currentString.isNotEmpty())
        tokens.add (new Token (currentString, font, lastCharType == 2));
}

void TextLayout::setText (const String& text, const Font& font) throw()
{
    clear();
    appendText (text, font);
}

//==============================================================================
void TextLayout::layout (int maxWidth,
                         const Justification& justification,
                         const bool attemptToBalanceLineLengths) throw()
{
    if (attemptToBalanceLineLengths)
    {
        const int originalW = maxWidth;
        int bestWidth = maxWidth;
        float bestLineProportion = 0.0f;

        while (maxWidth > originalW / 2)
        {
            layout (maxWidth, justification, false);

            if (getNumLines() <= 1)
                return;

            const int lastLineW = getLineWidth (getNumLines() - 1);
            const int lastButOneLineW = getLineWidth (getNumLines() - 2);

            const float prop = lastLineW / (float) lastButOneLineW;

            if (prop > 0.9f)
                return;

            if (prop > bestLineProportion)
            {
                bestLineProportion = prop;
                bestWidth = maxWidth;
            }

            maxWidth -= 10;
        }

        layout (bestWidth, justification, false);
    }
    else
    {
        int x = 0;
        int y = 0;
        int h = 0;
        totalLines = 0;
        int i;

        for (i = 0; i < tokens.size(); ++i)
        {
            Token* const t = tokens.getUnchecked(i);
            t->x = x;
            t->y = y;
            t->line = totalLines;
            x += t->w;
            h = jmax (h, t->h);

            const Token* nextTok = tokens [i + 1];

            if (nextTok == 0)
                break;

            if (t->isNewLine || ((! nextTok->isWhitespace) && x + nextTok->w > maxWidth))
            {
                // finished a line, so go back and update the heights of the things on it
                for (int j = i; j >= 0; --j)
                {
                    Token* const tok = tokens.getUnchecked(j);

                    if (tok->line == totalLines)
                        tok->lineHeight = h;
                    else
                        break;
                }

                x = 0;
                y += h;
                h = 0;
                ++totalLines;
            }
        }

        // finished a line, so go back and update the heights of the things on it
        for (int j = jmin (i, tokens.size() - 1); j >= 0; --j)
        {
            Token* const t = tokens.getUnchecked(j);

            if (t->line == totalLines)
                t->lineHeight = h;
            else
                break;
        }

        ++totalLines;

        if (! justification.testFlags (Justification::left))
        {
            int totalW = getWidth();

            for (i = totalLines; --i >= 0;)
            {
                const int lineW = getLineWidth (i);

                int dx = 0;
                if (justification.testFlags (Justification::horizontallyCentred))
                    dx = (totalW - lineW) / 2;
                else if (justification.testFlags (Justification::right))
                    dx = totalW - lineW;

                for (int j = tokens.size(); --j >= 0;)
                {
                    Token* const t = tokens.getUnchecked(j);

                    if (t->line == i)
                        t->x += dx;
                }
            }
        }
    }
}

//==============================================================================
int TextLayout::getLineWidth (const int lineNumber) const throw()
{
    int maxW = 0;

    for (int i = tokens.size(); --i >= 0;)
    {
        const Token* const t = tokens.getUnchecked(i);

        if (t->line == lineNumber && ! t->isWhitespace)
            maxW = jmax (maxW, t->x + t->w);
    }

    return maxW;
}

int TextLayout::getWidth() const throw()
{
    int maxW = 0;

    for (int i = tokens.size(); --i >= 0;)
    {
        const Token* const t = tokens.getUnchecked(i);
        if (! t->isWhitespace)
            maxW = jmax (maxW, t->x + t->w);
    }

    return maxW;
}

int TextLayout::getHeight() const throw()
{
    int maxH = 0;

    for (int i = tokens.size(); --i >= 0;)
    {
        const Token* const t = tokens.getUnchecked(i);

        if (! t->isWhitespace)
            maxH = jmax (maxH, t->y + t->h);
    }

    return maxH;
}

//==============================================================================
void TextLayout::draw (Graphics& g,
                       const int xOffset,
                       const int yOffset) const throw()
{
    for (int i = tokens.size(); --i >= 0;)
        tokens.getUnchecked(i)->draw (g, xOffset, yOffset);
}

void TextLayout::drawWithin (Graphics& g,
                             int x, int y, int w, int h,
                             const Justification& justification) const throw()
{
    justification.applyToRectangle (x, y, getWidth(), getHeight(),
                                    x, y, w, h);

    draw (g, x, y);
}


END_JUCE_NAMESPACE
