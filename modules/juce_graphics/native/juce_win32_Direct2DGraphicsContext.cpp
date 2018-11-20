/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

template <typename Type>
D2D1_RECT_F rectangleToRectF (const Rectangle<Type>& r)
{
    return D2D1::RectF ((float) r.getX(), (float) r.getY(), (float) r.getRight(), (float) r.getBottom());
}

static D2D1_COLOR_F colourToD2D (Colour c)
{
    return D2D1::ColorF::ColorF (c.getFloatRed(), c.getFloatGreen(), c.getFloatBlue(), c.getFloatAlpha());
}

static void pathToGeometrySink (const Path& path, ID2D1GeometrySink* sink, const AffineTransform& transform)
{
    Path::Iterator it (path);

    while (it.next())
    {
        switch (it.elementType)
        {
        case Path::Iterator::cubicTo:
        {
            D2D1_BEZIER_SEGMENT seg;

            transform.transformPoint   (it.x1, it.y1);
            seg.point1 = D2D1::Point2F (it.x1, it.y1);

            transform.transformPoint   (it.x2, it.y2);
            seg.point2 = D2D1::Point2F (it.x2, it.y2);

            transform.transformPoint   (it.x3, it.y3);
            seg.point3 = D2D1::Point2F (it.x3, it.y3);

            sink->AddBezier (seg);
            break;
        }

        case Path::Iterator::lineTo:
        {
            transform.transformPoint     (it.x1, it.y1);
            sink->AddLine (D2D1::Point2F (it.x1, it.y1));
            break;
        }

        case Path::Iterator::quadraticTo:
        {
            D2D1_QUADRATIC_BEZIER_SEGMENT seg;

            transform.transformPoint   (it.x1, it.y1);
            seg.point1 = D2D1::Point2F (it.x1, it.y1);

            transform.transformPoint   (it.x2, it.y2);
            seg.point2 = D2D1::Point2F (it.x2, it.y2);

            sink->AddQuadraticBezier (seg);
            break;
        }

        case Path::Iterator::closePath:
        {
            sink->EndFigure (D2D1_FIGURE_END_CLOSED);
            break;
        }

        case Path::Iterator::startNewSubPath:
        {
            transform.transformPoint         (it.x1, it.y1);
            sink->BeginFigure (D2D1::Point2F (it.x1, it.y1), D2D1_FIGURE_BEGIN_FILLED);
            break;
        }
        }
    }
}

static D2D1::Matrix3x2F transformToMatrix (const AffineTransform& transform)
{
    D2D1::Matrix3x2F matrix;
    matrix._11 = transform.mat00;
    matrix._12 = transform.mat10;
    matrix._21 = transform.mat01;
    matrix._22 = transform.mat11;
    matrix._31 = transform.mat02;
    matrix._32 = transform.mat12;
    return matrix;
}

static D2D1_POINT_2F pointTransformed (int x, int y, const AffineTransform& transform)
{
    transform.transformPoint (x, y);
    return D2D1::Point2F ((FLOAT) x, (FLOAT) y);
}

static void rectToGeometrySink (const Rectangle<int>& rect, ID2D1GeometrySink* sink, const AffineTransform& transform)
{
    sink->BeginFigure (pointTransformed (rect.getX(),     rect.getY(),       transform), D2D1_FIGURE_BEGIN_FILLED);
    sink->AddLine     (pointTransformed (rect.getRight(), rect.getY(),       transform));
    sink->AddLine     (pointTransformed (rect.getRight(), rect.getBottom(),  transform));
    sink->AddLine     (pointTransformed (rect.getX(),     rect.getBottom(),  transform));
    sink->EndFigure (D2D1_FIGURE_END_CLOSED);
}

//==============================================================================
struct Direct2DLowLevelGraphicsContext::Pimpl
{
    ID2D1PathGeometry* rectListToPathGeometry (const RectangleList<int>& clipRegion)
    {
        ID2D1PathGeometry* p = nullptr;
        factories->d2dFactory->CreatePathGeometry (&p);

        ComSmartPtr<ID2D1GeometrySink> sink;
        HRESULT hr = p->Open (sink.resetAndGetPointerAddress()); // xxx handle error
        sink->SetFillMode (D2D1_FILL_MODE_WINDING);

        for (int i = clipRegion.getNumRectangles(); --i >= 0;)
            rectToGeometrySink (clipRegion.getRectangle(i), sink, AffineTransform());

        hr = sink->Close();
        return p;
    }

    ID2D1PathGeometry* pathToPathGeometry (const Path& path, const AffineTransform& transform)
    {
        ID2D1PathGeometry* p = nullptr;
        factories->d2dFactory->CreatePathGeometry (&p);

        ComSmartPtr<ID2D1GeometrySink> sink;
        HRESULT hr = p->Open (sink.resetAndGetPointerAddress());
        sink->SetFillMode (D2D1_FILL_MODE_WINDING); // xxx need to check Path::isUsingNonZeroWinding()

        pathToGeometrySink (path, sink, transform);

        hr = sink->Close();
        return p;
    }

    SharedResourcePointer<Direct2DFactories> factories;

    ComSmartPtr<ID2D1HwndRenderTarget> renderingTarget;
    ComSmartPtr<ID2D1SolidColorBrush> colourBrush;
};

//==============================================================================
struct Direct2DLowLevelGraphicsContext::SavedState
{
public:
    SavedState (Direct2DLowLevelGraphicsContext& owner_)
        : owner (owner_)
    {
        if (owner.currentState != nullptr)
        {
            // xxx seems like a very slow way to create one of these, and this is a performance
            // bottleneck.. Can the same internal objects be shared by multiple state objects, maybe using copy-on-write?
            setFill (owner.currentState->fillType);
            currentBrush = owner.currentState->currentBrush;
            clipRect = owner.currentState->clipRect;
            transform = owner.currentState->transform;

            font = owner.currentState->font;
            currentFontFace = owner.currentState->currentFontFace;
        }
        else
        {
            const D2D1_SIZE_U size (owner.pimpl->renderingTarget->GetPixelSize());
            clipRect.setSize (size.width, size.height);
            setFill (FillType (Colours::black));
        }
    }

    ~SavedState()
    {
        clearClip();
        clearFont();
        clearFill();
        clearPathClip();
        clearImageClip();
        complexClipLayer = nullptr;
        bitmapMaskLayer = nullptr;
    }

    void clearClip()
    {
        popClips();
        shouldClipRect = false;
    }

    void clipToRectangle (const Rectangle<int>& r)
    {
        clearClip();
        clipRect = r.toFloat().transformed (transform).getSmallestIntegerContainer();
        shouldClipRect = true;
        pushClips();
    }

    void clearPathClip()
    {
        popClips();

        if (shouldClipComplex)
        {
            complexClipGeometry = nullptr;
            shouldClipComplex = false;
        }
    }

    void Direct2DLowLevelGraphicsContext::SavedState::clipToPath (ID2D1Geometry* geometry)
    {
        clearPathClip();

        if (complexClipLayer == nullptr)
            owner.pimpl->renderingTarget->CreateLayer (complexClipLayer.resetAndGetPointerAddress());

        complexClipGeometry = geometry;
        shouldClipComplex = true;
        pushClips();
    }

    void clearRectListClip()
    {
        popClips();

        if (shouldClipRectList)
        {
            rectListGeometry = nullptr;
            shouldClipRectList = false;
        }
    }

    void clipToRectList (ID2D1Geometry* geometry)
    {
        clearRectListClip();

        if (rectListLayer == nullptr)
            owner.pimpl->renderingTarget->CreateLayer (rectListLayer.resetAndGetPointerAddress());

        rectListGeometry = geometry;
        shouldClipRectList = true;
        pushClips();
    }

    void clearImageClip()
    {
        popClips();

        if (shouldClipBitmap)
        {
            maskBitmap = nullptr;
            bitmapMaskBrush = nullptr;
            shouldClipBitmap = false;
        }
    }

    void clipToImage (const Image& clipImage, const AffineTransform& clipTransform)
    {
        clearImageClip();

        if (bitmapMaskLayer == nullptr)
            owner.pimpl->renderingTarget->CreateLayer (bitmapMaskLayer.resetAndGetPointerAddress());

        D2D1_BRUSH_PROPERTIES brushProps;
        brushProps.opacity = 1;
        brushProps.transform = transformToMatrix (clipTransform);

        D2D1_BITMAP_BRUSH_PROPERTIES bmProps = D2D1::BitmapBrushProperties (D2D1_EXTEND_MODE_WRAP, D2D1_EXTEND_MODE_WRAP);

        D2D1_SIZE_U size;
        size.width  = clipImage.getWidth();
        size.height = clipImage.getHeight();

        D2D1_BITMAP_PROPERTIES bp = D2D1::BitmapProperties();

        maskImage = clipImage.convertedToFormat (Image::ARGB);
        Image::BitmapData bd (maskImage, Image::BitmapData::readOnly); // xxx should be maskImage?
        bp.pixelFormat = owner.pimpl->renderingTarget->GetPixelFormat();
        bp.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;

        HRESULT hr = owner.pimpl->renderingTarget->CreateBitmap (size, bd.data, bd.lineStride, bp, maskBitmap.resetAndGetPointerAddress());
        hr = owner.pimpl->renderingTarget->CreateBitmapBrush (maskBitmap, bmProps, brushProps, bitmapMaskBrush.resetAndGetPointerAddress());

        imageMaskLayerParams = D2D1::LayerParameters();
        imageMaskLayerParams.opacityBrush = bitmapMaskBrush;

        shouldClipBitmap = true;
        pushClips();
    }

    void popClips()
    {
        if (clipsBitmap)
        {
            owner.pimpl->renderingTarget->PopLayer();
            clipsBitmap = false;
        }

        if (clipsComplex)
        {
            owner.pimpl->renderingTarget->PopLayer();
            clipsComplex = false;
        }

        if (clipsRectList)
        {
            owner.pimpl->renderingTarget->PopLayer();
            clipsRectList = false;
        }

        if (clipsRect)
        {
            owner.pimpl->renderingTarget->PopAxisAlignedClip();
            clipsRect = false;
        }
    }

    void pushClips()
    {
        if (shouldClipRect && !clipsRect)
        {
            owner.pimpl->renderingTarget->PushAxisAlignedClip (rectangleToRectF (clipRect), D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
            clipsRect = true;
        }

        if (shouldClipRectList && !clipsRectList)
        {
            D2D1_LAYER_PARAMETERS layerParams = D2D1::LayerParameters();
            rectListGeometry->GetBounds (D2D1::IdentityMatrix(), &layerParams.contentBounds);
            layerParams.geometricMask = rectListGeometry;
            owner.pimpl->renderingTarget->PushLayer (layerParams, rectListLayer);
            clipsRectList = true;
        }

        if (shouldClipComplex && !clipsComplex)
        {
            D2D1_LAYER_PARAMETERS layerParams = D2D1::LayerParameters();
            complexClipGeometry->GetBounds (D2D1::IdentityMatrix(), &layerParams.contentBounds);
            layerParams.geometricMask = complexClipGeometry;
            owner.pimpl->renderingTarget->PushLayer (layerParams, complexClipLayer);
            clipsComplex = true;
        }

        if (shouldClipBitmap && !clipsBitmap)
        {
            owner.pimpl->renderingTarget->PushLayer (imageMaskLayerParams, bitmapMaskLayer);
            clipsBitmap = true;
        }
    }

    void setFill (const FillType& newFillType)
    {
        if (fillType != newFillType)
        {
            fillType = newFillType;
            clearFill();
        }
    }

    void clearFont()
    {
        currentFontFace = localFontFace = nullptr;
    }

    void setFont (const Font& newFont)
    {
        if (font != newFont)
        {
            font = newFont;
            clearFont();
        }
    }

    void createFont()
    {
        if (currentFontFace == nullptr)
        {
            WindowsDirectWriteTypeface* typeface = dynamic_cast<WindowsDirectWriteTypeface*> (font.getTypeface());
            currentFontFace = typeface->getIDWriteFontFace();
            fontHeightToEmSizeFactor = typeface->getUnitsToHeightScaleFactor();
        }
    }

    void setOpacity (float newOpacity)
    {
        fillType.setOpacity (newOpacity);

        if (currentBrush != nullptr)
            currentBrush->SetOpacity (newOpacity);
    }

    void clearFill()
    {
        gradientStops = nullptr;
        linearGradient = nullptr;
        radialGradient = nullptr;
        bitmap = nullptr;
        bitmapBrush = nullptr;
        currentBrush = nullptr;
    }

    void createBrush()
    {
        if (currentBrush == nullptr)
        {
            if (fillType.isColour())
            {
                D2D1_COLOR_F colour = colourToD2D (fillType.colour);
                owner.pimpl->colourBrush->SetColor (colour);
                currentBrush = owner.pimpl->colourBrush;
            }
            else if (fillType.isTiledImage())
            {
                D2D1_BRUSH_PROPERTIES brushProps;
                brushProps.opacity = fillType.getOpacity();
                brushProps.transform = transformToMatrix (fillType.transform);

                D2D1_BITMAP_BRUSH_PROPERTIES bmProps = D2D1::BitmapBrushProperties (D2D1_EXTEND_MODE_WRAP, D2D1_EXTEND_MODE_WRAP);

                image = fillType.image;

                D2D1_SIZE_U size;
                size.width = image.getWidth();
                size.height = image.getHeight();

                D2D1_BITMAP_PROPERTIES bp = D2D1::BitmapProperties();

                this->image = image.convertedToFormat (Image::ARGB);
                Image::BitmapData bd (this->image, Image::BitmapData::readOnly);
                bp.pixelFormat = owner.pimpl->renderingTarget->GetPixelFormat();
                bp.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;

                HRESULT hr = owner.pimpl->renderingTarget->CreateBitmap (size, bd.data, bd.lineStride, bp, bitmap.resetAndGetPointerAddress());
                hr = owner.pimpl->renderingTarget->CreateBitmapBrush (bitmap, bmProps, brushProps, bitmapBrush.resetAndGetPointerAddress());

                currentBrush = bitmapBrush;
            }
            else if (fillType.isGradient())
            {
                gradientStops = nullptr;

                D2D1_BRUSH_PROPERTIES brushProps;
                brushProps.opacity = fillType.getOpacity();
                brushProps.transform = transformToMatrix (fillType.transform.followedBy (transform));

                const int numColors = fillType.gradient->getNumColours();

                HeapBlock<D2D1_GRADIENT_STOP> stops (numColors);

                for (int i = fillType.gradient->getNumColours(); --i >= 0;)
                {
                    stops[i].color = colourToD2D (fillType.gradient->getColour (i));
                    stops[i].position = (FLOAT) fillType.gradient->getColourPosition (i);
                }

                owner.pimpl->renderingTarget->CreateGradientStopCollection (stops.getData(), numColors, gradientStops.resetAndGetPointerAddress());

                if (fillType.gradient->isRadial)
                {
                    radialGradient = nullptr;

                    const Point<float> p1 = fillType.gradient->point1;
                    const Point<float> p2 = fillType.gradient->point2;
                    float r = p1.getDistanceFrom(p2);

                    D2D1_RADIAL_GRADIENT_BRUSH_PROPERTIES props =
                        D2D1::RadialGradientBrushProperties(D2D1::Point2F(p1.x, p1.y),
                            D2D1::Point2F(0, 0),
                            r, r);

                    owner.pimpl->renderingTarget->CreateRadialGradientBrush(props, brushProps, gradientStops, radialGradient.resetAndGetPointerAddress());
                    currentBrush = radialGradient;
                }
                else
                {
                    linearGradient = 0;

                    const Point<float> p1 = fillType.gradient->point1;
                    const Point<float> p2 = fillType.gradient->point2;

                    D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES props =
                        D2D1::LinearGradientBrushProperties(D2D1::Point2F(p1.x, p1.y),
                            D2D1::Point2F(p2.x, p2.y));

                    owner.pimpl->renderingTarget->CreateLinearGradientBrush (props, brushProps, gradientStops, linearGradient.resetAndGetPointerAddress());

                    currentBrush = linearGradient;
                }
            }
        }
    }

    Direct2DLowLevelGraphicsContext& owner;

    AffineTransform transform;

    Font font;
    float fontHeightToEmSizeFactor = 1.0;

    IDWriteFontFace* currentFontFace = nullptr;
    ComSmartPtr<IDWriteFontFace> localFontFace;

    Rectangle<int> clipRect;
    bool clipsRect = false, shouldClipRect = false;

    Image image;
    ComSmartPtr<ID2D1Bitmap> bitmap; // xxx needs a better name - what is this for??
    bool clipsBitmap = false, shouldClipBitmap = false;

    ComSmartPtr<ID2D1Geometry> complexClipGeometry;
    D2D1_LAYER_PARAMETERS complexClipLayerParams;
    ComSmartPtr<ID2D1Layer> complexClipLayer;
    bool clipsComplex = false, shouldClipComplex = false;

    ComSmartPtr<ID2D1Geometry> rectListGeometry;
    D2D1_LAYER_PARAMETERS rectListLayerParams;
    ComSmartPtr<ID2D1Layer> rectListLayer;
    bool clipsRectList = false, shouldClipRectList = false;

    Image maskImage;
    D2D1_LAYER_PARAMETERS imageMaskLayerParams;
    ComSmartPtr<ID2D1Layer> bitmapMaskLayer;
    ComSmartPtr<ID2D1Bitmap> maskBitmap;
    ComSmartPtr<ID2D1BitmapBrush> bitmapMaskBrush;

    ID2D1Brush* currentBrush = nullptr;
    ComSmartPtr<ID2D1BitmapBrush> bitmapBrush;
    ComSmartPtr<ID2D1LinearGradientBrush> linearGradient;
    ComSmartPtr<ID2D1RadialGradientBrush> radialGradient;
    ComSmartPtr<ID2D1GradientStopCollection> gradientStops;

    FillType fillType;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SavedState)
};

//==============================================================================
Direct2DLowLevelGraphicsContext::Direct2DLowLevelGraphicsContext (HWND hwnd_)
    : hwnd (hwnd_),
      currentState (nullptr),
      pimpl (new Pimpl())
{
    RECT windowRect;
    GetClientRect (hwnd, &windowRect);
    D2D1_SIZE_U size = { (UINT32) (windowRect.right - windowRect.left), (UINT32) (windowRect.bottom - windowRect.top) };
    bounds.setSize (size.width, size.height);

    D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties();
    D2D1_HWND_RENDER_TARGET_PROPERTIES propsHwnd = D2D1::HwndRenderTargetProperties (hwnd, size);

    if (pimpl->factories->d2dFactory != nullptr)
    {
        HRESULT hr = pimpl->factories->d2dFactory->CreateHwndRenderTarget (props, propsHwnd, pimpl->renderingTarget.resetAndGetPointerAddress());
        jassert (SUCCEEDED (hr)); ignoreUnused (hr);
        hr = pimpl->renderingTarget->CreateSolidColorBrush (D2D1::ColorF::ColorF (0.0f, 0.0f, 0.0f, 1.0f), pimpl->colourBrush.resetAndGetPointerAddress());
    }
}

Direct2DLowLevelGraphicsContext::~Direct2DLowLevelGraphicsContext()
{
    states.clear();
}

void Direct2DLowLevelGraphicsContext::resized()
{
    RECT windowRect;
    GetClientRect (hwnd, &windowRect);
    D2D1_SIZE_U size = { (UINT32) (windowRect.right - windowRect.left), (UINT32) (windowRect.bottom - windowRect.top) };

    pimpl->renderingTarget->Resize (size);
    bounds.setSize (size.width, size.height);
}

void Direct2DLowLevelGraphicsContext::clear()
{
    pimpl->renderingTarget->Clear (D2D1::ColorF (D2D1::ColorF::White, 0.0f)); // xxx why white and not black?
}

void Direct2DLowLevelGraphicsContext::start()
{
    pimpl->renderingTarget->BeginDraw();
    saveState();
}

void Direct2DLowLevelGraphicsContext::end()
{
    states.clear();
    currentState = 0;
    pimpl->renderingTarget->EndDraw();
    pimpl->renderingTarget->CheckWindowState();
}

void Direct2DLowLevelGraphicsContext::setOrigin (Point<int> o)
{
    addTransform (AffineTransform::translation ((float) o.x, (float) o.y));
}

void Direct2DLowLevelGraphicsContext::addTransform (const AffineTransform& transform)
{
    currentState->transform = transform.followedBy (currentState->transform);
}

float Direct2DLowLevelGraphicsContext::getPhysicalPixelScaleFactor()
{
    return currentState->transform.getScaleFactor();
}

bool Direct2DLowLevelGraphicsContext::clipToRectangle (const Rectangle<int>& r)
{
    currentState->clipToRectangle (r);
    return ! isClipEmpty();
}

bool Direct2DLowLevelGraphicsContext::clipToRectangleList (const RectangleList<int>& clipRegion)
{
    currentState->clipToRectList (pimpl->rectListToPathGeometry (clipRegion));
    return ! isClipEmpty();
}

void Direct2DLowLevelGraphicsContext::excludeClipRectangle (const Rectangle<int>&)
{
    //xxx
}

void Direct2DLowLevelGraphicsContext::clipToPath (const Path& path, const AffineTransform& transform)
{
    currentState->clipToPath (pimpl->pathToPathGeometry (path, transform));
}

void Direct2DLowLevelGraphicsContext::clipToImageAlpha (const Image& sourceImage, const AffineTransform& transform)
{
    currentState->clipToImage (sourceImage, transform);
}

bool Direct2DLowLevelGraphicsContext::clipRegionIntersects (const Rectangle<int>& r)
{
    return currentState->clipRect.intersects (r.toFloat().transformed (currentState->transform).getSmallestIntegerContainer());
}

Rectangle<int> Direct2DLowLevelGraphicsContext::getClipBounds() const
{
    // xxx could this take into account complex clip regions?
    return currentState->clipRect.toFloat().transformed (currentState->transform.inverted()).getSmallestIntegerContainer();
}

bool Direct2DLowLevelGraphicsContext::isClipEmpty() const
{
    return currentState->clipRect.isEmpty();
}

void Direct2DLowLevelGraphicsContext::saveState()
{
    states.add (new SavedState (*this));
    currentState = states.getLast();
}

void Direct2DLowLevelGraphicsContext::restoreState()
{
        jassert (states.size() > 1); //you should never pop the last state!
    states.removeLast (1);
    currentState = states.getLast();
}

void Direct2DLowLevelGraphicsContext::beginTransparencyLayer (float /*opacity*/)
{
    jassertfalse; //xxx todo
}

void Direct2DLowLevelGraphicsContext::endTransparencyLayer()
{
    jassertfalse; //xxx todo
}

void Direct2DLowLevelGraphicsContext::setFill (const FillType& fillType)
{
    currentState->setFill (fillType);
}

void Direct2DLowLevelGraphicsContext::setOpacity (float newOpacity)
{
    currentState->setOpacity (newOpacity);
}

void Direct2DLowLevelGraphicsContext::setInterpolationQuality (Graphics::ResamplingQuality /*quality*/)
{
}

void Direct2DLowLevelGraphicsContext::fillRect (const Rectangle<int>& r, bool /*replaceExistingContents*/)
{
    fillRect (r.toFloat());
}

void Direct2DLowLevelGraphicsContext::fillRect (const Rectangle<float>& r)
{
    pimpl->renderingTarget->SetTransform (transformToMatrix (currentState->transform));
    currentState->createBrush();
    pimpl->renderingTarget->FillRectangle (rectangleToRectF (r), currentState->currentBrush);
    pimpl->renderingTarget->SetTransform (D2D1::IdentityMatrix());
}

void Direct2DLowLevelGraphicsContext::fillRectList (const RectangleList<float>& list)
{
    for (auto& r : list)
        fillRect (r);
}

void Direct2DLowLevelGraphicsContext::fillPath (const Path& p, const AffineTransform& transform)
{
    currentState->createBrush();
    ComSmartPtr<ID2D1Geometry> geometry (pimpl->pathToPathGeometry (p, transform.followedBy (currentState->transform)));

    if (pimpl->renderingTarget != nullptr)
        pimpl->renderingTarget->FillGeometry (geometry, currentState->currentBrush);
}

void Direct2DLowLevelGraphicsContext::drawImage (const Image& image, const AffineTransform& transform)
{
    pimpl->renderingTarget->SetTransform (transformToMatrix (transform.followedBy (currentState->transform)));

    D2D1_SIZE_U size;
    size.width = image.getWidth();
    size.height = image.getHeight();

    D2D1_BITMAP_PROPERTIES bp = D2D1::BitmapProperties();

    Image img (image.convertedToFormat (Image::ARGB));
    Image::BitmapData bd (img, Image::BitmapData::readOnly);
    bp.pixelFormat = pimpl->renderingTarget->GetPixelFormat();
    bp.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;

    {
        ComSmartPtr<ID2D1Bitmap> tempBitmap;
        pimpl->renderingTarget->CreateBitmap (size, bd.data, bd.lineStride, bp, tempBitmap.resetAndGetPointerAddress());
        if (tempBitmap != nullptr)
            pimpl->renderingTarget->DrawBitmap (tempBitmap);
    }

    pimpl->renderingTarget->SetTransform (D2D1::IdentityMatrix());
}

void Direct2DLowLevelGraphicsContext::drawLine (const Line <float>& line)
{
    // xxx doesn't seem to be correctly aligned, may need nudging by 0.5 to match the software renderer's behaviour
    pimpl->renderingTarget->SetTransform (transformToMatrix (currentState->transform));
    currentState->createBrush();

    pimpl->renderingTarget->DrawLine (D2D1::Point2F (line.getStartX(), line.getStartY()),
                                      D2D1::Point2F (line.getEndX(), line.getEndY()),
                                      currentState->currentBrush);
    pimpl->renderingTarget->SetTransform (D2D1::IdentityMatrix());
}

void Direct2DLowLevelGraphicsContext::setFont (const Font& newFont)
{
    currentState->setFont (newFont);
}

const Font& Direct2DLowLevelGraphicsContext::getFont()
{
    return currentState->font;
}

void Direct2DLowLevelGraphicsContext::drawGlyph (int glyphNumber, const AffineTransform& transform)
{
    currentState->createBrush();
    currentState->createFont();

    float hScale = currentState->font.getHorizontalScale();

    pimpl->renderingTarget->SetTransform (transformToMatrix (AffineTransform::scale (hScale, 1.0f)
                                                                             .followedBy (transform)
                                                                             .followedBy (currentState->transform)));

    const UINT16 glyphIndices = (UINT16) glyphNumber;
    const FLOAT glyphAdvances = 0;
    DWRITE_GLYPH_OFFSET offset;
    offset.advanceOffset = 0;
    offset.ascenderOffset = 0;

    DWRITE_GLYPH_RUN glyphRun;
    glyphRun.fontFace = currentState->currentFontFace;
    glyphRun.fontEmSize = (FLOAT) (currentState->font.getHeight() * currentState->fontHeightToEmSizeFactor);
    glyphRun.glyphCount = 1;
    glyphRun.glyphIndices = &glyphIndices;
    glyphRun.glyphAdvances = &glyphAdvances;
    glyphRun.glyphOffsets = &offset;
    glyphRun.isSideways = FALSE;
    glyphRun.bidiLevel = 0;

    pimpl->renderingTarget->DrawGlyphRun (D2D1::Point2F (0, 0), &glyphRun, currentState->currentBrush);
    pimpl->renderingTarget->SetTransform (D2D1::IdentityMatrix());
}

bool Direct2DLowLevelGraphicsContext::drawTextLayout (const AttributedString& text, const Rectangle<float>& area)
{
    pimpl->renderingTarget->SetTransform (transformToMatrix (currentState->transform));

    DirectWriteTypeLayout::drawToD2DContext (text, area,
                                             *(pimpl->renderingTarget),
                                             *(pimpl->factories->directWriteFactory),
                                             *(pimpl->factories->systemFonts));

    pimpl->renderingTarget->SetTransform (D2D1::IdentityMatrix());
    return true;
}

} // namespace juce
