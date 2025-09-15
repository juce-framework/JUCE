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

    static ComSmartPtr<ID2D1StrokeStyle1> pathStrokeTypeToStrokeStyle (ID2D1Factory1* factory,
                                                                       const PathStrokeType& strokeType)
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
Direct2DGraphicsContext::Direct2DGraphicsContext() = default;
Direct2DGraphicsContext::~Direct2DGraphicsContext() = default;

bool Direct2DGraphicsContext::startFrame (float dpiScale)
{
    const auto pimpl = getPimpl();
    const auto paintAreas = pimpl->getPaintAreas();
    currentState = pimpl->startFrame();

    if (currentState == nullptr)
        return false;

    if (auto deviceContext = pimpl->getDeviceContext())
    {
        resetPendingClipList();

        clipToRectangleList (paintAreas);

        // Clear the buffer *after* setting the clip region
        clearTargetBuffer();

        // Init font & brush
        setFont (currentState->font);
        currentState->updateCurrentBrush();

        addTransform (AffineTransform::scale (dpiScale));
    }

    return true;
}

void Direct2DGraphicsContext::endFrame()
{
    getPimpl()->finishFrame();

    currentState = nullptr;
    ++frame;
}

void Direct2DGraphicsContext::setOrigin (Point<int> o)
{
    applyPendingClipList();

    currentState->currentTransform.setOrigin (o);

    resetPendingClipList();
}

void Direct2DGraphicsContext::addTransform (const AffineTransform& transform)
{
    // The pending clip list is based on the transform stored in currentState, so apply the pending clip list before adding the transform
    applyPendingClipList();

    currentState->currentTransform.addTransform (transform);

    resetPendingClipList();
}

bool Direct2DGraphicsContext::clipToRectangle (const Rectangle<int>& r)
{
    const auto& transform = currentState->currentTransform;
    auto& deviceSpaceClipList = currentState->deviceSpaceClipList;

    JUCE_TRACE_EVENT_INT_RECT_LIST (etw::clipToRectangle, etw::direct2dKeyword, getFrameId(), r);

    // The renderer needs to keep track of the aggregate clip rectangles in order to correctly report the
    // clip region to the caller. The renderer also needs to push Direct2D clip layers to the device context
    // to perform the actual clipping. The reported clip region will not necessarily match the Direct2D clip region
    // if the clip region is transformed, or the clip region is an image or a path.
    //
    // Pushing Direct2D clip layers is expensive and there's no need to clip until something is actually drawn.
    // So - pendingClipList is a list of the areas that need to actually be clipped. Each fill or
    // draw method then applies any pending clip areas before drawing.
    //
    // Also - calling ID2D1DeviceContext::SetTransform is expensive, so check the current transform to see
    // if the renderer can pre-transform the clip rectangle instead.
    if (transform.isOnlyTranslated)
    {
        // The current transform is only a translation, so save a few cycles by just adding the
        // offset instead of transforming the rectangle; the software renderer does something similar.
        auto translatedR = r.toFloat() + transform.offset.toFloat();
        deviceSpaceClipList.clipTo (translatedR);

        pendingClipList.clipTo (translatedR);
    }
    else if (currentState->isCurrentTransformAxisAligned())
    {
        // The current transform is a simple scale + translation, so pre-transform the rectangle
        auto transformedR = transform.boundsAfterTransform (r.toFloat());
        deviceSpaceClipList.clipTo (transformedR);

        pendingClipList.clipTo (transformedR);
    }
    else
    {
        deviceSpaceClipList = getPimpl()->getFrameSize().toFloat();

        // The current transform is too complex to pre-transform the rectangle, so just add the
        // rectangle to the clip list. The renderer will need to call ID2D1DeviceContext::SetTransform
        // before applying the clip layer.
        pendingClipList.clipTo (r.toFloat());
    }

    return ! isClipEmpty();
}

bool Direct2DGraphicsContext::clipToRectangleList (const RectangleList<int>& newClipList)
{
    JUCE_SCOPED_TRACE_EVENT_FRAME_RECT_I32 (etw::clipToRectangleList, etw::direct2dKeyword, getFrameId(), newClipList)

    const auto& transform = currentState->currentTransform;
    auto& deviceSpaceClipList = currentState->deviceSpaceClipList;

    // This works a lot like clipToRect

    // Just one rectangle?
    if (newClipList.getNumRectangles() == 1)
        return clipToRectangle (newClipList.getRectangle (0));

    if (transform.isIdentity())
    {
        deviceSpaceClipList.clipTo (newClipList);

        pendingClipList.clipTo (newClipList);
    }
    else if (currentState->currentTransform.isOnlyTranslated)
    {
        RectangleList<int> offsetList (newClipList);
        offsetList.offsetAll (transform.offset);
        deviceSpaceClipList.clipTo (offsetList);

        pendingClipList.clipTo (offsetList);
    }
    else if (currentState->isCurrentTransformAxisAligned())
    {
        RectangleList<float> scaledList;

        for (auto& i : newClipList)
            scaledList.add (transform.boundsAfterTransform (i.toFloat()));

        deviceSpaceClipList.clipTo (scaledList);
        pendingClipList.clipTo (scaledList);
    }
    else
    {
        deviceSpaceClipList = getPimpl()->getFrameSize().toFloat();

        pendingClipList.clipTo (newClipList);
    }

    return ! isClipEmpty();
}

void Direct2DGraphicsContext::excludeClipRectangle (const Rectangle<int>& userSpaceExcludedRectangle)
{
    JUCE_SCOPED_TRACE_EVENT_FRAME_RECT_I32 (etw::excludeClipRectangle, etw::direct2dKeyword, getFrameId(), userSpaceExcludedRectangle)

    auto& transform = currentState->currentTransform;
    auto& deviceSpaceClipList = currentState->deviceSpaceClipList;
    const auto frameSize = getPimpl()->getFrameSize().toFloat();

    if (transform.isOnlyTranslated)
    {
        // Just a translation; pre-translate the exclusion area
        auto translatedR = transform.translated (userSpaceExcludedRectangle.toFloat()).getLargestIntegerWithin().toFloat();

        if (! translatedR.contains (frameSize))
        {
            deviceSpaceClipList.subtract (translatedR);
            pendingClipList.subtract (translatedR);
        }
    }
    else if (currentState->isCurrentTransformAxisAligned())
    {
        // Just a scale + translation; pre-transform the exclusion area
        auto transformedR = transform.boundsAfterTransform (userSpaceExcludedRectangle.toFloat()).getLargestIntegerWithin().toFloat();

        if (! transformedR.contains (frameSize))
        {
            deviceSpaceClipList.subtract (transformedR);
            pendingClipList.subtract (transformedR);
        }
    }
    else
    {
        applyPendingClipList();

        deviceSpaceClipList = frameSize;
        pendingClipList.subtract (userSpaceExcludedRectangle.toFloat());
    }
}

void Direct2DGraphicsContext::resetPendingClipList()
{
    auto& transform = currentState->currentTransform;

    const auto frameSize = transform.isOnlyTranslated || currentState->isCurrentTransformAxisAligned()
                         ? getPimpl()->getFrameSize()
                         : getPimpl()->getFrameSize().transformedBy (transform.getTransform().inverted());

    pendingClipList.reset (frameSize.toFloat());
}

void Direct2DGraphicsContext::applyPendingClipList()
{
    if (! pendingClipList.isClipApplied())
        return;

    auto& transform = currentState->currentTransform;
    const auto axisAligned = currentState->isCurrentTransformAxisAligned();
    const auto list = pendingClipList.getList();

    // Clip if the pending clip list is not empty and smaller than the frame size
    if (! list.containsRectangle (getPimpl()->getFrameSize().toFloat()) && ! list.isEmpty())
    {
        if (list.getNumRectangles() == 1 && axisAligned)
        {
            auto r = list.getRectangle (0);
            currentState->pushAliasedAxisAlignedClipLayer (r);
        }
        else
        {
            auto clipTransform = axisAligned ? AffineTransform{} : transform.getTransform();
            if (auto clipGeometry = D2DHelpers::rectListToPathGeometry (getPimpl()->getDirect2DFactory(),
                                                                        list,
                                                                        clipTransform,
                                                                        D2D1_FILL_MODE_WINDING,
                                                                        D2D1_FIGURE_BEGIN_FILLED,
                                                                        metrics.get()))
            {
                currentState->pushGeometryClipLayer (clipGeometry);
            }
        }

        resetPendingClipList();
    }
}

void Direct2DGraphicsContext::clipToPath (const Path& path, const AffineTransform& transform)
{
    JUCE_SCOPED_TRACE_EVENT_FRAME (etw::clipToPath, etw::direct2dKeyword, getFrameId());

    applyPendingClipList();

    // Set the clip list to the full size of the frame to match
    // the software renderer
    auto pathTransform = currentState->currentTransform.getTransformWith (transform);
    auto transformedBounds = path.getBounds().transformedBy (pathTransform);
    currentState->deviceSpaceClipList.clipTo (transformedBounds);

    if (auto deviceContext = getPimpl()->getDeviceContext())
    {
        currentState->pushGeometryClipLayer (D2DHelpers::pathToPathGeometry (getPimpl()->getDirect2DFactory(),
                                                                             path,
                                                                             pathTransform,
                                                                             D2D1_FIGURE_BEGIN_FILLED,
                                                                             metrics.get()));
    }
}

void Direct2DGraphicsContext::clipToImageAlpha (const Image& sourceImage, const AffineTransform& transform)
{
    JUCE_SCOPED_TRACE_EVENT_FRAME (etw::clipToImageAlpha, etw::direct2dKeyword, getFrameId());

    if (sourceImage.isNull())
        return;

    applyPendingClipList();

    // Put a rectangle clip layer under the image clip layer
    // The D2D bitmap brush will extend past the boundaries of sourceImage, so clip
    // to the sourceImage bounds
    auto brushTransform = currentState->currentTransform.getTransformWith (transform);

    if (D2DHelpers::isTransformAxisAligned (brushTransform))
    {
        currentState->pushAliasedAxisAlignedClipLayer (sourceImage.getBounds().toFloat().transformedBy (brushTransform));
    }
    else
    {
        const auto sourceImageRectF = D2DUtilities::toRECT_F (sourceImage.getBounds());
        ComSmartPtr<ID2D1RectangleGeometry> geometry;

        if (const auto hr = getPimpl()->getDirect2DFactory()->CreateRectangleGeometry (sourceImageRectF, geometry.resetAndGetPointerAddress());
            SUCCEEDED (hr) && geometry != nullptr)
        {
            currentState->pushTransformedRectangleGeometryClipLayer (geometry, brushTransform);
        }
    }

    // Set the clip list to the full size of the frame to match
    // the software renderer
    currentState->deviceSpaceClipList = getPimpl()->getFrameSize().toFloat();

    if (auto deviceContext = getPimpl()->getDeviceContext())
    {
        const auto maxDim = (int) deviceContext->GetMaximumBitmapSize();

        if (sourceImage.getWidth() > maxDim || sourceImage.getHeight() > maxDim)
        {
            // The Direct2D renderer doesn't currently support clipping to very large images
            jassertfalse;
            return;
        }

        const auto device = D2DUtilities::getDeviceForContext (deviceContext);
        const auto pagesAndArea = PagesAndArea::make (sourceImage, device);

        if (pagesAndArea.pages.empty())
            return;

        const auto bitmap = pagesAndArea.pages.front().bitmap;

        if (bitmap == nullptr)
            return;

        // Make a transformed bitmap brush using the bitmap
        // As usual, apply the current transform first *then* the transform parameter
        ComSmartPtr<ID2D1BitmapBrush> brush;
        const auto pageTransform = AffineTransform::translation (pagesAndArea.area.getTopLeft()).inverted();
        auto matrix = D2DUtilities::transformToMatrix (pageTransform.followedBy (brushTransform));
        D2D1_BRUSH_PROPERTIES brushProps = { 1.0f, matrix };

        auto bitmapBrushProps = D2D1::BitmapBrushProperties (D2D1_EXTEND_MODE_CLAMP, D2D1_EXTEND_MODE_CLAMP);
        auto hr = deviceContext->CreateBitmapBrush (bitmap, bitmapBrushProps, brushProps, brush.resetAndGetPointerAddress());

        if (FAILED (hr))
            return;

        // Push the clipping layer onto the layer stack
        // Don't set maskTransform in the LayerParameters struct; that only applies to geometry clipping
        // Do set the contentBounds member, transformed appropriately
        auto layerParams = D2D1::LayerParameters1();
        auto transformedBounds = sourceImage.getBounds().toFloat().transformedBy (brushTransform);
        layerParams.contentBounds = D2DUtilities::toRECT_F (transformedBounds);
        layerParams.opacityBrush = brush;

        currentState->pushLayer (layerParams);
    }
}

bool Direct2DGraphicsContext::clipRegionIntersects (const Rectangle<int>& r)
{
    const auto rect = currentState->currentTransform.isOnlyTranslated ? currentState->currentTransform.translated (r.toFloat())
                                                                      : currentState->currentTransform.boundsAfterTransform (r.toFloat());
    return currentState->deviceSpaceClipList.intersectsRectangle (rect);
}

Rectangle<int> Direct2DGraphicsContext::getClipBounds() const
{
    return currentState->currentTransform.deviceSpaceToUserSpace (currentState->deviceSpaceClipList.getBounds()).getSmallestIntegerContainer();
}

bool Direct2DGraphicsContext::isClipEmpty() const
{
    return getClipBounds().isEmpty();
}

void Direct2DGraphicsContext::saveState()
{
    JUCE_SCOPED_TRACE_EVENT_FRAME (etw::saveState, etw::direct2dKeyword, getFrameId());

    applyPendingClipList();

    currentState = getPimpl()->pushSavedState();
}

void Direct2DGraphicsContext::restoreState()
{
    JUCE_SCOPED_TRACE_EVENT_FRAME (etw::restoreState, etw::direct2dKeyword, getFrameId());

    currentState = getPimpl()->popSavedState();

    currentState->updateColourBrush();
    jassert (currentState);

    resetPendingClipList();
}

void Direct2DGraphicsContext::beginTransparencyLayer (float opacity)
{
    JUCE_SCOPED_TRACE_EVENT_FRAME (etw::beginTransparencyLayer, etw::direct2dKeyword, getFrameId());

    applyPendingClipList();

    if (auto deviceContext = getPimpl()->getDeviceContext())
        currentState->pushTransparencyLayer (opacity);
}

void Direct2DGraphicsContext::endTransparencyLayer()
{
    JUCE_SCOPED_TRACE_EVENT_FRAME (etw::endTransparencyLayer, etw::direct2dKeyword, getFrameId());

    if (auto deviceContext = getPimpl()->getDeviceContext())
        currentState->popTopLayer();
}

void Direct2DGraphicsContext::setFill (const FillType& fillType)
{
    JUCE_SCOPED_TRACE_EVENT_FRAME (etw::setFill, etw::direct2dKeyword, getFrameId());

    if (auto deviceContext = getPimpl()->getDeviceContext())
    {
        currentState->fillType = fillType;
        currentState->updateCurrentBrush();
    }
}

void Direct2DGraphicsContext::setOpacity (float newOpacity)
{
    JUCE_SCOPED_TRACE_EVENT_FRAME (etw::setOpacity, etw::direct2dKeyword, getFrameId());

    currentState->setOpacity (newOpacity);

    if (auto deviceContext = getPimpl()->getDeviceContext())
        currentState->updateCurrentBrush();
}

void Direct2DGraphicsContext::setInterpolationQuality (Graphics::ResamplingQuality quality)
{
    switch (quality)
    {
        case Graphics::ResamplingQuality::lowResamplingQuality:
            currentState->interpolationMode = D2D1_INTERPOLATION_MODE_NEAREST_NEIGHBOR;
            break;

        case Graphics::ResamplingQuality::mediumResamplingQuality:
            currentState->interpolationMode = D2D1_INTERPOLATION_MODE_LINEAR;
            break;

        case Graphics::ResamplingQuality::highResamplingQuality:
            currentState->interpolationMode = D2D1_INTERPOLATION_MODE_HIGH_QUALITY_CUBIC;
            break;
    }
}

Line<float> Direct2DGraphicsContext::offsetShape (Line<float> a, Point<float> b)
{
    return { a.getStart() + b, a.getEnd() + b };
}

Rectangle<float> Direct2DGraphicsContext::offsetShape (Rectangle<float> a, Point<float> b)
{
    return a + b;
}

RectangleList<float> Direct2DGraphicsContext::offsetShape (RectangleList<float> a, Point<float> b)
{
    a.offsetAll (b);
    return a;
}

template <typename Shape, typename Fn>
void Direct2DGraphicsContext::paintPrimitive (const Shape& shape, Fn&& primitiveOp)
{
    const auto& transform = currentState->currentTransform;

    applyPendingClipList();

    auto deviceContext = getPimpl()->getDeviceContext();

    if (deviceContext == nullptr)
        return;

    const auto fillTransform = transform.isOnlyTranslated
                             ? SavedState::BrushTransformFlags::applyWorldAndFillTypeTransforms
                             : SavedState::BrushTransformFlags::applyFillTypeTransform;

    const auto brush = currentState->getBrush (fillTransform);

    if (transform.isOnlyTranslated)
    {
        const auto translated = offsetShape (shape, transform.offset.toFloat());

        if (currentState->doesIntersectClipList (translated))
            primitiveOp (translated, deviceContext, brush);
    }
    else if (currentState->doesIntersectClipList (transform.boundsAfterTransform (shape)))
    {
        ScopedTransform scopedTransform { *getPimpl(), currentState };
        primitiveOp (shape, deviceContext, brush);
    }
}

void Direct2DGraphicsContext::fillRect (const Rectangle<int>& r, bool replaceExistingContents)
{
    if (r.isEmpty())
        return;

    if (replaceExistingContents)
    {
        applyPendingClipList();

        const auto asRectF = D2DUtilities::toRECT_F (r.toFloat());
        ComSmartPtr<ID2D1RectangleGeometry> rectGeometry;
        getPimpl()->getDirect2DFactory()->CreateRectangleGeometry (asRectF,
                                                                   rectGeometry.resetAndGetPointerAddress());

        const auto matrix = D2DUtilities::transformToMatrix (currentState->currentTransform.getTransform());
        ComSmartPtr<ID2D1TransformedGeometry> geo;
        getPimpl()->getDirect2DFactory()->CreateTransformedGeometry (rectGeometry,
                                                                     matrix,
                                                                     geo.resetAndGetPointerAddress());

        const auto brush = currentState->fillType.isInvisible() ? currentState->currentBrush : currentState->getBrush();
        currentState->layers.fillGeometryWithNoLayersActive (getPimpl()->getDeviceContext(), geo, brush);
        return;
    }

    const auto fill = [] (Rectangle<float> rect,
                          ComSmartPtr<ID2D1DeviceContext1> deviceContext,
                          ComSmartPtr<ID2D1Brush> brush)
    {
        if (brush != nullptr)
            deviceContext->FillRectangle (D2DUtilities::toRECT_F (rect), brush);
    };

    paintPrimitive (r.toFloat(), fill);
}

void Direct2DGraphicsContext::fillRect (const Rectangle<float>& r)
{
    if (r.isEmpty())
        return;

    auto fill = [] (Rectangle<float> rect, ComSmartPtr<ID2D1DeviceContext1> deviceContext, ComSmartPtr<ID2D1Brush> brush)
    {
        if (brush != nullptr)
            deviceContext->FillRectangle (D2DUtilities::toRECT_F (rect), brush);
    };

    paintPrimitive (r, fill);
}

void Direct2DGraphicsContext::fillRectList (const RectangleList<float>& list)
{
    if (getPimpl()->fillSpriteBatch (list))
        return;

    auto fill = [] (const RectangleList<float>& l, ComSmartPtr<ID2D1DeviceContext1> deviceContext, ComSmartPtr<ID2D1Brush> brush)
    {
        if (brush != nullptr)
            for (const auto& r : l)
                deviceContext->FillRectangle (D2DUtilities::toRECT_F (r), brush);
    };

    paintPrimitive (list, fill);
}

void Direct2DGraphicsContext::drawRect (const Rectangle<float>& r, float lineThickness)
{
    auto draw = [&] (Rectangle<float> rect, ComSmartPtr<ID2D1DeviceContext1> deviceContext, ComSmartPtr<ID2D1Brush> brush)
    {
        // ID2D1DeviceContext::DrawRectangle centers the stroke around the edges of the specified rectangle, but
        // the software renderer contains the stroke within the rectangle
        // To match the software renderer, reduce the rectangle by half the stroke width
        if (brush != nullptr)
            deviceContext->DrawRectangle (D2DUtilities::toRECT_F (rect.reduced (lineThickness * 0.5f)), brush, lineThickness);
    };

    paintPrimitive (r, draw);
}

void Direct2DGraphicsContext::fillPath (const Path& p, const AffineTransform& transform)
{
    JUCE_SCOPED_TRACE_EVENT_FRAME (etw::fillPath, etw::direct2dKeyword, getFrameId());

    if (p.getBounds().withZeroOrigin() == Rectangle<float>{})
        return;

    applyPendingClipList();

    const auto deviceContext = getPimpl()->getDeviceContext();
    const auto brush = currentState->getBrush (SavedState::applyFillTypeTransform);
    const auto factory = getPimpl()->getDirect2DFactory();
    const auto geometry = D2DHelpers::pathToPathGeometry (factory,
                                                          p,
                                                          transform,
                                                          D2D1_FIGURE_BEGIN_FILLED,
                                                          metrics.get());

    if (deviceContext == nullptr || brush == nullptr || geometry == nullptr)
        return;

    JUCE_D2DMETRICS_SCOPED_ELAPSED_TIME (metrics, fillGeometryTime)

    ScopedTransform scopedTransform { *getPimpl(), currentState };
    deviceContext->FillGeometry (geometry, brush);
}

void Direct2DGraphicsContext::strokePath (const Path& p, const PathStrokeType& strokeType, const AffineTransform& transform)
{
    JUCE_SCOPED_TRACE_EVENT_FRAME (etw::drawPath, etw::direct2dKeyword, getFrameId());

    if (p.getBounds().withZeroOrigin() == Rectangle<float>{})
        return;

    applyPendingClipList();

    const auto deviceContext = getPimpl()->getDeviceContext();
    const auto brush = currentState->getBrush (SavedState::applyFillTypeTransform);
    const auto factory = getPimpl()->getDirect2DFactory();
    const auto strokeStyle = D2DHelpers::pathStrokeTypeToStrokeStyle (factory, strokeType);
    const auto geometry = D2DHelpers::pathToPathGeometry (factory,
                                                          p,
                                                          transform,
                                                          D2D1_FIGURE_BEGIN_HOLLOW,
                                                          metrics.get());

    if (deviceContext == nullptr || brush == nullptr || geometry == nullptr || strokeStyle == nullptr)
        return;

    JUCE_D2DMETRICS_SCOPED_ELAPSED_TIME (metrics, drawGeometryTime)

    ScopedTransform scopedTransform { *getPimpl(), currentState };
    deviceContext->DrawGeometry (geometry, brush, strokeType.getStrokeThickness(), strokeStyle);
}

void Direct2DGraphicsContext::drawImage (const Image& imageIn, const AffineTransform& transform)
{
    JUCE_D2DMETRICS_SCOPED_ELAPSED_TIME (metrics, drawImageTime)

    JUCE_SCOPED_TRACE_EVENT_FRAME (etw::drawImage, etw::direct2dKeyword, getFrameId());

    if (imageIn.isNull())
        return;

    applyPendingClipList();

    if (auto deviceContext = getPimpl()->getDeviceContext())
    {
        const auto device = D2DUtilities::getDeviceForContext (deviceContext);
        const auto pagesAndArea = PagesAndArea::make (imageIn, device);

        if (pagesAndArea.pages.empty())
        {
            jassertfalse;
            return;
        }

        const auto imageTransform = currentState->currentTransform.getTransformWith (transform);

        auto drawTiles = [&] (auto&& getRect)
        {
            for (const auto& page : pagesAndArea.pages)
            {
                if (page.bitmap == nullptr)
                    continue;

                const auto pageBounds = page.getBounds();
                const auto intersection = pageBounds.toFloat().getIntersection (pagesAndArea.area.toFloat());

                if (intersection.isEmpty())
                    continue;

                const auto src = intersection - pageBounds.getPosition().toFloat();
                const auto dst = getRect (intersection - pagesAndArea.area.getPosition().toFloat());
                const auto [srcConverted, dstConverted] = std::tuple (D2DUtilities::toRECT_F (src),
                                                                      D2DUtilities::toRECT_F (dst));

                if (page.bitmap->GetPixelFormat().format == DXGI_FORMAT_A8_UNORM)
                {
                    const auto lastColour = currentState->colourBrush->GetColor();
                    const auto lastMode = deviceContext->GetAntialiasMode();

                    currentState->colourBrush->SetColor (D2D1::ColorF (1.0f, 1.0f, 1.0f, currentState->fillType.getOpacity()));
                    deviceContext->SetAntialiasMode (D2D1_ANTIALIAS_MODE_ALIASED);
                    deviceContext->FillOpacityMask (page.bitmap,
                                                    currentState->colourBrush,
                                                    dstConverted,
                                                    srcConverted);

                    deviceContext->SetAntialiasMode (lastMode);
                    currentState->colourBrush->SetColor (lastColour);
                }
                else
                {
                    const auto lastMode = deviceContext->GetAntialiasMode();

                    if (pagesAndArea.pages.size() > 1)
                        deviceContext->SetAntialiasMode (D2D1_ANTIALIAS_MODE_ALIASED);

                    deviceContext->DrawBitmap (page.bitmap,
                                               dstConverted,
                                               currentState->fillType.getOpacity(),
                                               currentState->interpolationMode,
                                               srcConverted,
                                               {});

                    deviceContext->SetAntialiasMode (lastMode);
                }
            }
        };

        if (imageTransform.isOnlyTranslation() || D2DHelpers::isTransformAxisAligned (imageTransform))
        {
            drawTiles ([&] (auto intersection)
            {
                return intersection.transformedBy (imageTransform);
            });

            return;
        }

        ScopedTransform scopedTransform { *getPimpl(), currentState, transform };

        drawTiles ([] (auto intersection)
        {
            return intersection;
        });
    }
}

void Direct2DGraphicsContext::drawLine (const Line<float>& line)
{
    drawLineWithThickness (line, 1.0f);
}

void Direct2DGraphicsContext::drawLineWithThickness (const Line<float>& line, float lineThickness)
{
    auto draw = [&] (Line<float> l, ComSmartPtr<ID2D1DeviceContext1> deviceContext, ComSmartPtr<ID2D1Brush> brush)
    {
        if (brush == nullptr)
            return;

        const auto makePoint = [] (const auto& x) { return D2D1::Point2F (x.getX(), x.getY()); };
        deviceContext->DrawLine (makePoint (l.getStart()),
                                 makePoint (l.getEnd()),
                                 brush,
                                 lineThickness);
    };

    paintPrimitive (line, draw);
}

void Direct2DGraphicsContext::setFont (const Font& newFont)
{
    JUCE_SCOPED_TRACE_EVENT_FRAME (etw::setFont, etw::direct2dKeyword, getFrameId());

    currentState->setFont (newFont);
}

const Font& Direct2DGraphicsContext::getFont()
{
    return currentState->font;
}

float Direct2DGraphicsContext::getPhysicalPixelScaleFactor() const
{
    if (currentState != nullptr)
        return currentState->currentTransform.getPhysicalPixelScaleFactor();

    // If this is hit, there's no frame in progress, so the scale factor isn't meaningful
    jassertfalse;
    return 1.0f;
}

void Direct2DGraphicsContext::drawRoundedRectangle (const Rectangle<float>& area, float cornerSize, float lineThickness)
{
    auto draw = [&] (Rectangle<float> rect, ComSmartPtr<ID2D1DeviceContext1> deviceContext, ComSmartPtr<ID2D1Brush> brush)
    {
        if (brush == nullptr)
            return;

        D2D1_ROUNDED_RECT roundedRect { D2DUtilities::toRECT_F (rect), cornerSize, cornerSize };
        deviceContext->DrawRoundedRectangle (roundedRect, brush, lineThickness);
    };

    paintPrimitive (area, draw);
}

void Direct2DGraphicsContext::fillRoundedRectangle (const Rectangle<float>& area, float cornerSize)
{
    auto fill = [&] (Rectangle<float> rect, ComSmartPtr<ID2D1DeviceContext1> deviceContext, ComSmartPtr<ID2D1Brush> brush)
    {
        if (brush == nullptr)
            return;

        D2D1_ROUNDED_RECT roundedRect { D2DUtilities::toRECT_F (rect), cornerSize, cornerSize };
        deviceContext->FillRoundedRectangle (roundedRect, brush);
    };

    paintPrimitive (area, fill);
}

void Direct2DGraphicsContext::drawEllipse (const Rectangle<float>& area, float lineThickness)
{
    auto draw = [&] (Rectangle<float> rect, ComSmartPtr<ID2D1DeviceContext1> deviceContext, ComSmartPtr<ID2D1Brush> brush)
    {
        if (brush == nullptr)
            return;

        auto centre = rect.getCentre();
        D2D1_ELLIPSE ellipse { { centre.x, centre.y }, rect.proportionOfWidth (0.5f), rect.proportionOfHeight (0.5f) };
        deviceContext->DrawEllipse (ellipse, brush, lineThickness);
    };

    paintPrimitive (area, draw);
}

void Direct2DGraphicsContext::fillEllipse (const Rectangle<float>& area)
{
    auto fill = [&] (Rectangle<float> rect, ComSmartPtr<ID2D1DeviceContext1> deviceContext, ComSmartPtr<ID2D1Brush> brush)
    {
        if (brush == nullptr)
            return;

        auto centre = rect.getCentre();
        D2D1_ELLIPSE ellipse { { centre.x, centre.y }, rect.proportionOfWidth (0.5f), rect.proportionOfHeight (0.5f) };
        deviceContext->FillEllipse (ellipse, brush);
    };

    paintPrimitive (area, fill);
}

void Direct2DGraphicsContext::drawGlyphs (Span<const uint16_t> glyphNumbers,
                                          Span<const Point<float>> positions,
                                          const AffineTransform& transform)
{
    jassert (glyphNumbers.size() == positions.size());

    JUCE_D2DMETRICS_SCOPED_ELAPSED_TIME (metrics, drawGlyphRunTime)

    JUCE_SCOPED_TRACE_EVENT_FRAME (etw::drawGlyphRun, etw::direct2dKeyword, getFrameId());

    if (currentState->fillType.isInvisible() || glyphNumbers.empty() || positions.empty())
        return;

    const auto& font = currentState->font;

    const auto deviceContext = getPimpl()->getDeviceContext();

    if (! deviceContext)
        return;

    const auto typeface = font.getTypefacePtr();
    const auto fontFace = [&]() -> ComSmartPtr<IDWriteFontFace>
    {
        if (auto* x = dynamic_cast<WindowsDirectWriteTypeface*> (typeface.get()))
            return x->getIDWriteFontFace();

        return {};
    }();

    if (fontFace == nullptr)
        return;

    const auto fontScale = font.getHorizontalScale();
    const auto textTransform = AffineTransform::scale (fontScale, 1.0f).followedBy (transform);
    const auto worldTransform = currentState->currentTransform.getTransform();
    const auto textAndWorldTransform = textTransform.followedBy (worldTransform);
    const auto onlyTranslated = textAndWorldTransform.isOnlyTranslation();

    const auto fillTransform = onlyTranslated
                             ? SavedState::BrushTransformFlags::applyWorldAndFillTypeTransforms
                             : SavedState::BrushTransformFlags::applyFillTypeTransform;

    auto brush = currentState->getBrush (fillTransform);

    if (brush == nullptr)
        return;

    const auto getBrushTransform = [] (auto brushIn) -> AffineTransform
    {
        D2D1::Matrix3x2F matrix{};
        brushIn->GetTransform (&matrix);
        return D2DUtilities::matrixToTransform (matrix);
    };

    applyPendingClipList();

    D2D1_POINT_2F baselineOrigin { 0.0f, 0.0f };

    if (onlyTranslated)
    {
        baselineOrigin = { textAndWorldTransform.getTranslationX(), textAndWorldTransform.getTranslationY() };
    }
    else
    {
        if (brush != currentState->colourBrush)
        {
            const auto brushTransform = getBrushTransform (brush);
            brush->SetTransform (D2DUtilities::transformToMatrix (brushTransform.followedBy (textTransform.inverted())));
        }

        getPimpl()->setDeviceContextTransform (textAndWorldTransform);
    }

    // There's no need to transform a plain colour brush
    jassert (brush != currentState->colourBrush || getBrushTransform (brush).isIdentity());

    auto& run = getPimpl()->glyphRun;
    run.replace (positions, fontScale);

    DWRITE_GLYPH_RUN directWriteGlyphRun;
    directWriteGlyphRun.fontFace = fontFace;
    directWriteGlyphRun.fontEmSize = font.getHeightInPoints();
    directWriteGlyphRun.glyphCount = (UINT32) glyphNumbers.size();
    directWriteGlyphRun.glyphIndices = glyphNumbers.data();
    directWriteGlyphRun.glyphAdvances = run.getAdvances();
    directWriteGlyphRun.glyphOffsets = run.getOffsets();
    directWriteGlyphRun.isSideways = FALSE;
    directWriteGlyphRun.bidiLevel = 0;

    const auto tryDrawColourGlyphs = [&]
    {
        // There's a helpful colour glyph rendering sample at
        // https://github.com/microsoft/Windows-universal-samples/blob/main/Samples/DWriteColorGlyph/cpp/CustomTextRenderer.cpp
        const auto factory = getPimpl()->getDirectWriteFactory4();

        if (factory == nullptr)
            return false;

        const auto ctx = deviceContext.getInterface<ID2D1DeviceContext4>();

        if (ctx == nullptr)
            return false;

        ComSmartPtr<IDWriteColorGlyphRunEnumerator1> enumerator;

        constexpr auto formats = DWRITE_GLYPH_IMAGE_FORMATS_TRUETYPE
                               | DWRITE_GLYPH_IMAGE_FORMATS_CFF
                               | DWRITE_GLYPH_IMAGE_FORMATS_COLR
                               | DWRITE_GLYPH_IMAGE_FORMATS_PNG
                               | DWRITE_GLYPH_IMAGE_FORMATS_JPEG
                               | DWRITE_GLYPH_IMAGE_FORMATS_TIFF
                               | DWRITE_GLYPH_IMAGE_FORMATS_PREMULTIPLIED_B8G8R8A8;

        if (const auto hr = factory->TranslateColorGlyphRun (baselineOrigin,
                                                             &directWriteGlyphRun,
                                                             nullptr,
                                                             formats,
                                                             DWRITE_MEASURING_MODE_NATURAL,
                                                             nullptr,
                                                             0,
                                                             enumerator.resetAndGetPointerAddress());
                FAILED (hr) || enumerator == nullptr)
        {
            // NOCOLOR is expected if the font has no colour glyphs. Other errors are not expected.
            jassert (hr == DWRITE_E_NOCOLOR && enumerator == nullptr);
            return false;
        }

        for (BOOL gotRun = false; SUCCEEDED (enumerator->MoveNext (&gotRun)) && gotRun;)
        {
            const DWRITE_COLOR_GLYPH_RUN1* colourRun = nullptr;

            if (FAILED (enumerator->GetCurrentRun (&colourRun)) || colourRun == nullptr)
                break;

            JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wswitch-enum")
            switch (colourRun->glyphImageFormat)
            {
                case DWRITE_GLYPH_IMAGE_FORMATS_PNG:
                case DWRITE_GLYPH_IMAGE_FORMATS_JPEG:
                case DWRITE_GLYPH_IMAGE_FORMATS_TIFF:
                case DWRITE_GLYPH_IMAGE_FORMATS_PREMULTIPLIED_B8G8R8A8:
                    ctx->DrawColorBitmapGlyphRun (colourRun->glyphImageFormat,
                                                  { colourRun->baselineOriginX, colourRun->baselineOriginY },
                                                  &colourRun->glyphRun,
                                                  colourRun->measuringMode);
                    break;

                case DWRITE_GLYPH_IMAGE_FORMATS_TRUETYPE:
                case DWRITE_GLYPH_IMAGE_FORMATS_CFF:
                case DWRITE_GLYPH_IMAGE_FORMATS_COLR:
                default:
                {
                    const auto useForeground = colourRun->paletteIndex == 0xffff;
                    const auto lastColour = currentState->colourBrush->GetColor();
                    const auto colourBrush = currentState->colourBrush;

                    if (! useForeground)
                        colourBrush->SetColor (colourRun->runColor);

                    const auto brushToUse = useForeground ? ComSmartPtr<ID2D1Brush> (brush)
                                                          : ComSmartPtr<ID2D1Brush> (colourBrush);

                    ctx->DrawGlyphRun ({ colourRun->baselineOriginX, colourRun->baselineOriginY },
                                       &colourRun->glyphRun,
                                       colourRun->glyphRunDescription,
                                       brushToUse,
                                       colourRun->measuringMode);

                    if (! useForeground)
                        colourBrush->SetColor (lastColour);

                    break;
                }
            }
            JUCE_END_IGNORE_WARNINGS_GCC_LIKE
        }

        return true;
    };

    if (! tryDrawColourGlyphs())
        deviceContext->DrawGlyphRun (baselineOrigin, &directWriteGlyphRun, brush);

    if (! onlyTranslated)
        getPimpl()->resetDeviceContextTransform();
}

Direct2DGraphicsContext::ScopedTransform::ScopedTransform (Pimpl& pimplIn, SavedState* stateIn)
    : pimpl (pimplIn), state (stateIn)
{
    pimpl.setDeviceContextTransform (stateIn->currentTransform.getTransform());
}

Direct2DGraphicsContext::ScopedTransform::ScopedTransform (Pimpl& pimplIn, SavedState* stateIn, const AffineTransform& transform)
    : pimpl (pimplIn), state (stateIn)
{
    pimpl.setDeviceContextTransform (stateIn->currentTransform.getTransformWith (transform));
}

Direct2DGraphicsContext::ScopedTransform::~ScopedTransform()
{
    pimpl.resetDeviceContextTransform();
}

//==============================================================================
//==============================================================================

#if JUCE_UNIT_TESTS

class Direct2DGraphicsContextTests : public UnitTest
{
public:
    Direct2DGraphicsContextTests() : UnitTest ("Direct2D Graphics Context", UnitTestCategories::graphics) {}

    void runTest() override
    {
        const auto imageWidth = 1 << 15;
        const auto imageHeight = 128;
        Image largeImageSoftware { Image::RGB, imageWidth, imageHeight, false, SoftwareImageType{} };

        {
            Graphics g { largeImageSoftware };
            g.setGradientFill ({ Colours::red, 0, 0, Colours::cyan, (float) largeImageSoftware.getWidth(), 0, false });
            g.fillAll();
        }

        constexpr auto targetDim = 512;

        const auto largeImageNative = NativeImageType{}.convert (largeImageSoftware);
        const auto subsection = largeImageNative.getClippedImage (largeImageNative.getBounds().withSizeKeepingCentre (1 << 14, 64));

        beginTest ("Render large images");
        {
            for (const auto& imageToDraw : { largeImageNative, subsection })
            {
                const AffineTransform transformsToTest[]
                {
                    {},
                    AffineTransform::translation ((float) targetDim - (float) imageToDraw.getWidth(), 0),
                    AffineTransform::translation (0, (float) targetDim - (float) imageToDraw.getHeight()),
                    AffineTransform::scale ((float) targetDim / imageWidth),
                    AffineTransform::scale ((float) targetDim / imageWidth)
                            .followedBy (AffineTransform::translation (32, 64)),
                    AffineTransform::scale (1.1f),
                    AffineTransform::scale ((float) targetDim / imageWidth,
                                            (float) targetDim / imageHeight),
                    AffineTransform::rotation (MathConstants<float>::pi * 0.25f),
                    AffineTransform::rotation (MathConstants<float>::pi * 0.25f, imageWidth * 0.5f, 0)
                            .followedBy (AffineTransform::translation (-imageWidth * 0.5f, 0)),
                };

                for (const auto& transform : transformsToTest)
                {
                    Image targetNative { Image::RGB, targetDim, targetDim, true, NativeImageType{} };
                    Image targetSoftware { Image::RGB, targetDim, targetDim, true, SoftwareImageType{} };

                    for (auto& image : { &targetNative, &targetSoftware })
                    {
                        Graphics g { *image };
                        g.drawImageTransformed (imageToDraw, transform);
                    }

                    auto pixelsToIgnore = createEdgeMask (imageToDraw.getWidth(),
                                                          imageToDraw.getHeight(),
                                                          targetNative.getWidth(),
                                                          targetNative.getHeight(),
                                                          transform);

                    compareImages (targetNative, targetSoftware, 16, pixelsToIgnore);
                }
            }
        }

        beginTest ("Check that there is no seam between D2D image tiles");
        {
            const auto width = 229;
            const auto height = 80 * width;

            Image filmStripSoftware { Image::RGB, width, height, true, SoftwareImageType{} };

            {
                Graphics g { filmStripSoftware };
                g.setGradientFill ({ Colours::red, 0, 0, Colours::cyan, (float) filmStripSoftware.getWidth(), 0, false });
                g.fillAll();
            }

            const auto filmStrip = NativeImageType{}.convert (filmStripSoftware);
            Image targetNative { Image::RGB, targetDim, targetDim, true, NativeImageType{} };
            Image targetSoftware { Image::RGB, targetDim, targetDim, true, SoftwareImageType{} };
            const auto transform = AffineTransform::scale (1.1f);

            for (auto* target : { &targetNative, &targetSoftware })
            {
                Graphics g { *target };
                g.setColour (Colours::orange);
                g.fillAll();
                g.addTransform (transform);
                g.drawImage (filmStrip, 0, 0, width, width, 0, (16384 / width) * width, width, width);
            }

            auto pixelsToIgnore = createEdgeMask (width,
                                                  width,
                                                  targetNative.getWidth(),
                                                  targetNative.getHeight(),
                                                  transform);

            compareImages (targetNative, targetSoftware, 1, pixelsToIgnore);
        }

        beginTest ("Gradient fill transform should compose with world transform correctly");
        {
            testGradientFillTransform (1.0f);
            testGradientFillTransform (1.5f);
        }

        beginTest ("Text gradient fill transform should compose with world transform correctly");
        {
            testTextGradientFillTransform (2.0f);
            testTextGradientFillTransform (1.5f);
            testTextGradientFillTransform (1.0f);
        }
    }

    static Image createEdgeMask (int sourceWidth,
                                 int sourceHeight,
                                 int maskWidth,
                                 int maskHeight,
                                 const AffineTransform& transform)
    {
        Image mask { Image::SingleChannel, maskWidth, maskHeight, true, SoftwareImageType{} };
        Graphics g { mask };
        g.addTransform (transform);
        g.setColour (Colours::white);
        g.drawRect (Rectangle<int> { 0, 0, sourceWidth + 1, sourceHeight + 1 }.toFloat(), 2.0f);

        return mask;
    }

    void compareImages (const Image& a, const Image& b, int stride, const Image& ignoreMask)
    {
        expect (a.getBounds() == b.getBounds());

        const Image::BitmapData bitmapA { a, Image::BitmapData::readOnly };
        const Image::BitmapData bitmapB { b, Image::BitmapData::readOnly };

        int64_t maxAbsError{};
        int64_t accumulatedError{};
        int64_t numSamples{};

        for (auto y = 0; y < a.getHeight(); y += stride)
        {
            for (auto x = 0; x < a.getWidth(); x += stride)
            {
                if (ignoreMask.getPixelAt (x, y) != Colour{})
                    continue;

                const auto expected = bitmapA.getPixelColour (x, y);
                const auto actual   = bitmapB.getPixelColour (x, y);

                for (auto& fn : { &Colour::getRed, &Colour::getGreen, &Colour::getBlue, &Colour::getAlpha })
                {
                    const auto signedError = ((int64_t) (actual.*fn)() - (int64_t) (expected.*fn)());
                    const auto absError = std::abs (signedError);
                    maxAbsError = std::max (maxAbsError, absError);

                    accumulatedError += absError;
                    ++numSamples;
                }
            }
        }

        const auto averageError = (double) accumulatedError / (double) numSamples;
        expect (std::abs (averageError) < 1.0 && maxAbsError < 10);
    }

    void testGradientFillTransform (float scale)
    {
        constexpr int size = 500;
        constexpr int circleSize = 100;
        constexpr int brushTranslation = 20;

        Image image { Image::RGB,
                      roundToInt (size * scale),
                      roundToInt (size * scale),
                      true };

        {
            for (int i = 0; i < size / circleSize; ++i)
            {
                Graphics g { image };

                g.addTransform (AffineTransform::scale (scale));
                g.addTransform (AffineTransform::translation ((float) i * circleSize, (float) i * circleSize));

                const auto fillCol1 { Colours::red };
                const auto fillCol2 { Colours::green };
                const auto centreLoc = circleSize / 2.0f;

                FillType innerGlowGrad = ColourGradient { fillCol1,
                                                          { centreLoc, centreLoc },
                                                          fillCol2,
                                                          { centreLoc, 0.0f },
                                                          true };

                innerGlowGrad.gradient->addColour (0.19, fillCol1);

                innerGlowGrad.transform = AffineTransform::scale (1.1f, 0.9f, centreLoc, centreLoc)
                                              .followedBy (AffineTransform::translation (brushTranslation,
                                                                                         brushTranslation));

                g.setFillType (innerGlowGrad);
                g.fillEllipse (0, 0, (float) circleSize, (float) circleSize);
            }
        }

        for (int i = 0; i < size / circleSize; ++i)
        {
            const auto getScaled = [scale] (Point<int> p)
            {
                return p.toFloat().transformedBy (AffineTransform::scale (scale)).roundToInt();
            };

            const Point<int> centre { circleSize / 2, circleSize / 2 };
            const Point<int> brushOffset { brushTranslation, brushTranslation };

            const auto redPosition = getScaled (centre + brushOffset);
            expect (image.getPixelAt (redPosition.getX(), redPosition.getY()) == Colours::red);

            const auto mostlyRedPosition = getScaled (centre);
            expect (approximatelyEqual (image.getPixelAt (mostlyRedPosition.getX(), mostlyRedPosition.getY()),
                                        Colour { 138, 59, 0 }));

            const auto greenPosition = getScaled (centre.withY (2));
            expect (image.getPixelAt (greenPosition.getX(), greenPosition.getY()) == Colours::green);

            const auto blackPosition = getScaled ({ circleSize - 2, 2 });
            expect (image.getPixelAt (blackPosition.getX(), blackPosition.getY()) == Colours::black);
        }
    }

    void testTextGradientFillTransform (float scale)
    {
        const auto typeface = loadTypeface (FontBinaryData::Karla_Regular_Typo_Off_Offsets_Off);

        constexpr int size = 500;

        Image image { Image::RGB,
                      roundToInt (size * scale),
                      roundToInt (size * scale),
                      true };

        const auto fillCol1 = Colours::cyan;
        const auto fillCol2 = Colours::magenta;
        const auto fillColMiddle = fillCol1.interpolatedWith (fillCol2, 0.5f);

        {
            Graphics g { image };
            g.addTransform (AffineTransform::scale (scale));

            g.setFont (FontOptions { typeface }.withPointHeight (50));
            g.setGradientFill ({ fillCol1, { size * 0.5f - 80, 0 }, fillCol2, { size * 0.5f + 80, 0.0f }, false });

            for (auto i = 0; i != 10; ++i)
            {
                g.drawText (String::repeatedString ("-", 100),
                            Rectangle { size * 2, size }.translated (i * 50 - 500, i * 50),
                            Justification::topLeft,
                            false);
            }
        }

        const auto getPixelAtScaled = [&image, scale] (Point<int> p)
        {
            const auto scaled = p.toFloat().transformedBy (AffineTransform::scale (scale)).roundToInt();
            return image.getPixelAt (scaled.x, scaled.y);
        };

        expect (approximatelyEqual (getPixelAtScaled ({ 15, 27 }), fillCol1));
        expect (approximatelyEqual (getPixelAtScaled ({ 485, 27 }), fillCol2));

        expect (approximatelyEqual (getPixelAtScaled ({ 15, 77 }), fillCol1));
        expect (approximatelyEqual (getPixelAtScaled ({ 485, 77 }), fillCol2));
        expect (approximatelyEqual (getPixelAtScaled ({ 250, 77 }), fillColMiddle));

        expect (approximatelyEqual (getPixelAtScaled ({ 15, 477 }), fillCol1));
        expect (approximatelyEqual (getPixelAtScaled ({ 485, 477 }), fillCol2));
        expect (approximatelyEqual (getPixelAtScaled ({ 250, 477 }), fillColMiddle));
    }

    static bool approximatelyEqual (const Colour& a, const Colour& b)
    {
        return    std::abs (a.getRed() - b.getRed()) < 2
               && std::abs (a.getGreen() - b.getGreen()) < 2
               && std::abs (a.getBlue() - b.getBlue()) < 2
               && std::abs (a.getAlpha() - b.getAlpha()) < 2;
    }

    static Typeface::Ptr loadTypeface (Span<const unsigned char> data)
    {
        return Typeface::createSystemTypefaceFor (data.data(), data.size());
    }
};

static Direct2DGraphicsContextTests direct2DGraphicsContextTests;

#endif

} // namespace juce
