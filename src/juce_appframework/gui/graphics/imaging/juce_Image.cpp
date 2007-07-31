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


#include "juce_Image.h"
#include "../contexts/juce_Graphics.h"
#include "../contexts/juce_LowLevelGraphicsSoftwareRenderer.h"
#include "../brushes/juce_GradientBrush.h"
#include "../colour/juce_PixelFormats.h"
#include "../../../../juce_core/containers/juce_SparseSet.h"

static const int fullAlphaThreshold = 253;


//==============================================================================
Image::Image (const PixelFormat format_,
              const int imageWidth_,
              const int imageHeight_)
  : format (format_),
    imageWidth (imageWidth_),
    imageHeight (imageHeight_),
    imageData (0)
{
    jassert (format_ == RGB || format_ == ARGB || format_ == SingleChannel);
    jassert (imageWidth_ > 0 && imageHeight_ > 0); // it's illegal to create a zero-sized image - the
                                                   //  actual image will be at least 1x1.
}

Image::Image (const PixelFormat format_,
              const int imageWidth_,
              const int imageHeight_,
              const bool clearImage)
  : format (format_),
    imageWidth (imageWidth_),
    imageHeight (imageHeight_)
{
    jassert (format_ == RGB || format_ == ARGB || format_ == SingleChannel);
    jassert (imageWidth_ > 0 && imageHeight_ > 0); // it's illegal to create a zero-sized image - the
                                                   //  actual image will be at least 1x1.

    pixelStride = (format == RGB) ? 3 : ((format == ARGB) ? 4 : 1);
    lineStride = (pixelStride * jmax (1, imageWidth_) + 3) & ~3;
    const int dataSize = lineStride * jmax (1, imageHeight_);

    imageData = (uint8*) (clearImage ? juce_calloc (dataSize)
                                     : juce_malloc (dataSize));
}

Image::Image (const Image& other)
   : format (other.format),
     imageWidth (other.imageWidth),
     imageHeight (other.imageHeight)
{
    pixelStride = (format == RGB) ? 3 : ((format == ARGB) ? 4 : 1);
    lineStride = (pixelStride * jmax (1, imageWidth) + 3) & ~3;
    const int dataSize = lineStride * jmax (1, imageHeight);

    imageData = (uint8*) juce_malloc (dataSize);

    int ls, ps;
    const uint8* srcData = other.lockPixelDataReadOnly (0, 0, imageWidth, imageHeight, ls, ps);
    setPixelData (0, 0, imageWidth, imageHeight, srcData, ls);
    other.releasePixelDataReadOnly (srcData);
}

Image::~Image()
{
    juce_free (imageData);
}

//==============================================================================
LowLevelGraphicsContext* Image::createLowLevelContext()
{
    return new LowLevelGraphicsSoftwareRenderer (*this);
}

//==============================================================================
uint8* Image::lockPixelDataReadWrite (int x, int y, int w, int h, int& ls, int& ps)
{
    jassert (x >= 0 && y >= 0 && w > 0 && h > 0 && x + w <= imageWidth && y + h <= imageHeight);
    w = w;
    h = h;

    ls = lineStride;
    ps = pixelStride;
    return imageData + x * pixelStride + y * lineStride;
}

void Image::releasePixelDataReadWrite (void*)
{
}

const uint8* Image::lockPixelDataReadOnly (int x, int y, int w, int h, int& ls, int& ps) const
{
    jassert (x >= 0 && y >= 0 && w > 0 && h > 0 && x + w <= imageWidth && y + h <= imageHeight);
    w = w;
    h = h;

    ls = lineStride;
    ps = pixelStride;
    return imageData + x * pixelStride + y * lineStride;
}

void Image::releasePixelDataReadOnly (const void*) const
{
}

void Image::setPixelData (int x, int y, int w, int h,
                          const uint8* sourcePixelData, int sourceLineStride)
{
    jassert (x >= 0 && y >= 0 && w > 0 && h > 0 && x + w <= imageWidth && y + h <= imageHeight);

    if (Rectangle::intersectRectangles (x, y, w, h, 0, 0, imageWidth, imageHeight))
    {
        int ls, ps;
        uint8* dest = lockPixelDataReadWrite (x, y, w, h, ls, ps);

        for (int i = 0; i < h; ++i)
        {
            memcpy (dest + ls * i,
                    sourcePixelData + sourceLineStride * i,
                    w * pixelStride);
        }

        releasePixelDataReadWrite (dest);
    }
}

//==============================================================================
void Image::clear (int dx, int dy, int dw, int dh,
                   const Colour& colourToClearTo)
{
    const PixelARGB col (colourToClearTo.getPixelARGB());

    int ls, ps;
    uint8* dstData = lockPixelDataReadWrite (dx, dy, dw, dh, ls, ps);
    uint8* dest = dstData;

    while (--dh >= 0)
    {
        uint8* line = dest;
        dest += ls;

        if (isARGB())
        {
            for (int x = dw; --x >= 0;)
            {
                ((PixelARGB*) line)->set (col);
                line += ps;
            }
        }
        else if (isRGB())
        {
            for (int x = dw; --x >= 0;)
            {
                ((PixelRGB*) line)->set (col);
                line += ps;
            }
        }
        else
        {
            for (int x = dw; --x >= 0;)
            {
                *line = col.getAlpha();
                line += ps;
            }
        }
    }

    releasePixelDataReadWrite (dstData);
}

Image* Image::createCopy (int newWidth, int newHeight,
                          const Graphics::ResamplingQuality quality) const
{
    if (newWidth < 0)
        newWidth = imageWidth;

    if (newHeight < 0)
        newHeight = imageHeight;

    Image* const newImage = new Image (format, newWidth, newHeight, true);

    Graphics g (*newImage);
    g.setImageResamplingQuality (quality);

    g.drawImage (this,
                 0, 0, newWidth, newHeight,
                 0, 0, imageWidth, imageHeight,
                 false);

    return newImage;
}

//==============================================================================
const Colour Image::getPixelAt (const int x, const int y) const
{
    Colour c;

    if (x >= 0 && x < imageWidth && y >= 0 && y < imageHeight)
    {
        int ls, ps;
        const uint8* const pixels = lockPixelDataReadOnly (x, y, 1, 1, ls, ps);

        if (isARGB())
        {
            PixelARGB p (*(const PixelARGB*) pixels);
            p.unpremultiply();
            c = Colour (p.getARGB());
        }
        else if (isRGB())
            c = Colour (((const PixelRGB*) pixels)->getARGB());
        else
            c = Colour ((uint8) 0, (uint8) 0, (uint8) 0, *pixels);

        releasePixelDataReadOnly (pixels);
    }

    return c;
}

void Image::setPixelAt (const int x, const int y,
                        const Colour& colour)
{
    if (x >= 0 && x < imageWidth && y >= 0 && y < imageHeight)
    {
        int ls, ps;
        uint8* const pixels = lockPixelDataReadWrite (x, y, 1, 1, ls, ps);
        const PixelARGB col (colour.getPixelARGB());

        if (isARGB())
            ((PixelARGB*) pixels)->set (col);
        else if (isRGB())
            ((PixelRGB*) pixels)->set (col);
        else
            *pixels = col.getAlpha();

        releasePixelDataReadWrite (pixels);
    }
}

void Image::multiplyAlphaAt (const int x, const int y,
                             const float multiplier)
{
    if (x >= 0 && x < imageWidth && y >= 0 && y < imageHeight && hasAlphaChannel())
    {
        int ls, ps;
        uint8* const pixels = lockPixelDataReadWrite (x, y, 1, 1, ls, ps);

        if (isARGB())
            ((PixelARGB*) pixels)->multiplyAlpha (multiplier);
        else
            *pixels = (uint8) (*pixels * multiplier);

        releasePixelDataReadWrite (pixels);
    }
}

void Image::multiplyAllAlphas (const float amountToMultiplyBy)
{
    if (hasAlphaChannel())
    {
        int ls, ps;
        uint8* const pixels = lockPixelDataReadWrite (0, 0, getWidth(), getHeight(), ls, ps);

        if (isARGB())
        {
            for (int y = 0; y < imageHeight; ++y)
            {
                uint8* p = pixels + y * ls;

                for (int x = 0; x < imageWidth; ++x)
                {
                    ((PixelARGB*) p)->multiplyAlpha (amountToMultiplyBy);
                    p += ps;
                }
            }
        }
        else
        {
            for (int y = 0; y < imageHeight; ++y)
            {
                uint8* p = pixels + y * ls;

                for (int x = 0; x < imageWidth; ++x)
                {
                    *p = (uint8) (*p * amountToMultiplyBy);
                    p += ps;
                }
            }
        }

        releasePixelDataReadWrite (pixels);
    }
    else
    {
        jassertfalse // can't do this without an alpha-channel!
    }
}

void Image::desaturate()
{
    if (isARGB() || isRGB())
    {
        int ls, ps;
        uint8* const pixels = lockPixelDataReadWrite (0, 0, getWidth(), getHeight(), ls, ps);

        if (isARGB())
        {
            for (int y = 0; y < imageHeight; ++y)
            {
                uint8* p = pixels + y * ls;

                for (int x = 0; x < imageWidth; ++x)
                {
                    ((PixelARGB*) p)->desaturate();
                    p += ps;
                }
            }
        }
        else
        {
            for (int y = 0; y < imageHeight; ++y)
            {
                uint8* p = pixels + y * ls;

                for (int x = 0; x < imageWidth; ++x)
                {
                    ((PixelRGB*) p)->desaturate();
                    p += ps;
                }
            }
        }

        releasePixelDataReadWrite (pixels);
    }
}

void Image::createSolidAreaMask (RectangleList& result, const float alphaThreshold) const
{
    if (hasAlphaChannel())
    {
        const uint8 threshold = (uint8) jlimit (0, 255, roundFloatToInt (alphaThreshold * 255.0f));
        SparseSet <int> pixelsOnRow;

        int ls, ps;
        const uint8* const pixels = lockPixelDataReadOnly (0, 0, imageWidth, imageHeight, ls, ps);

        for (int y = 0; y < imageHeight; ++y)
        {
            pixelsOnRow.clear();
            const uint8* lineData = pixels + ls * y;

            if (isARGB())
            {
                for (int x = 0; x < imageWidth; ++x)
                {
                    if (((const PixelARGB*) lineData)->getAlpha() >= threshold)
                        pixelsOnRow.addRange (x, 1);

                    lineData += ps;
                }
            }
            else
            {
                for (int x = 0; x < imageWidth; ++x)
                {
                    if (*lineData >= threshold)
                        pixelsOnRow.addRange (x, 1);

                    lineData += ps;
                }
            }

            for (int i = 0; i < pixelsOnRow.getNumRanges(); ++i)
            {
                int x, w;

                if (pixelsOnRow.getRange (i, x, w))
                    result.add (Rectangle (x, y, w, 1));
            }

            result.consolidate();
        }

        releasePixelDataReadOnly (pixels);
    }
    else
    {
        result.add (0, 0, imageWidth, imageHeight);
    }
}

void Image::moveImageSection (int dx, int dy,
                              int sx, int sy,
                              int w, int h)
{
    if (dx < 0)
    {
        w += dx;
        sx -= dx;
        dx = 0;
    }

    if (dy < 0)
    {
        h += dy;
        sy -= dy;
        dy = 0;
    }

    if (sx < 0)
    {
        w += sx;
        dx -= sx;
        sx = 0;
    }

    if (sy < 0)
    {
        h += sy;
        dy -= sy;
        sy = 0;
    }

    const int minX = jmin (dx, sx);
    const int minY = jmin (dy, sy);

    w = jmin (w, getWidth() - jmax (sx, dx));
    h = jmin (h, getHeight() - jmax (sy, dy));

    if (w > 0 && h > 0)
    {
        const int maxX = jmax (dx, sx) + w;
        const int maxY = jmax (dy, sy) + h;

        int ls, ps;
        uint8* const pixels = lockPixelDataReadWrite (minX, minY, maxX - minX, maxY - minY, ls, ps);

        uint8* dst       = pixels + ls * (dy - minY) + ps * (dx - minX);
        const uint8* src = pixels + ls * (sy - minY) + ps * (sx - minX);

        const int lineSize = ps * w;

        if (dy > sy)
        {
            while (--h >= 0)
            {
                const int offset = h * ls;
                memmove (dst + offset, src + offset, lineSize);
            }
        }
        else if (dst != src)
        {
            while (--h >= 0)
            {
                memmove (dst, src, lineSize);
                dst += ls;
                src += ls;
            }
        }

        releasePixelDataReadWrite (pixels);
    }
}


END_JUCE_NAMESPACE
