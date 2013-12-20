/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

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

namespace
{
    template <typename Type>
    Rectangle<Type> coordsToRectangle (Type x, Type y, Type w, Type h)
    {
       #if JUCE_DEBUG
        const int maxVal = 0x3fffffff;

        jassert ((int) x >= -maxVal && (int) x <= maxVal
              && (int) y >= -maxVal && (int) y <= maxVal
              && (int) w >= -maxVal && (int) w <= maxVal
              && (int) h >= -maxVal && (int) h <= maxVal);
       #endif

        return Rectangle<Type> (x, y, w, h);
    }
}

//==============================================================================
LowLevelGraphicsContext::LowLevelGraphicsContext() {}
LowLevelGraphicsContext::~LowLevelGraphicsContext() {}

//==============================================================================
Graphics::Graphics (const Image& imageToDrawOnto)
    : context (*imageToDrawOnto.createLowLevelContext()),
      contextToDelete (&context),
      saveStatePending (false)
{
    jassert (imageToDrawOnto.isValid()); // Can't draw into a null image!
}

Graphics::Graphics (LowLevelGraphicsContext& internalContext) noexcept
    : context (internalContext),
      saveStatePending (false)
{
}

Graphics::~Graphics()
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

bool Graphics::reduceClipRegion (const Rectangle<int>& area)
{
    saveStateIfPending();
    return context.clipToRectangle (area);
}

bool Graphics::reduceClipRegion (const int x, const int y, const int w, const int h)
{
    return reduceClipRegion (Rectangle<int> (x, y, w, h));
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

void Graphics::excludeClipRegion (const Rectangle<int>& rectangleToExclude)
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
    setOrigin (Point<int> (x, y));
}

void Graphics::addTransform (const AffineTransform& transform)
{
    saveStateIfPending();
    context.addTransform (transform);
}

bool Graphics::clipRegionIntersects (const Rectangle<int>& area) const
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

void Graphics::setOpacity (const float newOpacity)
{
    saveStateIfPending();
    context.setOpacity (newOpacity);
}

void Graphics::setGradientFill (const ColourGradient& gradient)
{
    setFillType (gradient);
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
    if (text.isNotEmpty())
    {
        // Don't pass any vertical placement flags to this method - they'll be ignored.
        jassert (justification.getOnlyVerticalFlags() == 0);

        const int flags = justification.getOnlyHorizontalFlags();

        if (flags == Justification::right)
        {
            if (startX < context.getClipBounds().getX())
                return;
        }
        else if (flags == Justification::left)
            if (startX > context.getClipBounds().getRight())
                return;

        GlyphArrangement arr;
        arr.addLineOfText (context.getFont(), text, (float) startX, (float) baselineY);

        if (flags != Justification::left)
        {
            float w = arr.getBoundingBox (0, -1, true).getWidth();

            if ((flags & (Justification::horizontallyCentred | Justification::horizontallyJustified)) != 0)
                w /= 2.0f;

            arr.draw (*this, AffineTransform::translation (-w, 0));
        }
        else
        {
            arr.draw (*this);
        }
    }
}

void Graphics::drawMultiLineText (const String& text, const int startX,
                                  const int baselineY, const int maximumLineWidth) const
{
    if (text.isNotEmpty()
         && startX < context.getClipBounds().getRight())
    {
        GlyphArrangement arr;
        arr.addJustifiedText (context.getFont(), text,
                              (float) startX, (float) baselineY, (float) maximumLineWidth,
                              Justification::left);
        arr.draw (*this);
    }
}

void Graphics::drawText (const String& text, const Rectangle<int>& area,
                         Justification justificationType,
                         const bool useEllipsesIfTooBig) const
{
    if (text.isNotEmpty() && context.clipRegionIntersects (area))
    {
        GlyphArrangement arr;
        arr.addCurtailedLineOfText (context.getFont(), text,
                                    0.0f, 0.0f, (float) area.getWidth(),
                                    useEllipsesIfTooBig);

        arr.justifyGlyphs (0, arr.getNumGlyphs(),
                           (float) area.getX(), (float) area.getY(),
                           (float) area.getWidth(), (float) area.getHeight(),
                           justificationType);
        arr.draw (*this);
    }
}

void Graphics::drawText (const String& text, const int x, const int y, const int width, const int height,
                         Justification justificationType,
                         const bool useEllipsesIfTooBig) const
{
    drawText (text, Rectangle<int> (x, y, width, height), justificationType, useEllipsesIfTooBig);
}

void Graphics::drawFittedText (const String& text, const Rectangle<int>& area,
                               Justification justification,
                               const int maximumNumberOfLines,
                               const float minimumHorizontalScale) const
{
    if (text.isNotEmpty() && (! area.isEmpty()) && context.clipRegionIntersects (area))
    {
        GlyphArrangement arr;
        arr.addFittedText (context.getFont(), text,
                           (float) area.getX(), (float) area.getY(),
                           (float) area.getWidth(), (float) area.getHeight(),
                           justification,
                           maximumNumberOfLines,
                           minimumHorizontalScale);

        arr.draw (*this);
    }
}

void Graphics::drawFittedText (const String& text, const int x, const int y, const int width, const int height,
                               Justification justification,
                               const int maximumNumberOfLines,
                               const float minimumHorizontalScale) const
{
    drawFittedText (text, coordsToRectangle (x, y, width, height),
                    justification, maximumNumberOfLines, minimumHorizontalScale);
}

//==============================================================================
void Graphics::fillRect (const Rectangle<int>& r) const
{
    context.fillRect (r, false);
}

void Graphics::fillRect (const Rectangle<float>& r) const
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
    for (const Rectangle<int>* r = rects.begin(), * const e = rects.end(); r != e; ++r)
        context.fillRect (*r, false);
}

void Graphics::setPixel (int x, int y) const
{
    context.fillRect (Rectangle<int> (x, y, 1, 1), false);
}

void Graphics::fillAll() const
{
    fillRect (context.getClipBounds());
}

void Graphics::fillAll (Colour colourToUse) const
{
    if (! colourToUse.isTransparent())
    {
        const Rectangle<int> clip (context.getClipBounds());

        context.saveState();
        context.setFill (colourToUse);
        context.fillRect (clip, false);
        context.restoreState();
    }
}


//==============================================================================
void Graphics::fillPath (const Path& path, const AffineTransform& transform) const
{
    if ((! context.isClipEmpty()) && ! path.isEmpty())
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

void Graphics::drawRect (const Rectangle<int>& r, int lineThickness) const
{
    drawRect (r.toFloat(), (float) lineThickness);
}

void Graphics::drawRect (Rectangle<float> r, const float lineThickness) const
{
    RectangleList<float> rects;
    rects.addWithoutMerging (r.removeFromTop    (lineThickness));
    rects.addWithoutMerging (r.removeFromBottom (lineThickness));
    rects.addWithoutMerging (r.removeFromLeft   (lineThickness));
    rects.addWithoutMerging (r.removeFromRight  (lineThickness));
    context.fillRectList (rects);
}

//==============================================================================
void Graphics::fillEllipse (const Rectangle<float>& area) const
{
    fillEllipse (area.getX(), area.getY(), area.getWidth(), area.getHeight());
}

void Graphics::fillEllipse (float x, float y, float width, float height) const
{
    Path p;
    p.addEllipse (x, y, width, height);
    fillPath (p);
}

void Graphics::drawEllipse (float x, float y, float width, float height, float lineThickness) const
{
    Path p;
    p.addEllipse (x, y, width, height);
    strokePath (p, PathStrokeType (lineThickness));
}

void Graphics::drawEllipse (const Rectangle<float>& area, float lineThickness) const
{
    drawEllipse (area.getX(), area.getY(), area.getWidth(), area.getHeight(), lineThickness);
}

void Graphics::fillRoundedRectangle (float x, float y, float width, float height, float cornerSize) const
{
    fillRoundedRectangle (coordsToRectangle (x, y, width, height), cornerSize);
}

void Graphics::fillRoundedRectangle (const Rectangle<float>& r, const float cornerSize) const
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

void Graphics::drawRoundedRectangle (const Rectangle<float>& r, float cornerSize, float lineThickness) const
{
    Path p;
    p.addRoundedRectangle (r, cornerSize);
    strokePath (p, PathStrokeType (lineThickness));
}

void Graphics::drawArrow (const Line<float>& line, float lineThickness, float arrowheadWidth, float arrowheadLength) const
{
    Path p;
    p.addArrow (line, lineThickness, arrowheadWidth, arrowheadLength);
    fillPath (p);
}

void Graphics::fillCheckerBoard (const Rectangle<int>& area,
                                 const int checkWidth, const int checkHeight,
                                 Colour colour1, Colour colour2) const
{
    jassert (checkWidth > 0 && checkHeight > 0); // can't be zero or less!

    if (checkWidth > 0 && checkHeight > 0)
    {
        context.saveState();

        if (colour1 == colour2)
        {
            context.setFill (colour1);
            context.fillRect (area, false);
        }
        else
        {
            const Rectangle<int> clipped (context.getClipBounds().getIntersection (area));

            if (! clipped.isEmpty())
            {
                context.clipToRectangle (clipped);

                const int checkNumX = (clipped.getX() - area.getX()) / checkWidth;
                const int checkNumY = (clipped.getY() - area.getY()) / checkHeight;
                const int startX = area.getX() + checkNumX * checkWidth;
                const int startY = area.getY() + checkNumY * checkHeight;
                const int right  = clipped.getRight();
                const int bottom = clipped.getBottom();

                for (int i = 0; i < 2; ++i)
                {
                    context.setFill (i == ((checkNumX ^ checkNumY) & 1) ? colour1 : colour2);

                    int cy = i;
                    for (int y = startY; y < bottom; y += checkHeight)
                        for (int x = startX + (cy++ & 1) * checkWidth; x < right; x += checkWidth * 2)
                            context.fillRect (Rectangle<int> (x, y, checkWidth, checkHeight), false);
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

void Graphics::drawLine (const Line<float>& line) const
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

void Graphics::drawLine (const Line<float>& line, const float lineThickness) const
{
    Path p;
    p.addLineSegment (line, lineThickness);
    fillPath (p);
}

void Graphics::drawDashedLine (const Line<float>& line, const float* const dashLengths,
                               const int numDashLengths, const float lineThickness, int n) const
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

void Graphics::drawImageWithin (const Image& imageToDraw,
                                int dx, int dy, int dw, int dh,
                                RectanglePlacement placementWithinTarget,
                                const bool fillAlphaChannelWithCurrentBrush) const
{
    if (imageToDraw.isValid())
        drawImageTransformed (imageToDraw,
                              placementWithinTarget.getTransformToFit (imageToDraw.getBounds().toFloat(),
                                                                       coordsToRectangle (dx, dy, dw, dh).toFloat()),
                              fillAlphaChannelWithCurrentBrush);
}

void Graphics::drawImage (const Image& imageToDraw,
                          int dx, int dy, int dw, int dh,
                          int sx, int sy, int sw, int sh,
                          const bool fillAlphaChannelWithCurrentBrush) const
{
    if (imageToDraw.isValid() && context.clipRegionIntersects (coordsToRectangle (dx, dy, dw, dh)))
        drawImageTransformed (imageToDraw.getClippedImage (coordsToRectangle (sx, sy, sw, sh)),
                              AffineTransform::scale (dw / (float) sw, dh / (float) sh)
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
