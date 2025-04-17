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
        applyInverseWorldTransform = 2,
        applyFillTypeTransform = 4,
        applyWorldAndFillTypeTransforms = applyFillTypeTransform | applyWorldTransform
    };

    ComSmartPtr<ID2D1Brush> getBrush (int flags = applyWorldAndFillTypeTransforms)
    {
        if (fillType.isInvisible())
            return nullptr;

        if (! fillType.isGradient() && ! fillType.isTiledImage())
            return currentBrush;

        Point<float> translation{};
        AffineTransform transform{};

        if (fillType.isGradient())
        {
            if ((flags & BrushTransformFlags::applyWorldTransform) != 0)
            {
                if (currentTransform.isOnlyTranslated)
                    translation = currentTransform.offset.toFloat();
                else
                    transform = currentTransform.getTransform();
            }

            if ((flags & BrushTransformFlags::applyFillTypeTransform) != 0)
            {
                if (fillType.transform.isOnlyTranslation())
                    translation += Point (fillType.transform.getTranslationX(), fillType.transform.getTranslationY());
                else
                    transform = transform.followedBy (fillType.transform);
            }

            if ((flags & BrushTransformFlags::applyInverseWorldTransform) != 0)
            {
                if (currentTransform.isOnlyTranslated)
                    translation -= currentTransform.offset.toFloat();
                else
                    transform = transform.followedBy (currentTransform.getTransform().inverted());
            }

            const auto p1 = fillType.gradient->point1 + translation;
            const auto p2 = fillType.gradient->point2 + translation;

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
        else if (fillType.isTiledImage())
        {
            if ((flags & BrushTransformFlags::applyWorldTransform) != 0)
                transform = currentTransform.getTransform();

            if ((flags & BrushTransformFlags::applyFillTypeTransform) != 0)
                transform = transform.followedBy (fillType.transform);

            if ((flags & BrushTransformFlags::applyInverseWorldTransform) != 0)
                transform = transform.followedBy (currentTransform.getTransform().inverted());
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

struct Direct2DGraphicsContext::Pimpl : private DxgiAdapterListener
{
public:
    explicit Pimpl (Direct2DGraphicsContext& ownerIn);
    ~Pimpl() override;

    virtual SavedState* startFrame();
    virtual HRESULT finishFrame();

    virtual bool prepare();
    virtual void teardown();
    virtual bool checkPaintReady();

    virtual RectangleList<int> getPaintAreas() const = 0;
    virtual Rectangle<int> getFrameSize() const = 0;
    virtual ComSmartPtr<ID2D1DeviceContext1> getDeviceContext() const = 0;
    virtual ComSmartPtr<ID2D1Image> getDeviceContextTarget() const = 0;

    SavedState* getCurrentSavedState() const;
    SavedState* pushFirstSavedState (Rectangle<int> initialClipRegion);

    SavedState* pushSavedState();
    SavedState* popSavedState();

    void popAllSavedStates();

    void setDeviceContextTransform (AffineTransform transform);
    void resetDeviceContextTransform();

    DxgiAdapter::Ptr getDefaultAdapter() const
    {
        return directX->adapters.getDefaultAdapter();
    }

    auto getDirect2DFactory() const
    {
        return directX->getD2DFactory();
    }

    auto getDirectWriteFactory() const
    {
        return directWrite->getDWriteFactory();
    }

    auto getDirectWriteFactory4() const
    {
        return directWrite->getDWriteFactory4();
    }

    auto& getFontCollection() const
    {
        return directWrite->getFonts();
    }

    bool fillSpriteBatch (const RectangleList<float>& list);

    static Line<float> offsetShape (Line<float> a, Point<float> b);
    static Rectangle<float> offsetShape (Rectangle<float> a, Point<float> b);
    static RectangleList<float> offsetShape (RectangleList<float> a, Point<float> b);

    template <typename Shape, typename Fn>
    void paintPrimitive (const Shape& shape, Fn&& primitiveOp)
    {
        const auto& transform = owner.currentState->currentTransform;

        owner.applyPendingClipList();

        auto deviceContext = getDeviceContext();

        if (deviceContext == nullptr)
            return;

        const auto fillTransform = transform.isOnlyTranslated
                                 ? SavedState::BrushTransformFlags::applyWorldAndFillTypeTransforms
                                 : SavedState::BrushTransformFlags::applyFillTypeTransform;

        const auto brush = owner.currentState->getBrush (fillTransform);

        if (transform.isOnlyTranslated)
        {
            const auto translated = offsetShape (shape, transform.offset.toFloat());

            if (owner.currentState->doesIntersectClipList (translated))
                primitiveOp (translated, deviceContext, brush);
        }
        else if (owner.currentState->doesIntersectClipList (transform.boundsAfterTransform (shape)))
        {
            ScopedTransform scopedTransform { *this, owner.currentState };
            primitiveOp (shape, deviceContext, brush);
        }
    }

    DirectWriteGlyphRun glyphRun;

private:
    static void resetTransform (ID2D1DeviceContext1* context);
    static void setTransform (ID2D1DeviceContext1* context, AffineTransform newTransform);

    DxgiAdapter::Ptr findAdapter() const;

    void adapterCreated (DxgiAdapter::Ptr newAdapter) override;
    void adapterRemoved (DxgiAdapter::Ptr expiringAdapter) override;

    Direct2DGraphicsContext& owner;
    SharedResourcePointer<DirectX> directX;
    SharedResourcePointer<Direct2DFactories> directWrite;

    std::optional<Direct2DDeviceResources> deviceResources;

    std::vector<std::unique_ptr<Direct2DGraphicsContext::SavedState>> savedClientStates;

   #if JUCE_DIRECT2D_METRICS
    int64 paintStartTicks = 0;
   #endif

    JUCE_DECLARE_WEAK_REFERENCEABLE (Pimpl)
};

} // namespace juce
