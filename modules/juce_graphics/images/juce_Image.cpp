/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

namespace BitmapDataDetail
{
    template <auto T>
    using FormatConstant = std::integral_constant<decltype (T), T>;

    using ARGB = FormatConstant<Image::PixelFormat::ARGB>;
    using RGB  = FormatConstant<Image::PixelFormat::RGB>;
    using A    = FormatConstant<Image::PixelFormat::SingleChannel>;

    static Colour getPixelColour (const uint8* pixel, A)
    {
        return Colour (*((const PixelAlpha*) pixel));
    }

    static Colour getPixelColour (const uint8* pixel, RGB)
    {
        return Colour (*((const PixelRGB*) pixel));
    }

    static Colour getPixelColour (const uint8* pixel, ARGB)
    {
        return Colour (((const PixelARGB*) pixel)->getUnpremultiplied());
    }

    static void setPixelColour (uint8* pixel, PixelARGB col, A)
    {
        ((PixelAlpha*) pixel)->set (col);
    }

    static void setPixelColour (uint8* pixel, PixelARGB col, RGB)
    {
        ((PixelRGB*) pixel)->set (col);
    }

    static void setPixelColour (uint8* pixel, PixelARGB col, ARGB)
    {
        ((PixelARGB*) pixel)->set (col);
    }

    using ConverterFn = void (*) (const Image::BitmapData& src, const Image::BitmapData& dst, int w, int h);

    template <typename From, typename To>
    static constexpr ConverterFn makeConverterFn (From, To)
    {
        struct GetPixel
        {
            explicit GetPixel (const Image::BitmapData& bd)
                : data (bd.data),
                  lineStride ((size_t) bd.lineStride),
                  pixelStride ((size_t) bd.pixelStride) {}

            uint8* operator() (int x, int y) const
            {
                return data + (size_t) y * (size_t) lineStride + (size_t) x * (size_t) pixelStride;
            }

            uint8* data;
            size_t lineStride, pixelStride;
        };

        return [] (const Image::BitmapData& src, const Image::BitmapData& dst, int w, int h)
        {
            const GetPixel getSrc { src }, getDst { dst };

            for (int y = 0; y < h; ++y)
            {
                for (int x = 0; x < w; ++x)
                {
                    const auto srcColour = getPixelColour (getSrc (x, y), From{});
                    setPixelColour (getDst (x, y), srcColour.getPixelARGB(), To{});
                }
            }
        };
    }

    template <typename From, typename... To>
    static constexpr auto makeConverterFns (From from, To... to)
    {
        return std::array<ConverterFn, sizeof... (To)> { makeConverterFn (from, to)... };
    }

    /** This structure holds a 2D array of function pointers.
        The structure is indexed by source format and destination format, where the function
        at that index will convert an entire BitmapData array between those two formats.

        This approach is designed to avoid branching, especially switch statements, from
        the inner loop of the conversion. At time of writing, compilers cannot automatically
        vectorise loops containing switch statements. Therefore, it's often faster to move
        switches or table lookups outside tight loops.

        This is the old, slow approach:

        @code
            for (int y = 0; y < dest.height; ++y)
                for (int x = 0; x < dest.width; ++x)
                    dest.setPixelColour (x, y, src.getPixelColour (x, y));
        @endcode

        This is a faster way to write the same thing:

        @code
            if (const auto* converter = converterFnTable.getConverterFor (src.pixelFormat, dest.pixelFormat))
                converter (src, dest, dest.width, dest.height);
        @endcode
    */
    template <typename... Formats>
    class ConverterFnTable
    {
    public:
        constexpr ConverterFn getConverterFor (Image::PixelFormat src, Image::PixelFormat dst) const
        {
            const auto srcIndex = toIndex (src, Formats{}...);
            const auto dstIndex = toIndex (dst, Formats{}...);

            if (srcIndex >= sizeof... (Formats) || dstIndex >= sizeof... (Formats))
                return nullptr;

            return table[srcIndex][dstIndex];
        }

    private:
        static size_t toIndex (Image::PixelFormat)
        {
            return 0;
        }

        template <typename Head, typename... Tail>
        static size_t toIndex (Image::PixelFormat src, Head head, Tail... tail)
        {
            return src == head() ? 0 : 1 + toIndex (src, tail...);
        }

        std::array<std::array<ConverterFn, sizeof... (Formats)>, sizeof... (Formats)> table
        {
            makeConverterFns (Formats{}, Formats{}...)...
        };
    };

    static bool convert (const Image::BitmapData& src, Image::BitmapData& dest)
    {
        if (std::tuple (src.width, src.height) != std::tuple (dest.width, dest.height))
            return false;

        static constexpr auto converterFnTable = ConverterFnTable<RGB, ARGB, A>{};

        if (src.pixelStride == dest.pixelStride && src.pixelFormat == dest.pixelFormat)
        {
            for (int y = 0; y < dest.height; ++y)
                memcpy (dest.getLinePointer (y), src.getLinePointer (y), (size_t) dest.pixelStride * (size_t) dest.width);
        }
        else
        {
            if (auto* converter = converterFnTable.getConverterFor (src.pixelFormat, dest.pixelFormat))
                converter (src, dest, dest.width, dest.height);
        }

        return true;
    }

    static Image convert (const Image::BitmapData& src, const ImageType& type)
    {
        Image result (type.create (src.pixelFormat, src.width, src.height, false));
        Image::BitmapData (result, Image::BitmapData::writeOnly).convertFrom (src);

        return result;
    }

    static void blurDataTriplets (uint8* d, int num, const int delta) noexcept
    {
        uint32 last = d[0];
        d[0] = (uint8) ((d[0] + d[delta] + 1) / 3);
        d += delta;

        num -= 2;

        do
        {
            const uint32 newLast = d[0];
            d[0] = (uint8) ((last + d[0] + d[delta] + 1) / 3);
            d += delta;
            last = newLast;
        }
        while (--num > 0);

        d[0] = (uint8) ((last + d[0] + 1) / 3);
    }

    static void blurSingleChannelImage (uint8* const data, const int w, const int h,
                                        const int lineStride, const int repetitions) noexcept
    {
        jassert (w > 2 && h > 2);

        for (int y = 0; y < h; ++y)
            for (int i = repetitions; --i >= 0;)
                blurDataTriplets (data + lineStride * y, w, 1);

        for (int x = 0; x < w; ++x)
            for (int i = repetitions; --i >= 0;)
                blurDataTriplets (data + x, h, lineStride);
    }

    template <class PixelType>
    struct PixelIterator
    {
        template <class PixelOperation>
        static void iterate (const Image::BitmapData& data, const PixelOperation& pixelOp)
        {
            for (int y = 0; y < data.height; ++y)
                for (int x = 0; x < data.width; ++x)
                    pixelOp (*reinterpret_cast<PixelType*> (data.getPixelPointer (x, y)));
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
            case Image::UnknownFormat:
            default:                    jassertfalse; break;
        }
    }
}

//==============================================================================
class SubsectionPixelData : public ImagePixelData
{
public:
    using Ptr = ReferenceCountedObjectPtr<SubsectionPixelData>;

    SubsectionPixelData (ImagePixelData::Ptr source, Rectangle<int> r)
        : ImagePixelData (source->pixelFormat, r.getWidth(), r.getHeight()),
          sourceImage (std::move (source)),
          area (r)
    {
    }

    Rectangle<int>      getSubsection()      const { return area; }
    ImagePixelData::Ptr getSourcePixelData() const { return sourceImage; }

    std::unique_ptr<LowLevelGraphicsContext> createLowLevelContext() override
    {
        auto g = sourceImage->createLowLevelContext();
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
        auto type = createType();
        auto result = type->create (pixelFormat, area.getWidth(), area.getHeight(), pixelFormat != Image::RGB);

        {
            Graphics g { Image { result } };
            g.drawImageAt (Image (*this), 0, 0);
        }

        return result;
    }

    std::unique_ptr<ImageType> createType() const override { return sourceImage->createType(); }

    void applySingleChannelBoxBlurEffectInArea (Rectangle<int> b, int radius) override
    {
        sourceImage->applySingleChannelBoxBlurEffectInArea (getIntersection (b), radius);
    }

    void applyGaussianBlurEffectInArea (Rectangle<int> b, float radius) override
    {
        sourceImage->applyGaussianBlurEffectInArea (getIntersection (b), radius);
    }

    void multiplyAllAlphasInArea (Rectangle<int> b, float amount) override
    {
        sourceImage->multiplyAllAlphasInArea (getIntersection (b), amount);
    }

    void desaturateInArea (Rectangle<int> b) override
    {
        sourceImage->desaturateInArea (getIntersection (b));
    }

    /* as we always hold a reference to image, don't double count */
    int getSharedCount() const noexcept override { return getReferenceCount() + sourceImage->getSharedCount() - 1; }

    NativeExtensions getNativeExtensions() override
    {
        struct Wrapped
        {
            explicit Wrapped (Ptr selfIn)
                : self (selfIn) {}

           #if JUCE_WINDOWS
            Span<const Direct2DPixelDataPage> getPages (ComSmartPtr<ID2D1Device1> x) const
            {
                return self->sourceImage->getNativeExtensions().getPages (x);
            }
           #endif

           #if JUCE_MAC || JUCE_IOS
            CGContextRef getCGContext() const
            {
                return self->sourceImage->getNativeExtensions().getCGContext();
            }

            CFUniquePtr<CGImageRef> getCGImage (CGColorSpaceRef colourSpace) const
            {
                const auto& parentNative = self->sourceImage->getNativeExtensions();
                const auto parentImage = parentNative.getCGImage (colourSpace);
                return CFUniquePtr<CGImageRef> { CGImageCreateWithImageInRect (parentImage.get(),
                                                                               makeCGRect (self->area + parentNative.getTopLeft())) };
            }
           #endif

            Point<int> getTopLeft() const
            {
                return self->sourceImage->getNativeExtensions().getTopLeft() + self->area.getTopLeft();
            }

            Ptr self;
        };

        return NativeExtensions { Wrapped { this } };
    }

private:
    Rectangle<int> getIntersection (Rectangle<int> b) const
    {
        return area.getIntersection (b + area.getTopLeft());
    }

    friend class Image;
    const ImagePixelData::Ptr sourceImage;
    const Rectangle<int> area;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SubsectionPixelData)
};

//==============================================================================
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

void ImagePixelData::applySingleChannelBoxBlurEffectInArea (Rectangle<int> bounds, int radius)
{
    if (pixelFormat == Image::SingleChannel)
    {
        const Image::BitmapData bm (Image { this }, bounds, Image::BitmapData::readWrite);
        BitmapDataDetail::blurSingleChannelImage (bm.data, bm.width, bm.height, bm.lineStride, 2 * radius);
    }
}

void ImagePixelData::applyGaussianBlurEffectInArea (Rectangle<int> bounds, float radius)
{
    ImageConvolutionKernel blurKernel (roundToInt (radius * 2.0f));
    blurKernel.createGaussianBlur (radius);

    Image target { this };
    blurKernel.applyToImage (target, Image { this }.createCopy(), bounds);
}

void ImagePixelData::multiplyAllAlphasInArea (Rectangle<int> b, float amount)
{
    if (pixelFormat == Image::ARGB || pixelFormat == Image::SingleChannel)
    {
        const Image::BitmapData destData (Image { this }, b, Image::BitmapData::readWrite);
        BitmapDataDetail::performPixelOp (destData, [&] (auto& p) { p.multiplyAlpha (amount); });
    }
}

void ImagePixelData::desaturateInArea (Rectangle<int> b)
{
    if (pixelFormat == Image::ARGB || pixelFormat == Image::RGB)
    {
        const Image::BitmapData destData (Image { this }, b, Image::BitmapData::readWrite);
        BitmapDataDetail::performPixelOp (destData, [] (auto& p) { p.desaturate(); });
    }
}

auto ImagePixelData::getNativeExtensions() -> NativeExtensions
{
    struct Wrapped
    {
        Point<int> getTopLeft() const { return {}; }

       #if JUCE_WINDOWS
        Span<const Direct2DPixelDataPage> getPages (ComSmartPtr<ID2D1Device1>) const { return {}; }
       #endif

       #if JUCE_MAC || JUCE_IOS
        CGContextRef getCGContext() const { return {}; }
        CFUniquePtr<CGImageRef> getCGImage (CGColorSpaceRef) const { return {}; }
       #endif
    };

    return NativeExtensions { Wrapped{} };
}

struct MoveImageParams
{
    Rectangle<int> src{};
    Point<int> dst{};

    MoveImageParams constrained (int width, int height) const
    {
        const auto intersectedSrc = src.getIntersection ({ width, height });
        const auto srcOffset = intersectedSrc.getPosition() - src.getPosition();
        const auto intersectedDst = intersectedSrc.withPosition (dst + srcOffset).getIntersection ({ width, height });

        if (intersectedDst.isEmpty())
            return {};

        const MoveImageParams result { intersectedDst.withPosition (intersectedDst.getPosition() + src.getPosition() - dst),
                                       intersectedDst.getPosition() };

        // postconditions
        jassert ((juce::Rectangle { width, height }.contains (result.src)));
        jassert ((juce::Rectangle { width, height }.contains (result.src.withPosition (result.dst))));

        return result;
    }

    bool operator== (const MoveImageParams& other) const
    {
        const auto tie = [] (auto& x) { return std::tuple (x.src, x.dst); };
        return tie (*this) == tie (other);
    }

    bool operator!= (const MoveImageParams& other) const
    {
        return ! operator== (other);
    }
};

static void moveValidatedImageSectionInSoftware (ImagePixelData& img,
                                                 Point<int> destTopLeft,
                                                 Rectangle<int> sourceRect)
{
    const auto minX = jmin (destTopLeft.x, sourceRect.getX());
    const auto minY = jmin (destTopLeft.y, sourceRect.getY());

    Image tempImage { &img };
    const Image::BitmapData destData (tempImage,
                                      minX,
                                      minY,
                                      sourceRect.getWidth(),
                                      sourceRect.getHeight(),
                                      Image::BitmapData::readWrite);

    const auto dstPos = destTopLeft - Point { minX, minY };
    const auto srcPos = sourceRect.getPosition() - Point { minX, minY };

          auto* dst = destData.getPixelPointer (dstPos.x, dstPos.y);
    const auto* src = destData.getPixelPointer (srcPos.x, srcPos.y);

    const auto lineSize = (size_t) destData.pixelStride * (size_t) sourceRect.getWidth();

    if (destTopLeft.y > sourceRect.getY())
    {
        for (auto h = sourceRect.getHeight(); --h >= 0;)
        {
            const auto offset = h * destData.lineStride;
            memmove (dst + offset, src + offset, lineSize);
        }
    }
    else if (dst != src)
    {
        for (auto h = sourceRect.getHeight(); --h >= 0;)
        {
            memmove (dst, src, lineSize);
            dst += destData.lineStride;
            src += destData.lineStride;
        }
    }
}

void ImagePixelData::moveImageSection (Point<int> destTopLeft, Rectangle<int> sourceRect)
{
    const auto constrained = MoveImageParams { sourceRect, destTopLeft }.constrained (width, height);

    if (! constrained.src.isEmpty())
        moveValidatedImageSection (constrained.dst, constrained.src);
}

void ImagePixelData::moveValidatedImageSection (Point<int> destTopLeft, Rectangle<int> sourceRect)
{
    moveValidatedImageSectionInSoftware (*this, destTopLeft, sourceRect);
}

//==============================================================================
ImageType::ImageType() = default;
ImageType::~ImageType() = default;

Image ImageType::convert (const Image& source) const
{
    if (source.isNull() || getTypeID() == source.getPixelData()->createType()->getTypeID())
        return source;

    const Image::BitmapData src (source, Image::BitmapData::readOnly);

    if (src.data == nullptr)
        return {};

    return BitmapDataDetail::convert (src, *this);
}

//==============================================================================
class SoftwarePixelData : public ImagePixelData
{
public:
    SoftwarePixelData (Image::PixelFormat formatToUse, int w, int h, bool clearImage)
        : ImagePixelData (formatToUse, w, h),
          pixelStride (formatToUse == Image::RGB ? 3 : ((formatToUse == Image::ARGB) ? 4 : 1)),
          lineStride ((pixelStride * jmax (1, w) + 3) & ~3)
    {
        imageData.allocate ((size_t) lineStride * (size_t) jmax (1, h), clearImage);
    }

    std::unique_ptr<LowLevelGraphicsContext> createLowLevelContext() override
    {
        sendDataChangeMessage();
        return std::make_unique<LowLevelGraphicsSoftwareRenderer> (Image (*this));
    }

    void initialiseBitmapData (Image::BitmapData& bitmap, int x, int y, Image::BitmapData::ReadWriteMode mode) override
    {
        const auto offset = (size_t) x * (size_t) pixelStride + (size_t) y * (size_t) lineStride;
        bitmap.data = imageData + offset;
        bitmap.size = (size_t) (height * lineStride) - offset;
        bitmap.pixelFormat = pixelFormat;
        bitmap.lineStride = lineStride;
        bitmap.pixelStride = pixelStride;

        if (mode != Image::BitmapData::readOnly)
            sendDataChangeMessage();
    }

    Ptr clone() override
    {
        auto s = new SoftwarePixelData (pixelFormat, width, height, false);
        memcpy (s->imageData, imageData, (size_t) lineStride * (size_t) height);
        return *s;
    }

    std::unique_ptr<ImageType> createType() const override    { return std::make_unique<SoftwareImageType>(); }

private:
    HeapBlock<uint8> imageData;
    const int pixelStride, lineStride;

    JUCE_LEAK_DETECTOR (SoftwarePixelData)
};

SoftwareImageType::SoftwareImageType() = default;
SoftwareImageType::~SoftwareImageType() = default;

ImagePixelData::Ptr SoftwareImageType::create (Image::PixelFormat format, int width, int height, bool clearImage) const
{
    return *new SoftwarePixelData (format, width, height, clearImage);
}

int SoftwareImageType::getTypeID() const
{
    return 2;
}

//==============================================================================
NativeImageType::NativeImageType() = default;
NativeImageType::~NativeImageType() = default;

int NativeImageType::getTypeID() const
{
    return 1;
}

#if JUCE_LINUX || JUCE_BSD
ImagePixelData::Ptr NativeImageType::create (Image::PixelFormat format, int width, int height, bool clearImage) const
{
    return new SoftwarePixelData (format, width, height, clearImage);
}
#endif

//==============================================================================

Image Image::getClippedImage (const Rectangle<int>& area) const
{
    if (area.contains (getBounds()))
        return *this;

    auto validArea = area.getIntersection (getBounds());

    if (validArea.isEmpty())
        return {};

    return Image { ImagePixelData::Ptr { new SubsectionPixelData { image, validArea } } };
}

//==============================================================================
Image::Image() noexcept = default;

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

Image::~Image() = default;

int Image::getReferenceCount() const noexcept           { return image == nullptr ? 0 : image->getSharedCount(); }

bool Image::isValid() const noexcept
{
    return image != nullptr;
}

int Image::getWidth() const noexcept                    { return image == nullptr ? 0 : image->width; }
int Image::getHeight() const noexcept                   { return image == nullptr ? 0 : image->height; }
Rectangle<int> Image::getBounds() const noexcept        { return image == nullptr ? Rectangle<int>() : Rectangle<int> (image->width, image->height); }
Image::PixelFormat Image::getFormat() const noexcept    { return image == nullptr ? UnknownFormat : image->pixelFormat; }
bool Image::isARGB() const noexcept                     { return getFormat() == ARGB; }
bool Image::isRGB() const noexcept                      { return getFormat() == RGB; }
bool Image::isSingleChannel() const noexcept            { return getFormat() == SingleChannel; }
bool Image::hasAlphaChannel() const noexcept            { return getFormat() != RGB; }

std::unique_ptr<LowLevelGraphicsContext> Image::createLowLevelContext() const
{
    if (image != nullptr)
        return image->createLowLevelContext();

    return {};
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

    auto type = image->createType();
    Image newImage (type->create (image->pixelFormat, newWidth, newHeight, hasAlphaChannel()));

    Graphics g (newImage);
    g.setImageResamplingQuality (quality);
    g.drawImageTransformed (*this, AffineTransform::scale ((float) newWidth  / (float) image->width,
                                                           (float) newHeight / (float) image->height), false);
    return newImage;
}

Image Image::convertedToFormat (PixelFormat newFormat) const
{
    if (image == nullptr || newFormat == image->pixelFormat)
        return *this;

    auto w = image->width, h = image->height;

    auto type = image->createType();
    Image newImage (type->create (newFormat, w, h, false));

    if (newImage.getFormat() == SingleChannel)
    {
        if (! hasAlphaChannel())
        {
            newImage.clear (getBounds(), Colours::black);
        }
        else
        {
            const BitmapData destData (newImage, { w, h }, BitmapData::writeOnly);
            const BitmapData srcData (*this, { w, h }, BitmapData::readOnly);

            for (int y = 0; y < h; ++y)
            {
                for (int x = 0; x < w; ++x)
                {
                    auto* dstPtr = reinterpret_cast<PixelAlpha*> (destData.getPixelPointer (x, y));
                    auto* srcPtr = reinterpret_cast<const PixelARGB*> (srcData.getPixelPointer (x, y));
                    dstPtr->set (*srcPtr);
                }
            }
        }
    }
    else if (image->pixelFormat == SingleChannel && newImage.getFormat() == ARGB)
    {
        const BitmapData destData (newImage, { w, h }, BitmapData::writeOnly);
        const BitmapData srcData (*this, { w, h }, BitmapData::readOnly);

        for (int y = 0; y < h; ++y)
        {
            for (int x = 0; x < w; ++x)
            {
                auto* dstPtr = reinterpret_cast<PixelARGB*> (destData.getPixelPointer (x, y));
                auto* srcPtr = reinterpret_cast<const PixelAlpha*> (srcData.getPixelPointer (x, y));
                dstPtr->set (*srcPtr);
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
Image::BitmapData::BitmapData (Image& im, int x, int y, int w, int h, ReadWriteMode mode)
    : BitmapData (im, { x, y, w, h }, mode)
{
}

Image::BitmapData::BitmapData (const Image& im, Rectangle<int> bounds, ReadWriteMode mode)
    : width (bounds.getWidth()), height (bounds.getHeight())
{
    // The BitmapData class must be given a valid image, and a valid rectangle within it!
    jassert (im.image != nullptr);
    jassert (bounds.getX() >= 0);
    jassert (bounds.getY() >= 0);
    jassert (bounds.getWidth() > 0);
    jassert (bounds.getHeight() > 0);
    jassert (bounds.getRight() <= im.getWidth());
    jassert (bounds.getBottom() <= im.getHeight());

    im.image->initialiseBitmapData (*this, bounds.getX(), bounds.getY(), mode);
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

Image::BitmapData::BitmapData (const Image& im, ReadWriteMode mode)
    : width (im.getWidth()),
      height (im.getHeight())
{
    // The BitmapData class must be given a valid image!
    jassert (im.image != nullptr);

    im.image->initialiseBitmapData (*this, 0, 0, mode);
    jassert (data != nullptr && pixelStride > 0 && lineStride != 0);
}

Colour Image::BitmapData::getPixelColour (int x, int y) const noexcept
{
    auto* pixel = getPixelPointer (x, y);

    switch (pixelFormat)
    {
        case ARGB:           return BitmapDataDetail::getPixelColour (pixel, BitmapDataDetail::ARGB{});
        case RGB:            return BitmapDataDetail::getPixelColour (pixel, BitmapDataDetail::RGB{});
        case SingleChannel:  return BitmapDataDetail::getPixelColour (pixel, BitmapDataDetail::A{});
        case UnknownFormat:
        default:             jassertfalse; break;
    }

    return {};
}

void Image::BitmapData::setPixelColour (int x, int y, Colour colour) const noexcept
{
    auto* pixel = getPixelPointer (x, y);
    auto col = colour.getPixelARGB();

    switch (pixelFormat)
    {
        case ARGB:           return BitmapDataDetail::setPixelColour (pixel, col, BitmapDataDetail::ARGB{});
        case RGB:            return BitmapDataDetail::setPixelColour (pixel, col, BitmapDataDetail::RGB{});
        case SingleChannel:  return BitmapDataDetail::setPixelColour (pixel, col, BitmapDataDetail::A{});
        case UnknownFormat:
        default:             jassertfalse; break;
    }
}

bool Image::BitmapData::convertFrom (const BitmapData& source)
{
    return BitmapDataDetail::convert (source, *this);
}

bool Image::setBackupEnabled (bool enabled)
{
    if (auto ptr = image)
    {
        if (auto* ext = ptr->getBackupExtensions())
        {
            ext->setBackupEnabled (enabled);
            return true;
        }
    }

    return false;
}

//==============================================================================
void Image::clear (const Rectangle<int>& area, Colour colourToClearTo)
{
    if (image == nullptr)
        return;

    auto g = image->createLowLevelContext();
    g->setFill (colourToClearTo);
    g->fillRect (area, true);
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

void Image::multiplyAllAlphas (float amountToMultiplyBy)
{
    if (auto ptr = image)
        ptr->multiplyAllAlphas (amountToMultiplyBy);
}

void Image::desaturate()
{
    if (auto ptr = image)
        ptr->desaturate();
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

void Image::moveImageSection (int dx, int dy, int sx, int sy, int w, int h)
{
    if (image != nullptr)
        image->moveImageSection ({ dx, dy }, { sx, sy, w, h });
}

//==============================================================================
#if JUCE_ALLOW_STATIC_NULL_VARIABLES

JUCE_BEGIN_IGNORE_DEPRECATION_WARNINGS

const Image Image::null;

JUCE_END_IGNORE_DEPRECATION_WARNINGS

#endif

#if JUCE_UNIT_TESTS

class ImagePixelDataClippingTests : public UnitTest
{
public:
    ImagePixelDataClippingTests()
        : UnitTest ("ImagePixelDataClippingTests", UnitTestCategories::graphics)
    {
    }

    void runTest() override
    {
        beginTest ("MoveImageParams constrains arguments appropriately");
        {
            expect ((MoveImageParams { { 300, 400 }, { 5, 5 } }.constrained (350, 450) == MoveImageParams { { 300, 400 }, { 5, 5 } }));
            expect ((MoveImageParams { { 350, 450 }, { 5, 5 } }.constrained (300, 400) == MoveImageParams { { 295, 395 }, { 5, 5 } }));
            expect ((MoveImageParams { { -5, -10, 20, 30 }, { 0, 0 } }.constrained (100, 100) == MoveImageParams { { 15, 20 }, { 5, 10 } }));
            expect ((MoveImageParams { { 1, 2, 10, 10 }, { -5, -5 } }.constrained (20, 20) == MoveImageParams { { 6, 7, 5, 5 }, { 0, 0 } }));
            expect ((MoveImageParams { { 40, 50, 100, 100 }, { 10, 10 } }.constrained (100, 100) == MoveImageParams { { 40, 50, 60, 50 }, { 10, 10 } }));
            expect ((MoveImageParams { { 20, 20, 10, 10 }, { -20, -20 } }.constrained (20, 20) == MoveImageParams { { 0, 0, 0, 0 }, { 0, 0 } }));
            expect ((MoveImageParams { { -20, -30, 100, 100 }, { -30, -40 } }.constrained (100, 100) == MoveImageParams { { 10, 10, 70, 60 }, { 0, 0 } }));
        }
    }
};

static ImagePixelDataClippingTests imagePixelDataClippingTests;

#endif

} // namespace juce
