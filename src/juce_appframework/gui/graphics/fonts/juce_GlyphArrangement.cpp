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

#include "juce_GlyphArrangement.h"
#include "../contexts/juce_LowLevelGraphicsSoftwareRenderer.h"
#include "../imaging/juce_Image.h"
#include "../../../application/juce_DeletedAtShutdown.h"

#define SHOULD_WRAP(x, wrapwidth)       (((x) - 0.0001f) >= (wrapwidth))

//==============================================================================
class FontGlyphAlphaMap
{
public:
    //==============================================================================
    bool draw (const Graphics& g, float x, const float y) const throw()
    {
        if (bitmap1 == 0)
            return false;

        x += xOrigin;
        const float xFloor = floorf (x);
        const int intX = (int) xFloor;

        g.drawImageAt (((x - xFloor) >= 0.5f && bitmap2 != 0) ? bitmap2 : bitmap1,
                       intX, (int) floorf (y + yOrigin), true);

        return true;
    }


    juce_UseDebuggingNewOperator

private:
    //==============================================================================
    Image* bitmap1;
    Image* bitmap2;
    float xOrigin, yOrigin;
    int lastAccessCount;
    Typeface::Ptr typeface;
    float height, horizontalScale;
    juce_wchar character;

    friend class GlyphCache;

    FontGlyphAlphaMap() throw()
        : bitmap1 (0),
          bitmap2 (0),
          lastAccessCount (0),
          height (0),
          horizontalScale (0),
          character (0)
    {
    }

    ~FontGlyphAlphaMap() throw()
    {
        delete bitmap1;
        delete bitmap2;
    }

    class AlphaBitmapRenderer
    {
        uint8* const data;
        const int stride;
        uint8* lineStart;

        AlphaBitmapRenderer (const AlphaBitmapRenderer&);
        const AlphaBitmapRenderer& operator= (const AlphaBitmapRenderer&);

    public:
        AlphaBitmapRenderer (uint8* const data_,
                             const int stride_) throw()
            : data (data_),
              stride (stride_)
        {
        }

        forcedinline void setEdgeTableYPos (const int y) throw()
        {
            lineStart = data + (stride * y);
        }

        forcedinline void handleEdgeTablePixel (const int x, const int alphaLevel) const throw()
        {
            lineStart [x] = (uint8) alphaLevel;
        }

        forcedinline void handleEdgeTableLine (const int x, int width, const int alphaLevel) const throw()
        {
            uint8* d = lineStart + x;

            while (--width >= 0)
                *d++ = (uint8) alphaLevel;
        }
    };

    Image* createAlphaMapFromPath (const Path& path,
                                   float& topLeftX, float& topLeftY,
                                   float xScale, float yScale,
                                   const float subPixelOffsetX) throw()
    {
        Image* im = 0;

        float px, py, pw, ph;
        path.getBounds (px, py, pw, ph);

        topLeftX = floorf (px * xScale);
        topLeftY = floorf (py * yScale);

        int bitmapWidth = roundFloatToInt (pw * xScale) + 2;
        int bitmapHeight = roundFloatToInt (ph * yScale) + 2;

        im = new Image (Image::SingleChannel, bitmapWidth, bitmapHeight, true);

        EdgeTable edgeTable (0, bitmapHeight, EdgeTable::Oversampling_16times);

        edgeTable.addPath (path, AffineTransform::scale (xScale, yScale)
                                   .translated (subPixelOffsetX - topLeftX, -topLeftY));

        int stride, pixelStride;
        uint8* const pixels = (uint8*) im->lockPixelDataReadWrite (0, 0, bitmapWidth, bitmapHeight, stride, pixelStride);

        jassert (pixelStride == 1);
        AlphaBitmapRenderer renderer (pixels, stride);
        edgeTable.iterate (renderer, 0, 0, bitmapWidth, bitmapHeight, 0);

        im->releasePixelDataReadWrite (pixels);

        return im;
    }

    void generate (Typeface* const face,
                   const juce_wchar character_,
                   const float fontHeight,
                   const float fontHorizontalScale) throw()
    {
        character = character_;
        typeface = face;
        height = fontHeight;
        horizontalScale = fontHorizontalScale;

        const Path* const glyphPath = face->getOutlineForGlyph (character_);

        deleteAndZero (bitmap1);
        deleteAndZero (bitmap2);

        const float fontHScale = fontHeight * fontHorizontalScale;

        if (glyphPath != 0 && ! glyphPath->isEmpty())
        {
            bitmap1 = createAlphaMapFromPath (*glyphPath, xOrigin, yOrigin, fontHScale, fontHeight, 0.0f);

            if (fontHScale < 24.0f)
                bitmap2 = createAlphaMapFromPath (*glyphPath, xOrigin, yOrigin, fontHScale, fontHeight, 0.5f);
        }
        else
        {
            xOrigin = yOrigin = 0;
        }
    }
};


//==============================================================================
static const int defaultNumGlyphsToCache = 120;
class GlyphCache;
static GlyphCache* cacheInstance = 0;

class GlyphCache  : private DeletedAtShutdown
{
public:
    //==============================================================================
    static GlyphCache* getInstance() throw()
    {
        if (cacheInstance == 0)
            cacheInstance = new GlyphCache();

        return cacheInstance;
    }

    const FontGlyphAlphaMap& getGlyphFor (Typeface* const typeface,
                                          const float fontHeight,
                                          const float fontHorizontalScale,
                                          const juce_wchar character) throw()
    {
        ++accessCounter;

        int oldestCounter = INT_MAX;
        int oldestIndex = 0;

        for (int i = numGlyphs; --i >= 0;)
        {
            FontGlyphAlphaMap& g = glyphs[i];

            if (g.character == character
                 && g.height == fontHeight
                 && g.typeface->hashCode() == typeface->hashCode()
                 && g.horizontalScale == fontHorizontalScale)
            {
                g.lastAccessCount = accessCounter;
                ++hits;
                return g;
            }

            if (oldestCounter > g.lastAccessCount)
            {
                oldestCounter = g.lastAccessCount;
                oldestIndex = i;
            }
        }

        ++misses;

        if (hits + misses > (numGlyphs << 4))
        {
            if (misses * 2 > hits)
                setCacheSize (numGlyphs + 32);

            hits = 0;
            misses = 0;
            oldestIndex = 0;
        }

        FontGlyphAlphaMap& oldest = glyphs [oldestIndex];
        oldest.lastAccessCount = accessCounter;

        oldest.generate (typeface,
                         character,
                         fontHeight,
                         fontHorizontalScale);

        return oldest;
    }

    void setCacheSize (const int num) throw()
    {
        if (numGlyphs != num)
        {
            numGlyphs = num;

            if (glyphs != 0)
                delete[] glyphs;

            glyphs = new FontGlyphAlphaMap [numGlyphs];

            hits = 0;
            misses = 0;
        }
    }

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    FontGlyphAlphaMap* glyphs;
    int numGlyphs, accessCounter;
    int hits, misses;

    GlyphCache() throw()
        : glyphs (0),
          numGlyphs (0),
          accessCounter (0)
    {
        setCacheSize (defaultNumGlyphsToCache);
    }

    ~GlyphCache() throw()
    {
        delete[] glyphs;

        jassert (cacheInstance == this);
        cacheInstance = 0;
    }

    GlyphCache (const GlyphCache&);
    const GlyphCache& operator= (const GlyphCache&);
};


//==============================================================================
PositionedGlyph::PositionedGlyph() throw()
{
}

void PositionedGlyph::draw (const Graphics& g) const throw()
{
    if (! glyphInfo->isWhitespace())
    {
        if (fontHeight < 100.0f && fontHeight > 0.1f && ! g.isVectorDevice())
        {
            const FontGlyphAlphaMap& alphaMap
                = GlyphCache::getInstance()->getGlyphFor (glyphInfo->getTypeface(),
                                                          fontHeight,
                                                          fontHorizontalScale,
                                                          getCharacter());

            alphaMap.draw (g, x, y);
        }
        else
        {
            // that's a bit of a dodgy size, isn't it??
            jassert (fontHeight > 0.0f && fontHeight < 4000.0f);

            draw (g, AffineTransform::identity);
        }
    }
}

void PositionedGlyph::draw (const Graphics& g,
                            const AffineTransform& transform) const throw()
{
    if (! glyphInfo->isWhitespace())
    {
        g.fillPath (glyphInfo->getPath(),
                    AffineTransform::scale (fontHeight * fontHorizontalScale, fontHeight)
                        .translated (x, y)
                        .followedBy (transform));
    }
}

void PositionedGlyph::createPath (Path& path) const throw()
{
    if (! glyphInfo->isWhitespace())
    {
        path.addPath (glyphInfo->getPath(),
                      AffineTransform::scale (fontHeight * fontHorizontalScale, fontHeight)
                        .translated (x, y));
    }
}

bool PositionedGlyph::hitTest (float px, float py) const throw()
{
    if (px >= getLeft() && px < getRight()
        && py >= getTop() && py < getBottom()
        && fontHeight > 0.0f
        && ! glyphInfo->isWhitespace())
    {
        AffineTransform::translation (-x, -y)
            .scaled (1.0f / (fontHeight * fontHorizontalScale), 1.0f / fontHeight)
            .transformPoint (px, py);

        return glyphInfo->getPath().contains (px, py);
    }

    return false;
}

void PositionedGlyph::moveBy (const float deltaX,
                              const float deltaY) throw()
{
    x += deltaX;
    y += deltaY;
}


//==============================================================================
GlyphArrangement::GlyphArrangement() throw()
    : numGlyphs (0),
      numAllocated (0),
      glyphs (0)
{
}

GlyphArrangement::GlyphArrangement (const GlyphArrangement& other) throw()
    : numGlyphs (0),
      numAllocated (0),
      glyphs (0)
{
    addGlyphArrangement (other);
}

const GlyphArrangement& GlyphArrangement::operator= (const GlyphArrangement& other) throw()
{
    if (this != &other)
    {
        clear();
        addGlyphArrangement (other);
    }

    return *this;
}

GlyphArrangement::~GlyphArrangement() throw()
{
    clear();
    juce_free (glyphs);
}

//==============================================================================
void GlyphArrangement::ensureNumGlyphsAllocated (const int minGlyphs) throw()
{
    if (numAllocated <= minGlyphs)
    {
        numAllocated = minGlyphs + 2;

        if (glyphs == 0)
            glyphs = (PositionedGlyph*) juce_malloc (numAllocated * sizeof (PositionedGlyph));
        else
            glyphs = (PositionedGlyph*) juce_realloc (glyphs, numAllocated * sizeof (PositionedGlyph));
    }
}

void GlyphArrangement::incGlyphRefCount (const int i) const throw()
{
    jassert (i >= 0 && i < numGlyphs);

    if (glyphs[i].glyphInfo != 0 && glyphs[i].glyphInfo->getTypeface() != 0)
        glyphs[i].glyphInfo->getTypeface()->incReferenceCount();
}

void GlyphArrangement::decGlyphRefCount (const int i) const throw()
{
    if (glyphs[i].glyphInfo != 0 && glyphs[i].glyphInfo->getTypeface() != 0)
        glyphs[i].glyphInfo->getTypeface()->decReferenceCount();
}

void GlyphArrangement::clear() throw()
{
    for (int i = numGlyphs; --i >= 0;)
        decGlyphRefCount (i);

    numGlyphs = 0;
}

PositionedGlyph& GlyphArrangement::getGlyph (const int index) const throw()
{
    jassert (index >= 0 && index < numGlyphs);

    return glyphs [index];
}

//==============================================================================
void GlyphArrangement::addGlyphArrangement (const GlyphArrangement& other) throw()
{
    ensureNumGlyphsAllocated (numGlyphs + other.numGlyphs);

    memcpy (glyphs + numGlyphs, other.glyphs,
            other.numGlyphs * sizeof (PositionedGlyph));

    for (int i = other.numGlyphs; --i >= 0;)
        incGlyphRefCount (numGlyphs++);
}

void GlyphArrangement::removeLast() throw()
{
    if (numGlyphs > 0)
        decGlyphRefCount (--numGlyphs);
}

void GlyphArrangement::removeRangeOfGlyphs (int startIndex, const int num) throw()
{
    jassert (startIndex >= 0);

    if (startIndex < 0)
        startIndex = 0;

    if (num < 0 || startIndex + num >= numGlyphs)
    {
        while (numGlyphs > startIndex)
            removeLast();
    }
    else if (num > 0)
    {
        int i;
        for (i = startIndex; i < startIndex + num; ++i)
            decGlyphRefCount (i);

        for (i = numGlyphs - (startIndex + num); --i >= 0;)
        {
            glyphs [startIndex] = glyphs [startIndex + num];
            ++startIndex;
        }

        numGlyphs -= num;
    }
}

//==============================================================================
void GlyphArrangement::addLineOfText (const Font& font,
                                      const String& text,
                                      const float xOffset,
                                      const float yOffset) throw()
{
    addCurtailedLineOfText (font, text,
                            xOffset, yOffset,
                            1.0e10f, false);
}

void GlyphArrangement::addCurtailedLineOfText (const Font& font,
                                               const String& text,
                                               float xOffset,
                                               const float yOffset,
                                               const float maxWidthPixels,
                                               const bool useEllipsis) throw()
{
    const int textLen = text.length();

    if (textLen > 0)
    {
        ensureNumGlyphsAllocated (numGlyphs + textLen + 3); // extra chars for ellipsis

        Typeface* const typeface = font.getTypeface();
        const float fontHeight = font.getHeight();
        const float ascent = font.getAscent();
        const float fontHorizontalScale = font.getHorizontalScale();
        const float heightTimesScale = fontHorizontalScale * fontHeight;
        const float kerningFactor = font.getExtraKerningFactor();
        const float startX = xOffset;

        const juce_wchar* const unicodeText = (const juce_wchar*) text;

        for (int i = 0; i < textLen; ++i)
        {
            const TypefaceGlyphInfo* const glyph = typeface->getGlyph (unicodeText[i]);

            if (glyph != 0)
            {
                jassert (numAllocated > numGlyphs);

                ensureNumGlyphsAllocated (numGlyphs);
                PositionedGlyph& pg = glyphs [numGlyphs];
                pg.glyphInfo = glyph;
                pg.x = xOffset;
                pg.y = yOffset;
                pg.w = heightTimesScale * glyph->getHorizontalSpacing (0);
                pg.fontHeight = fontHeight;
                pg.fontAscent = ascent;
                pg.fontHorizontalScale = fontHorizontalScale;
                pg.isUnderlined = font.isUnderlined();

                xOffset += heightTimesScale * (kerningFactor + glyph->getHorizontalSpacing (unicodeText [i + 1]));

                if (xOffset - startX > maxWidthPixels + 1.0f)
                {
                    // curtail the string if it's too wide..

                    if (useEllipsis && textLen > 3 && numGlyphs >= 3)
                        appendEllipsis (font, startX + maxWidthPixels);

                    break;
                }
                else
                {
                    if (glyph->getTypeface() != 0)
                        glyph->getTypeface()->incReferenceCount();

                    ++numGlyphs;
                }
            }
        }
    }
}

void GlyphArrangement::appendEllipsis (const Font& font, const float maxXPixels) throw()
{
    const TypefaceGlyphInfo* const dotGlyph = font.getTypeface()->getGlyph (T('.'));

    if (dotGlyph != 0)
    {
        if (numGlyphs > 0)
        {
            PositionedGlyph& glyph = glyphs [numGlyphs - 1];
            const float fontHeight = glyph.fontHeight;
            const float fontHorizontalScale = glyph.fontHorizontalScale;
            const float fontAscent = glyph.fontAscent;

            const float dx = fontHeight * fontHorizontalScale
                                * (font.getExtraKerningFactor() + dotGlyph->getHorizontalSpacing (T('.')));

            float xOffset = 0.0f, yOffset = 0.0f;

            for (int dotPos = 3; --dotPos >= 0 && numGlyphs > 0;)
            {
                removeLast();

                jassert (numAllocated > numGlyphs);
                PositionedGlyph& pg = glyphs [numGlyphs];
                xOffset = pg.x;
                yOffset = pg.y;

                if (numGlyphs == 0 || xOffset + dx * 3 <= maxXPixels)
                    break;
            }

            for (int i = 3; --i >= 0;)
            {
                jassert (numAllocated > numGlyphs);

                ensureNumGlyphsAllocated (numGlyphs);
                PositionedGlyph& pg = glyphs [numGlyphs];
                pg.glyphInfo = dotGlyph;
                pg.x = xOffset;
                pg.y = yOffset;
                pg.w = dx;
                pg.fontHeight = fontHeight;
                pg.fontAscent = fontAscent;
                pg.fontHorizontalScale = fontHorizontalScale;
                pg.isUnderlined = font.isUnderlined();

                xOffset += dx;

                if (dotGlyph->getTypeface() != 0)
                    dotGlyph->getTypeface()->incReferenceCount();

                ++numGlyphs;
            }
        }
    }
}

void GlyphArrangement::addJustifiedText (const Font& font,
                                         const String& text,
                                         float x, float y,
                                         const float maxLineWidth,
                                         const Justification& horizontalLayout) throw()
{
    int lineStartIndex = numGlyphs;
    addLineOfText (font, text, x, y);

    const float originalY = y;

    while (lineStartIndex < numGlyphs)
    {
        int i = lineStartIndex;

        if (glyphs[i].getCharacter() != T('\n') && glyphs[i].getCharacter() != T('\r'))
            ++i;

        const float lineMaxX = glyphs [lineStartIndex].getLeft() + maxLineWidth;
        int lastWordBreakIndex = -1;

        while (i < numGlyphs)
        {
            PositionedGlyph& pg = glyphs[i];
            const juce_wchar c = pg.getCharacter();

            if (c == T('\r') || c == T('\n'))
            {
                ++i;

                if (c == T('\r') && i < numGlyphs && glyphs [i].getCharacter() == T('\n'))
                    ++i;

                break;
            }
            else if (pg.isWhitespace())
            {
                lastWordBreakIndex = i + 1;
            }
            else if (SHOULD_WRAP (pg.getRight(), lineMaxX))
            {
                if (lastWordBreakIndex >= 0)
                    i = lastWordBreakIndex;

                break;
            }

            ++i;
        }

        const float currentLineStartX = glyphs [lineStartIndex].getLeft();
        float currentLineEndX = currentLineStartX;

        for (int j = i; --j >= lineStartIndex;)
        {
            if (! glyphs[j].isWhitespace())
            {
                currentLineEndX = glyphs[j].getRight();
                break;
            }
        }

        float deltaX = 0.0f;

        if (horizontalLayout.testFlags (Justification::horizontallyJustified))
            spreadOutLine (lineStartIndex, i - lineStartIndex, maxLineWidth);
        else if (horizontalLayout.testFlags (Justification::horizontallyCentred))
            deltaX = (maxLineWidth - (currentLineEndX - currentLineStartX)) * 0.5f;
        else if (horizontalLayout.testFlags (Justification::right))
            deltaX = maxLineWidth - (currentLineEndX - currentLineStartX);

        moveRangeOfGlyphs (lineStartIndex, i - lineStartIndex,
                           x + deltaX - currentLineStartX, y - originalY);

        lineStartIndex = i;

        y += font.getHeight();
    }
}

void GlyphArrangement::addFittedText (const Font& f,
                                      const String& text,
                                      float x, float y,
                                      float width, float height,
                                      const Justification& layout,
                                      int maximumLines,
                                      const float minimumHorizontalScale) throw()
{
    // doesn't make much sense if this is outside a sensible range of 0.5 to 1.0
    jassert (minimumHorizontalScale > 0 && minimumHorizontalScale <= 1.0f);

    if (text.containsAnyOf (T("\r\n")))
    {
        GlyphArrangement ga;
        ga.addJustifiedText (f, text, x, y, width, layout);

        float l, t, r, b;
        ga.getBoundingBox (0, -1, l, t, r, b, false);

        float dy = y - t;

        if (layout.testFlags (Justification::verticallyCentred))
            dy += (height - (b - t)) * 0.5f;
        else if (layout.testFlags (Justification::bottom))
            dy += height - (b - t);

        ga.moveRangeOfGlyphs (0, -1, 0.0f, dy);

        addGlyphArrangement (ga);

        return;
    }

    int startIndex = numGlyphs;
    addLineOfText (f, text.trim(), x, y);

    if (numGlyphs > startIndex)
    {
        float lineWidth = glyphs[numGlyphs - 1].getRight() - glyphs[startIndex].getLeft();

        if (lineWidth <= 0)
            return;

        if (lineWidth * minimumHorizontalScale < width)
        {
            if (lineWidth > width)
            {
                stretchRangeOfGlyphs (startIndex, numGlyphs - startIndex,
                                      width / lineWidth);

            }

            justifyGlyphs (startIndex, numGlyphs - startIndex,
                           x, y, width, height, layout);
        }
        else if (maximumLines <= 1)
        {
            const float ratio = jmax (minimumHorizontalScale, width / lineWidth);

            stretchRangeOfGlyphs (startIndex, numGlyphs - startIndex, ratio);

            while (numGlyphs > 0 && glyphs [numGlyphs - 1].x + glyphs [numGlyphs - 1].w >= x + width)
                removeLast();

            appendEllipsis (f, x + width);

            justifyGlyphs (startIndex, numGlyphs - startIndex,
                           x, y, width, height, layout);
        }
        else
        {
            Font font (f);

            String txt (text.trim());
            const int length = txt.length();
            int numLines = 1;
            const int originalStartIndex = startIndex;

            if (length <= 12 && ! txt.containsAnyOf (T(" -\t\r\n")))
                maximumLines = 1;

            maximumLines = jmin (maximumLines, length);

            while (numLines < maximumLines)
            {
                ++numLines;

                const float newFontHeight = height / (float)numLines;

                if (newFontHeight < 8.0f)
                    break;

                if (newFontHeight < font.getHeight())
                {
                    font.setHeight (newFontHeight);

                    while (numGlyphs > startIndex)
                        removeLast();

                    addLineOfText (font, txt, x, y);

                    lineWidth = glyphs[numGlyphs - 1].getRight() - glyphs[startIndex].getLeft();
                }

                if (numLines > lineWidth / width)
                    break;
            }

            if (numLines < 1)
                numLines = 1;

            float lineY = y;
            float widthPerLine = lineWidth / numLines;
            int lastLineStartIndex = 0;

            for (int line = 0; line < numLines; ++line)
            {
                int i = startIndex;
                lastLineStartIndex = i;
                float lineStartX = glyphs[startIndex].getLeft();

                while (i < numGlyphs)
                {
                    lineWidth = (glyphs[i].getRight() - lineStartX);

                    if (lineWidth > widthPerLine)
                    {
                        // got to a point where the line's too long, so skip forward to find a
                        // good place to break it..
                        const int searchStartIndex = i;

                        while (i < numGlyphs)
                        {
                            if ((glyphs[i].getRight() - lineStartX) * minimumHorizontalScale < width)
                            {
                                if (glyphs[i].isWhitespace()
                                     || glyphs[i].getCharacter() == T('-'))
                                {
                                    ++i;
                                    break;
                                }
                            }
                            else
                            {
                                // can't find a suitable break, so try looking backwards..
                                i = searchStartIndex;

                                for (int back = 1; back < jmin (5, i - startIndex - 1); ++back)
                                {
                                    if (glyphs[i - back].isWhitespace()
                                         || glyphs[i - back].getCharacter() == T('-'))
                                    {
                                        i -= back - 1;
                                        break;
                                    }
                                }

                                break;
                            }

                            ++i;
                        }

                        break;
                    }

                    ++i;
                }

                int wsStart = i;
                while (wsStart > 0 && glyphs[wsStart - 1].isWhitespace())
                    --wsStart;

                int wsEnd = i;

                while (wsEnd < numGlyphs && glyphs[wsEnd].isWhitespace())
                    ++wsEnd;

                removeRangeOfGlyphs (wsStart, wsEnd - wsStart);
                i = jmax (wsStart, startIndex + 1);

                lineWidth = glyphs[i - 1].getRight() - lineStartX;

                if (lineWidth > width)
                {
                    stretchRangeOfGlyphs (startIndex, i - startIndex,
                                          width / lineWidth);
                }

                justifyGlyphs (startIndex, i - startIndex,
                               x, lineY, width, font.getHeight(),
                               layout.getOnlyHorizontalFlags() | Justification::verticallyCentred);

                startIndex = i;
                lineY += font.getHeight();

                if (startIndex >= numGlyphs)
                    break;
            }

            if (startIndex < numGlyphs)
            {
                while (numGlyphs > startIndex)
                    removeLast();

                if (startIndex - originalStartIndex > 4)
                {
                    const float lineStartX = glyphs[lastLineStartIndex].getLeft();
                    appendEllipsis (font, lineStartX + width);

                    lineWidth = glyphs[startIndex - 1].getRight() - lineStartX;

                    if (lineWidth > width)
                    {
                        stretchRangeOfGlyphs (lastLineStartIndex, startIndex - lastLineStartIndex,
                                              width / lineWidth);
                    }

                    justifyGlyphs (lastLineStartIndex, startIndex - lastLineStartIndex,
                                   x, lineY - font.getHeight(), width, font.getHeight(),
                                   layout.getOnlyHorizontalFlags() | Justification::verticallyCentred);
                }

                startIndex = numGlyphs;
            }

            justifyGlyphs (originalStartIndex, startIndex - originalStartIndex,
                           x, y, width, height, layout.getFlags() & ~Justification::horizontallyJustified);
        }
    }
}

//==============================================================================
void GlyphArrangement::moveRangeOfGlyphs (int startIndex, int num,
                                          const float dx, const float dy) throw()
{
    jassert (startIndex >= 0);

    if (dx != 0.0f || dy != 0.0f)
    {
        if (num < 0 || startIndex + num > numGlyphs)
            num = numGlyphs - startIndex;

        while (--num >= 0)
        {
            jassert (startIndex >= 0 && startIndex <= numGlyphs);
            glyphs [startIndex++].moveBy (dx, dy);
        }
    }
}

void GlyphArrangement::stretchRangeOfGlyphs (int startIndex, int num,
                                             const float horizontalScaleFactor) throw()
{
    jassert (startIndex >= 0);

    if (num < 0 || startIndex + num > numGlyphs)
        num = numGlyphs - startIndex;

    if (num > 0)
    {
        const float xAnchor = glyphs[startIndex].getLeft();

        while (--num >= 0)
        {
            jassert (startIndex >= 0 && startIndex <= numGlyphs);
            PositionedGlyph& pg = glyphs[startIndex++];

            pg.x = xAnchor + (pg.x - xAnchor) * horizontalScaleFactor;
            pg.fontHorizontalScale *= horizontalScaleFactor;
            pg.w *= horizontalScaleFactor;
        }
    }
}

void GlyphArrangement::getBoundingBox (int startIndex, int num,
                                       float& left,
                                       float& top,
                                       float& right,
                                       float& bottom,
                                       const bool includeWhitespace) const throw()
{
    jassert (startIndex >= 0);

    if (num < 0 || startIndex + num > numGlyphs)
        num = numGlyphs - startIndex;

    left = 0.0f;
    top = 0.0f;
    right = 0.0f;
    bottom = 0.0f;
    bool isFirst = true;

    while (--num >= 0)
    {
        const PositionedGlyph& pg = glyphs [startIndex++];

        if (includeWhitespace || ! pg.isWhitespace())
        {
            if (isFirst)
            {
                isFirst = false;
                left    = pg.getLeft();
                top     = pg.getTop();
                right   = pg.getRight();
                bottom  = pg.getBottom();
            }
            else
            {
                left    = jmin (left, pg.getLeft());
                top     = jmin (top, pg.getTop());
                right   = jmax (right, pg.getRight());
                bottom  = jmax (bottom, pg.getBottom());
            }
        }
    }
}

void GlyphArrangement::justifyGlyphs (const int startIndex,
                                      const int num,
                                      const float x, const float y,
                                      const float width, const float height,
                                      const Justification& justification) throw()
{
    jassert (num >= 0 && startIndex >= 0);

    if (numGlyphs > 0 && num > 0)
    {
        float left, top, right, bottom;
        getBoundingBox (startIndex, num, left, top, right, bottom,
                        ! justification.testFlags (Justification::horizontallyJustified
                                                    | Justification::horizontallyCentred));

        float deltaX = 0.0f;

        if (justification.testFlags (Justification::horizontallyJustified))
            deltaX = x - left;
        else if (justification.testFlags (Justification::horizontallyCentred))
            deltaX = x + (width - (right - left)) * 0.5f - left;
        else if (justification.testFlags (Justification::right))
            deltaX = (x + width) - right;
        else
            deltaX = x - left;

        float deltaY = 0.0f;

        if (justification.testFlags (Justification::top))
            deltaY = y - top;
        else if (justification.testFlags (Justification::bottom))
            deltaY = (y + height) - bottom;
        else
            deltaY = y + (height - (bottom - top)) * 0.5f - top;

        moveRangeOfGlyphs (startIndex, num, deltaX, deltaY);

        if (justification.testFlags (Justification::horizontallyJustified))
        {
            int lineStart = 0;
            float baseY = glyphs [startIndex].getBaselineY();

            int i;
            for (i = 0; i < num; ++i)
            {
                const float glyphY = glyphs [startIndex + i].getBaselineY();

                if (glyphY != baseY)
                {
                    spreadOutLine (startIndex + lineStart, i - lineStart, width);

                    lineStart = i;
                    baseY = glyphY;
                }
            }

            if (i > lineStart)
                spreadOutLine (startIndex + lineStart, i - lineStart, width);
        }
    }
}

void GlyphArrangement::spreadOutLine (const int start, const int num, const float targetWidth) throw()
{
    if (start + num < numGlyphs
         && glyphs [start + num - 1].getCharacter() != T('\r')
         && glyphs [start + num - 1].getCharacter() != T('\n'))
    {
        int numSpaces = 0;
        int spacesAtEnd = 0;

        for (int i = 0; i < num; ++i)
        {
            if (glyphs [start + i].isWhitespace())
            {
                ++spacesAtEnd;
                ++numSpaces;
            }
            else
            {
                spacesAtEnd = 0;
            }
        }

        numSpaces -= spacesAtEnd;

        if (numSpaces > 0)
        {
            const float startX = glyphs [start].getLeft();
            const float endX = glyphs [start + num - 1 - spacesAtEnd].getRight();

            const float extraPaddingBetweenWords
                = (targetWidth - (endX - startX)) / (float) numSpaces;

            float deltaX = 0.0f;

            for (int i = 0; i < num; ++i)
            {
                glyphs [start + i].moveBy (deltaX, 0.0);

                if (glyphs [start + i].isWhitespace())
                    deltaX += extraPaddingBetweenWords;
            }
        }
    }
}

//==============================================================================
void GlyphArrangement::draw (const Graphics& g) const throw()
{
    for (int i = 0; i < numGlyphs; ++i)
    {
        glyphs[i].draw (g);

        if (glyphs[i].isUnderlined)
        {
            const float lineThickness = (glyphs[i].fontHeight - glyphs[i].fontAscent) * 0.3f;

            juce_wchar nextChar = 0;

            if (i < numGlyphs - 1
                 && glyphs[i + 1].y == glyphs[i].y)
            {
                nextChar = glyphs[i + 1].glyphInfo->getCharacter();
            }

            g.fillRect (glyphs[i].x,
                        glyphs[i].y + lineThickness * 2.0f,
                        glyphs[i].fontHeight
                         * glyphs[i].fontHorizontalScale
                         * glyphs[i].glyphInfo->getHorizontalSpacing (nextChar),
                        lineThickness);
        }
    }
}

void GlyphArrangement::draw (const Graphics& g, const AffineTransform& transform) const throw()
{
    for (int i = 0; i < numGlyphs; ++i)
    {
        glyphs[i].draw (g, transform);

        if (glyphs[i].isUnderlined)
        {
            const float lineThickness = (glyphs[i].fontHeight - glyphs[i].fontAscent) * 0.3f;

            juce_wchar nextChar = 0;

            if (i < numGlyphs - 1
                 && glyphs[i + 1].y == glyphs[i].y)
            {
                nextChar = glyphs[i + 1].glyphInfo->getCharacter();
            }

            Path p;
            p.addLineSegment (glyphs[i].x,
                              glyphs[i].y + lineThickness * 2.5f,
                              glyphs[i].x + glyphs[i].fontHeight
                                             * glyphs[i].fontHorizontalScale
                                             * glyphs[i].glyphInfo->getHorizontalSpacing (nextChar),
                              glyphs[i].y + lineThickness * 2.5f,
                              lineThickness);

            g.fillPath (p, transform);
        }
    }
}

void GlyphArrangement::createPath (Path& path) const throw()
{
    for (int i = 0; i < numGlyphs; ++i)
        glyphs[i].createPath (path);
}

int GlyphArrangement::findGlyphIndexAt (float x, float y) const throw()
{
    for (int i = 0; i < numGlyphs; ++i)
        if (glyphs[i].hitTest (x, y))
            return i;

    return -1;
}

END_JUCE_NAMESPACE
