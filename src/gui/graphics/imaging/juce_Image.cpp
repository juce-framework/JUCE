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


#include "juce_Image.h"
#include "../contexts/juce_Graphics.h"
#include "../contexts/juce_LowLevelGraphicsSoftwareRenderer.h"
#include "../colour/juce_PixelFormats.h"
#include "../../../containers/juce_SparseSet.h"

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

    imageDataAllocated.allocate (lineStride * jmax (1, imageHeight_), clearImage);
    imageData = imageDataAllocated;
}

Image::Image (const Image& other)
   : format (other.format),
     imageWidth (other.imageWidth),
     imageHeight (other.imageHeight)
{
    pixelStride = (format == RGB) ? 3 : ((format == ARGB) ? 4 : 1);
    lineStride = (pixelStride * jmax (1, imageWidth) + 3) & ~3;

    imageDataAllocated.malloc (lineStride * jmax (1, imageHeight));
    imageData = imageDataAllocated;

    BitmapData srcData (other, 0, 0, imageWidth, imageHeight);
    setPixelData (0, 0, imageWidth, imageHeight, srcData.data, srcData.lineStride);
}

Image::~Image()
{
}

//==============================================================================
LowLevelGraphicsContext* Image::createLowLevelContext()
{
    return new LowLevelGraphicsSoftwareRenderer (*this);
}

//==============================================================================
Image::BitmapData::BitmapData (Image& image, int x, int y, int w, int h, const bool /*makeWritable*/)
    : data (image.imageData + image.lineStride * y + image.pixelStride * x),
      lineStride (image.lineStride),
      pixelStride (image.pixelStride),
      width (w),
      height (h)
{
    jassert (x >= 0 && y >= 0 && w > 0 && h > 0 && x + w <= image.getWidth() && y + h <= image.getHeight());
}

Image::BitmapData::BitmapData (const Image& image, int x, int y, int w, int h)
    : data (image.imageData + image.lineStride * y + image.pixelStride * x),
      lineStride (image.lineStride),
      pixelStride (image.pixelStride),
      width (w),
      height (h)
{
    jassert (x >= 0 && y >= 0 && w > 0 && h > 0 && x + w <= image.getWidth() && y + h <= image.getHeight());
}

Image::BitmapData::~BitmapData()
{
}

void Image::setPixelData (int x, int y, int w, int h,
                          const uint8* sourcePixelData, int sourceLineStride)
{
    jassert (x >= 0 && y >= 0 && w > 0 && h > 0 && x + w <= imageWidth && y + h <= imageHeight);

    if (Rectangle::intersectRectangles (x, y, w, h, 0, 0, imageWidth, imageHeight))
    {
        const BitmapData dest (*this, x, y, w, h, true);

        for (int i = 0; i < h; ++i)
        {
            memcpy (dest.getLinePointer(i),
                    sourcePixelData + sourceLineStride * i,
                    w * dest.pixelStride);
        }
    }
}

//==============================================================================
void Image::clear (int dx, int dy, int dw, int dh,
                   const Colour& colourToClearTo)
{
    const PixelARGB col (colourToClearTo.getPixelARGB());

    const BitmapData destData (*this, dx, dy, dw, dh, true);
    uint8* dest = destData.data;

    while (--dh >= 0)
    {
        uint8* line = dest;
        dest += destData.lineStride;

        if (isARGB())
        {
            for (int x = dw; --x >= 0;)
            {
                ((PixelARGB*) line)->set (col);
                line += destData.pixelStride;
            }
        }
        else if (isRGB())
        {
            for (int x = dw; --x >= 0;)
            {
                ((PixelRGB*) line)->set (col);
                line += destData.pixelStride;
            }
        }
        else
        {
            for (int x = dw; --x >= 0;)
            {
                *line = col.getAlpha();
                line += destData.pixelStride;
            }
        }
    }
}

Image* Image::createCopy (int newWidth, int newHeight,
                          const Graphics::ResamplingQuality quality) const
{
    if (newWidth < 0)
        newWidth = imageWidth;

    if (newHeight < 0)
        newHeight = imageHeight;

    Image* const newImage = Image::createNativeImage (format, newWidth, newHeight, true);

    Graphics g (*newImage);
    g.setImageResamplingQuality (quality);

    g.drawImage (this,
                 0, 0, newWidth, newHeight,
                 0, 0, imageWidth, imageHeight,
                 false);

    return newImage;
}

Image* Image::createCopyOfAlphaChannel() const
{
    jassert (format != SingleChannel);

    Image* const newImage = Image::createNativeImage (SingleChannel, imageWidth, imageHeight, false);

    if (! hasAlphaChannel())
    {
        newImage->clear (0, 0, imageWidth, imageHeight, Colours::black);
    }
    else
    {
        const BitmapData destData (*newImage, 0, 0, imageWidth, imageHeight, true);
        const BitmapData srcData (*this, 0, 0, imageWidth, imageHeight);

        for (int y = 0; y < imageHeight; ++y)
        {
            const PixelARGB* src = (const PixelARGB*) srcData.getLinePointer(y);
            uint8* dst = destData.getLinePointer (y);

            for (int x = imageWidth; --x >= 0;)
            {
                *dst++ = src->getAlpha();
                ++src;
            }
        }
    }

    return newImage;
}

//==============================================================================
const Colour Image::getPixelAt (const int x, const int y) const
{
    Colour c;

    if (((unsigned int) x) < (unsigned int) imageWidth
         && ((unsigned int) y) < (unsigned int) imageHeight)
    {
        const BitmapData srcData (*this, x, y, 1, 1);

        if (isARGB())
        {
            PixelARGB p (*(const PixelARGB*) srcData.data);
            p.unpremultiply();
            c = Colour (p.getARGB());
        }
        else if (isRGB())
            c = Colour (((const PixelRGB*) srcData.data)->getARGB());
        else
            c = Colour ((uint8) 0, (uint8) 0, (uint8) 0, *(srcData.data));
    }

    return c;
}

void Image::setPixelAt (const int x, const int y,
                        const Colour& colour)
{
    if (((unsigned int) x) < (unsigned int) imageWidth
         && ((unsigned int) y) < (unsigned int) imageHeight)
    {
        const BitmapData destData (*this, x, y, 1, 1, true);
        const PixelARGB col (colour.getPixelARGB());

        if (isARGB())
            ((PixelARGB*) destData.data)->set (col);
        else if (isRGB())
            ((PixelRGB*) destData.data)->set (col);
        else
            *(destData.data) = col.getAlpha();
    }
}

void Image::multiplyAlphaAt (const int x, const int y,
                             const float multiplier)
{
    if (((unsigned int) x) < (unsigned int) imageWidth
         && ((unsigned int) y) < (unsigned int) imageHeight
         && hasAlphaChannel())
    {
        const BitmapData destData (*this, x, y, 1, 1, true);

        if (isARGB())
            ((PixelARGB*) destData.data)->multiplyAlpha (multiplier);
        else
            *(destData.data) = (uint8) (*(destData.data) * multiplier);
    }
}

void Image::multiplyAllAlphas (const float amountToMultiplyBy)
{
    if (hasAlphaChannel())
    {
        const BitmapData destData (*this, 0, 0, getWidth(), getHeight(), true);

        if (isARGB())
        {
            for (int y = 0; y < imageHeight; ++y)
            {
                uint8* p = destData.getLinePointer (y);

                for (int x = 0; x < imageWidth; ++x)
                {
                    ((PixelARGB*) p)->multiplyAlpha (amountToMultiplyBy);
                    p += destData.pixelStride;
                }
            }
        }
        else
        {
            for (int y = 0; y < imageHeight; ++y)
            {
                uint8* p = destData.getLinePointer (y);

                for (int x = 0; x < imageWidth; ++x)
                {
                    *p = (uint8) (*p * amountToMultiplyBy);
                    p += destData.pixelStride;
                }
            }
        }
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
        const BitmapData destData (*this, 0, 0, getWidth(), getHeight(), true);

        if (isARGB())
        {
            for (int y = 0; y < imageHeight; ++y)
            {
                uint8* p = destData.getLinePointer (y);

                for (int x = 0; x < imageWidth; ++x)
                {
                    ((PixelARGB*) p)->desaturate();
                    p += destData.pixelStride;
                }
            }
        }
        else
        {
            for (int y = 0; y < imageHeight; ++y)
            {
                uint8* p = destData.getLinePointer (y);

                for (int x = 0; x < imageWidth; ++x)
                {
                    ((PixelRGB*) p)->desaturate();
                    p += destData.pixelStride;
                }
            }
        }
    }
}

void Image::createSolidAreaMask (RectangleList& result, const float alphaThreshold) const
{
    if (hasAlphaChannel())
    {
        const uint8 threshold = (uint8) jlimit (0, 255, roundToInt (alphaThreshold * 255.0f));
        SparseSet <int> pixelsOnRow;

        const BitmapData srcData (*this, 0, 0, getWidth(), getHeight());

        for (int y = 0; y < imageHeight; ++y)
        {
            pixelsOnRow.clear();
            const uint8* lineData = srcData.getLinePointer (y);

            if (isARGB())
            {
                for (int x = 0; x < imageWidth; ++x)
                {
                    if (((const PixelARGB*) lineData)->getAlpha() >= threshold)
                        pixelsOnRow.addRange (x, 1);

                    lineData += srcData.pixelStride;
                }
            }
            else
            {
                for (int x = 0; x < imageWidth; ++x)
                {
                    if (*lineData >= threshold)
                        pixelsOnRow.addRange (x, 1);

                    lineData += srcData.pixelStride;
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

        const BitmapData destData (*this, minX, minY, maxX - minX, maxY - minY, true);

        uint8* dst       = destData.getPixelPointer (dx - minX, dy - minY);
        const uint8* src = destData.getPixelPointer (sx - minX, sy - minY);

        const int lineSize = destData.pixelStride * w;

        if (dy > sy)
        {
            while (--h >= 0)
            {
                const int offset = h * destData.lineStride;
                memmove (dst + offset, src + offset, lineSize);
            }
        }
        else if (dst != src)
        {
            while (--h >= 0)
            {
                memmove (dst, src, lineSize);
                dst += destData.lineStride;
                src += destData.lineStride;
            }
        }
    }
}


END_JUCE_NAMESPACE
