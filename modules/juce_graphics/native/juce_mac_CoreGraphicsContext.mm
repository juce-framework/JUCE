/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

//==============================================================================
// This class has been renamed from CoreGraphicsImage to avoid a symbol
// collision in Pro Tools 2019.12 and possibly 2020 depending on the Pro Tools
// release schedule.
class CoreGraphicsPixelData   : public ImagePixelData
{
public:
    CoreGraphicsPixelData (const Image::PixelFormat format, int w, int h, bool clearImage)
        : ImagePixelData (format, w, h)
    {
        pixelStride = format == Image::RGB ? 3 : ((format == Image::ARGB) ? 4 : 1);
        lineStride = (pixelStride * jmax (1, width) + 3) & ~3;

        auto numComponents = (size_t) lineStride * (size_t) jmax (1, height);

        // SDK version 10.14+ intermittently requires a bit of extra space
        // at the end of the image data. This feels like something has gone
        // wrong in Apple's code.
        numComponents += (size_t) lineStride;

        imageData->data.allocate (numComponents, clearImage);

        auto colourSpace = detail::ColorSpacePtr { CGColorSpaceCreateWithName ((format == Image::SingleChannel) ? kCGColorSpaceGenericGrayGamma2_2
                                                                                                                : kCGColorSpaceSRGB) };

        context = detail::ContextPtr { CGBitmapContextCreate (imageData->data, (size_t) width, (size_t) height, 8, (size_t) lineStride,
                                                              colourSpace.get(), getCGImageFlags (format)) };
    }

    ~CoreGraphicsPixelData() override
    {
        freeCachedImageRef();
    }

    std::unique_ptr<LowLevelGraphicsContext> createLowLevelContext() override
    {
        freeCachedImageRef();
        sendDataChangeMessage();
        return std::make_unique<CoreGraphicsContext> (context.get(), height);
    }

    void initialiseBitmapData (Image::BitmapData& bitmap, int x, int y, Image::BitmapData::ReadWriteMode mode) override
    {
        const auto offset = (size_t) (x * pixelStride + y * lineStride);
        bitmap.data = imageData->data + offset;
        bitmap.size = (size_t) (lineStride * height) - offset;
        bitmap.pixelFormat = pixelFormat;
        bitmap.lineStride = lineStride;
        bitmap.pixelStride = pixelStride;

        if (mode != Image::BitmapData::readOnly)
        {
            freeCachedImageRef();
            sendDataChangeMessage();
        }
    }

    ImagePixelData::Ptr clone() override
    {
        auto im = new CoreGraphicsPixelData (pixelFormat, width, height, false);
        memcpy (im->imageData->data, imageData->data, (size_t) (lineStride * height));
        return *im;
    }

    std::unique_ptr<ImageType> createType() const override    { return std::make_unique<NativeImageType>(); }

    //==============================================================================
    static CGImageRef getCachedImageRef (const Image& juceImage, CGColorSpaceRef colourSpace)
    {
        auto cgim = dynamic_cast<CoreGraphicsPixelData*> (juceImage.getPixelData());

        if (cgim != nullptr && cgim->cachedImageRef != nullptr)
            return CGImageRetain (cgim->cachedImageRef.get());

        CGImageRef ref = createImage (juceImage, colourSpace);

        if (cgim != nullptr)
            cgim->cachedImageRef.reset (CGImageRetain (ref));

        return ref;
    }

    static CGImageRef createImage (const Image& juceImage, CGColorSpaceRef colourSpace)
    {
        const Image::BitmapData srcData (juceImage, Image::BitmapData::readOnly);

        const auto provider = [&]
        {
            if (auto* cgim = dynamic_cast<CoreGraphicsPixelData*> (juceImage.getPixelData()))
            {
                return detail::DataProviderPtr { CGDataProviderCreateWithData (new ImageDataContainer::Ptr (cgim->imageData),
                                                                               srcData.data,
                                                                               srcData.size,
                                                                               [] (void * __nullable info, const void*, size_t) { delete (ImageDataContainer::Ptr*) info; }) };
            }

            const auto usableSize = jmin ((size_t) srcData.lineStride * (size_t) srcData.height, srcData.size);
            CFUniquePtr<CFDataRef> data (CFDataCreate (nullptr, (const UInt8*) srcData.data, (CFIndex) usableSize));
            return detail::DataProviderPtr { CGDataProviderCreateWithCFData (data.get()) };
        }();

        return CGImageCreate ((size_t) srcData.width,
                              (size_t) srcData.height,
                              8,
                              (size_t) srcData.pixelStride * 8,
                              (size_t) srcData.lineStride,
                              colourSpace, getCGImageFlags (juceImage.getFormat()), provider.get(),
                              nullptr, true, kCGRenderingIntentDefault);
    }

    //==============================================================================
    detail::ContextPtr context;
    detail::ImagePtr cachedImageRef;

    struct ImageDataContainer   : public ReferenceCountedObject
    {
        ImageDataContainer() = default;

        using Ptr = ReferenceCountedObjectPtr<ImageDataContainer>;
        HeapBlock<uint8> data;
    };

    ImageDataContainer::Ptr imageData = new ImageDataContainer();
    int pixelStride, lineStride;

private:
    void freeCachedImageRef()
    {
        cachedImageRef.reset();
    }

    static CGBitmapInfo getCGImageFlags (const Image::PixelFormat& format)
    {
       #if JUCE_BIG_ENDIAN
        return format == Image::ARGB ? ((uint32_t) kCGImageAlphaPremultipliedFirst | (uint32_t) kCGBitmapByteOrder32Big) : kCGBitmapByteOrderDefault;
       #else
        return format == Image::ARGB ? ((uint32_t) kCGImageAlphaPremultipliedFirst | (uint32_t) kCGBitmapByteOrder32Little) : kCGBitmapByteOrderDefault;
       #endif
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CoreGraphicsPixelData)
};

ImagePixelData::Ptr NativeImageType::create (Image::PixelFormat format, int width, int height, bool clearImage) const
{
    return *new CoreGraphicsPixelData (format == Image::RGB ? Image::ARGB : format, width, height, clearImage);
}

//==============================================================================
struct ScopedCGContextState
{
    explicit ScopedCGContextState (CGContextRef c)  : context (c)  { CGContextSaveGState (context); }
    ~ScopedCGContextState()                                        { CGContextRestoreGState (context); }

    CGContextRef context;
};

//==============================================================================
CoreGraphicsContext::CoreGraphicsContext (CGContextRef c, float h)
    : context (c),
      flipHeight (h),
      state (new SavedState())
{
    CGContextRetain (context.get());
    CGContextSaveGState (context.get());

   #if JUCE_MAC
    bool enableFontSmoothing
                 #if JUCE_DISABLE_COREGRAPHICS_FONT_SMOOTHING
                  = false;
                 #else
                  = true;
                 #endif

    CGContextSetShouldSmoothFonts (context.get(), enableFontSmoothing);
    CGContextSetAllowsFontSmoothing (context.get(), enableFontSmoothing);
   #endif

    CGContextSetShouldAntialias (context.get(), true);
    CGContextSetBlendMode (context.get(), kCGBlendModeNormal);
    rgbColourSpace.reset (CGColorSpaceCreateWithName (kCGColorSpaceSRGB));
    greyColourSpace.reset (CGColorSpaceCreateWithName (kCGColorSpaceGenericGrayGamma2_2));
    setFont (Font());
}

CoreGraphicsContext::~CoreGraphicsContext()
{
    CGContextRestoreGState (context.get());
}

//==============================================================================
void CoreGraphicsContext::setOrigin (Point<int> o)
{
    CGContextTranslateCTM (context.get(), o.x, -o.y);

    if (lastClipRectIsValid)
        lastClipRect.translate (-o.x, -o.y);
}

void CoreGraphicsContext::addTransform (const AffineTransform& transform)
{
    applyTransform (AffineTransform::verticalFlip ((float) flipHeight)
                                    .followedBy (transform)
                                    .translated (0, (float) -flipHeight)
                                    .scaled (1.0f, -1.0f));
    lastClipRectIsValid = false;

    jassert (getPhysicalPixelScaleFactor() > 0.0f);
}

float CoreGraphicsContext::getPhysicalPixelScaleFactor()
{
    auto t = CGContextGetUserSpaceToDeviceSpaceTransform (context.get());
    auto determinant = (t.a * t.d) - (t.c * t.b);

    return (float) std::sqrt (std::abs (determinant));
}

bool CoreGraphicsContext::clipToRectangle (const Rectangle<int>& r)
{
    CGContextClipToRect (context.get(), CGRectMake (r.getX(), flipHeight - r.getBottom(),
                                                    r.getWidth(), r.getHeight()));

    if (lastClipRectIsValid)
    {
        // This is actually incorrect, because the actual clip region may be complex, and
        // clipping its bounds to a rect may not be right... But, removing this shortcut
        // doesn't actually fix anything because CoreGraphics also ignores complex regions
        // when calculating the resultant clip bounds, and makes the same mistake!
        lastClipRect = lastClipRect.getIntersection (r);
        return ! lastClipRect.isEmpty();
    }

    return ! isClipEmpty();
}

bool CoreGraphicsContext::clipToRectangleListWithoutTest (const RectangleList<int>& clipRegion)
{
    if (clipRegion.isEmpty())
    {
        CGContextClipToRect (context.get(), CGRectZero);
        lastClipRectIsValid = true;
        lastClipRect = Rectangle<int>();
        return false;
    }

    auto numRects = (size_t) clipRegion.getNumRectangles();
    HeapBlock<CGRect> rects (numRects);

    int i = 0;
    for (auto& r : clipRegion)
        rects[i++] = CGRectMake (r.getX(), flipHeight - r.getBottom(), r.getWidth(), r.getHeight());

    CGContextClipToRects (context.get(), rects, numRects);
    lastClipRectIsValid = false;
    return true;
}

bool CoreGraphicsContext::clipToRectangleList (const RectangleList<int>& clipRegion)
{
    return clipToRectangleListWithoutTest (clipRegion) && ! isClipEmpty();
}

void CoreGraphicsContext::excludeClipRectangle (const Rectangle<int>& r)
{
    RectangleList<int> remaining (getClipBounds());
    remaining.subtract (r);
    clipToRectangleListWithoutTest (remaining);
}

void CoreGraphicsContext::clipToPath (const Path& path, const AffineTransform& transform)
{
    createPath (path, transform);

    if (path.isUsingNonZeroWinding())
        CGContextClip (context.get());
    else
        CGContextEOClip (context.get());

    lastClipRectIsValid = false;
}

void CoreGraphicsContext::clipToImageAlpha (const Image& sourceImage, const AffineTransform& transform)
{
    if (! transform.isSingularity())
    {
        Image singleChannelImage (sourceImage);

        if (sourceImage.getFormat() != Image::SingleChannel)
            singleChannelImage = sourceImage.convertedToFormat (Image::SingleChannel);

        auto image = detail::ImagePtr { CoreGraphicsPixelData::createImage (singleChannelImage, greyColourSpace.get()) };

        flip();
        auto t = AffineTransform::verticalFlip (sourceImage.getHeight()).followedBy (transform);
        applyTransform (t);

        auto r = convertToCGRect (sourceImage.getBounds());
        CGContextClipToMask (context.get(), r, image.get());

        applyTransform (t.inverted());
        flip();

        lastClipRectIsValid = false;
    }
}

bool CoreGraphicsContext::clipRegionIntersects (const Rectangle<int>& r)
{
    return getClipBounds().intersects (r);
}

Rectangle<int> CoreGraphicsContext::getClipBounds() const
{
    if (! lastClipRectIsValid)
    {
        auto bounds = CGRectIntegral (CGContextGetClipBoundingBox (context.get()));

        lastClipRectIsValid = true;
        lastClipRect.setBounds (roundToInt (bounds.origin.x),
                                roundToInt (flipHeight - (bounds.origin.y + bounds.size.height)),
                                roundToInt (bounds.size.width),
                                roundToInt (bounds.size.height));
    }

    return lastClipRect;
}

bool CoreGraphicsContext::isClipEmpty() const
{
    return getClipBounds().isEmpty();
}

//==============================================================================
void CoreGraphicsContext::saveState()
{
    CGContextSaveGState (context.get());
    stateStack.add (new SavedState (*state));
}

void CoreGraphicsContext::restoreState()
{
    CGContextRestoreGState (context.get());

    if (auto* top = stateStack.getLast())
    {
        state.reset (top);
        CGContextSetTextMatrix (context.get(), state->textMatrix);

        stateStack.removeLast (1, false);
        lastClipRectIsValid = false;
    }
    else
    {
        jassertfalse; // trying to pop with an empty stack!
    }
}

void CoreGraphicsContext::beginTransparencyLayer (float opacity)
{
    saveState();
    CGContextSetAlpha (context.get(), opacity);
    CGContextBeginTransparencyLayer (context.get(), nullptr);
}

void CoreGraphicsContext::endTransparencyLayer()
{
    CGContextEndTransparencyLayer (context.get());
    restoreState();
}

//==============================================================================
void CoreGraphicsContext::setFill (const FillType& fillType)
{
    state->setFill (fillType);

    if (fillType.isColour())
    {
        CGContextSetRGBFillColor (context.get(), fillType.colour.getFloatRed(), fillType.colour.getFloatGreen(),
                                  fillType.colour.getFloatBlue(), fillType.colour.getFloatAlpha());
        CGContextSetAlpha (context.get(), 1.0f);
    }
}

void CoreGraphicsContext::setOpacity (float newOpacity)
{
    state->fillType.setOpacity (newOpacity);
    setFill (state->fillType);
}

void CoreGraphicsContext::setInterpolationQuality (Graphics::ResamplingQuality quality)
{
    switch (quality)
    {
        case Graphics::lowResamplingQuality:    CGContextSetInterpolationQuality (context.get(), kCGInterpolationNone);   return;
        case Graphics::mediumResamplingQuality: CGContextSetInterpolationQuality (context.get(), kCGInterpolationMedium); return;
        case Graphics::highResamplingQuality:   CGContextSetInterpolationQuality (context.get(), kCGInterpolationHigh);   return;
        default: return;
    }
}

//==============================================================================
void CoreGraphicsContext::fillRect (const Rectangle<int>& r, bool replaceExistingContents)
{
    fillCGRect (CGRectMake (r.getX(), flipHeight - r.getBottom(), r.getWidth(), r.getHeight()), replaceExistingContents);
}

void CoreGraphicsContext::fillRect (const Rectangle<float>& r)
{
    fillCGRect (CGRectMake (r.getX(), flipHeight - r.getBottom(), r.getWidth(), r.getHeight()), false);
}

void CoreGraphicsContext::fillCGRect (const CGRect& cgRect, bool replaceExistingContents)
{
    if (replaceExistingContents)
    {
        CGContextSetBlendMode (context.get(), kCGBlendModeCopy);
        fillCGRect (cgRect, false);
        CGContextSetBlendMode (context.get(), kCGBlendModeNormal);
    }
    else
    {
        if (state->fillType.isColour())
        {
            CGContextFillRect (context.get(), cgRect);
        }
        else
        {
            ScopedCGContextState scopedState (context.get());

            CGContextClipToRect (context.get(), cgRect);

            if (state->fillType.isGradient())
                drawGradient();
            else
                drawImage (state->fillType.image, state->fillType.transform, true);
        }
    }
}

void CoreGraphicsContext::fillPath (const Path& path, const AffineTransform& transform)
{
    ScopedCGContextState scopedState (context.get());

    if (state->fillType.isColour())
    {
        flip();
        applyTransform (transform);
        createPath (path);

        if (path.isUsingNonZeroWinding())
            CGContextFillPath (context.get());
        else
            CGContextEOFillPath (context.get());
    }
    else
    {
        createPath (path, transform);

        if (path.isUsingNonZeroWinding())
            CGContextClip (context.get());
        else
            CGContextEOClip (context.get());

        if (state->fillType.isGradient())
            drawGradient();
        else
            drawImage (state->fillType.image, state->fillType.transform, true);
    }
}

void CoreGraphicsContext::drawImage (const Image& sourceImage, const AffineTransform& transform)
{
    drawImage (sourceImage, transform, false);
}

void CoreGraphicsContext::drawImage (const Image& sourceImage, const AffineTransform& transform, bool fillEntireClipAsTiles)
{
    auto iw = sourceImage.getWidth();
    auto ih = sourceImage.getHeight();

    auto colourSpace = sourceImage.getFormat() == Image::PixelFormat::SingleChannel ? greyColourSpace.get()
                                                                                    : rgbColourSpace.get();
    detail::ImagePtr image { CoreGraphicsPixelData::getCachedImageRef (sourceImage, colourSpace) };

    ScopedCGContextState scopedState (context.get());
    CGContextSetAlpha (context.get(), state->fillType.getOpacity());

    flip();
    applyTransform (AffineTransform::verticalFlip (ih).followedBy (transform));
    auto imageRect = CGRectMake (0, 0, iw, ih);

    if (fillEntireClipAsTiles)
    {
      #if JUCE_IOS
        CGContextDrawTiledImage (context.get(), imageRect, image.get());
      #else
        // There's a bug in CGContextDrawTiledImage that makes it incredibly slow
        // if it's doing a transformation - it's quicker to just draw lots of images manually
        if (&CGContextDrawTiledImage != nullptr && transform.isOnlyTranslation())
        {
            CGContextDrawTiledImage (context.get(), imageRect, image.get());
        }
        else
        {
            // Fallback to manually doing a tiled fill
            auto clip = CGRectIntegral (CGContextGetClipBoundingBox (context.get()));

            int x = 0, y = 0;
            while (x > clip.origin.x)   x -= iw;
            while (y > clip.origin.y)   y -= ih;

            auto right  = (int) (clip.origin.x + clip.size.width);
            auto bottom = (int) (clip.origin.y + clip.size.height);

            while (y < bottom)
            {
                for (int x2 = x; x2 < right; x2 += iw)
                    CGContextDrawImage (context.get(), CGRectMake (x2, y, iw, ih), image.get());

                y += ih;
            }
        }
      #endif
    }
    else
    {
        CGContextDrawImage (context.get(), imageRect, image.get());
    }
}

//==============================================================================
void CoreGraphicsContext::drawLine (const Line<float>& line)
{
    Path p;
    p.addLineSegment (line, 1.0f);
    fillPath (p, {});
}

void CoreGraphicsContext::fillRectList (const RectangleList<float>& list)
{
    HeapBlock<CGRect> rects (list.getNumRectangles());

    size_t num = 0;

    for (auto& r : list)
        rects[num++] = CGRectMake (r.getX(), flipHeight - r.getBottom(), r.getWidth(), r.getHeight());

    if (state->fillType.isColour())
    {
        CGContextFillRects (context.get(), rects, num);
    }
    else
    {
        ScopedCGContextState scopedState (context.get());

        CGContextClipToRects (context.get(), rects, num);

        if (state->fillType.isGradient())
            drawGradient();
        else
            drawImage (state->fillType.image, state->fillType.transform, true);
    }
}

void CoreGraphicsContext::setFont (const Font& newFont)
{
    if (state->font != newFont)
    {
        state->fontRef = nullptr;
        state->font = newFont;

        auto typeface = state->font.getTypefacePtr();

        if (auto osxTypeface = dynamic_cast<OSXTypeface*> (typeface.get()))
        {
            state->fontRef = osxTypeface->fontRef;
            CGContextSetFont (context.get(), state->fontRef);
            CGContextSetFontSize (context.get(), state->font.getHeight() * osxTypeface->fontHeightToPointsFactor);

            state->textMatrix = osxTypeface->renderingTransform;
            state->textMatrix.a *= state->font.getHorizontalScale();
            CGContextSetTextMatrix (context.get(), state->textMatrix);
            state->inverseTextMatrix = CGAffineTransformInvert (state->textMatrix);
         }
    }
}

const Font& CoreGraphicsContext::getFont()
{
    return state->font;
}

void CoreGraphicsContext::drawGlyph (int glyphNumber, const AffineTransform& transform)
{
    if (state->fontRef != nullptr && state->fillType.isColour())
    {
        auto cgTransformIsOnlyTranslation = [] (CGAffineTransform t)
        {
            return t.a == 1.0f && t.d == 1.0f && t.b == 0.0f && t.c == 0.0f;
        };

        if (transform.isOnlyTranslation() && cgTransformIsOnlyTranslation (state->inverseTextMatrix))
        {
            auto x = transform.mat02 + state->inverseTextMatrix.tx;
            auto y = transform.mat12 + state->inverseTextMatrix.ty;

            CGGlyph glyphs[1] = { (CGGlyph) glyphNumber };
            CGPoint positions[1] = { { x, flipHeight - roundToInt (y) } };
            CGContextShowGlyphsAtPositions (context.get(), glyphs, positions, 1);
        }
        else
        {
            ScopedCGContextState scopedState (context.get());

            flip();
            applyTransform (transform);
            CGContextConcatCTM (context.get(), state->inverseTextMatrix);
            auto cgTransform = state->textMatrix;
            cgTransform.d = -cgTransform.d;
            CGContextConcatCTM (context.get(), cgTransform);

            CGGlyph glyphs[1] = { (CGGlyph) glyphNumber };
            CGPoint positions[1] = { { 0.0f, 0.0f } };
            CGContextShowGlyphsAtPositions (context.get(), glyphs, positions, 1);
        }
    }
    else
    {
        Path p;
        auto& f = state->font;
        f.getTypefacePtr()->getOutlineForGlyph (glyphNumber, p);

        fillPath (p, AffineTransform::scale (f.getHeight() * f.getHorizontalScale(), f.getHeight())
                                     .followedBy (transform));
    }
}

bool CoreGraphicsContext::drawTextLayout (const AttributedString& text, const Rectangle<float>& area)
{
    return CoreTextTypeLayout::drawToCGContext (text, area, context.get(), (float) flipHeight);
}

CoreGraphicsContext::SavedState::SavedState()
    : font (1.0f)
{
}

CoreGraphicsContext::SavedState::SavedState (const SavedState& other)
    : fillType (other.fillType), font (other.font), fontRef (other.fontRef),
      textMatrix (other.textMatrix), inverseTextMatrix (other.inverseTextMatrix),
      gradient (other.gradient.get())
{
    if (gradient != nullptr)
        CGGradientRetain (gradient.get());
}

CoreGraphicsContext::SavedState::~SavedState() = default;

void CoreGraphicsContext::SavedState::setFill (const FillType& newFill)
{
    fillType = newFill;
    gradient = nullptr;
}

static CGGradientRef createGradient (const ColourGradient& g, CGColorSpaceRef colourSpace)
{
    auto numColours = g.getNumColours();
    auto data = (CGFloat*) alloca ((size_t) numColours * 5 * sizeof (CGFloat));
    auto locations = data;
    auto components = data + numColours;
    auto comps = components;

    for (int i = 0; i < numColours; ++i)
    {
        auto colour = g.getColour (i);
        *comps++ = (CGFloat) colour.getFloatRed();
        *comps++ = (CGFloat) colour.getFloatGreen();
        *comps++ = (CGFloat) colour.getFloatBlue();
        *comps++ = (CGFloat) colour.getFloatAlpha();
        locations[i] = (CGFloat) g.getColourPosition (i);

        // There's a bug (?) in the way the CG renderer works where it seems
        // to go wrong if you have two colour stops both at position 0..
        jassert (i == 0 || locations[i] != 0);
    }

    return CGGradientCreateWithColorComponents (colourSpace, components, locations, (size_t) numColours);
}

void CoreGraphicsContext::drawGradient()
{
    flip();
    applyTransform (state->fillType.transform);
    CGContextSetAlpha (context.get(), state->fillType.getOpacity());

    auto& g = *state->fillType.gradient;

    if (state->gradient == nullptr)
        state->gradient.reset (createGradient (g, rgbColourSpace.get()));

    auto p1 = convertToCGPoint (g.point1);
    auto p2 = convertToCGPoint (g.point2);

    if (g.isRadial)
        CGContextDrawRadialGradient (context.get(), state->gradient.get(), p1, 0, p1, g.point1.getDistanceFrom (g.point2),
                                     kCGGradientDrawsBeforeStartLocation | kCGGradientDrawsAfterEndLocation);
    else
        CGContextDrawLinearGradient (context.get(), state->gradient.get(), p1, p2,
                                     kCGGradientDrawsBeforeStartLocation | kCGGradientDrawsAfterEndLocation);
}

void CoreGraphicsContext::createPath (const Path& path) const
{
    CGContextBeginPath (context.get());

    for (Path::Iterator i (path); i.next();)
    {
        switch (i.elementType)
        {
            case Path::Iterator::startNewSubPath:  CGContextMoveToPoint (context.get(), i.x1, i.y1); break;
            case Path::Iterator::lineTo:           CGContextAddLineToPoint (context.get(), i.x1, i.y1); break;
            case Path::Iterator::quadraticTo:      CGContextAddQuadCurveToPoint (context.get(), i.x1, i.y1, i.x2, i.y2); break;
            case Path::Iterator::cubicTo:          CGContextAddCurveToPoint (context.get(), i.x1, i.y1, i.x2, i.y2, i.x3, i.y3); break;
            case Path::Iterator::closePath:        CGContextClosePath (context.get()); break;
            default:                               jassertfalse; break;
        }
    }
}

void CoreGraphicsContext::createPath (const Path& path, const AffineTransform& transform) const
{
    CGContextBeginPath (context.get());

    for (Path::Iterator i (path); i.next();)
    {
        switch (i.elementType)
        {
        case Path::Iterator::startNewSubPath:
            transform.transformPoint (i.x1, i.y1);
            CGContextMoveToPoint (context.get(), i.x1, flipHeight - i.y1);
            break;
        case Path::Iterator::lineTo:
            transform.transformPoint (i.x1, i.y1);
            CGContextAddLineToPoint (context.get(), i.x1, flipHeight - i.y1);
            break;
        case Path::Iterator::quadraticTo:
            transform.transformPoints (i.x1, i.y1, i.x2, i.y2);
            CGContextAddQuadCurveToPoint (context.get(), i.x1, flipHeight - i.y1, i.x2, flipHeight - i.y2);
            break;
        case Path::Iterator::cubicTo:
            transform.transformPoints (i.x1, i.y1, i.x2, i.y2, i.x3, i.y3);
            CGContextAddCurveToPoint (context.get(), i.x1, flipHeight - i.y1, i.x2, flipHeight - i.y2, i.x3, flipHeight - i.y3);
            break;
        case Path::Iterator::closePath:
            CGContextClosePath (context.get()); break;
        default:
            jassertfalse;
            break;
        }
    }
}

void CoreGraphicsContext::flip() const
{
    CGContextConcatCTM (context.get(), CGAffineTransformMake (1, 0, 0, -1, 0, flipHeight));
}

void CoreGraphicsContext::applyTransform (const AffineTransform& transform) const
{
    CGAffineTransform t;
    t.a  = transform.mat00;
    t.b  = transform.mat10;
    t.c  = transform.mat01;
    t.d  = transform.mat11;
    t.tx = transform.mat02;
    t.ty = transform.mat12;
    CGContextConcatCTM (context.get(), t);
}

//==============================================================================
#if USE_COREGRAPHICS_RENDERING && JUCE_USE_COREIMAGE_LOADER
Image juce_loadWithCoreImage (InputStream& input)
{
    struct MemoryBlockHolder   : public ReferenceCountedObject
    {
        using Ptr = ReferenceCountedObjectPtr<MemoryBlockHolder>;
        MemoryBlock block;
    };

    MemoryBlockHolder::Ptr memBlockHolder = new MemoryBlockHolder();
    input.readIntoMemoryBlock (memBlockHolder->block, -1);

    if (memBlockHolder->block.isEmpty())
        return {};

   #if JUCE_IOS
    JUCE_AUTORELEASEPOOL
   #endif
    {
      #if JUCE_IOS
        if (UIImage* uiImage = [UIImage imageWithData: [NSData dataWithBytesNoCopy: memBlockHolder->block.getData()
                                                                            length: memBlockHolder->block.getSize()
                                                                      freeWhenDone: NO]])
        {
            CGImageRef loadedImage = uiImage.CGImage;

      #else
        auto provider = detail::DataProviderPtr { CGDataProviderCreateWithData (new MemoryBlockHolder::Ptr (memBlockHolder),
                                                                                memBlockHolder->block.getData(),
                                                                                memBlockHolder->block.getSize(),
                                                                                [] (void * __nullable info, const void*, size_t) { delete (MemoryBlockHolder::Ptr*) info; }) };

        if (auto imageSource = CFUniquePtr<CGImageSourceRef> (CGImageSourceCreateWithDataProvider (provider.get(), nullptr)))
        {
            CFUniquePtr<CGImageRef> loadedImagePtr (CGImageSourceCreateImageAtIndex (imageSource.get(), 0, nullptr));
            auto* loadedImage = loadedImagePtr.get();
      #endif

            if (loadedImage != nullptr)
            {
                auto alphaInfo = CGImageGetAlphaInfo (loadedImage);
                const bool hasAlphaChan = (alphaInfo != kCGImageAlphaNone
                                             && alphaInfo != kCGImageAlphaNoneSkipLast
                                             && alphaInfo != kCGImageAlphaNoneSkipFirst);

                Image image (NativeImageType().create (Image::ARGB, // (CoreImage doesn't work with 24-bit images)
                                                       (int) CGImageGetWidth (loadedImage),
                                                       (int) CGImageGetHeight (loadedImage),
                                                       hasAlphaChan));

                auto cgImage = dynamic_cast<CoreGraphicsPixelData*> (image.getPixelData());
                jassert (cgImage != nullptr); // if USE_COREGRAPHICS_RENDERING is set, the CoreGraphicsPixelData class should have been used.

                CGContextDrawImage (cgImage->context.get(), convertToCGRect (image.getBounds()), loadedImage);
                CGContextFlush (cgImage->context.get());

                // Because it's impossible to create a truly 24-bit CG image, this flag allows a user
                // to find out whether the file they just loaded the image from had an alpha channel or not.
                image.getProperties()->set ("originalImageHadAlpha", hasAlphaChan);
                return image;
            }
        }
    }

    return {};
}
#endif

Image juce_createImageFromCIImage (CIImage*, int, int);
Image juce_createImageFromCIImage (CIImage* im, int w, int h)
{
    auto cgImage = new CoreGraphicsPixelData (Image::ARGB, w, h, false);

    CIContext* cic = [CIContext contextWithCGContext: cgImage->context.get() options: nil];
    [cic drawImage: im inRect: CGRectMake (0, 0, w, h) fromRect: CGRectMake (0, 0, w, h)];
    CGContextFlush (cgImage->context.get());

    return Image (*cgImage);
}

CGImageRef juce_createCoreGraphicsImage (const Image& juceImage, CGColorSpaceRef colourSpace)
{
    return CoreGraphicsPixelData::createImage (juceImage, colourSpace);
}

CGContextRef juce_getImageContext (const Image& image)
{
    if (auto cgi = dynamic_cast<CoreGraphicsPixelData*> (image.getPixelData()))
        return cgi->context.get();

    jassertfalse;
    return {};
}

#if JUCE_IOS
 Image juce_createImageFromUIImage (UIImage* img)
 {
     CGImageRef image = [img CGImage];

     Image retval (Image::ARGB, (int) CGImageGetWidth (image), (int) CGImageGetHeight (image), true);
     CGContextRef ctx = juce_getImageContext (retval);

     CGContextDrawImage (ctx, CGRectMake (0.0f, 0.0f, CGImageGetWidth (image), CGImageGetHeight (image)), image);

     return retval;
 }
#endif

#if JUCE_MAC
 NSImage* imageToNSImage (const ScaledImage& scaled)
 {
     const auto image = scaled.getImage();
     const auto scaleFactor = scaled.getScale();

     JUCE_AUTORELEASEPOOL
     {
         NSImage* im = [[NSImage alloc] init];
         auto requiredSize = NSMakeSize (image.getWidth() / scaleFactor, image.getHeight() / scaleFactor);

         [im setSize: requiredSize];
         detail::ColorSpacePtr colourSpace { CGColorSpaceCreateWithName (kCGColorSpaceSRGB) };
         detail::ImagePtr imageRef { juce_createCoreGraphicsImage (image, colourSpace.get()) };

         NSBitmapImageRep* imageRep = [[NSBitmapImageRep alloc] initWithCGImage: imageRef.get()];
         [imageRep setSize: requiredSize];
         [im addRepresentation: imageRep];
         [imageRep release];
         return im;
     }
 }
#endif

}
