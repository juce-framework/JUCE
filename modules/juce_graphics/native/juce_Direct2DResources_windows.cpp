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
    static DxgiAdapter::Ptr findAdapter (const DxgiAdapters& adapters, ID2D1Bitmap1* bitmap)
    {
        if (bitmap == nullptr)
            return {};

        ComSmartPtr<IDXGISurface> surface;
        bitmap->GetSurface (surface.resetAndGetPointerAddress());

        if (surface == nullptr)
            return {};

        ComSmartPtr<IDXGIDevice> device;
        JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wlanguage-extension-token")
        surface->GetDevice (__uuidof (device), (void**) device.resetAndGetPointerAddress());
        JUCE_END_IGNORE_WARNINGS_GCC_LIKE

        return findAdapter (adapters, device);
    }

    static DxgiAdapter::Ptr findAdapter (const DxgiAdapters& dxgiAdapters, IDXGIDevice* dxgiDevice)
    {
        if (dxgiDevice == nullptr)
            return {};

        ComSmartPtr<IDXGIAdapter> adapter;
        dxgiDevice->GetAdapter (adapter.resetAndGetPointerAddress());

        if (adapter == nullptr)
            return {};

        ComSmartPtr<IDXGIAdapter1> adapter1;
        adapter.QueryInterface (adapter1);

        if (adapter1 == nullptr)
            return {};

        const auto adapterLuid = getLUID (adapter1);

        const auto& adapters = dxgiAdapters.getAdapterArray();

        const auto it = std::find_if (adapters.begin(), adapters.end(), [&] (DxgiAdapter::Ptr ptr)
        {
            const auto tie = [] (const LUID& x) { return std::tie (x.LowPart, x.HighPart); };

            const auto thisLuid = getLUID (ptr->dxgiAdapter);
            return tie (thisLuid) == tie (adapterLuid);
        });

        if (it == adapters.end())
            return {};

        return *it;
    }

    static DxgiAdapter::Ptr findAdapter (const DxgiAdapters& dxgiAdapters, ID2D1DeviceContext1* context)
    {
        if (context == nullptr)
            return {};

        ComSmartPtr<ID2D1Device> device;
        context->GetDevice (device.resetAndGetPointerAddress());

        if (device == nullptr)
            return {};

        ComSmartPtr<IDXGIDevice> dxgiDevice;
        device.QueryInterface (dxgiDevice);

        return findAdapter (dxgiAdapters, dxgiDevice);
    }

    static LUID getLUID (ComSmartPtr<IDXGIAdapter1> adapter)
    {
        DXGI_ADAPTER_DESC1 desc{};
        adapter->GetDesc1 (&desc);
        return desc.AdapterLuid;
    }

    static std::optional<Direct2DDeviceResources> create (ComSmartPtr<ID2D1DeviceContext1> context)
    {
        if (context == nullptr)
            return {};

        Direct2DDeviceResources result;

        if (const auto hr = context->CreateSolidColorBrush (D2D1::ColorF (0.0f, 0.0f, 0.0f, 1.0f),
                                                            result.colourBrush.resetAndGetPointerAddress());
            FAILED (hr))
        {
            jassertfalse;
            return {};
        }

        result.rectangleListSpriteBatch = std::make_unique<RectangleListSpriteBatch>();

        return result;
    }

    ComSmartPtr<ID2D1SolidColorBrush> colourBrush;
    LinearGradientCache linearGradientCache;
    RadialGradientCache radialGradientCache;
    std::unique_ptr<RectangleListSpriteBatch> rectangleListSpriteBatch;

private:
    Direct2DDeviceResources() = default;
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

        chain2->SetMaximumFrameLatency (1);

        createBuffer (adapter);
        return buffer != nullptr ? S_OK : E_FAIL;
    }

    bool canPaint() const
    {
        return chain != nullptr && buffer != nullptr;
    }

    HRESULT resize (Rectangle<int> newSize)
    {
        if (chain == nullptr)
            return E_FAIL;

        constexpr auto minFrameSize = 1;
        constexpr auto maxFrameSize = 16384;

        auto scaledSize = newSize.getUnion ({ minFrameSize, minFrameSize })
                                 .getIntersection ({ maxFrameSize, maxFrameSize });

        buffer = nullptr;

        if (const auto hr = chain->ResizeBuffers (0, (UINT) scaledSize.getWidth(), (UINT) scaledSize.getHeight(), DXGI_FORMAT_B8G8R8A8_UNORM, swapChainFlags); FAILED (hr))
            return hr;

        ComSmartPtr<IDXGIDevice> device;
        JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wlanguage-extension-token")
        chain->GetDevice (__uuidof (device), (void**) device.resetAndGetPointerAddress());
        JUCE_END_IGNORE_WARNINGS_GCC_LIKE

        SharedResourcePointer<DirectX> directX;
        createBuffer (Direct2DDeviceResources::findAdapter (directX->adapters, device));

        return buffer != nullptr ? S_OK : E_FAIL;
    }

    Rectangle<int> getSize() const
    {
        const auto surface = getSurface();

        if (surface == nullptr)
            return {};

        DXGI_SURFACE_DESC desc{};
        if (FAILED (surface->GetDesc (&desc)))
            return {};

        return { (int) desc.Width, (int) desc.Height };
    }

    WindowsScopedEvent* getEvent()
    {
        if (swapChainEvent.has_value())
            return &*swapChainEvent;

        return nullptr;
    }

    auto getChain() const
    {
        return chain;
    }

    ComSmartPtr<ID2D1Bitmap1> getBuffer() const
    {
        return buffer;
    }

    static constexpr uint32 swapChainFlags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
    static constexpr uint32 presentSyncInterval = 1;
    static constexpr uint32 presentFlags = 0;

private:
    ComSmartPtr<IDXGISurface> getSurface() const
    {
        if (chain == nullptr)
            return nullptr;

        ComSmartPtr<IDXGISurface> surface;
        JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wlanguage-extension-token")
        if (const auto hr = chain->GetBuffer (0, __uuidof (surface), reinterpret_cast<void**> (surface.resetAndGetPointerAddress())); FAILED (hr))
            return nullptr;
        JUCE_END_IGNORE_WARNINGS_GCC_LIKE

        return surface;
    }

    void createBuffer (DxgiAdapter::Ptr adapter)
    {
        buffer = nullptr;

        const auto deviceContext = Direct2DDeviceContext::create (adapter);

        if (deviceContext == nullptr)
            return;

        const auto surface = getSurface();

        if (surface == nullptr)
            return;

        D2D1_BITMAP_PROPERTIES1 bitmapProperties{};
        bitmapProperties.bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW;
        bitmapProperties.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;
        bitmapProperties.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;

        deviceContext->CreateBitmapFromDxgiSurface (surface, bitmapProperties, buffer.resetAndGetPointerAddress());
    }

    ComSmartPtr<IDXGISwapChain1> chain;
    ComSmartPtr<ID2D1Bitmap1> buffer;
    std::optional<WindowsScopedEvent> swapChainEvent;
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
    static std::optional<CompositionTree> create (IDXGIDevice* dxgiDevice, HWND hwnd, IDXGISwapChain1* swapChain)
    {
        if (dxgiDevice == nullptr)
            return {};

        CompositionTree result;

        JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wlanguage-extension-token")
        if (const auto hr = DCompositionCreateDevice (dxgiDevice,
                                                      __uuidof (IDCompositionDevice),
                                                      reinterpret_cast<void**> (result.compositionDevice.resetAndGetPointerAddress()));
                FAILED (hr))
        {
            return {};
        }
        JUCE_END_IGNORE_WARNINGS_GCC_LIKE

        if (const auto hr = result.compositionDevice->CreateTargetForHwnd (hwnd, FALSE, result.compositionTarget.resetAndGetPointerAddress()); FAILED (hr))
            return {};
        if (const auto hr = result.compositionDevice->CreateVisual (result.compositionVisual.resetAndGetPointerAddress()); FAILED (hr))
            return {};
        if (const auto hr = result.compositionTarget->SetRoot (result.compositionVisual); FAILED (hr))
            return {};
        if (const auto hr = result.compositionVisual->SetContent (swapChain); FAILED (hr))
            return {};
        if (const auto hr = result.compositionDevice->Commit(); FAILED (hr))
            return {};

        return result;
    }

private:
    CompositionTree() = default;

    ComSmartPtr<IDCompositionDevice> compositionDevice;
    ComSmartPtr<IDCompositionTarget> compositionTarget;
    ComSmartPtr<IDCompositionVisual> compositionVisual;
};

} // namespace juce
