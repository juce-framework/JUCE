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

    DxgiAdapter (ComSmartPtr<ID2D1Factory2> d2dFactory, ComSmartPtr<IDXGIAdapter1> dxgiAdapterIn)
        : dxgiAdapter (dxgiAdapterIn)
    {
        for (UINT i = 0;; ++i)
        {
            ComSmartPtr<IDXGIOutput> output;
            const auto hr = dxgiAdapter->EnumOutputs (i, output.resetAndGetPointerAddress());

            if (hr == DXGI_ERROR_NOT_FOUND || hr == DXGI_ERROR_NOT_CURRENTLY_AVAILABLE)
                break;

            dxgiOutputs.push_back (output);
        }

        // This flag adds support for surfaces with a different color channel ordering
        // than the API default. It is required for compatibility with Direct2D.
        const auto creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
        jassert (dxgiAdapter);

        if (const auto hr = D3D11CreateDevice (dxgiAdapter,
                                               D3D_DRIVER_TYPE_UNKNOWN,
                                               nullptr,
                                               creationFlags,
                                               nullptr,
                                               0,
                                               D3D11_SDK_VERSION,
                                               direct3DDevice.resetAndGetPointerAddress(),
                                               nullptr,
                                               nullptr); FAILED (hr))
        {
            return;
        }

        if (const auto hr = direct3DDevice->QueryInterface (dxgiDevice.resetAndGetPointerAddress()); FAILED (hr))
            return;

        if (const auto hr = d2dFactory->CreateDevice (dxgiDevice, direct2DDevice.resetAndGetPointerAddress()); FAILED (hr))
            return;
    }

    void release()
    {
        direct2DDevice = nullptr;
        dxgiDevice = nullptr;
        dxgiOutputs.clear();
        dxgiAdapter = nullptr;
        direct3DDevice = nullptr; // release the Direct3D device after the adapter to avoid an exception with AMD
    }

    bool uniqueIDMatches (ReferenceCountedObjectPtr<DxgiAdapter> other) const
    {
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

            const auto adapter = adapterArray.add (new DxgiAdapter { d2dFactory, dxgiAdapter });
            listeners.call ([adapter] (DxgiAdapterListener& l) { l.adapterCreated (adapter); });
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

    static D2D1_COLOR_F toCOLOR_F (Colour c)
    {
        return { c.getFloatRed(), c.getFloatGreen(), c.getFloatBlue(), c.getFloatAlpha() };
    }

    static D2D1::Matrix3x2F transformToMatrix (const AffineTransform& transform)
    {
        return { transform.mat00, transform.mat10, transform.mat01, transform.mat11, transform.mat02, transform.mat12 };
    }
};

} // namespace juce
