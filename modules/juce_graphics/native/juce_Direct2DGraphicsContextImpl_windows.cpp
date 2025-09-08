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

struct ScopedBlendCopy
{
    explicit ScopedBlendCopy (ComSmartPtr<ID2D1DeviceContext1> c)
        : ctx (c)
    {
        ctx->SetPrimitiveBlend (D2D1_PRIMITIVE_BLEND_COPY);
    }

    ~ScopedBlendCopy()
    {
        ctx->SetPrimitiveBlend (blend);
    }

    ComSmartPtr<ID2D1DeviceContext1> ctx;
    D2D1_PRIMITIVE_BLEND blend = ctx->GetPrimitiveBlend();
};

class PushedLayers
{
public:
    PushedLayers() { pushedLayers.reserve (32); }
    PushedLayers (const PushedLayers&) { pushedLayers.reserve (32); }

   #if JUCE_DEBUG
    ~PushedLayers()
    {
        jassert (pushedLayers.empty());
    }
   #endif

    void push (ComSmartPtr<ID2D1DeviceContext1> context, const D2D1_LAYER_PARAMETERS1& layerParameters)
    {
        pushedLayers.emplace_back (OwningLayer { layerParameters });
        pushedLayers.back().push (context);
    }

    void push (ComSmartPtr<ID2D1DeviceContext1> context, const Rectangle<float>& r)
    {
        pushedLayers.emplace_back (r);
        pushedLayers.back().push (context);
    }

    void popOne (ComSmartPtr<ID2D1DeviceContext1> context)
    {
        if (pushedLayers.empty())
            return;

        pushedLayers.back().pop (context);
        pushedLayers.pop_back();
    }

    bool isEmpty() const
    {
        return pushedLayers.empty();
    }

    void fillGeometryWithNoLayersActive (ComSmartPtr<ID2D1DeviceContext1> ctx,
                                         ComSmartPtr<ID2D1Geometry> geo,
                                         ComSmartPtr<ID2D1Brush> brush)
    {
        ComSmartPtr<ID2D1Factory> factory;
        ctx->GetFactory (factory.resetAndGetPointerAddress());

        const auto hasGeoLayer = std::any_of (pushedLayers.begin(),
                                              pushedLayers.end(),
                                              [] (const auto& x) { return std::holds_alternative<OwningLayer> (x.var); });

        const auto intersection = [&]() -> ComSmartPtr<ID2D1Geometry>
        {
            if (! hasGeoLayer)
                return {};

            const auto contextSize = ctx->GetPixelSize();

            ComSmartPtr<ID2D1RectangleGeometry> rect;
            factory->CreateRectangleGeometry (D2D1::RectF (0.0f,
                                                           0.0f,
                                                           (float) contextSize.width,
                                                           (float) contextSize.height),
                                              rect.resetAndGetPointerAddress());

            ComSmartPtr<ID2D1Geometry> clip = rect;

            for (const auto& layer : pushedLayers)
            {
                ScopedGeometryWithSink scope { factory, D2D1_FILL_MODE_WINDING };

                if (auto* l = std::get_if<OwningLayer> (&layer.var))
                {
                    clip->CombineWithGeometry (l->geometry,
                                               D2D1_COMBINE_MODE_INTERSECT,
                                               l->params.maskTransform,
                                               scope.sink);
                }
                else if (auto* r = std::get_if<Rectangle<float>> (&layer.var))
                {
                    ComSmartPtr<ID2D1RectangleGeometry> temporaryRect;
                    factory->CreateRectangleGeometry (D2DUtilities::toRECT_F (*r),
                                                      temporaryRect.resetAndGetPointerAddress());
                    clip->CombineWithGeometry (temporaryRect,
                                               D2D1_COMBINE_MODE_INTERSECT,
                                               D2D1::Matrix3x2F::Identity(),
                                               scope.sink);
                }

                clip = scope.geometry;
            }

            return clip;
        }();

        const auto clipWithGeo = [&]() -> ComSmartPtr<ID2D1Geometry>
        {
            if (intersection == nullptr)
                return geo;

            ScopedGeometryWithSink scope { factory, D2D1_FILL_MODE_WINDING };
            intersection->CombineWithGeometry (geo,
                                               D2D1_COMBINE_MODE_INTERSECT,
                                               D2D1::Matrix3x2F::Identity(),
                                               scope.sink);
            return scope.geometry;
        }();

        if (intersection != nullptr)
        {
            std::for_each (pushedLayers.rbegin(),
                           pushedLayers.rend(),
                           [&] (const auto& layer) { layer.pop (ctx); });
        }

        {
            const ScopedBlendCopy scope { ctx };
            ctx->FillGeometry (clipWithGeo, brush);
        }

        if (intersection != nullptr)
        {
            pushedLayers.clear();

            auto newLayer = D2D1::LayerParameters1();
            newLayer.geometricMask = intersection;
            push (ctx, newLayer);
        }
    }

private:
    struct OwningLayer
    {
        explicit OwningLayer (const D2D1_LAYER_PARAMETERS1& p) : params (p) {}

        D2D1_LAYER_PARAMETERS1 params;
        ComSmartPtr<ID2D1Geometry> geometry = params.geometricMask != nullptr ? addComSmartPtrOwner (params.geometricMask) : nullptr;
        ComSmartPtr<ID2D1Brush> brush = params.opacityBrush != nullptr ? addComSmartPtrOwner (params.opacityBrush) : nullptr;
    };

    struct Layer
    {
        explicit Layer (std::variant<OwningLayer, Rectangle<float>> v) : var (std::move (v)) {}

        void push (ComSmartPtr<ID2D1DeviceContext1> context) const
        {
            if (auto* layer = std::get_if<OwningLayer> (&var))
                context->PushLayer (layer->params, nullptr);
            else if (auto* rect = std::get_if<Rectangle<float>> (&var))
                context->PushAxisAlignedClip (D2DUtilities::toRECT_F (*rect), D2D1_ANTIALIAS_MODE_ALIASED);
        }

        void pop (ComSmartPtr<ID2D1DeviceContext1> context) const
        {
            if (std::holds_alternative<OwningLayer> (var))
                context->PopLayer();
            else if (std::holds_alternative<Rectangle<float>> (var))
                context->PopAxisAlignedClip();
        }

        std::variant<OwningLayer, Rectangle<float>> var;
    };

    std::vector<Layer> pushedLayers;

    //==============================================================================
    // PushedLayer represents a Direct2D clipping or transparency layer
    //
    // D2D layers have to be pushed into the device context. Every push has to be
    // matched with a pop.
    //
    // D2D has special layers called "axis aligned clip layers" which clip to an
    // axis-aligned rectangle. Pushing an axis-aligned clip layer must be matched
    // with a call to deviceContext->PopAxisAlignedClip() in the reverse order
    // in which the layers were pushed.
    //
    // So if the pushed layer stack is built like this:
    //
    // PushLayer()
    // PushLayer()
    // PushAxisAlignedClip()
    // PushLayer()
    //
    // the layer stack must be popped like this:
    //
    // PopLayer()
    // PopAxisAlignedClip()
    // PopLayer()
    // PopLayer()
    //
    // PushedLayer, PushedAxisAlignedClipLayer, and LayerPopper all exist just to unwind the
    // layer stack accordingly.
};

struct PagesAndArea
{
    Image imageHandle;
    Span<const Direct2DPixelDataPage> pages;
    Rectangle<int> area;

    static PagesAndArea make (const Image& image, ComSmartPtr<ID2D1Device1> device)
    {
        using GetImage = Image (*) (const Image&);
        constexpr GetImage converters[] { [] (const Image& i) { return i; },
                                          [] (const Image& i) { return NativeImageType{}.convert (i); } };

        for (auto* getImage : converters)
        {
            const auto converted = getImage (image);
            const auto native = converted.getPixelData()->getNativeExtensions();

            if (auto pages = native.getPages (device); ! pages.empty())
                return PagesAndArea { converted, std::move (pages), converted.getBounds().withPosition (native.getTopLeft()) };
        }

        // Not sure how this could happen unless the NativeImageType no longer provides Windows native details...
        jassertfalse;
        return {};
    }
};

struct Direct2DGraphicsContext::SavedState
{
public:
    // Constructor for first stack entry
    SavedState (Direct2DGraphicsContext& ownerIn,
                Rectangle<int> frameSizeIn,
                ComSmartPtr<ID2D1DeviceContext1> deviceContext,
                ComSmartPtr<ID2D1SolidColorBrush>& colourBrushIn,
                Direct2DDeviceResources& deviceResourcesIn)
        : owner (ownerIn),
          context (deviceContext),
          currentBrush (colourBrushIn),
          colourBrush (colourBrushIn),
          deviceResources (deviceResourcesIn),
          deviceSpaceClipList (frameSizeIn.toFloat())
    {
    }

    void pushLayer (const D2D1_LAYER_PARAMETERS1& layerParameters)
    {
        layers.push (context, layerParameters);
    }

    void pushGeometryClipLayer (ComSmartPtr<ID2D1Geometry> geometry)
    {
        if (geometry != nullptr)
            pushLayer (D2D1::LayerParameters1 (D2D1::InfiniteRect(), geometry));
    }

    void pushTransformedRectangleGeometryClipLayer (ComSmartPtr<ID2D1RectangleGeometry> geometry, const AffineTransform& transform)
    {
        JUCE_D2DMETRICS_SCOPED_ELAPSED_TIME (owner.metrics, pushGeometryLayerTime)

        jassert (geometry != nullptr);
        auto layerParameters = D2D1::LayerParameters1 (D2D1::InfiniteRect(), geometry);
        layerParameters.maskTransform = D2DUtilities::transformToMatrix (transform);
        pushLayer (layerParameters);
    }

    void pushAliasedAxisAlignedClipLayer (const Rectangle<float>& r)
    {
        JUCE_D2DMETRICS_SCOPED_ELAPSED_TIME (owner.metrics, pushAliasedAxisAlignedLayerTime)

        layers.push (context, r);
    }

    void pushTransparencyLayer (float opacity)
    {
        pushLayer ({ D2D1::InfiniteRect(), nullptr, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE, D2D1::IdentityMatrix(), opacity, {}, {} });
    }

    void popLayers()
    {
        while (! layers.isEmpty())
            layers.popOne (context);
    }

    void popTopLayer()
    {
        layers.popOne (context);
    }

    void setFont (const Font& newFont)
    {
        font = newFont;
    }

    void setOpacity (float newOpacity)
    {
        fillType.setOpacity (newOpacity);
    }

    void clearFill()
    {
        linearGradient = nullptr;
        radialGradient = nullptr;
        bitmapBrush = nullptr;
        currentBrush = nullptr;
    }

    /** Translate a JUCE FillType to a Direct2D brush */
    void updateCurrentBrush()
    {
        if (fillType.isColour())
        {
            // Reuse the same colour brush
            currentBrush = colourBrush;
        }
        else if (fillType.isTiledImage())
        {
            if (fillType.image.isNull())
                return;

            const auto device = D2DUtilities::getDeviceForContext (context);
            const auto imageFormat = fillType.image.getFormat();
            const auto targetFormat = imageFormat == Image::SingleChannel ? Image::ARGB : imageFormat;
            const auto pagesAndArea = PagesAndArea::make (fillType.image.convertedToFormat (targetFormat), device);

            if (pagesAndArea.pages.empty())
                return;

            const auto bitmap = pagesAndArea.pages.front().bitmap;

            if (bitmap == nullptr)
                return;

            D2D1_BRUSH_PROPERTIES brushProps { fillType.getOpacity(), D2DUtilities::transformToMatrix (fillType.transform) };
            auto bmProps = D2D1::BitmapBrushProperties (D2D1_EXTEND_MODE_WRAP, D2D1_EXTEND_MODE_WRAP);
            const auto hr = context->CreateBitmapBrush (bitmap,
                                                        bmProps,
                                                        brushProps,
                                                        bitmapBrush.resetAndGetPointerAddress());

            if (FAILED (hr))
                return;

            currentBrush = bitmapBrush;
        }
        else if (fillType.isGradient())
        {
            if (fillType.gradient->isRadial)
            {
                radialGradient = deviceResources.radialGradientCache.get (*fillType.gradient, context, owner.metrics.get());
                currentBrush = radialGradient;
            }
            else
            {
                linearGradient = deviceResources.linearGradientCache.get (*fillType.gradient, context, owner.metrics.get());
                currentBrush = linearGradient;
            }
        }

        updateColourBrush();
    }

    void updateColourBrush()
    {
        if (colourBrush && fillType.isColour())
        {
            auto colour = D2DUtilities::toCOLOR_F (fillType.colour);
            colourBrush->SetColor (colour);
        }
    }

    enum BrushTransformFlags
    {
        noTransforms = 0,
        applyWorldTransform = 1,
        applyFillTypeTransform = 2,
        applyWorldAndFillTypeTransforms = applyFillTypeTransform | applyWorldTransform
    };

    ComSmartPtr<ID2D1Brush> getBrush (int flags = applyWorldAndFillTypeTransforms)
    {
        if (fillType.isInvisible())
            return nullptr;

        if (! fillType.isGradient() && ! fillType.isTiledImage())
            return currentBrush;

        AffineTransform transform{};

        if ((flags & BrushTransformFlags::applyWorldTransform) != 0)
            transform = currentTransform.getTransform();

        if ((flags & BrushTransformFlags::applyFillTypeTransform) != 0)
            transform = fillType.transform.followedBy (transform);

        if (fillType.isGradient())
        {
            const auto p1 = fillType.gradient->point1;
            const auto p2 = fillType.gradient->point2;

            if (fillType.gradient->isRadial)
            {
                const auto radius = p2.getDistanceFrom (p1);
                radialGradient->SetRadiusX (radius);
                radialGradient->SetRadiusY (radius);
                radialGradient->SetCenter ({ p1.x, p1.y });
            }
            else
            {
                linearGradient->SetStartPoint ({ p1.x, p1.y });
                linearGradient->SetEndPoint ({ p2.x, p2.y });
            }
        }

        currentBrush->SetTransform (D2DUtilities::transformToMatrix (transform));
        currentBrush->SetOpacity (fillType.getOpacity());

        return currentBrush;
    }

    bool doesIntersectClipList (Rectangle<int> r) const noexcept
    {
        return deviceSpaceClipList.intersects (r.toFloat());
    }

    bool doesIntersectClipList (Rectangle<float> r) const noexcept
    {
        return deviceSpaceClipList.intersects (r);
    }

    bool doesIntersectClipList (Line<float> r) const noexcept
    {
        return doesIntersectClipList (Rectangle { r.getStart(), r.getEnd() }.expanded (1.0f));
    }

    bool doesIntersectClipList (const RectangleList<float>& other) const noexcept
    {
        return deviceSpaceClipList.intersects (other);
    }

    bool isCurrentTransformAxisAligned() const noexcept
    {
        return currentTransform.isOnlyTranslated || (currentTransform.complexTransform.mat01 == 0.0f && currentTransform.complexTransform.mat10 == 0.0f);
    }

    static String toString (const RenderingHelpers::TranslationOrTransform& t)
    {
        String s;
        s << "Offset " << t.offset.toString() << newLine;
        s << "Transform " << t.complexTransform.mat00 << " " << t.complexTransform.mat01 << " " << t.complexTransform.mat02 << " / ";
        s << "          " << t.complexTransform.mat10 << " " << t.complexTransform.mat11 << " " << t.complexTransform.mat12 << newLine;
        return s;
    }

    PushedLayers layers;

    Direct2DGraphicsContext& owner;

    ComSmartPtr<ID2D1DeviceContext1> context;
    ComSmartPtr<ID2D1Brush> currentBrush;
    ComSmartPtr<ID2D1SolidColorBrush>& colourBrush; // reference to shared colour brush
    ComSmartPtr<ID2D1BitmapBrush> bitmapBrush;
    ComSmartPtr<ID2D1LinearGradientBrush> linearGradient;
    ComSmartPtr<ID2D1RadialGradientBrush> radialGradient;

    RenderingHelpers::TranslationOrTransform currentTransform;

    Direct2DDeviceResources& deviceResources;
    RectangleList<float> deviceSpaceClipList;

    Font font { FontOptions{} };

    FillType fillType;

    D2D1_INTERPOLATION_MODE interpolationMode = D2D1_INTERPOLATION_MODE_LINEAR;

    JUCE_LEAK_DETECTOR (SavedState)
};

//==============================================================================
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
