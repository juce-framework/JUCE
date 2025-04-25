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
    static std::optional<CompositionTree> create (IDXGIDevice* dxgiDevice,
                                                  HWND hwnd,
                                                  IDXGISwapChain1* swapChain)
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

//==============================================================================
struct Direct2DHwndContext::HwndPimpl : public Direct2DGraphicsContext::Pimpl
{
private:
    struct SwapChainThread
    {
        SwapChainThread (Direct2DHwndContext::HwndPimpl& ownerIn, HANDLE swapHandle)
            : owner (ownerIn),
              swapChainEventHandle (swapHandle)
        {
            SetWindowSubclass (owner.hwnd, subclassWindowProc, (UINT_PTR) this, (DWORD_PTR) this);
        }

        ~SwapChainThread()
        {
            RemoveWindowSubclass (owner.hwnd, subclassWindowProc, (UINT_PTR) this);
            SetEvent (quitEvent.getHandle());
            thread.join();
        }

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SwapChainThread)

    private:
        Direct2DHwndContext::HwndPimpl& owner;
        HANDLE swapChainEventHandle = nullptr;

        WindowsScopedEvent quitEvent;
        std::thread thread { [&] { threadLoop(); } };

        static constexpr uint32_t swapchainReadyMessageID = WM_USER + 124;

        bool handleWindowProcMessage (UINT message)
        {
            if (message == swapchainReadyMessageID)
            {
                owner.onSwapchainEvent();
                return true;
            }

            return false;
        }

        static LRESULT CALLBACK subclassWindowProc (HWND hwnd,
                                                    UINT message,
                                                    WPARAM wParam,
                                                    LPARAM lParam,
                                                    UINT_PTR,
                                                    DWORD_PTR referenceData)
        {
            auto* that = reinterpret_cast<SwapChainThread*> (referenceData);

            if (that != nullptr && that->handleWindowProcMessage (message))
                return 0;

            return DefSubclassProc (hwnd, message, wParam, lParam);
        }

        void threadLoop()
        {
            Thread::setCurrentThreadName ("JUCE D2D swap chain thread");

            for (;;)
            {
                const HANDLE handles[] { swapChainEventHandle, quitEvent.getHandle() };

                const auto waitResult = WaitForMultipleObjects ((DWORD) std::size (handles),
                                                                handles,
                                                                FALSE,
                                                                INFINITE);

                switch (waitResult)
                {
                    case WAIT_OBJECT_0:
                    {
                        PostMessage (owner.hwnd, swapchainReadyMessageID, 0, 0);
                        break;
                    }

                    case WAIT_OBJECT_0 + 1:
                        return;

                    case WAIT_FAILED:
                    default:
                        jassertfalse;
                        break;
                }
            }
        }
    };

    HWND hwnd;
    SwapChain swap;
    ComSmartPtr<ID2D1DeviceContext1> deviceContext;
    std::unique_ptr<SwapChainThread> swapChainThread;
    std::optional<CompositionTree> compositionTree;
    SwapchainDelegate& delegate;

    // Areas that must be repainted during the next paint call, between startFrame/endFrame
    RectangleList<int> deferredRepaints;

    // Areas that have been updated in the backbuffer, but not presented
    RectangleList<int> dirtyRegionsInBackBuffer;

    std::vector<RECT> dirtyRectangles;
    int64 lastFinishFrameTicks = 0;

    // Set to true after the swap event is signalled, indicating that we're allowed to try presenting
    // a new frame.
    bool swapEventReceived = false;

    void onSwapchainEvent()
    {
        swapEventReceived = true;
        delegate.onSwapchainEvent();
    }

    bool prepare() override
    {
        const auto adapter = getDefaultAdapter();

        if (adapter == nullptr)
            return false;

        if (deviceContext == nullptr)
            deviceContext = Direct2DDeviceContext::create (adapter);

        if (deviceContext == nullptr)
            return false;

        if (! Pimpl::prepare())
            return false;

        if (! hwnd || getClientRect().isEmpty())
            return false;

        if (! swap.canPaint())
        {
            if (auto hr = swap.create (hwnd, getClientRect(), adapter); FAILED (hr))
                return false;
        }

        if (swapChainThread == nullptr)
            if (auto* e = swap.getEvent())
                swapChainThread = std::make_unique<SwapChainThread> (*this, e->getHandle());

        if (! compositionTree.has_value())
            compositionTree = CompositionTree::create (adapter->dxgiDevice, hwnd, swap.getChain());

        if (! compositionTree.has_value())
            return false;

        return true;
    }

    void teardown() override
    {
        compositionTree.reset();
        swapChainThread = nullptr;
        deviceContext = nullptr;
        swap = {};

        Pimpl::teardown();
    }

    RectangleList<int> getPaintAreas() const override
    {
        return deferredRepaints;
    }

    bool checkPaintReady() override
    {
        const auto now = Time::getHighResolutionTicks();

        // Try not to saturate the message thread; this is a little crude. Perhaps some kind of credit system...
        if (Time::highResolutionTicksToSeconds (now - lastFinishFrameTicks) < 0.001)
            return false;

        bool ready = Pimpl::checkPaintReady();
        ready &= swap.canPaint();
        ready &= compositionTree.has_value();
        ready &= swapEventReceived;

        return ready;
    }

    JUCE_DECLARE_WEAK_REFERENCEABLE (HwndPimpl)

public:
    HwndPimpl (Direct2DHwndContext& ownerIn, HWND hwndIn, SwapchainDelegate& swapDelegate)
        : Pimpl (ownerIn),
          hwnd (hwndIn),
          delegate (swapDelegate)
    {
    }

    ~HwndPimpl() override = default;

    void handleShowWindow()
    {
        // One of the trickier problems was determining when Direct2D & DXGI resources can be safely created;
        // that's not really spelled out in the documentation.
        // This method is called when the component peer receives WM_SHOWWINDOW
        prepare();
        deferredRepaints = getClientRect();
    }

    Rectangle<int> getClientRect() const
    {
        RECT clientRect;
        GetClientRect (hwnd, &clientRect);

        return Rectangle<int>::leftTopRightBottom (clientRect.left,
                                                   clientRect.top,
                                                   clientRect.right,
                                                   clientRect.bottom);
    }

    Rectangle<int> getFrameSize() const override
    {
        return getClientRect();
    }

    ComSmartPtr<ID2D1DeviceContext1> getDeviceContext() const override
    {
        return deviceContext;
    }

    ComSmartPtr<ID2D1Image> getDeviceContextTarget() const override
    {
        return swap.getBuffer();
    }

    void setSize (Rectangle<int> size)
    {
        if (size == swap.getSize() || size.isEmpty())
            return;

        // Require the entire window to be repainted
        deferredRepaints = size;

        // The backbuffer has no valid content until we paint a full frame
        dirtyRegionsInBackBuffer.clear();

        InvalidateRect (hwnd, nullptr, TRUE);

        // Resize/scale the swap chain
        prepare();

        auto hr = swap.resize (size);
        jassert (SUCCEEDED (hr));
        if (FAILED (hr))
            teardown();
    }

    void addDeferredRepaint (Rectangle<int> deferredRepaint)
    {
        deferredRepaints.add (deferredRepaint);

        JUCE_TRACE_EVENT_INT_RECT (etw::repaint, etw::paintKeyword, deferredRepaint);
    }

    SavedState* startFrame() override
    {
        setSize (getClientRect());

        auto* savedState = Pimpl::startFrame();

        if (savedState == nullptr)
            return nullptr;

        // If a new frame is starting, clear deferredAreas in case repaint is called
        // while the frame is being painted to ensure the new areas are painted on the
        // next frame
        dirtyRegionsInBackBuffer.add (deferredRepaints);
        deferredRepaints.clear();

        JUCE_TRACE_LOG_D2D_PAINT_CALL (etw::direct2dHwndPaintStart, getFrameId());

        return savedState;
    }

    HRESULT finishFrame() override
    {
        const auto result = Pimpl::finishFrame();
        present();
        lastFinishFrameTicks = Time::getHighResolutionTicks();
        return result;
    }

    void present()
    {
        JUCE_D2DMETRICS_SCOPED_ELAPSED_TIME (getMetrics(), present1Duration);

        if (swap.getBuffer() == nullptr || dirtyRegionsInBackBuffer.isEmpty() || ! swapEventReceived)
            return;

        auto const swapChainSize = swap.getSize();
        DXGI_PRESENT_PARAMETERS params{};

        if (! dirtyRegionsInBackBuffer.containsRectangle (swapChainSize))
        {
            // Allocate enough memory for the array of dirty rectangles
            dirtyRectangles.resize ((size_t) dirtyRegionsInBackBuffer.getNumRectangles());

            // Fill the array of dirty rectangles, intersecting each paint area with the swap chain buffer
            params.pDirtyRects = dirtyRectangles.data();
            params.DirtyRectsCount = 0;

            for (const auto& area : dirtyRegionsInBackBuffer)
            {
                const auto intersection = area.getIntersection (swapChainSize);

                if (! intersection.isEmpty())
                    params.pDirtyRects[params.DirtyRectsCount++] = D2DUtilities::toRECT (intersection);
            }
        }

        // Present the freshly painted buffer
        const auto hr = swap.getChain()->Present1 (swap.presentSyncInterval,
                                                   swap.presentFlags,
                                                   &params);
        jassertquiet (SUCCEEDED (hr));

        if (FAILED (hr))
            return;

        // We managed to present a frame, so we should avoid rendering anything or calling
        // present again until that frame has been shown on-screen.
        swapEventReceived = false;

        // There's nothing waiting to be displayed in the backbuffer.
        dirtyRegionsInBackBuffer.clear();

        JUCE_TRACE_LOG_D2D_PAINT_CALL (etw::direct2dHwndPaintEnd, getFrameId());
    }

    Image createSnapshot() const
    {
        JUCE_ASSERT_MESSAGE_MANAGER_IS_LOCKED

        // This won't capture child windows. Perhaps a better approach would be to use
        // IGraphicsCaptureItemInterop, although this is only supported on Windows 10 v1903+

        if (deviceContext == nullptr)
            return {};

        const auto buffer = swap.getBuffer();

        if (buffer == nullptr)
            return {};

        // Create the bitmap to receive the snapshot
        D2D1_BITMAP_PROPERTIES1 bitmapProperties{};
        bitmapProperties.bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET;
        bitmapProperties.pixelFormat = buffer->GetPixelFormat();

        const auto swapRect = swap.getSize();
        const auto size = D2D1::SizeU ((UINT32) swapRect.getWidth(), (UINT32) swapRect.getHeight());

        ComSmartPtr<ID2D1Bitmap1> snapshot;

        auto hr = deviceContext->CreateBitmap (size,
                                               nullptr,
                                               0,
                                               bitmapProperties,
                                               snapshot.resetAndGetPointerAddress());

        if (FAILED (hr))
            return {};

        swap.getChain()->Present (0, DXGI_PRESENT_DO_NOT_WAIT);

        // Copy the swap chain buffer to the bitmap snapshot
        D2D_POINT_2U p { 0, 0 };
        const auto sourceRect = D2DUtilities::toRECT_U (swapRect);

        hr = snapshot->CopyFromBitmap (&p, buffer, &sourceRect);

        if (FAILED (hr))
            return {};

        const Image result { new Direct2DPixelData { D2DUtilities::getDeviceForContext (deviceContext),
                                                     snapshot } };

        swap.getChain()->Present (0, DXGI_PRESENT_DO_NOT_WAIT);

        return result;
    }
};

//==============================================================================
Direct2DHwndContext::Direct2DHwndContext (HWND windowHandle, SwapchainDelegate& swapDelegate)
{
   #if JUCE_DIRECT2D_METRICS
    metrics = new Direct2DMetrics { Direct2DMetricsHub::getInstance()->lock,
                                    "HWND " + String::toHexString ((pointer_sized_int) windowHandle),
                                    windowHandle };
    Direct2DMetricsHub::getInstance()->add (metrics);
   #endif

    pimpl = std::make_unique<HwndPimpl> (*this, windowHandle, swapDelegate);
}

Direct2DHwndContext::~Direct2DHwndContext()
{
   #if JUCE_DIRECT2D_METRICS
    Direct2DMetricsHub::getInstance()->remove (metrics);
   #endif
}

Direct2DGraphicsContext::Pimpl* Direct2DHwndContext::getPimpl() const noexcept
{
    return pimpl.get();
}

void Direct2DHwndContext::handleShowWindow()
{
    pimpl->handleShowWindow();
}

void Direct2DHwndContext::addDeferredRepaint (Rectangle<int> deferredRepaint)
{
    pimpl->addDeferredRepaint (deferredRepaint);
}

Image Direct2DHwndContext::createSnapshot() const
{
    return pimpl->createSnapshot();
}

void Direct2DHwndContext::clearTargetBuffer()
{
    applyPendingClipList();
    pimpl->getDeviceContext()->Clear();
}

} // namespace juce
