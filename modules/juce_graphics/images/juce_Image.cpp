/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

ImagePixelData::ImagePixelData (Image::PixelFormat format, int w, int h)
    : pixelFormat (format), width (w), height (h)
{
    jassert (format == Image::RGB || format == Image::ARGB || format == Image::SingleChannel);
    jassert (w > 0 && h > 0); // It's illegal to create a zero-sized image!
}

ImagePixelData::~ImagePixelData()
{
    listeners.call ([this] (Listener& l) { l.imageDataBeingDeleted (this); });
}

void ImagePixelData::sendDataChangeMessage()
{
    listeners.call ([this] (Listener& l) { l.imageDataChanged (this); });
}

int ImagePixelData::getSharedCount() const noexcept
{
    return getReferenceCount();
}

//==============================================================================
ImageType::ImageType() {}
ImageType::~ImageType() {}

Image ImageType::convert (const Image& source) const
{
    if (source.isNull() || getTypeID() == (std::unique_ptr<ImageType> (source.getPixelData()->createType())->getTypeID()))
        return source;

    const Image::BitmapData src (source, Image::BitmapData::readOnly);

    Image newImage (create (src.pixelFormat, src.width, src.height, false));
    Image::BitmapData dest (newImage, Image::BitmapData::writeOnly);

    if (src.pixelStride == dest.pixelStride && src.pixelFormat == dest.pixelFormat)
    {
        for (int y = 0; y < dest.height; ++y)
            memcpy (dest.getLinePointer (y), src.getLinePointer (y), (size_t) dest.lineStride);
    }
    else
    {
        for (int y = 0; y < dest.height; ++y)
            for (int x = 0; x < dest.width; ++x)
                dest.setPixelColour (x, y, src.getPixelColour (x, y));
    }

    return newImage;
}

//==============================================================================
class SoftwarePixelData  : public ImagePixelData
{
public:
    SoftwarePixelData (Image::PixelFormat formatToUse, int w, int h, bool clearImage)
        : ImagePixelData (formatToUse, w, h),
          pixelStride (formatToUse == Image::RGB ? 3 : ((formatToUse == Image::ARGB) ? 4 : 1)),
          lineStride ((pixelStride * jmax (1, w) + 3) & ~3)
    {
        imageData.allocate ((size_t) lineStride * (size_t) jmax (1, h), clearImage);
    }

    LowLevelGraphicsContext* createLowLevelContext() override
    {
        sendDataChangeMessage();
        return new LowLevelGraphicsSoftwareRenderer (Image (*this));
    }

    void initialiseBitmapData (Image::BitmapData& bitmap, int x, int y, Image::BitmapData::ReadWriteMode mode) override
    {
        bitmap.data = imageData + (size_t) x * (size_t) pixelStride + (size_t) y * (size_t) lineStride;
        bitmap.pixelFormat = pixelFormat;
        bitmap.lineStride = lineStride;
        bitmap.pixelStride = pixelStride;

        if (mode != Image::BitmapData::readOnly)
            sendDataChangeMessage();
    }

    ImagePixelData::Ptr clone() override
    {
        auto s = new SoftwarePixelData (pixelFormat, width, height, false);
        memcpy (s->imageData, imageData, (size_t) lineStride * (size_t) height);
        return *s;
    }

    ImageType* createType() const override    { return new SoftwareImageType(); }

private:
    HeapBlock<uint8> imageData;
    const int pixelStride, lineStride;

    JUCE_LEAK_DETECTOR (SoftwarePixelData)
};

SoftwareImageType::SoftwareImageType() {}
SoftwareImageType::~SoftwareImageType() {}

ImagePixelData::Ptr SoftwareImageType::create (Image::PixelFormat format, int width, int height, bool clearImage) const
{
    return *new SoftwarePixelData (format, width, height, clearImage);
}

int SoftwareImageType::getTypeID() const
{
    return 2;
}

//==============================================================================
NativeImageType::NativeImageType() {}
NativeImageType::~NativeImageType() {}

int NativeImageType::getTypeID() const
{
    return 1;
}

#if JUCE_WINDOWS || JUCE_LINUX
ImagePixelData::Ptr NativeImageType::create (Image::PixelFormat format, int width, int height, bool clearImage) const
{
    return new SoftwarePixelData (format, width, height, clearImage);
}
#endif

//==============================================================================
class SubsectionPixelData  : public ImagePixelData
{
public:
    SubsectionPixelData (ImagePixelData::Ptr source, Rectangle<int> r)
        : ImagePixelData (source->pixelFormat, r.getWidth(), r.getHeight()),
          sourceImage (std::move (source)), area (r)
    {
    }

    LowLevelGraphicsContext* createLowLevelContext() override
    {
        LowLevelGraphicsContext* g = sourceImage->createLowLevelContext();
        g->clipToRectangle (area);
        g->setOrigin (area.getPosition());
        return g;
    }

    void initialiseBitmapData (Image::BitmapData& bitmap, int x, int y, Image::BitmapData::ReadWriteMode mode) override
    {
        sourceImage->initialiseBitmapData (bitmap, x + area.getX(), y + area.getY(), mode);

        if (mode != Image::BitmapData::readOnly)
            sendDataChangeMessage();
    }

    ImagePixelData::Ptr clone() override
    {
        jassert (getReferenceCount() > 0); // (This method can't be used on an unowned pointer, as it will end up self-deleting)
        const std::unique_ptr<ImageType> type (createType());

        Image newImage (type->create (pixelFormat, area.getWidth(), area.getHeight(), pixelFormat != Image::RGB));

        {
            Graphics g (newImage);
            g.drawImageAt (Image (*this), 0, 0);
        }

        return *newImage.getPixelData();
    }

    ImageType* createType() const override          { return sourceImage->createType(); }

    /* as we always hold a reference to image, don't double count */
    int getSharedCount() const noexcept override    { return getReferenceCount() + sourceImage->getSharedCount() - 1; }

private:
    friend class Image;
    const ImagePixelData::Ptr sourceImage;
    const Rectangle<int> area;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SubsectionPixelData)
};

Image Image::getClippedImage (const Rectangle<int>& area) const
{
    if (area.contains (getBounds()))
        return *this;

    auto validArea = area.getIntersection (getBounds());

    if (validArea.isEmpty())
        return {};

    return Image (*new SubsectionPixelData (image, validArea));
}


//==============================================================================
Image::Image() noexcept
{
}

Image::Image (ReferenceCountedObjectPtr<ImagePixelData> instance) noexcept
    : image (std::move (instance))
{
}

Image::Image (PixelFormat format, int width, int height, bool clearImage)
    : image (NativeImageType().create (format, width, height, clearImage))
{
}

Image::Image (PixelFormat format, int width, int height, bool clearImage, const ImageType& type)
    : image (type.create (format, width, height, clearImage))
{
}

Image::Image (const Image& other) noexcept
    : image (other.image)
{
}

Image& Image::operator= (const Image& other)
{
    image = other.image;
    return *this;
}

Image::Image (Image&& other) noexcept
    : image (std::move (other.image))
{
}

Image& Image::operator= (Image&& other) noexcept
{
    image = std::move (other.image);
    return *this;
}

Image::~Image()
{
}

JUCE_DECLARE_DEPRECATED_STATIC (const Image Image::null;)

int Image::getReferenceCount() const noexcept           { return image == nullptr ? 0 : image->getSharedCount(); }
int Image::getWidth() const noexcept                    { return image == nullptr ? 0 : image->width; }
int Image::getHeight() const noexcept                   { return image == nullptr ? 0 : image->height; }
Rectangle<int> Image::getBounds() const noexcept        { return image == nullptr ? Rectangle<int>() : Rectangle<int> (image->width, image->height); }
Image::PixelFormat Image::getFormat() const noexcept    { return image == nullptr ? UnknownFormat : image->pixelFormat; }
bool Image::isARGB() const noexcept                     { return getFormat() == ARGB; }
bool Image::isRGB() const noexcept                      { return getFormat() == RGB; }
bool Image::isSingleChannel() const noexcept            { return getFormat() == SingleChannel; }
bool Image::hasAlphaChannel() const noexcept            { return getFormat() != RGB; }

LowLevelGraphicsContext* Image::createLowLevelContext() const
{
    return image == nullptr ? nullptr : image->createLowLevelContext();
}

void Image::duplicateIfShared()
{
    if (getReferenceCount() > 1)
        image = image->clone();
}

Image Image::createCopy() const
{
    if (image != nullptr)
        return Image (image->clone());

    return {};
}

Image Image::rescaled (int newWidth, int newHeight, Graphics::ResamplingQuality quality) const
{
    if (image == nullptr || (image->width == newWidth && image->height == newHeight))
        return *this;

    const std::unique_ptr<ImageType> type (image->createType());
    Image newImage (type->create (image->pixelFormat, newWidth, newHeight, hasAlphaChannel()));

    Graphics g (newImage);
    g.setImageResamplingQuality (quality);
    g.drawImageTransformed (*this, AffineTransform::scale (newWidth  / (float) image->width,
                                                           newHeight / (float) image->height), false);
    return newImage;
}

Image Image::convertedToFormat (PixelFormat newFormat) const
{
    if (image == nullptr || newFormat == image->pixelFormat)
        return *this;

    auto w = image->width, h = image->height;

    const std::unique_ptr<ImageType> type (image->createType());
    Image newImage (type->create (newFormat, w, h, false));

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
                auto src = reinterpret_cast<const PixelARGB*> (srcData.getLinePointer (y));
                auto dst = destData.getLinePointer (y);

                for (int x = 0; x < w; ++x)
                    dst[x] = src[x].getAlpha();
            }
        }
    }
    else if (image->pixelFormat == SingleChannel && newFormat == Image::ARGB)
    {
        const BitmapData destData (newImage, 0, 0, w, h, BitmapData::writeOnly);
        const BitmapData srcData (*this, 0, 0, w, h);

        for (int y = 0; y < h; ++y)
        {
            auto src = reinterpret_cast<const PixelAlpha*> (srcData.getLinePointer (y));
            auto dst = reinterpret_cast<PixelARGB*> (destData.getLinePointer (y));

            for (int x = 0; x < w; ++x)
                dst[x].set (src[x]);
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
Image::BitmapData::BitmapData (Image& im, int x, int y, int w, int h, BitmapData::ReadWriteMode mode)
    : width (w), height (h)
{
    // The BitmapData class must be given a valid image, and a valid rectangle within it!
    jassert (im.image != nullptr);
    jassert (x >= 0 && y >= 0 && w > 0 && h > 0 && x + w <= im.getWidth() && y + h <= im.getHeight());

    im.image->initialiseBitmapData (*this, x, y, mode);
    jassert (data != nullptr && pixelStride > 0 && lineStride != 0);
}

Image::BitmapData::BitmapData (const Image& im, int x, int y, int w, int h)
    : width (w), height (h)
{
    // The BitmapData class must be given a valid image, and a valid rectangle within it!
    jassert (im.image != nullptr);
    jassert (x >= 0 && y >= 0 && w > 0 && h > 0 && x + w <= im.getWidth() && y + h <= im.getHeight());

    im.image->initialiseBitmapData (*this, x, y, readOnly);
    jassert (data != nullptr && pixelStride > 0 && lineStride != 0);
}

Image::BitmapData::BitmapData (const Image& im, BitmapData::ReadWriteMode mode)
    : width (im.getWidth()),
      height (im.getHeight())
{
    // The BitmapData class must be given a valid image!
    jassert (im.image != nullptr);

    im.image->initialiseBitmapData (*this, 0, 0, mode);
    jassert (data != nullptr && pixelStride > 0 && lineStride != 0);
}

Image::BitmapData::~BitmapData()
{
}

Colour Image::BitmapData::getPixelColour (int x, int y) const noexcept
{
    jassert (isPositiveAndBelow (x, width) && isPositiveAndBelow (y, height));

    auto pixel = getPixelPointer (x, y);

    switch (pixelFormat)
    {
        case Image::ARGB:           return Colour ( ((const PixelARGB*)  pixel)->getUnpremultiplied());
        case Image::RGB:            return Colour (*((const PixelRGB*)   pixel));
        case Image::SingleChannel:  return Colour (*((const PixelAlpha*) pixel));
        default:                    jassertfalse; break;
    }

    return {};
}

void Image::BitmapData::setPixelColour (int x, int y, Colour colour) const noexcept
{
    jassert (isPositiveAndBelow (x, width) && isPositiveAndBelow (y, height));

    auto pixel = getPixelPointer (x, y);
    auto col = colour.getPixelARGB();

    switch (pixelFormat)
    {
        case Image::ARGB:           ((PixelARGB*)  pixel)->set (col); break;
        case Image::RGB:            ((PixelRGB*)   pixel)->set (col); break;
        case Image::SingleChannel:  ((PixelAlpha*) pixel)->set (col); break;
        default:                    jassertfalse; break;
    }
}

//==============================================================================
void Image::clear (const Rectangle<int>& area, Colour colourToClearTo)
{
    if (image != nullptr)
    {
        const std::unique_ptr<LowLevelGraphicsContext> g (image->createLowLevelContext());
        g->setFill (colourToClearTo);
        g->fillRect (area, true);
    }
}

//==============================================================================
Colour Image::getPixelAt (int x, int y) const
{
    if (isPositiveAndBelow (x, getWidth()) && isPositiveAndBelow (y, getHeight()))
    {
        const BitmapData srcData (*this, x, y, 1, 1);
        return srcData.getPixelColour (0, 0);
    }

    return {};
}

void Image::setPixelAt (int x, int y, Colour colour)
{
    if (isPositiveAndBelow (x, getWidth()) && isPositiveAndBelow (y, getHeight()))
    {
        const BitmapData destData (*this, x, y, 1, 1, BitmapData::writeOnly);
        destData.setPixelColour (0, 0, colour);
    }
}

void Image::multiplyAlphaAt (int x, int y, float multiplier)
{
    if (isPositiveAndBelow (x, getWidth()) && isPositiveAndBelow (y, getHeight())
         && hasAlphaChannel())
    {
        const BitmapData destData (*this, x, y, 1, 1, BitmapData::readWrite);

        if (isARGB())
            reinterpret_cast<PixelARGB*> (destData.data)->multiplyAlpha (multiplier);
        else
            *(destData.data) = (uint8) (*(destData.data) * multiplier);
    }
}

template <class PixelType>
struct PixelIterator
{
    template <class PixelOperation>
    static void iterate (const Image::BitmapData& data, const PixelOperation& pixelOp)
    {
        for (int y = 0; y < data.height; ++y)
        {
            auto p = data.getLinePointer (y);

            for (int x = 0; x < data.width; ++x)
            {
                pixelOp (*reinterpret_cast<PixelType*> (p));
                p += data.pixelStride;
            }
        }
    }
};

template <class PixelOperation>
static void performPixelOp (const Image::BitmapData& data, const PixelOperation& pixelOp)
{
    switch (data.pixelFormat)
    {
        case Image::ARGB:           PixelIterator<PixelARGB> ::iterate (data, pixelOp); break;
        case Image::RGB:            PixelIterator<PixelRGB>  ::iterate (data, pixelOp); break;
        case Image::SingleChannel:  PixelIterator<PixelAlpha>::iterate (data, pixelOp); break;
        default:                    jassertfalse; break;
    }
}

struct AlphaMultiplyOp
{
    float alpha;

    template <class PixelType>
    void operator() (PixelType& pixel) const
    {
        pixel.multiplyAlpha (alpha);
    }
};

void Image::multiplyAllAlphas (float amountToMultiplyBy)
{
    jassert (hasAlphaChannel());

    const BitmapData destData (*this, 0, 0, getWidth(), getHeight(), BitmapData::readWrite);
    performPixelOp (destData, AlphaMultiplyOp { amountToMultiplyBy });
}

struct DesaturateOp
{
    template <class PixelType>
    void operator() (PixelType& pixel) const
    {
        pixel.desaturate();
    }
};

void Image::desaturate()
{
    if (isARGB() || isRGB())
    {
        const BitmapData destData (*this, 0, 0, getWidth(), getHeight(), BitmapData::readWrite);
        performPixelOp (destData, DesaturateOp());
    }
}

void Image::createSolidAreaMask (RectangleList<int>& result, float alphaThreshold) const
{
    if (hasAlphaChannel())
    {
        auto threshold = (uint8) jlimit (0, 255, roundToInt (alphaThreshold * 255.0f));
        SparseSet<int> pixelsOnRow;

        const BitmapData srcData (*this, 0, 0, getWidth(), getHeight());

        for (int y = 0; y < srcData.height; ++y)
        {
            pixelsOnRow.clear();
            auto lineData = srcData.getLinePointer (y);

            if (isARGB())
            {
                for (int x = 0; x < srcData.width; ++x)
                {
                    if (reinterpret_cast<const PixelARGB*> (lineData)->getAlpha() >= threshold)
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
                auto range = pixelsOnRow.getRange (i);
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

    w = jmin (w, getWidth()  - jmax (sx, dx));
    h = jmin (h, getHeight() - jmax (sy, dy));

    if (w > 0 && h > 0)
    {
        auto maxX = jmax (dx, sx) + w;
        auto maxY = jmax (dy, sy) + h;

        const BitmapData destData (*this, minX, minY, maxX - minX, maxY - minY, BitmapData::readWrite);

        auto dst = destData.getPixelPointer (dx - minX, dy - minY);
        auto src = destData.getPixelPointer (sx - minX, sy - minY);

        auto lineSize = (size_t) (destData.pixelStride * w);

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

} // namespace juce
