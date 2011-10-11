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

BEGIN_JUCE_NAMESPACE

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
             && isIntegerTranlation (t))
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
    static inline bool isIntegerTranlation (const AffineTransform& t) noexcept
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
        : accessCounter (0), hits (0), misses (0)
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
        int oldestCounter = std::numeric_limits<int>::max();
        CachedGlyphType* oldest = nullptr;

        for (int i = glyphs.size(); --i >= 0;)
        {
            CachedGlyphType* const glyph = glyphs.getUnchecked (i);

            if (glyph->glyph == glyphNumber && glyph->font == font)
            {
                ++hits;
                glyph->lastAccessCount = accessCounter;
                glyph->draw (target, x, y);
                return;
            }

            if (glyph->lastAccessCount <= oldestCounter)
            {
                oldestCounter = glyph->lastAccessCount;
                oldest = glyph;
            }
        }

        if (hits + ++misses > (glyphs.size() << 4))
        {
            if (misses * 2 > hits)
                addNewGlyphSlots (32);

            hits = misses = 0;
            oldest = glyphs.getLast();
        }

        jassert (oldest != nullptr);
        oldest->lastAccessCount = accessCounter;
        oldest->generate (font, glyphNumber);
        oldest->draw (target, x, y);
    }

private:
    friend class OwnedArray <CachedGlyphType>;
    OwnedArray <CachedGlyphType> glyphs;
    int accessCounter, hits, misses;

    void addNewGlyphSlots (int num)
    {
        while (--num >= 0)
            glyphs.add (new CachedGlyphType());
    }

    static GlyphCache*& getSingletonPointer() noexcept
    {
        static GlyphCache* g = nullptr;
        return g;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GlyphCache);
};

}

END_JUCE_NAMESPACE

#endif   // __JUCE_RENDERINGHELPERS_JUCEHEADER__
