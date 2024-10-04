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
// This class has been renamed from CoreGraphicsImage to avoid a symbol
// collision in Pro Tools 2019.12 and possibly 2020 depending on the Pro Tools
// release schedule.
class CoreGraphicsPixelData final : public ImagePixelData
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

    struct ImageDataContainer final : public ReferenceCountedObject
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

struct CoreGraphicsContext::SavedState
{
    SavedState() = default;

    SavedState (const SavedState& other)
        : fillType (other.fillType), font (other.font),
          textMatrix (other.textMatrix), inverseTextMatrix (other.inverseTextMatrix),
          gradient (other.gradient.get() != nullptr ? CGGradientRetain (other.gradient.get()) : nullptr)
    {
    }

    ~SavedState() = default;

    void setFill (const FillType& newFill)
    {
        fillType = newFill;
        gradient = nullptr;
    }

    FillType fillType;
    Font font { FontOptions { 1.0f } };
    CFUniquePtr<CTFontRef> fontRef{};
    CGAffineTransform textMatrix = CGAffineTransformIdentity,
               inverseTextMatrix = CGAffineTransformIdentity;
    detail::GradientPtr gradient = {};
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
    setFont (FontOptions());
}

CoreGraphicsContext::~CoreGraphicsContext()
{
    CGContextRestoreGState (context.get());
}

//==============================================================================
void CoreGraphicsContext::setOrigin (Point<int> o)
{
    CGContextTranslateCTM (context.get(), o.x, -o.y);

    if (lastClipRect.has_value())
        lastClipRect->translate (-o.x, -o.y);
}

void CoreGraphicsContext::addTransform (const AffineTransform& transform)
{
    applyTransform (AffineTransform::verticalFlip ((float) flipHeight)
                                    .followedBy (transform)
                                    .translated (0, (float) -flipHeight)
                                    .scaled (1.0f, -1.0f));
    lastClipRect.reset();

    jassert (getPhysicalPixelScaleFactor() > 0.0f);
}

float CoreGraphicsContext::getPhysicalPixelScaleFactor() const
{
    auto t = CGContextGetUserSpaceToDeviceSpaceTransform (context.get());
    auto determinant = (t.a * t.d) - (t.c * t.b);

    return (float) std::sqrt (std::abs (determinant));
}

bool CoreGraphicsContext::clipToRectangle (const Rectangle<int>& r)
{
    CGContextClipToRect (context.get(), CGRectMake (r.getX(), flipHeight - r.getBottom(),
                                                    r.getWidth(), r.getHeight()));

    if (lastClipRect.has_value())
    {
        // This is actually incorrect, because the actual clip region may be complex, and
        // clipping its bounds to a rect may not be right... But, removing this shortcut
        // doesn't actually fix anything because CoreGraphics also ignores complex regions
        // when calculating the resultant clip bounds, and makes the same mistake!
        lastClipRect = lastClipRect->getIntersection (r);
        return ! lastClipRect->isEmpty();
    }

    return ! isClipEmpty();
}

bool CoreGraphicsContext::clipToRectangleListWithoutTest (const RectangleList<float>& clipRegion)
{
    if (clipRegion.isEmpty())
    {
        CGContextClipToRect (context.get(), CGRectZero);
        lastClipRect = Rectangle<int>();
        return false;
    }

    std::vector<CGRect> rects ((size_t) clipRegion.getNumRectangles());
    std::transform (clipRegion.begin(), clipRegion.end(), rects.begin(), [this] (const auto& r)
    {
        return CGRectMake (r.getX(), flipHeight - r.getBottom(), r.getWidth(), r.getHeight());
    });

    CGContextClipToRects (context.get(), rects.data(), rects.size());
    lastClipRect.reset();
    return true;
}

bool CoreGraphicsContext::clipToRectangleList (const RectangleList<int>& clipRegion)
{
    RectangleList<float> converted;

    for (auto& rect : clipRegion)
        converted.add (rect.toFloat());

    return clipToRectangleListWithoutTest (converted) && ! isClipEmpty();
}

void CoreGraphicsContext::excludeClipRectangle (const Rectangle<int>& r)
{
    const auto cgTransform = CGContextGetUserSpaceToDeviceSpaceTransform (context.get());
    const auto transform = AffineTransform { (float) cgTransform.a,
                                             (float) cgTransform.c,
                                             (float) cgTransform.tx,
                                             (float) cgTransform.b,
                                             (float) cgTransform.d,
                                             (float) cgTransform.ty };

    const auto flip = [this] (auto rect) { return rect.withY ((float) (flipHeight - rect.getBottom())); };
    const auto flipped = flip (r.toFloat());

    const auto snapped = flipped.toFloat().transformedBy (transform).getLargestIntegerWithin().toFloat();

    const auto correctedRect = transform.isOnlyTranslationOrScale()
                             ? snapped.transformedBy (transform.inverted())
                             : flipped.toFloat();

    RectangleList<float> remaining (getClipBounds().toFloat());
    remaining.subtract (flip (correctedRect));
    clipToRectangleListWithoutTest (remaining);
}

void CoreGraphicsContext::setContextClipToCurrentPath (bool useNonZeroWinding)
{
    if (useNonZeroWinding)
        CGContextClip (context.get());
    else
        CGContextEOClip (context.get());
}

void CoreGraphicsContext::clipToPath (const Path& path, const AffineTransform& transform)
{
    createPath (path, transform);
    setContextClipToCurrentPath (path.isUsingNonZeroWinding());
    lastClipRect.reset();
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
        auto t = AffineTransform::verticalFlip ((float) sourceImage.getHeight()).followedBy (transform);
        applyTransform (t);

        auto r = convertToCGRect (sourceImage.getBounds());
        CGContextClipToMask (context.get(), r, image.get());
        CGContextClipToRect (context.get(), r);

        applyTransform (t.inverted());
        flip();

        lastClipRect.reset();
    }
}

bool CoreGraphicsContext::clipRegionIntersects (const Rectangle<int>& r)
{
    return getClipBounds().intersects (r);
}

Rectangle<int> CoreGraphicsContext::getClipBounds() const
{
    if (! lastClipRect.has_value())
    {
        auto bounds = CGRectIntegral (CGContextGetClipBoundingBox (context.get()));

        lastClipRect = Rectangle<int> (roundToInt (bounds.origin.x),
                                       roundToInt (flipHeight - (bounds.origin.y + bounds.size.height)),
                                       roundToInt (bounds.size.width),
                                       roundToInt (bounds.size.height));
    }

    return *lastClipRect;
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
        lastClipRect.reset();
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
        const CGFloat components[] { fillType.colour.getFloatRed(),
                                     fillType.colour.getFloatGreen(),
                                     fillType.colour.getFloatBlue(),
                                     fillType.colour.getFloatAlpha() };

        const detail::ColorPtr color { CGColorCreate (rgbColourSpace.get(), components) };
        CGContextSetFillColorWithColor (context.get(), color.get());
        CGContextSetStrokeColorWithColor (context.get(), color.get());
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
void CoreGraphicsContext::fillAll()
{
    // The clip rectangle is expanded in order to avoid having alpha blended pixels at the edges.
    // The clipping mechanism will take care of cutting off pixels beyond the clip bounds. This is
    // a hard cutoff and will ensure that no semi-transparent pixels will remain inside the filled
    // area.
    const auto clipBounds = getClipBounds();

    const auto clipBoundsOnDevice = CGContextConvertSizeToDeviceSpace (context.get(),
                                                                       CGSize { (CGFloat) clipBounds.getWidth(),
                                                                                (CGFloat) clipBounds.getHeight() });

    const auto inverseScale = clipBoundsOnDevice.width > (CGFloat) 0.0
                            ? (int) (clipBounds.getWidth() / clipBoundsOnDevice.width)
                            : 0;
    const auto expansion = jmax (1, inverseScale);

    fillRect (clipBounds.expanded (expansion), false);
}

void CoreGraphicsContext::fillRect (const Rectangle<int>& r, bool replaceExistingContents)
{
    fillCGRect (convertToCGRectFlipped (r), replaceExistingContents);
}

void CoreGraphicsContext::fillRect (const Rectangle<float>& r)
{
    fillCGRect (convertToCGRectFlipped (r), false);
}

void CoreGraphicsContext::fillCGRect (const CGRect& cgRect, bool replaceExistingContents)
{
    if (CGRectIsEmpty (cgRect))
        return;

    if (replaceExistingContents)
    {
        CGContextSetBlendMode (context.get(), kCGBlendModeCopy);
        fillCGRect (cgRect, false);
        CGContextSetBlendMode (context.get(), kCGBlendModeNormal);
        return;
    }

    if (state->fillType.isColour())
    {
        CGContextFillRect (context.get(), cgRect);
        return;
    }

    ScopedCGContextState scopedState (context.get());
    CGContextClipToRect (context.get(), cgRect);

    if (state->fillType.isGradient())
        drawGradient();
    else
        drawImage (state->fillType.image, state->fillType.transform, true);
}

void CoreGraphicsContext::drawCurrentPath (CGPathDrawingMode mode)
{
    if (state->fillType.isColour())
    {
        CGContextDrawPath (context.get(), mode);
        return;
    }

    if (mode == kCGPathStroke)
        CGContextReplacePathWithStrokedPath (context.get());

    ScopedCGContextState scopedState (context.get());
    setContextClipToCurrentPath (  mode == kCGPathFill
                                 || mode == kCGPathStroke
                                 || mode == kCGPathFillStroke);

    if (state->fillType.isGradient())
        drawGradient();
    else
        drawImage (state->fillType.image, state->fillType.transform, true);
}

void CoreGraphicsContext::fillPath (const Path& path, const AffineTransform& transform)
{
    createPath (path, transform);
    drawCurrentPath (path.isUsingNonZeroWinding() ? kCGPathFill : kCGPathEOFill);
}

void CoreGraphicsContext::strokePath (const Path& path, const PathStrokeType& strokeType, const AffineTransform& transform)
{
    const auto lineCap = [&]
    {
        switch (strokeType.getEndStyle())
        {
            case PathStrokeType::EndCapStyle::butt:    return kCGLineCapButt;
            case PathStrokeType::EndCapStyle::square:  return kCGLineCapSquare;
            case PathStrokeType::EndCapStyle::rounded: return kCGLineCapRound;
            default: jassertfalse;                     return kCGLineCapButt;
        }
    }();

    const auto lineJoin = [&]
    {
        switch (strokeType.getJointStyle())
        {
            case PathStrokeType::JointStyle::mitered: return kCGLineJoinMiter;
            case PathStrokeType::JointStyle::curved:  return kCGLineJoinRound;
            case PathStrokeType::JointStyle::beveled: return kCGLineJoinBevel;
            default: jassertfalse;                    return kCGLineJoinMiter;
        }
    }();

    CGContextSetLineWidth (context.get(), strokeType.getStrokeThickness());
    CGContextSetLineCap (context.get(), lineCap);
    CGContextSetLineJoin (context.get(), lineJoin);

    createPath (path, transform);
    drawCurrentPath (kCGPathStroke);
}

void CoreGraphicsContext::drawEllipse (const Rectangle<float>& area, float lineThickness)
{
    CGContextBeginPath (context.get());
    CGContextSetLineWidth (context.get(), lineThickness);
    CGContextSetLineCap (context.get(), kCGLineCapButt);
    CGContextSetLineJoin (context.get(), kCGLineJoinMiter);
    CGContextAddEllipseInRect (context.get(), convertToCGRectFlipped (area));
    drawCurrentPath (kCGPathStroke);
}

void CoreGraphicsContext::fillEllipse (const Rectangle<float>& area)
{
    CGContextBeginPath (context.get());
    CGContextAddEllipseInRect (context.get(), convertToCGRectFlipped (area));
    drawCurrentPath (kCGPathFill);
}

void CoreGraphicsContext::drawRoundedRectangle (const Rectangle<float>& r, float cornerSize, float lineThickness)
{
    CGContextBeginPath (context.get());

    if (state->fillType.isColour())
    {
        CGContextSetLineWidth (context.get(), lineThickness);
        CGContextSetLineCap (context.get(), kCGLineCapButt);
        CGContextSetLineJoin (context.get(), kCGLineJoinMiter);

        detail::PathPtr path { CGPathCreateWithRoundedRect (convertToCGRectFlipped (r), cornerSize, cornerSize, nullptr) };
        CGContextAddPath (context.get(), path.get());
        drawCurrentPath (kCGPathStroke);
    }
    else
    {
        lineThickness *= 0.5f;

        const auto outsideRect = r.expanded (lineThickness);
        const auto outsideCornerSize = cornerSize + lineThickness;

        const auto insideRect = r.reduced (lineThickness);
        const auto insideCornerSize = cornerSize - lineThickness;

        detail::MutablePathPtr path { CGPathCreateMutable() };
        CGPathAddRoundedRect (path.get(), nullptr, convertToCGRectFlipped (outsideRect),
                              std::clamp (outsideCornerSize, 0.0f, outsideRect.getWidth() / 2.0f),
                              std::clamp (outsideCornerSize, 0.0f, outsideRect.getHeight() / 2.0f));

        CGPathAddRoundedRect (path.get(), nullptr, convertToCGRectFlipped (insideRect),
                              std::clamp (insideCornerSize, 0.0f, insideRect.getWidth() / 2.0f),
                              std::clamp (insideCornerSize, 0.0f, insideRect.getHeight() / 2.0f));

        CGContextAddPath (context.get(), path.get());
        drawCurrentPath (kCGPathEOFill);
    }
}

void CoreGraphicsContext::fillRoundedRectangle (const Rectangle<float>& r, float cornerSize)
{
    CGContextBeginPath (context.get());
    detail::PathPtr path { CGPathCreateWithRoundedRect (convertToCGRectFlipped (r), cornerSize, cornerSize, nullptr) };
    CGContextAddPath (context.get(), path.get());
    drawCurrentPath (kCGPathFill);
}

void CoreGraphicsContext::drawLineWithThickness (const Line<float>& line, float lineThickness)
{
    const auto reversed = line.reversed();
    lineThickness *= 0.5f;
    const auto p1 = line.getPointAlongLine (0, lineThickness);
    const auto p2 = line.getPointAlongLine (0, -lineThickness);
    const auto p3 = reversed.getPointAlongLine (0, lineThickness);
    const auto p4 = reversed.getPointAlongLine (0, -lineThickness);

    CGContextBeginPath      (context.get());
    CGContextMoveToPoint    (context.get(), (CGFloat) p1.getX(), flipHeight - (CGFloat) p1.getY());
    CGContextAddLineToPoint (context.get(), (CGFloat) p2.getX(), flipHeight - (CGFloat) p2.getY());
    CGContextAddLineToPoint (context.get(), (CGFloat) p3.getX(), flipHeight - (CGFloat) p3.getY());
    CGContextAddLineToPoint (context.get(), (CGFloat) p4.getX(), flipHeight - (CGFloat) p4.getY());
    CGContextClosePath      (context.get());

    drawCurrentPath (kCGPathFill);
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
    applyTransform (AffineTransform::verticalFlip ((float) ih).followedBy (transform));
    auto imageRect = CGRectMake (0, 0, iw, ih);

    if (fillEntireClipAsTiles)
    {
      #if JUCE_IOS
        CGContextDrawTiledImage (context.get(), imageRect, image.get());
      #else
        // There's a bug in CGContextDrawTiledImage that makes it incredibly slow
        // if it's doing a transformation - it's quicker to just draw lots of images manually,
        // but we might not be able to draw the images ourselves if the clipping region is not
        // finite
        const auto doCustomTiling = [&]
        {
            if (transform.isOnlyTranslation())
                return false;

            const auto bound = CGContextGetClipBoundingBox (context.get());

            if (CGRectIsNull (bound))
                return false;

            const auto clip = CGRectIntegral (bound);

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

            return true;
        };

        if (! doCustomTiling())
            CGContextDrawTiledImage (context.get(), imageRect, image.get());
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
    if (list.isEmpty())
        return;

    std::vector<CGRect> rects;
    rects.reserve ((size_t) list.getNumRectangles());

    for (auto& r : list)
        rects.push_back (CGRectMake (r.getX(), flipHeight - r.getBottom(), r.getWidth(), r.getHeight()));

    if (state->fillType.isColour())
    {
        CGContextFillRects (context.get(), rects.data(), rects.size());
        return;
    }

    ScopedCGContextState scopedState (context.get());
    CGContextClipToRects (context.get(), rects.data(), rects.size());

    if (state->fillType.isGradient())
        drawGradient();
    else
        drawImage (state->fillType.image, state->fillType.transform, true);
}

void CoreGraphicsContext::setFont (const Font& newFont)
{
    if (state->font != newFont)
    {
        state->font = newFont;
        state->fontRef = nullptr;
    }
}

const Font& CoreGraphicsContext::getFont()
{
    return state->font;
}

void CoreGraphicsContext::drawGlyphs (Span<const uint16_t> glyphs,
                                      Span<const Point<float>> positions,
                                      const AffineTransform& transform)
{
    jassert (glyphs.size() == positions.size());

    if (state->fillType.isColour())
    {
        const auto scale = state->font.getHorizontalScale();

        if (state->fontRef == nullptr)
        {
            const auto hbFont = state->font.getNativeDetails().font;
            state->fontRef.reset (hb_coretext_font_get_ct_font (hbFont.get()));
            CFRetain (state->fontRef.get());

            const auto slant = hb_font_get_synthetic_slant (hbFont.get());

            state->textMatrix = CGAffineTransformMake (scale, 0, slant * scale, 1.0f, 0, 0);
            CGContextSetTextMatrix (context.get(), state->textMatrix);
            state->inverseTextMatrix = CGAffineTransformInvert (state->textMatrix);
        }

        ScopedCGContextState scopedState (context.get());
        flip();
        applyTransform (AffineTransform::scale (1.0f, -1.0f).followedBy (transform));

        CopyableHeapBlock<CGPoint> pos (glyphs.size());
        std::transform (positions.begin(), positions.end(), pos.begin(), [scale] (const auto& p) { return CGPointMake (p.x / scale, -p.y); });

        CTFontDrawGlyphs (state->fontRef.get(), glyphs.data(), pos.data(), glyphs.size(), context.get());
        return;
    }

    for (const auto [index, glyph] : enumerate (glyphs, size_t{}))
    {
        Path p;
        auto& f = state->font;
        f.getTypefacePtr()->getOutlineForGlyph (f.getMetricsKind(), glyph, p);
        const auto scale = f.getHeight();
        fillPath (p, AffineTransform::scale (scale * f.getHorizontalScale(), scale).translated (positions[index]).followedBy (transform));
    }
}

static CGGradientRef createGradient (const ColourGradient& g, CGColorSpaceRef colourSpace)
{
    auto numColours = g.getNumColours();
    std::vector<CGFloat> data ((size_t) numColours * 5);
    auto locations = data.data();
    auto components = locations + numColours;
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
        // to go wrong if you have two colour stops both at position 0.
        jassert (i == 0 || ! approximatelyEqual (locations[i], (CGFloat) 0.0));
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

template <class RectType>
CGRect CoreGraphicsContext::convertToCGRectFlipped (RectType r) const noexcept
{
    return CGRectMake ((CGFloat) r.getX(),
                       flipHeight - (CGFloat) r.getBottom(),
                       (CGFloat) r.getWidth(),
                       (CGFloat) r.getHeight());
}

//==============================================================================
#if USE_COREGRAPHICS_RENDERING && JUCE_USE_COREIMAGE_LOADER
Image juce_loadWithCoreImage (InputStream& input)
{
    struct MemoryBlockHolder final : public ReferenceCountedObject
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

     CGContextDrawImage (ctx, CGRectMake (0.0f, 0.0f, (CGFloat) CGImageGetWidth (image), (CGFloat) CGImageGetHeight (image)), image);

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
