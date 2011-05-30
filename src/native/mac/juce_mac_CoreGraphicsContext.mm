/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

// (This file gets included by juce_mac_NativeCode.mm, rather than being
// compiled on its own).
#if JUCE_INCLUDED_FILE

//==============================================================================
class CoreGraphicsImage : public Image::SharedImage
{
public:
    //==============================================================================
    CoreGraphicsImage (const Image::PixelFormat format_, const int width_, const int height_, const bool clearImage)
        : Image::SharedImage (format_, width_, height_)
    {
        pixelStride = format_ == Image::RGB ? 3 : ((format_ == Image::ARGB) ? 4 : 1);
        lineStride = (pixelStride * jmax (1, width) + 3) & ~3;

        imageData.allocate (lineStride * jmax (1, height), clearImage);

        CGColorSpaceRef colourSpace = (format == Image::SingleChannel) ? CGColorSpaceCreateDeviceGray()
                                                                       : CGColorSpaceCreateDeviceRGB();

        context = CGBitmapContextCreate (imageData, width, height, 8, lineStride,
                                         colourSpace, getCGImageFlags (format_));

        CGColorSpaceRelease (colourSpace);
    }

    ~CoreGraphicsImage()
    {
        CGContextRelease (context);
    }

    Image::ImageType getType() const    { return Image::NativeImage; }
    LowLevelGraphicsContext* createLowLevelContext();

    void initialiseBitmapData (Image::BitmapData& bitmap, int x, int y, Image::BitmapData::ReadWriteMode /*mode*/)
    {
        bitmap.data = imageData + x * pixelStride + y * lineStride;
        bitmap.pixelFormat = format;
        bitmap.lineStride = lineStride;
        bitmap.pixelStride = pixelStride;
    }

    SharedImage* clone()
    {
        CoreGraphicsImage* im = new CoreGraphicsImage (format, width, height, false);
        memcpy (im->imageData, imageData, lineStride * height);
        return im;
    }

    //==============================================================================
    static CGImageRef createImage (const Image& juceImage, const bool forAlpha,
                                   CGColorSpaceRef colourSpace, const bool mustOutliveSource)
    {
        const CoreGraphicsImage* nativeImage = dynamic_cast <const CoreGraphicsImage*> (juceImage.getSharedImage());

        if (nativeImage != nullptr && (juceImage.getFormat() == Image::SingleChannel || ! forAlpha))
        {
            return CGBitmapContextCreateImage (nativeImage->context);
        }
        else
        {
            const Image::BitmapData srcData (juceImage, Image::BitmapData::readOnly);
            CGDataProviderRef provider;

            if (mustOutliveSource)
            {
                CFDataRef data = CFDataCreate (0, (const UInt8*) srcData.data, (CFIndex) (srcData.lineStride * srcData.height));
                provider = CGDataProviderCreateWithCFData (data);
                CFRelease (data);
            }
            else
            {
                provider = CGDataProviderCreateWithData (0, srcData.data, srcData.lineStride * srcData.height, 0);
            }

            CGImageRef imageRef = CGImageCreate (srcData.width, srcData.height,
                                                 8, srcData.pixelStride * 8, srcData.lineStride,
                                                 colourSpace, getCGImageFlags (juceImage.getFormat()), provider,
                                                 0, true, kCGRenderingIntentDefault);

            CGDataProviderRelease (provider);
            return imageRef;
        }
    }

  #if JUCE_MAC
    static NSImage* createNSImage (const Image& image)
    {
        JUCE_AUTORELEASEPOOL

        NSImage* im = [[NSImage alloc] init];
        [im setSize: NSMakeSize (image.getWidth(), image.getHeight())];
        [im lockFocus];

        CGColorSpaceRef colourSpace = CGColorSpaceCreateDeviceRGB();
        CGImageRef imageRef = createImage (image, false, colourSpace, false);
        CGColorSpaceRelease (colourSpace);

        CGContextRef cg = (CGContextRef) [[NSGraphicsContext currentContext] graphicsPort];
        CGContextDrawImage (cg, CGRectMake (0, 0, image.getWidth(), image.getHeight()), imageRef);

        CGImageRelease (imageRef);
        [im unlockFocus];

        return im;
    }
  #endif

    //==============================================================================
    CGContextRef context;
    HeapBlock<uint8> imageData;
    int pixelStride, lineStride;

private:
    static CGBitmapInfo getCGImageFlags (const Image::PixelFormat& format)
    {
      #if JUCE_BIG_ENDIAN
        return format == Image::ARGB ? (kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Big) : kCGBitmapByteOrderDefault;
      #else
        return format == Image::ARGB ? (kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Little) : kCGBitmapByteOrderDefault;
      #endif
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CoreGraphicsImage);
};

Image::SharedImage* Image::SharedImage::createNativeImage (PixelFormat format, int width, int height, bool clearImage)
{
  #if USE_COREGRAPHICS_RENDERING
    return new CoreGraphicsImage (format == RGB ? ARGB : format, width, height, clearImage);
  #else
    return createSoftwareImage (format, width, height, clearImage);
  #endif
}

//==============================================================================
class CoreGraphicsContext   : public LowLevelGraphicsContext
{
public:
    CoreGraphicsContext (CGContextRef context_, const float flipHeight_)
        : context (context_),
          flipHeight (flipHeight_),
          lastClipRectIsValid (false),
          state (new SavedState())
    {
        CGContextRetain (context);
        CGContextSaveGState(context);
        CGContextSetShouldSmoothFonts (context, true);
        CGContextSetShouldAntialias (context, true);
        CGContextSetBlendMode (context, kCGBlendModeNormal);
        rgbColourSpace = CGColorSpaceCreateDeviceRGB();
        greyColourSpace = CGColorSpaceCreateDeviceGray();
        gradientCallbacks.version = 0;
        gradientCallbacks.evaluate = SavedState::gradientCallback;
        gradientCallbacks.releaseInfo = 0;
        setFont (Font());
    }

    ~CoreGraphicsContext()
    {
        CGContextRestoreGState (context);
        CGContextRelease (context);
        CGColorSpaceRelease (rgbColourSpace);
        CGColorSpaceRelease (greyColourSpace);
    }

    //==============================================================================
    bool isVectorDevice() const         { return false; }

    void setOrigin (int x, int y)
    {
        CGContextTranslateCTM (context, x, -y);

        if (lastClipRectIsValid)
            lastClipRect.translate (-x, -y);
    }

    void addTransform (const AffineTransform& transform)
    {
        applyTransform (AffineTransform::scale (1.0f, -1.0f)
                                        .translated (0, flipHeight)
                                        .followedBy (transform)
                                        .translated (0, -flipHeight)
                                        .scaled (1.0f, -1.0f));
        lastClipRectIsValid = false;
    }

    float getScaleFactor()
    {
        CGAffineTransform t = CGContextGetCTM (context);
        return (float) juce_hypot (t.a + t.c, t.b + t.d);
    }

    bool clipToRectangle (const Rectangle<int>& r)
    {
        CGContextClipToRect (context, CGRectMake (r.getX(), flipHeight - r.getBottom(), r.getWidth(), r.getHeight()));

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

    bool clipToRectangleList (const RectangleList& clipRegion)
    {
        if (clipRegion.isEmpty())
        {
            CGContextClipToRect (context, CGRectMake (0, 0, 0, 0));
            lastClipRectIsValid = true;
            lastClipRect = Rectangle<int>();
            return false;
        }
        else
        {
            const int numRects = clipRegion.getNumRectangles();

            HeapBlock <CGRect> rects (numRects);
            for (int i = 0; i < numRects; ++i)
            {
                const Rectangle<int>& r = clipRegion.getRectangle(i);
                rects[i] = CGRectMake (r.getX(), flipHeight - r.getBottom(), r.getWidth(), r.getHeight());
            }

            CGContextClipToRects (context, rects, numRects);
            lastClipRectIsValid = false;
            return ! isClipEmpty();
        }
    }

    void excludeClipRectangle (const Rectangle<int>& r)
    {
        RectangleList remaining (getClipBounds());
        remaining.subtract (r);
        clipToRectangleList (remaining);
        lastClipRectIsValid = false;
    }

    void clipToPath (const Path& path, const AffineTransform& transform)
    {
        createPath (path, transform);
        CGContextClip (context);
        lastClipRectIsValid = false;
    }

    void clipToImageAlpha (const Image& sourceImage, const AffineTransform& transform)
    {
        if (! transform.isSingularity())
        {
            Image singleChannelImage (sourceImage);

            if (sourceImage.getFormat() != Image::SingleChannel)
                singleChannelImage = sourceImage.convertedToFormat (Image::SingleChannel);

            CGImageRef image = CoreGraphicsImage::createImage (singleChannelImage, true, greyColourSpace, true);

            flip();
            AffineTransform t (AffineTransform::scale (1.0f, -1.0f).translated (0, sourceImage.getHeight()).followedBy (transform));
            applyTransform (t);

            CGRect r = CGRectMake (0, 0, sourceImage.getWidth(), sourceImage.getHeight());
            CGContextClipToMask (context, r, image);

            applyTransform (t.inverted());
            flip();

            CGImageRelease (image);
            lastClipRectIsValid = false;
        }
    }

    bool clipRegionIntersects (const Rectangle<int>& r)
    {
        return getClipBounds().intersects (r);
    }

    const Rectangle<int> getClipBounds() const
    {
        if (! lastClipRectIsValid)
        {
            CGRect bounds = CGRectIntegral (CGContextGetClipBoundingBox (context));

            lastClipRectIsValid = true;
            lastClipRect.setBounds (roundToInt (bounds.origin.x),
                                    roundToInt (flipHeight - (bounds.origin.y + bounds.size.height)),
                                    roundToInt (bounds.size.width),
                                    roundToInt (bounds.size.height));
        }

        return lastClipRect;
    }

    bool isClipEmpty() const
    {
        return getClipBounds().isEmpty();
    }

    //==============================================================================
    void saveState()
    {
        CGContextSaveGState (context);
        stateStack.add (new SavedState (*state));
    }

    void restoreState()
    {
        CGContextRestoreGState (context);

        SavedState* const top = stateStack.getLast();

        if (top != nullptr)
        {
            state = top;
            stateStack.removeLast (1, false);
            lastClipRectIsValid = false;
        }
        else
        {
            jassertfalse; // trying to pop with an empty stack!
        }
    }

    void beginTransparencyLayer (float opacity)
    {
        saveState();
        CGContextSetAlpha (context, opacity);
        CGContextBeginTransparencyLayer (context, 0);
    }

    void endTransparencyLayer()
    {
        CGContextEndTransparencyLayer (context);
        restoreState();
    }

    //==============================================================================
    void setFill (const FillType& fillType)
    {
        state->setFill (fillType);

        if (fillType.isColour())
        {
            CGContextSetRGBFillColor (context, fillType.colour.getFloatRed(), fillType.colour.getFloatGreen(),
                                      fillType.colour.getFloatBlue(), fillType.colour.getFloatAlpha());
            CGContextSetAlpha (context, 1.0f);
        }
    }

    void setOpacity (float newOpacity)
    {
        state->fillType.setOpacity (newOpacity);
        setFill (state->fillType);
    }

    void setInterpolationQuality (Graphics::ResamplingQuality quality)
    {
        CGContextSetInterpolationQuality (context, quality == Graphics::lowResamplingQuality
                                                    ? kCGInterpolationLow
                                                    : kCGInterpolationHigh);
    }

    //==============================================================================
    void fillRect (const Rectangle<int>& r, const bool replaceExistingContents)
    {
        fillCGRect (CGRectMake (r.getX(), flipHeight - r.getBottom(), r.getWidth(), r.getHeight()), replaceExistingContents);
    }

    void fillCGRect (const CGRect& cgRect, const bool replaceExistingContents)
    {
        if (replaceExistingContents)
        {
          #if MAC_OS_X_VERSION_MAX_ALLOWED < MAC_OS_X_VERSION_10_5
            CGContextClearRect (context, cgRect);
          #else
           #if MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_5
            if (CGContextDrawLinearGradient == 0) // (just a way of checking whether we're running in 10.5 or later)
                CGContextClearRect (context, cgRect);
            else
           #endif
                CGContextSetBlendMode (context, kCGBlendModeCopy);
          #endif

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

    void fillPath (const Path& path, const AffineTransform& transform)
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

    void drawImage (const Image& sourceImage, const AffineTransform& transform, const bool fillEntireClipAsTiles)
    {
        const int iw = sourceImage.getWidth();
        const int ih = sourceImage.getHeight();
        CGImageRef image = CoreGraphicsImage::createImage (sourceImage, false, rgbColourSpace, false);

        CGContextSaveGState (context);
        CGContextSetAlpha (context, state->fillType.getOpacity());

        flip();
        applyTransform (AffineTransform::scale (1.0f, -1.0f).translated (0, ih).followedBy (transform));
        CGRect imageRect = CGRectMake (0, 0, iw, ih);

        if (fillEntireClipAsTiles)
        {
          #if JUCE_IOS
            CGContextDrawTiledImage (context, imageRect, image);
          #else
           #if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_5
            // There's a bug in CGContextDrawTiledImage that makes it incredibly slow
            // if it's doing a transformation - it's quicker to just draw lots of images manually
            if (CGContextDrawTiledImage != 0 && transform.isOnlyTranslation())
                CGContextDrawTiledImage (context, imageRect, image);
            else
           #endif
            {
                // Fallback to manually doing a tiled fill on 10.4
                CGRect clip = CGRectIntegral (CGContextGetClipBoundingBox (context));

                int x = 0, y = 0;
                while (x > clip.origin.x)   x -= iw;
                while (y > clip.origin.y)   y -= ih;

                const int right = (int) (clip.origin.x + clip.size.width);
                const int bottom = (int) (clip.origin.y + clip.size.height);

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

        CGImageRelease (image); // (This causes a memory bug in iPhone sim 3.0 - try upgrading to a later version if you hit this)
        CGContextRestoreGState (context);
    }

    //==============================================================================
    void drawLine (const Line<float>& line)
    {
        if (state->fillType.isColour())
        {
            CGContextSetLineCap (context, kCGLineCapSquare);
            CGContextSetLineWidth (context, 1.0f);
            CGContextSetRGBStrokeColor (context,
                                        state->fillType.colour.getFloatRed(), state->fillType.colour.getFloatGreen(),
                                        state->fillType.colour.getFloatBlue(), state->fillType.colour.getFloatAlpha());

            CGPoint cgLine[] = { { (CGFloat) line.getStartX(), flipHeight - (CGFloat) line.getStartY() },
                                 { (CGFloat) line.getEndX(),   flipHeight - (CGFloat) line.getEndY()   } };

            CGContextStrokeLineSegments (context, cgLine, 1);
        }
        else
        {
            Path p;
            p.addLineSegment (line, 1.0f);
            fillPath (p, AffineTransform::identity);
        }
    }

    void drawVerticalLine (const int x, float top, float bottom)
    {
        if (state->fillType.isColour())
        {
          #if MAC_OS_X_VERSION_MIN_REQUIRED > MAC_OS_X_VERSION_10_5
            CGContextFillRect (context, CGRectMake (x, flipHeight - bottom, 1.0f, bottom - top));
          #else
            // On Leopard, unless both co-ordinates are non-integer, it disables anti-aliasing, so nudge
            // the x co-ord slightly to trick it..
            CGContextFillRect (context, CGRectMake (x + 1.0f / 256.0f, flipHeight - bottom, 1.0f + 1.0f / 256.0f, bottom - top));
          #endif
        }
        else
        {
            fillCGRect (CGRectMake ((float) x, flipHeight - bottom, 1.0f, bottom - top), false);
        }
    }

    void drawHorizontalLine (const int y, float left, float right)
    {
        if (state->fillType.isColour())
        {
          #if MAC_OS_X_VERSION_MIN_REQUIRED > MAC_OS_X_VERSION_10_5
            CGContextFillRect (context, CGRectMake (left, flipHeight - (y + 1.0f), right - left, 1.0f));
          #else
            // On Leopard, unless both co-ordinates are non-integer, it disables anti-aliasing, so nudge
            // the x co-ord slightly to trick it..
            CGContextFillRect (context, CGRectMake (left, flipHeight - (y + (1.0f + 1.0f / 256.0f)), right - left, 1.0f + 1.0f / 256.0f));
          #endif
        }
        else
        {
            fillCGRect (CGRectMake (left, flipHeight - (y + 1), right - left, 1.0f), false);
        }
    }

    void setFont (const Font& newFont)
    {
        if (state->font != newFont)
        {
            state->fontRef = 0;
            state->font = newFont;

            MacTypeface* mf = dynamic_cast <MacTypeface*> (state->font.getTypeface());

            if (mf != nullptr)
            {
                state->fontRef = mf->fontRef;
                CGContextSetFont (context, state->fontRef);
                CGContextSetFontSize (context, state->font.getHeight() * mf->fontHeightToCGSizeFactor);

                state->fontTransform = mf->renderingTransform;
                state->fontTransform.a *= state->font.getHorizontalScale();
                CGContextSetTextMatrix (context, state->fontTransform);
            }
        }
    }

    Font getFont()
    {
        return state->font;
    }

    void drawGlyph (int glyphNumber, const AffineTransform& transform)
    {
        if (state->fontRef != 0 && state->fillType.isColour())
        {
            if (transform.isOnlyTranslation())
            {
                CGContextSetTextMatrix (context, state->fontTransform); // have to set this each time, as it's not saved as part of the state

                CGGlyph g = glyphNumber;
                CGContextShowGlyphsAtPoint (context, transform.getTranslationX(),
                                            flipHeight - roundToInt (transform.getTranslationY()), &g, 1);
            }
            else
            {
                CGContextSaveGState (context);
                flip();
                applyTransform (transform);

                CGAffineTransform t = state->fontTransform;
                t.d = -t.d;
                CGContextSetTextMatrix (context, t);

                CGGlyph g = glyphNumber;
                CGContextShowGlyphsAtPoint (context, 0, 0, &g, 1);

                CGContextRestoreGState (context);
            }
        }
        else
        {
            Path p;
            Font& f = state->font;
            f.getTypeface()->getOutlineForGlyph (glyphNumber, p);

            fillPath (p, AffineTransform::scale (f.getHeight() * f.getHorizontalScale(), f.getHeight())
                                         .followedBy (transform));
        }
    }

private:
    CGContextRef context;
    const CGFloat flipHeight;
    CGColorSpaceRef rgbColourSpace, greyColourSpace;
    CGFunctionCallbacks gradientCallbacks;
    mutable Rectangle<int> lastClipRect;
    mutable bool lastClipRectIsValid;

    struct SavedState
    {
        SavedState()
            : font (1.0f), fontRef (0), fontTransform (CGAffineTransformIdentity),
              shading (0), numGradientLookupEntries (0)
        {
        }

        SavedState (const SavedState& other)
            : fillType (other.fillType), font (other.font), fontRef (other.fontRef),
              fontTransform (other.fontTransform), shading (0),
              gradientLookupTable (other.numGradientLookupEntries),
              numGradientLookupEntries (other.numGradientLookupEntries)
        {
            memcpy (gradientLookupTable, other.gradientLookupTable, sizeof (PixelARGB) * numGradientLookupEntries);
        }

        ~SavedState()
        {
            if (shading != 0)
                CGShadingRelease (shading);
        }

        void setFill (const FillType& newFill)
        {
            fillType = newFill;

            if (fillType.isGradient() && shading != 0)
            {
                CGShadingRelease (shading);
                shading = 0;
            }
        }

        CGShadingRef getShading (CoreGraphicsContext& owner)
        {
            if (shading == 0)
            {
                ColourGradient& g = *(fillType.gradient);
                numGradientLookupEntries = g.createLookupTable (fillType.transform, gradientLookupTable) - 1;

                CGFunctionRef function = CGFunctionCreate (this, 1, 0, 4, 0, &(owner.gradientCallbacks));
                CGPoint p1 (CGPointMake (g.point1.getX(), g.point1.getY()));

                if (g.isRadial)
                {
                    shading = CGShadingCreateRadial (owner.rgbColourSpace, p1, 0,
                                                     p1, g.point1.getDistanceFrom (g.point2),
                                                     function, true, true);
                }
                else
                {
                    shading = CGShadingCreateAxial (owner.rgbColourSpace, p1,
                                                    CGPointMake (g.point2.getX(), g.point2.getY()),
                                                    function, true, true);
                }

                CGFunctionRelease (function);
            }

            return shading;
        }

        static void gradientCallback (void* info, const CGFloat* inData, CGFloat* outData)
        {
            const SavedState* const s = static_cast <const SavedState*> (info);

            const int index = roundToInt (s->numGradientLookupEntries * inData[0]);
            PixelARGB colour (s->gradientLookupTable [jlimit (0, s->numGradientLookupEntries, index)]);
            colour.unpremultiply();

            outData[0] = colour.getRed() / 255.0f;
            outData[1] = colour.getGreen() / 255.0f;
            outData[2] = colour.getBlue() / 255.0f;
            outData[3] = colour.getAlpha() / 255.0f;
        }

        FillType fillType;
        Font font;
        CGFontRef fontRef;
        CGAffineTransform fontTransform;

    private:
        CGShadingRef shading;
        HeapBlock <PixelARGB> gradientLookupTable;
        int numGradientLookupEntries;
    };

    ScopedPointer <SavedState> state;
    OwnedArray <SavedState> stateStack;

    void drawGradient()
    {
        flip();
        applyTransform (state->fillType.transform);

        CGContextSetInterpolationQuality (context, kCGInterpolationDefault); // (This is required for 10.4, where there's a crash if
                                                                             // you draw a gradient with high quality interp enabled).
        CGContextSetAlpha (context, state->fillType.getOpacity());
        CGContextDrawShading (context, state->getShading (*this));
    }

    void createPath (const Path& path) const
    {
        CGContextBeginPath (context);
        Path::Iterator i (path);

        while (i.next())
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

    void createPath (const Path& path, const AffineTransform& transform) const
    {
        CGContextBeginPath (context);
        Path::Iterator i (path);

        while (i.next())
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

    void flip() const
    {
        CGContextConcatCTM (context, CGAffineTransformMake (1, 0, 0, -1, 0, flipHeight));
    }

    void applyTransform (const AffineTransform& transform) const
    {
        CGAffineTransform t;
        t.a = transform.mat00;
        t.b = transform.mat10;
        t.c = transform.mat01;
        t.d = transform.mat11;
        t.tx = transform.mat02;
        t.ty = transform.mat12;
        CGContextConcatCTM (context, t);
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CoreGraphicsContext);
};

LowLevelGraphicsContext* CoreGraphicsImage::createLowLevelContext()
{
    return new CoreGraphicsContext (context, height);
}

//==============================================================================
#if USE_COREGRAPHICS_RENDERING && ! DONT_USE_COREIMAGE_LOADER
Image juce_loadWithCoreImage (InputStream& input)
{
    MemoryBlock data;
    input.readIntoMemoryBlock (data, -1);

  #if JUCE_IOS
    JUCE_AUTORELEASEPOOL
    UIImage* image = [UIImage imageWithData: [NSData dataWithBytesNoCopy: data.getData()
                                                                  length: data.getSize()
                                                            freeWhenDone: NO]];

    if (image != nil)
    {
        CGImageRef loadedImage = image.CGImage;

  #else
    CGDataProviderRef provider = CGDataProviderCreateWithData (0, data.getData(), data.getSize(), 0);
    CGImageSourceRef imageSource = CGImageSourceCreateWithDataProvider (provider, 0);
    CGDataProviderRelease (provider);

    if (imageSource != 0)
    {
        CGImageRef loadedImage = CGImageSourceCreateImageAtIndex (imageSource, 0, 0);
        CFRelease (imageSource);
  #endif

        if (loadedImage != 0)
        {
            CGImageAlphaInfo alphaInfo = CGImageGetAlphaInfo (loadedImage);
            const bool hasAlphaChan = (alphaInfo != kCGImageAlphaNone
                                         && alphaInfo != kCGImageAlphaNoneSkipLast
                                         && alphaInfo != kCGImageAlphaNoneSkipFirst);

            Image image (Image::ARGB, // (CoreImage doesn't work with 24-bit images)
                         (int) CGImageGetWidth (loadedImage), (int) CGImageGetHeight (loadedImage),
                         hasAlphaChan, Image::NativeImage);

            CoreGraphicsImage* const cgImage = dynamic_cast<CoreGraphicsImage*> (image.getSharedImage());
            jassert (cgImage != nullptr); // if USE_COREGRAPHICS_RENDERING is set, the CoreGraphicsImage class should have been used.

            CGContextDrawImage (cgImage->context, CGRectMake (0, 0, image.getWidth(), image.getHeight()), loadedImage);
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

    return Image::null;
}
#endif

#endif
