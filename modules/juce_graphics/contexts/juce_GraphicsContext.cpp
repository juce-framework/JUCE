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

struct GraphicsFontHelpers
{
    static auto compareFont (const Font& a, const Font& b) { return Font::compare (a, b); }
};

static auto operator< (const Font& a, const Font& b)
{
    return GraphicsFontHelpers::compareFont (a, b);
}

template <typename T>
static auto operator< (const Rectangle<T>& a, const Rectangle<T>& b)
{
    const auto tie = [] (auto& t) { return std::make_tuple (t.getX(), t.getY(), t.getWidth(), t.getHeight()); };
    return tie (a) < tie (b);
}

static auto operator< (const Justification& a, const Justification& b)
{
    return a.getFlags() < b.getFlags();
}

//==============================================================================
namespace
{
    struct ConfiguredArrangement
    {
        void draw (const Graphics& g) const { arrangement.draw (g, transform); }

        GlyphArrangement arrangement;
        AffineTransform transform;
    };

    template <typename ArrangementArgs>
    class GlyphArrangementCache final : public DeletedAtShutdown
    {
    public:
        GlyphArrangementCache() = default;

        ~GlyphArrangementCache() override
        {
            clearSingletonInstance();
        }

        template <typename ConfigureArrangement>
        void draw (const Graphics& g, ArrangementArgs&& args, ConfigureArrangement&& configureArrangement)
        {
            const ScopedTryLock stl (lock);

            if (! stl.isLocked())
            {
                configureArrangement (args).draw (g);
                return;
            }

            const auto cached = [&]
            {
                const auto iter = cache.find (args);

                if (iter != cache.end())
                {
                    if (iter->second.cachePosition != cacheOrder.begin())
                        cacheOrder.splice (cacheOrder.begin(), cacheOrder, iter->second.cachePosition);

                    return iter;
                }

                auto result = cache.emplace (std::move (args), CachedGlyphArrangement { configureArrangement (args), {} }).first;
                cacheOrder.push_front (result);
                return result;
            }();

            cached->second.cachePosition = cacheOrder.begin();
            cached->second.configured.draw (g);

            while (cache.size() > cacheSize)
            {
                cache.erase (cacheOrder.back());
                cacheOrder.pop_back();
            }
        }

        JUCE_DECLARE_SINGLETON (GlyphArrangementCache<ArrangementArgs>, false)

    private:
        struct CachedGlyphArrangement
        {
            using CachePtr = typename std::map<ArrangementArgs, CachedGlyphArrangement>::const_iterator;
            ConfiguredArrangement configured;
            typename std::list<CachePtr>::const_iterator cachePosition;
        };

        static constexpr size_t cacheSize = 128;
        std::map<ArrangementArgs, CachedGlyphArrangement> cache;
        std::list<typename CachedGlyphArrangement::CachePtr> cacheOrder;
        CriticalSection lock;
    };

    template <typename ArrangementArgs>
    juce::SingletonHolder<GlyphArrangementCache<ArrangementArgs>, juce::CriticalSection, false> GlyphArrangementCache<ArrangementArgs>::singletonHolder;

    //==============================================================================
    template <typename Type>
    Rectangle<Type> coordsToRectangle (Type x, Type y, Type w, Type h) noexcept
    {
       #if JUCE_DEBUG
        const int maxVal = 0x3fffffff;

        jassertquiet ((int) x >= -maxVal && (int) x <= maxVal
                   && (int) y >= -maxVal && (int) y <= maxVal
                   && (int) w >= 0 && (int) w <= maxVal
                   && (int) h >= 0 && (int) h <= maxVal);
       #endif

        return { x, y, w, h };
    }
}

//==============================================================================
Graphics::Graphics (const Image& imageToDrawOnto)
    : contextHolder (imageToDrawOnto.createLowLevelContext()),
      context (*contextHolder)
{
    jassert (imageToDrawOnto.isValid()); // Can't draw into a null image!
}

Graphics::Graphics (LowLevelGraphicsContext& internalContext) noexcept
    : context (internalContext)
{
}

//==============================================================================
void Graphics::resetToDefaultState()
{
    saveStateIfPending();
    context.setFill (FillType());
    context.setFont (Font());
    context.setInterpolationQuality (Graphics::mediumResamplingQuality);
}

bool Graphics::isVectorDevice() const
{
    return context.isVectorDevice();
}

bool Graphics::reduceClipRegion (Rectangle<int> area)
{
    saveStateIfPending();
    return context.clipToRectangle (area);
}

bool Graphics::reduceClipRegion (int x, int y, int w, int h)
{
    return reduceClipRegion (coordsToRectangle (x, y, w, h));
}

bool Graphics::reduceClipRegion (const RectangleList<int>& clipRegion)
{
    saveStateIfPending();
    return context.clipToRectangleList (clipRegion);
}

bool Graphics::reduceClipRegion (const Path& path, const AffineTransform& transform)
{
    saveStateIfPending();
    context.clipToPath (path, transform);
    return ! context.isClipEmpty();
}

bool Graphics::reduceClipRegion (const Image& image, const AffineTransform& transform)
{
    saveStateIfPending();
    context.clipToImageAlpha (image, transform);
    return ! context.isClipEmpty();
}

void Graphics::excludeClipRegion (Rectangle<int> rectangleToExclude)
{
    saveStateIfPending();
    context.excludeClipRectangle (rectangleToExclude);
}

bool Graphics::isClipEmpty() const
{
    return context.isClipEmpty();
}

Rectangle<int> Graphics::getClipBounds() const
{
    return context.getClipBounds();
}

void Graphics::saveState()
{
    saveStateIfPending();
    saveStatePending = true;
}

void Graphics::restoreState()
{
    if (saveStatePending)
        saveStatePending = false;
    else
        context.restoreState();
}

void Graphics::saveStateIfPending()
{
    if (saveStatePending)
    {
        saveStatePending = false;
        context.saveState();
    }
}

void Graphics::setOrigin (Point<int> newOrigin)
{
    saveStateIfPending();
    context.setOrigin (newOrigin);
}

void Graphics::setOrigin (int x, int y)
{
    setOrigin ({ x, y });
}

void Graphics::addTransform (const AffineTransform& transform)
{
    saveStateIfPending();
    context.addTransform (transform);
}

bool Graphics::clipRegionIntersects (Rectangle<int> area) const
{
    return context.clipRegionIntersects (area);
}

void Graphics::beginTransparencyLayer (float layerOpacity)
{
    saveStateIfPending();
    context.beginTransparencyLayer (layerOpacity);
}

void Graphics::endTransparencyLayer()
{
    context.endTransparencyLayer();
}

//==============================================================================
void Graphics::setColour (Colour newColour)
{
    saveStateIfPending();
    context.setFill (newColour);
}

void Graphics::setOpacity (float newOpacity)
{
    saveStateIfPending();
    context.setOpacity (newOpacity);
}

void Graphics::setGradientFill (const ColourGradient& gradient)
{
    setFillType (gradient);
}

void Graphics::setGradientFill (ColourGradient&& gradient)
{
    setFillType (std::move (gradient));
}

void Graphics::setTiledImageFill (const Image& imageToUse, const int anchorX, const int anchorY, const float opacity)
{
    saveStateIfPending();
    context.setFill (FillType (imageToUse, AffineTransform::translation ((float) anchorX, (float) anchorY)));
    context.setOpacity (opacity);
}

void Graphics::setFillType (const FillType& newFill)
{
    saveStateIfPending();
    context.setFill (newFill);
}

//==============================================================================
void Graphics::setFont (const Font& newFont)
{
    saveStateIfPending();
    context.setFont (newFont);
}

void Graphics::setFont (const float newFontHeight)
{
    setFont (context.getFont().withHeight (newFontHeight));
}

Font Graphics::getCurrentFont() const
{
    return context.getFont();
}

//==============================================================================
void Graphics::drawSingleLineText (const String& text, const int startX, const int baselineY,
                                   Justification justification) const
{
    if (text.isEmpty())
        return;

    // Don't pass any vertical placement flags to this method - they'll be ignored.
    jassert (justification.getOnlyVerticalFlags() == 0);

    auto flags = justification.getOnlyHorizontalFlags();

    if (flags == Justification::right && startX < context.getClipBounds().getX())
        return;

    if (flags == Justification::left && startX > context.getClipBounds().getRight())
        return;

    struct ArrangementArgs
    {
        auto tie() const noexcept { return std::tie (font, text, startX, baselineY); }
        bool operator< (const ArrangementArgs& other) const { return tie() < other.tie(); }

        const Font font;
        const String text;
        const int startX, baselineY, flags;
    };

    auto configureArrangement = [] (const ArrangementArgs& args)
    {
        AffineTransform transform;
        GlyphArrangement arrangement;
        arrangement.addLineOfText (args.font, args.text, (float) args.startX, (float) args.baselineY);

        if (args.flags != Justification::left)
        {
            auto w = arrangement.getBoundingBox (0, -1, true).getWidth();

            if ((args.flags & (Justification::horizontallyCentred | Justification::horizontallyJustified)) != 0)
                w /= 2.0f;

            transform = AffineTransform::translation (-w, 0);
        }

        return ConfiguredArrangement { std::move (arrangement), std::move (transform) };
    };

    GlyphArrangementCache<ArrangementArgs>::getInstance()->draw (*this,
                                                                 { context.getFont(), text, startX, baselineY, flags },
                                                                 std::move (configureArrangement));
}

void Graphics::drawMultiLineText (const String& text, const int startX,
                                  const int baselineY, const int maximumLineWidth,
                                  Justification justification, const float leading) const
{
    if (text.isEmpty() || startX >= context.getClipBounds().getRight())
        return;

    struct ArrangementArgs
    {
        auto tie() const noexcept { return std::tie (font, text, startX, baselineY, maximumLineWidth, justification, leading); }
        bool operator< (const ArrangementArgs& other) const { return tie() < other.tie(); }

        const Font font;
        const String text;
        const int startX, baselineY, maximumLineWidth;
        const Justification justification;
        const float leading;
    };

    auto configureArrangement = [] (const ArrangementArgs& args)
    {
        GlyphArrangement arrangement;
        arrangement.addJustifiedText (args.font, args.text,
                                      (float) args.startX, (float) args.baselineY, (float) args.maximumLineWidth,
                                      args.justification, args.leading);
        return ConfiguredArrangement { std::move (arrangement), {} };
    };

    GlyphArrangementCache<ArrangementArgs>::getInstance()->draw (*this,
                                                                 { context.getFont(), text, startX, baselineY, maximumLineWidth, justification, leading },
                                                                 std::move (configureArrangement));
}

void Graphics::drawText (const String& text, Rectangle<float> area,
                         Justification justificationType, bool useEllipsesIfTooBig) const
{
    if (text.isEmpty() || ! context.clipRegionIntersects (area.getSmallestIntegerContainer()))
        return;

    struct ArrangementArgs
    {
        auto tie() const noexcept { return std::tie (font, text, area, justificationType, useEllipsesIfTooBig); }
        bool operator< (const ArrangementArgs& other) const { return tie() < other.tie(); }

        const Font font;
        const String text;
        const Rectangle<float> area;
        const Justification justificationType;
        const bool useEllipsesIfTooBig;
    };

    auto configureArrangement = [] (const ArrangementArgs& args)
    {
        GlyphArrangement arrangement;
        arrangement.addCurtailedLineOfText (args.font, args.text, 0.0f, 0.0f,
                                            args.area.getWidth(), args.useEllipsesIfTooBig);

        arrangement.justifyGlyphs (0, arrangement.getNumGlyphs(),
                                   args.area.getX(), args.area.getY(), args.area.getWidth(), args.area.getHeight(),
                                   args.justificationType);
        return ConfiguredArrangement { std::move (arrangement), {} };
    };

    GlyphArrangementCache<ArrangementArgs>::getInstance()->draw (*this,
                                                                 { context.getFont(), text, area, justificationType, useEllipsesIfTooBig },
                                                                 std::move (configureArrangement));
}

void Graphics::drawText (const String& text, Rectangle<int> area,
                         Justification justificationType, bool useEllipsesIfTooBig) const
{
    drawText (text, area.toFloat(), justificationType, useEllipsesIfTooBig);
}

void Graphics::drawText (const String& text, int x, int y, int width, int height,
                         Justification justificationType, const bool useEllipsesIfTooBig) const
{
    drawText (text, coordsToRectangle (x, y, width, height), justificationType, useEllipsesIfTooBig);
}

void Graphics::drawFittedText (const String& text, Rectangle<int> area,
                               Justification justification,
                               const int maximumNumberOfLines,
                               const float minimumHorizontalScale) const
{
    if (text.isEmpty() || area.isEmpty() || ! context.clipRegionIntersects (area))
        return;

    struct ArrangementArgs
    {
        auto tie() const noexcept { return std::tie (font, text, area, justification, maximumNumberOfLines, minimumHorizontalScale); }
        bool operator< (const ArrangementArgs& other) const noexcept { return tie() < other.tie(); }

        const Font font;
        const String text;
        const Rectangle<float> area;
        const Justification justification;
        const int maximumNumberOfLines;
        const float minimumHorizontalScale;
    };

    auto configureArrangement = [] (const ArrangementArgs& args)
    {
        GlyphArrangement arrangement;
        arrangement.addFittedText (args.font, args.text,
                                   args.area.getX(), args.area.getY(),
                                   args.area.getWidth(), args.area.getHeight(),
                                   args.justification,
                                   args.maximumNumberOfLines,
                                   args.minimumHorizontalScale);
        return ConfiguredArrangement { std::move (arrangement), {} };
    };

    GlyphArrangementCache<ArrangementArgs>::getInstance()->draw (*this,
                                                                 { context.getFont(), text, area.toFloat(), justification, maximumNumberOfLines, minimumHorizontalScale },
                                                                 std::move (configureArrangement));
}

void Graphics::drawFittedText (const String& text, int x, int y, int width, int height,
                               Justification justification,
                               const int maximumNumberOfLines,
                               const float minimumHorizontalScale) const
{
    drawFittedText (text, coordsToRectangle (x, y, width, height),
                    justification, maximumNumberOfLines, minimumHorizontalScale);
}

//==============================================================================
void Graphics::fillRect (Rectangle<int> r) const
{
    context.fillRect (r, false);
}

void Graphics::fillRect (Rectangle<float> r) const
{
    context.fillRect (r);
}

void Graphics::fillRect (int x, int y, int width, int height) const
{
    context.fillRect (coordsToRectangle (x, y, width, height), false);
}

void Graphics::fillRect (float x, float y, float width, float height) const
{
    fillRect (coordsToRectangle (x, y, width, height));
}

void Graphics::fillRectList (const RectangleList<float>& rectangles) const
{
    context.fillRectList (rectangles);
}

void Graphics::fillRectList (const RectangleList<int>& rects) const
{
    for (auto& r : rects)
        context.fillRect (r, false);
}

void Graphics::fillAll() const
{
    context.fillAll();
}

void Graphics::fillAll (Colour colourToUse) const
{
    if (! colourToUse.isTransparent())
    {
        context.saveState();
        context.setFill (colourToUse);
        context.fillAll();
        context.restoreState();
    }
}


//==============================================================================
void Graphics::fillPath (const Path& path) const
{
    if (! (context.isClipEmpty() || path.isEmpty()))
        context.fillPath (path, AffineTransform());
}

void Graphics::fillPath (const Path& path, const AffineTransform& transform) const
{
    if (! (context.isClipEmpty() || path.isEmpty()))
        context.fillPath (path, transform);
}

void Graphics::strokePath (const Path& path,
                           const PathStrokeType& strokeType,
                           const AffineTransform& transform) const
{
    Path stroke;
    strokeType.createStrokedPath (stroke, path, transform, context.getPhysicalPixelScaleFactor());
    fillPath (stroke);
}

//==============================================================================
void Graphics::drawRect (float x, float y, float width, float height, float lineThickness) const
{
    drawRect (coordsToRectangle (x, y, width, height), lineThickness);
}

void Graphics::drawRect (int x, int y, int width, int height, int lineThickness) const
{
    drawRect (coordsToRectangle (x, y, width, height), lineThickness);
}

void Graphics::drawRect (Rectangle<int> r, int lineThickness) const
{
    drawRect (r.toFloat(), (float) lineThickness);
}

void Graphics::drawRect (Rectangle<float> r, const float lineThickness) const
{
    jassert (r.getWidth() >= 0.0f && r.getHeight() >= 0.0f);

    RectangleList<float> rects;
    rects.addWithoutMerging (r.removeFromTop    (lineThickness));
    rects.addWithoutMerging (r.removeFromBottom (lineThickness));
    rects.addWithoutMerging (r.removeFromLeft   (lineThickness));
    rects.addWithoutMerging (r.removeFromRight  (lineThickness));
    context.fillRectList (rects);
}

//==============================================================================
void Graphics::fillEllipse (Rectangle<float> area) const
{
    Path p;
    p.addEllipse (area);
    fillPath (p);
}

void Graphics::fillEllipse (float x, float y, float w, float h) const
{
    fillEllipse (coordsToRectangle (x, y, w, h));
}

void Graphics::drawEllipse (float x, float y, float width, float height, float lineThickness) const
{
    drawEllipse (coordsToRectangle (x, y, width, height), lineThickness);
}

void Graphics::drawEllipse (Rectangle<float> area, float lineThickness) const
{
    Path p;

    if (area.getWidth() == area.getHeight())
    {
        // For a circle, we can avoid having to generate a stroke
        p.addEllipse (area.expanded (lineThickness * 0.5f));
        p.addEllipse (area.reduced  (lineThickness * 0.5f));
        p.setUsingNonZeroWinding (false);
        fillPath (p);
    }
    else
    {
        p.addEllipse (area);
        strokePath (p, PathStrokeType (lineThickness));
    }
}

void Graphics::fillRoundedRectangle (float x, float y, float width, float height, float cornerSize) const
{
    fillRoundedRectangle (coordsToRectangle (x, y, width, height), cornerSize);
}

void Graphics::fillRoundedRectangle (Rectangle<float> r, const float cornerSize) const
{
    Path p;
    p.addRoundedRectangle (r, cornerSize);
    fillPath (p);
}

void Graphics::drawRoundedRectangle (float x, float y, float width, float height,
                                     float cornerSize, float lineThickness) const
{
    drawRoundedRectangle (coordsToRectangle (x, y, width, height), cornerSize, lineThickness);
}

void Graphics::drawRoundedRectangle (Rectangle<float> r, float cornerSize, float lineThickness) const
{
    Path p;
    p.addRoundedRectangle (r, cornerSize);
    strokePath (p, PathStrokeType (lineThickness));
}

void Graphics::drawArrow (Line<float> line, float lineThickness, float arrowheadWidth, float arrowheadLength) const
{
    Path p;
    p.addArrow (line, lineThickness, arrowheadWidth, arrowheadLength);
    fillPath (p);
}

void Graphics::fillCheckerBoard (Rectangle<float> area, float checkWidth, float checkHeight,
                                 Colour colour1, Colour colour2) const
{
    jassert (checkWidth > 0 && checkHeight > 0); // can't be zero or less!

    if (checkWidth > 0 && checkHeight > 0)
    {
        context.saveState();

        if (colour1 == colour2)
        {
            context.setFill (colour1);
            context.fillRect (area);
        }
        else
        {
            auto clipped = context.getClipBounds().getIntersection (area.getSmallestIntegerContainer());

            if (! clipped.isEmpty())
            {
                const int checkNumX = (int) (((float) clipped.getX() - area.getX()) / checkWidth);
                const int checkNumY = (int) (((float) clipped.getY() - area.getY()) / checkHeight);
                const float startX = area.getX() + (float) checkNumX * checkWidth;
                const float startY = area.getY() + (float) checkNumY * checkHeight;
                const float right  = (float) clipped.getRight();
                const float bottom = (float) clipped.getBottom();

                for (int i = 0; i < 2; ++i)
                {
                    int cy = i;
                    RectangleList<float> checks;

                    for (float y = startY; y < bottom; y += checkHeight)
                        for (float x = startX + (cy++ & 1) * checkWidth; x < right; x += checkWidth * 2.0f)
                            checks.addWithoutMerging ({ x, y, checkWidth, checkHeight });

                    checks.clipTo (area);
                    context.setFill (i == ((checkNumX ^ checkNumY) & 1) ? colour1 : colour2);
                    context.fillRectList (checks);
                }
            }
        }

        context.restoreState();
    }
}

//==============================================================================
void Graphics::drawVerticalLine (const int x, float top, float bottom) const
{
    if (top < bottom)
        context.fillRect (Rectangle<float> ((float) x, top, 1.0f, bottom - top));
}

void Graphics::drawHorizontalLine (const int y, float left, float right) const
{
    if (left < right)
        context.fillRect (Rectangle<float> (left, (float) y, right - left, 1.0f));
}

void Graphics::drawLine (Line<float> line) const
{
    context.drawLine (line);
}

void Graphics::drawLine (float x1, float y1, float x2, float y2) const
{
    context.drawLine (Line<float> (x1, y1, x2, y2));
}

void Graphics::drawLine (float x1, float y1, float x2, float y2, float lineThickness) const
{
    drawLine (Line<float> (x1, y1, x2, y2), lineThickness);
}

void Graphics::drawLine (Line<float> line, const float lineThickness) const
{
    Path p;
    p.addLineSegment (line, lineThickness);
    fillPath (p);
}

void Graphics::drawDashedLine (Line<float> line, const float* dashLengths,
                               int numDashLengths, float lineThickness, int n) const
{
    jassert (n >= 0 && n < numDashLengths); // your start index must be valid!

    const Point<double> delta ((line.getEnd() - line.getStart()).toDouble());
    const double totalLen = delta.getDistanceFromOrigin();

    if (totalLen >= 0.1)
    {
        const double onePixAlpha = 1.0 / totalLen;

        for (double alpha = 0.0; alpha < 1.0;)
        {
            jassert (dashLengths[n] > 0); // can't have zero-length dashes!

            const double lastAlpha = alpha;
            alpha += dashLengths [n] * onePixAlpha;
            n = (n + 1) % numDashLengths;

            if ((n & 1) != 0)
            {
                const Line<float> segment (line.getStart() + (delta * lastAlpha).toFloat(),
                                           line.getStart() + (delta * jmin (1.0, alpha)).toFloat());

                if (lineThickness != 1.0f)
                    drawLine (segment, lineThickness);
                else
                    context.drawLine (segment);
            }
        }
    }
}

//==============================================================================
void Graphics::setImageResamplingQuality (const Graphics::ResamplingQuality newQuality)
{
    saveStateIfPending();
    context.setInterpolationQuality (newQuality);
}

//==============================================================================
void Graphics::drawImageAt (const Image& imageToDraw, int x, int y, bool fillAlphaChannel) const
{
    drawImageTransformed (imageToDraw,
                          AffineTransform::translation ((float) x, (float) y),
                          fillAlphaChannel);
}

void Graphics::drawImage (const Image& imageToDraw, Rectangle<float> targetArea,
                          RectanglePlacement placementWithinTarget, bool fillAlphaChannelWithCurrentBrush) const
{
    if (imageToDraw.isValid())
        drawImageTransformed (imageToDraw,
                              placementWithinTarget.getTransformToFit (imageToDraw.getBounds().toFloat(), targetArea),
                              fillAlphaChannelWithCurrentBrush);
}

void Graphics::drawImageWithin (const Image& imageToDraw, int dx, int dy, int dw, int dh,
                                RectanglePlacement placementWithinTarget, bool fillAlphaChannelWithCurrentBrush) const
{
    drawImage (imageToDraw, coordsToRectangle (dx, dy, dw, dh).toFloat(),
               placementWithinTarget, fillAlphaChannelWithCurrentBrush);
}

void Graphics::drawImage (const Image& imageToDraw,
                          int dx, int dy, int dw, int dh,
                          int sx, int sy, int sw, int sh,
                          const bool fillAlphaChannelWithCurrentBrush) const
{
    if (imageToDraw.isValid() && context.clipRegionIntersects (coordsToRectangle (dx, dy, dw, dh)))
        drawImageTransformed (imageToDraw.getClippedImage (coordsToRectangle (sx, sy, sw, sh)),
                              AffineTransform::scale ((float) dw / (float) sw, (float) dh / (float) sh)
                                              .translated ((float) dx, (float) dy),
                              fillAlphaChannelWithCurrentBrush);
}

void Graphics::drawImageTransformed (const Image& imageToDraw,
                                     const AffineTransform& transform,
                                     const bool fillAlphaChannelWithCurrentBrush) const
{
    if (imageToDraw.isValid() && ! context.isClipEmpty())
    {
        if (fillAlphaChannelWithCurrentBrush)
        {
            context.saveState();
            context.clipToImageAlpha (imageToDraw, transform);
            fillAll();
            context.restoreState();
        }
        else
        {
            context.drawImage (imageToDraw, transform);
        }
    }
}

//==============================================================================
Graphics::ScopedSaveState::ScopedSaveState (Graphics& g)  : context (g)
{
    context.saveState();
}

Graphics::ScopedSaveState::~ScopedSaveState()
{
    context.restoreState();
}

} // namespace juce
