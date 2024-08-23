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

class NativeReadOnlyDataReleaser : public Image::BitmapData::BitmapDataReleaser
{
public:
    NativeReadOnlyDataReleaser (Image::PixelFormat pixelFormat,
                                int w,
                                int h,
                                ComSmartPtr<ID2D1DeviceContext1> deviceContextIn,
                                ComSmartPtr<ID2D1Bitmap1> sourceBitmap,
                                Point<int> offset)
        : bitmap (Direct2DBitmap::createBitmap (deviceContextIn,
                                                pixelFormat,
                                                { (UINT32) w, (UINT32) h },
                                                D2D1_BITMAP_OPTIONS_CPU_READ | D2D1_BITMAP_OPTIONS_CANNOT_DRAW))
    {
        const D2D1_POINT_2U destPoint { 0, 0 };
        const Rectangle fullRect { w, h };
        const auto sourceRect = D2DUtilities::toRECT_U (fullRect.getIntersection (fullRect.withPosition (offset)));

        if (auto hr = bitmap->CopyFromBitmap (&destPoint, sourceBitmap, &sourceRect); FAILED (hr))
            return;

        D2D1_MAPPED_RECT mappedRect{};
        bitmap->Map (D2D1_MAP_OPTIONS_READ, &mappedRect);
        data = mappedRect.bits;
        pitch = mappedRect.pitch;
    }

    ~NativeReadOnlyDataReleaser() override
    {
        bitmap->Unmap();
    }

    auto getData() const
    {
        return data;
    }

    auto getPitch() const
    {
        return pitch;
    }

private:
    ComSmartPtr<ID2D1Bitmap1> bitmap;
    BYTE* data = nullptr;
    UINT32 pitch = 0;
};

class SoftwareDataReleaser : public Image::BitmapData::BitmapDataReleaser
{
public:
    SoftwareDataReleaser (std::unique_ptr<Image::BitmapData::BitmapDataReleaser> r,
                          Image backupIn,
                          ComSmartPtr<ID2D1Bitmap1> nativeBitmapIn,
                          Image::BitmapData::ReadWriteMode modeIn,
                          D2D1_RECT_U targetRectIn)
        : oldReleaser (std::move (r)),
          backup (std::move (backupIn)),
          nativeBitmap (nativeBitmapIn),
          targetRect (targetRectIn),
          mode (modeIn)
    {
    }

    static void flushImage (Image softwareImage, ComSmartPtr<ID2D1Bitmap1> native, D2D1_RECT_U target)
    {
        if (native == nullptr)
            return;

        if (softwareImage.getFormat() == Image::PixelFormat::RGB)
            softwareImage = softwareImage.convertedToFormat (Image::PixelFormat::ARGB);

        const Image::BitmapData bitmapData { softwareImage,
                                             (int) target.left,
                                             (int) target.top,
                                             (int) (target.right - target.left),
                                             (int) (target.bottom - target.top),
                                             Image::BitmapData::readOnly };
        const auto hr = native->CopyFromMemory (&target, bitmapData.data, (UINT32) bitmapData.lineStride);
        jassertquiet (SUCCEEDED (hr));
    }

    ~SoftwareDataReleaser() override
    {
        // Ensure that writes to the backup bitmap have been flushed before reading from it
        oldReleaser = nullptr;

        if (mode != Image::BitmapData::ReadWriteMode::readOnly)
            flushImage (backup, nativeBitmap, targetRect);
    }

private:
    std::unique_ptr<Image::BitmapData::BitmapDataReleaser> oldReleaser;
    Image backup;
    ComSmartPtr<ID2D1Bitmap1> nativeBitmap;
    D2D1_RECT_U targetRect{};
    Image::BitmapData::ReadWriteMode mode{};
};

ComSmartPtr<ID2D1Bitmap1> Direct2DPixelData::createAdapterBitmap() const
{
    auto bitmap = Direct2DBitmap::createBitmap (context,
                                                pixelFormat,
                                                { (UINT32) width, (UINT32) height },
                                                D2D1_BITMAP_OPTIONS_TARGET);

    // The bitmap may be slightly too large due
    // to DPI scaling, so fill it with transparent black
    if (bitmap == nullptr || ! clearImage)
        return bitmap;

    context->SetTarget (bitmap);
    context->BeginDraw();
    context->Clear();
    context->EndDraw();
    context->SetTarget (nullptr);

    return bitmap;
}

void Direct2DPixelData::createDeviceResources()
{
    if (adapter == nullptr)
        adapter = directX->adapters.getDefaultAdapter();

    if (context == nullptr)
        context = Direct2DDeviceContext::create (adapter);

    if (nativeBitmap == nullptr)
    {
        nativeBitmap = createAdapterBitmap();

        if (backup.isValid())
            SoftwareDataReleaser::flushImage (backup, nativeBitmap, { 0, 0, (UINT32) width, (UINT32) height });
    }
}

void Direct2DPixelData::initBitmapDataReadOnly (Image::BitmapData& bitmap, int x, int y)
{
    const auto pixelStride = getPixelStride();
    const auto lineStride = getLineStride();

    const auto offset = (size_t) x * (size_t) pixelStride + (size_t) y * (size_t) lineStride;
    bitmap.pixelFormat = pixelFormat;
    bitmap.pixelStride = pixelStride;
    bitmap.lineStride = lineStride;
    bitmap.size = (size_t) (height * lineStride) - offset;

    JUCE_TRACE_LOG_D2D_IMAGE_MAP_DATA;

    auto releaser = std::make_unique<NativeReadOnlyDataReleaser> (pixelFormat,
                                                                  width,
                                                                  height,
                                                                  context,
                                                                  getAdapterD2D1Bitmap(),
                                                                  Point { x, y });
    bitmap.data = releaser->getData();
    bitmap.lineStride = (int) releaser->getPitch();
    bitmap.dataReleaser = std::move (releaser);
}

auto Direct2DPixelData::make (Image::PixelFormat formatToUse,
                              int widthIn,
                              int heightIn,
                              bool clearImageIn,
                              DxgiAdapter::Ptr adapterIn) -> Ptr
{
    return new Direct2DPixelData (formatToUse, widthIn, heightIn, clearImageIn, adapterIn);
}

auto Direct2DPixelData::fromDirect2DBitmap (DxgiAdapter::Ptr adapterIn,
                                            ComSmartPtr<ID2D1DeviceContext1> contextIn,
                                            ComSmartPtr<ID2D1Bitmap1> bitmapIn) -> Ptr
{
    const auto size = bitmapIn->GetPixelSize();
    Ptr result = new Direct2DPixelData { Image::ARGB, (int) size.width, (int) size.height, false, nullptr };
    result->adapter = adapterIn;
    result->context = contextIn;
    result->nativeBitmap = bitmapIn;
    return result;
}

Direct2DPixelData::Direct2DPixelData (Image::PixelFormat f, int widthIn, int heightIn, bool clear, DxgiAdapter::Ptr adapterIn)
    : ImagePixelData (f, widthIn, heightIn),
      clearImage (clear),
      adapter (adapterIn != nullptr ? adapterIn : directX->adapters.getDefaultAdapter())
{
    directX->adapters.addListener (*this);
}

Direct2DPixelData::~Direct2DPixelData()
{
    directX->adapters.removeListener (*this);
}

std::unique_ptr<LowLevelGraphicsContext> Direct2DPixelData::createLowLevelContext()
{
    sendDataChangeMessage();

    struct FlushingContext : public Direct2DImageContext
    {
        explicit FlushingContext (Direct2DPixelData::Ptr p)
            : Direct2DImageContext (p->context, p->getAdapterD2D1Bitmap(), Rectangle { p->width, p->height }),
              ptr (startFrame (1.0f) ? p : nullptr)
        {
        }

        ~FlushingContext() override
        {
            if (ptr == nullptr)
                return;

            endFrame();
            ptr->flushToSoftwareBackup();
        }

        Direct2DPixelData::Ptr ptr;
    };

    return std::make_unique<FlushingContext> (this);
}

void Direct2DPixelData::initialiseBitmapData (Image::BitmapData& bitmap, int x, int y, Image::BitmapData::ReadWriteMode mode)
{
    JUCE_TRACE_LOG_D2D_IMAGE_MAP_DATA;

    // The native format matches the JUCE format, and there's no need to write from CPU->GPU, so
    // map the GPU memory as read-only and return that.
    if (mode == Image::BitmapData::ReadWriteMode::readOnly && pixelFormat != Image::PixelFormat::RGB)
    {
        initBitmapDataReadOnly (bitmap, x, y);
        return;
    }

    // The native format does not match the JUCE format, or the user wants to read the current state of the image.
    // If the user wants to read the image, then we'll need to copy it to CPU memory.
    if (mode != Image::BitmapData::ReadWriteMode::writeOnly)
    {
        // Store the previous width and height, and set up the BitmapData to cover the entire image area
        const auto oldW = std::exchange (bitmap.width, width);
        const auto oldH = std::exchange (bitmap.height, height);

        // Map the image as read-only.
        initBitmapDataReadOnly (bitmap, 0, 0);
        // Copy the mapped image to CPU memory in the correct format.
        backup = BitmapDataDetail::convert (bitmap, SoftwareImageType{});
        // Unmap the image (important, the BitmapData is reused later on).
        bitmap.dataReleaser = {};

        // Reset the initial width and height
        bitmap.width  = oldW;
        bitmap.height = oldH;
    }

    // If the user doesn't want to read from the image, then we may need to create a blank image that they can write to.
    if (! backup.isValid())
        backup = Image { SoftwareImageType{}.create (pixelFormat, width, height, clearImage) };

    // Redirect the BitmapData to our backup software image.
    backup.getPixelData()->initialiseBitmapData (bitmap, x, y, mode);

    // When this dataReleaser is destroyed, then if the mode is not read-only, image data will be copied
    // from the software image to GPU memory.
    bitmap.dataReleaser = std::make_unique<SoftwareDataReleaser> (std::move (bitmap.dataReleaser),
                                                                  backup,
                                                                  getAdapterD2D1Bitmap(),
                                                                  mode,
                                                                  D2D1_RECT_U { (UINT32) x, (UINT32) y, (UINT32) width, (UINT32) height });
}

void Direct2DPixelData::flushToSoftwareBackup()
{
    backup = SoftwareImageType{}.convert (Image { this });
}

ImagePixelData::Ptr Direct2DPixelData::clone()
{
    auto cloned = make (pixelFormat, width, height, false, nullptr);

    if (cloned == nullptr)
        return {};

    cloned->backup = backup.createCopy();

    const D2D1_POINT_2U destinationPoint { 0, 0 };
    const auto sourceRectU = D2DUtilities::toRECT_U (Rectangle { width, height });
    const auto sourceD2D1Bitmap = getAdapterD2D1Bitmap();
    const auto destinationD2D1Bitmap = cloned->getAdapterD2D1Bitmap();

    if (sourceD2D1Bitmap == nullptr || destinationD2D1Bitmap == nullptr)
        return {};

    if (const auto hr = destinationD2D1Bitmap->CopyFromBitmap (&destinationPoint, sourceD2D1Bitmap, &sourceRectU); FAILED (hr))
    {
        jassertfalse;
        return {};
    }

    return cloned;
}

void Direct2DPixelData::applyGaussianBlurEffect (float radius, Image& result)
{
    // The result must be a separate image!
    jassert (result.getPixelData() != this);

    if (context == nullptr)
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

    effect->SetInput (0, getAdapterD2D1Bitmap());
    effect->SetValue (D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, radius / 3.0f);

    const auto tie = [] (const auto& x) { return std::tuple (x.pixelFormat, x.width, x.height); };
    const auto originalPixelData = dynamic_cast<Direct2DPixelData*> (result.getPixelData());

    if (originalPixelData == nullptr || tie (*this) != tie (*originalPixelData))
        result = Image { make (pixelFormat, width, height, false, adapter) };

    const auto outputPixelData = dynamic_cast<Direct2DPixelData*> (result.getPixelData());

    if (outputPixelData == nullptr)
    {
        result = {};
        return;
    }

    outputPixelData->createDeviceResources();
    auto outputDataContext = outputPixelData->context;

    if (outputDataContext == nullptr)
    {
        result = {};
        return;
    }

    outputDataContext->SetTarget (outputPixelData->getAdapterD2D1Bitmap());
    outputDataContext->BeginDraw();
    outputDataContext->Clear();
    outputDataContext->DrawImage (effect);
    outputDataContext->EndDraw();
    outputDataContext->SetTarget (nullptr);
}

void Direct2DPixelData::applySingleChannelBoxBlurEffect (int radius, Image& result)
{
    // The result must be a separate image!
    jassert (result.getPixelData() != this);

    if (context == nullptr)
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

    begin->SetInput (0, getAdapterD2D1Bitmap());

    const auto originalPixelData = dynamic_cast<Direct2DPixelData*> (result.getPixelData());

    if (originalPixelData == nullptr || std::tuple (Image::SingleChannel, width, height) != std::tuple (originalPixelData->pixelFormat, originalPixelData->width, originalPixelData->height))
        result = Image { make (Image::SingleChannel, width, height, false, adapter) };

    const auto outputPixelData = dynamic_cast<Direct2DPixelData*> (result.getPixelData());

    if (outputPixelData == nullptr)
    {
        result = {};
        return;
    }

    outputPixelData->createDeviceResources();
    auto outputDataContext = outputPixelData->context;

    if (outputDataContext == nullptr)
    {
        result = {};
        return;
    }

    outputDataContext->SetTarget (outputPixelData->getAdapterD2D1Bitmap());
    outputDataContext->BeginDraw();
    outputDataContext->Clear();
    outputDataContext->DrawImage (end);
    outputDataContext->EndDraw();
    outputDataContext->SetTarget (nullptr);
}

std::unique_ptr<ImageType> Direct2DPixelData::createType() const
{
    return std::make_unique<NativeImageType>();
}

void Direct2DPixelData::adapterCreated (DxgiAdapter::Ptr)
{
}

void Direct2DPixelData::adapterRemoved (DxgiAdapter::Ptr)
{
    adapter = nullptr;
    context = nullptr;
    nativeBitmap = nullptr;
}

ComSmartPtr<ID2D1Bitmap1> Direct2DPixelData::getAdapterD2D1Bitmap()
{
    createDeviceResources();
    return nativeBitmap;
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

    return Direct2DPixelData::make (format, width, height, clearImage, nullptr);
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
        beginTest ("Direct2DImageUnitTest");

        random = getRandom();

        for (auto format : formats)
            compareSameFormat (format);

        testFormatConversion();
    }

    Rectangle<int> randomRectangleWithin (Rectangle<int> container) noexcept
    {
        auto x = random.nextInt (container.getWidth() - 2);
        auto y = random.nextInt (container.getHeight() - 2);
        auto w = random.nextInt (container.getHeight() - x);
        auto h = random.nextInt (container.getWidth() - y);
        h = jmax (h, 1);
        w = jmax (w, 1);
        return Rectangle<int> { x, y, w, h };
    }

    void compareSameFormat (Image::PixelFormat format)
    {
        auto softwareImage = Image { SoftwareImageType{}.create (format, 100, 100, true) };
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

    void testFormatConversion()
    {
        for (auto sourceFormat : formats)
        {
            for (auto destFormat : formats)
            {
                auto softwareStartImage = Image { SoftwareImageType {}.create (sourceFormat, 100, 100, true) };
                {
                    Graphics g { softwareStartImage };
                    g.fillCheckerBoard (softwareStartImage.getBounds().toFloat(), 21.0f, 21.0f, makeRandomColor(), makeRandomColor());
                }
                auto convertedSoftwareImage = softwareStartImage.convertedToFormat (destFormat);

                compareImages (softwareStartImage, convertedSoftwareImage, compareFunctions[{ sourceFormat, destFormat }]);

                auto direct2DImage = NativeImageType {}.convert (softwareStartImage);

                compareImages (softwareStartImage, direct2DImage, compareFunctions[{ sourceFormat, sourceFormat }]);

                auto convertedDirect2DImage = direct2DImage.convertedToFormat (destFormat);

                compareImages (softwareStartImage, convertedDirect2DImage, compareFunctions[{ sourceFormat, destFormat }]);
            }
        }
    }

    Colour makeRandomColor()
    {
        uint8 red = (uint8) random.nextInt (255);
        uint8 green = (uint8) random.nextInt (255);
        uint8 blue = (uint8) random.nextInt (255);
        uint8 alpha = (uint8) random.nextInt (255);
        return Colour { red, green, blue, alpha };
    }


    Random random;
    std::array<Image::PixelFormat, 3> const formats { Image::RGB, Image::ARGB, Image::SingleChannel };
    std::map<std::pair<Image::PixelFormat, Image::PixelFormat>, std::function<bool (uint8*, uint8*)>> compareFunctions;
};

static Direct2DImageUnitTest direct2DImageUnitTest;

#endif

} // namespace juce
