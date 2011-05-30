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

#include "../../../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_Image.h"
#include "../contexts/juce_Graphics.h"
#include "../contexts/juce_LowLevelGraphicsSoftwareRenderer.h"
#include "../colour/juce_PixelFormats.h"
#include "../../../containers/juce_SparseSet.h"


//==============================================================================
Image::SharedImage::SharedImage (const PixelFormat format_, const int width_, const int height_)
    : format (format_), width (width_), height (height_)
{
    jassert (format_ == RGB || format_ == ARGB || format_ == SingleChannel);
    jassert (width > 0 && height > 0); // It's illegal to create a zero-sized image!
}

Image::SharedImage::~SharedImage()
{
}

//==============================================================================
class SoftwareSharedImage  : public Image::SharedImage
{
public:
    SoftwareSharedImage (const Image::PixelFormat format_, const int width_, const int height_, const bool clearImage)
        : Image::SharedImage (format_, width_, height_),
          pixelStride (format_ == Image::RGB ? 3 : ((format_ == Image::ARGB) ? 4 : 1)),
          lineStride ((pixelStride * jmax (1, width_) + 3) & ~3)
    {
        imageData.allocate (lineStride * jmax (1, height_), clearImage);
    }

    Image::ImageType getType() const
    {
        return Image::SoftwareImage;
    }

    LowLevelGraphicsContext* createLowLevelContext()
    {
        return new LowLevelGraphicsSoftwareRenderer (Image (this));
    }

    void initialiseBitmapData (Image::BitmapData& bitmap, int x, int y, Image::BitmapData::ReadWriteMode /*mode*/)
    {
        bitmap.data = imageData + x * pixelStride + y * lineStride;
        bitmap.pixelFormat = format;
        bitmap.lineStride = lineStride;
        bitmap.pixelStride = pixelStride;
    }

    Image::SharedImage* clone()
    {
        SoftwareSharedImage* s = new SoftwareSharedImage (format, width, height, false);
        memcpy (s->imageData, imageData, lineStride * height);
        return s;
    }

private:
    HeapBlock<uint8> imageData;
    const int pixelStride, lineStride;

    JUCE_LEAK_DETECTOR (SoftwareSharedImage);
};

Image::SharedImage* Image::SharedImage::createSoftwareImage (Image::PixelFormat format, int width, int height, bool clearImage)
{
    return new SoftwareSharedImage (format, width, height, clearImage);
}

//==============================================================================
class SubsectionSharedImage  : public Image::SharedImage
{
public:
    SubsectionSharedImage (Image::SharedImage* const image_, const Rectangle<int>& area_)
        : Image::SharedImage (image_->getPixelFormat(), area_.getWidth(), area_.getHeight()),
          image (image_), area (area_)
    {
    }

    Image::ImageType getType() const
    {
        return Image::SoftwareImage;
    }

    LowLevelGraphicsContext* createLowLevelContext()
    {
        LowLevelGraphicsContext* g = image->createLowLevelContext();
        g->clipToRectangle (area);
        g->setOrigin (area.getX(), area.getY());
        return g;
    }

    void initialiseBitmapData (Image::BitmapData& bitmap, int x, int y, Image::BitmapData::ReadWriteMode mode)
    {
        image->initialiseBitmapData (bitmap, x + area.getX(), y + area.getY(), mode);
    }

    Image::SharedImage* clone()
    {
        return new SubsectionSharedImage (image->clone(), area);
    }

private:
    const ReferenceCountedObjectPtr<Image::SharedImage> image;
    const Rectangle<int> area;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SubsectionSharedImage);
};

Image Image::getClippedImage (const Rectangle<int>& area) const
{
    if (area.contains (getBounds()))
        return *this;

    const Rectangle<int> validArea (area.getIntersection (getBounds()));
    if (validArea.isEmpty())
        return Image::null;

    return Image (new SubsectionSharedImage (image, validArea));
}


//==============================================================================
Image::Image()
{
}

Image::Image (SharedImage* const instance)
    : image (instance)
{
}

Image::Image (const PixelFormat format,
              const int width, const int height,
              const bool clearImage, const ImageType type)
    : image (type == Image::NativeImage ? SharedImage::createNativeImage (format, width, height, clearImage)
                                        : new SoftwareSharedImage (format, width, height, clearImage))
{
}

Image::Image (const Image& other)
   : image (other.image)
{
}

Image& Image::operator= (const Image& other)
{
    image = other.image;
    return *this;
}

Image::~Image()
{
}

const Image Image::null;

LowLevelGraphicsContext* Image::createLowLevelContext() const
{
    return image == nullptr ? nullptr : image->createLowLevelContext();
}

void Image::duplicateIfShared()
{
    if (image != nullptr && image->getReferenceCount() > 1)
        image = image->clone();
}

Image Image::rescaled (const int newWidth, const int newHeight, const Graphics::ResamplingQuality quality) const
{
    if (image == nullptr || (image->width == newWidth && image->height == newHeight))
        return *this;

    Image newImage (image->format, newWidth, newHeight, hasAlphaChannel(), image->getType());

    Graphics g (newImage);
    g.setImageResamplingQuality (quality);
    g.drawImage (*this, 0, 0, newWidth, newHeight, 0, 0, image->width, image->height, false);

    return newImage;
}

Image Image::convertedToFormat (PixelFormat newFormat) const
{
    if (image == nullptr || newFormat == image->format)
        return *this;

    const int w = image->width, h = image->height;
    Image newImage (newFormat, w, h, false, image->getType());

    if (newFormat == SingleChannel)
    {
        if (! hasAlphaChannel())
        {
            newImage.clear (getBounds(), Colours::black);
        }
        else
        {
            const BitmapData destData (newImage, 0, 0, w, h, BitmapData::writeOnly);
            const BitmapData srcData (*this, 0, 0, w, h);

            for (int y = 0; y < h; ++y)
            {
                const PixelARGB* src = (const PixelARGB*) srcData.getLinePointer(y);
                uint8* dst = destData.getLinePointer (y);

                for (int x = w; --x >= 0;)
                {
                    *dst++ = src->getAlpha();
                    ++src;
                }
            }
        }
    }
    else
    {
        if (hasAlphaChannel())
            newImage.clear (getBounds());

        Graphics g (newImage);
        g.drawImageAt (*this, 0, 0);
    }

    return newImage;
}

NamedValueSet* Image::getProperties() const
{
    return image == nullptr ? nullptr : &(image->userData);
}

//==============================================================================
Image::BitmapData::BitmapData (Image& image, const int x, const int y, const int w, const int h, BitmapData::ReadWriteMode mode)
    : width (w),
      height (h)
{
    // The BitmapData class must be given a valid image, and a valid rectangle within it!
    jassert (image.image != nullptr);
    jassert (x >= 0 && y >= 0 && w > 0 && h > 0 && x + w <= image.getWidth() && y + h <= image.getHeight());

    image.image->initialiseBitmapData (*this, x, y, mode);
    jassert (data != nullptr && pixelStride > 0 && lineStride != 0);
}

Image::BitmapData::BitmapData (const Image& image, const int x, const int y, const int w, const int h)
    : width (w),
      height (h)
{
    // The BitmapData class must be given a valid image, and a valid rectangle within it!
    jassert (image.image != nullptr);
    jassert (x >= 0 && y >= 0 && w > 0 && h > 0 && x + w <= image.getWidth() && y + h <= image.getHeight());

    image.image->initialiseBitmapData (*this, x, y, readOnly);
    jassert (data != nullptr && pixelStride > 0 && lineStride != 0);
}

Image::BitmapData::BitmapData (const Image& image, BitmapData::ReadWriteMode mode)
    : width (image.getWidth()),
      height (image.getHeight())
{
    // The BitmapData class must be given a valid image!
    jassert (image.image != nullptr);

    image.image->initialiseBitmapData (*this, 0, 0, mode);
    jassert (data != nullptr && pixelStride > 0 && lineStride != 0);
}

Image::BitmapData::~BitmapData()
{
}

const Colour Image::BitmapData::getPixelColour (const int x, const int y) const noexcept
{
    jassert (isPositiveAndBelow (x, width) && isPositiveAndBelow (y, height));

    const uint8* const pixel = getPixelPointer (x, y);

    switch (pixelFormat)
    {
        case Image::ARGB:           return Colour (((const PixelARGB*) pixel)->getUnpremultipliedARGB());
        case Image::RGB:            return Colour (((const PixelRGB*) pixel)->getUnpremultipliedARGB());
        case Image::SingleChannel:  return Colour (((const PixelAlpha*) pixel)->getUnpremultipliedARGB());
        default:                    jassertfalse; break;
    }

    return Colour();
}

void Image::BitmapData::setPixelColour (const int x, const int y, const Colour& colour) const noexcept
{
    jassert (isPositiveAndBelow (x, width) && isPositiveAndBelow (y, height));

    uint8* const pixel = getPixelPointer (x, y);
    const PixelARGB col (colour.getPixelARGB());

    switch (pixelFormat)
    {
        case Image::ARGB:           ((PixelARGB*) pixel)->set (col); break;
        case Image::RGB:            ((PixelRGB*) pixel)->set (col); break;
        case Image::SingleChannel:  *pixel = col.getAlpha(); break;
        default:                    jassertfalse; break;
    }
}

void Image::setPixelData (int x, int y, int w, int h,
                          const uint8* const sourcePixelData, const int sourceLineStride)
{
    jassert (x >= 0 && y >= 0 && w > 0 && h > 0 && x + w <= getWidth() && y + h <= getHeight());

    if (Rectangle<int>::intersectRectangles (x, y, w, h, 0, 0, getWidth(), getHeight()))
    {
        const BitmapData dest (*this, x, y, w, h, BitmapData::writeOnly);

        for (int i = 0; i < h; ++i)
        {
            memcpy (dest.getLinePointer(i),
                    sourcePixelData + sourceLineStride * i,
                    w * dest.pixelStride);
        }
    }
}

//==============================================================================
void Image::clear (const Rectangle<int>& area, const Colour& colourToClearTo)
{
    const Rectangle<int> clipped (area.getIntersection (getBounds()));

    if (! clipped.isEmpty())
    {
        const PixelARGB col (colourToClearTo.getPixelARGB());

        const BitmapData destData (*this, clipped.getX(), clipped.getY(), clipped.getWidth(), clipped.getHeight(), BitmapData::writeOnly);
        uint8* dest = destData.data;
        int dh = clipped.getHeight();

        while (--dh >= 0)
        {
            uint8* line = dest;
            dest += destData.lineStride;

            if (isARGB())
            {
                for (int x = clipped.getWidth(); --x >= 0;)
                {
                    ((PixelARGB*) line)->set (col);
                    line += destData.pixelStride;
                }
            }
            else if (isRGB())
            {
                for (int x = clipped.getWidth(); --x >= 0;)
                {
                    ((PixelRGB*) line)->set (col);
                    line += destData.pixelStride;
                }
            }
            else
            {
                for (int x = clipped.getWidth(); --x >= 0;)
                {
                    *line = col.getAlpha();
                    line += destData.pixelStride;
                }
            }
        }
    }
}

//==============================================================================
const Colour Image::getPixelAt (const int x, const int y) const
{
    if (isPositiveAndBelow (x, getWidth()) && isPositiveAndBelow (y, getHeight()))
    {
        const BitmapData srcData (*this, x, y, 1, 1);
        return srcData.getPixelColour (0, 0);
    }

    return Colour();
}

void Image::setPixelAt (const int x, const int y, const Colour& colour)
{
    if (isPositiveAndBelow (x, getWidth()) && isPositiveAndBelow (y, getHeight()))
    {
        const BitmapData destData (*this, x, y, 1, 1, BitmapData::writeOnly);
        destData.setPixelColour (0, 0, colour);
    }
}

void Image::multiplyAlphaAt (const int x, const int y, const float multiplier)
{
    if (isPositiveAndBelow (x, getWidth()) && isPositiveAndBelow (y, getHeight())
         && hasAlphaChannel())
    {
        const BitmapData destData (*this, x, y, 1, 1, BitmapData::readWrite);

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
        const BitmapData destData (*this, 0, 0, getWidth(), getHeight(), BitmapData::readWrite);

        if (isARGB())
        {
            for (int y = 0; y < destData.height; ++y)
            {
                uint8* p = destData.getLinePointer (y);

                for (int x = 0; x < destData.width; ++x)
                {
                    ((PixelARGB*) p)->multiplyAlpha (amountToMultiplyBy);
                    p += destData.pixelStride;
                }
            }
        }
        else
        {
            for (int y = 0; y < destData.height; ++y)
            {
                uint8* p = destData.getLinePointer (y);

                for (int x = 0; x < destData.width; ++x)
                {
                    *p = (uint8) (*p * amountToMultiplyBy);
                    p += destData.pixelStride;
                }
            }
        }
    }
    else
    {
        jassertfalse; // can't do this without an alpha-channel!
    }
}

void Image::desaturate()
{
    if (isARGB() || isRGB())
    {
        const BitmapData destData (*this, 0, 0, getWidth(), getHeight(), BitmapData::readWrite);

        if (isARGB())
        {
            for (int y = 0; y < destData.height; ++y)
            {
                uint8* p = destData.getLinePointer (y);

                for (int x = 0; x < destData.width; ++x)
                {
                    ((PixelARGB*) p)->desaturate();
                    p += destData.pixelStride;
                }
            }
        }
        else
        {
            for (int y = 0; y < destData.height; ++y)
            {
                uint8* p = destData.getLinePointer (y);

                for (int x = 0; x < destData.width; ++x)
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
        SparseSet<int> pixelsOnRow;

        const BitmapData srcData (*this, 0, 0, getWidth(), getHeight());

        for (int y = 0; y < srcData.height; ++y)
        {
            pixelsOnRow.clear();
            const uint8* lineData = srcData.getLinePointer (y);

            if (isARGB())
            {
                for (int x = 0; x < srcData.width; ++x)
                {
                    if (((const PixelARGB*) lineData)->getAlpha() >= threshold)
                        pixelsOnRow.addRange (Range<int> (x, x + 1));

                    lineData += srcData.pixelStride;
                }
            }
            else
            {
                for (int x = 0; x < srcData.width; ++x)
                {
                    if (*lineData >= threshold)
                        pixelsOnRow.addRange (Range<int> (x, x + 1));

                    lineData += srcData.pixelStride;
                }
            }

            for (int i = 0; i < pixelsOnRow.getNumRanges(); ++i)
            {
                const Range<int> range (pixelsOnRow.getRange (i));
                result.add (Rectangle<int> (range.getStart(), y, range.getLength(), 1));
            }

            result.consolidate();
        }
    }
    else
    {
        result.add (0, 0, getWidth(), getHeight());
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

        const BitmapData destData (*this, minX, minY, maxX - minX, maxY - minY, BitmapData::readWrite);

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
