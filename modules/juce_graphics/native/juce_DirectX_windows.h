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

constexpr auto enableDirectXDebugLayer = false;

class ScopedMultithread
{
public:
    explicit ScopedMultithread (ID2D1Multithread* multithreadIn)
        : multithread (addComSmartPtrOwner (multithreadIn))
    {
        multithreadIn->Enter();
    }

    ~ScopedMultithread()
    {
        multithread->Leave();
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ScopedMultithread)

    ComSmartPtr<ID2D1Multithread> multithread;
};

/*  ScopedGeometryWithSink creates an ID2D1PathGeometry object with an open sink. */
struct ScopedGeometryWithSink
{
    ScopedGeometryWithSink (ID2D1Factory* factory, D2D1_FILL_MODE fillMode)
    {
        if (const auto hr = factory->CreatePathGeometry (geometry.resetAndGetPointerAddress()); FAILED (hr))
            return;

        if (const auto hr = geometry->Open (sink.resetAndGetPointerAddress()); FAILED (hr))
            return;

        sink->SetFillMode (fillMode);
    }

    ~ScopedGeometryWithSink()
    {
        if (sink == nullptr)
            return;

        const auto hr = sink->Close();
        jassertquiet (SUCCEEDED (hr));
    }

    ComSmartPtr<ID2D1PathGeometry> geometry;
    ComSmartPtr<ID2D1GeometrySink> sink;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ScopedGeometryWithSink)
};

class WindowsScopedEvent
{
public:
    explicit WindowsScopedEvent (HANDLE handleIn)
        : handle (handleIn)
    {
    }

    WindowsScopedEvent()
        : WindowsScopedEvent (CreateEvent (nullptr, FALSE, FALSE, nullptr))
    {
    }

    HANDLE getHandle() const noexcept
    {
        return handle.get();
    }

private:
    std::unique_ptr<std::remove_pointer_t<HANDLE>, FunctionPointerDestructor<CloseHandle>> handle;
};

//==============================================================================
struct D2DHelpers
{
    static bool isTransformAxisAligned (const AffineTransform& transform)
    {
        return transform.mat01 == 0.0f && transform.mat10 == 0.0f;
    }

    static void pathToGeometrySink (const Path& path,
                                    ID2D1GeometrySink* sink,
                                    const AffineTransform& transform,
                                    D2D1_FIGURE_BEGIN figureMode)
    {
        class ScopedFigure
        {
        public:
            ScopedFigure (ID2D1GeometrySink* s, D2D1_POINT_2F pt, D2D1_FIGURE_BEGIN mode)
                : sink (s)
            {
                sink->BeginFigure (pt, mode);
            }

            ~ScopedFigure()
            {
                if (sink != nullptr)
                    sink->EndFigure (end);
            }

            void setClosed()
            {
                end = D2D1_FIGURE_END_CLOSED;
            }

        private:
            ID2D1GeometrySink* sink = nullptr;
            D2D1_FIGURE_END end = D2D1_FIGURE_END_OPEN;

            JUCE_DECLARE_NON_COPYABLE (ScopedFigure)
            JUCE_DECLARE_NON_MOVEABLE (ScopedFigure)
        };

        // Every call to BeginFigure must have a matching call to EndFigure. But - the Path does not necessarily
        // have matching startNewSubPath and closePath markers.
        D2D1_POINT_2F lastLocation{};
        std::optional<ScopedFigure> figure;
        Path::Iterator it (path);

        const auto doTransform = [&transform] (float x, float y)
        {
            transform.transformPoint (x, y);
            return D2D1_POINT_2F { x, y };
        };

        const auto updateFigure = [&] (float x, float y)
        {
            if (! figure.has_value())
                figure.emplace (sink, lastLocation, figureMode);

            lastLocation = doTransform (x, y);
        };

        while (it.next())
        {
            switch (it.elementType)
            {
                case Path::Iterator::lineTo:
                    updateFigure (it.x1, it.y1);
                    sink->AddLine (lastLocation);
                    break;

                case Path::Iterator::quadraticTo:
                    updateFigure (it.x2, it.y2);
                    sink->AddQuadraticBezier ({ doTransform (it.x1, it.y1), lastLocation });
                    break;

                case Path::Iterator::cubicTo:
                    updateFigure (it.x3, it.y3);
                    sink->AddBezier ({ doTransform (it.x1, it.y1), doTransform (it.x2, it.y2), lastLocation });
                    break;

                case Path::Iterator::closePath:
                    if (figure.has_value())
                        figure->setClosed();

                    figure.reset();
                    break;

                case Path::Iterator::startNewSubPath:
                    figure.reset();
                    lastLocation = doTransform (it.x1, it.y1);
                    figure.emplace (sink, lastLocation, figureMode);
                    break;
            }
        }
    }

    static D2D1_POINT_2F pointTransformed (Point<float> pt, const AffineTransform& transform)
    {
        transform.transformPoint (pt.x, pt.y);
        return { (FLOAT) pt.x, (FLOAT) pt.y };
    }

    static void rectToGeometrySink (const Rectangle<float>& rect,
                                    ID2D1GeometrySink* sink,
                                    const AffineTransform& transform,
                                    D2D1_FIGURE_BEGIN figureMode)
    {
        const auto a = pointTransformed (rect.getTopLeft(),     transform);
        const auto b = pointTransformed (rect.getTopRight(),    transform);
        const auto c = pointTransformed (rect.getBottomRight(), transform);
        const auto d = pointTransformed (rect.getBottomLeft(),  transform);

        sink->BeginFigure (a, figureMode);
        sink->AddLine (b);
        sink->AddLine (c);
        sink->AddLine (d);
        sink->EndFigure (D2D1_FIGURE_END_CLOSED);
    }

    static ComSmartPtr<ID2D1Geometry> rectListToPathGeometry (ID2D1Factory* factory,
                                                              const RectangleList<float>& clipRegion,
                                                              const AffineTransform& transform,
                                                              D2D1_FILL_MODE fillMode,
                                                              D2D1_FIGURE_BEGIN figureMode,
                                                              [[maybe_unused]] Direct2DMetrics* metrics)
    {
        JUCE_D2DMETRICS_SCOPED_ELAPSED_TIME (metrics, createGeometryTime)
        ScopedGeometryWithSink objects { factory, fillMode };

        if (objects.sink == nullptr)
            return {};

        for (int i = clipRegion.getNumRectangles(); --i >= 0;)
            rectToGeometrySink (clipRegion.getRectangle (i), objects.sink, transform, figureMode);

        return objects.geometry;
    }

    static ComSmartPtr<ID2D1Geometry> pathToPathGeometry (ID2D1Factory* factory,
                                                          const Path& path,
                                                          const AffineTransform& transform,
                                                          D2D1_FIGURE_BEGIN figureMode,
                                                          [[maybe_unused]] Direct2DMetrics* metrics)
    {
        JUCE_D2DMETRICS_SCOPED_ELAPSED_TIME (metrics, createGeometryTime)
        ScopedGeometryWithSink objects { factory, path.isUsingNonZeroWinding() ? D2D1_FILL_MODE_WINDING : D2D1_FILL_MODE_ALTERNATE };

        if (objects.sink == nullptr)
            return {};

        pathToGeometrySink (path, objects.sink, transform, figureMode);

        return objects.geometry;
    }

    static ComSmartPtr<ID2D1StrokeStyle1> pathStrokeTypeToStrokeStyle (ID2D1Factory1* factory, const PathStrokeType& strokeType)
    {
        // JUCE JointStyle   ID2D1StrokeStyle
        // ---------------   ----------------
        // mitered           D2D1_LINE_JOIN_MITER
        // curved            D2D1_LINE_JOIN_ROUND
        // beveled           D2D1_LINE_JOIN_BEVEL
        //
        // JUCE EndCapStyle  ID2D1StrokeStyle
        // ----------------  ----------------
        // butt              D2D1_CAP_STYLE_FLAT
        // square            D2D1_CAP_STYLE_SQUARE
        // rounded           D2D1_CAP_STYLE_ROUND
        auto lineJoin = D2D1_LINE_JOIN_MITER;
        switch (strokeType.getJointStyle())
        {
            case PathStrokeType::JointStyle::mitered:
                // already set
                break;

            case PathStrokeType::JointStyle::curved:
                lineJoin = D2D1_LINE_JOIN_ROUND;
                break;

            case PathStrokeType::JointStyle::beveled:
                lineJoin = D2D1_LINE_JOIN_BEVEL;
                break;

            default:
                // invalid EndCapStyle
                jassertfalse;
                break;
        }

        auto capStyle = D2D1_CAP_STYLE_FLAT;
        switch (strokeType.getEndStyle())
        {
            case PathStrokeType::EndCapStyle::butt:
                // already set
                break;

            case PathStrokeType::EndCapStyle::square:
                capStyle = D2D1_CAP_STYLE_SQUARE;
                break;

            case PathStrokeType::EndCapStyle::rounded:
                capStyle = D2D1_CAP_STYLE_ROUND;
                break;

            default:
                // invalid EndCapStyle
                jassertfalse;
                break;
        }

        D2D1_STROKE_STYLE_PROPERTIES1 strokeStyleProperties
        {
            capStyle,
            capStyle,
            capStyle,
            lineJoin,
            strokeType.getStrokeThickness(),
            D2D1_DASH_STYLE_SOLID,
            0.0f,
            D2D1_STROKE_TRANSFORM_TYPE_NORMAL
        };
        ComSmartPtr<ID2D1StrokeStyle1> strokeStyle;
        factory->CreateStrokeStyle (strokeStyleProperties,
                                    nullptr,
                                    0,
                                    strokeStyle.resetAndGetPointerAddress());

        return strokeStyle;
    }

};

//==============================================================================
/** Heap storage for a DirectWrite glyph run */
class DirectWriteGlyphRun
{
public:
    void replace (Span<const Point<float>> positions, float scale)
    {
        advances.resize (positions.size(), 0.0f);
        offsets.resize (positions.size());
        std::transform (positions.begin(), positions.end(), offsets.begin(), [&] (auto& g)
        {
            return DWRITE_GLYPH_OFFSET { g.x / scale, -g.y };
        });
    }

    auto* getAdvances() const { return advances.data(); }
    auto* getOffsets()  const { return offsets .data(); }

private:
    std::vector<float> advances;
    std::vector<DWRITE_GLYPH_OFFSET> offsets;
};

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
        const auto creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT
                                 | (enableDirectXDebugLayer ? D3D11_CREATE_DEVICE_DEBUG : 0);

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

private:
    ComSmartPtr<ID2D1Factory2> d2dSharedFactory = [&]
    {
        D2D1_FACTORY_OPTIONS options;
        options.debugLevel = enableDirectXDebugLayer ? D2D1_DEBUG_LEVEL_INFORMATION : D2D1_DEBUG_LEVEL_NONE;
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

    static D2D1::Matrix3x2F transformToMatrix (const AffineTransform& t)
    {
        return { t.mat00, t.mat10, t.mat01, t.mat11, t.mat02, t.mat12 };
    }

    static AffineTransform matrixToTransform (const D2D1_MATRIX_3X2_F& m)
    {
        return { m._11, m._21, m._31, m._12, m._22, m._32 };
    }

    static Rectangle<int> rectFromSize (D2D1_SIZE_U s)
    {
        return { (int) s.width, (int) s.height };
    }

    static ComSmartPtr<ID2D1Device1> getDeviceForContext (ComSmartPtr<ID2D1DeviceContext1> context)
    {
        if (context == nullptr)
            return {};

        ComSmartPtr<ID2D1Device> device;
        context->GetDevice (device.resetAndGetPointerAddress());
        return device.getInterface<ID2D1Device1>();
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

    class AssignableDirectX
    {
    public:
        AssignableDirectX() = default;
        AssignableDirectX (const AssignableDirectX&) {}
        AssignableDirectX (AssignableDirectX&&) noexcept {}
        AssignableDirectX& operator= (const AssignableDirectX&) { return *this; }
        AssignableDirectX& operator= (AssignableDirectX&&) noexcept { return *this; }
        ~AssignableDirectX() = default;

        DirectX* operator->() const { return directX.operator->(); }

    private:
        SharedResourcePointer<DirectX> directX;
    };

    AssignableDirectX directX;
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

static String getLocalisedName (IDWriteLocalizedStrings* names)
{
    jassert (names != nullptr);

    uint32 index = 0;
    BOOL exists = false;
    [[maybe_unused]] auto hr = names->FindLocaleName (L"en-us", &index, &exists);

    if (! exists)
        index = 0;

    uint32 length = 0;
    hr = names->GetStringLength (index, &length);

    HeapBlock<wchar_t> name (length + 1);
    hr = names->GetString (index, name, length + 1);

    return static_cast<const wchar_t*> (name);
}

static String getFontFamilyName (IDWriteFontFamily* family)
{
    jassert (family != nullptr);
    ComSmartPtr<IDWriteLocalizedStrings> familyNames;
    auto hr = family->GetFamilyNames (familyNames.resetAndGetPointerAddress());
    jassertquiet (SUCCEEDED (hr));
    return getLocalisedName (familyNames);
}

static String getFontFaceName (IDWriteFont* font)
{
    jassert (font != nullptr);
    ComSmartPtr<IDWriteLocalizedStrings> faceNames;
    auto hr = font->GetFaceNames (faceNames.resetAndGetPointerAddress());
    jassertquiet (SUCCEEDED (hr));
    return getLocalisedName (faceNames);
}

template <typename Range>
static StringArray stringArrayFromRange (Range&& range)
{
    StringArray result;

    for (const auto& item : range)
        result.add (item);

    return result;
}

class AggregateFontCollection
{
public:
    explicit AggregateFontCollection (ComSmartPtr<IDWriteFontCollection> baseCollection)
        : collections { std::move (baseCollection) } {}

    StringArray findAllTypefaceNames()
    {
        const std::scoped_lock lock { mutex };

        std::set<String> strings;

        for (const auto& collection : collections)
        {
            const auto count = collection->GetFontFamilyCount();

            for (auto i = decltype (count){}; i < count; ++i)
            {
                ComSmartPtr<IDWriteFontFamily> family;

                if (FAILED (collection->GetFontFamily (i, family.resetAndGetPointerAddress())) || family == nullptr)
                    continue;

                strings.insert (getFontFamilyName (family));
            }
        }

        return stringArrayFromRange (strings);
    }

    static std::vector<ComSmartPtr<IDWriteFont>> getAllFontsInFamily (IDWriteFontFamily* fontFamily)
    {
        const auto fontFacesCount = fontFamily->GetFontCount();
        std::vector<ComSmartPtr<IDWriteFont>> result;
        result.reserve (fontFacesCount);

        for (UINT32 i = 0; i < fontFacesCount; ++i)
        {
            ComSmartPtr<IDWriteFont> dwFont;

            if (FAILED (fontFamily->GetFont (i, dwFont.resetAndGetPointerAddress())))
                continue;

            if (dwFont->GetSimulations() != DWRITE_FONT_SIMULATIONS_NONE)
                continue;

            result.push_back (dwFont);
        }

        return result;
    }

    StringArray findAllTypefaceStyles (const String& family)
    {
        const std::scoped_lock lock { mutex };

        for (const auto& collection : collections)
        {
            BOOL fontFound = false;
            uint32 fontIndex = 0;

            if (FAILED (collection->FindFamilyName (family.toWideCharPointer(), &fontIndex, &fontFound)) || ! fontFound)
                continue;

            ComSmartPtr<IDWriteFontFamily> fontFamily;

            if (FAILED (collection->GetFontFamily (fontIndex, fontFamily.resetAndGetPointerAddress())) || fontFamily == nullptr)
                continue;

            std::set<String> uniqueResults;
            StringArray orderedResults;

            for (const auto& font : getAllFontsInFamily (fontFamily))
            {
                const auto name = getFontFaceName (font);

                if (uniqueResults.insert (name).second)
                    orderedResults.add (name);
            }

            return orderedResults;
        }

        return {};
    }

    ComSmartPtr<IDWriteFontFamily> getFamilyByName (const wchar_t* name)
    {
        const std::scoped_lock lock { mutex };

        for (const auto& collection : collections)
        {
            const auto fontIndex = [&]
            {
                BOOL   found = false;
                UINT32 index = 0;

                return (SUCCEEDED (collection->FindFamilyName (name, &index, &found)) && found)
                       ? index
                       : (UINT32) -1;
            }();

            if (fontIndex == (UINT32) -1)
                continue;

            ComSmartPtr<IDWriteFontFamily> family;

            if (FAILED (collection->GetFontFamily (fontIndex, family.resetAndGetPointerAddress())) || family == nullptr)
                continue;

            return family;
        }

        return {};
    }

    ComSmartPtr<IDWriteFont> findFontForFace (IDWriteFontFace* face)
    {
        for (const auto& collection : collections)
        {
            ComSmartPtr<IDWriteFont> result;

            if (SUCCEEDED (collection->GetFontFromFontFace (face, result.resetAndGetPointerAddress())))
                return result;
        }

        return {};
    }

    void addCollection (ComSmartPtr<IDWriteFontCollection> collection)
    {
        const std::scoped_lock lock { mutex };
        collections.push_back (std::move (collection));
    }

    void removeCollection (ComSmartPtr<IDWriteFontCollection> collection)
    {
        const std::scoped_lock lock { mutex };
        const auto iter = std::find (collections.begin(), collections.end(), collection);

        if (iter != collections.end())
            collections.erase (iter);
    }

    struct MapResult
    {
        ComSmartPtr<IDWriteFont> font;
        UINT32 length{};
        float scale{};
    };

    /*  Tries matching against each collection in turn.
        If any collection is able to match the entire string, then uses the appropriate font
        from that collection.
        Otherwise, returns the font that is able to match the longest sequence of characters,
        preferring user-provided fonts.
    */
    MapResult mapCharacters (IDWriteFontFallback* fallback,
                             IDWriteTextAnalysisSource* analysisSource,
                             UINT32 textPosition,
                             UINT32 textLength,
                             wchar_t const* baseFamilyName,
                             DWRITE_FONT_WEIGHT baseWeight,
                             DWRITE_FONT_STYLE baseStyle,
                             DWRITE_FONT_STRETCH baseStretch) noexcept
    {
        const std::scoped_lock lock { mutex };

        // For reasons I don't understand, the system may pick better substitutions when passing
        // nullptr, instead of the system collection, as the "default collection to use".
        auto collectionsToCheck = collections;
        collectionsToCheck.insert (collectionsToCheck.begin(), nullptr);

        MapResult bestMatch;
        for (const auto& collection : collectionsToCheck)
        {
            MapResult result;
            const auto status = fallback->MapCharacters (analysisSource,
                                                         textPosition,
                                                         textLength,
                                                         collection,
                                                         baseFamilyName,
                                                         baseWeight,
                                                         baseStyle,
                                                         baseStretch,
                                                         &result.length,
                                                         result.font.resetAndGetPointerAddress(),
                                                         &result.scale);

            if (FAILED (status) || result.font == nullptr)
                continue;

            if (result.length == textLength)
                return result;

            if (result.length >= bestMatch.length)
                bestMatch = result;
        }

        return bestMatch;
    }

private:
    std::vector<ComSmartPtr<IDWriteFontCollection>> collections;
    std::mutex mutex;
};

class MemoryFontFileStream final : public ComBaseClassHelper<IDWriteFontFileStream>
{
public:
    explicit MemoryFontFileStream (std::shared_ptr<const MemoryBlock> blockIn)
        : block (std::move (blockIn))
    {
    }

    JUCE_COMRESULT GetFileSize (UINT64* fileSize) noexcept override
    {
        *fileSize = block->getSize();
        return S_OK;
    }

    JUCE_COMRESULT GetLastWriteTime (UINT64* lastWriteTime) noexcept override
    {
        *lastWriteTime = 0;
        return S_OK;
    }

    JUCE_COMRESULT ReadFileFragment (const void** fragmentStart,
                                     UINT64 fileOffset,
                                     UINT64 fragmentSize,
                                     void** fragmentContext) noexcept override
    {
        if (fileOffset + fragmentSize > block->getSize())
        {
            *fragmentStart   = nullptr;
            *fragmentContext = nullptr;
            return E_INVALIDARG;
        }

        *fragmentStart   = addBytesToPointer (block->getData(), fileOffset);
        *fragmentContext = this;
        return S_OK;
    }

    void WINAPI ReleaseFileFragment (void*) noexcept override {}

private:
    std::shared_ptr<const MemoryBlock> block;
};

class MemoryFontFileLoader final : public ComBaseClassHelper<IDWriteFontFileLoader>
{
public:
    explicit MemoryFontFileLoader (MemoryBlock blob)
        : block (std::make_shared<const MemoryBlock> (std::move (blob)))
    {
    }

    HRESULT WINAPI CreateStreamFromKey (const void* fontFileReferenceKey,
                                        UINT32 keySize,
                                        IDWriteFontFileStream** fontFileStream) noexcept override
    {
        if (keySize != Uuid::size())
            return E_INVALIDARG;

        Uuid requestedKey { static_cast<const uint8*> (fontFileReferenceKey) };

        if (requestedKey == uuid)
        {
            *fontFileStream = new MemoryFontFileStream { block };
            return S_OK;
        }

        return E_INVALIDARG;
    }

    Uuid getUuid() const { return uuid; }

private:
    std::shared_ptr<const MemoryBlock> block;
    Uuid uuid;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MemoryFontFileLoader)
};

class FontFileEnumerator final : public ComBaseClassHelper<IDWriteFontFileEnumerator>
{
public:
    FontFileEnumerator (IDWriteFactory& factoryIn, ComSmartPtr<MemoryFontFileLoader> loaderIn)
        : factory (factoryIn), loader (loaderIn) {}

    HRESULT WINAPI GetCurrentFontFile (IDWriteFontFile** fontFile) noexcept override
    {
        *fontFile = nullptr;

        if (! isPositiveAndBelow (rawDataIndex, 1))
            return E_FAIL;

        const auto uuid = loader->getUuid();
        return factory.CreateCustomFontFileReference (uuid.getRawData(),
                                                      (UINT32) uuid.size(),
                                                      loader,
                                                      fontFile);
    }

    HRESULT WINAPI MoveNext (BOOL* hasCurrentFile) noexcept override
    {
        ++rawDataIndex;
        *hasCurrentFile = rawDataIndex < 1 ? TRUE : FALSE;
        return S_OK;
    }

private:
    IDWriteFactory& factory;
    ComSmartPtr<MemoryFontFileLoader> loader;
    size_t rawDataIndex = std::numeric_limits<size_t>::max();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FontFileEnumerator)
};

class DirectWriteCustomFontCollectionLoader final : public ComBaseClassHelper<IDWriteFontCollectionLoader>
{
public:
    explicit DirectWriteCustomFontCollectionLoader (IDWriteFactory& factoryIn)
        : factory (factoryIn)
    {
    }

    ~DirectWriteCustomFontCollectionLoader() override
    {
        for (const auto& loader : fileLoaders)
            factory.UnregisterFontFileLoader (loader);
    }

    Uuid addRawFontData (Span<const std::byte> blob)
    {
        const auto loader = becomeComSmartPtrOwner (new MemoryFontFileLoader { { blob.data(), blob.size() } });

        factory.RegisterFontFileLoader (loader);

        fileLoaders.push_back (loader);

        return fileLoaders.back()->getUuid();
    }

    HRESULT WINAPI CreateEnumeratorFromKey (IDWriteFactory* factoryIn,
                                            const void* collectionKey,
                                            UINT32 collectionKeySize,
                                            IDWriteFontFileEnumerator** fontFileEnumerator) noexcept override
    {
        if (collectionKeySize != Uuid::size())
            return E_INVALIDARG;

        const Uuid requestedCollectionKey { static_cast<const uint8*> (collectionKey) };

        for (const auto& loader : fileLoaders)
        {
            if (loader->getUuid() != requestedCollectionKey)
                continue;

            *fontFileEnumerator = new FontFileEnumerator { *factoryIn, loader };
            return S_OK;
        }

        return E_INVALIDARG;
    }

private:
    IDWriteFactory& factory;
    std::vector<ComSmartPtr<MemoryFontFileLoader>> fileLoaders;
};

//==============================================================================
class Direct2DFactories
{
public:
    Direct2DFactories()
    {
        ComSmartPtr<IDWriteFontCollection> collection;

        if (SUCCEEDED (directWriteFactory->GetSystemFontCollection (collection.resetAndGetPointerAddress(), FALSE)) && collection != nullptr)
            fonts.emplace (collection);
        else
            jassertfalse;
    }

    ~Direct2DFactories()
    {
        if (directWriteFactory == nullptr)
            return;

        directWriteFactory->UnregisterFontCollectionLoader (collectionLoader);
    }

    [[nodiscard]] ComSmartPtr<IDWriteFactory> getDWriteFactory() const { return directWriteFactory; }
    [[nodiscard]] ComSmartPtr<IDWriteFactory4> getDWriteFactory4() const { return directWriteFactory4; }
    [[nodiscard]] AggregateFontCollection& getFonts() { jassert (fonts.has_value()); return *fonts; }
    [[nodiscard]] ComSmartPtr<DirectWriteCustomFontCollectionLoader> getCollectionLoader() const { return collectionLoader; }

private:
    DynamicLibrary direct2dDll { "d2d1.dll" }, directWriteDll { "DWrite.dll" };

    const ComSmartPtr<ID2D1Factory> d2dFactory = [&]() -> ComSmartPtr<ID2D1Factory>
    {
        JUCE_LOAD_WINAPI_FUNCTION (direct2dDll,
                                   D2D1CreateFactory,
                                   d2d1CreateFactory,
                                   HRESULT,
                                   (D2D1_FACTORY_TYPE, REFIID, D2D1_FACTORY_OPTIONS*, void**))

        if (d2d1CreateFactory == nullptr)
            return {};

        D2D1_FACTORY_OPTIONS options;
        options.debugLevel = D2D1_DEBUG_LEVEL_NONE;

        ComSmartPtr<ID2D1Factory> result;

        JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wlanguage-extension-token")
        d2d1CreateFactory (D2D1_FACTORY_TYPE_SINGLE_THREADED,
                            __uuidof (ID2D1Factory),
                            &options,
                           (void**) result.resetAndGetPointerAddress());
        JUCE_END_IGNORE_WARNINGS_GCC_LIKE

        return result;
    }();

    const ComSmartPtr<IDWriteFactory> directWriteFactory = [&]() -> ComSmartPtr<IDWriteFactory>
    {
        JUCE_LOAD_WINAPI_FUNCTION (directWriteDll,
                                   DWriteCreateFactory,
                                   dWriteCreateFactory,
                                   HRESULT,
                                   (DWRITE_FACTORY_TYPE, REFIID, IUnknown**))

        if (dWriteCreateFactory == nullptr)
            return {};

        ComSmartPtr<IDWriteFactory> result;

        JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wlanguage-extension-token")
        dWriteCreateFactory (DWRITE_FACTORY_TYPE_SHARED,
                             __uuidof (IDWriteFactory),
                             (IUnknown**) result.resetAndGetPointerAddress());
        JUCE_END_IGNORE_WARNINGS_GCC_LIKE

        return result;
    }();

    const ComSmartPtr<DirectWriteCustomFontCollectionLoader> collectionLoader = [&]() -> ComSmartPtr<DirectWriteCustomFontCollectionLoader>
    {
        auto result = becomeComSmartPtrOwner (new DirectWriteCustomFontCollectionLoader { *directWriteFactory });
        directWriteFactory->RegisterFontCollectionLoader (result);

        return result;
    }();

    const ComSmartPtr<IDWriteFactory4> directWriteFactory4 = directWriteFactory.getInterface<IDWriteFactory4>();

    std::optional<AggregateFontCollection> fonts;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Direct2DFactories)
};

} // namespace juce
