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

#ifndef __JUCE_RENDERINGHELPERS_JUCEHEADER__
#define __JUCE_RENDERINGHELPERS_JUCEHEADER__

namespace RenderingHelpers
{

//==============================================================================
/** Holds either a simple integer translation, or an affine transform.
*/
class TranslationOrTransform
{
public:
    TranslationOrTransform (int xOffset_, int yOffset_) noexcept
        : xOffset (xOffset_), yOffset (yOffset_), isOnlyTranslated (true)
    {
    }

    TranslationOrTransform (const TranslationOrTransform& other) noexcept
        : complexTransform (other.complexTransform),
          xOffset (other.xOffset), yOffset (other.yOffset),
          isOnlyTranslated (other.isOnlyTranslated)
    {
    }

    AffineTransform getTransform() const noexcept
    {
        return isOnlyTranslated ? AffineTransform::translation ((float) xOffset, (float) yOffset)
                                : complexTransform;
    }

    AffineTransform getTransformWith (const AffineTransform& userTransform) const noexcept
    {
        return isOnlyTranslated ? userTransform.translated ((float) xOffset, (float) yOffset)
                                : userTransform.followedBy (complexTransform);
    }

    void setOrigin (const int x, const int y) noexcept
    {
        if (isOnlyTranslated)
        {
            xOffset += x;
            yOffset += y;
        }
        else
        {
            complexTransform = AffineTransform::translation ((float) x, (float) y)
                                               .followedBy (complexTransform);
        }
    }

    void addTransform (const AffineTransform& t) noexcept
    {
        if (isOnlyTranslated
             && t.isOnlyTranslation()
             && isIntegerTranslation (t))
        {
            xOffset += (int) t.getTranslationX();
            yOffset += (int) t.getTranslationY();
        }
        else
        {
            complexTransform = getTransformWith (t);
            isOnlyTranslated = false;
        }
    }

    float getScaleFactor() const noexcept
    {
        return isOnlyTranslated ? 1.0f : complexTransform.getScaleFactor();
    }

    void moveOriginInDeviceSpace (const int dx, const int dy) noexcept
    {
        if (isOnlyTranslated)
        {
            xOffset += dx;
            yOffset += dy;
        }
        else
        {
            complexTransform = complexTransform.translated ((float) dx, (float) dx);
        }
    }

    template <typename Type>
    Rectangle<Type> translated (const Rectangle<Type>& r) const noexcept
    {
        jassert (isOnlyTranslated);
        return r.translated (static_cast <Type> (xOffset),
                             static_cast <Type> (yOffset));
    }

    Rectangle<int> deviceSpaceToUserSpace (const Rectangle<int>& r) const noexcept
    {
        return isOnlyTranslated ? r.translated (-xOffset, -yOffset)
                                : r.toFloat().transformed (complexTransform.inverted()).getSmallestIntegerContainer();
    }

    AffineTransform complexTransform;
    int xOffset, yOffset;
    bool isOnlyTranslated;

private:
    static inline bool isIntegerTranslation (const AffineTransform& t) noexcept
    {
        const int tx = (int) (t.getTranslationX() * 256.0f);
        const int ty = (int) (t.getTranslationY() * 256.0f);
        return ((tx | ty) & 0xf8) == 0;
    }
};

//==============================================================================
template <class CachedGlyphType, class RenderTargetType>
class GlyphCache  : private DeletedAtShutdown
{
public:
    GlyphCache()
    {
        addNewGlyphSlots (120);
    }

    ~GlyphCache()
    {
        getSingletonPointer() = nullptr;
    }

    static GlyphCache& getInstance()
    {
        GlyphCache*& g = getSingletonPointer();

        if (g == nullptr)
            g = new GlyphCache();

        return *g;
    }

    //==============================================================================
    void drawGlyph (RenderTargetType& target, const Font& font, const int glyphNumber, float x, float y)
    {
        ++accessCounter;
        CachedGlyphType* glyph = nullptr;

        const ScopedReadLock srl (lock);

        for (int i = glyphs.size(); --i >= 0;)
        {
            CachedGlyphType* const g = glyphs.getUnchecked (i);

            if (g->glyph == glyphNumber && g->font == font)
            {
                glyph = g;
                ++hits;
                break;
            }
        }

        if (glyph == nullptr)
        {
            ++misses;
            const ScopedWriteLock swl (lock);

            if (hits.value + misses.value > glyphs.size() * 16)
            {
                if (misses.value * 2 > hits.value)
                    addNewGlyphSlots (32);

                hits.set (0);
                misses.set (0);
                glyph = glyphs.getLast();
            }
            else
            {
                glyph = findLeastRecentlyUsedGlyph();
            }

            jassert (glyph != nullptr);
            glyph->generate (font, glyphNumber);
        }

        glyph->lastAccessCount = accessCounter.value;
        glyph->draw (target, x, y);
    }

private:
    friend class OwnedArray <CachedGlyphType>;
    OwnedArray <CachedGlyphType> glyphs;
    Atomic<int> accessCounter, hits, misses;
    ReadWriteLock lock;

    void addNewGlyphSlots (int num)
    {
        while (--num >= 0)
            glyphs.add (new CachedGlyphType());
    }

    CachedGlyphType* findLeastRecentlyUsedGlyph() const noexcept
    {
        CachedGlyphType* oldest = glyphs.getLast();
        int oldestCounter = oldest->lastAccessCount;

        for (int i = glyphs.size() - 1; --i >= 0;)
        {
            CachedGlyphType* const glyph = glyphs.getUnchecked(i);

            if (glyph->lastAccessCount <= oldestCounter)
            {
                oldestCounter = glyph->lastAccessCount;
                oldest = glyph;
            }
        }

        return oldest;
    }

    static GlyphCache*& getSingletonPointer() noexcept
    {
        static GlyphCache* g = nullptr;
        return g;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GlyphCache);
};

//==============================================================================
template <class RendererType>
class CachedGlyphEdgeTable
{
public:
    CachedGlyphEdgeTable() : glyph (0), lastAccessCount (0) {}

    void draw (RendererType& state, float x, const float y) const
    {
        if (snapToIntegerCoordinate)
            x = std::floor (x + 0.5f);

        if (edgeTable != nullptr)
            state.fillEdgeTable (*edgeTable, x, roundToInt (y));
    }

    void generate (const Font& newFont, const int glyphNumber)
    {
        font = newFont;
        Typeface* const typeface = newFont.getTypeface();
        snapToIntegerCoordinate = typeface->isHinted();
        glyph = glyphNumber;

        const float fontHeight = font.getHeight();
        edgeTable = typeface->getEdgeTableForGlyph (glyphNumber,
                                                    AffineTransform::scale (fontHeight * font.getHorizontalScale(), fontHeight)
                                                                  #if JUCE_MAC || JUCE_IOS
                                                                    .translated (0.0f, -0.5f)
                                                                  #endif
                                                    );
    }

    Font font;
    int glyph, lastAccessCount;
    bool snapToIntegerCoordinate;

private:
    ScopedPointer <EdgeTable> edgeTable;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CachedGlyphEdgeTable);
};

//==============================================================================
template <class StateObjectType>
class SavedStateStack
{
public:
    SavedStateStack (StateObjectType* const initialState) noexcept
        : currentState (initialState)
    {}

    inline StateObjectType* operator->() const noexcept     { return currentState; }
    inline StateObjectType& operator*()  const noexcept     { return *currentState; }

    void save()
    {
        stack.add (new StateObjectType (*currentState));
    }

    void restore()
    {
        StateObjectType* const top = stack.getLast();

        if (top != nullptr)
        {
            currentState = top;
            stack.removeLast (1, false);
        }
        else
        {
            jassertfalse; // trying to pop with an empty stack!
        }
    }

    void beginTransparencyLayer (float opacity)
    {
        save();
        currentState = currentState->beginTransparencyLayer (opacity);
    }

    void endTransparencyLayer()
    {
        const ScopedPointer<StateObjectType> finishedTransparencyLayer (currentState);
        restore();
        currentState->endTransparencyLayer (*finishedTransparencyLayer);
    }

private:
    ScopedPointer<StateObjectType> currentState;
    OwnedArray<StateObjectType> stack;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SavedStateStack);
};

//==============================================================================
// Calculates the alpha values and positions for rendering the edges of a non-pixel
// aligned rectangle.
struct FloatRectangleRasterisingInfo
{
    FloatRectangleRasterisingInfo (const Rectangle<float>& area)
        : left   (roundToInt (256.0f * area.getX())),
          top    (roundToInt (256.0f * area.getY())),
          right  (roundToInt (256.0f * area.getRight())),
          bottom (roundToInt (256.0f * area.getBottom()))
    {
        if ((top >> 8) == (bottom >> 8))
        {
            topAlpha = bottom - top;
            bottomAlpha = 0;
            totalTop = top >> 8;
            totalBottom = bottom = top = totalTop + 1;
        }
        else
        {
            if ((top & 255) == 0)
            {
                topAlpha = 0;
                top = totalTop = (top >> 8);
            }
            else
            {
                topAlpha = 255 - (top & 255);
                totalTop = (top >> 8);
                top = totalTop + 1;
            }

            bottomAlpha = bottom & 255;
            bottom >>= 8;
            totalBottom = bottom + (bottomAlpha != 0 ? 1 : 0);
        }

        if ((left >> 8) == (right >> 8))
        {
            leftAlpha = right - left;
            rightAlpha = 0;
            totalLeft = (left >> 8);
            totalRight = right = left = totalLeft + 1;
        }
        else
        {
            if ((left & 255) == 0)
            {
                leftAlpha = 0;
                left = totalLeft = (left >> 8);
            }
            else
            {
                leftAlpha = 255 - (left & 255);
                totalLeft = (left >> 8);
                left = totalLeft + 1;
            }

            rightAlpha = right & 255;
            right >>= 8;
            totalRight = right + (rightAlpha != 0 ? 1 : 0);
        }
    }

    template <class Callback>
    void iterate (Callback& callback) const
    {
        if (topAlpha != 0)       callback (totalLeft, totalTop, totalRight - totalLeft, 1, topAlpha);
        if (bottomAlpha != 0)    callback (totalLeft, bottom,   totalRight - totalLeft, 1, bottomAlpha);
        if (leftAlpha != 0)      callback (totalLeft, totalTop, 1, totalBottom - totalTop, leftAlpha);
        if (rightAlpha != 0)     callback (right,     totalTop, 1, totalBottom - totalTop, rightAlpha);

        callback (left, top, right - left, bottom - top, 255);
    }

    inline bool isOnePixelWide() const noexcept            { return right - left == 1 && leftAlpha + rightAlpha == 0; }

    inline int getTopLeftCornerAlpha() const noexcept      { return (topAlpha * leftAlpha) >> 8; }
    inline int getTopRightCornerAlpha() const noexcept     { return (topAlpha * rightAlpha) >> 8; }
    inline int getBottomLeftCornerAlpha() const noexcept   { return (bottomAlpha * leftAlpha) >> 8; }
    inline int getBottomRightCornerAlpha() const noexcept  { return (bottomAlpha * rightAlpha) >> 8; }

    //==============================================================================
    int left, top, right, bottom;  // bounds of the solid central area, excluding anti-aliased edges
    int totalTop, totalLeft, totalBottom, totalRight; // bounds of the total area, including edges
    int topAlpha, leftAlpha, bottomAlpha, rightAlpha; // alpha of each anti-aliased edge
};

}

#endif   // __JUCE_RENDERINGHELPERS_JUCEHEADER__
