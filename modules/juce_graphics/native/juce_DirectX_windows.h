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

//==============================================================================
/** Heap storage for a DirectWrite glyph run */
class DirectWriteGlyphRun
{
public:
    void replace (Span<const Point<float>> positions, float scale);

    auto* getAdvances() const { return advances.data(); }
    auto* getOffsets()  const { return offsets .data(); }

private:
    std::vector<float> advances;
    std::vector<DWRITE_GLYPH_OFFSET> offsets;
};

struct DxgiAdapter : public ReferenceCountedObject
{
    using Ptr = ReferenceCountedObjectPtr<DxgiAdapter>;

    static Ptr create (ComSmartPtr<ID2D1Factory2>, ComSmartPtr<IDXGIAdapter1>);

    bool uniqueIDMatches (ReferenceCountedObjectPtr<DxgiAdapter> other) const;

    LUID getAdapterUniqueID() const;

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
    explicit DxgiAdapters (ComSmartPtr<ID2D1Factory2> d2dFactoryIn);

    ~DxgiAdapters();

    void updateAdapters();

    void releaseAdapters();

    const auto& getAdapterArray() const
    {
        return adapterArray;
    }

    auto getFactory() const
    {
        return factory;
    }

    DxgiAdapter::Ptr getAdapterForHwnd (HWND hwnd) const;

    DxgiAdapter::Ptr getDefaultAdapter() const;

    void addListener (DxgiAdapterListener& l);
    void removeListener (DxgiAdapterListener& l);

private:
    static ComSmartPtr<IDXGIFactory2> makeDxgiFactory();

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
    DirectX();

    auto getD2DFactory() const { return d2dSharedFactory; }

private:
    ComSmartPtr<ID2D1Factory2> d2dSharedFactory;

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

    static ComSmartPtr<ID2D1Device1> getDeviceForContext (ComSmartPtr<ID2D1DeviceContext1> context);
};

//==============================================================================
struct Direct2DDeviceContext
{
    static ComSmartPtr<ID2D1DeviceContext1> create (ComSmartPtr<ID2D1Device1> device);

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
                                               Image::PixelFormat outputFormat);

    static ComSmartPtr<ID2D1Bitmap1> createBitmap (ComSmartPtr<ID2D1DeviceContext1> deviceContext,
                                                   Image::PixelFormat format,
                                                   D2D_SIZE_U size,
                                                   D2D1_BITMAP_OPTIONS options);
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
    void findRECTAndValidate (HWND windowHandle);

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

class LinearGradientCache
{
public:
    ComSmartPtr<ID2D1LinearGradientBrush> get (const ColourGradient& gradient,
                                               ComSmartPtr<ID2D1DeviceContext1> deviceContext,
                                               Direct2DMetrics* metrics);

private:
    LruCache<ColourGradient, ComSmartPtr<ID2D1LinearGradientBrush>> cache;
};

class RadialGradientCache
{
public:
    ComSmartPtr<ID2D1RadialGradientBrush> get (const ColourGradient& gradient,
                                               ComSmartPtr<ID2D1DeviceContext1> deviceContext,
                                               Direct2DMetrics* metrics);

private:
    LruCache<ColourGradient, ComSmartPtr<ID2D1RadialGradientBrush>> cache;
};

class RectangleListSpriteBatch
{
    struct TransformCallback
    {
        virtual Rectangle<float> transform (Rectangle<float>) const = 0;
    };

public:
    RectangleListSpriteBatch() = default;

    ~RectangleListSpriteBatch();

    void release();

    template <typename TransformRectangle>
    bool fillRectangles (ComSmartPtr<ID2D1DeviceContext1> deviceContext,
                         const RectangleList<float>& rectangles,
                         const Colour colour,
                         TransformRectangle&& transformRectangle,
                         [[maybe_unused]] Direct2DMetrics* metrics)
    {
        struct Callback : public TransformCallback
        {
            explicit Callback (TransformRectangle&& x) : fn (x) {}
            Rectangle<float> transform (Rectangle<float> x) const override { return fn (x); }
            TransformRectangle fn;
        };

        const Callback callback { std::forward<TransformRectangle> (transformRectangle) };
        return fillRectanglesImpl (deviceContext, rectangles, colour, callback, metrics);
    }

private:
    ComSmartPtr<ID2D1SpriteBatch> getSpriteBatch (ID2D1DeviceContext3& dc, uint32 key);

    bool fillRectanglesImpl (ComSmartPtr<ID2D1DeviceContext1> deviceContext,
                             const RectangleList<float>& rectangles,
                             Colour colour,
                             const TransformCallback& transformRectangle,
                             [[maybe_unused]] Direct2DMetrics* metrics);

    static constexpr uint32 rectangleSize = 32;
    ComSmartPtr<ID2D1BitmapRenderTarget> whiteRectangle;
    HeapBlock<D2D1_RECT_F> destinations;
    size_t destinationsCapacity = 0;
    LruCache<uint32, ComSmartPtr<ID2D1SpriteBatch>, 8> spriteBatches;
};

class Direct2DDeviceResources
{
public:
    static DxgiAdapter::Ptr findAdapter (const DxgiAdapters& adapters, ID2D1Bitmap1* bitmap);
    static DxgiAdapter::Ptr findAdapter (const DxgiAdapters& dxgiAdapters, IDXGIDevice* dxgiDevice);
    static DxgiAdapter::Ptr findAdapter (const DxgiAdapters& dxgiAdapters, ID2D1DeviceContext1* context);

    static LUID getLUID (ComSmartPtr<IDXGIAdapter1> adapter);

    static std::optional<Direct2DDeviceResources> create (ComSmartPtr<ID2D1DeviceContext1> context);

    ComSmartPtr<ID2D1SolidColorBrush> colourBrush;
    LinearGradientCache linearGradientCache;
    RadialGradientCache radialGradientCache;
    std::unique_ptr<RectangleListSpriteBatch> rectangleListSpriteBatch;

private:
    Direct2DDeviceResources() = default;
};

//==============================================================================
class AggregateFontCollection
{
public:
    explicit AggregateFontCollection (ComSmartPtr<IDWriteFontCollection> baseCollection);

    StringArray findAllTypefaceNames();

    static std::vector<ComSmartPtr<IDWriteFont>> getAllFontsInFamily (IDWriteFontFamily* fontFamily);

    StringArray findAllTypefaceStyles (const String& family);

    ComSmartPtr<IDWriteFontFamily> getFamilyByName (const wchar_t* name);

    ComSmartPtr<IDWriteFont> findFontForFace (IDWriteFontFace* face);

    void addCollection (ComSmartPtr<IDWriteFontCollection> collection);

    void removeCollection (ComSmartPtr<IDWriteFontCollection> collection);

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
                             DWRITE_FONT_STRETCH baseStretch) noexcept;

private:
    std::vector<ComSmartPtr<IDWriteFontCollection>> collections;
    std::mutex mutex;
};

class DirectWriteCustomFontCollectionLoader final : public ComBaseClassHelper<IDWriteFontCollectionLoader>
{
public:
    explicit DirectWriteCustomFontCollectionLoader (IDWriteFactory& factoryIn);

    ~DirectWriteCustomFontCollectionLoader() override;

    Uuid addRawFontData (Span<const std::byte> blob);

    HRESULT WINAPI CreateEnumeratorFromKey (IDWriteFactory* factoryIn,
                                            const void* collectionKey,
                                            UINT32 collectionKeySize,
                                            IDWriteFontFileEnumerator** fontFileEnumerator) noexcept override;

private:
    class MemoryFontFileLoader;
    class FontFileEnumerator;

    IDWriteFactory& factory;
    std::vector<ComSmartPtr<MemoryFontFileLoader>> fileLoaders;
};

//==============================================================================
class Direct2DFactories
{
public:
    Direct2DFactories();

    ~Direct2DFactories();

    [[nodiscard]] ComSmartPtr<IDWriteFactory> getDWriteFactory() const { return directWriteFactory; }
    [[nodiscard]] ComSmartPtr<IDWriteFactory4> getDWriteFactory4() const { return directWriteFactory4; }
    [[nodiscard]] AggregateFontCollection& getFonts() { jassert (fonts.has_value()); return *fonts; }
    [[nodiscard]] ComSmartPtr<DirectWriteCustomFontCollectionLoader> getCollectionLoader() const { return collectionLoader; }

private:
    DynamicLibrary direct2dDll { "d2d1.dll" }, directWriteDll { "DWrite.dll" };

    const ComSmartPtr<ID2D1Factory> d2dFactory;
    const ComSmartPtr<IDWriteFactory> directWriteFactory;
    const ComSmartPtr<DirectWriteCustomFontCollectionLoader> collectionLoader;
    const ComSmartPtr<IDWriteFactory4> directWriteFactory4 = directWriteFactory.getInterface<IDWriteFactory4>();

    std::optional<AggregateFontCollection> fonts;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Direct2DFactories)
};

} // namespace juce
