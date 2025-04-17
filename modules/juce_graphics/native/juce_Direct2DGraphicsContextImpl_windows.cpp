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

bool Direct2DGraphicsContext::Pimpl::prepare()
{
    if (! deviceResources.has_value())
        deviceResources = Direct2DDeviceResources::create (getDeviceContext());

    return deviceResources.has_value();
}

void Direct2DGraphicsContext::Pimpl::teardown()
{
    deviceResources.reset();
}

bool Direct2DGraphicsContext::Pimpl::checkPaintReady()
{
    return deviceResources.has_value();
}

Direct2DGraphicsContext::Pimpl::Pimpl (Direct2DGraphicsContext& ownerIn)
    : owner (ownerIn)
{
    directX->adapters.addListener (*this);
}

Direct2DGraphicsContext::Pimpl::~Pimpl()
{
    directX->adapters.removeListener (*this);

    popAllSavedStates();
}

auto Direct2DGraphicsContext::Pimpl::startFrame() -> SavedState*
{
    prepare();

    // Anything to paint?
    const auto paintAreas = getPaintAreas();
    const auto paintBounds = paintAreas.getBounds();

    if (! getFrameSize().intersects (paintBounds) || paintBounds.isEmpty() || paintAreas.isEmpty())
        return nullptr;

    // Is Direct2D ready to paint?
    if (! checkPaintReady())
        return nullptr;

   #if JUCE_DIRECT2D_METRICS
    owner.metrics->startFrame();
   #endif

    JUCE_TRACE_EVENT_INT_RECT_LIST (etw::startD2DFrame, etw::direct2dKeyword, owner.getFrameId(), paintAreas);

    const auto deviceContext = getDeviceContext();

    // Init device context transform
    resetTransform (deviceContext);

    // Start drawing
    deviceContext->SetTarget (getDeviceContextTarget());
    deviceContext->BeginDraw();

    // Init the save state stack and return the first saved state
    return pushFirstSavedState (paintBounds);
}

HRESULT Direct2DGraphicsContext::Pimpl::finishFrame()
{
    // Fully pop the state stack
    popAllSavedStates();

    // Finish drawing
    // SetTarget(nullptr) so the device context doesn't hold a reference to the swap chain buffer
    HRESULT hr = S_OK;
    {
        JUCE_D2DMETRICS_SCOPED_ELAPSED_TIME (owner.metrics, endDrawDuration)
        JUCE_SCOPED_TRACE_EVENT_FRAME (etw::endDraw, etw::direct2dKeyword, owner.getFrameId());

        const auto deviceContext = getDeviceContext();
        hr = deviceContext->EndDraw();
        deviceContext->SetTarget (nullptr);
    }

    jassert (SUCCEEDED (hr));

    if (FAILED (hr))
        teardown();

   #if JUCE_DIRECT2D_METRICS
    owner.metrics->finishFrame();
   #endif

    return hr;
}

auto Direct2DGraphicsContext::Pimpl::getCurrentSavedState() const -> SavedState*
{
    return ! savedClientStates.empty() ? savedClientStates.back().get() : nullptr;
}

auto Direct2DGraphicsContext::Pimpl::pushFirstSavedState (Rectangle<int> initialClipRegion) -> SavedState*
{
    jassert (savedClientStates.empty());

    savedClientStates.push_back (std::make_unique<SavedState> (owner,
                                                               initialClipRegion,
                                                               getDeviceContext(),
                                                               deviceResources->colourBrush,
                                                               *deviceResources));

    return getCurrentSavedState();
}

auto Direct2DGraphicsContext::Pimpl::pushSavedState() -> SavedState*
{
    jassert (! savedClientStates.empty());

    savedClientStates.push_back (std::make_unique<SavedState> (*savedClientStates.back()));

    return getCurrentSavedState();
}

auto Direct2DGraphicsContext::Pimpl::popSavedState() -> SavedState*
{
    savedClientStates.back()->popLayers();
    savedClientStates.pop_back();

    return getCurrentSavedState();
}

void Direct2DGraphicsContext::Pimpl::popAllSavedStates()
{
    while (! savedClientStates.empty())
        popSavedState();
}

void Direct2DGraphicsContext::Pimpl::setDeviceContextTransform (AffineTransform transform)
{
    setTransform (getDeviceContext(), transform);
}

void Direct2DGraphicsContext::Pimpl::resetDeviceContextTransform()
{
    resetTransform (getDeviceContext());
}

bool Direct2DGraphicsContext::Pimpl::fillSpriteBatch (const RectangleList<float>& list)
{
    if (! owner.currentState->fillType.isColour())
        return false;

    auto* rectangleListSpriteBatch = deviceResources->rectangleListSpriteBatch.get();

    if (rectangleListSpriteBatch == nullptr)
        return false;

    const auto deviceContext = getDeviceContext();

    if (deviceContext == nullptr)
        return false;

    owner.applyPendingClipList();

    const auto& transform = owner.currentState->currentTransform;

    if (transform.isOnlyTranslated)
    {
        auto translateRectangle = [&] (const Rectangle<float>& r) -> Rectangle<float>
        {
            return transform.translated (r);
        };

        return rectangleListSpriteBatch->fillRectangles (deviceContext,
                                                         list,
                                                         owner.currentState->fillType.colour,
                                                         translateRectangle,
                                                         owner.metrics.get());
    }

    if (owner.currentState->isCurrentTransformAxisAligned())
    {
        auto transformRectangle = [&] (const Rectangle<float>& r) -> Rectangle<float>
        {
            return transform.boundsAfterTransform (r);
        };

        return rectangleListSpriteBatch->fillRectangles (deviceContext,
                                                         list,
                                                         owner.currentState->fillType.colour,
                                                         transformRectangle,
                                                         owner.metrics.get());
    }

    auto checkRectangleWithoutTransforming = [&] (const Rectangle<float>& r) -> Rectangle<float>
    {
        return r;
    };

    ScopedTransform scopedTransform { *this, owner.currentState };
    return rectangleListSpriteBatch->fillRectangles (deviceContext,
                                                     list,
                                                     owner.currentState->fillType.colour,
                                                     checkRectangleWithoutTransforming,
                                                     owner.metrics.get());
}

Line<float> Direct2DGraphicsContext::Pimpl::offsetShape (Line<float> a, Point<float> b)
{
    return Line<float> { a.getStart() + b, a.getEnd() + b };
}

Rectangle<float> Direct2DGraphicsContext::Pimpl::offsetShape (Rectangle<float> a, Point<float> b)
{
    return a + b;
}

RectangleList<float> Direct2DGraphicsContext::Pimpl::offsetShape (RectangleList<float> a, Point<float> b)
{
    a.offsetAll (b);
    return a;
}

void Direct2DGraphicsContext::Pimpl::resetTransform (ID2D1DeviceContext1* context)
{
    context->SetTransform (D2D1::IdentityMatrix());
}

void Direct2DGraphicsContext::Pimpl::setTransform (ID2D1DeviceContext1* context, AffineTransform newTransform)
{
    context->SetTransform (D2DUtilities::transformToMatrix (newTransform));
}

DxgiAdapter::Ptr Direct2DGraphicsContext::Pimpl::findAdapter() const
{
    return Direct2DDeviceResources::findAdapter (directX->adapters, getDeviceContext());
}

void Direct2DGraphicsContext::Pimpl::adapterCreated (DxgiAdapter::Ptr newAdapter)
{
    const auto adapter = findAdapter();

    if (adapter == nullptr || ! adapter->uniqueIDMatches (newAdapter))
        teardown();
}

void Direct2DGraphicsContext::Pimpl::adapterRemoved (DxgiAdapter::Ptr expiringAdapter)
{
    const auto adapter = findAdapter();

    if (adapter != nullptr && adapter->uniqueIDMatches (expiringAdapter))
        teardown();
}

} // namespace juce
