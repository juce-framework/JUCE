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
class CoreGraphicsContext   : public LowLevelGraphicsContext
{
public:
    CoreGraphicsContext (CGContextRef context_, const float flipHeight_)
        : context (context_),
          flipHeight (flipHeight_)
    {
        CGContextRetain (context);
    }

    ~CoreGraphicsContext()
    {
        CGContextRelease (context);
    }

    //==============================================================================
    bool isVectorDevice() const         { return false; }

    void setOrigin (int x, int y)
    {
        CGContextTranslateCTM (context, x, -y);
    }

    bool reduceClipRegion (int x, int y, int w, int h)
    {
        CGContextClipToRect (context, CGRectMake (x, flipHeight - (y + h), w, h));
        return ! isClipEmpty();
    }

    bool reduceClipRegion (const RectangleList& clipRegion)
    {
        const int numRects = clipRegion.getNumRectangles();
        CGRect* const rects = new CGRect [numRects];
        for (int i = 0; i < numRects; ++i)
        {
            const Rectangle& r = clipRegion.getRectangle(i);
            rects[i] = CGRectMake (r.getX(), flipHeight - r.getBottom(), r.getWidth(), r.getHeight());
        }

        CGContextClipToRects (context, rects, numRects);
        delete[] rects;

        return ! isClipEmpty();
    }

    void excludeClipRegion (int x, int y, int w, int h)
    {
        RectangleList r (getClipBounds());
        r.subtract (Rectangle (x, y, w, h));
        reduceClipRegion (r);
    }

    void saveState()
    {
        CGContextSaveGState (context);
    }

    void restoreState()
    {
        CGContextRestoreGState (context);
    }

    bool clipRegionIntersects (int x, int y, int w, int h)
    {
        return getClipBounds().intersects (Rectangle (x, y, w, h));
    }

    const Rectangle getClipBounds() const
    {
        CGRect bounds = CGRectIntegral (CGContextGetClipBoundingBox (context));

        return Rectangle (roundFloatToInt (bounds.origin.x),
                          roundFloatToInt (flipHeight - (bounds.origin.y + bounds.size.height)),
                          roundFloatToInt (bounds.size.width),
                          roundFloatToInt (bounds.size.height));
    }

    bool isClipEmpty() const
    {
        return CGRectIsEmpty (CGContextGetClipBoundingBox (context));
    }

    //==============================================================================
    void fillRectWithColour (int x, int y, int w, int h, const Colour& colour, const bool replaceExistingContents)
    {
        CGContextSetBlendMode (context, replaceExistingContents ? kCGBlendModeCopy : kCGBlendModeNormal);
        CGContextSetAlpha (context, 1.0f);
        setColour (colour);
        CGContextFillRect (context, CGRectMake (x, flipHeight - (y + h), w, h));
    }

    void fillRectWithGradient (int x, int y, int w, int h, const ColourGradient& gradient)
    {
        CGContextSaveGState (context);
        CGContextClipToRect (context, CGRectMake (x, flipHeight - (y + h), w, h));
        flip();
        drawGradient (gradient);
        CGContextRestoreGState (context);
    }

    void fillPathWithColour (const Path& path, const AffineTransform& transform, const Colour& colour, EdgeTable::OversamplingLevel /*quality*/)
    {
        CGContextSetAlpha (context, 1.0f);
        CGContextSaveGState (context);
        flip();
        applyTransform (transform);
        createPath (path);
        setColour (colour);
        CGContextSetBlendMode (context, kCGBlendModeNormal);
        CGContextFillPath (context);
        CGContextRestoreGState (context);
    }

    void fillPathWithGradient (const Path& path, const AffineTransform& transform, const ColourGradient& gradient, EdgeTable::OversamplingLevel quality)
    {
        CGContextSaveGState (context);
        createPath (path, transform);
        CGContextClip (context);
        flip();
        applyTransform (gradient.transform);
        drawGradient (gradient);
        CGContextRestoreGState (context);
    }

    void fillPathWithImage (const Path& path, const AffineTransform& transform,
                            const Image& image, int imageX, int imageY, float alpha, EdgeTable::OversamplingLevel /*quality*/)
    {
        CGContextSaveGState (context);
        createPath (path, transform);
        CGContextClip (context);
        blendImage (image, imageX, imageY, image.getWidth(), image.getHeight(), 0, 0, alpha);
        CGContextRestoreGState (context);
    }

    void fillAlphaChannelWithColour (const Image& alphaImage, int alphaImageX, int alphaImageY, const Colour& colour)
    {
        Image* singleChannelImage = createAlphaChannelImage (alphaImage);
        CGImageRef image = createImage (*singleChannelImage, true);

        CGContextSetAlpha (context, 1.0f);
        CGContextSaveGState (context);
        CGRect r = CGRectMake (alphaImageX, flipHeight - (alphaImageY + alphaImage.getHeight()),
                               alphaImage.getWidth(), alphaImage.getHeight());
        CGContextClipToMask (context, r, image);
        setColour (colour);
        CGContextSetBlendMode (context, kCGBlendModeNormal);
        CGContextFillRect (context, r);
        CGContextRestoreGState (context);

        CGImageRelease (image);
        deleteAlphaChannelImage (alphaImage, singleChannelImage);
    }

    void fillAlphaChannelWithGradient (const Image& alphaImage, int alphaImageX, int alphaImageY, const ColourGradient& gradient)
    {
        Image* singleChannelImage = createAlphaChannelImage (alphaImage);
        CGImageRef image = createImage (*singleChannelImage, true);

        CGContextSaveGState (context);
        CGRect r = CGRectMake (alphaImageX, flipHeight - (alphaImageY + alphaImage.getHeight()),
                               alphaImage.getWidth(), alphaImage.getHeight());
        CGContextClipToMask (context, r, image);
        flip();
        drawGradient (gradient);
        CGContextRestoreGState (context);

        CGImageRelease (image);
        deleteAlphaChannelImage (alphaImage, singleChannelImage);
    }

    void fillAlphaChannelWithImage (const Image& alphaImage, int alphaImageX, int alphaImageY,
                                    const Image& fillerImage, int fillerImageX, int fillerImageY, float alpha)
    {
        Image* singleChannelImage = createAlphaChannelImage (alphaImage);
        CGImageRef image = createImage (*singleChannelImage, true);

        CGContextSaveGState (context);
        CGRect r = CGRectMake (alphaImageX, flipHeight - (alphaImageY + alphaImage.getHeight()),
                               alphaImage.getWidth(), alphaImage.getHeight());
        CGContextClipToMask (context, r, image);

        blendImage (fillerImage, fillerImageX, fillerImageY,
                    fillerImage.getWidth(), fillerImage.getHeight(),
                    0, 0, alpha);

        CGContextRestoreGState (context);

        CGImageRelease (image);
        deleteAlphaChannelImage (alphaImage, singleChannelImage);
    }

    //==============================================================================
    void blendImage (const Image& sourceImage,
                     int destX, int destY, int destW, int destH, int sourceX, int sourceY,
                     float alpha)
    {
        CGContextSetBlendMode (context, kCGBlendModeNormal);
        CGImageRef image = createImage (sourceImage, false);

        CGContextSaveGState (context);
        CGContextClipToRect (context, CGRectMake (destX, flipHeight - (destY + destH), destW, destH));
        CGContextSetAlpha (context, alpha);
        CGContextDrawImage (context, CGRectMake (destX - sourceX,
                                                 flipHeight - ((destY - sourceY) + sourceImage.getHeight()),
                                                 sourceImage.getWidth(),
                                                 sourceImage.getHeight()), image);

        CGContextRestoreGState (context);
        CGImageRelease (image);
    }

    void blendImageWarping (const Image& sourceImage,
                            int srcClipX, int srcClipY, int srcClipW, int srcClipH,
                            const AffineTransform& transform,
                            float alpha, const Graphics::ResamplingQuality quality)
    {
        CGContextSetBlendMode (context, kCGBlendModeNormal);
        CGImageRef fullImage = createImage (sourceImage, false);
        CGImageRef image = CGImageCreateWithImageInRect (fullImage, CGRectMake (srcClipX, sourceImage.getHeight() - (srcClipY + srcClipH),
                                                                                srcClipW, srcClipH));
        CGImageRelease (fullImage);

        CGContextSaveGState (context);
        CGContextSetAlpha (context, alpha);
        flip();
        applyTransform (AffineTransform::scale (1.0f, -1.0f).translated (0, sourceImage.getHeight()).followedBy (transform));

        CGContextSetInterpolationQuality (context, quality == Graphics::lowResamplingQuality
                                                    ? kCGInterpolationLow
                                                    : kCGInterpolationHigh);

        CGContextDrawImage (context, CGRectMake (0, 0, sourceImage.getWidth(),
                                                 sourceImage.getHeight()), image);

        CGImageRelease (image);
        CGContextRestoreGState (context);
    }

    //==============================================================================
    void drawLine (double x1, double y1, double x2, double y2, const Colour& colour)
    {
        CGContextSetAlpha (context, 1.0f);
        CGContextSetRGBStrokeColor (context, colour.getFloatRed(), colour.getFloatGreen(),
                                    colour.getFloatBlue(), colour.getFloatAlpha());
        CGContextSetLineCap (context, kCGLineCapSquare);
        CGContextSetLineWidth (context, 1.0f);

        CGPoint line[] = { { x1 + 0.5f, flipHeight - (y1 + 0.5f) },
                           { x2 + 0.5f, flipHeight - (y2 + 0.5f) } };
        CGContextStrokeLineSegments (context, line, 1);
    }

    void drawVerticalLine (const int x, double top, double bottom, const Colour& colour)
    {
        setColour (colour);
        CGContextSetBlendMode (context, kCGBlendModeNormal);
        CGContextFillRect (context, CGRectMake (x, flipHeight - bottom, 1.0f, bottom - top));
    }

    void drawHorizontalLine (const int y, double left, double right, const Colour& colour)
    {
        setColour (colour);
        CGContextSetBlendMode (context, kCGBlendModeNormal);
        CGContextFillRect (context, CGRectMake (left, y, right - left, 1.0f));
    }

private:
    CGContextRef context;
    const float flipHeight;

    void setColour (const Colour& colour) const throw()
    {
        CGContextSetRGBFillColor (context,
                                  colour.getFloatRed(), colour.getFloatGreen(),
                                  colour.getFloatBlue(), colour.getFloatAlpha());
    }

    static void gradientCallback (void* info, const CGFloat* inData, CGFloat* outData)
    {
        const ColourGradient* const g = (const ColourGradient*) info;
        const Colour c (g->getColourAtPosition (inData[0]));
        outData[0] = c.getFloatRed();
        outData[1] = c.getFloatGreen();
        outData[2] = c.getFloatBlue();
        outData[3] = c.getFloatAlpha();
    }

    CGShadingRef createGradient (const ColourGradient& gradient) const throw()
    {
        CGShadingRef result = 0;
        CGColorSpaceRef colourSpace = CGColorSpaceCreateDeviceRGB();

        CGFunctionCallbacks callbacks = { 0, gradientCallback, 0 };
        CGFunctionRef function = CGFunctionCreate ((void*) &gradient, 1, 0, 4, 0, &callbacks);
        CGPoint p1 = CGPointMake (gradient.x1, gradient.y1);

        if (gradient.isRadial)
        {
            result = CGShadingCreateRadial (colourSpace,
                                            p1, 0,
                                            p1, hypotf (gradient.x1 - gradient.x2, gradient.y1 - gradient.y2),
                                            function, true, true);
        }
        else
        {
            result = CGShadingCreateAxial (colourSpace, p1,
                                           CGPointMake (gradient.x2, gradient.y2),
                                           function, true, true);
        }

        CGColorSpaceRelease (colourSpace);
        CGFunctionRelease (function);
        return result;
    }

    void drawGradient (const ColourGradient& gradient) const throw()
    {
        CGContextSetBlendMode (context, kCGBlendModeNormal);
        CGContextSetAlpha (context, 1.0f);
        CGShadingRef shading = createGradient (gradient);
        CGContextDrawShading (context, shading);
        CGShadingRelease (shading);
    }

    void createPath (const Path& path) const throw()
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

    void createPath (const Path& path, const AffineTransform& transform) const throw()
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

    CGImageRef createImage (const Image& juceImage, const bool forAlpha) const throw()
    {
        int lineStride = 0;
        int pixelStride = 0;
        const uint8* imageData = juceImage.lockPixelDataReadOnly (0, 0, juceImage.getWidth(), juceImage.getHeight(),
                                                                  lineStride, pixelStride);

        CGDataProviderRef provider = CGDataProviderCreateWithData (0, imageData, lineStride * pixelStride, 0);

        CGColorSpaceRef colourSpace = forAlpha ? CGColorSpaceCreateDeviceGray()
                                               : CGColorSpaceCreateDeviceRGB();

        CGImageRef imageRef = CGImageCreate (juceImage.getWidth(), juceImage.getHeight(),
                                  8, pixelStride * 8, lineStride,
                                  colourSpace,
                                  (juceImage.hasAlphaChannel() && ! forAlpha)
                                             ? (kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Little)
                                             : kCGBitmapByteOrderDefault,
                                  provider,
                                  0, true, kCGRenderingIntentDefault);

        CGColorSpaceRelease (colourSpace);
        CGDataProviderRelease (provider);

        juceImage.releasePixelDataReadOnly (imageData);
        return imageRef;
    }

    static Image* createAlphaChannelImage (const Image& im) throw()
    {
        if (im.getFormat() == Image::SingleChannel)
            return const_cast <Image*> (&im);

        return im.createCopyOfAlphaChannel();
    }

    static void deleteAlphaChannelImage (const Image& im, Image* const alphaIm) throw()
    {
        if (im.getFormat() != Image::SingleChannel)
            delete alphaIm;
    }

    void flip() const throw()
    {
        CGContextConcatCTM (context, CGAffineTransformMake (1, 0, 0, -1, 0, flipHeight));
    }

    void applyTransform (const AffineTransform& transform) const throw()
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
};


#endif
