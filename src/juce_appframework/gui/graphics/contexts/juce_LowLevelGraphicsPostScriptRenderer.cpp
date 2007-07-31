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

#include "juce_LowLevelGraphicsPostScriptRenderer.h"
#include "juce_EdgeTable.h"
#include "../imaging/juce_Image.h"
#include "../colour/juce_PixelFormats.h"
#include "../geometry/juce_PathStrokeType.h"
#include "../geometry/juce_Rectangle.h"
#include "../../../../juce_core/containers/juce_SparseSet.h"

#if JUCE_MSVC
  #pragma warning (disable: 4996) // deprecated sprintf warning
#endif


// this will throw an assertion if you try to draw something that's not
// possible in postscript
#define WARN_ABOUT_NON_POSTSCRIPT_OPERATIONS 0


//==============================================================================
#if defined (JUCE_DEBUG) && WARN_ABOUT_NON_POSTSCRIPT_OPERATIONS
 #define notPossibleInPostscriptAssert jassertfalse
#else
 #define notPossibleInPostscriptAssert
#endif

//==============================================================================
LowLevelGraphicsPostScriptRenderer::LowLevelGraphicsPostScriptRenderer (OutputStream& resultingPostScript,
                                                                        const String& documentTitle,
                                                                        const int totalWidth_,
                                                                        const int totalHeight_)
    : out (resultingPostScript),
      totalWidth (totalWidth_),
      totalHeight (totalHeight_),
      xOffset (0),
      yOffset (0),
      needToClip (true)
{
    clip = new RectangleList (Rectangle (0, 0, totalWidth_, totalHeight_));

    const float scale = jmin ((520.0f / totalWidth_), (750.0f / totalHeight));

    out << "%!PS-Adobe-3.0 EPSF-3.0"
           "\n%%BoundingBox: 0 0 600 824"
           "\n%%Pages: 0"
           "\n%%Creator: Raw Material Software JUCE"
           "\n%%Title: " << documentTitle <<
           "\n%%CreationDate: none"
           "\n%%LanguageLevel: 2"
           "\n%%EndComments"
           "\n%%BeginProlog"
           "\n%%BeginResource: JRes"
           "\n/bd {bind def} bind def"
           "\n/c {setrgbcolor} bd"
           "\n/m {moveto} bd"
           "\n/l {lineto} bd"
           "\n/rl {rlineto} bd"
           "\n/ct {curveto} bd"
           "\n/cp {closepath} bd"
           "\n/pr {3 index 3 index moveto 1 index 0 rlineto 0 1 index rlineto pop neg 0 rlineto pop pop closepath} bd"
           "\n/doclip {initclip newpath} bd"
           "\n/endclip {clip newpath} bd"
           "\n%%EndResource"
           "\n%%EndProlog"
           "\n%%BeginSetup"
           "\n%%EndSetup"
           "\n%%Page: 1 1"
           "\n%%BeginPageSetup"
           "\n%%EndPageSetup\n\n"
        << "40 800 translate\n"
        << scale << ' ' << scale << " scale\n\n";
}

LowLevelGraphicsPostScriptRenderer::~LowLevelGraphicsPostScriptRenderer()
{
    delete clip;
}

//==============================================================================
bool LowLevelGraphicsPostScriptRenderer::isVectorDevice() const
{
    return true;
}

void LowLevelGraphicsPostScriptRenderer::setOrigin (int x, int y)
{
    if (x != 0 || y != 0)
    {
        xOffset += x;
        yOffset += y;
        needToClip = true;
    }
}

bool LowLevelGraphicsPostScriptRenderer::reduceClipRegion (int x, int y, int w, int h)
{
    needToClip = true;
    return clip->clipTo (Rectangle (x + xOffset, y + yOffset, w, h));
}

bool LowLevelGraphicsPostScriptRenderer::reduceClipRegion (const RectangleList& clipRegion)
{
    needToClip = true;
    return clip->clipTo (clipRegion);
}

void LowLevelGraphicsPostScriptRenderer::excludeClipRegion (int x, int y, int w, int h)
{
    needToClip = true;
    clip->subtract (Rectangle (x + xOffset, y + yOffset, w, h));
}

bool LowLevelGraphicsPostScriptRenderer::clipRegionIntersects (int x, int y, int w, int h)
{
    return clip->intersectsRectangle (Rectangle (x + xOffset, y + yOffset, w, h));
}

const Rectangle LowLevelGraphicsPostScriptRenderer::getClipBounds() const
{
    return clip->getBounds().translated (-xOffset, -yOffset);
}

bool LowLevelGraphicsPostScriptRenderer::isClipEmpty() const
{
    return clip->isEmpty();
}

//==============================================================================
LowLevelGraphicsPostScriptRenderer::SavedState::SavedState (RectangleList* const clip_,
                                                            const int xOffset_, const int yOffset_)
    : clip (clip_),
      xOffset (xOffset_),
      yOffset (yOffset_)
{
}

LowLevelGraphicsPostScriptRenderer::SavedState::~SavedState()
{
    delete clip;
}

void LowLevelGraphicsPostScriptRenderer::saveState()
{
    stateStack.add (new SavedState (new RectangleList (*clip), xOffset, yOffset));
}

void LowLevelGraphicsPostScriptRenderer::restoreState()
{
    SavedState* const top = stateStack.getLast();

    if (top != 0)
    {
        clip->swapWith (*top->clip);

        xOffset = top->xOffset;
        yOffset = top->yOffset;

        stateStack.removeLast();

        needToClip = true;
    }
    else
    {
        jassertfalse // trying to pop with an empty stack!
    }
}

//==============================================================================
void LowLevelGraphicsPostScriptRenderer::writeClip()
{
    if (needToClip)
    {
        needToClip = false;

        out << "doclip ";

        int itemsOnLine = 0;

        for (RectangleList::Iterator i (*clip); i.next();)
        {
            if (++itemsOnLine == 6)
            {
                itemsOnLine = 0;
                out << '\n';
            }

            const Rectangle& r = *i.getRectangle();

            out << r.getX() << ' ' << -r.getY() << ' '
                << r.getWidth() << ' ' << -r.getHeight() << " pr ";
        }

        out << "endclip\n";
    }
}

void LowLevelGraphicsPostScriptRenderer::writeColour (const Colour& colour)
{
    Colour c (Colours::white.overlaidWith (colour));

    if (lastColour != c)
    {
        lastColour = c;

        out << String (c.getFloatRed(), 3) << ' '
            << String (c.getFloatGreen(), 3) << ' '
            << String (c.getFloatBlue(), 3) << " c\n";
    }
}

void LowLevelGraphicsPostScriptRenderer::writeXY (const float x, const float y) const
{
    out << String (x, 2) << ' '
        << String (-y, 2) << ' ';
}

void LowLevelGraphicsPostScriptRenderer::writePath (const Path& path) const
{
    out << "newpath ";

    float lastX = 0.0f;
    float lastY = 0.0f;
    int itemsOnLine = 0;

    Path::Iterator i (path);

    while (i.next())
    {
        if (++itemsOnLine == 4)
        {
            itemsOnLine = 0;
            out << '\n';
        }

        switch (i.elementType)
        {
        case Path::Iterator::startNewSubPath:
            writeXY (i.x1, i.y1);
            lastX = i.x1;
            lastY = i.y1;
            out << "m ";
            break;

        case Path::Iterator::lineTo:
            writeXY (i.x1, i.y1);
            lastX = i.x1;
            lastY = i.y1;
            out << "l ";
            break;

        case Path::Iterator::quadraticTo:
            {
                const float cp1x = lastX + (i.x1 - lastX) * 2.0f / 3.0f;
                const float cp1y = lastY + (i.y1 - lastY) * 2.0f / 3.0f;
                const float cp2x = cp1x + (i.x2 - lastX) / 3.0f;
                const float cp2y = cp1y + (i.y2 - lastY) / 3.0f;

                writeXY (cp1x, cp1y);
                writeXY (cp2x, cp2y);
                writeXY (i.x2, i.y2);
                out << "ct ";
                lastX = i.x2;
                lastY = i.y2;
            }
            break;

        case Path::Iterator::cubicTo:
            writeXY (i.x1, i.y1);
            writeXY (i.x2, i.y2);
            writeXY (i.x3, i.y3);
            out << "ct ";
            lastX = i.x3;
            lastY = i.y3;
            break;

        case Path::Iterator::closePath:
            out << "cp ";
            break;

        default:
            jassertfalse
            break;
        }
    }

    out << '\n';
}

void LowLevelGraphicsPostScriptRenderer::writeTransform (const AffineTransform& trans) const
{
    out << "[ "
        << trans.mat00 << ' '
        << trans.mat10 << ' '
        << trans.mat01 << ' '
        << trans.mat11 << ' '
        << trans.mat02 << ' '
        << trans.mat12 << " ] concat ";
}

//==============================================================================
void LowLevelGraphicsPostScriptRenderer::fillRectWithColour (int x, int y, int w, int h, const Colour& colour, const bool /*replaceExistingContents*/)
{
    writeClip();
    writeColour (colour);

    x += xOffset;
    y += yOffset;

    out << x << ' ' << -(y + h) << ' ' << w << ' ' << h << " rectfill\n";
}

void LowLevelGraphicsPostScriptRenderer::fillRectWithGradient (int x, int y, int w, int h, const ColourGradient& gradient)
{
    Path p;
    p.addRectangle ((float) x, (float) y, (float) w, (float) h);

    fillPathWithGradient (p, AffineTransform::identity, gradient, EdgeTable::Oversampling_256times);
}

//==============================================================================
void LowLevelGraphicsPostScriptRenderer::fillPathWithColour (const Path& path, const AffineTransform& t,
                                                             const Colour& colour, EdgeTable::OversamplingLevel /*quality*/)
{
    writeClip();

    Path p (path);
    p.applyTransform (t.translated ((float) xOffset, (float) yOffset));
    writePath (p);

    writeColour (colour);

    out << "fill\n";
}

void LowLevelGraphicsPostScriptRenderer::fillPathWithGradient (const Path& path, const AffineTransform& t, const ColourGradient& gradient, EdgeTable::OversamplingLevel /*quality*/)
{
    // this doesn't work correctly yet - it could be improved to handle solid gradients, but
    // postscript can't do semi-transparent ones.
    notPossibleInPostscriptAssert   // you can disable this warning by setting the WARN_ABOUT_NON_POSTSCRIPT_OPERATIONS flag at the top of this file

    writeClip();
    out << "gsave ";

    {
        Path p (path);
        p.applyTransform (t.translated ((float) xOffset, (float) yOffset));
        writePath (p);
        out << "clip\n";
    }

    int numColours = 256;
    PixelARGB* const colours = gradient.createLookupTable (numColours);

    for (int i = numColours; --i >= 0;)
        colours[i].unpremultiply();

    const Rectangle bounds (clip->getBounds());

    // ideally this would draw lots of lines or ellipses to approximate the gradient, but for the
    // time-being, this just fills it with the average colour..
    writeColour (Colour (colours [numColours / 2].getARGB()));
    out << bounds.getX() << ' ' << -bounds.getBottom() << ' ' << bounds.getWidth() << ' ' << bounds.getHeight() << " rectfill\n";


    juce_free (colours);
    out << "grestore\n";
}

void LowLevelGraphicsPostScriptRenderer::fillPathWithImage (const Path& path, const AffineTransform& transform,
                                                            const Image& sourceImage,
                                                            int imageX, int imageY,
                                                            float opacity, EdgeTable::OversamplingLevel /*quality*/)
{
    writeClip();

    out << "gsave ";
    Path p (path);
    p.applyTransform (transform.translated ((float) xOffset, (float) yOffset));
    writePath (p);
    out << "clip\n";

    blendImage (sourceImage, imageX, imageY, sourceImage.getWidth(), sourceImage.getHeight(), 0, 0, opacity);

    out << "grestore\n";
}


//==============================================================================
void LowLevelGraphicsPostScriptRenderer::fillAlphaChannelWithColour (const Image& /*clipImage*/, int x, int y, const Colour& colour)
{
    x += xOffset;
    y += yOffset;

    writeClip();
    writeColour (colour);

    notPossibleInPostscriptAssert   // you can disable this warning by setting the WARN_ABOUT_NON_POSTSCRIPT_OPERATIONS flag at the top of this file
}

void LowLevelGraphicsPostScriptRenderer::fillAlphaChannelWithGradient (const Image& /*alphaChannelImage*/, int imageX, int imageY, const ColourGradient& /*gradient*/)
{
    imageX += xOffset;
    imageY += yOffset;

    writeClip();

    notPossibleInPostscriptAssert   // you can disable this warning by setting the WARN_ABOUT_NON_POSTSCRIPT_OPERATIONS flag at the top of this file
}

void LowLevelGraphicsPostScriptRenderer::fillAlphaChannelWithImage (const Image& /*alphaImage*/, int alphaImageX, int alphaImageY,
                                                                    const Image& /*fillerImage*/, int fillerImageX, int fillerImageY, float /*opacity*/)
{
    alphaImageX += xOffset;
    alphaImageY += yOffset;

    fillerImageX += xOffset;
    fillerImageY += yOffset;

    writeClip();

    notPossibleInPostscriptAssert   // you can disable this warning by setting the WARN_ABOUT_NON_POSTSCRIPT_OPERATIONS flag at the top of this file
}


//==============================================================================
void LowLevelGraphicsPostScriptRenderer::blendImageRescaling (const Image& sourceImage,
                                                              int dx, int dy, int dw, int dh,
                                                              int sx, int sy, int sw, int sh,
                                                              float alpha,
                                                              const Graphics::ResamplingQuality quality)
{
    if (sw > 0 && sh > 0)
    {
        jassert (sx >= 0 && sx + sw <= sourceImage.getWidth());
        jassert (sy >= 0 && sy + sh <= sourceImage.getHeight());

        if (sw == dw && sh == dh)
        {
            blendImage (sourceImage,
                        dx, dy, dw, dh,
                        sx, sy, alpha);
        }
        else
        {
            blendImageWarping (sourceImage,
                               sx, sy, sw, sh,
                               AffineTransform::scale (dw / (float) sw,
                                                       dh / (float) sh)
                                   .translated ((float) (dx - sx),
                                                (float) (dy - sy)),
                               alpha,
                               quality);
        }
    }
}

//==============================================================================
void LowLevelGraphicsPostScriptRenderer::blendImage (const Image& sourceImage, int dx, int dy, int dw, int dh, int sx, int sy, float opacity)
{
    blendImageWarping (sourceImage,
                       sx, sy, dw, dh,
                       AffineTransform::translation ((float) dx, (float) dy),
                       opacity, Graphics::highResamplingQuality);
}

//==============================================================================
void LowLevelGraphicsPostScriptRenderer::writeImage (const Image& im,
                                                     const int sx, const int sy,
                                                     const int maxW, const int maxH) const
{
    out << "{<\n";

    const int w = jmin (maxW, im.getWidth());
    const int h = jmin (maxH, im.getHeight());

    int charsOnLine = 0;
    int lineStride, pixelStride;
    const uint8* data = im.lockPixelDataReadOnly (0, 0, w, h, lineStride, pixelStride);

    Colour pixel;

    for (int y = h; --y >= 0;)
    {
        for (int x = 0; x < w; ++x)
        {
            const uint8* pixelData = data + lineStride * y + pixelStride * x;

            if (x >= sx && y >= sy)
            {
                if (im.isARGB())
                {
                    PixelARGB p (*(const PixelARGB*) pixelData);
                    p.unpremultiply();
                    pixel = Colours::white.overlaidWith (Colour (p.getARGB()));
                }
                else if (im.isRGB())
                {
                    pixel = Colour (((const PixelRGB*) pixelData)->getARGB());
                }
                else
                {
                    pixel = Colour ((uint8) 0, (uint8) 0, (uint8) 0, *pixelData);
                }
            }
            else
            {
                pixel = Colours::transparentWhite;
            }

            char colourString [16];
            sprintf (colourString, "%x%x%x", pixel.getRed(), pixel.getGreen(), pixel.getBlue());

            out << (const char*) colourString;
            charsOnLine += 3;

            if (charsOnLine > 100)
            {
                out << '\n';
                charsOnLine = 0;
            }
        }
    }

    im.releasePixelDataReadOnly (data);

    out << "\n>}\n";
}

void LowLevelGraphicsPostScriptRenderer::blendImageWarping (const Image& sourceImage,
                                                            int srcClipX, int srcClipY,
                                                            int srcClipW, int srcClipH,
                                                            const AffineTransform& t,
                                                            float /*opacity*/,
                                                            const Graphics::ResamplingQuality /*quality*/)
{
    const int w = jmin (sourceImage.getWidth(), srcClipX + srcClipW);
    const int h = jmin (sourceImage.getHeight(), srcClipY + srcClipH);

    writeClip();

    out << "gsave ";
    writeTransform (t.translated ((float) xOffset, (float) yOffset)
                     .scaled (1.0f, -1.0f));

    RectangleList imageClip;
    sourceImage.createSolidAreaMask (imageClip, 0.5f);
    imageClip.clipTo (Rectangle (srcClipX, srcClipY, srcClipW, srcClipH));

    out << "newpath ";
    int itemsOnLine = 0;

    for (RectangleList::Iterator i (imageClip); i.next();)
    {
        if (++itemsOnLine == 6)
        {
            out << '\n';
            itemsOnLine = 0;
        }

        const Rectangle& r = *i.getRectangle();

        out << r.getX() << ' ' << r.getY() << ' ' << r.getWidth() << ' ' << r.getHeight() << " pr ";
    }

    out << " clip newpath\n";

    out << w << ' ' << h << " scale\n";
    out << w << ' ' << h << " 8 [" << w << " 0 0 -" << h << ' ' << (int) 0 << ' ' << h << " ]\n";

    writeImage (sourceImage, srcClipX, srcClipY, srcClipW, srcClipH);

    out << "false 3 colorimage grestore\n";
    needToClip = true;
}


//==============================================================================
void LowLevelGraphicsPostScriptRenderer::drawLine (double x1, double y1, double x2, double y2, const Colour& colour)
{
    Path p;
    p.addLineSegment ((float) x1, (float) y1, (float) x2, (float) y2, 1.0f);

    fillPathWithColour (p, AffineTransform::identity, colour, EdgeTable::Oversampling_256times);
}

void LowLevelGraphicsPostScriptRenderer::drawVerticalLine (const int x, double top, double bottom, const Colour& col)
{
    drawLine (x, top, x, bottom, col);
}


void LowLevelGraphicsPostScriptRenderer::drawHorizontalLine (const int y, double left, double right, const Colour& col)
{
    drawLine (left, y, right, y, col);
}


END_JUCE_NAMESPACE
