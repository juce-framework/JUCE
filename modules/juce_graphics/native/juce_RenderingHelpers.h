/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

JUCE_BEGIN_IGNORE_WARNINGS_MSVC (4127)

namespace RenderingHelpers
{

//==============================================================================
/** Holds either a simple integer translation, or an affine transform.

    @tags{Graphics}
*/
class TranslationOrTransform
{
public:
    TranslationOrTransform() = default;
    TranslationOrTransform (Point<int> origin) noexcept  : offset (origin) {}

    TranslationOrTransform (const TranslationOrTransform& other) = default;

    AffineTransform getTransform() const noexcept
    {
        return isOnlyTranslated ? AffineTransform::translation (offset)
                                : complexTransform;
    }

    AffineTransform getTransformWith (const AffineTransform& userTransform) const noexcept
    {
        return isOnlyTranslated ? userTransform.translated (offset)
                                : userTransform.followedBy (complexTransform);
    }

    bool isIdentity() const noexcept
    {
        return isOnlyTranslated && offset.isOrigin();
    }

    void setOrigin (Point<int> delta) noexcept
    {
        if (isOnlyTranslated)
            offset += delta;
        else
            complexTransform = AffineTransform::translation (delta)
                                               .followedBy (complexTransform);
    }

    void addTransform (const AffineTransform& t) noexcept
    {
        if (isOnlyTranslated && t.isOnlyTranslation())
        {
            auto tx = (int) (t.getTranslationX() * 256.0f);
            auto ty = (int) (t.getTranslationY() * 256.0f);

            if (((tx | ty) & 0xf8) == 0)
            {
                offset += Point<int> (tx >> 8, ty >> 8);
                return;
            }
        }

        complexTransform = getTransformWith (t);
        isOnlyTranslated = false;
        isRotated = (complexTransform.mat01 != 0.0f || complexTransform.mat10 != 0.0f
                      || complexTransform.mat00 < 0 || complexTransform.mat11 < 0);
    }

    float getPhysicalPixelScaleFactor() const noexcept
    {
        return isOnlyTranslated ? 1.0f : std::sqrt (std::abs (complexTransform.getDeterminant()));
    }

    void moveOriginInDeviceSpace (Point<int> delta) noexcept
    {
        if (isOnlyTranslated)
            offset += delta;
        else
            complexTransform = complexTransform.translated (delta);
    }

    Rectangle<int> translated (Rectangle<int> r) const noexcept
    {
        jassert (isOnlyTranslated);
        return r + offset;
    }

    Rectangle<float> translated (Rectangle<float> r) const noexcept
    {
        jassert (isOnlyTranslated);
        return r + offset.toFloat();
    }

    template <typename RectangleOrPoint>
    RectangleOrPoint transformed (RectangleOrPoint r) const noexcept
    {
        jassert (! isOnlyTranslated);
        return r.transformedBy (complexTransform);
    }

    template <typename Type>
    Rectangle<Type> deviceSpaceToUserSpace (Rectangle<Type> r) const noexcept
    {
        return isOnlyTranslated ? r - offset
                                : r.transformedBy (complexTransform.inverted());
    }

    AffineTransform complexTransform;
    Point<int> offset;
    bool isOnlyTranslated = true, isRotated = false;
};

//==============================================================================
/** Holds a cache of recently-used glyph objects of some type.

    @tags{Graphics}
*/
template <class CachedGlyphType, class RenderTargetType>
class GlyphCache  : private DeletedAtShutdown
{
public:
    GlyphCache()
    {
        reset();
    }

    ~GlyphCache() override
    {
        getSingletonPointer() = nullptr;
    }

    static GlyphCache& getInstance()
    {
        auto& g = getSingletonPointer();

        if (g == nullptr)
            g = new GlyphCache();

        return *g;
    }

    //==============================================================================
    void reset()
    {
        const ScopedLock sl (lock);
        glyphs.clear();
        addNewGlyphSlots (120);
        hits = 0;
        misses = 0;
    }

    void drawGlyph (RenderTargetType& target, const Font& font, const int glyphNumber, Point<float> pos)
    {
        if (auto glyph = findOrCreateGlyph (font, glyphNumber))
        {
            glyph->lastAccessCount = ++accessCounter;
            glyph->draw (target, pos);
        }
    }

    ReferenceCountedObjectPtr<CachedGlyphType> findOrCreateGlyph (const Font& font, int glyphNumber)
    {
        const ScopedLock sl (lock);

        if (auto g = findExistingGlyph (font, glyphNumber))
        {
            ++hits;
            return g;
        }

        ++misses;
        auto g = getGlyphForReuse();
        jassert (g != nullptr);
        g->generate (font, glyphNumber);
        return g;
    }

private:
    ReferenceCountedArray<CachedGlyphType> glyphs;
    Atomic<int> accessCounter, hits, misses;
    CriticalSection lock;

    ReferenceCountedObjectPtr<CachedGlyphType> findExistingGlyph (const Font& font, int glyphNumber) const noexcept
    {
        for (auto g : glyphs)
            if (g->glyph == glyphNumber && g->font == font)
                return *g;

        return {};
    }

    ReferenceCountedObjectPtr<CachedGlyphType> getGlyphForReuse()
    {
        if (hits.get() + misses.get() > glyphs.size() * 16)
        {
            if (misses.get() * 2 > hits.get())
                addNewGlyphSlots (32);

            hits = 0;
            misses = 0;
        }

        if (auto g = findLeastRecentlyUsedGlyph())
            return *g;

        addNewGlyphSlots (32);
        return glyphs.getLast();
    }

    void addNewGlyphSlots (int num)
    {
        glyphs.ensureStorageAllocated (glyphs.size() + num);

        while (--num >= 0)
            glyphs.add (new CachedGlyphType());
    }

    CachedGlyphType* findLeastRecentlyUsedGlyph() const noexcept
    {
        CachedGlyphType* oldest = nullptr;
        auto oldestCounter = std::numeric_limits<int>::max();

        for (auto* g : glyphs)
        {
            if (g->lastAccessCount <= oldestCounter
                 && g->getReferenceCount() == 1)
            {
                oldestCounter = g->lastAccessCount;
                oldest = g;
            }
        }

        return oldest;
    }

    static GlyphCache*& getSingletonPointer() noexcept
    {
        static GlyphCache* g = nullptr;
        return g;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GlyphCache)
};

//==============================================================================
/** Caches a glyph as an edge-table.

    @tags{Graphics}
*/
template <class RendererType>
class CachedGlyphEdgeTable  : public ReferenceCountedObject
{
public:
    CachedGlyphEdgeTable() = default;

    void draw (RendererType& state, Point<float> pos) const
    {
        if (snapToIntegerCoordinate)
            pos.x = std::floor (pos.x + 0.5f);

        if (edgeTable != nullptr)
            state.fillEdgeTable (*edgeTable, pos.x, roundToInt (pos.y));
    }

    void generate (const Font& newFont, int glyphNumber)
    {
        font = newFont;
        auto typeface = newFont.getTypefacePtr();
        snapToIntegerCoordinate = typeface->isHinted();
        glyph = glyphNumber;

        auto fontHeight = font.getHeight();
        edgeTable.reset (typeface->getEdgeTableForGlyph (glyphNumber,
                                                         AffineTransform::scale (fontHeight * font.getHorizontalScale(),
                                                                                 fontHeight), fontHeight));
    }

    Font font;
    std::unique_ptr<EdgeTable> edgeTable;
    int glyph = 0, lastAccessCount = 0;
    bool snapToIntegerCoordinate = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CachedGlyphEdgeTable)
};

//==============================================================================
/** Calculates the alpha values and positions for rendering the edges of a
    non-pixel-aligned rectangle.

    @tags{Graphics}
*/
struct FloatRectangleRasterisingInfo
{
    FloatRectangleRasterisingInfo (Rectangle<float> area)
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

//==============================================================================
/** Contains classes for calculating the colour of pixels within various types of gradient. */
namespace GradientPixelIterators
{
    /** Iterates the colour of pixels in a linear gradient */
    struct Linear
    {
        Linear (const ColourGradient& gradient, const AffineTransform& transform,
                const PixelARGB* colours, int numColours)
            : lookupTable (colours),
              numEntries (numColours)
        {
            jassert (numColours >= 0);
            auto p1 = gradient.point1;
            auto p2 = gradient.point2;

            if (! transform.isIdentity())
            {
                auto p3 = Line<float> (p2, p1).getPointAlongLine (0.0f, 100.0f);

                p1.applyTransform (transform);
                p2.applyTransform (transform);
                p3.applyTransform (transform);

                p2 = Line<float> (p2, p3).findNearestPointTo (p1);
            }

            vertical   = std::abs (p1.x - p2.x) < 0.001f;
            horizontal = std::abs (p1.y - p2.y) < 0.001f;

            if (vertical)
            {
                scale = roundToInt ((double) ((int64_t) numEntries << (int) numScaleBits) / (double) (p2.y - p1.y));
                start = roundToInt (p1.y * (float) scale);
            }
            else if (horizontal)
            {
                scale = roundToInt ((double) ((int64_t) numEntries << (int) numScaleBits) / (double) (p2.x - p1.x));
                start = roundToInt (p1.x * (float) scale);
            }
            else
            {
                grad = (p2.getY() - p1.y) / (double) (p1.x - p2.x);
                yTerm = p1.getY() - p1.x / grad;
                scale = roundToInt ((double) ((int64_t) numEntries << (int) numScaleBits) / (yTerm * grad - (p2.y * grad - p2.x)));
                grad *= scale;
            }
        }

        forcedinline void setY (int y) noexcept
        {
            if (vertical)
                linePix = lookupTable[jlimit (0, numEntries, (y * scale - start) >> (int) numScaleBits)];
            else if (! horizontal)
                start = roundToInt ((y - yTerm) * grad);
        }

        inline PixelARGB getPixel (int x) const noexcept
        {
            return vertical ? linePix
                            : lookupTable[jlimit (0, numEntries, (x * scale - start) >> (int) numScaleBits)];
        }

        const PixelARGB* const lookupTable;
        const int numEntries;
        PixelARGB linePix;
        int start, scale;
        double grad, yTerm;
        bool vertical, horizontal;
        enum { numScaleBits = 12 };

        JUCE_DECLARE_NON_COPYABLE (Linear)
    };

    //==============================================================================
    /** Iterates the colour of pixels in a circular radial gradient */
    struct Radial
    {
        Radial (const ColourGradient& gradient, const AffineTransform&,
                const PixelARGB* colours, int numColours)
            : lookupTable (colours),
              numEntries (numColours),
              gx1 (gradient.point1.x),
              gy1 (gradient.point1.y)
        {
            jassert (numColours >= 0);
            auto diff = gradient.point1 - gradient.point2;
            maxDist = diff.x * diff.x + diff.y * diff.y;
            invScale = numEntries / std::sqrt (maxDist);
            jassert (roundToInt (std::sqrt (maxDist) * invScale) <= numEntries);
        }

        forcedinline void setY (int y) noexcept
        {
            dy = y - gy1;
            dy *= dy;
        }

        inline PixelARGB getPixel (int px) const noexcept
        {
            auto x = px - gx1;
            x *= x;
            x += dy;

            return lookupTable[x >= maxDist ? numEntries : roundToInt (std::sqrt (x) * invScale)];
        }

        const PixelARGB* const lookupTable;
        const int numEntries;
        const double gx1, gy1;
        double maxDist, invScale, dy;

        JUCE_DECLARE_NON_COPYABLE (Radial)
    };

    //==============================================================================
    /** Iterates the colour of pixels in a skewed radial gradient */
    struct TransformedRadial   : public Radial
    {
        TransformedRadial (const ColourGradient& gradient, const AffineTransform& transform,
                           const PixelARGB* colours, int numColours)
            : Radial (gradient, transform, colours, numColours),
              inverseTransform (transform.inverted())
        {
            tM10 = inverseTransform.mat10;
            tM00 = inverseTransform.mat00;
        }

        forcedinline void setY (int y) noexcept
        {
            auto floatY = (float) y;
            lineYM01 = inverseTransform.mat01 * floatY + inverseTransform.mat02 - gx1;
            lineYM11 = inverseTransform.mat11 * floatY + inverseTransform.mat12 - gy1;
        }

        inline PixelARGB getPixel (int px) const noexcept
        {
            double x = px;
            auto y = tM10 * x + lineYM11;
            x = tM00 * x + lineYM01;
            x *= x;
            x += y * y;

            if (x >= maxDist)
                return lookupTable[numEntries];

            return lookupTable[jmin (numEntries, roundToInt (std::sqrt (x) * invScale))];
        }

    private:
        double tM10, tM00, lineYM01, lineYM11;
        const AffineTransform inverseTransform;

        JUCE_DECLARE_NON_COPYABLE (TransformedRadial)
    };
}

#define JUCE_PERFORM_PIXEL_OP_LOOP(op) \
{ \
    const int destStride = destData.pixelStride;  \
    do { dest->op; dest = addBytesToPointer (dest, destStride); } while (--width > 0); \
}

//==============================================================================
/** Contains classes for filling edge tables with various fill types. */
namespace EdgeTableFillers
{
    /** Fills an edge-table with a solid colour. */
    template <class PixelType, bool replaceExisting = false>
    struct SolidColour
    {
        SolidColour (const Image::BitmapData& image, PixelARGB colour)
            : destData (image), sourceColour (colour)
        {
            if (sizeof (PixelType) == 3 && (size_t) destData.pixelStride == sizeof (PixelType))
                areRGBComponentsEqual = sourceColour.getRed() == sourceColour.getGreen()
                                            && sourceColour.getGreen() == sourceColour.getBlue();
            else
                areRGBComponentsEqual = false;
        }

        forcedinline void setEdgeTableYPos (int y) noexcept
        {
            linePixels = (PixelType*) destData.getLinePointer (y);
        }

        forcedinline void handleEdgeTablePixel (int x, int alphaLevel) const noexcept
        {
            if (replaceExisting)
                getPixel (x)->set (sourceColour);
            else
                getPixel (x)->blend (sourceColour, (uint32) alphaLevel);
        }

        forcedinline void handleEdgeTablePixelFull (int x) const noexcept
        {
            if (replaceExisting)
                getPixel (x)->set (sourceColour);
            else
                getPixel (x)->blend (sourceColour);
        }

        forcedinline void handleEdgeTableLine (int x, int width, int alphaLevel) const noexcept
        {
            auto p = sourceColour;
            p.multiplyAlpha (alphaLevel);

            auto* dest = getPixel (x);

            if (replaceExisting || p.getAlpha() >= 0xff)
                replaceLine (dest, p, width);
            else
                blendLine (dest, p, width);
        }

        forcedinline void handleEdgeTableLineFull (int x, int width) const noexcept
        {
            auto* dest = getPixel (x);

            if (replaceExisting || sourceColour.getAlpha() >= 0xff)
                replaceLine (dest, sourceColour, width);
            else
                blendLine (dest, sourceColour, width);
        }

        void handleEdgeTableRectangle (int x, int y, int width, int height, int alphaLevel) noexcept
        {
            auto p = sourceColour;
            p.multiplyAlpha (alphaLevel);

            setEdgeTableYPos (y);
            auto* dest = getPixel (x);

            if (replaceExisting || p.getAlpha() >= 0xff)
            {
                while (--height >= 0)
                {
                    replaceLine (dest, p, width);
                    dest = addBytesToPointer (dest, destData.lineStride);
                }
            }
            else
            {
                while (--height >= 0)
                {
                    blendLine (dest, p, width);
                    dest = addBytesToPointer (dest, destData.lineStride);
                }
            }
        }

        void handleEdgeTableRectangleFull (int x, int y, int width, int height) noexcept
        {
            handleEdgeTableRectangle (x, y, width, height, 255);
        }

    private:
        const Image::BitmapData& destData;
        PixelType* linePixels;
        PixelARGB sourceColour;
        bool areRGBComponentsEqual;

        forcedinline PixelType* getPixel (int x) const noexcept
        {
            return addBytesToPointer (linePixels, x * destData.pixelStride);
        }

        inline void blendLine (PixelType* dest, PixelARGB colour, int width) const noexcept
        {
            JUCE_PERFORM_PIXEL_OP_LOOP (blend (colour))
        }

        forcedinline void replaceLine (PixelRGB* dest, PixelARGB colour, int width) const noexcept
        {
            if ((size_t) destData.pixelStride == sizeof (*dest) && areRGBComponentsEqual)
                memset ((void*) dest, colour.getRed(), (size_t) width * 3);   // if all the component values are the same, we can cheat..
            else
                JUCE_PERFORM_PIXEL_OP_LOOP (set (colour));
        }

        forcedinline void replaceLine (PixelAlpha* dest, const PixelARGB colour, int width) const noexcept
        {
            if ((size_t) destData.pixelStride == sizeof (*dest))
                memset ((void*) dest, colour.getAlpha(), (size_t) width);
            else
                JUCE_PERFORM_PIXEL_OP_LOOP (setAlpha (colour.getAlpha()))
        }

        forcedinline void replaceLine (PixelARGB* dest, const PixelARGB colour, int width) const noexcept
        {
            JUCE_PERFORM_PIXEL_OP_LOOP (set (colour))
        }

        JUCE_DECLARE_NON_COPYABLE (SolidColour)
    };

    //==============================================================================
    /** Fills an edge-table with a gradient. */
    template <class PixelType, class GradientType>
    struct Gradient  : public GradientType
    {
        Gradient (const Image::BitmapData& dest, const ColourGradient& gradient, const AffineTransform& transform,
                  const PixelARGB* colours, int numColours)
            : GradientType (gradient, transform, colours, numColours - 1),
              destData (dest)
        {
        }

        forcedinline void setEdgeTableYPos (int y) noexcept
        {
            linePixels = (PixelType*) destData.getLinePointer (y);
            GradientType::setY (y);
        }

        forcedinline void handleEdgeTablePixel (int x, int alphaLevel) const noexcept
        {
            getPixel (x)->blend (GradientType::getPixel (x), (uint32) alphaLevel);
        }

        forcedinline void handleEdgeTablePixelFull (int x) const noexcept
        {
            getPixel (x)->blend (GradientType::getPixel (x));
        }

        void handleEdgeTableLine (int x, int width, int alphaLevel) const noexcept
        {
            auto* dest = getPixel (x);

            if (alphaLevel < 0xff)
                JUCE_PERFORM_PIXEL_OP_LOOP (blend (GradientType::getPixel (x++), (uint32) alphaLevel))
            else
                JUCE_PERFORM_PIXEL_OP_LOOP (blend (GradientType::getPixel (x++)))
        }

        void handleEdgeTableLineFull (int x, int width) const noexcept
        {
            auto* dest = getPixel (x);
            JUCE_PERFORM_PIXEL_OP_LOOP (blend (GradientType::getPixel (x++)))
        }

        void handleEdgeTableRectangle (int x, int y, int width, int height, int alphaLevel) noexcept
        {
            while (--height >= 0)
            {
                setEdgeTableYPos (y++);
                handleEdgeTableLine (x, width, alphaLevel);
            }
        }

        void handleEdgeTableRectangleFull (int x, int y, int width, int height) noexcept
        {
            while (--height >= 0)
            {
                setEdgeTableYPos (y++);
                handleEdgeTableLineFull (x, width);
            }
        }

    private:
        const Image::BitmapData& destData;
        PixelType* linePixels;

        forcedinline PixelType* getPixel (int x) const noexcept
        {
            return addBytesToPointer (linePixels, x * destData.pixelStride);
        }

        JUCE_DECLARE_NON_COPYABLE (Gradient)
    };

    //==============================================================================
    /** Fills an edge-table with a non-transformed image. */
    template <class DestPixelType, class SrcPixelType, bool repeatPattern>
    struct ImageFill
    {
        ImageFill (const Image::BitmapData& dest, const Image::BitmapData& src, int alpha, int x, int y)
            : destData (dest),
              srcData (src),
              extraAlpha (alpha + 1),
              xOffset (repeatPattern ? negativeAwareModulo (x, src.width)  - src.width  : x),
              yOffset (repeatPattern ? negativeAwareModulo (y, src.height) - src.height : y)
        {
        }

        forcedinline void setEdgeTableYPos (int y) noexcept
        {
            linePixels = (DestPixelType*) destData.getLinePointer (y);
            y -= yOffset;

            if (repeatPattern)
            {
                jassert (y >= 0);
                y %= srcData.height;
            }

            sourceLineStart = (SrcPixelType*) srcData.getLinePointer (y);
        }

        forcedinline void handleEdgeTablePixel (int x, int alphaLevel) const noexcept
        {
            alphaLevel = (alphaLevel * extraAlpha) >> 8;

            getDestPixel (x)->blend (*getSrcPixel (repeatPattern ? ((x - xOffset) % srcData.width) : (x - xOffset)), (uint32) alphaLevel);
        }

        forcedinline void handleEdgeTablePixelFull (int x) const noexcept
        {
            getDestPixel (x)->blend (*getSrcPixel (repeatPattern ? ((x - xOffset) % srcData.width) : (x - xOffset)), (uint32) extraAlpha);
        }

        void handleEdgeTableLine (int x, int width, int alphaLevel) const noexcept
        {
            auto* dest = getDestPixel (x);
            alphaLevel = (alphaLevel * extraAlpha) >> 8;
            x -= xOffset;

            if (repeatPattern)
            {
                if (alphaLevel < 0xfe)
                    JUCE_PERFORM_PIXEL_OP_LOOP (blend (*getSrcPixel (x++ % srcData.width), (uint32) alphaLevel))
                else
                    JUCE_PERFORM_PIXEL_OP_LOOP (blend (*getSrcPixel (x++ % srcData.width)))
            }
            else
            {
                jassert (x >= 0 && x + width <= srcData.width);

                if (alphaLevel < 0xfe)
                    JUCE_PERFORM_PIXEL_OP_LOOP (blend (*getSrcPixel (x++), (uint32) alphaLevel))
                else
                    copyRow (dest, getSrcPixel (x), width);
            }
        }

        void handleEdgeTableLineFull (int x, int width) const noexcept
        {
            auto* dest = getDestPixel (x);
            x -= xOffset;

            if (repeatPattern)
            {
                if (extraAlpha < 0xfe)
                    JUCE_PERFORM_PIXEL_OP_LOOP (blend (*getSrcPixel (x++ % srcData.width), (uint32) extraAlpha))
                else
                    JUCE_PERFORM_PIXEL_OP_LOOP (blend (*getSrcPixel (x++ % srcData.width)))
            }
            else
            {
                jassert (x >= 0 && x + width <= srcData.width);

                if (extraAlpha < 0xfe)
                    JUCE_PERFORM_PIXEL_OP_LOOP (blend (*getSrcPixel (x++), (uint32) extraAlpha))
                else
                    copyRow (dest, getSrcPixel (x), width);
            }
        }

        void handleEdgeTableRectangle (int x, int y, int width, int height, int alphaLevel) noexcept
        {
            while (--height >= 0)
            {
                setEdgeTableYPos (y++);
                handleEdgeTableLine (x, width, alphaLevel);
            }
        }

        void handleEdgeTableRectangleFull (int x, int y, int width, int height) noexcept
        {
            while (--height >= 0)
            {
                setEdgeTableYPos (y++);
                handleEdgeTableLineFull (x, width);
            }
        }

        void clipEdgeTableLine (EdgeTable& et, int x, int y, int width)
        {
            jassert (x - xOffset >= 0 && x + width - xOffset <= srcData.width);
            auto* s = (SrcPixelType*) srcData.getLinePointer (y - yOffset);
            auto* mask = (uint8*) (s + x - xOffset);

            if (sizeof (SrcPixelType) == sizeof (PixelARGB))
                mask += PixelARGB::indexA;

            et.clipLineToMask (x, y, mask, sizeof (SrcPixelType), width);
        }

    private:
        const Image::BitmapData& destData;
        const Image::BitmapData& srcData;
        const int extraAlpha, xOffset, yOffset;
        DestPixelType* linePixels;
        SrcPixelType* sourceLineStart;

        forcedinline DestPixelType* getDestPixel (int x) const noexcept
        {
            return addBytesToPointer (linePixels, x * destData.pixelStride);
        }

        forcedinline SrcPixelType const* getSrcPixel (int x) const noexcept
        {
            return addBytesToPointer (sourceLineStart, x * srcData.pixelStride);
        }

        forcedinline void copyRow (DestPixelType* dest, SrcPixelType const* src, int width) const noexcept
        {
            auto destStride = destData.pixelStride;
            auto srcStride  = srcData.pixelStride;

            if (destStride == srcStride
                 && srcData.pixelFormat  == Image::RGB
                 && destData.pixelFormat == Image::RGB)
            {
                memcpy ((void*) dest, src, (size_t) (width * srcStride));
            }
            else
            {
                do
                {
                    dest->blend (*src);
                    dest = addBytesToPointer (dest, destStride);
                    src  = addBytesToPointer (src, srcStride);
                } while (--width > 0);
            }
        }

        JUCE_DECLARE_NON_COPYABLE (ImageFill)
    };

    //==============================================================================
    /** Fills an edge-table with a transformed image. */
    template <class DestPixelType, class SrcPixelType, bool repeatPattern>
    struct TransformedImageFill
    {
        TransformedImageFill (const Image::BitmapData& dest, const Image::BitmapData& src,
                              const AffineTransform& transform, int alpha, Graphics::ResamplingQuality q)
            : interpolator (transform,
                            q != Graphics::lowResamplingQuality ? 0.5f : 0.0f,
                            q != Graphics::lowResamplingQuality ? -128 : 0),
              destData (dest),
              srcData (src),
              extraAlpha (alpha + 1),
              quality (q),
              maxX (src.width  - 1),
              maxY (src.height - 1)
        {
            scratchBuffer.malloc (scratchSize);
        }

        forcedinline void setEdgeTableYPos (int newY) noexcept
        {
            currentY = newY;
            linePixels = (DestPixelType*) destData.getLinePointer (newY);
        }

        forcedinline void handleEdgeTablePixel (int x, int alphaLevel) noexcept
        {
            SrcPixelType p;
            generate (&p, x, 1);

            getDestPixel (x)->blend (p, (uint32) (alphaLevel * extraAlpha) >> 8);
        }

        forcedinline void handleEdgeTablePixelFull (int x) noexcept
        {
            SrcPixelType p;
            generate (&p, x, 1);

            getDestPixel (x)->blend (p, (uint32) extraAlpha);
        }

        void handleEdgeTableLine (int x, int width, int alphaLevel) noexcept
        {
            if (width > (int) scratchSize)
            {
                scratchSize = (size_t) width;
                scratchBuffer.malloc (scratchSize);
            }

            SrcPixelType* span = scratchBuffer;
            generate (span, x, width);

            auto* dest = getDestPixel (x);
            alphaLevel *= extraAlpha;
            alphaLevel >>= 8;

            if (alphaLevel < 0xfe)
                JUCE_PERFORM_PIXEL_OP_LOOP (blend (*span++, (uint32) alphaLevel))
            else
                JUCE_PERFORM_PIXEL_OP_LOOP (blend (*span++))
        }

        forcedinline void handleEdgeTableLineFull (int x, int width) noexcept
        {
            handleEdgeTableLine (x, width, 255);
        }

        void handleEdgeTableRectangle (int x, int y, int width, int height, int alphaLevel) noexcept
        {
            while (--height >= 0)
            {
                setEdgeTableYPos (y++);
                handleEdgeTableLine (x, width, alphaLevel);
            }
        }

        void handleEdgeTableRectangleFull (int x, int y, int width, int height) noexcept
        {
            while (--height >= 0)
            {
                setEdgeTableYPos (y++);
                handleEdgeTableLineFull (x, width);
            }
        }

        void clipEdgeTableLine (EdgeTable& et, int x, int y, int width)
        {
            if (width > (int) scratchSize)
            {
                scratchSize = (size_t) width;
                scratchBuffer.malloc (scratchSize);
            }

            currentY = y;
            generate (scratchBuffer.get(), x, width);

            et.clipLineToMask (x, y,
                               reinterpret_cast<uint8*> (scratchBuffer.get()) + SrcPixelType::indexA,
                               sizeof (SrcPixelType), width);
        }

    private:
        forcedinline DestPixelType* getDestPixel (int x) const noexcept
        {
            return addBytesToPointer (linePixels, x * destData.pixelStride);
        }

        //==============================================================================
        template <class PixelType>
        void generate (PixelType* dest, int x, int numPixels) noexcept
        {
            this->interpolator.setStartOfLine ((float) x, (float) currentY, numPixels);

            do
            {
                int hiResX, hiResY;
                this->interpolator.next (hiResX, hiResY);

                int loResX = hiResX >> 8;
                int loResY = hiResY >> 8;

                if (repeatPattern)
                {
                    loResX = negativeAwareModulo (loResX, srcData.width);
                    loResY = negativeAwareModulo (loResY, srcData.height);
                }

                if (quality != Graphics::lowResamplingQuality)
                {
                    if (isPositiveAndBelow (loResX, maxX))
                    {
                        if (isPositiveAndBelow (loResY, maxY))
                        {
                            // In the centre of the image..
                            render4PixelAverage (dest, this->srcData.getPixelPointer (loResX, loResY),
                                                 hiResX & 255, hiResY & 255);
                            ++dest;
                            continue;
                        }

                        if (! repeatPattern)
                        {
                            // At a top or bottom edge..
                            if (loResY < 0)
                                render2PixelAverageX (dest, this->srcData.getPixelPointer (loResX, 0), hiResX & 255);
                            else
                                render2PixelAverageX (dest, this->srcData.getPixelPointer (loResX, maxY), hiResX & 255);

                            ++dest;
                            continue;
                        }
                    }
                    else
                    {
                        if (isPositiveAndBelow (loResY, maxY) && ! repeatPattern)
                        {
                            // At a left or right hand edge..
                            if (loResX < 0)
                                render2PixelAverageY (dest, this->srcData.getPixelPointer (0, loResY), hiResY & 255);
                            else
                                render2PixelAverageY (dest, this->srcData.getPixelPointer (maxX, loResY), hiResY & 255);

                            ++dest;
                            continue;
                        }
                    }
                }

                if (! repeatPattern)
                {
                    if (loResX < 0)     loResX = 0;
                    if (loResY < 0)     loResY = 0;
                    if (loResX > maxX)  loResX = maxX;
                    if (loResY > maxY)  loResY = maxY;
                }

                dest->set (*(const PixelType*) this->srcData.getPixelPointer (loResX, loResY));
                ++dest;

            } while (--numPixels > 0);
        }

        //==============================================================================
        void render4PixelAverage (PixelARGB* dest, const uint8* src, int subPixelX, int subPixelY) noexcept
        {
            uint32 c[4] = { 256 * 128, 256 * 128, 256 * 128, 256 * 128 };

            auto weight = (uint32) ((256 - subPixelX) * (256 - subPixelY));
            c[0] += weight * src[0];
            c[1] += weight * src[1];
            c[2] += weight * src[2];
            c[3] += weight * src[3];

            src += this->srcData.pixelStride;

            weight = (uint32) (subPixelX * (256 - subPixelY));
            c[0] += weight * src[0];
            c[1] += weight * src[1];
            c[2] += weight * src[2];
            c[3] += weight * src[3];

            src += this->srcData.lineStride;

            weight = (uint32) (subPixelX * subPixelY);
            c[0] += weight * src[0];
            c[1] += weight * src[1];
            c[2] += weight * src[2];
            c[3] += weight * src[3];

            src -= this->srcData.pixelStride;

            weight = (uint32) ((256 - subPixelX) * subPixelY);
            c[0] += weight * src[0];
            c[1] += weight * src[1];
            c[2] += weight * src[2];
            c[3] += weight * src[3];

            dest->setARGB ((uint8) (c[PixelARGB::indexA] >> 16),
                           (uint8) (c[PixelARGB::indexR] >> 16),
                           (uint8) (c[PixelARGB::indexG] >> 16),
                           (uint8) (c[PixelARGB::indexB] >> 16));
        }

        void render2PixelAverageX (PixelARGB* dest, const uint8* src, uint32 subPixelX) noexcept
        {
            uint32 c[4] = { 128, 128, 128, 128 };

            uint32 weight = 256 - subPixelX;
            c[0] += weight * src[0];
            c[1] += weight * src[1];
            c[2] += weight * src[2];
            c[3] += weight * src[3];

            src += this->srcData.pixelStride;

            weight = subPixelX;
            c[0] += weight * src[0];
            c[1] += weight * src[1];
            c[2] += weight * src[2];
            c[3] += weight * src[3];

            dest->setARGB ((uint8) (c[PixelARGB::indexA] >> 8),
                           (uint8) (c[PixelARGB::indexR] >> 8),
                           (uint8) (c[PixelARGB::indexG] >> 8),
                           (uint8) (c[PixelARGB::indexB] >> 8));
        }

        void render2PixelAverageY (PixelARGB* dest, const uint8* src, uint32 subPixelY) noexcept
        {
            uint32 c[4] = { 128, 128, 128, 128 };

            uint32 weight = 256 - subPixelY;
            c[0] += weight * src[0];
            c[1] += weight * src[1];
            c[2] += weight * src[2];
            c[3] += weight * src[3];

            src += this->srcData.lineStride;

            weight = subPixelY;
            c[0] += weight * src[0];
            c[1] += weight * src[1];
            c[2] += weight * src[2];
            c[3] += weight * src[3];

            dest->setARGB ((uint8) (c[PixelARGB::indexA] >> 8),
                           (uint8) (c[PixelARGB::indexR] >> 8),
                           (uint8) (c[PixelARGB::indexG] >> 8),
                           (uint8) (c[PixelARGB::indexB] >> 8));
        }

        //==============================================================================
        void render4PixelAverage (PixelRGB* dest, const uint8* src, uint32 subPixelX, uint32 subPixelY) noexcept
        {
            uint32 c[3] = { 256 * 128, 256 * 128, 256 * 128 };

            uint32 weight = (256 - subPixelX) * (256 - subPixelY);
            c[0] += weight * src[0];
            c[1] += weight * src[1];
            c[2] += weight * src[2];

            src += this->srcData.pixelStride;

            weight = subPixelX * (256 - subPixelY);
            c[0] += weight * src[0];
            c[1] += weight * src[1];
            c[2] += weight * src[2];

            src += this->srcData.lineStride;

            weight = subPixelX * subPixelY;
            c[0] += weight * src[0];
            c[1] += weight * src[1];
            c[2] += weight * src[2];

            src -= this->srcData.pixelStride;

            weight = (256 - subPixelX) * subPixelY;
            c[0] += weight * src[0];
            c[1] += weight * src[1];
            c[2] += weight * src[2];

            dest->setARGB ((uint8) 255,
                           (uint8) (c[PixelRGB::indexR] >> 16),
                           (uint8) (c[PixelRGB::indexG] >> 16),
                           (uint8) (c[PixelRGB::indexB] >> 16));
        }

        void render2PixelAverageX (PixelRGB* dest, const uint8* src, uint32 subPixelX) noexcept
        {
            uint32 c[3] = { 128, 128, 128 };

            const uint32 weight = 256 - subPixelX;
            c[0] += weight * src[0];
            c[1] += weight * src[1];
            c[2] += weight * src[2];

            src += this->srcData.pixelStride;

            c[0] += subPixelX * src[0];
            c[1] += subPixelX * src[1];
            c[2] += subPixelX * src[2];

            dest->setARGB ((uint8) 255,
                           (uint8) (c[PixelRGB::indexR] >> 8),
                           (uint8) (c[PixelRGB::indexG] >> 8),
                           (uint8) (c[PixelRGB::indexB] >> 8));
        }

        void render2PixelAverageY (PixelRGB* dest, const uint8* src, uint32 subPixelY) noexcept
        {
            uint32 c[3] = { 128, 128, 128 };

            const uint32 weight = 256 - subPixelY;
            c[0] += weight * src[0];
            c[1] += weight * src[1];
            c[2] += weight * src[2];

            src += this->srcData.lineStride;

            c[0] += subPixelY * src[0];
            c[1] += subPixelY * src[1];
            c[2] += subPixelY * src[2];

            dest->setARGB ((uint8) 255,
                           (uint8) (c[PixelRGB::indexR] >> 8),
                           (uint8) (c[PixelRGB::indexG] >> 8),
                           (uint8) (c[PixelRGB::indexB] >> 8));
        }

        //==============================================================================
        void render4PixelAverage (PixelAlpha* dest, const uint8* src, uint32 subPixelX, uint32 subPixelY) noexcept
        {
            uint32 c = 256 * 128;
            c += src[0] * ((256 - subPixelX) * (256 - subPixelY));
            src += this->srcData.pixelStride;
            c += src[0] * (subPixelX * (256 - subPixelY));
            src += this->srcData.lineStride;
            c += src[0] * (subPixelX * subPixelY);
            src -= this->srcData.pixelStride;

            c += src[0] * ((256 - subPixelX) * subPixelY);

            *((uint8*) dest) = (uint8) (c >> 16);
        }

        void render2PixelAverageX (PixelAlpha* dest, const uint8* src, uint32 subPixelX) noexcept
        {
            uint32 c = 128;
            c += src[0] * (256 - subPixelX);
            src += this->srcData.pixelStride;
            c += src[0] * subPixelX;
            *((uint8*) dest) = (uint8) (c >> 8);
        }

        void render2PixelAverageY (PixelAlpha* dest, const uint8* src, uint32 subPixelY) noexcept
        {
            uint32 c = 128;
            c += src[0] * (256 - subPixelY);
            src += this->srcData.lineStride;
            c += src[0] * subPixelY;
            *((uint8*) dest) = (uint8) (c >> 8);
        }

        //==============================================================================
        struct TransformedImageSpanInterpolator
        {
            TransformedImageSpanInterpolator (const AffineTransform& transform, float offsetFloat, int offsetInt) noexcept
                : inverseTransform (transform.inverted()),
                  pixelOffset (offsetFloat), pixelOffsetInt (offsetInt)
            {}

            void setStartOfLine (float sx, float sy, int numPixels) noexcept
            {
                jassert (numPixels > 0);

                sx += pixelOffset;
                sy += pixelOffset;
                auto x1 = sx, y1 = sy;
                sx += (float) numPixels;
                inverseTransform.transformPoints (x1, y1, sx, sy);

                xBresenham.set ((int) (x1 * 256.0f), (int) (sx * 256.0f), numPixels, pixelOffsetInt);
                yBresenham.set ((int) (y1 * 256.0f), (int) (sy * 256.0f), numPixels, pixelOffsetInt);
            }

            void next (int& px, int& py) noexcept
            {
                px = xBresenham.n;  xBresenham.stepToNext();
                py = yBresenham.n;  yBresenham.stepToNext();
            }

        private:
            struct BresenhamInterpolator
            {
                BresenhamInterpolator() = default;

                void set (int n1, int n2, int steps, int offsetInt) noexcept
                {
                    numSteps = steps;
                    step = (n2 - n1) / numSteps;
                    remainder = modulo = (n2 - n1) % numSteps;
                    n = n1 + offsetInt;

                    if (modulo <= 0)
                    {
                        modulo += numSteps;
                        remainder += numSteps;
                        --step;
                    }

                    modulo -= numSteps;
                }

                forcedinline void stepToNext() noexcept
                {
                    modulo += remainder;
                    n += step;

                    if (modulo > 0)
                    {
                        modulo -= numSteps;
                        ++n;
                    }
                }

                int n;

            private:
                int numSteps, step, modulo, remainder;
            };

            const AffineTransform inverseTransform;
            BresenhamInterpolator xBresenham, yBresenham;
            const float pixelOffset;
            const int pixelOffsetInt;

            JUCE_DECLARE_NON_COPYABLE (TransformedImageSpanInterpolator)
        };

        //==============================================================================
        TransformedImageSpanInterpolator interpolator;
        const Image::BitmapData& destData;
        const Image::BitmapData& srcData;
        const int extraAlpha;
        const Graphics::ResamplingQuality quality;
        const int maxX, maxY;
        int currentY;
        DestPixelType* linePixels;
        HeapBlock<SrcPixelType> scratchBuffer;
        size_t scratchSize = 2048;

        JUCE_DECLARE_NON_COPYABLE (TransformedImageFill)
    };


    //==============================================================================
    template <class Iterator>
    void renderImageTransformed (Iterator& iter, const Image::BitmapData& destData, const Image::BitmapData& srcData,
                                 int alpha, const AffineTransform& transform, Graphics::ResamplingQuality quality, bool tiledFill)
    {
        switch (destData.pixelFormat)
        {
        case Image::ARGB:
            switch (srcData.pixelFormat)
            {
            case Image::ARGB:
                if (tiledFill)  { TransformedImageFill<PixelARGB, PixelARGB, true>  r (destData, srcData, transform, alpha, quality); iter.iterate (r); }
                else            { TransformedImageFill<PixelARGB, PixelARGB, false> r (destData, srcData, transform, alpha, quality); iter.iterate (r); }
                break;
            case Image::RGB:
                if (tiledFill)  { TransformedImageFill<PixelARGB, PixelRGB, true>  r (destData, srcData, transform, alpha, quality); iter.iterate (r); }
                else            { TransformedImageFill<PixelARGB, PixelRGB, false> r (destData, srcData, transform, alpha, quality); iter.iterate (r); }
                break;
            case Image::SingleChannel:
            case Image::UnknownFormat:
            default:
                if (tiledFill)  { TransformedImageFill<PixelARGB, PixelAlpha, true>  r (destData, srcData, transform, alpha, quality); iter.iterate (r); }
                else            { TransformedImageFill<PixelARGB, PixelAlpha, false> r (destData, srcData, transform, alpha, quality); iter.iterate (r); }
                break;
            }
            break;

        case Image::RGB:
        {
            switch (srcData.pixelFormat)
            {
            case Image::ARGB:
                if (tiledFill)  { TransformedImageFill<PixelRGB, PixelARGB, true>  r (destData, srcData, transform, alpha, quality); iter.iterate (r); }
                else            { TransformedImageFill<PixelRGB, PixelARGB, false> r (destData, srcData, transform, alpha, quality); iter.iterate (r); }
                break;
            case Image::RGB:
                if (tiledFill)  { TransformedImageFill<PixelRGB, PixelRGB, true>  r (destData, srcData, transform, alpha, quality); iter.iterate (r); }
                else            { TransformedImageFill<PixelRGB, PixelRGB, false> r (destData, srcData, transform, alpha, quality); iter.iterate (r); }
                break;
            case Image::SingleChannel:
            case Image::UnknownFormat:
            default:
                if (tiledFill)  { TransformedImageFill<PixelRGB, PixelAlpha, true>  r (destData, srcData, transform, alpha, quality); iter.iterate (r); }
                else            { TransformedImageFill<PixelRGB, PixelAlpha, false> r (destData, srcData, transform, alpha, quality); iter.iterate (r); }
                break;
            }
            break;
        }

        case Image::SingleChannel:
        case Image::UnknownFormat:
        default:
            switch (srcData.pixelFormat)
            {
            case Image::ARGB:
                if (tiledFill)  { TransformedImageFill<PixelAlpha, PixelARGB, true>  r (destData, srcData, transform, alpha, quality); iter.iterate (r); }
                else            { TransformedImageFill<PixelAlpha, PixelARGB, false> r (destData, srcData, transform, alpha, quality); iter.iterate (r); }
                break;
            case Image::RGB:
                if (tiledFill)  { TransformedImageFill<PixelAlpha, PixelRGB, true>  r (destData, srcData, transform, alpha, quality); iter.iterate (r); }
                else            { TransformedImageFill<PixelAlpha, PixelRGB, false> r (destData, srcData, transform, alpha, quality); iter.iterate (r); }
                break;
            case Image::SingleChannel:
            case Image::UnknownFormat:
            default:
                if (tiledFill)  { TransformedImageFill<PixelAlpha, PixelAlpha, true>  r (destData, srcData, transform, alpha, quality); iter.iterate (r); }
                else            { TransformedImageFill<PixelAlpha, PixelAlpha, false> r (destData, srcData, transform, alpha, quality); iter.iterate (r); }
                break;
            }
            break;
        }
    }

    template <class Iterator>
    void renderImageUntransformed (Iterator& iter, const Image::BitmapData& destData, const Image::BitmapData& srcData, int alpha, int x, int y, bool tiledFill)
    {
        switch (destData.pixelFormat)
        {
        case Image::ARGB:
            switch (srcData.pixelFormat)
            {
            case Image::ARGB:
                if (tiledFill)  { ImageFill<PixelARGB, PixelARGB, true>  r (destData, srcData, alpha, x, y); iter.iterate (r); }
                else            { ImageFill<PixelARGB, PixelARGB, false> r (destData, srcData, alpha, x, y); iter.iterate (r); }
                break;
            case Image::RGB:
                if (tiledFill)  { ImageFill<PixelARGB, PixelRGB, true>  r (destData, srcData, alpha, x, y); iter.iterate (r); }
                else            { ImageFill<PixelARGB, PixelRGB, false> r (destData, srcData, alpha, x, y); iter.iterate (r); }
                break;
            case Image::SingleChannel:
            case Image::UnknownFormat:
            default:
                if (tiledFill)  { ImageFill<PixelARGB, PixelAlpha, true>  r (destData, srcData, alpha, x, y); iter.iterate (r); }
                else            { ImageFill<PixelARGB, PixelAlpha, false> r (destData, srcData, alpha, x, y); iter.iterate (r); }
                break;
            }
            break;

        case Image::RGB:
            switch (srcData.pixelFormat)
            {
            case Image::ARGB:
                if (tiledFill)  { ImageFill<PixelRGB, PixelARGB, true>  r (destData, srcData, alpha, x, y); iter.iterate (r); }
                else            { ImageFill<PixelRGB, PixelARGB, false> r (destData, srcData, alpha, x, y); iter.iterate (r); }
                break;
            case Image::RGB:
                if (tiledFill)  { ImageFill<PixelRGB, PixelRGB, true>  r (destData, srcData, alpha, x, y); iter.iterate (r); }
                else            { ImageFill<PixelRGB, PixelRGB, false> r (destData, srcData, alpha, x, y); iter.iterate (r); }
                break;
            case Image::SingleChannel:
            case Image::UnknownFormat:
            default:
                if (tiledFill)  { ImageFill<PixelRGB, PixelAlpha, true>  r (destData, srcData, alpha, x, y); iter.iterate (r); }
                else            { ImageFill<PixelRGB, PixelAlpha, false> r (destData, srcData, alpha, x, y); iter.iterate (r); }
                break;
            }
            break;

        case Image::SingleChannel:
        case Image::UnknownFormat:
        default:
            switch (srcData.pixelFormat)
            {
            case Image::ARGB:
                if (tiledFill)  { ImageFill<PixelAlpha, PixelARGB, true>   r (destData, srcData, alpha, x, y); iter.iterate (r); }
                else            { ImageFill<PixelAlpha, PixelARGB, false>  r (destData, srcData, alpha, x, y); iter.iterate (r); }
                break;
            case Image::RGB:
                if (tiledFill)  { ImageFill<PixelAlpha, PixelRGB, true>    r (destData, srcData, alpha, x, y); iter.iterate (r); }
                else            { ImageFill<PixelAlpha, PixelRGB, false>   r (destData, srcData, alpha, x, y); iter.iterate (r); }
                break;
            case Image::SingleChannel:
            case Image::UnknownFormat:
            default:
                if (tiledFill)  { ImageFill<PixelAlpha, PixelAlpha, true>  r (destData, srcData, alpha, x, y); iter.iterate (r); }
                else            { ImageFill<PixelAlpha, PixelAlpha, false> r (destData, srcData, alpha, x, y); iter.iterate (r); }
                break;
            }
            break;
        }
    }

    template <class Iterator, class DestPixelType>
    void renderSolidFill (Iterator& iter, const Image::BitmapData& destData, PixelARGB fillColour, bool replaceContents, DestPixelType*)
    {
        if (replaceContents)
        {
            EdgeTableFillers::SolidColour<DestPixelType, true> r (destData, fillColour);
            iter.iterate (r);
        }
        else
        {
            EdgeTableFillers::SolidColour<DestPixelType, false> r (destData, fillColour);
            iter.iterate (r);
        }
    }

    template <class Iterator, class DestPixelType>
    void renderGradient (Iterator& iter, const Image::BitmapData& destData, const ColourGradient& g, const AffineTransform& transform,
                         const PixelARGB* lookupTable, int numLookupEntries, bool isIdentity, DestPixelType*)
    {
        if (g.isRadial)
        {
            if (isIdentity)
            {
                EdgeTableFillers::Gradient<DestPixelType, GradientPixelIterators::Radial> renderer (destData, g, transform, lookupTable, numLookupEntries);
                iter.iterate (renderer);
            }
            else
            {
                EdgeTableFillers::Gradient<DestPixelType, GradientPixelIterators::TransformedRadial> renderer (destData, g, transform, lookupTable, numLookupEntries);
                iter.iterate (renderer);
            }
        }
        else
        {
            EdgeTableFillers::Gradient<DestPixelType, GradientPixelIterators::Linear> renderer (destData, g, transform, lookupTable, numLookupEntries);
            iter.iterate (renderer);
        }
    }
}

//==============================================================================
template <class SavedStateType>
struct ClipRegions
{
    struct Base  : public SingleThreadedReferenceCountedObject
    {
        Base() = default;
        ~Base() override = default;

        using Ptr = ReferenceCountedObjectPtr<Base>;

        virtual Ptr clone() const = 0;
        virtual Ptr applyClipTo (const Ptr& target) const = 0;

        virtual Ptr clipToRectangle (Rectangle<int>) = 0;
        virtual Ptr clipToRectangleList (const RectangleList<int>&) = 0;
        virtual Ptr excludeClipRectangle (Rectangle<int>) = 0;
        virtual Ptr clipToPath (const Path&, const AffineTransform&) = 0;
        virtual Ptr clipToEdgeTable (const EdgeTable&) = 0;
        virtual Ptr clipToImageAlpha (const Image&, const AffineTransform&, Graphics::ResamplingQuality) = 0;
        virtual void translate (Point<int> delta) = 0;

        virtual bool clipRegionIntersects (Rectangle<int>) const = 0;
        virtual Rectangle<int> getClipBounds() const = 0;

        virtual void fillRectWithColour (SavedStateType&, Rectangle<int>, PixelARGB colour, bool replaceContents) const = 0;
        virtual void fillRectWithColour (SavedStateType&, Rectangle<float>, PixelARGB colour) const = 0;
        virtual void fillAllWithColour (SavedStateType&, PixelARGB colour, bool replaceContents) const = 0;
        virtual void fillAllWithGradient (SavedStateType&, ColourGradient&, const AffineTransform&, bool isIdentity) const = 0;
        virtual void renderImageTransformed (SavedStateType&, const Image&, int alpha, const AffineTransform&, Graphics::ResamplingQuality, bool tiledFill) const = 0;
        virtual void renderImageUntransformed (SavedStateType&, const Image&, int alpha, int x, int y, bool tiledFill) const = 0;
    };

    //==============================================================================
    struct EdgeTableRegion  : public Base
    {
        EdgeTableRegion (const EdgeTable& e)            : edgeTable (e) {}
        EdgeTableRegion (Rectangle<int> r)              : edgeTable (r) {}
        EdgeTableRegion (Rectangle<float> r)            : edgeTable (r) {}
        EdgeTableRegion (const RectangleList<int>& r)   : edgeTable (r) {}
        EdgeTableRegion (const RectangleList<float>& r) : edgeTable (r) {}
        EdgeTableRegion (Rectangle<int> bounds, const Path& p, const AffineTransform& t) : edgeTable (bounds, p, t) {}

        EdgeTableRegion (const EdgeTableRegion& other)  : Base(), edgeTable (other.edgeTable) {}
        EdgeTableRegion& operator= (const EdgeTableRegion&) = delete;

        using Ptr = typename Base::Ptr;

        Ptr clone() const override                           { return *new EdgeTableRegion (*this); }
        Ptr applyClipTo (const Ptr& target) const override   { return target->clipToEdgeTable (edgeTable); }

        Ptr clipToRectangle (Rectangle<int> r) override
        {
            edgeTable.clipToRectangle (r);
            return edgeTable.isEmpty() ? Ptr() : Ptr (*this);
        }

        Ptr clipToRectangleList (const RectangleList<int>& r) override
        {
            RectangleList<int> inverse (edgeTable.getMaximumBounds());

            if (inverse.subtract (r))
                for (auto& i : inverse)
                    edgeTable.excludeRectangle (i);

            return edgeTable.isEmpty() ? Ptr() : Ptr (*this);
        }

        Ptr excludeClipRectangle (Rectangle<int> r) override
        {
            edgeTable.excludeRectangle (r);
            return edgeTable.isEmpty() ? Ptr() : Ptr (*this);
        }

        Ptr clipToPath (const Path& p, const AffineTransform& transform) override
        {
            EdgeTable et (edgeTable.getMaximumBounds(), p, transform);
            edgeTable.clipToEdgeTable (et);
            return edgeTable.isEmpty() ? Ptr() : Ptr (*this);
        }

        Ptr clipToEdgeTable (const EdgeTable& et) override
        {
            edgeTable.clipToEdgeTable (et);
            return edgeTable.isEmpty() ? Ptr() : Ptr (*this);
        }

        Ptr clipToImageAlpha (const Image& image, const AffineTransform& transform, Graphics::ResamplingQuality quality) override
        {
            const Image::BitmapData srcData (image, Image::BitmapData::readOnly);

            if (transform.isOnlyTranslation())
            {
                // If our translation doesn't involve any distortion, just use a simple blit..
                auto tx = (int) (transform.getTranslationX() * 256.0f);
                auto ty = (int) (transform.getTranslationY() * 256.0f);

                if (quality == Graphics::lowResamplingQuality || ((tx | ty) & 224) == 0)
                {
                    auto imageX = ((tx + 128) >> 8);
                    auto imageY = ((ty + 128) >> 8);

                    if (image.getFormat() == Image::ARGB)
                        straightClipImage (srcData, imageX, imageY, (PixelARGB*) nullptr);
                    else
                        straightClipImage (srcData, imageX, imageY, (PixelAlpha*) nullptr);

                    return edgeTable.isEmpty() ? Ptr() : Ptr (*this);
                }
            }

            if (transform.isSingularity())
                return Ptr();

            {
                Path p;
                p.addRectangle (0, 0, (float) srcData.width, (float) srcData.height);
                EdgeTable et2 (edgeTable.getMaximumBounds(), p, transform);
                edgeTable.clipToEdgeTable (et2);
            }

            if (! edgeTable.isEmpty())
            {
                if (image.getFormat() == Image::ARGB)
                    transformedClipImage (srcData, transform, quality, (PixelARGB*) nullptr);
                else
                    transformedClipImage (srcData, transform, quality, (PixelAlpha*) nullptr);
            }

            return edgeTable.isEmpty() ? Ptr() : Ptr (*this);
        }

        void translate (Point<int> delta) override
        {
            edgeTable.translate ((float) delta.x, delta.y);
        }

        bool clipRegionIntersects (Rectangle<int> r) const override
        {
            return edgeTable.getMaximumBounds().intersects (r);
        }

        Rectangle<int> getClipBounds() const override
        {
            return edgeTable.getMaximumBounds();
        }

        void fillRectWithColour (SavedStateType& state, Rectangle<int> area, PixelARGB colour, bool replaceContents) const override
        {
            auto totalClip = edgeTable.getMaximumBounds();
            auto clipped = totalClip.getIntersection (area);

            if (! clipped.isEmpty())
            {
                EdgeTableRegion et (clipped);
                et.edgeTable.clipToEdgeTable (edgeTable);
                state.fillWithSolidColour (et.edgeTable, colour, replaceContents);
            }
        }

        void fillRectWithColour (SavedStateType& state, Rectangle<float> area, PixelARGB colour) const override
        {
            auto totalClip = edgeTable.getMaximumBounds().toFloat();
            auto clipped = totalClip.getIntersection (area);

            if (! clipped.isEmpty())
            {
                EdgeTableRegion et (clipped);
                et.edgeTable.clipToEdgeTable (edgeTable);
                state.fillWithSolidColour (et.edgeTable, colour, false);
            }
        }

        void fillAllWithColour (SavedStateType& state, PixelARGB colour, bool replaceContents) const override
        {
            state.fillWithSolidColour (edgeTable, colour, replaceContents);
        }

        void fillAllWithGradient (SavedStateType& state, ColourGradient& gradient, const AffineTransform& transform, bool isIdentity) const override
        {
            state.fillWithGradient (edgeTable, gradient, transform, isIdentity);
        }

        void renderImageTransformed (SavedStateType& state, const Image& src, int alpha, const AffineTransform& transform, Graphics::ResamplingQuality quality, bool tiledFill) const override
        {
            state.renderImageTransformed (edgeTable, src, alpha, transform, quality, tiledFill);
        }

        void renderImageUntransformed (SavedStateType& state, const Image& src, int alpha, int x, int y, bool tiledFill) const override
        {
            state.renderImageUntransformed (edgeTable, src, alpha, x, y, tiledFill);
        }

        EdgeTable edgeTable;

    private:
        template <class SrcPixelType>
        void transformedClipImage (const Image::BitmapData& srcData, const AffineTransform& transform, Graphics::ResamplingQuality quality, const SrcPixelType*)
        {
            EdgeTableFillers::TransformedImageFill<SrcPixelType, SrcPixelType, false> renderer (srcData, srcData, transform, 255, quality);

            for (int y = 0; y < edgeTable.getMaximumBounds().getHeight(); ++y)
                renderer.clipEdgeTableLine (edgeTable, edgeTable.getMaximumBounds().getX(), y + edgeTable.getMaximumBounds().getY(),
                                            edgeTable.getMaximumBounds().getWidth());
        }

        template <class SrcPixelType>
        void straightClipImage (const Image::BitmapData& srcData, int imageX, int imageY, const SrcPixelType*)
        {
            Rectangle<int> r (imageX, imageY, srcData.width, srcData.height);
            edgeTable.clipToRectangle (r);

            EdgeTableFillers::ImageFill<SrcPixelType, SrcPixelType, false> renderer (srcData, srcData, 255, imageX, imageY);

            for (int y = 0; y < r.getHeight(); ++y)
                renderer.clipEdgeTableLine (edgeTable, r.getX(), y + r.getY(), r.getWidth());
        }
    };

    //==============================================================================
    class RectangleListRegion  : public Base
    {
    public:
        RectangleListRegion (Rectangle<int> r) : clip (r) {}
        RectangleListRegion (const RectangleList<int>& r)  : clip (r) {}
        RectangleListRegion (const RectangleListRegion& other) : Base(), clip (other.clip) {}

        using Ptr = typename Base::Ptr;

        Ptr clone() const override                           { return *new RectangleListRegion (*this); }
        Ptr applyClipTo (const Ptr& target) const override   { return target->clipToRectangleList (clip); }

        Ptr clipToRectangle (Rectangle<int> r) override
        {
            clip.clipTo (r);
            return clip.isEmpty() ? Ptr() : Ptr (*this);
        }

        Ptr clipToRectangleList (const RectangleList<int>& r) override
        {
            clip.clipTo (r);
            return clip.isEmpty() ? Ptr() : Ptr (*this);
        }

        Ptr excludeClipRectangle (Rectangle<int> r) override
        {
            clip.subtract (r);
            return clip.isEmpty() ? Ptr() : Ptr (*this);
        }

        Ptr clipToPath (const Path& p, const AffineTransform& transform) override  { return toEdgeTable()->clipToPath (p, transform); }
        Ptr clipToEdgeTable (const EdgeTable& et) override                         { return toEdgeTable()->clipToEdgeTable (et); }

        Ptr clipToImageAlpha (const Image& image, const AffineTransform& transform, Graphics::ResamplingQuality quality) override
        {
            return toEdgeTable()->clipToImageAlpha (image, transform, quality);
        }

        void translate (Point<int> delta) override                    { clip.offsetAll (delta); }
        bool clipRegionIntersects (Rectangle<int> r) const override   { return clip.intersects (r); }
        Rectangle<int> getClipBounds() const override                 { return clip.getBounds(); }

        void fillRectWithColour (SavedStateType& state, Rectangle<int> area, PixelARGB colour, bool replaceContents) const override
        {
            SubRectangleIterator iter (clip, area);
            state.fillWithSolidColour (iter, colour, replaceContents);
        }

        void fillRectWithColour (SavedStateType& state, Rectangle<float> area, PixelARGB colour) const override
        {
            SubRectangleIteratorFloat iter (clip, area);
            state.fillWithSolidColour (iter, colour, false);
        }

        void fillAllWithColour (SavedStateType& state, PixelARGB colour, bool replaceContents) const override
        {
            state.fillWithSolidColour (*this, colour, replaceContents);
        }

        void fillAllWithGradient (SavedStateType& state, ColourGradient& gradient, const AffineTransform& transform, bool isIdentity) const override
        {
            state.fillWithGradient (*this, gradient, transform, isIdentity);
        }

        void renderImageTransformed (SavedStateType& state, const Image& src, int alpha, const AffineTransform& transform, Graphics::ResamplingQuality quality, bool tiledFill) const override
        {
            state.renderImageTransformed (*this, src, alpha, transform, quality, tiledFill);
        }

        void renderImageUntransformed (SavedStateType& state, const Image& src, int alpha, int x, int y, bool tiledFill) const override
        {
            state.renderImageUntransformed (*this, src, alpha, x, y, tiledFill);
        }

        RectangleList<int> clip;

        //==============================================================================
        template <class Renderer>
        void iterate (Renderer& r) const noexcept
        {
            for (auto& i : clip)
            {
                auto x = i.getX();
                auto w = i.getWidth();
                jassert (w > 0);
                auto bottom = i.getBottom();

                for (int y = i.getY(); y < bottom; ++y)
                {
                    r.setEdgeTableYPos (y);
                    r.handleEdgeTableLineFull (x, w);
                }
            }
        }

    private:
        //==============================================================================
        class SubRectangleIterator
        {
        public:
            SubRectangleIterator (const RectangleList<int>& clipList, Rectangle<int> clipBounds)
                : clip (clipList), area (clipBounds)
            {}

            template <class Renderer>
            void iterate (Renderer& r) const noexcept
            {
                for (auto& i : clip)
                {
                    auto rect = i.getIntersection (area);

                    if (! rect.isEmpty())
                        r.handleEdgeTableRectangleFull (rect.getX(), rect.getY(), rect.getWidth(), rect.getHeight());
                }
            }

        private:
            const RectangleList<int>& clip;
            const Rectangle<int> area;

            JUCE_DECLARE_NON_COPYABLE (SubRectangleIterator)
        };

        //==============================================================================
        class SubRectangleIteratorFloat
        {
        public:
            SubRectangleIteratorFloat (const RectangleList<int>& clipList, Rectangle<float> clipBounds) noexcept
                : clip (clipList), area (clipBounds)
            {
            }

            template <class Renderer>
            void iterate (Renderer& r) const noexcept
            {
                const RenderingHelpers::FloatRectangleRasterisingInfo f (area);

                for (auto& i : clip)
                {
                    auto clipLeft   = i.getX();
                    auto clipRight  = i.getRight();
                    auto clipTop    = i.getY();
                    auto clipBottom = i.getBottom();

                    if (f.totalBottom > clipTop && f.totalTop < clipBottom
                         && f.totalRight > clipLeft && f.totalLeft < clipRight)
                    {
                        if (f.isOnePixelWide())
                        {
                            if (f.topAlpha != 0 && f.totalTop >= clipTop)
                            {
                                r.setEdgeTableYPos (f.totalTop);
                                r.handleEdgeTablePixel (f.left, f.topAlpha);
                            }

                            auto y1 = jmax (clipTop, f.top);
                            auto y2 = jmin (f.bottom, clipBottom);
                            auto h = y2 - y1;

                            if (h > 0)
                                r.handleEdgeTableRectangleFull (f.left, y1, 1, h);

                            if (f.bottomAlpha != 0 && f.bottom < clipBottom)
                            {
                                r.setEdgeTableYPos (f.bottom);
                                r.handleEdgeTablePixel (f.left, f.bottomAlpha);
                            }
                        }
                        else
                        {
                            auto clippedLeft   = jmax (f.left, clipLeft);
                            auto clippedWidth  = jmin (f.right, clipRight) - clippedLeft;
                            bool doLeftAlpha  = f.leftAlpha != 0 && f.totalLeft >= clipLeft;
                            bool doRightAlpha = f.rightAlpha != 0 && f.right < clipRight;

                            if (f.topAlpha != 0 && f.totalTop >= clipTop)
                            {
                                r.setEdgeTableYPos (f.totalTop);

                                if (doLeftAlpha)        r.handleEdgeTablePixel (f.totalLeft, f.getTopLeftCornerAlpha());
                                if (clippedWidth > 0)   r.handleEdgeTableLine (clippedLeft, clippedWidth, f.topAlpha);
                                if (doRightAlpha)       r.handleEdgeTablePixel (f.right, f.getTopRightCornerAlpha());
                            }

                            auto y1 = jmax (clipTop, f.top);
                            auto y2 = jmin (f.bottom, clipBottom);
                            auto h = y2 - y1;

                            if (h > 0)
                            {
                                if (h == 1)
                                {
                                    r.setEdgeTableYPos (y1);

                                    if (doLeftAlpha)        r.handleEdgeTablePixel (f.totalLeft, f.leftAlpha);
                                    if (clippedWidth > 0)   r.handleEdgeTableLineFull (clippedLeft, clippedWidth);
                                    if (doRightAlpha)       r.handleEdgeTablePixel (f.right, f.rightAlpha);
                                }
                                else
                                {
                                    if (doLeftAlpha)        r.handleEdgeTableRectangle (f.totalLeft, y1, 1, h, f.leftAlpha);
                                    if (clippedWidth > 0)   r.handleEdgeTableRectangleFull (clippedLeft, y1, clippedWidth, h);
                                    if (doRightAlpha)       r.handleEdgeTableRectangle (f.right, y1, 1, h, f.rightAlpha);
                                }
                            }

                            if (f.bottomAlpha != 0 && f.bottom < clipBottom)
                            {
                                r.setEdgeTableYPos (f.bottom);

                                if (doLeftAlpha)        r.handleEdgeTablePixel (f.totalLeft, f.getBottomLeftCornerAlpha());
                                if (clippedWidth > 0)   r.handleEdgeTableLine (clippedLeft, clippedWidth, f.bottomAlpha);
                                if (doRightAlpha)       r.handleEdgeTablePixel (f.right, f.getBottomRightCornerAlpha());
                            }
                        }
                    }
                }
            }

        private:
            const RectangleList<int>& clip;
            Rectangle<float> area;

            JUCE_DECLARE_NON_COPYABLE (SubRectangleIteratorFloat)
        };

        Ptr toEdgeTable() const   { return *new EdgeTableRegion (clip); }

        RectangleListRegion& operator= (const RectangleListRegion&) = delete;
    };
};

//==============================================================================
template <class SavedStateType>
class SavedStateBase
{
public:
    using BaseRegionType           = typename ClipRegions<SavedStateType>::Base;
    using EdgeTableRegionType      = typename ClipRegions<SavedStateType>::EdgeTableRegion;
    using RectangleListRegionType  = typename ClipRegions<SavedStateType>::RectangleListRegion;

    SavedStateBase (Rectangle<int> initialClip)
        : clip (new RectangleListRegionType (initialClip)),
          interpolationQuality (Graphics::mediumResamplingQuality), transparencyLayerAlpha (1.0f)
    {
    }

    SavedStateBase (const RectangleList<int>& clipList, Point<int> origin)
        : clip (new RectangleListRegionType (clipList)), transform (origin),
          interpolationQuality (Graphics::mediumResamplingQuality), transparencyLayerAlpha (1.0f)
    {
    }

    SavedStateBase (const SavedStateBase& other)
        : clip (other.clip), transform (other.transform), fillType (other.fillType),
          interpolationQuality (other.interpolationQuality),
          transparencyLayerAlpha (other.transparencyLayerAlpha)
    {
    }

    SavedStateType& getThis() noexcept  { return *static_cast<SavedStateType*> (this); }

    bool clipToRectangle (Rectangle<int> r)
    {
        if (clip != nullptr)
        {
            if (transform.isOnlyTranslated)
            {
                cloneClipIfMultiplyReferenced();
                clip = clip->clipToRectangle (transform.translated (r));
            }
            else if (! transform.isRotated)
            {
                cloneClipIfMultiplyReferenced();
                clip = clip->clipToRectangle (transform.transformed (r));
            }
            else
            {
                Path p;
                p.addRectangle (r);
                clipToPath (p, {});
            }
        }

        return clip != nullptr;
    }

    bool clipToRectangleList (const RectangleList<int>& r)
    {
        if (clip != nullptr)
        {
            if (transform.isOnlyTranslated)
            {
                cloneClipIfMultiplyReferenced();

                if (transform.isIdentity())
                {
                    clip = clip->clipToRectangleList (r);
                }
                else
                {
                    RectangleList<int> offsetList (r);
                    offsetList.offsetAll (transform.offset);
                    clip = clip->clipToRectangleList (offsetList);
                }
            }
            else if (! transform.isRotated)
            {
                cloneClipIfMultiplyReferenced();
                RectangleList<int> scaledList;

                for (auto& i : r)
                    scaledList.add (transform.transformed (i));

                clip = clip->clipToRectangleList (scaledList);
            }
            else
            {
                clipToPath (r.toPath(), {});
            }
        }

        return clip != nullptr;
    }

    static Rectangle<int> getLargestIntegerWithin (Rectangle<float> r)
    {
        auto x1 = (int) std::ceil (r.getX());
        auto y1 = (int) std::ceil (r.getY());
        auto x2 = (int) std::floor (r.getRight());
        auto y2 = (int) std::floor (r.getBottom());

        return { x1, y1, x2 - x1, y2 - y1 };
    }

    bool excludeClipRectangle (Rectangle<int> r)
    {
        if (clip != nullptr)
        {
            cloneClipIfMultiplyReferenced();

            if (transform.isOnlyTranslated)
            {
                clip = clip->excludeClipRectangle (getLargestIntegerWithin (transform.translated (r.toFloat())));
            }
            else if (! transform.isRotated)
            {
                clip = clip->excludeClipRectangle (getLargestIntegerWithin (transform.transformed (r.toFloat())));
            }
            else
            {
                Path p;
                p.addRectangle (r.toFloat());
                p.applyTransform (transform.complexTransform);
                p.addRectangle (clip->getClipBounds().toFloat());
                p.setUsingNonZeroWinding (false);
                clip = clip->clipToPath (p, {});
            }
        }

        return clip != nullptr;
    }

    void clipToPath (const Path& p, const AffineTransform& t)
    {
        if (clip != nullptr)
        {
            cloneClipIfMultiplyReferenced();
            clip = clip->clipToPath (p, transform.getTransformWith (t));
        }
    }

    void clipToImageAlpha (const Image& sourceImage, const AffineTransform& t)
    {
        if (clip != nullptr)
        {
            if (sourceImage.hasAlphaChannel())
            {
                cloneClipIfMultiplyReferenced();
                clip = clip->clipToImageAlpha (sourceImage, transform.getTransformWith (t), interpolationQuality);
            }
            else
            {
                Path p;
                p.addRectangle (sourceImage.getBounds());
                clipToPath (p, t);
            }
        }
    }

    bool clipRegionIntersects (Rectangle<int> r) const
    {
        if (clip != nullptr)
        {
            if (transform.isOnlyTranslated)
                return clip->clipRegionIntersects (transform.translated (r));

            return getClipBounds().intersects (r);
        }

        return false;
    }

    Rectangle<int> getClipBounds() const
    {
        return clip != nullptr ? transform.deviceSpaceToUserSpace (clip->getClipBounds())
                               : Rectangle<int>();
    }

    void setFillType (const FillType& newFill)
    {
        fillType = newFill;
    }

    void fillTargetRect (Rectangle<int> r, bool replaceContents)
    {
        if (fillType.isColour())
        {
            clip->fillRectWithColour (getThis(), r, fillType.colour.getPixelARGB(), replaceContents);
        }
        else
        {
            auto clipped = clip->getClipBounds().getIntersection (r);

            if (! clipped.isEmpty())
                fillShape (*new RectangleListRegionType (clipped), false);
        }
    }

    void fillTargetRect (Rectangle<float> r)
    {
        if (fillType.isColour())
        {
            clip->fillRectWithColour (getThis(), r, fillType.colour.getPixelARGB());
        }
        else
        {
            auto clipped = clip->getClipBounds().toFloat().getIntersection (r);

            if (! clipped.isEmpty())
                fillShape (*new EdgeTableRegionType (clipped), false);
        }
    }

    template <typename CoordType>
    void fillRectAsPath (Rectangle<CoordType> r)
    {
        Path p;
        p.addRectangle (r);
        fillPath (p, {});
    }

    void fillRect (Rectangle<int> r, bool replaceContents)
    {
        if (clip != nullptr)
        {
            if (transform.isOnlyTranslated)
            {
                fillTargetRect (transform.translated (r), replaceContents);
            }
            else if (! transform.isRotated)
            {
                fillTargetRect (transform.transformed (r), replaceContents);
            }
            else
            {
                jassert (! replaceContents); // not implemented..
                fillRectAsPath (r);
            }
        }
    }

    void fillRect (Rectangle<float> r)
    {
        if (clip != nullptr)
        {
            if (transform.isOnlyTranslated)
                fillTargetRect (transform.translated (r));
            else if (! transform.isRotated)
                fillTargetRect (transform.transformed (r));
            else
                fillRectAsPath (r);
        }
    }

    void fillRectList (const RectangleList<float>& list)
    {
        if (clip != nullptr)
        {
            if (list.getNumRectangles() == 1)
                return fillRect (*list.begin());

            if (transform.isIdentity())
            {
                fillShape (*new EdgeTableRegionType (list), false);
            }
            else if (! transform.isRotated)
            {
                RectangleList<float> transformed (list);

                if (transform.isOnlyTranslated)
                    transformed.offsetAll (transform.offset.toFloat());
                else
                    transformed.transformAll (transform.getTransform());

                fillShape (*new EdgeTableRegionType (transformed), false);
            }
            else
            {
                fillPath (list.toPath(), {});
            }
        }
    }

    void fillPath (const Path& path, const AffineTransform& t)
    {
        if (clip != nullptr)
        {
            auto trans = transform.getTransformWith (t);
            auto clipRect = clip->getClipBounds();

            if (path.getBoundsTransformed (trans).getSmallestIntegerContainer().intersects (clipRect))
                fillShape (*new EdgeTableRegionType (clipRect, path, trans), false);
        }
    }

    void fillEdgeTable (const EdgeTable& edgeTable, float x, int y)
    {
        if (clip != nullptr)
        {
            auto* edgeTableClip = new EdgeTableRegionType (edgeTable);
            edgeTableClip->edgeTable.translate (x, y);

            if (fillType.isColour())
            {
                auto brightness = fillType.colour.getBrightness() - 0.5f;

                if (brightness > 0.0f)
                    edgeTableClip->edgeTable.multiplyLevels (1.0f + 1.6f * brightness);
            }

            fillShape (*edgeTableClip, false);
        }
    }

    void drawLine (Line<float> line)
    {
        Path p;
        p.addLineSegment (line, 1.0f);
        fillPath (p, {});
    }

    void drawImage (const Image& sourceImage, const AffineTransform& trans)
    {
        if (clip != nullptr && ! fillType.colour.isTransparent())
            renderImage (sourceImage, trans, {});
    }

    static bool isOnlyTranslationAllowingError (const AffineTransform& t, float tolerance) noexcept
    {
        return std::abs (t.mat01) < tolerance
            && std::abs (t.mat10) < tolerance
            && std::abs (t.mat00 - 1.0f) < tolerance
            && std::abs (t.mat11 - 1.0f) < tolerance;
    }

    void renderImage (const Image& sourceImage, const AffineTransform& trans, const BaseRegionType* tiledFillClipRegion)
    {
        auto t = transform.getTransformWith (trans);
        auto alpha = fillType.colour.getAlpha();

        if (isOnlyTranslationAllowingError (t, 0.002f))
        {
            // If our translation doesn't involve any distortion, just use a simple blit..
            auto tx = (int) (t.getTranslationX() * 256.0f);
            auto ty = (int) (t.getTranslationY() * 256.0f);

            if (interpolationQuality == Graphics::lowResamplingQuality || ((tx | ty) & 224) == 0)
            {
                tx = ((tx + 128) >> 8);
                ty = ((ty + 128) >> 8);

                if (tiledFillClipRegion != nullptr)
                {
                    tiledFillClipRegion->renderImageUntransformed (getThis(), sourceImage, alpha, tx, ty, true);
                }
                else
                {
                    Rectangle<int> area (tx, ty, sourceImage.getWidth(), sourceImage.getHeight());
                    area = area.getIntersection (getThis().getMaximumBounds());

                    if (! area.isEmpty())
                        if (auto c = clip->applyClipTo (*new EdgeTableRegionType (area)))
                            c->renderImageUntransformed (getThis(), sourceImage, alpha, tx, ty, false);
                }

                return;
            }
        }

        if (! t.isSingularity())
        {
            if (tiledFillClipRegion != nullptr)
            {
                tiledFillClipRegion->renderImageTransformed (getThis(), sourceImage, alpha,
                                                             t, interpolationQuality, true);
            }
            else
            {
                Path p;
                p.addRectangle (sourceImage.getBounds());

                if (auto c = clip->clone()->clipToPath (p, t))
                    c->renderImageTransformed (getThis(), sourceImage, alpha,
                                               t, interpolationQuality, false);
            }
        }
    }

    void fillShape (typename BaseRegionType::Ptr shapeToFill, bool replaceContents)
    {
        jassert (clip != nullptr);
        shapeToFill = clip->applyClipTo (shapeToFill);

        if (shapeToFill != nullptr)
        {
            if (fillType.isGradient())
            {
                jassert (! replaceContents); // that option is just for solid colours

                auto g2 = *(fillType.gradient);
                g2.multiplyOpacity (fillType.getOpacity());
                auto t = transform.getTransformWith (fillType.transform).translated (-0.5f, -0.5f);

                bool isIdentity = t.isOnlyTranslation();

                if (isIdentity)
                {
                    // If our translation doesn't involve any distortion, we can speed it up..
                    g2.point1.applyTransform (t);
                    g2.point2.applyTransform (t);
                    t = {};
                }

                shapeToFill->fillAllWithGradient (getThis(), g2, t, isIdentity);
            }
            else if (fillType.isTiledImage())
            {
                renderImage (fillType.image, fillType.transform, shapeToFill.get());
            }
            else
            {
                shapeToFill->fillAllWithColour (getThis(), fillType.colour.getPixelARGB(), replaceContents);
            }
        }
    }

    void cloneClipIfMultiplyReferenced()
    {
        if (clip->getReferenceCount() > 1)
            clip = clip->clone();
    }

    typename BaseRegionType::Ptr clip;
    RenderingHelpers::TranslationOrTransform transform;
    FillType fillType;
    Graphics::ResamplingQuality interpolationQuality;
    float transparencyLayerAlpha;
};

//==============================================================================
class SoftwareRendererSavedState  : public SavedStateBase<SoftwareRendererSavedState>
{
    using BaseClass = SavedStateBase<SoftwareRendererSavedState>;

public:
    SoftwareRendererSavedState (const Image& im, Rectangle<int> clipBounds)
        : BaseClass (clipBounds), image (im)
    {
    }

    SoftwareRendererSavedState (const Image& im, const RectangleList<int>& clipList, Point<int> origin)
        : BaseClass (clipList, origin), image (im)
    {
    }

    SoftwareRendererSavedState (const SoftwareRendererSavedState& other) = default;

    SoftwareRendererSavedState* beginTransparencyLayer (float opacity)
    {
        auto* s = new SoftwareRendererSavedState (*this);

        if (clip != nullptr)
        {
            auto layerBounds = clip->getClipBounds();

            s->image = Image (Image::ARGB, layerBounds.getWidth(), layerBounds.getHeight(), true);
            s->transparencyLayerAlpha = opacity;
            s->transform.moveOriginInDeviceSpace (-layerBounds.getPosition());
            s->cloneClipIfMultiplyReferenced();
            s->clip->translate (-layerBounds.getPosition());
        }

        return s;
    }

    void endTransparencyLayer (SoftwareRendererSavedState& finishedLayerState)
    {
        if (clip != nullptr)
        {
            auto layerBounds = clip->getClipBounds();

            auto g = image.createLowLevelContext();
            g->setOpacity (finishedLayerState.transparencyLayerAlpha);
            g->drawImage (finishedLayerState.image, AffineTransform::translation (layerBounds.getPosition()));
        }
    }

    using GlyphCacheType = GlyphCache<CachedGlyphEdgeTable<SoftwareRendererSavedState>, SoftwareRendererSavedState>;

    static void clearGlyphCache()
    {
        GlyphCacheType::getInstance().reset();
    }

    //==============================================================================
    void drawGlyph (int glyphNumber, const AffineTransform& trans)
    {
        if (clip != nullptr)
        {
            if (trans.isOnlyTranslation() && ! transform.isRotated)
            {
                auto& cache = GlyphCacheType::getInstance();
                Point<float> pos (trans.getTranslationX(), trans.getTranslationY());

                if (transform.isOnlyTranslated)
                {
                    cache.drawGlyph (*this, font, glyphNumber, pos + transform.offset.toFloat());
                }
                else
                {
                    pos = transform.transformed (pos);

                    Font f (font);
                    f.setHeight (font.getHeight() * transform.complexTransform.mat11);

                    auto xScale = transform.complexTransform.mat00 / transform.complexTransform.mat11;

                    if (std::abs (xScale - 1.0f) > 0.01f)
                        f.setHorizontalScale (xScale);

                    cache.drawGlyph (*this, f, glyphNumber, pos);
                }
            }
            else
            {
                auto fontHeight = font.getHeight();

                auto t = transform.getTransformWith (AffineTransform::scale (fontHeight * font.getHorizontalScale(), fontHeight)
                                                                     .followedBy (trans));

                std::unique_ptr<EdgeTable> et (font.getTypefacePtr()->getEdgeTableForGlyph (glyphNumber, t, fontHeight));

                if (et != nullptr)
                    fillShape (*new EdgeTableRegionType (*et), false);
            }
        }
    }

    Rectangle<int> getMaximumBounds() const     { return image.getBounds(); }

    //==============================================================================
    template <typename IteratorType>
    void renderImageTransformed (IteratorType& iter, const Image& src, int alpha, const AffineTransform& trans, Graphics::ResamplingQuality quality, bool tiledFill) const
    {
        Image::BitmapData destData (image, Image::BitmapData::readWrite);
        const Image::BitmapData srcData (src, Image::BitmapData::readOnly);
        EdgeTableFillers::renderImageTransformed (iter, destData, srcData, alpha, trans, quality, tiledFill);
    }

    template <typename IteratorType>
    void renderImageUntransformed (IteratorType& iter, const Image& src, int alpha, int x, int y, bool tiledFill) const
    {
        Image::BitmapData destData (image, Image::BitmapData::readWrite);
        const Image::BitmapData srcData (src, Image::BitmapData::readOnly);
        EdgeTableFillers::renderImageUntransformed (iter, destData, srcData, alpha, x, y, tiledFill);
    }

    template <typename IteratorType>
    void fillWithSolidColour (IteratorType& iter, PixelARGB colour, bool replaceContents) const
    {
        Image::BitmapData destData (image, Image::BitmapData::readWrite);

        switch (destData.pixelFormat)
        {
            case Image::ARGB:   EdgeTableFillers::renderSolidFill (iter, destData, colour, replaceContents, (PixelARGB*) nullptr); break;
            case Image::RGB:    EdgeTableFillers::renderSolidFill (iter, destData, colour, replaceContents, (PixelRGB*) nullptr); break;
            case Image::SingleChannel:
            case Image::UnknownFormat:
            default:            EdgeTableFillers::renderSolidFill (iter, destData, colour, replaceContents, (PixelAlpha*) nullptr); break;
        }
    }

    template <typename IteratorType>
    void fillWithGradient (IteratorType& iter, ColourGradient& gradient, const AffineTransform& trans, bool isIdentity) const
    {
        HeapBlock<PixelARGB> lookupTable;
        auto numLookupEntries = gradient.createLookupTable (trans, lookupTable);
        jassert (numLookupEntries > 0);

        Image::BitmapData destData (image, Image::BitmapData::readWrite);

        switch (destData.pixelFormat)
        {
            case Image::ARGB:   EdgeTableFillers::renderGradient (iter, destData, gradient, trans, lookupTable, numLookupEntries, isIdentity, (PixelARGB*) nullptr); break;
            case Image::RGB:    EdgeTableFillers::renderGradient (iter, destData, gradient, trans, lookupTable, numLookupEntries, isIdentity, (PixelRGB*) nullptr); break;
            case Image::SingleChannel:
            case Image::UnknownFormat:
            default:            EdgeTableFillers::renderGradient (iter, destData, gradient, trans, lookupTable, numLookupEntries, isIdentity, (PixelAlpha*) nullptr); break;
        }
    }

    //==============================================================================
    Image image;
    Font font;

private:
    SoftwareRendererSavedState& operator= (const SoftwareRendererSavedState&) = delete;
};

//==============================================================================
template <class StateObjectType>
class SavedStateStack
{
public:
    SavedStateStack (StateObjectType* initialState) noexcept
        : currentState (initialState)
    {}

    SavedStateStack() = default;

    void initialise (StateObjectType* state)
    {
        currentState.reset (state);
    }

    inline StateObjectType* operator->() const noexcept     { return currentState.get(); }
    inline StateObjectType& operator*()  const noexcept     { return *currentState; }

    void save()
    {
        stack.add (new StateObjectType (*currentState));
    }

    void restore()
    {
        if (auto* top = stack.getLast())
        {
            currentState.reset (top);
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
        currentState.reset (currentState->beginTransparencyLayer (opacity));
    }

    void endTransparencyLayer()
    {
        std::unique_ptr<StateObjectType> finishedTransparencyLayer (currentState.release());
        restore();
        currentState->endTransparencyLayer (*finishedTransparencyLayer);
    }

private:
    std::unique_ptr<StateObjectType> currentState;
    OwnedArray<StateObjectType> stack;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SavedStateStack)
};

//==============================================================================
template <class SavedStateType>
class StackBasedLowLevelGraphicsContext  : public LowLevelGraphicsContext
{
public:
    bool isVectorDevice() const override                                         { return false; }
    void setOrigin (Point<int> o) override                                       { stack->transform.setOrigin (o); }
    void addTransform (const AffineTransform& t) override                        { stack->transform.addTransform (t); }
    float getPhysicalPixelScaleFactor() override                                 { return stack->transform.getPhysicalPixelScaleFactor(); }
    Rectangle<int> getClipBounds() const override                                { return stack->getClipBounds(); }
    bool isClipEmpty() const override                                            { return stack->clip == nullptr; }
    bool clipRegionIntersects (const Rectangle<int>& r) override                 { return stack->clipRegionIntersects (r); }
    bool clipToRectangle (const Rectangle<int>& r) override                      { return stack->clipToRectangle (r); }
    bool clipToRectangleList (const RectangleList<int>& r) override              { return stack->clipToRectangleList (r); }
    void excludeClipRectangle (const Rectangle<int>& r) override                 { stack->excludeClipRectangle (r); }
    void clipToPath (const Path& path, const AffineTransform& t) override        { stack->clipToPath (path, t); }
    void clipToImageAlpha (const Image& im, const AffineTransform& t) override   { stack->clipToImageAlpha (im, t); }
    void saveState() override                                                    { stack.save(); }
    void restoreState() override                                                 { stack.restore(); }
    void beginTransparencyLayer (float opacity) override                         { stack.beginTransparencyLayer (opacity); }
    void endTransparencyLayer() override                                         { stack.endTransparencyLayer(); }
    void setFill (const FillType& fillType) override                             { stack->setFillType (fillType); }
    void setOpacity (float newOpacity) override                                  { stack->fillType.setOpacity (newOpacity); }
    void setInterpolationQuality (Graphics::ResamplingQuality quality) override  { stack->interpolationQuality = quality; }
    void fillRect (const Rectangle<int>& r, bool replace) override               { stack->fillRect (r, replace); }
    void fillRect (const Rectangle<float>& r) override                           { stack->fillRect (r); }
    void fillRectList (const RectangleList<float>& list) override                { stack->fillRectList (list); }
    void fillPath (const Path& path, const AffineTransform& t) override          { stack->fillPath (path, t); }
    void drawImage (const Image& im, const AffineTransform& t) override          { stack->drawImage (im, t); }
    void drawGlyph (int glyphNumber, const AffineTransform& t) override          { stack->drawGlyph (glyphNumber, t); }
    void drawLine (const Line<float>& line) override                             { stack->drawLine (line); }
    void setFont (const Font& newFont) override                                  { stack->font = newFont; }
    const Font& getFont() override                                               { return stack->font; }

protected:
    StackBasedLowLevelGraphicsContext (SavedStateType* initialState) : stack (initialState) {}
    StackBasedLowLevelGraphicsContext() = default;

    RenderingHelpers::SavedStateStack<SavedStateType> stack;
};

}

JUCE_END_IGNORE_WARNINGS_MSVC

} // namespace juce
