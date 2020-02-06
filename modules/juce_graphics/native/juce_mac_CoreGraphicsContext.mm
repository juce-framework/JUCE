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

       # if JUCE_MAC && defined (MAC_OS_X_VERSION_10_14) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_14
        // This version of the SDK intermittently requires a bit of extra space
        // at the end of the image data. This feels like something has gone
        // wrong in Apple's code.
        numComponents += (size_t) lineStride;
       #endif

        imageDataHolder->data.allocate (numComponents, clearImage);

        CGColorSpaceRef colourSpace = (format == Image::SingleChannel) ? CGColorSpaceCreateDeviceGray()
                                                                       : CGColorSpaceCreateDeviceRGB();

        context = CGBitmapContextCreate (imageDataHolder->data, (size_t) width, (size_t) height, 8, (size_t) lineStride,
                                         colourSpace, getCGImageFlags (format));

        CGColorSpaceRelease (colourSpace);
    }

    ~CoreGraphicsPixelData() override
    {
        freeCachedImageRef();
        CGContextRelease (context);
    }

    std::unique_ptr<LowLevelGraphicsContext> createLowLevelContext() override
    {
        freeCachedImageRef();
        sendDataChangeMessage();
        return std::make_unique<CoreGraphicsContext> (context, height, 1.0f);
    }

    void initialiseBitmapData (Image::BitmapData& bitmap, int x, int y, Image::BitmapData::ReadWriteMode mode) override
    {
        bitmap.data = imageDataHolder->data + x * pixelStride + y * lineStride;
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
        memcpy (im->imageDataHolder->data, imageDataHolder->data, (size_t) (lineStride * height));
        return *im;
    }

    std::unique_ptr<ImageType> createType() const override    { return std::make_unique<NativeImageType>(); }

    //==============================================================================
    static CGImageRef getCachedImageRef (const Image& juceImage, CGColorSpaceRef colourSpace)
    {
        auto cgim = dynamic_cast<CoreGraphicsPixelData*> (juceImage.getPixelData());

        if (cgim != nullptr && cgim->cachedImageRef != nullptr)
        {
            CGImageRetain (cgim->cachedImageRef);
            return cgim->cachedImageRef;
        }

        CGImageRef ref = createImage (juceImage, colourSpace, false);

        if (cgim != nullptr)
            cgim->cachedImageRef = CGImageRetain (ref);

        return ref;
    }

    static CGImageRef createImage (const Image& juceImage, CGColorSpaceRef colourSpace, bool mustOutliveSource)
    {
        const Image::BitmapData srcData (juceImage, Image::BitmapData::readOnly);
        CGDataProviderRef provider;

        if (mustOutliveSource)
        {
            CFDataRef data = CFDataCreate (nullptr, (const UInt8*) srcData.data, (CFIndex) ((size_t) srcData.lineStride * (size_t) srcData.height));
            provider = CGDataProviderCreateWithCFData (data);
            CFRelease (data);
        }
        else
        {
            auto* imageDataContainer = [](const Image& img) -> HeapBlockContainer::Ptr*
            {
                if (auto* cgim = dynamic_cast<CoreGraphicsPixelData*> (img.getPixelData()))
                    return new HeapBlockContainer::Ptr (cgim->imageDataHolder);

                return nullptr;
            } (juceImage);

            provider = CGDataProviderCreateWithData (imageDataContainer,
                                                     srcData.data,
                                                     (size_t) srcData.lineStride * (size_t) srcData.height,
                                                     [] (void * __nullable info, const void*, size_t) { delete (HeapBlockContainer::Ptr*) info; });
        }

        CGImageRef imageRef = CGImageCreate ((size_t) srcData.width,
                                             (size_t) srcData.height,
                                             8,
                                             (size_t) srcData.pixelStride * 8,
                                             (size_t) srcData.lineStride,
                                             colourSpace, getCGImageFlags (juceImage.getFormat()), provider,
                                             nullptr, true, kCGRenderingIntentDefault);

        CGDataProviderRelease (provider);
        return imageRef;
    }

    //==============================================================================
    CGContextRef context;
    CGImageRef cachedImageRef = {};

    struct HeapBlockContainer   : public ReferenceCountedObject
    {
        using Ptr = ReferenceCountedObjectPtr<HeapBlockContainer>;
        HeapBlock<uint8> data;
    };

    HeapBlockContainer::Ptr imageDataHolder = new HeapBlockContainer();
    int pixelStride, lineStride;

private:
    void freeCachedImageRef()
    {
        if (cachedImageRef != CGImageRef())
        {
            CGImageRelease (cachedImageRef);
            cachedImageRef = {};
        }
    }

    static CGBitmapInfo getCGImageFlags (const Image::PixelFormat& format)
    {
       #if JUCE_BIG_ENDIAN
        return format == Image::ARGB ? (kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Big) : kCGBitmapByteOrderDefault;
       #else
        return format == Image::ARGB ? (kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Little) : kCGBitmapByteOrderDefault;
       #endif
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CoreGraphicsPixelData)
};

ImagePixelData::Ptr NativeImageType::create (Image::PixelFormat format, int width, int height, bool clearImage) const
{
    return *new CoreGraphicsPixelData (format == Image::RGB ? Image::ARGB : format, width, height, clearImage);
}

//==============================================================================
CoreGraphicsContext::CoreGraphicsContext (CGContextRef c, float h, float scale)
    : context (c),
      flipHeight (h),
      targetScale (scale),
      state (new SavedState())
{
    CGContextRetain (context);
    CGContextSaveGState (context);

    bool enableFontSmoothing
            #if JUCE_DISABLE_COREGRAPHICS_FONT_SMOOTHING
             = false;
            #else
             = true;
            #endif

    CGContextSetShouldSmoothFonts (context, enableFontSmoothing);
    CGContextSetAllowsFontSmoothing (context, enableFontSmoothing);
    CGContextSetShouldAntialias (context, true);
    CGContextSetBlendMode (context, kCGBlendModeNormal);
    rgbColourSpace = CGColorSpaceCreateDeviceRGB();
    greyColourSpace = CGColorSpaceCreateDeviceGray();
    setFont (Font());
}

CoreGraphicsContext::~CoreGraphicsContext()
{
    CGContextRestoreGState (context);
    CGContextRelease (context);
    CGColorSpaceRelease (rgbColourSpace);
    CGColorSpaceRelease (greyColourSpace);
}

//==============================================================================
void CoreGraphicsContext::setOrigin (Point<int> o)
{
    CGContextTranslateCTM (context, o.x, -o.y);

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
    jassert (getPhysicalPixelScaleFactor() > 0.0f);
}

float CoreGraphicsContext::getPhysicalPixelScaleFactor()
{
    auto t = CGContextGetCTM (context);

    return targetScale * (float) (juce_hypot (t.a, t.c) + juce_hypot (t.b, t.d)) / 2.0f;
}

bool CoreGraphicsContext::clipToRectangle (const Rectangle<int>& r)
{
    CGContextClipToRect (context, CGRectMake (r.getX(), flipHeight - r.getBottom(),
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
        CGContextClipToRect (context, CGRectZero);
        lastClipRectIsValid = true;
        lastClipRect = Rectangle<int>();
        return false;
    }

    auto numRects = (size_t) clipRegion.getNumRectangles();
    HeapBlock<CGRect> rects (numRects);

    int i = 0;
    for (auto& r : clipRegion)
        rects[i++] = CGRectMake (r.getX(), flipHeight - r.getBottom(), r.getWidth(), r.getHeight());

    CGContextClipToRects (context, rects, numRects);
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
        CGContextClip (context);
    else
        CGContextEOClip (context);

    lastClipRectIsValid = false;
}

void CoreGraphicsContext::clipToImageAlpha (const Image& sourceImage, const AffineTransform& transform)
{
    if (! transform.isSingularity())
    {
        Image singleChannelImage (sourceImage);

        if (sourceImage.getFormat() != Image::SingleChannel)
            singleChannelImage = sourceImage.convertedToFormat (Image::SingleChannel);

        CGImageRef image = CoreGraphicsPixelData::createImage (singleChannelImage, greyColourSpace, true);

        flip();
        auto t = AffineTransform::verticalFlip (sourceImage.getHeight()).followedBy (transform);
        applyTransform (t);

        auto r = convertToCGRect (sourceImage.getBounds());
        CGContextClipToMask (context, r, image);

        applyTransform (t.inverted());
        flip();

        CGImageRelease (image);
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
        auto bounds = CGRectIntegral (CGContextGetClipBoundingBox (context));

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
    CGContextSaveGState (context);
    stateStack.add (new SavedState (*state));
}

void CoreGraphicsContext::restoreState()
{
    CGContextRestoreGState (context);

    if (auto* top = stateStack.getLast())
    {
        state.reset (top);
        CGContextSetTextMatrix (context, state->textMatrix);

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
    CGContextSetAlpha (context, opacity);
    CGContextBeginTransparencyLayer (context, nullptr);
}

void CoreGraphicsContext::endTransparencyLayer()
{
    CGContextEndTransparencyLayer (context);
    restoreState();
}

//==============================================================================
void CoreGraphicsContext::setFill (const FillType& fillType)
{
    state->setFill (fillType);

    if (fillType.isColour())
    {
        CGContextSetRGBFillColor (context, fillType.colour.getFloatRed(), fillType.colour.getFloatGreen(),
                                  fillType.colour.getFloatBlue(), fillType.colour.getFloatAlpha());
        CGContextSetAlpha (context, 1.0f);
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
        case Graphics::lowResamplingQuality:    CGContextSetInterpolationQuality (context, kCGInterpolationNone);   return;
        case Graphics::mediumResamplingQuality: CGContextSetInterpolationQuality (context, kCGInterpolationMedium); return;
        case Graphics::highResamplingQuality:   CGContextSetInterpolationQuality (context, kCGInterpolationHigh);   return;
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
        CGContextSetBlendMode (context, kCGBlendModeCopy);
        fillCGRect (cgRect, false);
        CGContextSetBlendMode (context, kCGBlendModeNormal);
    }
    else
    {
        if (state->fillType.isColour())
        {
            CGContextFillRect (context, cgRect);
        }
        else if (state->fillType.isGradient())
        {
            CGContextSaveGState (context);
            CGContextClipToRect (context, cgRect);
            drawGradient();
            CGContextRestoreGState (context);
        }
        else
        {
            CGContextSaveGState (context);
            CGContextClipToRect (context, cgRect);
            drawImage (state->fillType.image, state->fillType.transform, true);
            CGContextRestoreGState (context);
        }
    }
}

void CoreGraphicsContext::fillPath (const Path& path, const AffineTransform& transform)
{
    CGContextSaveGState (context);

    if (state->fillType.isColour())
    {
        flip();
        applyTransform (transform);
        createPath (path);

        if (path.isUsingNonZeroWinding())
            CGContextFillPath (context);
        else
            CGContextEOFillPath (context);
    }
    else
    {
        createPath (path, transform);

        if (path.isUsingNonZeroWinding())
            CGContextClip (context);
        else
            CGContextEOClip (context);

        if (state->fillType.isGradient())
            drawGradient();
        else
            drawImage (state->fillType.image, state->fillType.transform, true);
    }

    CGContextRestoreGState (context);
}

void CoreGraphicsContext::drawImage (const Image& sourceImage, const AffineTransform& transform)
{
    drawImage (sourceImage, transform, false);
}

void CoreGraphicsContext::drawImage (const Image& sourceImage, const AffineTransform& transform, bool fillEntireClipAsTiles)
{
    auto iw = sourceImage.getWidth();
    auto ih = sourceImage.getHeight();

    auto colourSpace = sourceImage.getFormat() == Image::PixelFormat::SingleChannel ? greyColourSpace
                                                                                    : rgbColourSpace;
    CGImageRef image = CoreGraphicsPixelData::getCachedImageRef (sourceImage, colourSpace);

    CGContextSaveGState (context);
    CGContextSetAlpha (context, state->fillType.getOpacity());

    flip();
    applyTransform (AffineTransform::verticalFlip (ih).followedBy (transform));
    auto imageRect = CGRectMake (0, 0, iw, ih);

    if (fillEntireClipAsTiles)
    {
      #if JUCE_IOS
        CGContextDrawTiledImage (context, imageRect, image);
      #else
        // There's a bug in CGContextDrawTiledImage that makes it incredibly slow
        // if it's doing a transformation - it's quicker to just draw lots of images manually
        if (&CGContextDrawTiledImage != nullptr && transform.isOnlyTranslation())
        {
            CGContextDrawTiledImage (context, imageRect, image);
        }
        else
        {
            // Fallback to manually doing a tiled fill
            auto clip = CGRectIntegral (CGContextGetClipBoundingBox (context));

            int x = 0, y = 0;
            while (x > clip.origin.x)   x -= iw;
            while (y > clip.origin.y)   y -= ih;

            auto right  = (int) (clip.origin.x + clip.size.width);
            auto bottom = (int) (clip.origin.y + clip.size.height);

            while (y < bottom)
            {
                for (int x2 = x; x2 < right; x2 += iw)
                    CGContextDrawImage (context, CGRectMake (x2, y, iw, ih), image);

                y += ih;
            }
        }
      #endif
    }
    else
    {
        CGContextDrawImage (context, imageRect, image);
    }

    CGImageRelease (image); // (This causes a memory bug in iOS sim 3.0 - try upgrading to a later version if you hit this)
    CGContextRestoreGState (context);
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
        CGContextFillRects (context, rects, num);
    }
    else if (state->fillType.isGradient())
    {
        CGContextSaveGState (context);
        CGContextClipToRects (context, rects, num);
        drawGradient();
        CGContextRestoreGState (context);
    }
    else
    {
        CGContextSaveGState (context);
        CGContextClipToRects (context, rects, num);
        drawImage (state->fillType.image, state->fillType.transform, true);
        CGContextRestoreGState (context);
    }
}

void CoreGraphicsContext::setFont (const Font& newFont)
{
    if (state->font != newFont)
    {
        state->fontRef = nullptr;
        state->font = newFont;

        if (auto osxTypeface = dynamic_cast<OSXTypeface*> (state->font.getTypeface()))
        {
            state->fontRef = osxTypeface->fontRef;
            CGContextSetFont (context, state->fontRef);
            CGContextSetFontSize (context, state->font.getHeight() * osxTypeface->fontHeightToPointsFactor);

            state->textMatrix = osxTypeface->renderingTransform;
            state->textMatrix.a *= state->font.getHorizontalScale();
            CGContextSetTextMatrix (context, state->textMatrix);
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
        auto cgTransformIsOnlyTranslation = [](CGAffineTransform t)
        {
            return t.a == 1.0f && t.d == 1.0f && t.b == 0.0f && t.c == 0.0f;
        };

        if (transform.isOnlyTranslation() && cgTransformIsOnlyTranslation (state->inverseTextMatrix))
        {
            auto x = transform.mat02 + state->inverseTextMatrix.tx;
            auto y = transform.mat12 + state->inverseTextMatrix.ty;

            CGGlyph glyphs[1] = { (CGGlyph) glyphNumber };
            CGPoint positions[1] = { { x, flipHeight - roundToInt (y) } };
            CGContextShowGlyphsAtPositions (context, glyphs, positions, 1);
        }
        else
        {
            CGContextSaveGState (context);

            flip();
            applyTransform (transform);
            CGContextConcatCTM (context, state->inverseTextMatrix);
            auto cgTransform = state->textMatrix;
            cgTransform.d = -cgTransform.d;
            CGContextConcatCTM (context, cgTransform);

            CGGlyph glyphs[1] = { (CGGlyph) glyphNumber };
            CGPoint positions[1] = { { 0.0f, 0.0f } };
            CGContextShowGlyphsAtPositions (context, glyphs, positions, 1);

            CGContextRestoreGState (context);
        }
    }
    else
    {
        Path p;
        auto& f = state->font;
        f.getTypeface()->getOutlineForGlyph (glyphNumber, p);

        fillPath (p, AffineTransform::scale (f.getHeight() * f.getHorizontalScale(), f.getHeight())
                                     .followedBy (transform));
    }
}

bool CoreGraphicsContext::drawTextLayout (const AttributedString& text, const Rectangle<float>& area)
{
    CoreTextTypeLayout::drawToCGContext (text, area, context, (float) flipHeight);
    return true;
}

CoreGraphicsContext::SavedState::SavedState()
    : font (1.0f)
{
}

CoreGraphicsContext::SavedState::SavedState (const SavedState& other)
    : fillType (other.fillType), font (other.font), fontRef (other.fontRef),
      textMatrix (other.textMatrix), inverseTextMatrix (other.inverseTextMatrix),
      gradient (other.gradient)
{
    if (gradient != nullptr)
        CGGradientRetain (gradient);
}

CoreGraphicsContext::SavedState::~SavedState()
{
    if (gradient != nullptr)
        CGGradientRelease (gradient);
}

void CoreGraphicsContext::SavedState::setFill (const FillType& newFill)
{
    fillType = newFill;

    if (gradient != nullptr)
    {
        CGGradientRelease (gradient);
        gradient = nullptr;
    }
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
    CGContextSetAlpha (context, state->fillType.getOpacity());

    auto& g = *state->fillType.gradient;

    if (state->gradient == nullptr)
        state->gradient = createGradient (g, rgbColourSpace);

    auto p1 = convertToCGPoint (g.point1);
    auto p2 = convertToCGPoint (g.point2);

    if (g.isRadial)
        CGContextDrawRadialGradient (context, state->gradient, p1, 0, p1, g.point1.getDistanceFrom (g.point2),
                                     kCGGradientDrawsBeforeStartLocation | kCGGradientDrawsAfterEndLocation);
    else
        CGContextDrawLinearGradient (context, state->gradient, p1, p2,
                                     kCGGradientDrawsBeforeStartLocation | kCGGradientDrawsAfterEndLocation);
}

void CoreGraphicsContext::createPath (const Path& path) const
{
    CGContextBeginPath (context);

    for (Path::Iterator i (path); i.next();)
    {
        switch (i.elementType)
        {
            case Path::Iterator::startNewSubPath:  CGContextMoveToPoint (context, i.x1, i.y1); break;
            case Path::Iterator::lineTo:           CGContextAddLineToPoint (context, i.x1, i.y1); break;
            case Path::Iterator::quadraticTo:      CGContextAddQuadCurveToPoint (context, i.x1, i.y1, i.x2, i.y2); break;
            case Path::Iterator::cubicTo:          CGContextAddCurveToPoint (context, i.x1, i.y1, i.x2, i.y2, i.x3, i.y3); break;
            case Path::Iterator::closePath:        CGContextClosePath (context); break;
            default:                               jassertfalse; break;
        }
    }
}

void CoreGraphicsContext::createPath (const Path& path, const AffineTransform& transform) const
{
    CGContextBeginPath (context);

    for (Path::Iterator i (path); i.next();)
    {
        switch (i.elementType)
        {
        case Path::Iterator::startNewSubPath:
            transform.transformPoint (i.x1, i.y1);
            CGContextMoveToPoint (context, i.x1, flipHeight - i.y1);
            break;
        case Path::Iterator::lineTo:
            transform.transformPoint (i.x1, i.y1);
            CGContextAddLineToPoint (context, i.x1, flipHeight - i.y1);
            break;
        case Path::Iterator::quadraticTo:
            transform.transformPoints (i.x1, i.y1, i.x2, i.y2);
            CGContextAddQuadCurveToPoint (context, i.x1, flipHeight - i.y1, i.x2, flipHeight - i.y2);
            break;
        case Path::Iterator::cubicTo:
            transform.transformPoints (i.x1, i.y1, i.x2, i.y2, i.x3, i.y3);
            CGContextAddCurveToPoint (context, i.x1, flipHeight - i.y1, i.x2, flipHeight - i.y2, i.x3, flipHeight - i.y3);
            break;
        case Path::Iterator::closePath:
            CGContextClosePath (context); break;
        default:
            jassertfalse;
            break;
        }
    }
}

void CoreGraphicsContext::flip() const
{
    CGContextConcatCTM (context, CGAffineTransformMake (1, 0, 0, -1, 0, flipHeight));
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
    CGContextConcatCTM (context, t);
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
        auto provider = CGDataProviderCreateWithData (new MemoryBlockHolder::Ptr (memBlockHolder),
                                                      memBlockHolder->block.getData(),
                                                      memBlockHolder->block.getSize(),
                                                      [] (void * __nullable info, const void*, size_t) { delete (MemoryBlockHolder::Ptr*) info; });
        auto imageSource = CGImageSourceCreateWithDataProvider (provider, nullptr);
        CGDataProviderRelease (provider);

        if (imageSource != nullptr)
        {
            auto loadedImage = CGImageSourceCreateImageAtIndex (imageSource, 0, nullptr);
            CFRelease (imageSource);
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

                CGContextDrawImage (cgImage->context, convertToCGRect (image.getBounds()), loadedImage);
                CGContextFlush (cgImage->context);

               #if ! JUCE_IOS
                CFRelease (loadedImage);
               #endif

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

    CIContext* cic = [CIContext contextWithCGContext: cgImage->context options: nil];
    [cic drawImage: im inRect: CGRectMake (0, 0, w, h) fromRect: CGRectMake (0, 0, w, h)];
    CGContextFlush (cgImage->context);

    return Image (*cgImage);
}

CGImageRef juce_createCoreGraphicsImage (const Image& juceImage, CGColorSpaceRef colourSpace,
                                         const bool mustOutliveSource)
{
    return CoreGraphicsPixelData::createImage (juceImage, colourSpace, mustOutliveSource);
}

CGContextRef juce_getImageContext (const Image& image)
{
    if (auto cgi = dynamic_cast<CoreGraphicsPixelData*> (image.getPixelData()))
        return cgi->context;

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
 NSImage* imageToNSImage (const Image& image, float scaleFactor)
 {
     JUCE_AUTORELEASEPOOL
     {
         NSImage* im = [[NSImage alloc] init];
         auto requiredSize = NSMakeSize (image.getWidth() / scaleFactor, image.getHeight() / scaleFactor);

         [im setSize: requiredSize];
         auto colourSpace = CGColorSpaceCreateDeviceRGB();
         auto imageRef = juce_createCoreGraphicsImage (image, colourSpace, true);
         CGColorSpaceRelease (colourSpace);

         NSBitmapImageRep* imageRep = [[NSBitmapImageRep alloc] initWithCGImage: imageRef];
         [imageRep setSize: requiredSize];
         [im addRepresentation: imageRep];
         [imageRep release];
         CGImageRelease (imageRef);
         return im;
     }
 }
#endif

}
