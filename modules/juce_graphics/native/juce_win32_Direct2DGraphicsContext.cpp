/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

class Direct2DLowLevelGraphicsContext   : public LowLevelGraphicsContext
{
public:
    Direct2DLowLevelGraphicsContext (HWND hwnd_)
        : hwnd (hwnd_),
          currentState (nullptr)
    {
        RECT windowRect;
        GetClientRect (hwnd, &windowRect);
        D2D1_SIZE_U size = { windowRect.right - windowRect.left, windowRect.bottom - windowRect.top };
        bounds.setSize (size.width, size.height);

        D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties();
        D2D1_HWND_RENDER_TARGET_PROPERTIES propsHwnd = D2D1::HwndRenderTargetProperties (hwnd, size);

        if (factories->d2dFactory != nullptr)
        {
            HRESULT hr = factories->d2dFactory->CreateHwndRenderTarget (props, propsHwnd, renderingTarget.resetAndGetPointerAddress());
            jassert (SUCCEEDED (hr)); ignoreUnused (hr);
            hr = renderingTarget->CreateSolidColorBrush (D2D1::ColorF::ColorF (0.0f, 0.0f, 0.0f, 1.0f), colourBrush.resetAndGetPointerAddress());
        }
    }

    ~Direct2DLowLevelGraphicsContext()
    {
        states.clear();
    }

    void resized()
    {
        RECT windowRect;
        GetClientRect (hwnd, &windowRect);
        D2D1_SIZE_U size = { windowRect.right - windowRect.left, windowRect.bottom - windowRect.top };

        renderingTarget->Resize (size);
        bounds.setSize (size.width, size.height);
    }

    void clear()
    {
        renderingTarget->Clear (D2D1::ColorF (D2D1::ColorF::White, 0.0f)); // xxx why white and not black?
    }

    void start()
    {
        renderingTarget->BeginDraw();
        saveState();
    }

    void end()
    {
        states.clear();
        currentState = 0;
        renderingTarget->EndDraw();
        renderingTarget->CheckWindowState();
    }

    bool isVectorDevice() const { return false; }

    void setOrigin (Point<int> o)
    {
        addTransform (AffineTransform::translation ((float) o.x, (float) o.y));
    }

    void addTransform (const AffineTransform& transform)
    {
        currentState->transform = transform.followedBy (currentState->transform);
    }

    float getPhysicalPixelScaleFactor()
    {
        return currentState->transform.getScaleFactor();
    }

    bool clipToRectangle (const Rectangle<int>& r)
    {
        currentState->clipToRectangle (r);
        return ! isClipEmpty();
    }

    bool clipToRectangleList (const RectangleList<int>& clipRegion)
    {
        currentState->clipToRectList (rectListToPathGeometry (clipRegion));
        return ! isClipEmpty();
    }

    void excludeClipRectangle (const Rectangle<int>&)
    {
        //xxx
    }

    void clipToPath (const Path& path, const AffineTransform& transform)
    {
        currentState->clipToPath (pathToPathGeometry (path, transform));
    }

    void clipToImageAlpha (const Image& sourceImage, const AffineTransform& transform)
    {
        currentState->clipToImage (sourceImage, transform);
    }

    bool clipRegionIntersects (const Rectangle<int>& r)
    {
        return currentState->clipRect.intersects (r.toFloat().transformed (currentState->transform).getSmallestIntegerContainer());
    }

    Rectangle<int> getClipBounds() const
    {
        // xxx could this take into account complex clip regions?
        return currentState->clipRect.toFloat().transformed (currentState->transform.inverted()).getSmallestIntegerContainer();
    }

    bool isClipEmpty() const
    {
        return currentState->clipRect.isEmpty();
    }

    void saveState()
    {
        states.add (new SavedState (*this));
        currentState = states.getLast();
    }

    void restoreState()
    {
        jassert (states.size() > 1) //you should never pop the last state!
        states.removeLast (1);
        currentState = states.getLast();
    }

    void beginTransparencyLayer (float /*opacity*/)
    {
        jassertfalse; //xxx todo
    }

    void endTransparencyLayer()
    {
        jassertfalse; //xxx todo
    }

    void setFill (const FillType& fillType)
    {
        currentState->setFill (fillType);
    }

    void setOpacity (float newOpacity)
    {
        currentState->setOpacity (newOpacity);
    }

    void setInterpolationQuality (Graphics::ResamplingQuality /*quality*/)
    {
    }

    void fillRect (const Rectangle<int>& r, bool /*replaceExistingContents*/)
    {
        fillRect (r.toFloat());
    }

    void fillRect (const Rectangle<float>& r)
    {
        renderingTarget->SetTransform (transformToMatrix (currentState->transform));
        currentState->createBrush();
        renderingTarget->FillRectangle (rectangleToRectF (r), currentState->currentBrush);
        renderingTarget->SetTransform (D2D1::IdentityMatrix());
    }

    void fillRectList (const RectangleList<float>& list)
    {
        for (const Rectangle<float>* r = list.begin(), * const e = list.end(); r != e; ++r)
            fillRect (*r);
    }

    void fillPath (const Path& p, const AffineTransform& transform)
    {
        currentState->createBrush();
        ComSmartPtr<ID2D1Geometry> geometry (pathToPathGeometry (p, transform.followedBy (currentState->transform)));

        if (renderingTarget != nullptr)
            renderingTarget->FillGeometry (geometry, currentState->currentBrush);
    }

    void drawImage (const Image& image, const AffineTransform& transform)
    {
        renderingTarget->SetTransform (transformToMatrix (transform.followedBy (currentState->transform)));

        D2D1_SIZE_U size;
        size.width = image.getWidth();
        size.height = image.getHeight();

        D2D1_BITMAP_PROPERTIES bp = D2D1::BitmapProperties();

        Image img (image.convertedToFormat (Image::ARGB));
        Image::BitmapData bd (img, Image::BitmapData::readOnly);
        bp.pixelFormat = renderingTarget->GetPixelFormat();
        bp.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;

        {
            ComSmartPtr<ID2D1Bitmap> tempBitmap;
            renderingTarget->CreateBitmap (size, bd.data, bd.lineStride, bp, tempBitmap.resetAndGetPointerAddress());
            if (tempBitmap != nullptr)
                renderingTarget->DrawBitmap (tempBitmap);
        }

        renderingTarget->SetTransform (D2D1::IdentityMatrix());
    }

    void drawLine (const Line <float>& line)
    {
        // xxx doesn't seem to be correctly aligned, may need nudging by 0.5 to match the software renderer's behaviour
        renderingTarget->SetTransform (transformToMatrix (currentState->transform));
        currentState->createBrush();

        renderingTarget->DrawLine (D2D1::Point2F (line.getStartX(), line.getStartY()),
                                   D2D1::Point2F (line.getEndX(), line.getEndY()),
                                   currentState->currentBrush);
        renderingTarget->SetTransform (D2D1::IdentityMatrix());
    }

    void setFont (const Font& newFont)
    {
        currentState->setFont (newFont);
    }

    const Font& getFont()
    {
        return currentState->font;
    }

    void drawGlyph (int glyphNumber, const AffineTransform& transform)
    {
        currentState->createBrush();
        currentState->createFont();

        float hScale = currentState->font.getHorizontalScale();

        renderingTarget->SetTransform (transformToMatrix (AffineTransform::scale (hScale, 1.0f)
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

        renderingTarget->DrawGlyphRun (D2D1::Point2F (0, 0), &glyphRun, currentState->currentBrush);
        renderingTarget->SetTransform (D2D1::IdentityMatrix());
    }

    bool drawTextLayout (const AttributedString& text, const Rectangle<float>& area)
    {
        renderingTarget->SetTransform (transformToMatrix (currentState->transform));

        DirectWriteTypeLayout::drawToD2DContext (text, area, *renderingTarget, factories->directWriteFactory,
                                                 factories->d2dFactory, factories->systemFonts);

        renderingTarget->SetTransform (D2D1::IdentityMatrix());
        return true;
    }

    //==============================================================================
    class SavedState
    {
    public:
        SavedState (Direct2DLowLevelGraphicsContext& owner_)
          : owner (owner_), currentBrush (0),
            fontHeightToEmSizeFactor (1.0f), currentFontFace (0),
            clipsRect (false), shouldClipRect (false),
            clipsRectList (false), shouldClipRectList (false),
            clipsComplex (false), shouldClipComplex (false),
            clipsBitmap (false), shouldClipBitmap (false)
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
                const D2D1_SIZE_U size (owner.renderingTarget->GetPixelSize());
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
            complexClipLayer = 0;
            bitmapMaskLayer = 0;
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
                complexClipGeometry = 0;
                shouldClipComplex = false;
            }
        }

        void clipToPath (ID2D1Geometry* geometry)
        {
            clearPathClip();

            if (complexClipLayer == 0)
                owner.renderingTarget->CreateLayer (complexClipLayer.resetAndGetPointerAddress());

            complexClipGeometry = geometry;
            shouldClipComplex = true;
            pushClips();
        }

        void clearRectListClip()
        {
            popClips();

            if (shouldClipRectList)
            {
                rectListGeometry = 0;
                shouldClipRectList = false;
            }
        }

        void clipToRectList (ID2D1Geometry* geometry)
        {
            clearRectListClip();

            if (rectListLayer == 0)
                owner.renderingTarget->CreateLayer (rectListLayer.resetAndGetPointerAddress());

            rectListGeometry = geometry;
            shouldClipRectList = true;
            pushClips();
        }

        void clearImageClip()
        {
            popClips();

            if (shouldClipBitmap)
            {
                maskBitmap = 0;
                bitmapMaskBrush = 0;
                shouldClipBitmap = false;
            }
        }

        void clipToImage (const Image& image, const AffineTransform& transform)
        {
            clearImageClip();

            if (bitmapMaskLayer == 0)
                owner.renderingTarget->CreateLayer (bitmapMaskLayer.resetAndGetPointerAddress());

            D2D1_BRUSH_PROPERTIES brushProps;
            brushProps.opacity = 1;
            brushProps.transform = transformToMatrix (transform);

            D2D1_BITMAP_BRUSH_PROPERTIES bmProps = D2D1::BitmapBrushProperties (D2D1_EXTEND_MODE_WRAP, D2D1_EXTEND_MODE_WRAP);

            D2D1_SIZE_U size;
            size.width = image.getWidth();
            size.height = image.getHeight();

            D2D1_BITMAP_PROPERTIES bp = D2D1::BitmapProperties();

            maskImage = image.convertedToFormat (Image::ARGB);
            Image::BitmapData bd (this->image, Image::BitmapData::readOnly); // xxx should be maskImage?
            bp.pixelFormat = owner.renderingTarget->GetPixelFormat();
            bp.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;

            HRESULT hr = owner.renderingTarget->CreateBitmap (size, bd.data, bd.lineStride, bp, maskBitmap.resetAndGetPointerAddress());
            hr = owner.renderingTarget->CreateBitmapBrush (maskBitmap, bmProps, brushProps, bitmapMaskBrush.resetAndGetPointerAddress());

            imageMaskLayerParams = D2D1::LayerParameters();
            imageMaskLayerParams.opacityBrush = bitmapMaskBrush;

            shouldClipBitmap = true;
            pushClips();
        }

        void popClips()
        {
            if (clipsBitmap)
            {
                owner.renderingTarget->PopLayer();
                clipsBitmap = false;
            }

            if (clipsComplex)
            {
                owner.renderingTarget->PopLayer();
                clipsComplex = false;
            }

            if (clipsRectList)
            {
                owner.renderingTarget->PopLayer();
                clipsRectList = false;
            }

            if (clipsRect)
            {
                owner.renderingTarget->PopAxisAlignedClip();
                clipsRect = false;
            }
        }

        void pushClips()
        {
            if (shouldClipRect && ! clipsRect)
            {
                owner.renderingTarget->PushAxisAlignedClip (rectangleToRectF (clipRect), D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
                clipsRect = true;
            }

            if (shouldClipRectList && ! clipsRectList)
            {
                D2D1_LAYER_PARAMETERS layerParams = D2D1::LayerParameters();
                rectListGeometry->GetBounds (D2D1::IdentityMatrix(), &layerParams.contentBounds);
                layerParams.geometricMask = rectListGeometry;
                owner.renderingTarget->PushLayer (layerParams, rectListLayer);
                clipsRectList = true;
            }

            if (shouldClipComplex && ! clipsComplex)
            {
                D2D1_LAYER_PARAMETERS layerParams = D2D1::LayerParameters();
                complexClipGeometry->GetBounds (D2D1::IdentityMatrix(), &layerParams.contentBounds);
                layerParams.geometricMask = complexClipGeometry;
                owner.renderingTarget->PushLayer (layerParams, complexClipLayer);
                clipsComplex = true;
            }

            if (shouldClipBitmap && ! clipsBitmap)
            {
                owner.renderingTarget->PushLayer (imageMaskLayerParams, bitmapMaskLayer);
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
            currentFontFace = localFontFace = 0;
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
                fontHeightToEmSizeFactor = typeface->unitsToHeightScaleFactor();
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
            gradientStops = 0;
            linearGradient = 0;
            radialGradient = 0;
            bitmap = 0;
            bitmapBrush = 0;
            currentBrush = 0;
        }

        void createBrush()
        {
            if (currentBrush == 0)
            {
                if (fillType.isColour())
                {
                    D2D1_COLOR_F colour = colourToD2D (fillType.colour);
                    owner.colourBrush->SetColor (colour);
                    currentBrush = owner.colourBrush;
                }
                else if (fillType.isTiledImage())
                {
                    D2D1_BRUSH_PROPERTIES brushProps;
                    brushProps.opacity = fillType.getOpacity();
                    brushProps.transform = transformToMatrix (fillType.transform);

                    D2D1_BITMAP_BRUSH_PROPERTIES bmProps = D2D1::BitmapBrushProperties (D2D1_EXTEND_MODE_WRAP,D2D1_EXTEND_MODE_WRAP);

                    image = fillType.image;

                    D2D1_SIZE_U size;
                    size.width = image.getWidth();
                    size.height = image.getHeight();

                    D2D1_BITMAP_PROPERTIES bp = D2D1::BitmapProperties();

                    this->image = image.convertedToFormat (Image::ARGB);
                    Image::BitmapData bd (this->image, Image::BitmapData::readOnly);
                    bp.pixelFormat = owner.renderingTarget->GetPixelFormat();
                    bp.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;

                    HRESULT hr = owner.renderingTarget->CreateBitmap (size, bd.data, bd.lineStride, bp, bitmap.resetAndGetPointerAddress());
                    hr = owner.renderingTarget->CreateBitmapBrush (bitmap, bmProps, brushProps, bitmapBrush.resetAndGetPointerAddress());

                    currentBrush = bitmapBrush;
                }
                else if (fillType.isGradient())
                {
                    gradientStops = 0;

                    D2D1_BRUSH_PROPERTIES brushProps;
                    brushProps.opacity = fillType.getOpacity();
                    brushProps.transform = transformToMatrix (fillType.transform.followedBy (transform));

                    const int numColors = fillType.gradient->getNumColours();

                    HeapBlock<D2D1_GRADIENT_STOP> stops (numColors);

                    for (int i = fillType.gradient->getNumColours(); --i >= 0;)
                    {
                        stops[i].color = colourToD2D (fillType.gradient->getColour(i));
                        stops[i].position = (FLOAT) fillType.gradient->getColourPosition(i);
                    }

                    owner.renderingTarget->CreateGradientStopCollection (stops.getData(), numColors, gradientStops.resetAndGetPointerAddress());

                    if (fillType.gradient->isRadial)
                    {
                        radialGradient = 0;

                        const Point<float> p1 = fillType.gradient->point1;
                        const Point<float> p2 = fillType.gradient->point2;
                        float r = p1.getDistanceFrom (p2);

                        D2D1_RADIAL_GRADIENT_BRUSH_PROPERTIES props =
                            D2D1::RadialGradientBrushProperties (D2D1::Point2F (p1.x, p1.y),
                                                                 D2D1::Point2F (0, 0),
                                                                 r, r);

                        owner.renderingTarget->CreateRadialGradientBrush (props, brushProps, gradientStops, radialGradient.resetAndGetPointerAddress());
                        currentBrush = radialGradient;
                    }
                    else
                    {
                        linearGradient = 0;

                        const Point<float> p1 = fillType.gradient->point1;
                        const Point<float> p2 = fillType.gradient->point2;

                        D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES props =
                            D2D1::LinearGradientBrushProperties (D2D1::Point2F (p1.x, p1.y),
                                                                 D2D1::Point2F (p2.x, p2.y));

                        owner.renderingTarget->CreateLinearGradientBrush (props, brushProps, gradientStops, linearGradient.resetAndGetPointerAddress());

                        currentBrush = linearGradient;
                    }
                }
            }
        }

        //==============================================================================
        //xxx most of these members should probably be private...

        Direct2DLowLevelGraphicsContext& owner;

        AffineTransform transform;

        Font font;
        float fontHeightToEmSizeFactor;
        IDWriteFontFace* currentFontFace;
        ComSmartPtr<IDWriteFontFace> localFontFace;

        FillType fillType;

        Image image;
        ComSmartPtr<ID2D1Bitmap> bitmap; // xxx needs a better name - what is this for??

        Rectangle<int> clipRect;
        bool clipsRect, shouldClipRect;

        ComSmartPtr<ID2D1Geometry> complexClipGeometry;
        D2D1_LAYER_PARAMETERS complexClipLayerParams;
        ComSmartPtr<ID2D1Layer> complexClipLayer;
        bool clipsComplex, shouldClipComplex;

        ComSmartPtr<ID2D1Geometry> rectListGeometry;
        D2D1_LAYER_PARAMETERS rectListLayerParams;
        ComSmartPtr<ID2D1Layer> rectListLayer;
        bool clipsRectList, shouldClipRectList;

        Image maskImage;
        D2D1_LAYER_PARAMETERS imageMaskLayerParams;
        ComSmartPtr<ID2D1Layer> bitmapMaskLayer;
        ComSmartPtr<ID2D1Bitmap> maskBitmap;
        ComSmartPtr<ID2D1BitmapBrush> bitmapMaskBrush;
        bool clipsBitmap, shouldClipBitmap;

        ID2D1Brush* currentBrush;
        ComSmartPtr<ID2D1BitmapBrush> bitmapBrush;
        ComSmartPtr<ID2D1LinearGradientBrush> linearGradient;
        ComSmartPtr<ID2D1RadialGradientBrush> radialGradient;
        ComSmartPtr<ID2D1GradientStopCollection> gradientStops;

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SavedState)
    };

    //==============================================================================
private:
    SharedResourcePointer<Direct2DFactories> factories;
    HWND hwnd;
    ComSmartPtr<ID2D1HwndRenderTarget> renderingTarget;
    ComSmartPtr<ID2D1SolidColorBrush> colourBrush;
    Rectangle<int> bounds;

    SavedState* currentState;
    OwnedArray<SavedState> states;

    //==============================================================================
    template <typename Type>
    static D2D1_RECT_F rectangleToRectF (const Rectangle<Type>& r)
    {
        return D2D1::RectF ((float) r.getX(), (float) r.getY(), (float) r.getRight(), (float) r.getBottom());
    }

    static D2D1_COLOR_F colourToD2D (Colour c)
    {
        return D2D1::ColorF::ColorF (c.getFloatRed(), c.getFloatGreen(), c.getFloatBlue(), c.getFloatAlpha());
    }

    static D2D1_POINT_2F pointTransformed (int x, int y, const AffineTransform& transform)
    {
        transform.transformPoint (x, y);
        return D2D1::Point2F ((FLOAT) x, (FLOAT) y);
    }

    static void rectToGeometrySink (const Rectangle<int>& rect, ID2D1GeometrySink* sink)
    {
        sink->BeginFigure (pointTransformed (rect.getX(), rect.getY()), D2D1_FIGURE_BEGIN_FILLED);
        sink->AddLine (pointTransformed (rect.getRight(), rect.getY()));
        sink->AddLine (pointTransformed (rect.getRight(), rect.getBottom()));
        sink->AddLine (pointTransformed (rect.getX(), rect.getBottom()));
        sink->EndFigure (D2D1_FIGURE_END_CLOSED);
    }

    static ID2D1PathGeometry* rectListToPathGeometry (const RectangleList<int>& clipRegion)
    {
        ID2D1PathGeometry* p = nullptr;
        factories->d2dFactory->CreatePathGeometry (&p);

        ComSmartPtr<ID2D1GeometrySink> sink;
        HRESULT hr = p->Open (sink.resetAndGetPointerAddress()); // xxx handle error
        sink->SetFillMode (D2D1_FILL_MODE_WINDING);

        for (int i = clipRegion.getNumRectangles(); --i >= 0;)
            rectToGeometrySink (clipRegion.getRectangle(i), sink);

        hr = sink->Close();
        return p;
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

                    transform.transformPoint (it.x1, it.y1);
                    seg.point1 = D2D1::Point2F (it.x1, it.y1);

                    transform.transformPoint (it.x2, it.y2);
                    seg.point2 = D2D1::Point2F (it.x2, it.y2);

                    transform.transformPoint(it.x3, it.y3);
                    seg.point3 = D2D1::Point2F (it.x3, it.y3);

                    sink->AddBezier (seg);
                    break;
                }

                case Path::Iterator::lineTo:
                {
                    transform.transformPoint (it.x1, it.y1);
                    sink->AddLine (D2D1::Point2F (it.x1, it.y1));
                    break;
                }

                case Path::Iterator::quadraticTo:
                {
                    D2D1_QUADRATIC_BEZIER_SEGMENT seg;

                    transform.transformPoint (it.x1, it.y1);
                    seg.point1 = D2D1::Point2F (it.x1, it.y1);

                    transform.transformPoint (it.x2, it.y2);
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
                    transform.transformPoint (it.x1, it.y1);
                    sink->BeginFigure (D2D1::Point2F (it.x1, it.y1), D2D1_FIGURE_BEGIN_FILLED);
                    break;
                }
            }
        }
    }

    static ID2D1PathGeometry* pathToPathGeometry (const Path& path, const AffineTransform& transform)
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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Direct2DLowLevelGraphicsContext)
};
