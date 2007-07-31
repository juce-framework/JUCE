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

#include "juce_Graphics.h"
#include "../fonts/juce_GlyphArrangement.h"
#include "../geometry/juce_PathStrokeType.h"
#include "juce_EdgeTable.h"
#include "juce_LowLevelGraphicsContext.h"


static const Graphics::ResamplingQuality defaultQuality = Graphics::mediumResamplingQuality;

//==============================================================================
#define MINIMUM_COORD -0x3fffffff
#define MAXIMUM_COORD 0x3fffffff

#define ASSERT_COORDS_ARE_SENSIBLE_NUMBERS(x, y, w, h) \
    jassert ((int) x >= MINIMUM_COORD  \
              && (int) x <= MAXIMUM_COORD \
              && (int) y >= MINIMUM_COORD \
              && (int) y <= MAXIMUM_COORD \
              && (int) w >= MINIMUM_COORD \
              && (int) w <= MAXIMUM_COORD \
              && (int) h >= MINIMUM_COORD \
              && (int) h <= MAXIMUM_COORD);


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
      ownsContext (true),
      state (new GraphicsState()),
      saveStatePending (false)
{
}

Graphics::Graphics (LowLevelGraphicsContext* const internalContext) throw()
    : context (internalContext),
      ownsContext (false),
      state (new GraphicsState()),
      saveStatePending (false)
{
}

Graphics::~Graphics() throw()
{
    delete state;

    if (ownsContext)
        delete context;
}

//==============================================================================
void Graphics::resetToDefaultState() throw()
{
    setColour (Colours::black);
    state->font.resetToDefaultState();
    state->quality = defaultQuality;
}

bool Graphics::isVectorDevice() const throw()
{
    return context->isVectorDevice();
}

bool Graphics::reduceClipRegion (const int x, const int y,
                                 const int w, const int h) throw()
{
    saveStateIfPending();
    return context->reduceClipRegion (x, y, w, h);
}

bool Graphics::reduceClipRegion (const RectangleList& clipRegion) throw()
{
    saveStateIfPending();
    return context->reduceClipRegion (clipRegion);
}

void Graphics::excludeClipRegion (const int x, const int y,
                                  const int w, const int h) throw()
{
    saveStateIfPending();
    context->excludeClipRegion (x, y, w, h);
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
    {
        saveStatePending = false;
    }
    else
    {
        const int stackSize = stateStack.size();

        if (stackSize > 0)
        {
            context->restoreState();

            delete state;
            state = stateStack.getUnchecked (stackSize - 1);

            stateStack.removeLast (1, false);
        }
        else
        {
            // Trying to call restoreState() more times than you've called saveState() !
            // Be careful to correctly match each saveState() with exactly one call to restoreState().
            jassertfalse
        }
    }
}

void Graphics::saveStateIfPending() throw()
{
    if (saveStatePending)
    {
        saveStatePending = false;

        context->saveState();
        stateStack.add (new GraphicsState (*state));
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
    return context->clipRegionIntersects (x, y, w, h);
}

//==============================================================================
void Graphics::setColour (const Colour& newColour) throw()
{
    saveStateIfPending();
    state->colour = newColour;
    deleteAndZero (state->brush);
}

const Colour& Graphics::getCurrentColour() const throw()
{
    return state->colour;
}

void Graphics::setOpacity (const float newOpacity) throw()
{
    saveStateIfPending();
    state->colour = state->colour.withAlpha (newOpacity);
}

void Graphics::setBrush (const Brush* const newBrush) throw()
{
    saveStateIfPending();
    delete state->brush;

    if (newBrush != 0)
        state->brush = newBrush->createCopy();
    else
        state->brush = 0;
}

//==============================================================================
Graphics::GraphicsState::GraphicsState() throw()
    : colour (Colours::black),
      brush (0),
      quality (defaultQuality)
{
}

Graphics::GraphicsState::GraphicsState (const GraphicsState& other) throw()
    : colour (other.colour),
      brush (other.brush != 0 ? other.brush->createCopy() : 0),
      font (other.font),
      quality (other.quality)
{
}

Graphics::GraphicsState::~GraphicsState() throw()
{
    delete brush;
}

//==============================================================================
void Graphics::setFont (const Font& newFont) throw()
{
    saveStateIfPending();
    state->font = newFont;
}

void Graphics::setFont (const float newFontHeight,
                        const int newFontStyleFlags) throw()
{
    saveStateIfPending();
    state->font.setSizeAndStyle (newFontHeight, newFontStyleFlags, 1.0f, 0.0f);
}

const Font& Graphics::getCurrentFont() const throw()
{
    return state->font;
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
        arr.addLineOfText (state->font, text, (float) startX, (float) baselineY);
        arr.draw (*this);
    }
}

void Graphics::drawTextAsPath (const String& text,
                               const AffineTransform& transform) const throw()
{
    if (text.isNotEmpty())
    {
        GlyphArrangement arr;
        arr.addLineOfText (state->font, text, 0.0f, 0.0f);
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
        arr.addJustifiedText (state->font, text,
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
    if (text.isNotEmpty() && context->clipRegionIntersects (x, y, width, height))
    {
        GlyphArrangement arr;

        arr.addCurtailedLineOfText (state->font, text,
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
         && context->clipRegionIntersects (x, y, width, height))
    {
        GlyphArrangement arr;

        arr.addFittedText (state->font, text,
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
    ASSERT_COORDS_ARE_SENSIBLE_NUMBERS (x, y, width, height);

    SolidColourBrush colourBrush (state->colour);
    (state->brush != 0 ? *(state->brush) : (Brush&) colourBrush).paintRectangle (*context, x, y, width, height);
}

void Graphics::fillRect (const Rectangle& r) const throw()
{
    fillRect (r.getX(),
              r.getY(),
              r.getWidth(),
              r.getHeight());
}

void Graphics::fillRect (const float x,
                         const float y,
                         const float width,
                         const float height) const throw()
{
    // passing in a silly number can cause maths problems in rendering!
    ASSERT_COORDS_ARE_SENSIBLE_NUMBERS (x, y, width, height);

    Path p;
    p.addRectangle (x, y, width, height);
    fillPath (p);
}

void Graphics::setPixel (int x, int y) const throw()
{
    if (context->clipRegionIntersects (x, y, 1, 1))
    {
        SolidColourBrush colourBrush (state->colour);
        (state->brush != 0 ? *(state->brush) : (Brush&) colourBrush).paintRectangle (*context, x, y, 1, 1);
    }
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

        context->fillRectWithColour (clip.getX(), clip.getY(), clip.getWidth(), clip.getHeight(),
                                     colourToUse, false);
    }
}


//==============================================================================
void Graphics::fillPath (const Path& path,
                         const AffineTransform& transform) const throw()
{
    if ((! context->isClipEmpty()) && ! path.isEmpty())
    {
        SolidColourBrush colourBrush (state->colour);
        (state->brush != 0 ? *(state->brush) : (Brush&) colourBrush).paintPath (*context, path, transform);
    }
}

void Graphics::strokePath (const Path& path,
                           const PathStrokeType& strokeType,
                           const AffineTransform& transform) const throw()
{
    if (! state->colour.isTransparent())
    {
        Path stroke;
        strokeType.createStrokedPath (stroke, path, transform);
        fillPath (stroke);
    }
}

//==============================================================================
void Graphics::drawRect (const int x,
                         const int y,
                         const int width,
                         const int height,
                         const int lineThickness) const throw()
{
    // passing in a silly number can cause maths problems in rendering!
    ASSERT_COORDS_ARE_SENSIBLE_NUMBERS (x, y, width, height);

    SolidColourBrush colourBrush (state->colour);
    Brush& b = (state->brush != 0 ? *(state->brush) : (Brush&) colourBrush);

    b.paintRectangle (*context, x, y, width, lineThickness);
    b.paintRectangle (*context, x, y + lineThickness, lineThickness, height - lineThickness * 2);
    b.paintRectangle (*context, x + width - lineThickness, y + lineThickness, lineThickness, height - lineThickness * 2);
    b.paintRectangle (*context, x, y + height - lineThickness, width, lineThickness);
}

void Graphics::drawBevel (const int x,
                          const int y,
                          const int width,
                          const int height,
                          const int bevelThickness,
                          const Colour& topLeftColour,
                          const Colour& bottomRightColour,
                          const bool useGradient) const throw()
{
    // passing in a silly number can cause maths problems in rendering!
    ASSERT_COORDS_ARE_SENSIBLE_NUMBERS (x, y, width, height);

    if (clipRegionIntersects (x, y, width, height))
    {
        const float oldOpacity = state->colour.getFloatAlpha();
        const float ramp = oldOpacity / bevelThickness;

        for (int i = bevelThickness; --i >= 0;)
        {
            const float op = useGradient ? ramp * (bevelThickness - i)
                                         : oldOpacity;

            context->fillRectWithColour (x + i, y + i, width - i * 2, 1, topLeftColour.withMultipliedAlpha (op), false);
            context->fillRectWithColour (x + i, y + i + 1, 1, height - i * 2 - 2, topLeftColour.withMultipliedAlpha (op * 0.75f), false);
            context->fillRectWithColour (x + i, y + height - i - 1, width - i * 2, 1, bottomRightColour.withMultipliedAlpha (op), false);
            context->fillRectWithColour (x + width - i - 1, y + i + 1, 1, height - i * 2 - 2, bottomRightColour.withMultipliedAlpha (op  * 0.75f), false);
        }
    }
}

//==============================================================================
void Graphics::fillEllipse (const float x,
                            const float y,
                            const float width,
                            const float height) const throw()
{
    // passing in a silly number can cause maths problems in rendering!
    ASSERT_COORDS_ARE_SENSIBLE_NUMBERS (x, y, width, height);

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
    ASSERT_COORDS_ARE_SENSIBLE_NUMBERS (x, y, width, height);

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
    ASSERT_COORDS_ARE_SENSIBLE_NUMBERS (x, y, width, height);

    Path p;
    p.addRoundedRectangle (x, y, width, height, cornerSize);
    fillPath (p);
}

void Graphics::drawRoundedRectangle (const float x,
                                     const float y,
                                     const float width,
                                     const float height,
                                     const float cornerSize,
                                     const float lineThickness) const throw()
{
    // passing in a silly number can cause maths problems in rendering!
    ASSERT_COORDS_ARE_SENSIBLE_NUMBERS (x, y, width, height);

    Path p;
    p.addRoundedRectangle (x, y, width, height, cornerSize);
    strokePath (p, PathStrokeType (lineThickness));
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
        if (colour1 == colour2)
        {
            context->fillRectWithColour (x, y, width, height, colour1, false);
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
                    context->fillRectWithColour (xx, y,
                                                 jmin (checkWidth, right - xx),
                                                 jmin (checkHeight, bottom - y),
                                                 ((cx++ & 1) == 0) ? colour1 : colour2,
                                                 false);

                ++cy;
                y += checkHeight;
            }
        }
    }
}

//==============================================================================
void Graphics::drawVerticalLine (const int x, float top, float bottom) const throw()
{
    SolidColourBrush colourBrush (state->colour);
    (state->brush != 0 ? *(state->brush) : (Brush&) colourBrush).paintVerticalLine (*context, x, top, bottom);
}

void Graphics::drawHorizontalLine (const int y, float left, float right) const throw()
{
    SolidColourBrush colourBrush (state->colour);
    (state->brush != 0 ? *(state->brush) : (Brush&) colourBrush).paintHorizontalLine (*context, y, left, right);
}

void Graphics::drawLine (float x1, float y1,
                         float x2, float y2) const throw()
{
    if (! context->isClipEmpty())
    {
        SolidColourBrush colourBrush (state->colour);
        (state->brush != 0 ? *(state->brush) : (Brush&) colourBrush).paintLine (*context, x1, y1, x2, y2);
    }
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
    state->quality = newQuality;
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
    ASSERT_COORDS_ARE_SENSIBLE_NUMBERS (destX, destY, destW, destH);

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
                           roundDoubleToInt (newX), roundDoubleToInt (newY),
                           roundDoubleToInt (newW), roundDoubleToInt (newH),
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
    ASSERT_COORDS_ARE_SENSIBLE_NUMBERS (dx, dy, dw, dh);
    ASSERT_COORDS_ARE_SENSIBLE_NUMBERS (sx, sy, sw, sh);

    if (imageToDraw == 0 || ! context->clipRegionIntersects  (dx, dy, dw, dh))
        return;

    if (sw == dw && sh == dh)
    {
        if (sx < 0)
        {
            dx -= sx;
            dw += sx;
            sw += sx;
            sx = 0;
        }

        if (sx + sw > imageToDraw->getWidth())
        {
            const int amount = sx + sw - imageToDraw->getWidth();
            dw -= amount;
            sw -= amount;
        }

        if (sy < 0)
        {
            dy -= sy;
            dh += sy;
            sh += sy;
            sy = 0;
        }

        if (sy + sh > imageToDraw->getHeight())
        {
            const int amount = sy + sh - imageToDraw->getHeight();
            dh -= amount;
            sh -= amount;
        }

        if (dw <= 0 || dh <= 0 || sw <= 0 || sh <= 0)
            return;

        if (fillAlphaChannelWithCurrentBrush)
        {
            SolidColourBrush colourBrush (state->colour);
            (state->brush != 0 ? *(state->brush) : (Brush&) colourBrush)
                .paintAlphaChannel (*context, *imageToDraw,
                                    dx - sx, dy - sy,
                                    dx, dy,
                                    dw, dh);
        }
        else
        {
            context->blendImage (*imageToDraw,
                                 dx, dy, dw, dh, sx, sy,
                                 state->colour.getFloatAlpha());
        }
    }
    else
    {
        if (dw <= 0 || dh <= 0 || sw <= 0 || sh <= 0)
            return;

        if (fillAlphaChannelWithCurrentBrush)
        {
            if (imageToDraw->isRGB())
            {
                fillRect (dx, dy, dw, dh);
            }
            else
            {
                int tx = dx;
                int ty = dy;
                int tw = dw;
                int th = dh;

                if (context->getClipBounds().intersectRectangle (tx, ty, tw, th))
                {
                    Image temp (imageToDraw->getFormat(), tw, th, true);
                    Graphics g (temp);
                    g.setImageResamplingQuality (state->quality);
                    g.setOrigin (dx - tx, dy - ty);

                    g.drawImage (imageToDraw,
                                 0, 0, dw, dh,
                                 sx, sy, sw, sh,
                                 false);

                    SolidColourBrush colourBrush (state->colour);
                    (state->brush != 0 ? *(state->brush) : (Brush&) colourBrush)
                        .paintAlphaChannel (*context, temp, tx, ty, tx, ty, tw, th);
                }
            }
        }
        else
        {
            context->blendImageRescaling (*imageToDraw,
                                          dx, dy, dw, dh,
                                          sx, sy, sw, sh,
                                          state->colour.getFloatAlpha(),
                                          state->quality);
        }
    }
}

void Graphics::drawImageTransformed (const Image* const imageToDraw,
                                     int sourceClipX,
                                     int sourceClipY,
                                     int sourceClipWidth,
                                     int sourceClipHeight,
                                     const AffineTransform& transform,
                                     const bool fillAlphaChannelWithCurrentBrush) const throw()
{
    if (imageToDraw != 0
         && (! context->isClipEmpty())
         && ! transform.isSingularity())
    {
        if (fillAlphaChannelWithCurrentBrush)
        {
            Path p;
            p.addRectangle ((float) sourceClipX, (float) sourceClipY,
                            (float) sourceClipWidth, (float) sourceClipHeight);

            p.applyTransform (transform);

            float dx, dy, dw, dh;
            p.getBounds (dx, dy, dw, dh);
            int tx = (int) dx;
            int ty = (int) dy;
            int tw = roundFloatToInt (dw) + 2;
            int th = roundFloatToInt (dh) + 2;

            if (context->getClipBounds().intersectRectangle (tx, ty, tw, th))
            {
                Image temp (imageToDraw->getFormat(), tw, th, true);
                Graphics g (temp);
                g.setImageResamplingQuality (state->quality);

                g.drawImageTransformed (imageToDraw,
                                        sourceClipX,
                                        sourceClipY,
                                        sourceClipWidth,
                                        sourceClipHeight,
                                        transform.translated ((float) -tx, (float) -ty),
                                        false);

                SolidColourBrush colourBrush (state->colour);
                (state->brush != 0 ? *(state->brush) : (Brush&) colourBrush).paintAlphaChannel (*context, temp, tx, ty, tx, ty, tw, th);
            }
        }
        else
        {
            context->blendImageWarping (*imageToDraw,
                                        sourceClipX,
                                        sourceClipY,
                                        sourceClipWidth,
                                        sourceClipHeight,
                                        transform,
                                        state->colour.getFloatAlpha(),
                                        state->quality);
        }
    }
}


END_JUCE_NAMESPACE
