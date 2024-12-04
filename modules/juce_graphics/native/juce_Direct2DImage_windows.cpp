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

Rectangle<int> Direct2DPixelDataPage::getBounds() const
{
    return bitmap != nullptr ? D2DUtilities::rectFromSize (bitmap->GetPixelSize()).withPosition (topLeft)
                             : Rectangle<int>{};
}

//==============================================================================
static ComSmartPtr<ID2D1Device1> getDeviceForContext (ComSmartPtr<ID2D1DeviceContext1> context)
{
    if (context == nullptr)
        return {};

    ComSmartPtr<ID2D1Device> device;
    context->GetDevice (device.resetAndGetPointerAddress());
    return device.getInterface<ID2D1Device1>();
}

static std::vector<Direct2DPixelDataPage> makePages (ComSmartPtr<ID2D1Device1> device,
                                                     ImagePixelData::Ptr backingData,
                                                     bool needsClear)
{
    if (device == nullptr || backingData == nullptr)
    {
        jassertfalse;
        return {};
    }

    // We create a new context rather than reusing an existing one, because we'll run into problems
    // if we call BeginDraw/EndDraw on a context that's already doing its own drawing
    const auto context = Direct2DDeviceContext::create (device);

    if (context == nullptr)
    {
        jassertfalse;
        return {};
    }

    const auto maxDim = (size_t) context->GetMaximumBitmapSize();
    std::vector<Direct2DPixelDataPage> result;

    const auto width = (size_t) backingData->width;
    const auto height = (size_t) backingData->height;
    const auto pixelFormat = backingData->pixelFormat;

    for (size_t h = 0; h < height; h += maxDim)
    {
        const auto tileHeight = (UINT32) jmin (maxDim, height - h);

        for (size_t w = 0; w < width; w += maxDim)
        {
            const auto tileWidth = (UINT32) jmin (maxDim, width - w);

            const auto bitmap = Direct2DBitmap::createBitmap (context,
                                                              pixelFormat,
                                                              D2D1::SizeU (tileWidth, tileHeight),
                                                              D2D1_BITMAP_OPTIONS_TARGET);

            jassert (bitmap != nullptr);

            if (needsClear)
            {
                context->SetTarget (bitmap);
                context->BeginDraw();
                context->Clear();
                context->EndDraw();
            }

            result.push_back (Direct2DPixelDataPage { bitmap, { (int) w, (int) h } });
        }
    }

    return result;
}

/*  Maps the content of the provided bitmap and copies it into target, which should be a software
    bitmap.
*/
static bool readFromDirect2DBitmap (ComSmartPtr<ID2D1DeviceContext1> context,
                                    ComSmartPtr<ID2D1Bitmap1> bitmap,
                                    ImagePixelData::Ptr target)
{
    if (bitmap == nullptr || context == nullptr || target == nullptr)
        return false;

    const auto size = bitmap->GetPixelSize();

    if (std::tuple (target->width, target->height) != std::tuple ((int) size.width, (int) size.height))
    {
        // Mismatched sizes, unable to read D2D image back into software image!
        jassertfalse;
        return false;
    }

    const auto readableBitmap = Direct2DBitmap::createBitmap (context,
                                                              target->pixelFormat,
                                                              size,
                                                              D2D1_BITMAP_OPTIONS_CPU_READ | D2D1_BITMAP_OPTIONS_CANNOT_DRAW);

    const auto dstPoint = D2D1::Point2U();
    const auto srcRect = D2DUtilities::toRECT_U (D2DUtilities::rectFromSize (size));
    readableBitmap->CopyFromBitmap (&dstPoint, bitmap, &srcRect);

    // This is only used to construct a read-only BitmapData backed by a texture for conversion to a
    // software image
    struct TexturePixelData : public ImagePixelData
    {
        TexturePixelData (ComSmartPtr<ID2D1Bitmap1> bitmapIn, Image::PixelFormat format, int w, int h)
            : ImagePixelData (format, w, h),
              bitmap (bitmapIn)
        {
        }

        std::unique_ptr<LowLevelGraphicsContext> createLowLevelContext() override
        {
            jassertfalse; // This should never be called
            return {};
        }

        Ptr clone() override
        {
            jassertfalse; // This should never be called
            return {};
        }

        std::unique_ptr<ImageType> createType() const override
        {
            jassertfalse; // This should never be called
            return {};
        }

        void initialiseBitmapData (Image::BitmapData& bd, int x, int y, Image::BitmapData::ReadWriteMode mode) override
        {
            if (mode != Image::BitmapData::readOnly)
            {
                // This type only supports read-only access
                jassertfalse;
                return;
            }

            struct Releaser : public Image::BitmapData::BitmapDataReleaser
            {
                explicit Releaser (ComSmartPtr<ID2D1Bitmap1> toUnmapIn) : toUnmap (toUnmapIn) {}
                ~Releaser() override { toUnmap->Unmap(); }
                ComSmartPtr<ID2D1Bitmap1> toUnmap;
            };

            D2D1_MAPPED_RECT mapped{};
            bitmap->Map (D2D1_MAP_OPTIONS_READ, &mapped);
            const auto dataEnd = mapped.bits + bitmap->GetPixelSize().height * mapped.pitch;

            bd.pixelFormat = pixelFormat;
            bd.pixelStride = pixelFormat == Image::SingleChannel ? 1 : 4;
            bd.lineStride = (int) mapped.pitch;
            bd.data = mapped.bits + x * bd.pixelStride + y * (int) mapped.pitch;
            bd.size = (size_t) (dataEnd - bd.data);
            bd.dataReleaser = std::make_unique<Releaser> (bitmap);
        }

        ComSmartPtr<ID2D1Bitmap1> bitmap;
    };

    Image srcImage { new TexturePixelData { readableBitmap,
                                            target->pixelFormat,
                                            (int) size.width,
                                            (int) size.height } };
    Image::BitmapData srcBitmap { srcImage, Image::BitmapData::readOnly };
    Image::BitmapData dstBitmap { Image { target }, Image::BitmapData::writeOnly };
    BitmapDataDetail::convert (srcBitmap, dstBitmap);

    return true;
}

/*  Returns new software bitmap storage with content matching the provided hardware bitmap. */
static ImagePixelData::Ptr readFromDirect2DBitmap (ComSmartPtr<ID2D1DeviceContext1> context,
                                                   ComSmartPtr<ID2D1Bitmap1> bitmap)
{
    if (bitmap == nullptr)
        return {};

    const auto size = bitmap->GetPixelSize();
    const auto result = SoftwareImageType{}.create (Image::ARGB,
                                                    (int) size.width,
                                                    (int) size.height,
                                                    false);

    if (result == nullptr || ! readFromDirect2DBitmap (context, bitmap, result))
        return {};

    return result;
}

Direct2DPixelDataPages::Direct2DPixelDataPages (ComSmartPtr<ID2D1Bitmap1> bitmap,
                                                ImagePixelData::Ptr image)
    : backingData (image),
      pages { Page { bitmap, {} } },
      upToDate (true)
{
    // The backup image must be a software image
    jassert (image->createType()->getTypeID() == SoftwareImageType{}.getTypeID());
}

Direct2DPixelDataPages::Direct2DPixelDataPages (ComSmartPtr<ID2D1DeviceContext1> context,
                                                ImagePixelData::Ptr image,
                                                State initialState)
    : backingData (image),
      pages (makePages (getDeviceForContext (context), backingData, initialState == State::cleared)),
      upToDate (initialState != State::unsuitableToRead)
{
    // The backup image must be a software image
    jassert (image->createType()->getTypeID() == SoftwareImageType{}.getTypeID());
}

auto Direct2DPixelDataPages::getPages() -> Span<const Page>
{
    if (std::exchange (upToDate, true))
        return pages;

    auto sourceToUse = backingData->pixelFormat == Image::RGB
                     ? Image { backingData }.convertedToFormat (Image::ARGB)
                     : Image { backingData };

    for (const auto& page : pages)
    {
        const auto pageBounds = page.getBounds();
        const Image::BitmapData bitmapData { sourceToUse,
                                             pageBounds.getX(),
                                             pageBounds.getY(),
                                             pageBounds.getWidth(),
                                             pageBounds.getHeight(),
                                             Image::BitmapData::readOnly };

        const auto target = D2DUtilities::toRECT_U (pageBounds.withZeroOrigin());
        const auto hr = page.bitmap->CopyFromMemory (&target, bitmapData.data, (UINT32) bitmapData.lineStride);
        jassertquiet (SUCCEEDED (hr));
    }

    return pages;
}

//==============================================================================
Direct2DPixelData::Direct2DPixelData (ImagePixelData::Ptr ptr, State initialState)
    : ImagePixelData { ptr->pixelFormat, ptr->width, ptr->height },
      backingData (ptr),
      state (initialState)
{
    jassert (backingData->createType()->getTypeID() == SoftwareImageType{}.getTypeID());
    directX->adapters.addListener (*this);
}

Direct2DPixelData::Direct2DPixelData (ComSmartPtr<ID2D1DeviceContext1> context,
                                      ComSmartPtr<ID2D1Bitmap1> page)
    : Direct2DPixelData (readFromDirect2DBitmap (context, page), State::drawn)
{
    if (const auto device1 = getDeviceForContext (context))
        pagesForDevice.emplace (device1, Direct2DPixelDataPages { page, backingData });
}

Direct2DPixelData::Direct2DPixelData (Image::PixelFormat formatToUse, int w, int h, bool clearIn)
    : Direct2DPixelData { SoftwareImageType{}.create (formatToUse, w, h, clearIn),
                          clearIn ? State::initiallyCleared : State::initiallyUndefined }
{
}

Direct2DPixelData::~Direct2DPixelData()
{
    directX->adapters.removeListener (*this);
}

auto Direct2DPixelData::getIteratorForContext (ComSmartPtr<ID2D1DeviceContext1> context)
{
    const auto device1 = getDeviceForContext (context);

    if (device1 == nullptr)
        return pagesForDevice.end();

    const auto iter = pagesForDevice.find (device1);

    if (iter != pagesForDevice.end())
        return iter;

    const auto initialState = [&]
    {
        switch (state)
        {
            // If our image is currently cleared, then the initial state of the page should also
            // be cleared.
            case State::initiallyCleared:
                return Pages::State::cleared;

            // If our image holds junk, then it must be written before first read, which means
            // that the cached pages must also be written before first read. Don't mark the new
            // pages as needing a sync yet - there's a chance that we'll render directly into
            // the new pages, in which case copying the initial state from the software image
            // would be unnecessary and wasteful.
            case State::initiallyUndefined:
                return Pages::State::suitableToRead;

            // If the software image has been written with valid data, then we need to preserve
            // this data when reading or writing (e.g. to a subsection, or with transparency)
            // to the new pages, so mark the new pages as needing a sync before first access.
            case State::drawn:
                return Pages::State::unsuitableToRead;

            // If this is hit, there's already another BitmapData or Graphics context active on this
            // image. Only one BitmapData or Graphics context may be active on an Image at a time.
            case State::drawing:
                jassertfalse;
                return Pages::State::unsuitableToRead;
        }

        // Unhandled switch case?
        jassertfalse;
        return Pages::State::unsuitableToRead;
    }();

    const auto pair = pagesForDevice.emplace (device1, Pages { context, backingData, initialState });
    return pair.first;
}

std::unique_ptr<LowLevelGraphicsContext> Direct2DPixelData::createLowLevelContext()
{
    if (state == State::drawing)
    {
        // If this is hit, there's already a BitmapData or Graphics context active, drawing to this
        // image. Perhaps you have two Graphics instances drawing into the image, or maybe you
        // called Image::clear while also having a Graphics instance in scope that is targeting this
        // image. A Direct2D Image can only have a single Graphics object active at a time.
        // To fix this issue, check the call stack to see where this assertion is being hit, then
        // modify the program to ensure no other BitmapData or Graphics context is active at this point.
        jassertfalse;

        struct InertContext : public LowLevelGraphicsContext
        {
            bool isVectorDevice() const override { return false; }
            void setOrigin (Point<int>) override {}
            void addTransform (const AffineTransform&) override {}
            float getPhysicalPixelScaleFactor() const override { return 1.0f; }
            bool clipToRectangle (const Rectangle<int>&) override { return false; }
            bool clipToRectangleList (const RectangleList<int>&) override { return false; }
            void excludeClipRectangle (const Rectangle<int>&) override {}
            void clipToPath (const Path&, const AffineTransform&) override {}
            void clipToImageAlpha (const Image&, const AffineTransform&) override {}
            bool clipRegionIntersects (const Rectangle<int>&) override { return false; }
            Rectangle<int> getClipBounds() const override { return {}; }
            bool isClipEmpty() const override { return true; }
            void saveState() override {}
            void restoreState() override {}
            void beginTransparencyLayer (float) override {}
            void endTransparencyLayer() override {}
            void setFill (const FillType&) override {}
            void setOpacity (float) override {}
            void setInterpolationQuality (Graphics::ResamplingQuality) override {}
            void fillRect (const Rectangle<int>&, bool) override {}
            void fillRect (const Rectangle<float>&) override {}
            void fillRectList (const RectangleList<float>&) override {}
            void fillPath (const Path&, const AffineTransform&) override {}
            void drawImage (const Image&, const AffineTransform&) override {}
            void drawLine (const Line<float>&) override {}
            void setFont (const Font& f) override { font = f; }
            const Font& getFont() override { return font; }
            void drawGlyphs (Span<const uint16_t>, Span<const Point<float>>, const AffineTransform&) override {}
            uint64_t getFrameId() const override { return 0; }
            Font font { FontOptions{} };
        };

        return std::make_unique<InertContext>();
    }

    sendDataChangeMessage();

    const auto invalidateAllAndReturnSoftwareContext = [this]
    {
        // If this is hit, something has gone wrong when trying to create a Direct2D renderer,
        // and we're about to fall back to a software renderer instead.
        jassertfalse;

        for (auto& pair : pagesForDevice)
            pair.second.markOutdated();

        return backingData->createLowLevelContext();
    };

    const auto adapter = directX->adapters.getDefaultAdapter();

    if (adapter == nullptr)
        return invalidateAllAndReturnSoftwareContext();

    const auto context = Direct2DDeviceContext::create (adapter);

    if (context == nullptr)
        return invalidateAllAndReturnSoftwareContext();

    const auto maxSize = (int) context->GetMaximumBitmapSize();

    if (maxSize < width || maxSize < height)
        return invalidateAllAndReturnSoftwareContext();

    const auto iter = getIteratorForContext (context);
    jassert (iter != pagesForDevice.end());

    const auto pages = iter->second.getPages();

    if (pages.empty() || pages.front().bitmap == nullptr)
        return invalidateAllAndReturnSoftwareContext();

    // Every page *other than the page we're about to render onto* will need to be updated from the
    // software image before it is next read.
    for (auto i = pagesForDevice.begin(); i != pagesForDevice.end(); ++i)
        if (i != iter)
            i->second.markOutdated();

    struct FlushingContext : public Direct2DImageContext
    {
        FlushingContext (Direct2DPixelData::Ptr selfIn,
                         ComSmartPtr<ID2D1DeviceContext1> context,
                         ComSmartPtr<ID2D1Bitmap1> target)
            : Direct2DImageContext (context, target, D2DUtilities::rectFromSize (target->GetPixelSize())),
              storedContext (context),
              storedTarget (target),
              self (selfIn),
              backup (startFrame (1.0f) ? selfIn->backingData : nullptr)
        {
            if (backup != nullptr)
                self->state = State::drawing;
        }

        ~FlushingContext() override
        {
            if (backup == nullptr)
                return;

            endFrame();
            readFromDirect2DBitmap (storedContext, storedTarget, backup);
            self->state = State::drawn;
        }

        ComSmartPtr<ID2D1DeviceContext1> storedContext;
        ComSmartPtr<ID2D1Bitmap1> storedTarget;
        Direct2DPixelData::Ptr self;
        ImagePixelData::Ptr backup;
    };

    return std::make_unique<FlushingContext> (this, context, pages.front().bitmap);
}

void Direct2DPixelData::initialiseBitmapData (Image::BitmapData& bitmap,
                                              int x,
                                              int y,
                                              Image::BitmapData::ReadWriteMode mode)
{
    backingData->initialiseBitmapData (bitmap, x, y, mode);

    // If we're only reading, then we can assume that the bitmap data was flushed to the software
    // image directly after it was last modified by d2d, so we can just use the BitmapData
    // initialised by the backing data.
    // If we're writing, then we'll need to update our textures next time we try to use them, so
    // mark them as outdated.
    if (mode == Image::BitmapData::readOnly)
        return;

    struct Releaser : public Image::BitmapData::BitmapDataReleaser
    {
        Releaser (std::unique_ptr<BitmapDataReleaser> wrappedIn, Direct2DPixelData::Ptr selfIn)
            : wrapped (std::move (wrappedIn)), self (std::move (selfIn))
        {
            // If this is hit, there's already another BitmapData or Graphics context active on this
            // image. Only one BitmapData or Graphics context may be active on an Image at a time.
            jassert (self->state != State::drawing);
            self->state = State::drawing;
        }

        ~Releaser() override
        {
            self->state = State::drawn;

            for (auto& pair : self->pagesForDevice)
                pair.second.markOutdated();
        }

        std::unique_ptr<BitmapDataReleaser> wrapped;
        Direct2DPixelData::Ptr self;
    };

    bitmap.dataReleaser = std::make_unique<Releaser> (std::move (bitmap.dataReleaser), this);
}

void Direct2DPixelData::applyGaussianBlurEffect (float radius, Image& result)
{
    // The result must be a separate image!
    jassert (result.getPixelData() != this);

    const auto adapter = directX->adapters.getDefaultAdapter();

    if (adapter == nullptr)
    {
        result = {};
        return;
    }

    const auto context = Direct2DDeviceContext::create (adapter);
    const auto maxSize = (int) context->GetMaximumBitmapSize();

    if (context == nullptr || maxSize < width || maxSize < height)
    {
        result = {};
        return;
    }

    ComSmartPtr<ID2D1Effect> effect;
    if (const auto hr = context->CreateEffect (CLSID_D2D1GaussianBlur, effect.resetAndGetPointerAddress());
        FAILED (hr) || effect == nullptr)
    {
        result = {};
        return;
    }

    effect->SetInput (0, getFirstPageForContext (context));
    effect->SetValue (D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, radius / 3.0f);

    const auto outputPixelData = Direct2DBitmap::createBitmap (context,
                                                               Image::ARGB,
                                                               D2D1::SizeU ((UINT32) width, (UINT32) height),
                                                               D2D1_BITMAP_OPTIONS_TARGET);

    context->SetTarget (outputPixelData);
    context->BeginDraw();
    context->Clear();
    context->DrawImage (effect);
    context->EndDraw();

    result = Image { new Direct2DPixelData { context, outputPixelData } };
}

void Direct2DPixelData::applySingleChannelBoxBlurEffect (int radius, Image& result)
{
    // The result must be a separate image!
    jassert (result.getPixelData() != this);

    const auto adapter = directX->adapters.getDefaultAdapter();

    if (adapter == nullptr)
    {
        result = {};
        return;
    }

    const auto context = Direct2DDeviceContext::create (adapter);
    const auto maxSize = (int) context->GetMaximumBitmapSize();

    if (context == nullptr || maxSize < width || maxSize < height)
    {
        result = {};
        return;
    }

    constexpr FLOAT kernel[] { 1.0f / 9.0f, 2.0f / 9.0f, 3.0f / 9.0f, 2.0f / 9.0f, 1.0f / 9.0f };

    ComSmartPtr<ID2D1Effect> begin, end;

    for (auto horizontal : { false, true })
    {
        for (auto i = 0; i < radius; ++i)
        {
            ComSmartPtr<ID2D1Effect> effect;
            if (const auto hr = context->CreateEffect (CLSID_D2D1ConvolveMatrix, effect.resetAndGetPointerAddress());
                FAILED (hr) || effect == nullptr)
            {
                result = {};
                return;
            }

            effect->SetValue (D2D1_CONVOLVEMATRIX_PROP_KERNEL_SIZE_X, (UINT32) (horizontal ? std::size (kernel) : 1));
            effect->SetValue (D2D1_CONVOLVEMATRIX_PROP_KERNEL_SIZE_Y, (UINT32) (horizontal ? 1 : std::size (kernel)));
            effect->SetValue (D2D1_CONVOLVEMATRIX_PROP_KERNEL_MATRIX, kernel);

            if (begin == nullptr)
            {
                begin = effect;
                end = effect;
            }
            else
            {
                effect->SetInputEffect (0, end);
                end = effect;
            }
        }
    }

    if (begin == nullptr)
    {
        result = {};
        return;
    }

    begin->SetInput (0, getFirstPageForContext (context));

    const auto outputPixelData = Direct2DBitmap::createBitmap (context,
                                                               Image::ARGB,
                                                               D2D1::SizeU ((UINT32) width, (UINT32) height),
                                                               D2D1_BITMAP_OPTIONS_TARGET);

    context->SetTarget (outputPixelData);
    context->BeginDraw();
    context->Clear();
    context->DrawImage (end);
    context->EndDraw();

    result = Image { new Direct2DPixelData { context, outputPixelData } };
}

auto Direct2DPixelData::getPagesForContext (ComSmartPtr<ID2D1DeviceContext1> context) -> Span<const Page>
{
    return getIteratorForContext (context)->second.getPages();
}

//==============================================================================
ImagePixelData::Ptr NativeImageType::create (Image::PixelFormat format, int width, int height, bool clearImage) const
{
    SharedResourcePointer<DirectX> directX;

    if (directX->adapters.getFactory() == nullptr)
    {
        // Make sure the DXGI factory exists
        //
        // The caller may be trying to create an Image from a static variable; if this is a DLL, then this is
        // probably called from DllMain. You can't create a DXGI factory from DllMain, so fall back to a
        // software image.
        return new SoftwarePixelData { format, width, height, clearImage };
    }

    return new Direct2DPixelData (format, width, height, clearImage);
}

//==============================================================================
//==============================================================================
#if JUCE_UNIT_TESTS

class Direct2DImageUnitTest final : public UnitTest
{
public:
    Direct2DImageUnitTest()
        : UnitTest ("Direct2DImageUnitTest", UnitTestCategories::graphics)
    {
        compareFunctions[{ Image::RGB, Image::RGB }] = [] (uint8* rgb1, uint8* rgb2)
        {
            return rgb1[0] == rgb2[0] && rgb1[1] == rgb2[1] && rgb1[2] == rgb2[2];
        };

        compareFunctions[{ Image::RGB, Image::ARGB }] = [] (uint8* rgb, uint8* argb)
        {
            // Compare bytes directly to avoid alpha premultiply issues
            return rgb[0] == argb[0] && // blue
                   rgb[1] == argb[1] && // green
                   rgb[2] == argb[2]; // red
        };

        compareFunctions[{ Image::RGB, Image::SingleChannel }] = [] (uint8*, uint8* singleChannel)
        {
            return *singleChannel == 0xff;
        };

        compareFunctions[{ Image::ARGB, Image::RGB }] = [] (uint8* argb, uint8* rgb)
        {
            // Compare bytes directly to avoid alpha premultiply issues
            return argb[0] == rgb[0] && argb[1] == rgb[1] && argb[2] == rgb[2];
        };

        compareFunctions[{ Image::ARGB, Image::ARGB }] = [] (uint8* argb1, uint8* argb2)
        {
            return *reinterpret_cast<uint32*> (argb1) == *reinterpret_cast<uint32*> (argb2);
        };

        compareFunctions[{ Image::ARGB, Image::SingleChannel }] = [] (uint8* argb, uint8* singleChannel)
        {
            return argb[3] == *singleChannel;
        };

        compareFunctions[{ Image::SingleChannel, Image::RGB }] = [] (uint8* singleChannel, uint8* rgb)
        {
            auto alpha = *singleChannel;
            return rgb[0] == alpha && rgb[1] == alpha && rgb[2] == alpha;
        };

        compareFunctions[{ Image::SingleChannel, Image::ARGB }] = [] (uint8* singleChannel, uint8* argb)
        {
            return *singleChannel == argb[3];
        };

        compareFunctions[{ Image::SingleChannel, Image::SingleChannel }] = [] (uint8* singleChannel1, uint8* singleChannel2)
        {
            return *singleChannel1 == *singleChannel2;
        };
    }

    void runTest() override
    {
        random = getRandom();

        constexpr auto multiPageSize = (1 << 14) + 512 + 3;

        beginTest ("Direct2DImageUnitTest");
        {
            for (const auto size : { 100, multiPageSize })
            {
                for (auto format : formats)
                {
                    compareSameFormat (format, size, 32);
                    compareSameFormat (format, 32, size);
                }

                testFormatConversion (size, 32);
                testFormatConversion (32, size);
            }
        }

        beginTest ("Ensure data parity across mapped page boundaries");
        {
            const auto adapterToUse = directX->adapters.getDefaultAdapter();
            const auto contextToUse = Direct2DDeviceContext::create (adapterToUse);

            for (auto sourceFormat : formats)
            {
                Image softwareImage { SoftwareImageType{}.create (sourceFormat, multiPageSize, 32, true) };

                {
                    const Image::BitmapData bitmap { softwareImage, Image::BitmapData::writeOnly };

                    for (int y = 0; y < bitmap.height; y++)
                    {
                        auto line = bitmap.getLinePointer (y);

                        for (int x = 0; x < bitmap.lineStride; x++)
                            line[x] = (uint8_t) jmap (x, 0, bitmap.lineStride, 0, 256);
                    }
                }

                for (auto destFormat : formats)
                {
                    auto d2dImage = NativeImageType{}.convert (softwareImage)
                                                     .convertedToFormat (destFormat);

                    const auto maxPageBounds = [&]
                    {
                        if (auto* data = dynamic_cast<Direct2DPixelData*> (d2dImage.getPixelData()))
                            if (auto pages = data->getPagesForContext (contextToUse); ! pages.empty())
                                return pages.front().getBounds();

                        return Rectangle<int>{};
                    }();

                    const auto boundarySize = softwareImage.getHeight();
                    const auto pageBoundary = softwareImage.getBounds().getIntersection ({ maxPageBounds.getWidth() - boundarySize / 2,
                                                                                           0,
                                                                                           boundarySize,
                                                                                           boundarySize });

                    const Image::BitmapData data1 { softwareImage,
                                                    pageBoundary.getX(),
                                                    pageBoundary.getY(),
                                                    pageBoundary.getWidth(),
                                                    pageBoundary.getHeight(),
                                                    Image::BitmapData::ReadWriteMode::readOnly };
                    const Image::BitmapData data2 { d2dImage,
                                                    pageBoundary.getX(),
                                                    pageBoundary.getY(),
                                                    pageBoundary.getWidth(),
                                                    pageBoundary.getHeight(),
                                                    Image::BitmapData::ReadWriteMode::readOnly };

                    auto f = compareFunctions.at ({ data1.pixelFormat, data2.pixelFormat });

                    for (int y = 0; y < data1.height; y++)
                    {
                        for (int x = 0; x < data1.width; x++)
                        {
                            auto p1 = data1.getPixelPointer (x, y);
                            auto p2 = data2.getPixelPointer (x, y);

                            expect (f (p1, p2));
                        }
                    }
                }
            }
        }
    }

    Rectangle<int> randomRectangleWithin (Rectangle<int> container) noexcept
    {
        const auto w = random.nextInt ({ 1, container.getWidth() });
        const auto h = random.nextInt ({ 1, container.getHeight() });
        const auto x = random.nextInt ({ container.getX(), container.getRight() - w });
        const auto y = random.nextInt ({ container.getY(), container.getBottom() - h });

        return { x, y, w, h };
    }

    void compareSameFormat (Image::PixelFormat format, int width, int height)
    {
        auto softwareImage = Image { SoftwareImageType{}.create (format, width, height, true) };
        {
            Graphics g { softwareImage };
            g.fillCheckerBoard (softwareImage.getBounds().toFloat(), 21.0f, 21.0f, makeRandomColor(), makeRandomColor());
        }

        auto direct2DImage = NativeImageType{}.convert (softwareImage);

        compareImages (softwareImage, direct2DImage, compareFunctions[{ softwareImage.getFormat(), direct2DImage.getFormat() }]);
        checkReadWriteModes (softwareImage);
        checkReadWriteModes (direct2DImage);
    }

    void compareImages (Image& image1, Image& image2, std::function<bool (uint8*, uint8*)> compareBytes)
    {
        {
            // BitmapData width & height should match
            Rectangle<int> area = randomRectangleWithin (image1.getBounds());
            Image::BitmapData data1 { image1, area.getX(), area.getY(), area.getWidth(), area.getHeight(), Image::BitmapData::ReadWriteMode::readOnly };
            Image::BitmapData data2 { image2, area.getX(), area.getY(), area.getWidth(), area.getHeight(), Image::BitmapData::ReadWriteMode::readOnly };

            expect (data1.width == data2.width);
            expect (data1.height == data2.height);
        }

        {
            // Bitmap data should match after ImageType::convert
            Image::BitmapData data1 { image1, Image::BitmapData::ReadWriteMode::readOnly };
            Image::BitmapData data2 { image2, Image::BitmapData::ReadWriteMode::readOnly };

            for (int y = 0; y < data1.height; ++y)
            {
                auto line1 = data1.getLinePointer (y);
                auto line2 = data2.getLinePointer (y);

                for (int x = 0; x < data1.width; ++x)
                {
                    expect (compareBytes (line1, line2), "Failed comparing format " + String { image1.getFormat() } + " to " + String { image2.getFormat() });

                    line1 += data1.pixelStride;
                    line2 += data2.pixelStride;
                }
            }
        }

        {
            // Subsection data should match
            // Should be able to have two different BitmapData objects simultaneously for the same source image
            Rectangle<int> area1 = randomRectangleWithin (image1.getBounds());
            Rectangle<int> area2 = randomRectangleWithin (image1.getBounds());
            Image::BitmapData data1 { image1, Image::BitmapData::ReadWriteMode::readOnly };
            Image::BitmapData data2a { image2, area1.getX(), area1.getY(), area1.getWidth(), area1.getHeight(), Image::BitmapData::ReadWriteMode::readOnly };
            Image::BitmapData data2b { image2, area2.getX(), area2.getY(), area2.getWidth(), area2.getHeight(), Image::BitmapData::ReadWriteMode::readOnly };

            auto compareSubsection = [&] (Image::BitmapData& subsection1, Image::BitmapData& subsection2, Rectangle<int> area)
            {
                for (int y = 0; y < area.getHeight(); ++y)
                {
                    auto line1 = subsection1.getLinePointer (y + area.getY());
                    auto line2 = subsection2.getLinePointer (y);

                    for (int x = 0; x < area.getWidth(); ++x)
                    {
                        expect (compareBytes (line1 + (x + area.getX()) * subsection1.pixelStride, line2 + x * subsection2.pixelStride));
                    }
                }
            };

            compareSubsection (data1, data2a, area1);
            compareSubsection (data1, data2b, area2);
        }
    }

    void checkReadWriteModes (Image& image)
    {
        // Check read and write modes
        int x = random.nextInt (image.getWidth());
        auto writeColor = makeRandomColor().withAlpha (1.0f);
        auto expectedColor = writeColor;
        switch (image.getFormat())
        {
            case Image::SingleChannel:
            {
                auto alpha = writeColor.getAlpha();
                expectedColor = Colour { alpha, alpha, alpha, alpha };
                break;
            }

            case Image::RGB:
            case Image::ARGB:
                break;

            case Image::UnknownFormat:
            default:
                jassertfalse;
                break;
        }

        {
            Image::BitmapData data { image, Image::BitmapData::ReadWriteMode::writeOnly };

            for (int y = 0; y < data.height; ++y)
                data.setPixelColour (x, y, writeColor);
        }

        {
            Image::BitmapData data { image, Image::BitmapData::ReadWriteMode::readOnly };

            for (int y = 0; y < data.height; ++y)
            {
                auto color = data.getPixelColour (x, y);
                expect (color == expectedColor);
            }
        }
    }

    void testFormatConversion (int width, int height)
    {
        for (auto sourceFormat : formats)
        {
            for (auto destFormat : formats)
            {
                Image softwareStartImage { SoftwareImageType {}.create (sourceFormat, width, height, true) };
                {
                    Graphics g { softwareStartImage };
                    g.fillCheckerBoard (softwareStartImage.getBounds().toFloat(), 21.0f, 21.0f, makeRandomColor(), makeRandomColor());
                }

                auto convertedSoftwareImage = softwareStartImage.convertedToFormat (destFormat);
                compareImages (softwareStartImage, convertedSoftwareImage, compareFunctions[{ sourceFormat, destFormat }]);

                auto direct2DImage = NativeImageType{}.convert (softwareStartImage);
                compareImages (softwareStartImage, direct2DImage, compareFunctions[{ sourceFormat, sourceFormat }]);

                auto convertedDirect2DImage = direct2DImage.convertedToFormat (destFormat);
                compareImages (softwareStartImage, convertedDirect2DImage, compareFunctions[{ sourceFormat, destFormat }]);
            }
        }
    }

    Colour makeRandomColor()
    {
        uint8 red   = (uint8) random.nextInt (255);
        uint8 green = (uint8) random.nextInt (255);
        uint8 blue  = (uint8) random.nextInt (255);
        uint8 alpha = (uint8) random.nextInt (255);
        return Colour { red, green, blue, alpha };
    }

    SharedResourcePointer<DirectX> directX;
    Random random;
    std::array<Image::PixelFormat, 3> const formats { Image::RGB, Image::ARGB, Image::SingleChannel };
    std::map<std::pair<Image::PixelFormat, Image::PixelFormat>, std::function<bool (uint8*, uint8*)>> compareFunctions;
};

static Direct2DImageUnitTest direct2DImageUnitTest;

#endif

} // namespace juce
