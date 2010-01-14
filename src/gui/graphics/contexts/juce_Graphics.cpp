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

#include "juce_Graphics.h"
#include "../fonts/juce_GlyphArrangement.h"
#include "../geometry/juce_PathStrokeType.h"
#include "juce_LowLevelGraphicsContext.h"

static const Graphics::ResamplingQuality defaultQuality = Graphics::mediumResamplingQuality;

//==============================================================================
template <typename Type>
static bool areCoordsSensibleNumbers (Type x, Type y, Type w, Type h)
{
    const int maxVal = 0x3fffffff;

    return (int) x >= -maxVal && (int) x <= maxVal
        && (int) y >= -maxVal && (int) y <= maxVal
        && (int) w >= -maxVal && (int) w <= maxVal
        && (int) h >= -maxVal && (int) h <= maxVal;
}

//==============================================================================
LowLevelGraphicsContext::LowLevelGraphicsContext()
{
}

LowLevelGraphicsContext::~LowLevelGraphicsContext()
{
}

//==============================================================================
Graphics::Graphics (Image& imageToDrawOnto) throw()
    : context (imageToDrawOnto.createLowLevelContext()),
      contextToDelete (context),
      saveStatePending (false)
{
    resetToDefaultState();
}

Graphics::Graphics (LowLevelGraphicsContext* const internalContext) throw()
    : context (internalContext),
      saveStatePending (false)
{
    resetToDefaultState();
}

Graphics::~Graphics() throw()
{
}

//==============================================================================
void Graphics::resetToDefaultState() throw()
{
    saveStateIfPending();
    context->setFill (FillType());
    context->setFont (Font());
    context->setInterpolationQuality (defaultQuality);
}

bool Graphics::isVectorDevice() const throw()
{
    return context->isVectorDevice();
}

bool Graphics::reduceClipRegion (const int x, const int y,
                                 const int w, const int h) throw()
{
    saveStateIfPending();
    return context->clipToRectangle (Rectangle (x, y, w, h));
}

bool Graphics::reduceClipRegion (const RectangleList& clipRegion) throw()
{
    saveStateIfPending();
    return context->clipToRectangleList (clipRegion);
}

bool Graphics::reduceClipRegion (const Path& path, const AffineTransform& transform) throw()
{
    saveStateIfPending();
    context->clipToPath (path, transform);
    return ! context->isClipEmpty();
}

bool Graphics::reduceClipRegion (const Image& image, const Rectangle& sourceClipRegion, const AffineTransform& transform) throw()
{
    saveStateIfPending();
    context->clipToImageAlpha (image, sourceClipRegion, transform);
    return ! context->isClipEmpty();
}

void Graphics::excludeClipRegion (const int x, const int y,
                                  const int w, const int h) throw()
{
    saveStateIfPending();
    context->excludeClipRectangle (Rectangle (x, y, w, h));
}

bool Graphics::isClipEmpty() const throw()
{
    return context->isClipEmpty();
}

const Rectangle Graphics::getClipBounds() const throw()
{
    return context->getClipBounds();
}

void Graphics::saveState() throw()
{
    saveStateIfPending();
    saveStatePending = true;
}

void Graphics::restoreState() throw()
{
    if (saveStatePending)
        saveStatePending = false;
    else
        context->restoreState();
}

void Graphics::saveStateIfPending() throw()
{
    if (saveStatePending)
    {
        saveStatePending = false;
        context->saveState();
    }
}

void Graphics::setOrigin (const int newOriginX,
                          const int newOriginY) throw()
{
    saveStateIfPending();
    context->setOrigin (newOriginX, newOriginY);
}

bool Graphics::clipRegionIntersects (const int x, const int y,
                                     const int w, const int h) const throw()
{
    return context->clipRegionIntersects (Rectangle (x, y, w, h));
}

//==============================================================================
void Graphics::setColour (const Colour& newColour) throw()
{
    saveStateIfPending();
    context->setFill (FillType (newColour));
}

void Graphics::setOpacity (const float newOpacity) throw()
{
    saveStateIfPending();
    context->setOpacity (newOpacity);
}

void Graphics::setGradientFill (const ColourGradient& gradient) throw()
{
    saveStateIfPending();
    context->setFill (FillType (gradient));
}

void Graphics::setTiledImageFill (const Image& imageToUse,
                                  const int anchorX,
                                  const int anchorY,
                                  const float opacity) throw()
{
    saveStateIfPending();
    context->setFill (FillType (imageToUse, AffineTransform::translation ((float) anchorX, (float) anchorY)));
    context->setOpacity (opacity);
}

void Graphics::setFillType (const FillType& newFill) throw()
{
    saveStateIfPending();
    context->setFill (newFill);
}

//==============================================================================
void Graphics::setFont (const Font& newFont) throw()
{
    saveStateIfPending();
    context->setFont (newFont);
}

void Graphics::setFont (const float newFontHeight,
                        const int newFontStyleFlags) throw()
{
    saveStateIfPending();
    Font f (context->getFont());
    f.setSizeAndStyle (newFontHeight, newFontStyleFlags, 1.0f, 0);
    context->setFont (f);
}

//==============================================================================
void Graphics::drawSingleLineText (const String& text,
                                   const int startX,
                                   const int baselineY) const throw()
{
    if (text.isNotEmpty()
         && startX < context->getClipBounds().getRight())
    {
        GlyphArrangement arr;
        arr.addLineOfText (context->getFont(), text, (float) startX, (float) baselineY);
        arr.draw (*this);
    }
}

void Graphics::drawTextAsPath (const String& text,
                               const AffineTransform& transform) const throw()
{
    if (text.isNotEmpty())
    {
        GlyphArrangement arr;
        arr.addLineOfText (context->getFont(), text, 0.0f, 0.0f);
        arr.draw (*this, transform);
    }
}

void Graphics::drawMultiLineText (const String& text,
                                  const int startX,
                                  const int baselineY,
                                  const int maximumLineWidth) const throw()
{
    if (text.isNotEmpty()
         && startX < context->getClipBounds().getRight())
    {
        GlyphArrangement arr;
        arr.addJustifiedText (context->getFont(), text,
                              (float) startX, (float) baselineY, (float) maximumLineWidth,
                              Justification::left);
        arr.draw (*this);
    }
}

void Graphics::drawText (const String& text,
                         const int x,
                         const int y,
                         const int width,
                         const int height,
                         const Justification& justificationType,
                         const bool useEllipsesIfTooBig) const throw()
{
    if (text.isNotEmpty() && context->clipRegionIntersects (Rectangle (x, y, width, height)))
    {
        GlyphArrangement arr;

        arr.addCurtailedLineOfText (context->getFont(), text,
                                    0.0f, 0.0f, (float)width,
                                    useEllipsesIfTooBig);

        arr.justifyGlyphs (0, arr.getNumGlyphs(),
                           (float) x, (float) y,
                           (float) width, (float) height,
                           justificationType);
        arr.draw (*this);
    }
}

void Graphics::drawFittedText (const String& text,
                               const int x,
                               const int y,
                               const int width,
                               const int height,
                               const Justification& justification,
                               const int maximumNumberOfLines,
                               const float minimumHorizontalScale) const throw()
{
    if (text.isNotEmpty()
         && width > 0 && height > 0
         && context->clipRegionIntersects (Rectangle (x, y, width, height)))
    {
        GlyphArrangement arr;

        arr.addFittedText (context->getFont(), text,
                           (float) x, (float) y,
                           (float) width, (float) height,
                           justification,
                           maximumNumberOfLines,
                           minimumHorizontalScale);

        arr.draw (*this);
    }
}

//==============================================================================
void Graphics::fillRect (int x,
                         int y,
                         int width,
                         int height) const throw()
{
    // passing in a silly number can cause maths problems in rendering!
    jassert (areCoordsSensibleNumbers (x, y, width, height));

    context->fillRect (Rectangle (x, y, width, height), false);
}

void Graphics::fillRect (const Rectangle& r) const throw()
{
    context->fillRect (r, false);
}

void Graphics::fillRect (const float x,
                         const float y,
                         const float width,
                         const float height) const throw()
{
    // passing in a silly number can cause maths problems in rendering!
    jassert (areCoordsSensibleNumbers (x, y, width, height));

    Path p;
    p.addRectangle (x, y, width, height);
    fillPath (p);
}

void Graphics::setPixel (int x, int y) const throw()
{
    context->fillRect (Rectangle (x, y, 1, 1), false);
}

void Graphics::fillAll() const throw()
{
    fillRect (context->getClipBounds());
}

void Graphics::fillAll (const Colour& colourToUse) const throw()
{
    if (! colourToUse.isTransparent())
    {
        const Rectangle clip (context->getClipBounds());

        context->saveState();
        context->setFill (FillType (colourToUse));
        context->fillRect (clip, false);
        context->restoreState();
    }
}


//==============================================================================
void Graphics::fillPath (const Path& path,
                         const AffineTransform& transform) const throw()
{
    if ((! context->isClipEmpty()) && ! path.isEmpty())
        context->fillPath (path, transform);
}

void Graphics::strokePath (const Path& path,
                           const PathStrokeType& strokeType,
                           const AffineTransform& transform) const throw()
{
    Path stroke;
    strokeType.createStrokedPath (stroke, path, transform);
    fillPath (stroke);
}

//==============================================================================
void Graphics::drawRect (const int x,
                         const int y,
                         const int width,
                         const int height,
                         const int lineThickness) const throw()
{
    // passing in a silly number can cause maths problems in rendering!
    jassert (areCoordsSensibleNumbers (x, y, width, height));

    context->fillRect (Rectangle (x, y, width, lineThickness), false);
    context->fillRect (Rectangle (x, y + lineThickness, lineThickness, height - lineThickness * 2), false);
    context->fillRect (Rectangle (x + width - lineThickness, y + lineThickness, lineThickness, height - lineThickness * 2), false);
    context->fillRect (Rectangle (x, y + height - lineThickness, width, lineThickness), false);
}

void Graphics::drawRect (const float x,
                         const float y,
                         const float width,
                         const float height,
                         const float lineThickness) const throw()
{
    // passing in a silly number can cause maths problems in rendering!
    jassert (areCoordsSensibleNumbers (x, y, width, height));

    Path p;
    p.addRectangle (x, y, width, lineThickness);
    p.addRectangle (x, y + lineThickness, lineThickness, height - lineThickness * 2.0f);
    p.addRectangle (x + width - lineThickness, y + lineThickness, lineThickness, height - lineThickness * 2.0f);
    p.addRectangle (x, y + height - lineThickness, width, lineThickness);
    fillPath (p);
}

void Graphics::drawRect (const Rectangle& r,
                         const int lineThickness) const throw()
{
    drawRect (r.getX(), r.getY(),
              r.getWidth(), r.getHeight(),
              lineThickness);
}

void Graphics::drawBevel (const int x,
                          const int y,
                          const int width,
                          const int height,
                          const int bevelThickness,
                          const Colour& topLeftColour,
                          const Colour& bottomRightColour,
                          const bool useGradient,
                          const bool sharpEdgeOnOutside) const throw()
{
    // passing in a silly number can cause maths problems in rendering!
    jassert (areCoordsSensibleNumbers (x, y, width, height));

    if (clipRegionIntersects (x, y, width, height))
    {
        context->saveState();

        const float oldOpacity = 1.0f;//xxx state->colour.getFloatAlpha();
        const float ramp = oldOpacity / bevelThickness;

        for (int i = bevelThickness; --i >= 0;)
        {
            const float op = useGradient ? ramp * (sharpEdgeOnOutside ? bevelThickness - i : i)
                                         : oldOpacity;

            context->setFill (FillType (topLeftColour.withMultipliedAlpha (op)));
            context->fillRect (Rectangle (x + i, y + i, width - i * 2, 1), false);
            context->setFill (FillType (topLeftColour.withMultipliedAlpha (op * 0.75f)));
            context->fillRect (Rectangle (x + i, y + i + 1, 1, height - i * 2 - 2), false);
            context->setFill (FillType (bottomRightColour.withMultipliedAlpha (op)));
            context->fillRect (Rectangle (x + i, y + height - i - 1, width - i * 2, 1), false);
            context->setFill (FillType (bottomRightColour.withMultipliedAlpha (op  * 0.75f)));
            context->fillRect (Rectangle (x + width - i - 1, y + i + 1, 1, height - i * 2 - 2), false);
        }

        context->restoreState();
    }
}

//==============================================================================
void Graphics::fillEllipse (const float x,
                            const float y,
                            const float width,
                            const float height) const throw()
{
    // passing in a silly number can cause maths problems in rendering!
    jassert (areCoordsSensibleNumbers (x, y, width, height));

    Path p;
    p.addEllipse (x, y, width, height);
    fillPath (p);
}

void Graphics::drawEllipse (const float x,
                            const float y,
                            const float width,
                            const float height,
                            const float lineThickness) const throw()
{
    // passing in a silly number can cause maths problems in rendering!
    jassert (areCoordsSensibleNumbers (x, y, width, height));

    Path p;
    p.addEllipse (x, y, width, height);
    strokePath (p, PathStrokeType (lineThickness));
}

void Graphics::fillRoundedRectangle (const float x,
                                     const float y,
                                     const float width,
                                     const float height,
                                     const float cornerSize) const throw()
{
    // passing in a silly number can cause maths problems in rendering!
    jassert (areCoordsSensibleNumbers (x, y, width, height));

    Path p;
    p.addRoundedRectangle (x, y, width, height, cornerSize);
    fillPath (p);
}

void Graphics::fillRoundedRectangle (const Rectangle& r,
                                     const float cornerSize) const throw()
{
    fillRoundedRectangle ((float) r.getX(),
                          (float) r.getY(),
                          (float) r.getWidth(),
                          (float) r.getHeight(),
                          cornerSize);
}

void Graphics::drawRoundedRectangle (const float x,
                                     const float y,
                                     const float width,
                                     const float height,
                                     const float cornerSize,
                                     const float lineThickness) const throw()
{
    // passing in a silly number can cause maths problems in rendering!
    jassert (areCoordsSensibleNumbers (x, y, width, height));

    Path p;
    p.addRoundedRectangle (x, y, width, height, cornerSize);
    strokePath (p, PathStrokeType (lineThickness));
}

void Graphics::drawRoundedRectangle (const Rectangle& r,
                                     const float cornerSize,
                                     const float lineThickness) const throw()
{
    drawRoundedRectangle ((float) r.getX(),
                          (float) r.getY(),
                          (float) r.getWidth(),
                          (float) r.getHeight(),
                          cornerSize, lineThickness);
}


void Graphics::drawArrow (const float startX,
                          const float startY,
                          const float endX,
                          const float endY,
                          const float lineThickness,
                          const float arrowheadWidth,
                          const float arrowheadLength) const throw()
{
    Path p;
    p.addArrow (startX, startY, endX, endY,
                lineThickness, arrowheadWidth, arrowheadLength);
    fillPath (p);
}

void Graphics::fillCheckerBoard (int x, int y,
                                 int width, int height,
                                 const int checkWidth,
                                 const int checkHeight,
                                 const Colour& colour1,
                                 const Colour& colour2) const throw()
{
    jassert (checkWidth > 0 && checkHeight > 0); // can't be zero or less!

    if (checkWidth > 0 && checkHeight > 0)
    {
        context->saveState();

        if (colour1 == colour2)
        {
            context->setFill (FillType (colour1));
            context->fillRect (Rectangle (x, y, width, height), false);
        }
        else
        {
            const Rectangle clip (context->getClipBounds());

            const int right  = jmin (x + width, clip.getRight());
            const int bottom = jmin (y + height, clip.getBottom());

            int cy = 0;
            while (y < bottom)
            {
                int cx = cy;

                for (int xx = x; xx < right; xx += checkWidth)
                {
                    context->setFill (FillType (((cx++ & 1) == 0) ? colour1 : colour2));
                    context->fillRect (Rectangle (xx, y, jmin (checkWidth, right - xx), jmin (checkHeight, bottom - y)),
                                       false);
                }

                ++cy;
                y += checkHeight;
            }
        }

        context->restoreState();
    }
}

//==============================================================================
void Graphics::drawVerticalLine (const int x, float top, float bottom) const throw()
{
    context->drawVerticalLine (x, top, bottom);
}

void Graphics::drawHorizontalLine (const int y, float left, float right) const throw()
{
    context->drawHorizontalLine (y, left, right);
}

void Graphics::drawLine (float x1, float y1, float x2, float y2) const throw()
{
    context->drawLine (x1, y1, x2, y2);
}

void Graphics::drawLine (const float startX,
                         const float startY,
                         const float endX,
                         const float endY,
                         const float lineThickness) const throw()
{
    Path p;
    p.addLineSegment (startX, startY, endX, endY, lineThickness);
    fillPath (p);
}

void Graphics::drawLine (const Line& line) const throw()
{
    drawLine (line.getStartX(), line.getStartY(), line.getEndX(), line.getEndY());
}

void Graphics::drawLine (const Line& line,
                         const float lineThickness) const throw()
{
    drawLine (line.getStartX(), line.getStartY(), line.getEndX(), line.getEndY(), lineThickness);
}

void Graphics::drawDashedLine (const float startX,
                               const float startY,
                               const float endX,
                               const float endY,
                               const float* const dashLengths,
                               const int numDashLengths,
                               const float lineThickness) const throw()
{
    const double dx = endX - startX;
    const double dy = endY - startY;
    const double totalLen = juce_hypot (dx, dy);

    if (totalLen >= 0.5)
    {
        const double onePixAlpha = 1.0 / totalLen;

        double alpha = 0.0;
        float x = startX;
        float y = startY;
        int n = 0;

        while (alpha < 1.0f)
        {
            alpha = jmin (1.0, alpha + dashLengths[n++] * onePixAlpha);
            n = n % numDashLengths;

            const float oldX = x;
            const float oldY = y;

            x = (float) (startX + dx * alpha);
            y = (float) (startY + dy * alpha);

            if ((n & 1) != 0)
            {
                if (lineThickness != 1.0f)
                    drawLine (oldX, oldY, x, y, lineThickness);
                else
                    drawLine (oldX, oldY, x, y);
            }
        }
    }
}

//==============================================================================
void Graphics::setImageResamplingQuality (const Graphics::ResamplingQuality newQuality) throw()
{
    saveStateIfPending();
    context->setInterpolationQuality (newQuality);
}

//==============================================================================
void Graphics::drawImageAt (const Image* const imageToDraw,
                            const int topLeftX,
                            const int topLeftY,
                            const bool fillAlphaChannelWithCurrentBrush) const throw()
{
    if (imageToDraw != 0)
    {
        const int imageW = imageToDraw->getWidth();
        const int imageH = imageToDraw->getHeight();

        drawImage (imageToDraw,
                   topLeftX, topLeftY, imageW, imageH,
                   0, 0, imageW, imageH,
                   fillAlphaChannelWithCurrentBrush);
    }
}

void Graphics::drawImageWithin (const Image* const imageToDraw,
                                const int destX,
                                const int destY,
                                const int destW,
                                const int destH,
                                const RectanglePlacement& placementWithinTarget,
                                const bool fillAlphaChannelWithCurrentBrush) const throw()
{
    // passing in a silly number can cause maths problems in rendering!
    jassert (areCoordsSensibleNumbers (destX, destY, destW, destH));

    if (imageToDraw != 0)
    {
        const int imageW = imageToDraw->getWidth();
        const int imageH = imageToDraw->getHeight();

        if (imageW > 0 && imageH > 0)
        {
            double newX = 0.0, newY = 0.0;
            double newW = imageW;
            double newH = imageH;

            placementWithinTarget.applyTo (newX, newY, newW, newH,
                                           destX, destY, destW, destH);

            if (newW > 0 && newH > 0)
            {
                drawImage (imageToDraw,
                           roundToInt (newX), roundToInt (newY),
                           roundToInt (newW), roundToInt (newH),
                           0, 0, imageW, imageH,
                           fillAlphaChannelWithCurrentBrush);
            }
        }
    }
}

void Graphics::drawImage (const Image* const imageToDraw,
                          int dx, int dy, int dw, int dh,
                          int sx, int sy, int sw, int sh,
                          const bool fillAlphaChannelWithCurrentBrush) const throw()
{
    // passing in a silly number can cause maths problems in rendering!
    jassert (areCoordsSensibleNumbers (dx, dy, dw, dh));
    jassert (areCoordsSensibleNumbers (sx, sy, sw, sh));

    if (context->clipRegionIntersects  (Rectangle (dx, dy, dw, dh)))
    {
        drawImageTransformed (imageToDraw, Rectangle (sx, sy, sw, sh),
                              AffineTransform::scale (dw / (float) sw, dh / (float) sh)
                                              .translated ((float) dx, (float) dy),
                              fillAlphaChannelWithCurrentBrush);
    }
}

void Graphics::drawImageTransformed (const Image* const imageToDraw,
                                     const Rectangle& imageSubRegion,
                                     const AffineTransform& transform,
                                     const bool fillAlphaChannelWithCurrentBrush) const throw()
{
    if (imageToDraw != 0 && ! context->isClipEmpty())
    {
        const Rectangle srcClip (imageSubRegion.getIntersection (imageToDraw->getBounds()));

        if (fillAlphaChannelWithCurrentBrush)
        {
            context->saveState();
            context->clipToImageAlpha (*imageToDraw, srcClip, transform);
            fillAll();
            context->restoreState();
        }
        else
        {
            context->drawImage (*imageToDraw, srcClip, transform, false);
        }
    }
}


END_JUCE_NAMESPACE
