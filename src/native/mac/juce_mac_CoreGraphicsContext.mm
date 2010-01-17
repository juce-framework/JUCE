/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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
class CoreGraphicsImage : public Image
{
public:
    //==============================================================================
    CoreGraphicsImage (const PixelFormat format_,
                       const int imageWidth_,
                       const int imageHeight_,
                       const bool clearImage)
        : Image (format_, imageWidth_, imageHeight_, clearImage)
    {
        CGColorSpaceRef colourSpace = format == Image::SingleChannel ? CGColorSpaceCreateDeviceGray()
                                                                     : CGColorSpaceCreateDeviceRGB();

        context = CGBitmapContextCreate (imageData, imageWidth, imageHeight, 8, lineStride,
                                         colourSpace, getCGImageFlags (*this));

        CGColorSpaceRelease (colourSpace);
    }

    ~CoreGraphicsImage()
    {
        CGContextRelease (context);
    }

    LowLevelGraphicsContext* createLowLevelContext();

    //==============================================================================
    static CGImageRef createImage (const Image& juceImage, const bool forAlpha, CGColorSpaceRef colourSpace)
    {
        const CoreGraphicsImage* nativeImage = dynamic_cast <const CoreGraphicsImage*> (&juceImage);

        if (nativeImage != 0 && (juceImage.getFormat() == Image::SingleChannel || ! forAlpha))
        {
            return CGBitmapContextCreateImage (nativeImage->context);
        }
        else
        {
            const Image::BitmapData srcData (juceImage, 0, 0, juceImage.getWidth(), juceImage.getHeight());

            CGDataProviderRef provider = CGDataProviderCreateWithData (0, srcData.data, srcData.lineStride * srcData.pixelStride, 0);

            CGImageRef imageRef = CGImageCreate (srcData.width, srcData.height,
                                                 8, srcData.pixelStride * 8, srcData.lineStride,
                                                 colourSpace, getCGImageFlags (juceImage), provider,
                                                 0, true, kCGRenderingIntentDefault);

            CGDataProviderRelease (provider);
            return imageRef;
        }
    }

#if JUCE_MAC
    static NSImage* createNSImage (const Image& image)
    {
        const ScopedAutoReleasePool pool;

        NSImage* im = [[NSImage alloc] init];
        [im setSize: NSMakeSize (image.getWidth(), image.getHeight())];
        [im lockFocus];

        CGColorSpaceRef colourSpace = CGColorSpaceCreateDeviceRGB();
        CGImageRef imageRef = createImage (image, false, colourSpace);
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

private:
    static CGBitmapInfo getCGImageFlags (const Image& image)
    {
#if JUCE_BIG_ENDIAN
        return image.getFormat() == Image::ARGB ? (kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Big) : kCGBitmapByteOrderDefault;
#else
        return image.getFormat() == Image::ARGB ? (kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Little) : kCGBitmapByteOrderDefault;
#endif
    }
};

Image* Image::createNativeImage (const PixelFormat format, const int imageWidth, const int imageHeight, const bool clearImage)
{
#if USE_COREGRAPHICS_RENDERING
    return new CoreGraphicsImage (format == RGB ? ARGB : format, imageWidth, imageHeight, clearImage);
#else
    return new Image (format, imageWidth, imageHeight, clearImage);
#endif
}

//==============================================================================
class CoreGraphicsContext   : public LowLevelGraphicsContext
{
public:
    CoreGraphicsContext (CGContextRef context_, const float flipHeight_)
        : context (context_),
          flipHeight (flipHeight_),
          numGradientLookupEntries (0)
    {
        CGContextRetain (context);
        CGContextSaveGState(context);
        CGContextSetShouldSmoothFonts (context, true);
        CGContextSetShouldAntialias (context, true);
        CGContextSetBlendMode (context, kCGBlendModeNormal);
        rgbColourSpace = CGColorSpaceCreateDeviceRGB();
        greyColourSpace = CGColorSpaceCreateDeviceGray();
        gradientCallbacks.version = 0;
        gradientCallbacks.evaluate = gradientCallback;
        gradientCallbacks.releaseInfo = 0;
        state = new SavedState();
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
    }

    bool clipToRectangle (const Rectangle& r)
    {
        CGContextClipToRect (context, CGRectMake (r.getX(), flipHeight - r.getBottom(), r.getWidth(), r.getHeight()));
        return ! isClipEmpty();
    }

    bool clipToRectangleList (const RectangleList& clipRegion)
    {
        if (clipRegion.isEmpty())
        {
            CGContextClipToRect (context, CGRectMake (0, 0, 0, 0));
            return false;
        }
        else
        {
            const int numRects = clipRegion.getNumRectangles();

            HeapBlock <CGRect> rects (numRects);
            for (int i = 0; i < numRects; ++i)
            {
                const Rectangle& r = clipRegion.getRectangle(i);
                rects[i] = CGRectMake (r.getX(), flipHeight - r.getBottom(), r.getWidth(), r.getHeight());
            }

            CGContextClipToRects (context, rects, numRects);
            return ! isClipEmpty();
        }
    }

    void excludeClipRectangle (const Rectangle& r)
    {
        RectangleList remaining (getClipBounds());
        remaining.subtract (r);
        clipToRectangleList (remaining);
    }

    void clipToPath (const Path& path, const AffineTransform& transform)
    {
        createPath (path, transform);
        CGContextClip (context);
    }

    void clipToImageAlpha (const Image& sourceImage, const Rectangle& srcClip, const AffineTransform& transform)
    {
        if (! transform.isSingularity())
        {
            Image* singleChannelImage = createAlphaChannelImage (sourceImage);
            CGImageRef image = CoreGraphicsImage::createImage (*singleChannelImage, true, greyColourSpace);

            flip();
            AffineTransform t (AffineTransform::scale (1.0f, -1.0f).translated (0, sourceImage.getHeight()).followedBy (transform));
            applyTransform (t);

            CGRect r = CGRectMake (0, 0, sourceImage.getWidth(), sourceImage.getHeight());
            CGContextClipToMask (context, r, image);

            applyTransform (t.inverted());
            flip();

            CGImageRelease (image);
            deleteAlphaChannelImage (sourceImage, singleChannelImage);
        }
    }

    bool clipRegionIntersects (const Rectangle& r)
    {
        return getClipBounds().intersects (r);
    }

    const Rectangle getClipBounds() const
    {
        CGRect bounds = CGRectIntegral (CGContextGetClipBoundingBox (context));

        return Rectangle (roundToInt (bounds.origin.x),
                          roundToInt (flipHeight - (bounds.origin.y + bounds.size.height)),
                          roundToInt (bounds.size.width),
                          roundToInt (bounds.size.height));
    }

    bool isClipEmpty() const
    {
        return CGRectIsEmpty (CGContextGetClipBoundingBox (context));
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

        if (top != 0)
        {
            state = top;
            stateStack.removeLast (1, false);
        }
        else
        {
            jassertfalse // trying to pop with an empty stack!
        }
    }

    //==============================================================================
    void setFill (const FillType& fillType)
    {
        state->fillType = fillType;

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
    void fillRect (const Rectangle& r, const bool replaceExistingContents)
    {
        CGRect cgRect = CGRectMake (r.getX(), flipHeight - r.getBottom(), r.getWidth(), r.getHeight());

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

            fillRect (r, false);
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
                drawImage (*(state->fillType.image), state->fillType.image->getBounds(), state->fillType.transform, true);
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
                drawImage (*(state->fillType.image), state->fillType.image->getBounds(), state->fillType.transform, true);
        }

        CGContextRestoreGState (context);
    }

    void drawImage (const Image& sourceImage, const Rectangle& srcClip,
                    const AffineTransform& transform, const bool fillEntireClipAsTiles)
    {
        jassert (sourceImage.getBounds().contains (srcClip));

        CGImageRef fullImage = CoreGraphicsImage::createImage (sourceImage, false, rgbColourSpace);
        CGImageRef image = fullImage;

        if (srcClip != sourceImage.getBounds())
        {
            image = CGImageCreateWithImageInRect (fullImage, CGRectMake (srcClip.getX(), srcClip.getY(),
                                                                         srcClip.getWidth(), srcClip.getHeight()));
            CGImageRelease (fullImage);
        }

        CGContextSaveGState (context);
        CGContextSetAlpha (context, state->fillType.getOpacity());

        flip();
        applyTransform (AffineTransform::scale (1.0f, -1.0f).translated (0, srcClip.getHeight()).followedBy (transform));
        CGRect imageRect = CGRectMake (0, 0, srcClip.getWidth(), srcClip.getHeight());

        if (fillEntireClipAsTiles)
        {
#if JUCE_IPHONE || (MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_5)
            CGContextDrawTiledImage (context, imageRect, image);
#else
  #if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_5
            if (CGContextDrawTiledImage != 0)
                CGContextDrawTiledImage (context, imageRect, image);
            else
  #endif
            {
                // Fallback to manually doing a tiled fill on 10.4
                CGRect clip = CGRectIntegral (CGContextGetClipBoundingBox (context));
                const int iw = srcClip.getWidth();
                const int ih = srcClip.getHeight();

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
    void drawLine (double x1, double y1, double x2, double y2)
    {
        CGContextSetLineCap (context, kCGLineCapSquare);
        CGContextSetLineWidth (context, 1.0f);
        CGContextSetRGBStrokeColor (context,
                                    state->fillType.colour.getFloatRed(), state->fillType.colour.getFloatGreen(),
                                    state->fillType.colour.getFloatBlue(), state->fillType.colour.getFloatAlpha());

        CGPoint line[] = { { (float) x1 + 0.5f, flipHeight - ((float) y1 + 0.5f) },
                           { (float) x2 + 0.5f, flipHeight - ((float) y2 + 0.5f) } };

        CGContextStrokeLineSegments (context, line, 1);
    }

    void drawVerticalLine (const int x, double top, double bottom)
    {
#if MAC_OS_X_VERSION_MIN_REQUIRED > MAC_OS_X_VERSION_10_5
        CGContextFillRect (context, CGRectMake (x, flipHeight - (float) bottom, 1.0f, (float) (bottom - top)));
#else
        // On Leopard, unless both co-ordinates are non-integer, it disables anti-aliasing, so nudge
        // the x co-ord slightly to trick it..
        CGContextFillRect (context, CGRectMake (x + 1.0f / 256.0f, flipHeight - (float) bottom, 1.0f + 1.0f / 256.0f, (float) (bottom - top)));
#endif
    }

    void drawHorizontalLine (const int y, double left, double right)
    {
#if MAC_OS_X_VERSION_MIN_REQUIRED > MAC_OS_X_VERSION_10_5
        CGContextFillRect (context, CGRectMake ((float) left, flipHeight - (y + 1.0f), (float) (right - left), 1.0f));
#else
        // On Leopard, unless both co-ordinates are non-integer, it disables anti-aliasing, so nudge
        // the x co-ord slightly to trick it..
        CGContextFillRect (context, CGRectMake ((float) left, flipHeight - (y + (1.0f + 1.0f / 256.0f)), (float) (right - left), 1.0f + 1.0f / 256.0f));
#endif
    }

    void setFont (const Font& newFont)
    {
        if (state->font != newFont)
        {
            state->fontRef = 0;
            state->font = newFont;

            MacTypeface* mf = dynamic_cast <MacTypeface*> ((Typeface*) state->font.getTypeface());

            if (mf != 0)
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

    const Font getFont()
    {
        return state->font;
    }

    void drawGlyph (int glyphNumber, const AffineTransform& transform)
    {
        if (state->fontRef != 0 && state->fillType.isColour())
        {
            if (transform.isOnlyTranslation())
            {
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

                CGContextSetTextMatrix (context, state->fontTransform);
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
    const float flipHeight;
    CGColorSpaceRef rgbColourSpace, greyColourSpace;
    CGFunctionCallbacks gradientCallbacks;

    struct SavedState
    {
        SavedState()
            : font (1.0f), fontRef (0), fontTransform (CGAffineTransformIdentity)
        {
        }

        SavedState (const SavedState& other)
            : fillType (other.fillType), font (other.font), fontRef (other.fontRef),
              fontTransform (other.fontTransform)
        {
        }

        ~SavedState()
        {
        }

        FillType fillType;
        Font font;
        CGFontRef fontRef;
        CGAffineTransform fontTransform;
    };

    ScopedPointer <SavedState> state;
    OwnedArray <SavedState> stateStack;
    HeapBlock <PixelARGB> gradientLookupTable;
    int numGradientLookupEntries;

    static void gradientCallback (void* info, const CGFloat* inData, CGFloat* outData)
    {
        const CoreGraphicsContext* const g = (const CoreGraphicsContext*) info;

        const int index = roundToInt (g->numGradientLookupEntries * inData[0]);
        PixelARGB colour (g->gradientLookupTable [jlimit (0, g->numGradientLookupEntries, index)]);
        colour.unpremultiply();

        outData[0] = colour.getRed() / 255.0f;
        outData[1] = colour.getGreen() / 255.0f;
        outData[2] = colour.getBlue() / 255.0f;
        outData[3] = colour.getAlpha() / 255.0f;
    }

    CGShadingRef createGradient (const AffineTransform& transform, ColourGradient gradient)
    {
        numGradientLookupEntries = gradient.createLookupTable (transform, gradientLookupTable);
        --numGradientLookupEntries;

        CGShadingRef result = 0;
        CGFunctionRef function = CGFunctionCreate ((void*) this, 1, 0, 4, 0, &gradientCallbacks);
        CGPoint p1 (CGPointMake (gradient.x1, gradient.y1));

        if (gradient.isRadial)
        {
            result = CGShadingCreateRadial (rgbColourSpace, p1, 0,
                                            p1, hypotf (gradient.x1 - gradient.x2, gradient.y1 - gradient.y2),
                                            function, true, true);
        }
        else
        {
            result = CGShadingCreateAxial (rgbColourSpace, p1,
                                           CGPointMake (gradient.x2, gradient.y2),
                                           function, true, true);
        }

        CGFunctionRelease (function);
        return result;
    }

    void drawGradient()
    {
        flip();
        applyTransform (state->fillType.transform);

        CGContextSetInterpolationQuality (context, kCGInterpolationDefault); // (This is required for 10.4, where there's a crash if
                                                                             // you draw a gradient with high quality interp enabled).
        CGShadingRef shading = createGradient (state->fillType.transform, *(state->fillType.gradient));
        CGContextSetAlpha (context, state->fillType.getOpacity());
        CGContextDrawShading (context, shading);
        CGShadingRelease (shading);
    }

    void createPath (const Path& path) const
    {
        CGContextBeginPath (context);
        Path::Iterator i (path);

        while (i.next())
        {
            switch (i.elementType)
            {
            case Path::Iterator::startNewSubPath:
                CGContextMoveToPoint (context, i.x1, i.y1);
                break;
            case Path::Iterator::lineTo:
                CGContextAddLineToPoint (context, i.x1, i.y1);
                break;
            case Path::Iterator::quadraticTo:
                CGContextAddQuadCurveToPoint (context, i.x1, i.y1, i.x2, i.y2);
                break;
            case Path::Iterator::cubicTo:
                CGContextAddCurveToPoint (context, i.x1, i.y1, i.x2, i.y2, i.x3, i.y3);
                break;
            case Path::Iterator::closePath:
                CGContextClosePath (context); break;
            default:
                jassertfalse
                break;
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
                transform.transformPoint (i.x1, i.y1);
                transform.transformPoint (i.x2, i.y2);
                CGContextAddQuadCurveToPoint (context, i.x1, flipHeight - i.y1, i.x2, flipHeight - i.y2);
                break;
            case Path::Iterator::cubicTo:
                transform.transformPoint (i.x1, i.y1);
                transform.transformPoint (i.x2, i.y2);
                transform.transformPoint (i.x3, i.y3);
                CGContextAddCurveToPoint (context, i.x1, flipHeight - i.y1, i.x2, flipHeight - i.y2, i.x3, flipHeight - i.y3);
                break;
            case Path::Iterator::closePath:
                CGContextClosePath (context); break;
            default:
                jassertfalse
                break;
            }
        }
    }

    static Image* createAlphaChannelImage (const Image& im)
    {
        if (im.getFormat() == Image::SingleChannel)
            return const_cast <Image*> (&im);

        return im.createCopyOfAlphaChannel();
    }

    static void deleteAlphaChannelImage (const Image& im, Image* const alphaIm)
    {
        if (im.getFormat() != Image::SingleChannel)
            delete alphaIm;
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

    float flipY (float y) const
    {
        return flipHeight - y;
    }
};

LowLevelGraphicsContext* CoreGraphicsImage::createLowLevelContext()
{
    return new CoreGraphicsContext (context, imageHeight);
}

#endif
