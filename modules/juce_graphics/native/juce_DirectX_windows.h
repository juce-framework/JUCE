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

struct DxgiAdapter : public ReferenceCountedObject
{
    using Ptr = ReferenceCountedObjectPtr<DxgiAdapter>;

    static Ptr create (ComSmartPtr<ID2D1Factory2> d2dFactory, ComSmartPtr<IDXGIAdapter1> dxgiAdapterIn)
    {
        if (dxgiAdapterIn == nullptr || d2dFactory == nullptr)
            return {};

        Ptr result = new DxgiAdapter;
        result->dxgiAdapter = dxgiAdapterIn;

        for (UINT i = 0;; ++i)
        {
            ComSmartPtr<IDXGIOutput> output;
            const auto hr = result->dxgiAdapter->EnumOutputs (i, output.resetAndGetPointerAddress());

            if (hr == DXGI_ERROR_NOT_FOUND || hr == DXGI_ERROR_NOT_CURRENTLY_AVAILABLE)
                break;

            result->dxgiOutputs.push_back (output);
        }

        // This flag adds support for surfaces with a different color channel ordering
        // than the API default. It is required for compatibility with Direct2D.
        const auto creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

        if (const auto hr = D3D11CreateDevice (result->dxgiAdapter,
                                               D3D_DRIVER_TYPE_UNKNOWN,
                                               nullptr,
                                               creationFlags,
                                               nullptr,
                                               0,
                                               D3D11_SDK_VERSION,
                                               result->direct3DDevice.resetAndGetPointerAddress(),
                                               nullptr,
                                               nullptr); FAILED (hr))
        {
            return {};
        }

        if (const auto hr = result->direct3DDevice->QueryInterface (result->dxgiDevice.resetAndGetPointerAddress()); FAILED (hr))
            return {};

        if (const auto hr = d2dFactory->CreateDevice (result->dxgiDevice, result->direct2DDevice.resetAndGetPointerAddress()); FAILED (hr))
            return {};

        return result;
    }

    bool uniqueIDMatches (ReferenceCountedObjectPtr<DxgiAdapter> other) const
    {
        if (other == nullptr)
            return false;

        auto luid = getAdapterUniqueID();
        auto otherLuid = other->getAdapterUniqueID();
        return (luid.HighPart == otherLuid.HighPart) && (luid.LowPart == otherLuid.LowPart);
    }

    LUID getAdapterUniqueID() const
    {
        DXGI_ADAPTER_DESC1 desc;

        if (auto hr = dxgiAdapter->GetDesc1 (&desc); SUCCEEDED (hr))
            return desc.AdapterLuid;

        return LUID { 0, 0 };
    }

    ComSmartPtr<ID3D11Device> direct3DDevice;
    ComSmartPtr<IDXGIDevice> dxgiDevice;
    ComSmartPtr<ID2D1Device1> direct2DDevice;
    ComSmartPtr<IDXGIAdapter1> dxgiAdapter;
    std::vector<ComSmartPtr<IDXGIOutput>> dxgiOutputs;

private:
    DxgiAdapter() = default;
};

struct DxgiAdapterListener
{
    virtual ~DxgiAdapterListener() = default;
    virtual void adapterCreated (DxgiAdapter::Ptr adapter) = 0;
    virtual void adapterRemoved (DxgiAdapter::Ptr adapter) = 0;
};

class DxgiAdapters
{
public:
    explicit DxgiAdapters (ComSmartPtr<ID2D1Factory2> d2dFactoryIn)
        : d2dFactory (d2dFactoryIn)
    {
        updateAdapters();
    }

    ~DxgiAdapters()
    {
        releaseAdapters();
    }

    void updateAdapters()
    {
        if (factory != nullptr && factory->IsCurrent() && ! adapterArray.isEmpty())
            return;

        releaseAdapters();

        if (factory == nullptr || ! factory->IsCurrent())
            factory = makeDxgiFactory();

        if (factory == nullptr)
        {
            // If you hit this, we were unable to create a DXGI Factory, so we won't be able to
            // render anything using Direct2D.
            // Maybe this version of Windows doesn't have Direct2D support.
            jassertfalse;
            return;
        }

        for (UINT i = 0;; ++i)
        {
            ComSmartPtr<IDXGIAdapter1> dxgiAdapter;

            if (factory->EnumAdapters1 (i, dxgiAdapter.resetAndGetPointerAddress()) == DXGI_ERROR_NOT_FOUND)
                break;

            if (const auto adapter = DxgiAdapter::create (d2dFactory, dxgiAdapter))
            {
                adapterArray.add (adapter);
                listeners.call ([adapter] (DxgiAdapterListener& l) { l.adapterCreated (adapter); });
            }
        }
    }

    void releaseAdapters()
    {
        for (const auto& adapter : adapterArray)
            listeners.call ([adapter] (DxgiAdapterListener& l) { l.adapterRemoved (adapter); });

        adapterArray.clear();
    }

    const auto& getAdapterArray() const
    {
        return adapterArray;
    }

    auto getFactory() const
    {
        return factory;
    }

    DxgiAdapter::Ptr getAdapterForHwnd (HWND hwnd) const
    {
        const auto monitor = MonitorFromWindow (hwnd, MONITOR_DEFAULTTONULL);

        if (monitor == nullptr)
            return getDefaultAdapter();

        for (auto& adapter : adapterArray)
        {
            for (const auto& dxgiOutput : adapter->dxgiOutputs)
            {
                DXGI_OUTPUT_DESC desc{};

                if (FAILED (dxgiOutput->GetDesc (&desc)))
                    continue;

                if (desc.Monitor == monitor)
                    return adapter;
            }
        }

        return getDefaultAdapter();
    }

    DxgiAdapter::Ptr getDefaultAdapter() const
    {
        return adapterArray.getFirst();
    }

    void addListener (DxgiAdapterListener& l)
    {
        listeners.add (&l);
    }

    void removeListener (DxgiAdapterListener& l)
    {
        listeners.remove (&l);
    }

private:
    static ComSmartPtr<IDXGIFactory2> makeDxgiFactory()
    {
        JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wlanguage-extension-token")
        ComSmartPtr<IDXGIFactory2> result;
        if (const auto hr = CreateDXGIFactory2 (0, __uuidof (IDXGIFactory2), (void**) result.resetAndGetPointerAddress()); SUCCEEDED (hr))
            return result;
        JUCE_END_IGNORE_WARNINGS_GCC_LIKE

        // If CreateDXGIFactory fails, check to see if this is being called in the context of DllMain.
        // CreateDXGIFactory will always fail if called from the context of DllMain. In this case, the renderer
        // will create a software image instead as a fallback, but that won't perform as well.
        //
        // You may be creating an Image as a static object, which will likely be created in the context of DllMain.
        // Consider deferring your Image creation until later.
        jassertfalse;
        return {};
    }

    ComSmartPtr<ID2D1Factory2> d2dFactory;

    // It's possible that we'll need to add/remove listeners from background threads, especially in
    // the case that Images are created on a background thread.
    ThreadSafeListenerList<DxgiAdapterListener> listeners;
    ComSmartPtr<IDXGIFactory2> factory = makeDxgiFactory();
    ReferenceCountedArray<DxgiAdapter> adapterArray;
};

class DirectX
{
public:
    DirectX() = default;

    auto getD2DFactory() const { return d2dSharedFactory; }
    auto getD2DMultithread() const { return multithread; }

private:
    ComSmartPtr<ID2D1Factory2> d2dSharedFactory = [&]
    {
        D2D1_FACTORY_OPTIONS options;
        options.debugLevel = D2D1_DEBUG_LEVEL_NONE;
        JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wlanguage-extension-token")
        ComSmartPtr<ID2D1Factory2> result;
        auto hr = D2D1CreateFactory (D2D1_FACTORY_TYPE_MULTI_THREADED,
                                     __uuidof (ID2D1Factory2),
                                     &options,
                                     (void**) result.resetAndGetPointerAddress());
        jassertquiet (SUCCEEDED (hr));
        JUCE_END_IGNORE_WARNINGS_GCC_LIKE

        return result;
    }();

    ComSmartPtr<ID2D1Multithread> multithread = [&]
    {
        ComSmartPtr<ID2D1Multithread> result;
        d2dSharedFactory->QueryInterface<ID2D1Multithread> (result.resetAndGetPointerAddress());
        return result;
    }();

public:
    DxgiAdapters adapters { d2dSharedFactory };

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DirectX)
};

struct D2DUtilities
{
    template <typename Type>
    static D2D1_RECT_F toRECT_F (const Rectangle<Type>& r)
    {
        return { (float) r.getX(), (float) r.getY(), (float) r.getRight(), (float) r.getBottom() };
    }

    template <typename Type>
    static D2D1_RECT_U toRECT_U (const Rectangle<Type>& r)
    {
        return { (UINT32) r.getX(), (UINT32) r.getY(), (UINT32) r.getRight(), (UINT32) r.getBottom() };
    }

    template <typename Type>
    static RECT toRECT (const Rectangle<Type>& r)
    {
        return { r.getX(), r.getY(), r.getRight(), r.getBottom() };
    }

    static Rectangle<int> toRectangle (const RECT& r)
    {
        return Rectangle<int>::leftTopRightBottom (r.left, r.top, r.right, r.bottom);
    }

    static Point<int> toPoint (POINT p) noexcept          { return { p.x, p.y }; }
    static POINT toPOINT (Point<int> p) noexcept          { return { p.x, p.y }; }

    static D2D1_POINT_2U toPOINT_2U (Point<int> p)        { return D2D1::Point2U ((UINT32) p.x, (UINT32) p.y); }

    static D2D1_COLOR_F toCOLOR_F (Colour c)
    {
        return { c.getFloatRed(), c.getFloatGreen(), c.getFloatBlue(), c.getFloatAlpha() };
    }

    static D2D1::Matrix3x2F transformToMatrix (const AffineTransform& transform)
    {
        return { transform.mat00, transform.mat10, transform.mat01, transform.mat11, transform.mat02, transform.mat12 };
    }

    static Rectangle<int> rectFromSize (D2D1_SIZE_U s)
    {
        return { (int) s.width, (int) s.height };
    }
};

//==============================================================================
struct Direct2DDeviceContext
{
    static ComSmartPtr<ID2D1DeviceContext1> create (ComSmartPtr<ID2D1Device1> device)
    {
        ComSmartPtr<ID2D1DeviceContext1> result;

        if (const auto hr = device->CreateDeviceContext (D2D1_DEVICE_CONTEXT_OPTIONS_ENABLE_MULTITHREADED_OPTIMIZATIONS,
                                                         result.resetAndGetPointerAddress());
            FAILED (hr))
        {
            jassertfalse;
            return {};
        }

        result->SetTextAntialiasMode (D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);
        result->SetAntialiasMode (D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
        result->SetUnitMode (D2D1_UNIT_MODE_PIXELS);

        return result;
    }

    static ComSmartPtr<ID2D1DeviceContext1> create (DxgiAdapter::Ptr adapter)
    {
        return adapter != nullptr ? create (adapter->direct2DDevice) : nullptr;
    }

    Direct2DDeviceContext() = delete;
};

//==============================================================================
struct Direct2DBitmap
{
    Direct2DBitmap() = delete;

    static ComSmartPtr<ID2D1Bitmap1> toBitmap (const Image& image,
                                               ComSmartPtr<ID2D1DeviceContext1> deviceContext,
                                               Image::PixelFormat outputFormat)
    {
        JUCE_D2DMETRICS_SCOPED_ELAPSED_TIME (Direct2DMetricsHub::getInstance()->imageContextMetrics, createBitmapTime);

        jassert (outputFormat == Image::ARGB || outputFormat == Image::SingleChannel);

        JUCE_TRACE_LOG_D2D_PAINT_CALL (etw::createDirect2DBitmapFromImage, etw::graphicsKeyword);

        // Calling Image::convertedToFormat could cause unchecked recursion since convertedToFormat
        // calls Graphics::drawImageAt which calls Direct2DGraphicsContext::drawImage which calls this function...
        //
        // Use a software image for the conversion instead so the Graphics::drawImageAt call doesn't go
        // through the Direct2D renderer
        //
        // Be sure to explicitly set the DPI to 96.0 for the image; otherwise it will default to the screen DPI
        // and may be scaled incorrectly
        const auto convertedImage = SoftwareImageType{}.convert (image).convertedToFormat (outputFormat);

        if (! convertedImage.isValid())
            return {};

        Image::BitmapData bitmapData { convertedImage, Image::BitmapData::readWrite };

        D2D1_BITMAP_PROPERTIES1 bitmapProperties{};
        bitmapProperties.pixelFormat.format = outputFormat == Image::SingleChannel
                                              ? DXGI_FORMAT_A8_UNORM
                                              : DXGI_FORMAT_B8G8R8A8_UNORM;
        bitmapProperties.pixelFormat.alphaMode = outputFormat == Image::RGB
                                                 ? D2D1_ALPHA_MODE_IGNORE
                                                 : D2D1_ALPHA_MODE_PREMULTIPLIED;
        bitmapProperties.dpiX = USER_DEFAULT_SCREEN_DPI;
        bitmapProperties.dpiY = USER_DEFAULT_SCREEN_DPI;

        const D2D1_SIZE_U size { (UINT32) image.getWidth(), (UINT32) image.getHeight() };

        ComSmartPtr<ID2D1Bitmap1> bitmap;
        deviceContext->CreateBitmap (size,
                                     bitmapData.data,
                                     (UINT32) bitmapData.lineStride,
                                     bitmapProperties,
                                     bitmap.resetAndGetPointerAddress());
        return bitmap;
    }

    static ComSmartPtr<ID2D1Bitmap1> createBitmap (ComSmartPtr<ID2D1DeviceContext1> deviceContext,
                                                   Image::PixelFormat format,
                                                   D2D_SIZE_U size,
                                                   D2D1_BITMAP_OPTIONS options)
    {
        JUCE_TRACE_LOG_D2D_PAINT_CALL (etw::createDirect2DBitmap, etw::graphicsKeyword);

        JUCE_D2DMETRICS_SCOPED_ELAPSED_TIME (Direct2DMetricsHub::getInstance()->imageContextMetrics, createBitmapTime);

        // Verify that the GPU can handle a bitmap of this size
        //
        // If you need a bitmap larger than this, you'll need to either split it up into multiple bitmaps
        // or use a software image (see SoftwareImageType).
        const auto maxBitmapSize = deviceContext->GetMaximumBitmapSize();
        jassertquiet (size.width <= maxBitmapSize && size.height <= maxBitmapSize);

        const auto pixelFormat = D2D1::PixelFormat (format == Image::SingleChannel
                                                        ? DXGI_FORMAT_A8_UNORM
                                                        : DXGI_FORMAT_B8G8R8A8_UNORM,
                                                    format == Image::RGB
                                                        ? D2D1_ALPHA_MODE_IGNORE
                                                        : D2D1_ALPHA_MODE_PREMULTIPLIED);
        const auto bitmapProperties = D2D1::BitmapProperties1 (options, pixelFormat);

        ComSmartPtr<ID2D1Bitmap1> bitmap;
        const auto hr = deviceContext->CreateBitmap (size,
                                                     {},
                                                     {},
                                                     bitmapProperties,
                                                     bitmap.resetAndGetPointerAddress());

        jassertquiet (SUCCEEDED (hr) && bitmap != nullptr);
        return bitmap;
    }
};

//==============================================================================
/*  UpdateRegion extracts the invalid region for a window
    UpdateRegion is used to service WM_PAINT to add the invalid region of a window to
    deferredRepaints. UpdateRegion marks the region as valid, and the region should be painted on the
    next vblank.
    This is similar to the invalid region update in HWNDComponentPeer::handlePaintMessage()
*/
class UpdateRegion
{
public:
    void findRECTAndValidate (HWND windowHandle)
    {
        numRect = 0;

        auto regionHandle = CreateRectRgn (0, 0, 0, 0);

        if (regionHandle == nullptr)
        {
            ValidateRect (windowHandle, nullptr);
            return;
        }

        auto regionType = GetUpdateRgn (windowHandle, regionHandle, false);

        if (regionType == SIMPLEREGION || regionType == COMPLEXREGION)
        {
            auto regionDataBytes = GetRegionData (regionHandle, (DWORD) block.getSize(), (RGNDATA*) block.getData());

            if (regionDataBytes > block.getSize())
            {
                block.ensureSize (regionDataBytes);
                regionDataBytes = GetRegionData (regionHandle, (DWORD) block.getSize(), (RGNDATA*) block.getData());
            }

            if (regionDataBytes > 0)
            {
                auto header = (RGNDATAHEADER const* const) block.getData();

                if (header->iType == RDH_RECTANGLES)
                    numRect = header->nCount;
            }
        }

        if (numRect > 0)
            ValidateRgn (windowHandle, regionHandle);
        else
            ValidateRect (windowHandle, nullptr);

        DeleteObject (regionHandle);
    }

    void clear()
    {
        numRect = 0;
    }

    Span<const RECT> getRects() const
    {
        auto header = (RGNDATAHEADER const* const) block.getData();
        auto* data = (RECT*) (header + 1);
        return { data, numRect };
    }

private:
    MemoryBlock block { 1024 };
    uint32 numRect = 0;
};

} // namespace juce
