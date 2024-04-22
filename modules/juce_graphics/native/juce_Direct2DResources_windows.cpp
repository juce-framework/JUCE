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

struct Direct2DDeviceContext
{
    HRESULT createHwndRenderTarget (HWND hwnd)
    {
        if (hwndRenderTarget != nullptr)
            return S_OK;

        SharedResourcePointer<DirectX> directX;

        D2D1_SIZE_U size { 1, 1 };

        D2D1_RENDER_TARGET_PROPERTIES renderTargetProps{};
        renderTargetProps.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;
        renderTargetProps.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;

        D2D1_HWND_RENDER_TARGET_PROPERTIES hwndRenderTargetProps{};
        hwndRenderTargetProps.hwnd = hwnd;
        hwndRenderTargetProps.pixelSize = size;
        hwndRenderTargetProps.presentOptions = D2D1_PRESENT_OPTIONS_IMMEDIATELY | D2D1_PRESENT_OPTIONS_RETAIN_CONTENTS;
        return directX->getD2DFactory()->CreateHwndRenderTarget (&renderTargetProps,
                                                                 &hwndRenderTargetProps,
                                                                 hwndRenderTarget.resetAndGetPointerAddress());
    }

    void resetTransform()
    {
        context->SetTransform (D2D1::IdentityMatrix());
    }

    void setTransform (AffineTransform newTransform)
    {
        context->SetTransform (D2DUtilities::transformToMatrix (newTransform));
    }

    void release()
    {
        hwndRenderTarget = nullptr;
        context = nullptr;
    }

    static ComSmartPtr<ID2D1DeviceContext1> createContext (DxgiAdapter::Ptr adapter)
    {
        ComSmartPtr<ID2D1DeviceContext1> result;

        if (const auto hr = adapter->direct2DDevice->CreateDeviceContext (D2D1_DEVICE_CONTEXT_OPTIONS_ENABLE_MULTITHREADED_OPTIMIZATIONS,
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

    ComSmartPtr<ID2D1DeviceContext1> context;
    ComSmartPtr<ID2D1HwndRenderTarget> hwndRenderTarget;
};

class Direct2DBitmap final
{
public:
    static ComSmartPtr<ID2D1Bitmap1> fromImage (const Image& image,
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
                                                   int lineStride,
                                                   D2D1_BITMAP_OPTIONS options)
    {
        JUCE_TRACE_LOG_D2D_PAINT_CALL (etw::createDirect2DBitmap, etw::graphicsKeyword);

        JUCE_D2DMETRICS_SCOPED_ELAPSED_TIME (Direct2DMetricsHub::getInstance()->imageContextMetrics, createBitmapTime);

       #if JUCE_DEBUG
        // Verify that the GPU can handle a bitmap of this size
        //
        // If you need a bitmap larger than this, you'll need to either split it up into multiple bitmaps
        // or use a software image (see SoftwareImageType).
        auto maxBitmapSize = deviceContext->GetMaximumBitmapSize();
        jassert (size.width <= maxBitmapSize && size.height <= maxBitmapSize);
       #endif

        D2D1_BITMAP_PROPERTIES1 bitmapProperties{};
        bitmapProperties.dpiX = bitmapProperties.dpiY = USER_DEFAULT_SCREEN_DPI;
        bitmapProperties.pixelFormat.format = format == Image::SingleChannel
                                            ? DXGI_FORMAT_A8_UNORM
                                            : DXGI_FORMAT_B8G8R8A8_UNORM;
        bitmapProperties.pixelFormat.alphaMode = format == Image::RGB
                                               ? D2D1_ALPHA_MODE_IGNORE
                                               : D2D1_ALPHA_MODE_PREMULTIPLIED;
        bitmapProperties.bitmapOptions = options;

        ComSmartPtr<ID2D1Bitmap1> bitmap;
        deviceContext->CreateBitmap (size,
                                     nullptr,
                                     (UINT32) lineStride,
                                     bitmapProperties,
                                     bitmap.resetAndGetPointerAddress());
        return bitmap;
    }

    Direct2DBitmap() = delete;
};

static ComSmartPtr<ID2D1GradientStopCollection> makeGradientStopCollection (const ColourGradient& gradient,
                                                                            ComSmartPtr<ID2D1DeviceContext1> deviceContext,
                                                                            [[maybe_unused]] Direct2DMetrics* metrics) noexcept
{
    JUCE_D2DMETRICS_SCOPED_ELAPSED_TIME (metrics, createGradientTime);

    const int numColors = gradient.getNumColours();

    std::vector<D2D1_GRADIENT_STOP> stops ((size_t) numColors);

    for (auto [index, stop] : enumerate (stops, int{}))
    {
        stop.color = D2DUtilities::toCOLOR_F (gradient.getColour (index));
        stop.position = (FLOAT) gradient.getColourPosition (index);
    }

    ComSmartPtr<ID2D1GradientStopCollection> result;
    deviceContext->CreateGradientStopCollection (stops.data(), (UINT32) stops.size(), result.resetAndGetPointerAddress());
    return result;
}

class LinearGradientCache
{
public:
    ComSmartPtr<ID2D1LinearGradientBrush> get (const ColourGradient& gradient,
                                               ComSmartPtr<ID2D1DeviceContext1> deviceContext,
                                               Direct2DMetrics* metrics)
    {
        jassert (! gradient.isRadial);

        return cache.get (gradient, [&deviceContext, &metrics] (const auto& key)
        {
            const auto gradientStops = makeGradientStopCollection (key, deviceContext, metrics);
            const auto p1 = key.point1;
            const auto p2 = key.point2;
            const auto linearGradientBrushProperties = D2D1::LinearGradientBrushProperties ({ p1.x, p1.y }, { p2.x, p2.y });
            const D2D1_BRUSH_PROPERTIES brushProps { 1.0f, D2D1::IdentityMatrix() };

            ComSmartPtr<ID2D1LinearGradientBrush> result;
            deviceContext->CreateLinearGradientBrush (linearGradientBrushProperties,
                                                      brushProps,
                                                      gradientStops,
                                                      result.resetAndGetPointerAddress());
            return result;
        });
    }

private:
    LruCache<ColourGradient, ComSmartPtr<ID2D1LinearGradientBrush>> cache;
};

class RadialGradientCache
{
public:
    ComSmartPtr<ID2D1RadialGradientBrush> get (const ColourGradient& gradient,
                                               ComSmartPtr<ID2D1DeviceContext1> deviceContext,
                                               Direct2DMetrics* metrics)
    {
        jassert (gradient.isRadial);

        return cache.get (gradient, [&deviceContext, &metrics] (const auto& key)
        {
            const auto gradientStops = makeGradientStopCollection (key, deviceContext, metrics);

            const auto p1 = key.point1;
            const auto p2 = key.point2;
            const auto r = p1.getDistanceFrom (p2);
            const auto radialGradientBrushProperties = D2D1::RadialGradientBrushProperties ({ p1.x, p1.y }, {}, r, r);
            const D2D1_BRUSH_PROPERTIES brushProps { 1.0F, D2D1::IdentityMatrix() };

            ComSmartPtr<ID2D1RadialGradientBrush> result;
            deviceContext->CreateRadialGradientBrush (radialGradientBrushProperties,
                                                      brushProps,
                                                      gradientStops,
                                                      result.resetAndGetPointerAddress());
            return result;
        });
    }

private:
    LruCache<ColourGradient, ComSmartPtr<ID2D1RadialGradientBrush>> cache;
};

class RectangleListSpriteBatch
{
public:
    RectangleListSpriteBatch() = default;

    ~RectangleListSpriteBatch()
    {
        release();
    }

    void release()
    {
        whiteRectangle = nullptr;
        spriteBatches = {};
        destinations.free();
        destinationsCapacity = 0;
    }

    template <typename TransformRectangle>
    void fillRectangles (ComSmartPtr<ID2D1DeviceContext1> deviceContext,
                         const RectangleList<float>& rectangles,
                         const Colour colour,
                         TransformRectangle&& transformRectangle,
                         [[maybe_unused]] Direct2DMetrics* metrics)
    {
        if (rectangles.isEmpty())
            return;

        JUCE_D2DMETRICS_SCOPED_ELAPSED_TIME (metrics, spriteBatchTime)

        auto numRectanglesPainted = 0;
        while (numRectanglesPainted < rectangles.getNumRectangles())
        {
            auto numRectanglesRemaining = rectangles.getNumRectangles() - numRectanglesPainted;
            auto spriteBatchSize = isPowerOfTwo (numRectanglesRemaining) ? numRectanglesRemaining : (nextPowerOfTwo (numRectanglesRemaining) >> 1);

            {
                JUCE_D2DMETRICS_SCOPED_ELAPSED_TIME (metrics, spriteBatchSetupTime);

                if (destinationsCapacity < (size_t) spriteBatchSize)
                {
                    destinations.calloc (spriteBatchSize);
                    destinationsCapacity = (size_t) spriteBatchSize;
                }

                auto destination = destinations.getData();

                for (int i = numRectanglesPainted; i < numRectanglesPainted + spriteBatchSize; ++i)
                {
                    auto r = rectangles.getRectangle (i);
                    r = transformRectangle (r);
                    *destination = D2DUtilities::toRECT_F (r);
                    ++destination;
                }
            }

            if (! whiteRectangle)
            {
                JUCE_D2DMETRICS_SCOPED_ELAPSED_TIME (metrics, createSpriteSourceTime);

                auto hr = deviceContext->CreateCompatibleRenderTarget (D2D1_SIZE_F { (float) rectangleSize, (float) rectangleSize },
                                                                       D2D1_SIZE_U { rectangleSize, rectangleSize },
                                                                       D2D1_PIXEL_FORMAT { DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED },
                                                                       whiteRectangle.resetAndGetPointerAddress());
                if (FAILED (hr))
                    return;

                whiteRectangle->BeginDraw();
                whiteRectangle->Clear (D2D1_COLOR_F { 1.0f, 1.0f, 1.0f, 1.0f });
                whiteRectangle->EndDraw();
            }

            ComSmartPtr<ID2D1Bitmap> bitmap;
            if (auto hr = whiteRectangle->GetBitmap (bitmap.resetAndGetPointerAddress()); SUCCEEDED (hr))
            {
                ComSmartPtr<ID2D1DeviceContext3> deviceContext3;
                if (hr = deviceContext->QueryInterface<ID2D1DeviceContext3> (deviceContext3.resetAndGetPointerAddress()); SUCCEEDED (hr))
                {
                    auto d2dColour = D2DUtilities::toCOLOR_F (colour);
                    auto spriteBatch = getSpriteBatch (*deviceContext3, (uint32) spriteBatchSize);

                    if (spriteBatch == nullptr)
                        return;

                    auto setCount = jmin ((uint32) spriteBatchSize, spriteBatch->GetSpriteCount());
                    auto addCount = (uint32) spriteBatchSize > setCount ? (uint32) spriteBatchSize - setCount : 0;

                    if (setCount != 0)
                    {
                        JUCE_D2DMETRICS_SCOPED_ELAPSED_TIME (metrics, setSpritesTime);

                        spriteBatch->SetSprites (0, setCount, destinations.getData(), nullptr, &d2dColour, nullptr, sizeof (D2D1_RECT_F), 0, 0, 0);
                    }

                    if (addCount != 0)
                    {
                        JUCE_D2DMETRICS_SCOPED_ELAPSED_TIME (metrics, addSpritesTime);

                        spriteBatch->AddSprites (addCount, destinations.getData() + setCount, nullptr, &d2dColour, nullptr, sizeof (D2D1_RECT_F), 0, 0, 0);
                    }

                    JUCE_D2DMETRICS_SCOPED_ELAPSED_TIME (metrics, drawSpritesTime);

                    deviceContext3->SetAntialiasMode (D2D1_ANTIALIAS_MODE_ALIASED);
                    deviceContext3->DrawSpriteBatch (spriteBatch, bitmap);
                    deviceContext3->SetAntialiasMode (D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
                }
            }

            numRectanglesPainted += spriteBatchSize;
        }
    }

private:
    ComSmartPtr<ID2D1SpriteBatch> getSpriteBatch (ID2D1DeviceContext3& dc, uint32 key)
    {
        return spriteBatches.get ((uint32) key, [&dc] (auto) -> ComSmartPtr<ID2D1SpriteBatch>
        {
            ComSmartPtr<ID2D1SpriteBatch> result;
            if (const auto hr = dc.CreateSpriteBatch (result.resetAndGetPointerAddress()); SUCCEEDED (hr))
                return result;

            return nullptr;
        });
    }

    static constexpr uint32 rectangleSize = 32;
    ComSmartPtr<ID2D1BitmapRenderTarget> whiteRectangle;
    HeapBlock<D2D1_RECT_F> destinations;
    size_t destinationsCapacity = 0;
    LruCache<uint32, ComSmartPtr<ID2D1SpriteBatch>, 8> spriteBatches;
};

class Direct2DDeviceResources
{
public:
    Direct2DDeviceResources() = default;

    // Create a Direct2D device context for a DXGI adapter
    HRESULT create (DxgiAdapter::Ptr adapter)
    {
        jassert (adapter);

        if (deviceContext.context == nullptr)
            deviceContext.context = Direct2DDeviceContext::createContext (adapter);

        if (colourBrush == nullptr)
        {
            if (const auto hr = deviceContext.context->CreateSolidColorBrush (D2D1::ColorF (0.0f, 0.0f, 0.0f, 1.0f),
                                                                              colourBrush.resetAndGetPointerAddress());
                FAILED (hr))
            {
                jassertfalse;
                return hr;
            }
        }

        if (rectangleListSpriteBatch == nullptr)
            rectangleListSpriteBatch = std::make_unique<RectangleListSpriteBatch>();

        return S_OK;
    }

    void release()
    {
        rectangleListSpriteBatch = nullptr;
        linearGradientCache = {};
        radialGradientCache = {};
        colourBrush = nullptr;
        deviceContext.release();
    }

    bool canPaint (DxgiAdapter::Ptr adapter) const
    {
        return adapter->direct2DDevice != nullptr && deviceContext.context != nullptr && colourBrush != nullptr;
    }

    Direct2DDeviceContext deviceContext;
    ComSmartPtr<ID2D1SolidColorBrush> colourBrush;
    LinearGradientCache linearGradientCache;
    RadialGradientCache radialGradientCache;
    std::unique_ptr<RectangleListSpriteBatch> rectangleListSpriteBatch;
};

class SwapChain
{
public:
    SwapChain() = default;

    HRESULT create (HWND hwnd, Rectangle<int> size, DxgiAdapter::Ptr adapter)
    {
        if (chain != nullptr || hwnd == nullptr)
            return S_OK;

        SharedResourcePointer<DirectX> directX;
        auto dxgiFactory = directX->adapters.getFactory();

        if (dxgiFactory == nullptr || adapter->direct3DDevice == nullptr)
            return E_FAIL;

        buffer = nullptr;
        chain = nullptr;

        // Make the waitable swap chain
        // Create the swap chain with premultiplied alpha support for transparent windows
        DXGI_SWAP_CHAIN_DESC1 swapChainDescription = {};
        swapChainDescription.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        swapChainDescription.Width = (UINT) size.getWidth();
        swapChainDescription.Height = (UINT) size.getHeight();
        swapChainDescription.SampleDesc.Count = 1;
        swapChainDescription.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDescription.BufferCount = 2;
        swapChainDescription.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
        swapChainDescription.Flags = swapChainFlags;

        swapChainDescription.Scaling = DXGI_SCALING_STRETCH;
        swapChainDescription.AlphaMode = DXGI_ALPHA_MODE_PREMULTIPLIED;

        if (const auto hr = dxgiFactory->CreateSwapChainForComposition (adapter->direct3DDevice,
                                                                        &swapChainDescription,
                                                                        nullptr,
                                                                        chain.resetAndGetPointerAddress());
            FAILED (hr))
        {
            return hr;
        }

        // Get the waitable swap chain presentation event and set the maximum frame latency
        ComSmartPtr<IDXGISwapChain2> chain2;
        if (const auto hr = chain.QueryInterface (chain2); FAILED (hr))
            return hr;

        if (chain2 == nullptr)
            return E_FAIL;

        swapChainEvent.emplace (chain2->GetFrameLatencyWaitableObject());
        if (swapChainEvent->getHandle() == INVALID_HANDLE_VALUE)
            return E_NOINTERFACE;

        if (const auto hr = chain2->SetMaximumFrameLatency (1); SUCCEEDED (hr))
            state = State::chainAllocated;

        return S_OK;
    }

    HRESULT createBuffer (ComSmartPtr<ID2D1DeviceContext> deviceContext)
    {
        if (deviceContext == nullptr || chain == nullptr || buffer != nullptr)
            return S_OK;

        ComSmartPtr<IDXGISurface> surface;
        JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wlanguage-extension-token")
        if (const auto hr = chain->GetBuffer (0, __uuidof (surface), reinterpret_cast<void**> (surface.resetAndGetPointerAddress())); FAILED (hr))
            return hr;
        JUCE_END_IGNORE_WARNINGS_GCC_LIKE

        D2D1_BITMAP_PROPERTIES1 bitmapProperties{};
        bitmapProperties.bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW;
        bitmapProperties.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;
        bitmapProperties.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;

        if (const auto hr = deviceContext->CreateBitmapFromDxgiSurface (surface, bitmapProperties, buffer.resetAndGetPointerAddress()); FAILED (hr))
            return hr;

        state = State::bufferAllocated;
        return S_OK;
    }

    void release()
    {
        buffer = nullptr;
        chain = nullptr;
        state = State::idle;
    }

    bool canPaint() const
    {
        return chain != nullptr && buffer != nullptr && state >= State::bufferAllocated;
    }

    HRESULT resize (Rectangle<int> newSize, ComSmartPtr<ID2D1DeviceContext> deviceContext)
    {
        if (chain == nullptr)
            return E_FAIL;

        auto scaledSize = newSize.getUnion ({ Direct2DGraphicsContext::minFrameSize, Direct2DGraphicsContext::minFrameSize })
                                 .getIntersection ({ Direct2DGraphicsContext::maxFrameSize, Direct2DGraphicsContext::maxFrameSize });

        buffer = nullptr;
        state = State::chainAllocated;

        if (const auto hr = chain->ResizeBuffers (0, (UINT) scaledSize.getWidth(), (UINT) scaledSize.getHeight(), DXGI_FORMAT_B8G8R8A8_UNORM, swapChainFlags); FAILED (hr))
            return hr;

        if (const auto hr = createBuffer (deviceContext); FAILED (hr))
        {
            release();
            return hr;
        }

        return S_OK;
    }

    Rectangle<int> getSize() const
    {
        if (buffer == nullptr)
            return {};

        auto size = buffer->GetPixelSize();
        return { (int) size.width, (int) size.height };
    }

    static constexpr uint32 swapChainFlags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
    static constexpr uint32 presentSyncInterval = 1;
    static constexpr uint32 presentFlags = 0;
    ComSmartPtr<IDXGISwapChain1> chain;
    ComSmartPtr<ID2D1Bitmap1> buffer;
    std::optional<WindowsScopedEvent> swapChainEvent;

    enum class State
    {
        idle,
        chainAllocated,
        bufferAllocated,
        bufferFilled
    };
    State state = State::idle;
};

//==============================================================================
/*  DirectComposition
    Using DirectComposition enables transparent windows and smoother window
    resizing

    This class builds a simple DirectComposition tree that ultimately contains
    the swap chain
*/
class CompositionTree
{
public:
    HRESULT create (IDXGIDevice* const dxgiDevice, HWND hwnd, IDXGISwapChain1* const swapChain)
    {
        if (compositionDevice != nullptr)
            return S_OK;

        if (dxgiDevice == nullptr)
            return E_FAIL;

        JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wlanguage-extension-token")
        if (const auto hr = DCompositionCreateDevice (dxgiDevice,
                                                      __uuidof (IDCompositionDevice),
                                                      reinterpret_cast<void**> (compositionDevice.resetAndGetPointerAddress()));
            FAILED (hr))
        {
            return hr;
        }
        JUCE_END_IGNORE_WARNINGS_GCC_LIKE

        if (const auto hr = compositionDevice->CreateTargetForHwnd (hwnd, FALSE, compositionTarget.resetAndGetPointerAddress()); FAILED (hr))
            return hr;
        if (const auto hr = compositionDevice->CreateVisual (compositionVisual.resetAndGetPointerAddress()); FAILED (hr))
            return hr;
        if (const auto hr = compositionTarget->SetRoot (compositionVisual); FAILED (hr))
            return hr;
        if (const auto hr = compositionVisual->SetContent (swapChain); FAILED (hr))
            return hr;
        if (const auto hr = compositionDevice->Commit(); FAILED (hr))
            return hr;

        return S_OK;
    }

    void release()
    {
        compositionVisual = nullptr;
        compositionTarget = nullptr;
        compositionDevice = nullptr;
    }

    bool canPaint() const
    {
        return compositionVisual != nullptr;
    }

private:
    ComSmartPtr<IDCompositionDevice> compositionDevice;
    ComSmartPtr<IDCompositionTarget> compositionTarget;
    ComSmartPtr<IDCompositionVisual> compositionVisual;
};

} // namespace juce
