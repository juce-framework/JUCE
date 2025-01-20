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

struct Direct2DHwndContext::HwndPimpl : public Direct2DGraphicsContext::Pimpl
{
private:
    struct SwapChainThread : private AsyncUpdater
    {
        SwapChainThread (Direct2DHwndContext::HwndPimpl& ownerIn, HANDLE swapHandle)
            : owner (ownerIn),
              swapChainEventHandle (swapHandle)
        {
        }

        ~SwapChainThread() override
        {
            cancelPendingUpdate();
            SetEvent (quitEvent.getHandle());
            thread.join();
        }

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SwapChainThread)

    private:
        Direct2DHwndContext::HwndPimpl& owner;
        HANDLE swapChainEventHandle = nullptr;

        WindowsScopedEvent quitEvent;
        std::thread thread { [&] { threadLoop(); } };

        void handleAsyncUpdate() override
        {
            owner.swapEventReceived = true;
            owner.present();
        }

        void threadLoop()
        {
            Thread::setCurrentThreadName ("JUCE D2D swap chain thread");

            for (;;)
            {
                const HANDLE handles[] { swapChainEventHandle, quitEvent.getHandle() };

                const auto waitResult = WaitForMultipleObjects ((DWORD) std::size (handles), handles, FALSE, INFINITE);

                switch (waitResult)
                {
                    case WAIT_OBJECT_0:
                    {
                        triggerAsyncUpdate();
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

    SwapChain swap;
    ComSmartPtr<ID2D1DeviceContext1> deviceContext;
    std::unique_ptr<SwapChainThread> swapChainThread;
    std::optional<CompositionTree> compositionTree;

    // Areas that must be repainted during the next paint call, between startFrame/endFrame
    RectangleList<int> deferredRepaints;

    // Areas that have been updated in the backbuffer, but not presented
    RectangleList<int> dirtyRegionsInBackBuffer;

    std::vector<RECT> dirtyRectangles;
    int64 lastFinishFrameTicks = 0;
    HWND hwnd = nullptr;

    // Set to true after the swap event is signalled, indicating that we're allowed to try presenting
    // a new frame.
    bool swapEventReceived = false;

    bool prepare() override
    {
        const auto adapter = directX->adapters.getAdapterForHwnd (hwnd);

        if (adapter == nullptr)
            return false;

        if (deviceContext == nullptr)
            deviceContext = Direct2DDeviceContext::create (adapter);

        if (deviceContext == nullptr)
            return false;

        if (! deviceResources.has_value())
            deviceResources = Direct2DDeviceResources::create (deviceContext);

        if (! deviceResources.has_value())
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
        // Try not to saturate the message thread; this is a little crude. Perhaps some kind of credit system...
        if (auto now = Time::getHighResolutionTicks(); Time::highResolutionTicksToSeconds (now - lastFinishFrameTicks) < 0.001)
            return false;

        bool ready = Pimpl::checkPaintReady();
        ready &= swap.canPaint();
        ready &= compositionTree.has_value();

        return ready;
    }

    JUCE_DECLARE_WEAK_REFERENCEABLE (HwndPimpl)

public:
    HwndPimpl (Direct2DHwndContext& ownerIn, HWND hwndIn)
        : Pimpl (ownerIn),
          hwnd (hwndIn)
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

        return Rectangle<int>::leftTopRightBottom (clientRect.left, clientRect.top, clientRect.right, clientRect.bottom);
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

        JUCE_TRACE_EVENT_INT_RECT (etw::repaint, etw::paintKeyword, snappedRectangle);
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

        JUCE_TRACE_LOG_D2D_PAINT_CALL (etw::direct2dHwndPaintStart, owner.getFrameId());

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
        JUCE_D2DMETRICS_SCOPED_ELAPSED_TIME (owner.metrics, present1Duration);

        if (swap.getBuffer() == nullptr || dirtyRegionsInBackBuffer.isEmpty() || ! swapEventReceived)
            return;

        auto const swapChainSize = swap.getSize();
        DXGI_PRESENT_PARAMETERS presentParameters{};

        if (! dirtyRegionsInBackBuffer.containsRectangle (swapChainSize))
        {
            // Allocate enough memory for the array of dirty rectangles
            dirtyRectangles.resize ((size_t) dirtyRegionsInBackBuffer.getNumRectangles());

            // Fill the array of dirty rectangles, intersecting each paint area with the swap chain buffer
            presentParameters.pDirtyRects = dirtyRectangles.data();
            presentParameters.DirtyRectsCount = 0;

            for (const auto& area : dirtyRegionsInBackBuffer)
            {
                if (const auto intersection = area.getIntersection (swapChainSize); ! intersection.isEmpty())
                    presentParameters.pDirtyRects[presentParameters.DirtyRectsCount++] = D2DUtilities::toRECT (intersection);
            }
        }

        // Present the freshly painted buffer
        const auto hr = swap.getChain()->Present1 (swap.presentSyncInterval, swap.presentFlags, &presentParameters);
        jassertquiet (SUCCEEDED (hr));

        if (FAILED (hr))
            return;

        // We managed to present a frame, so we should avoid rendering anything or calling
        // present again until that frame has been shown on-screen.
        swapEventReceived = false;

        // There's nothing waiting to be displayed in the backbuffer.
        dirtyRegionsInBackBuffer.clear();

        JUCE_TRACE_LOG_D2D_PAINT_CALL (etw::direct2dHwndPaintEnd, owner.getFrameId());
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

        if (const auto hr = deviceContext->CreateBitmap (size, nullptr, 0, bitmapProperties, snapshot.resetAndGetPointerAddress()); FAILED (hr))
            return {};

        swap.getChain()->Present (0, DXGI_PRESENT_DO_NOT_WAIT);

        // Copy the swap chain buffer to the bitmap snapshot
        D2D_POINT_2U p { 0, 0 };
        const auto sourceRect = D2DUtilities::toRECT_U (swapRect);

        if (const auto hr = snapshot->CopyFromBitmap (&p, buffer, &sourceRect); FAILED (hr))
            return {};

        const Image result { new Direct2DPixelData { D2DUtilities::getDeviceForContext (deviceContext), snapshot } };

        swap.getChain()->Present (0, DXGI_PRESENT_DO_NOT_WAIT);

        return result;
    }
};

//==============================================================================
Direct2DHwndContext::Direct2DHwndContext (HWND windowHandle)
{
   #if JUCE_DIRECT2D_METRICS
    metrics = new Direct2DMetrics { Direct2DMetricsHub::getInstance()->lock,
                                    "HWND " + String::toHexString ((pointer_sized_int) windowHandle),
                                    windowHandle };
    Direct2DMetricsHub::getInstance()->add (metrics);
   #endif

    pimpl = std::make_unique<HwndPimpl> (*this, windowHandle);
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
