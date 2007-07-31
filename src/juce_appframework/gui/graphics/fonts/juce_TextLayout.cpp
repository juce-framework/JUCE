/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#include "../../../../juce_core/basics/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_TextLayout.h"
#include "../contexts/juce_Graphics.h"


//==============================================================================
class TextLayoutToken
{
public:
    String text;
    Font font;
    int x, y, w, h;
    int line, lineHeight;
    bool isWhitespace, isNewLine;

    TextLayoutToken (const String& t,
                     const Font& f,
                     const bool isWhitespace_) throw()
        : text (t),
          font (f),
          x(0),
          y(0),
          isWhitespace (isWhitespace_)
    {
        w = font.getStringWidth (t);
        h = roundFloatToInt (f.getHeight());
        isNewLine = t.containsAnyOf (T("\r\n"));
    }

    TextLayoutToken (const TextLayoutToken& other) throw()
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

    ~TextLayoutToken() throw()
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
                                    + roundFloatToInt (font.getAscent()));
        }
    }

    juce_UseDebuggingNewOperator
};


//==============================================================================
TextLayout::TextLayout() throw()
    : tokens (64),
      totalLines (0)
{
}

TextLayout::TextLayout (const String& text,
                        const Font& font) throw()
    : tokens (64),
      totalLines (0)
{
    appendText (text, font);
}

TextLayout::TextLayout (const TextLayout& other) throw()
    : tokens (64),
      totalLines (0)
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
            tokens.add (new TextLayoutToken (*(const TextLayoutToken*)(other.tokens.getUnchecked(i))));
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
    for (int i = tokens.size(); --i >= 0;)
    {
        TextLayoutToken* const t = (TextLayoutToken*)tokens.getUnchecked(i);
        delete t;
    }

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
                tokens.add (new TextLayoutToken (currentString, font,
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
        tokens.add (new TextLayoutToken (currentString,
                                         font,
                                         lastCharType == 2));
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
            TextLayoutToken* const t = (TextLayoutToken*)tokens.getUnchecked(i);
            t->x = x;
            t->y = y;
            t->line = totalLines;
            x += t->w;
            h = jmax (h, t->h);

            const TextLayoutToken* nextTok = (TextLayoutToken*) tokens [i + 1];

            if (nextTok == 0)
                break;

            if (t->isNewLine || ((! nextTok->isWhitespace) && x + nextTok->w > maxWidth))
            {
                // finished a line, so go back and update the heights of the things on it
                for (int j = i; j >= 0; --j)
                {
                    TextLayoutToken* const tok = (TextLayoutToken*)tokens.getUnchecked(j);

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
            TextLayoutToken* const t = (TextLayoutToken*) tokens.getUnchecked(j);

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
                    TextLayoutToken* const t = (TextLayoutToken*)tokens.getUnchecked(j);

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
        const TextLayoutToken* const t = (TextLayoutToken*) tokens.getUnchecked(i);

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
        const TextLayoutToken* const t = (TextLayoutToken*) tokens.getUnchecked(i);
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
        const TextLayoutToken* const t = (TextLayoutToken*) tokens.getUnchecked(i);

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
        ((TextLayoutToken*) tokens.getUnchecked(i))->draw (g, xOffset, yOffset);
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
