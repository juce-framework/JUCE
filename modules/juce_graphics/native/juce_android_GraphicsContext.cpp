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

namespace GraphicsHelpers
{
    jobject createPaint (Graphics::ResamplingQuality quality)
    {
        jint constructorFlags = 1 /*ANTI_ALIAS_FLAG*/
                                | 4 /*DITHER_FLAG*/
                                | 128 /*SUBPIXEL_TEXT_FLAG*/;

        if (quality > Graphics::lowResamplingQuality)
            constructorFlags |= 2; /*FILTER_BITMAP_FLAG*/

        return getEnv()->NewObject (Paint, Paint.constructor, constructorFlags);
    }

    const jobject createMatrix (JNIEnv* env, const AffineTransform& t)
    {
        jobject m = env->NewObject (Matrix, Matrix.constructor);

        jfloat values[9] = { t.mat00, t.mat01, t.mat02,
                             t.mat10, t.mat11, t.mat12,
                             0.0f, 0.0f, 1.0f };

        jfloatArray javaArray = env->NewFloatArray (9);
        env->SetFloatArrayRegion (javaArray, 0, 9, values);

        env->CallVoidMethod (m, Matrix.setValues, javaArray);
        env->DeleteLocalRef (javaArray);

        return m;
    }
}

#if USE_ANDROID_CANVAS

//==============================================================================
#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
 METHOD (constructor,    "<init>",          "(Landroid/graphics/Bitmap;)V") \
 METHOD (drawRect,       "drawRect",        "(FFFFLandroid/graphics/Paint;)V") \
 METHOD (translate,      "translate",       "(FF)V") \
 METHOD (clipPath,       "clipPath",        "(Landroid/graphics/Path;)Z") \
 METHOD (clipRect,       "clipRect",        "(FFFF)Z") \
 METHOD (clipRegion,     "clipRegion",      "(Landroid/graphics/Region;)Z") \
 METHOD (concat,         "concat",          "(Landroid/graphics/Matrix;)V") \
 METHOD (drawBitmap,     "drawBitmap",      "(Landroid/graphics/Bitmap;Landroid/graphics/Matrix;Landroid/graphics/Paint;)V") \
 METHOD (drawBitmapAt,   "drawBitmap",      "(Landroid/graphics/Bitmap;FFLandroid/graphics/Paint;)V") \
 METHOD (drawMemoryBitmap, "drawBitmap",    "([IIIFFIIZLandroid/graphics/Paint;)V") \
 METHOD (drawLine,       "drawLine",        "(FFFFLandroid/graphics/Paint;)V") \
 METHOD (drawPath,       "drawPath",        "(Landroid/graphics/Path;Landroid/graphics/Paint;)V") \
 METHOD (drawText,       "drawText",        "(Ljava/lang/String;FFLandroid/graphics/Paint;)V") \
 METHOD (getClipBounds,  "getClipBounds",   "(Landroid/graphics/Rect;)Z") \
 METHOD (getClipBounds2, "getClipBounds",   "()Landroid/graphics/Rect;") \
 METHOD (getMatrix,      "getMatrix",       "()Landroid/graphics/Matrix;") \
 METHOD (save,           "save",            "()I") \
 METHOD (restore,        "restore",         "()V") \
 METHOD (saveLayerAlpha, "saveLayerAlpha",  "(FFFFII)I")

DECLARE_JNI_CLASS (Canvas, "android/graphics/Canvas");
#undef JNI_CLASS_MEMBERS

//==============================================================================
#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
 METHOD (constructor,   "<init>",        "()V") \
 METHOD (moveTo,        "moveTo",        "(FF)V") \
 METHOD (lineTo,        "lineTo",        "(FF)V") \
 METHOD (quadTo,        "quadTo",        "(FFFF)V") \
 METHOD (cubicTo,       "cubicTo",       "(FFFFFF)V") \
 METHOD (closePath,     "close",         "()V") \
 METHOD (computeBounds, "computeBounds", "(Landroid/graphics/RectF;Z)V") \

DECLARE_JNI_CLASS (PathClass, "android/graphics/Path");
#undef JNI_CLASS_MEMBERS

//==============================================================================
#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
 METHOD (constructor, "<init>", "()V"); \
 METHOD (regionUnion, "union",  "(Landroid/graphics/Rect;)Z"); \

DECLARE_JNI_CLASS (RegionClass, "android/graphics/Region");
#undef JNI_CLASS_MEMBERS

//==============================================================================
#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
 STATICMETHOD (createBitmap, "createBitmap", "(IILandroid/graphics/Bitmap$Config;)Landroid/graphics/Bitmap;") \
 METHOD (bitmapCopy, "copy",      "(Landroid/graphics/Bitmap$Config;Z)Landroid/graphics/Bitmap;") \
 METHOD (getPixels, "getPixels",  "([IIIIIII)V") \
 METHOD (setPixels, "setPixels",  "([IIIIIII)V") \
 METHOD (recycle, "recycle",      "()V") \

DECLARE_JNI_CLASS (BitmapClass, "android/graphics/Bitmap");
#undef JNI_CLASS_MEMBERS

//==============================================================================
#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
 STATICFIELD (ARGB_8888, "ARGB_8888", "Landroid/graphics/Bitmap$Config;") \
 STATICFIELD (ALPHA_8,   "ALPHA_8",   "Landroid/graphics/Bitmap$Config;") \

DECLARE_JNI_CLASS (BitmapConfig, "android/graphics/Bitmap$Config");
#undef JNI_CLASS_MEMBERS

//==============================================================================
#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
 METHOD (constructor, "<init>", "(Landroid/graphics/Bitmap;Landroid/graphics/Shader$TileMode;Landroid/graphics/Shader$TileMode;)V")

DECLARE_JNI_CLASS (BitmapShader, "android/graphics/BitmapShader");
#undef JNI_CLASS_MEMBERS

//==============================================================================
#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
 METHOD (setLocalMatrix, "setLocalMatrix", "(Landroid/graphics/Matrix;)V")

DECLARE_JNI_CLASS (ShaderClass, "android/graphics/Shader");
#undef JNI_CLASS_MEMBERS

//==============================================================================
#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
 STATICFIELD (CLAMP, "CLAMP", "Landroid/graphics/Shader$TileMode;")

DECLARE_JNI_CLASS (ShaderTileMode, "android/graphics/Shader$TileMode");
#undef JNI_CLASS_MEMBERS

//==============================================================================
#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
 METHOD (constructor, "<init>", "(FFFF[I[FLandroid/graphics/Shader$TileMode;)V") \

DECLARE_JNI_CLASS (LinearGradientClass, "android/graphics/LinearGradient");
#undef JNI_CLASS_MEMBERS

//==============================================================================
#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
 METHOD (constructor, "<init>", "(FFF[I[FLandroid/graphics/Shader$TileMode;)V") \

DECLARE_JNI_CLASS (RadialGradientClass, "android/graphics/RadialGradient");
#undef JNI_CLASS_MEMBERS

//==============================================================================
class AndroidImage  : public ImagePixelData
{
public:
    AndroidImage (const int width_, const int height_, const bool clearImage)
        : ImagePixelData (Image::ARGB, width_, height_),
          bitmap (createBitmap (width_, height_, false))
    {
    }

    AndroidImage (const int width_, const int height_, const GlobalRef& bitmap_)
        : ImagePixelData (Image::ARGB, width_, height_),
          bitmap (bitmap_)
    {
    }

    ~AndroidImage()
    {
        if (bitmap != 0)
            bitmap.callVoidMethod (BitmapClass.recycle);
    }

    LowLevelGraphicsContext* createLowLevelContext();

    void initialiseBitmapData (Image::BitmapData& bm, int x, int y, Image::BitmapData::ReadWriteMode mode)
    {
        bm.lineStride = width * sizeof (jint);
        bm.pixelStride = sizeof (jint);
        bm.pixelFormat = Image::ARGB;
        bm.dataReleaser = new CopyHandler (*this, bm, x, y, mode);
    }

    ImagePixelData* clone()
    {
        JNIEnv* env = getEnv();
        jobject mode = env->GetStaticObjectField (BitmapConfig, BitmapConfig.ARGB_8888);
        GlobalRef newCopy (bitmap.callObjectMethod (BitmapClass.bitmapCopy, mode, true));
        env->DeleteLocalRef (mode);

        return new AndroidImage (width, height, newCopy);
    }

    ImageType* createType() const    { return new NativeImageType(); }

    static jobject createBitmap (int width, int height, bool asSingleChannel)
    {
        JNIEnv* env = getEnv();
        jobject mode = env->GetStaticObjectField (BitmapConfig, asSingleChannel ? BitmapConfig.ALPHA_8
                                                                                : BitmapConfig.ARGB_8888);
        jobject result = env->CallStaticObjectMethod (BitmapClass, BitmapClass.createBitmap, width, height, mode);
        env->DeleteLocalRef (mode);
        return result;
    }

    //==============================================================================
    GlobalRef bitmap;

private:
    class CopyHandler  : public Image::BitmapData::BitmapDataReleaser
    {
    public:
        CopyHandler (AndroidImage& owner_, Image::BitmapData& bitmapData_,
                     const int x_, const int y_, const Image::BitmapData::ReadWriteMode mode_)
            : owner (owner_), bitmapData (bitmapData_), mode (mode_), x (x_), y (y_)
        {
            JNIEnv* env = getEnv();

            intArray = env->NewIntArray (bitmapData.width * bitmapData.height);

            if (mode != Image::BitmapData::writeOnly)
                owner_.bitmap.callVoidMethod (BitmapClass.getPixels, intArray, 0, bitmapData.width, x_, y_,
                                              bitmapData.width, bitmapData.height);

            bitmapData.data = (uint8*) env->GetIntArrayElements (intArray, 0);

            if (mode != Image::BitmapData::writeOnly)
            {
                for (int yy = 0; yy < bitmapData.height; ++yy)
                {
                    PixelARGB* p = (PixelARGB*) bitmapData.getLinePointer (yy);

                    for (int xx = 0; xx < bitmapData.width; ++xx)
                        p[xx].premultiply();
                }
            }
        }

        ~CopyHandler()
        {
            JNIEnv* env = getEnv();

            if (mode != Image::BitmapData::readOnly)
            {
                for (int yy = 0; yy < bitmapData.height; ++yy)
                {
                    PixelARGB* p = (PixelARGB*) bitmapData.getLinePointer (yy);

                    for (int xx = 0; xx < bitmapData.width; ++xx)
                        p[xx].unpremultiply();
                }
            }

            env->ReleaseIntArrayElements (intArray, (jint*) bitmapData.data, 0);

            if (mode != Image::BitmapData::readOnly)
                owner.bitmap.callVoidMethod (BitmapClass.setPixels, intArray, 0, bitmapData.width, x, y,
                                             bitmapData.width, bitmapData.height);

            env->DeleteLocalRef (intArray);
        }

    private:
        AndroidImage& owner;
        Image::BitmapData& bitmapData;
        jintArray intArray;
        const Image::BitmapData::ReadWriteMode mode;
        const int x, y;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CopyHandler);
    };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AndroidImage);
};
#endif

ImagePixelData* NativeImageType::create (Image::PixelFormat format, int width, int height, bool clearImage) const
{
   #if USE_ANDROID_CANVAS
    if (pixelFormat != Image::SingleChannel)
        return new AndroidImage (width, height, clearImage);
   #endif

    return SoftwareImageType().create (format, width, height, clearImage);
}

#if USE_ANDROID_CANVAS
//==============================================================================
class AndroidLowLevelGraphicsContext   : public LowLevelGraphicsContext
{
public:
    AndroidLowLevelGraphicsContext (jobject canvas_)
        : originalCanvas (canvas_),
          currentState (new SavedState (canvas_))
    {
        setFill (Colours::black);
    }

    ~AndroidLowLevelGraphicsContext()
    {
        while (stateStack.size() > 0)
            restoreState();

        currentState->flattenImageClippingLayer (originalCanvas);
    }

    bool isVectorDevice() const { return false; }

    //==============================================================================
    void setOrigin (int x, int y)
    {
        getCanvas().callVoidMethod (Canvas.translate, (float) x, (float) y);
    }

    void addTransform (const AffineTransform& transform)
    {
        getCanvas().callVoidMethod (Canvas.concat, createMatrixRef (getEnv(), transform).get());
    }

    float getScaleFactor()
    {
        return 1.0f;
    }

    bool clipToRectangle (const Rectangle<int>& r)
    {
        return getCanvas().callBooleanMethod (Canvas.clipRect, (float) r.getX(), (float) r.getY(), (float) r.getRight(), (float) r.getBottom());
    }

    bool clipToRectangleList (const RectangleList& clipRegion)
    {
        RectangleList excluded (getClipBounds());
        excluded.subtract (clipRegion);

        const int numRects = excluded.getNumRectangles();

        for (int i = 0; i < numRects; ++i)
            excludeClipRectangle (excluded.getRectangle(i));
    }

    void excludeClipRectangle (const Rectangle<int>& r)
    {
        android.activity.callVoidMethod (JuceAppActivity.excludeClipRegion, getCanvas().get(),
                                         (float) r.getX(), (float) r.getY(), (float) r.getRight(), (float) r.getBottom());
    }

    void clipToPath (const Path& path, const AffineTransform& transform)
    {
        (void) getCanvas().callBooleanMethod (Canvas.clipPath, createPath (getEnv(), path, transform).get());
    }

    void clipToImageAlpha (const Image& sourceImage, const AffineTransform& transform)
    {
        // XXX couldn't get image clipping to work...
        JNIEnv* env = getEnv();

        {
            Path p;
            p.addRectangle (sourceImage.getBounds().toFloat());
            clipToPath (p, transform);
        }

        Rectangle<int> bounds (getClipBounds());

        jobject temporaryLayerBitmap = AndroidImage::createBitmap (bounds.getWidth(), bounds.getHeight(), false);
        jobject temporaryCanvas = env->NewObject (Canvas, Canvas.constructor, temporaryLayerBitmap);

        setFill (Colours::red);
        env->CallVoidMethod (temporaryCanvas, Canvas.drawRect,
                             (jfloat) 20, (jfloat) 20, (jfloat) 300, (jfloat) 200,
                                    getCurrentPaint());

        env->CallVoidMethod (temporaryCanvas, Canvas.translate,
                             (jfloat) -bounds.getX(), (jfloat) -bounds.getY());

        Image maskImage (Image::SingleChannel, bounds.getWidth(), bounds.getHeight(), true);

        {
            Graphics g (maskImage);
            g.setOrigin (-bounds.getWidth(), -bounds.getHeight());
            g.drawImageTransformed (sourceImage, transform);
        }

        SavedState* const top = stateStack.getLast();
        currentState->clipToImage (top != nullptr ? top->canvas.get() : originalCanvas,
                                   temporaryCanvas, temporaryLayerBitmap, maskImage,
                                   bounds.getX(), bounds.getY());
    }

    bool clipRegionIntersects (const Rectangle<int>& r)
    {
        return getClipBounds().intersects (r);
    }

    Rectangle<int> getClipBounds() const
    {
        JNIEnv* env = getEnv();
        jobject rect = getCanvas().callObjectMethod (Canvas.getClipBounds2);

        const int left   = env->GetIntField (rect, RectClass.left);
        const int top    = env->GetIntField (rect, RectClass.top);
        const int right  = env->GetIntField (rect, RectClass.right);
        const int bottom = env->GetIntField (rect, RectClass.bottom);
        env->DeleteLocalRef (rect);

        return Rectangle<int> (left, top, right - left, bottom - top);
    }

    bool isClipEmpty() const
    {
        LocalRef<jobject> tempRect (getEnv()->NewObject (RectClass, RectClass.constructor, 0, 0, 0, 0));
        return ! getCanvas().callBooleanMethod (Canvas.getClipBounds, tempRect.get());
    }

    //==============================================================================
    void setFill (const FillType& fillType)
    {
        currentState->setFillType (fillType);
    }

    void setOpacity (float newOpacity)
    {
        currentState->setAlpha (newOpacity);
    }

    void setInterpolationQuality (Graphics::ResamplingQuality quality)
    {
        currentState->setInterpolationQuality (quality);
    }

    //==============================================================================
    void fillRect (const Rectangle<int>& r, bool replaceExistingContents)
    {
        getCanvas().callVoidMethod (Canvas.drawRect,
                                    (float) r.getX(), (float) r.getY(), (float) r.getRight(), (float) r.getBottom(),
                                    getCurrentPaint());
    }

    void fillPath (const Path& path, const AffineTransform& transform)
    {
        getCanvas().callVoidMethod (Canvas.drawPath, createPath (getEnv(), path, transform).get(),
                                    getCurrentPaint());
    }

    void drawImage (const Image& sourceImage, const AffineTransform& transform)
    {
        AndroidImage* androidImage = dynamic_cast <AndroidImage*> (sourceImage.getPixelData());

        if (androidImage != 0)
        {
            JNIEnv* env = getEnv();
            getCanvas().callVoidMethod (Canvas.drawBitmap, androidImage->bitmap.get(),
                                        createMatrixRef (env, transform).get(), getImagePaint());
        }
        else
        {
            if (transform.isOnlyTranslation())
            {
                JNIEnv* env = getEnv();

                Image::BitmapData bm (sourceImage, Image::BitmapData::readOnly);

                jintArray imageData = env->NewIntArray (bm.width * bm.height);
                jint* dest = env->GetIntArrayElements (imageData, 0);

                if (dest != 0)
                {
                    const uint8* srcLine = bm.getLinePointer (0);
                    jint* dstLine = dest;

                    for (int y = 0; y < bm.height; ++y)
                    {
                        switch (bm.pixelFormat)
                        {
                            case Image::ARGB:           copyPixels (dstLine, (PixelARGB*) srcLine, bm.width, bm.pixelStride); break;
                            case Image::RGB:            copyPixels (dstLine, (PixelRGB*) srcLine, bm.width, bm.pixelStride); break;
                            case Image::SingleChannel:  copyPixels (dstLine, (PixelAlpha*) srcLine, bm.width, bm.pixelStride); break;
                            default:                    jassertfalse; break;
                        }

                        srcLine += bm.lineStride;
                        dstLine += bm.width;
                    }

                    env->ReleaseIntArrayElements (imageData, dest, 0);

                    getCanvas().callVoidMethod (Canvas.drawMemoryBitmap, imageData, 0, bm.width,
                                                transform.getTranslationX(), transform.getTranslationY(),
                                                bm.width, bm.height, true, getImagePaint());
                    env->DeleteLocalRef (imageData);
                }
            }
            else
            {
                saveState();
                addTransform (transform);
                drawImage (sourceImage, AffineTransform::identity);
                restoreState();
            }
        }
    }

    void drawLine (const Line <float>& line)
    {
        getCanvas().callVoidMethod (Canvas.drawLine, line.getStartX(), line.getStartY(),
                                    line.getEndX(), line.getEndY(), getCurrentPaint());
    }

    void drawVerticalLine (int x, float top, float bottom)
    {
        getCanvas().callVoidMethod (Canvas.drawRect, (float) x, top, x + 1.0f, bottom, getCurrentPaint());
    }

    void drawHorizontalLine (int y, float left, float right)
    {
        getCanvas().callVoidMethod (Canvas.drawRect, left, (float) y, right, y + 1.0f, getCurrentPaint());
    }

    void setFont (const Font& newFont)
    {
        if (currentState->font != newFont)
        {
            currentState->font = newFont;
            currentState->typefaceNeedsUpdate = true;
        }
    }

    const Font& getFont()
    {
        return currentState->font;
    }

    void drawGlyph (int glyphNumber, const AffineTransform& transform)
    {
        if (transform.isOnlyTranslation())
        {
            getCanvas().callVoidMethod (Canvas.drawText, javaStringFromChar ((juce_wchar) glyphNumber).get(),
                                        transform.getTranslationX(), transform.getTranslationY(),
                                        currentState->getPaintForTypeface());
        }
        else
        {
            saveState();
            addTransform (transform);
            drawGlyph (glyphNumber, AffineTransform::identity);
            restoreState();
        }
    }

    //==============================================================================
    void saveState()
    {
        (void) getCanvas().callIntMethod (Canvas.save);
        stateStack.add (new SavedState (*currentState));
    }

    void restoreState()
    {
        SavedState* const top = stateStack.getLast();

        if (top != 0)
        {
            currentState->flattenImageClippingLayer (top->canvas);

            currentState = top;
            stateStack.removeLast (1, false);
        }
        else
        {
            jassertfalse; // trying to pop with an empty stack!
        }

        getCanvas().callVoidMethod (Canvas.restore);
    }

    void beginTransparencyLayer (float opacity)
    {
        Rectangle<int> clip (getClipBounds());

        (void) getCanvas().callIntMethod (Canvas.saveLayerAlpha,
                                          (float) clip.getX(),
                                          (float) clip.getY(),
                                          (float) clip.getRight(),
                                          (float) clip.getBottom(),
                                          jlimit (0, 255, roundToInt (opacity * 255.0f)),
                                          31 /*ALL_SAVE_FLAG*/);

        stateStack.add (new SavedState (*currentState));
    }

    void endTransparencyLayer()
    {
        restoreState();
    }

    //==============================================================================
    class SavedState
    {
    public:
        SavedState (jobject canvas_)
            : canvas (canvas_), font (1.0f), quality (Graphics::highResamplingQuality),
              fillNeedsUpdate (true), typefaceNeedsUpdate (true)
        {
        }

        SavedState (const SavedState& other)
            : canvas (other.canvas), fillType (other.fillType), font (other.font),
              quality (other.quality), fillNeedsUpdate (true), typefaceNeedsUpdate (true)
        {
        }

        void setFillType (const FillType& newType)
        {
            fillNeedsUpdate = true;
            fillType = newType;
        }

        void setAlpha (float alpha)
        {
            fillNeedsUpdate = true;
            fillType.colour = fillType.colour.withAlpha (alpha);
        }

        void setInterpolationQuality (Graphics::ResamplingQuality quality_)
        {
            if (quality != quality_)
            {
                quality = quality_;
                fillNeedsUpdate = true;
                paint.clear();
            }
        }

        jobject getPaint()
        {
            if (fillNeedsUpdate)
            {
                JNIEnv* env = getEnv();

                if (paint.get() == 0)
                    paint = GlobalRef (GraphicsHelpers::createPaint (quality));

                if (fillType.isColour())
                {
                    env->DeleteLocalRef (paint.callObjectMethod (Paint.setShader, (jobject) 0));
                    paint.callVoidMethod (Paint.setColor, colourToInt (fillType.colour));
                }
                else if (fillType.isGradient())
                {
                    const ColourGradient& g = *fillType.gradient;
                    const Point<float> p1 (g.point1);
                    const Point<float> p2 (g.point2);

                    const int numColours = g.getNumColours();
                    jintArray coloursArray = env->NewIntArray (numColours);
                    jfloatArray positionsArray = env->NewFloatArray (numColours);

                    {
                        HeapBlock<int> colours (numColours);
                        HeapBlock<float> positions (numColours);

                        for (int i = 0; i < numColours; ++i)
                        {
                            colours[i] = colourToInt (g.getColour (i));
                            positions[i] = (float) g.getColourPosition(i);
                        }

                        env->SetIntArrayRegion (coloursArray, 0, numColours, colours.getData());
                        env->SetFloatArrayRegion (positionsArray, 0, numColours, positions.getData());
                    }

                    jobject tileMode = env->GetStaticObjectField (ShaderTileMode, ShaderTileMode.CLAMP);

                    jobject shader;
                    if (fillType.gradient->isRadial)
                    {
                        shader = env->NewObject (RadialGradientClass,
                                                 RadialGradientClass.constructor,
                                                 p1.getX(), p1.getY(),
                                                 p1.getDistanceFrom (p2),
                                                 coloursArray, positionsArray,
                                                 tileMode);
                    }
                    else
                    {
                        shader = env->NewObject (LinearGradientClass,
                                                 LinearGradientClass.constructor,
                                                 p1.getX(), p1.getY(), p2.getX(), p2.getY(),
                                                 coloursArray, positionsArray,
                                                 tileMode);
                    }

                    env->DeleteLocalRef (tileMode);
                    env->DeleteLocalRef (coloursArray);
                    env->DeleteLocalRef (positionsArray);

                    env->CallVoidMethod (shader, ShaderClass.setLocalMatrix, createMatrixRef (env, fillType.transform).get());
                    env->DeleteLocalRef (paint.callObjectMethod (Paint.setShader, shader));

                    env->DeleteLocalRef (shader);
                }
                else
                {
                    // TODO xxx
                }
            }

            return paint.get();
        }

        jobject getPaintForTypeface()
        {
            jobject p = getPaint();

            if (typefaceNeedsUpdate)
            {
                typefaceNeedsUpdate = false;
                const Typeface::Ptr t (font.getTypeface());
                AndroidTypeface* atf = dynamic_cast <AndroidTypeface*> (t.getObject());

                if (atf != 0)
                {
                    paint.callObjectMethod (Paint.setTypeface, atf->typeface.get());
                    paint.callVoidMethod (Paint.setTextSize, font.getHeight());

                    const float hScale = font.getHorizontalScale();

                    if (hScale < 0.99f || hScale > 1.01f)
                        paint.callVoidMethod (Paint.setTextScaleX, hScale);
                }

                fillNeedsUpdate = true;
                paint.callVoidMethod (Paint.setAlpha, (jint) fillType.colour.getAlpha());
            }

            return p;
        }

        jobject getImagePaint()
        {
            jobject p = getPaint();
            paint.callVoidMethod (Paint.setAlpha, (jint) fillType.colour.getAlpha());
            fillNeedsUpdate = true;
            return p;
        }

        void flattenImageClippingLayer (jobject previousCanvas)
        {
            // XXX couldn't get image clipping to work...

            if (temporaryLayerBitmap != 0)
            {
                JNIEnv* env = getEnv();

                jobject tileMode = env->GetStaticObjectField (ShaderTileMode, ShaderTileMode.CLAMP);
                jobject shader = env->NewObject (BitmapShader, BitmapShader.constructor,
                                                 temporaryLayerBitmap.get(), tileMode, tileMode);
                env->DeleteLocalRef (tileMode);

                jobject compositingPaint = GraphicsHelpers::createPaint (quality);
                env->CallObjectMethod (compositingPaint, Paint.setShader, shader);
                env->DeleteLocalRef (shader);

                LocalRef<jobject> maskBitmap (createAlphaBitmap (env, maskImage));
                maskImage = Image::null;

                env->CallVoidMethod (previousCanvas, Canvas.drawBitmapAt,
                                     maskBitmap.get(), (jfloat) maskLayerX, (jfloat) maskLayerY, compositingPaint);

                env->DeleteLocalRef (compositingPaint);

                canvas = GlobalRef (previousCanvas);

                env->CallVoidMethod (temporaryLayerBitmap.get(), BitmapClass.recycle);
                env->CallVoidMethod (maskBitmap.get(), BitmapClass.recycle);

                temporaryLayerBitmap.clear();
            }
        }

        void clipToImage (jobject previousCanvas,
                          jobject temporaryCanvas, jobject temporaryLayerBitmap_,
                          const Image& maskImage_,
                          int maskLayerX_, int maskLayerY_)
        {
            // XXX couldn't get image clipping to work...
            flattenImageClippingLayer (previousCanvas);

            maskLayerX = maskLayerX_;
            maskLayerY = maskLayerY_;
            canvas = GlobalRef (temporaryCanvas);
            temporaryLayerBitmap = GlobalRef (temporaryLayerBitmap_);
            maskImage = maskImage_;
        }

        static jobject createAlphaBitmap (JNIEnv* env, const Image& image)
        {
            Image::BitmapData bm (image, Image::BitmapData::readOnly);

            jobject bitmap = AndroidImage::createBitmap (bm.width, bm.height, true);

            jintArray intArray = env->NewIntArray (bm.width * bm.height);
            jint* const dest = env->GetIntArrayElements (intArray, 0);

            for (int yy = 0; yy < bm.height; ++yy)
            {
                PixelAlpha* src = (PixelAlpha*) bm.getLinePointer (yy);
                jint* destLine = dest + yy * bm.width;

                for (int xx = 0; xx < bm.width; ++xx)
                {
                    destLine[xx] = src->getAlpha();
                    src = addBytesToPointer (src, bm.pixelStride);
                }
            }

            env->ReleaseIntArrayElements (intArray, (jint*) dest, 0);
            env->CallVoidMethod (bitmap, BitmapClass.setPixels, intArray, 0, bm.width, 0, 0, bm.width, bm.height);
            env->DeleteLocalRef (intArray);
            return bitmap;
        }

        GlobalRef canvas, temporaryLayerBitmap;
        FillType fillType;
        Font font;
        GlobalRef paint;
        bool fillNeedsUpdate, typefaceNeedsUpdate;
        Graphics::ResamplingQuality quality;
        Image maskImage;
        int maskLayerX, maskLayerY;
    };

private:
    //==============================================================================
    GlobalRef originalCanvas;

    ScopedPointer <SavedState> currentState;
    OwnedArray <SavedState> stateStack;

    GlobalRef& getCanvas() const noexcept     { return currentState->canvas; }

    jobject getCurrentPaint() const     { return currentState->getPaint(); }
    jobject getImagePaint() const       { return currentState->getImagePaint(); }

    static LocalRef<jobject> createPath (JNIEnv* env, const Path& path)
    {
        jobject p = env->NewObject (PathClass, PathClass.constructor);

        Path::Iterator i (path);

        while (i.next())
        {
            switch (i.elementType)
            {
                case Path::Iterator::startNewSubPath:  env->CallVoidMethod (p, PathClass.moveTo, i.x1, i.y1); break;
                case Path::Iterator::lineTo:           env->CallVoidMethod (p, PathClass.lineTo, i.x1, i.y1); break;
                case Path::Iterator::quadraticTo:      env->CallVoidMethod (p, PathClass.quadTo, i.x1, i.y1, i.x2, i.y2); break;
                case Path::Iterator::cubicTo:          env->CallVoidMethod (p, PathClass.cubicTo, i.x1, i.y1, i.x2, i.y2, i.x3, i.y3); break;
                case Path::Iterator::closePath:        env->CallVoidMethod (p, PathClass.closePath); break;
                default:                               jassertfalse; break;
            }
        }

        return LocalRef<jobject> (p);
    }

    static LocalRef<jobject> createPath (JNIEnv* env, const Path& path, const AffineTransform& transform)
    {
        if (transform.isIdentity())
            return createPath (env, path);

        Path tempPath (path);
        tempPath.applyTransform (transform);
        return createPath (env, tempPath);
    }

    static LocalRef<jobject> createMatrixRef (JNIEnv* env, const AffineTransform& t)
    {
        return LocalRef<jobject> (GraphicsHelpers::createMatrix (env, t));
    }

    static LocalRef<jobject> createRect (JNIEnv* env, const Rectangle<int>& r)
    {
        return LocalRef<jobject> (env->NewObject (RectClass, RectClass.constructor,
                                                  r.getX(), r.getY(), r.getRight(), r.getBottom()));
    }

    static LocalRef<jobject> createRegion (JNIEnv* env, const RectangleList& list)
    {
        jobject region = env->NewObject (RegionClass, RegionClass.constructor);

        const int numRects = list.getNumRectangles();

        for (int i = 0; i < numRects; ++i)
            env->CallBooleanMethod (region, RegionClass.regionUnion, createRect (env, list.getRectangle(i)).get());

        return LocalRef<jobject> (region);
    }

    static int colourToInt (const Colour& col) noexcept
    {
        return col.getARGB();
    }

    template <class PixelType>
    static void copyPixels (jint* const dest, const PixelType* src, const int width, const int pixelStride) noexcept
    {
        for (int x = 0; x < width; ++x)
        {
            dest[x] = src->getUnpremultipliedARGB();
            src = addBytesToPointer (src, pixelStride);
        }
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AndroidLowLevelGraphicsContext);
};

LowLevelGraphicsContext* AndroidImage::createLowLevelContext()
{
    jobject canvas = getEnv()->NewObject (Canvas, Canvas.constructor, bitmap.get());
    return new AndroidLowLevelGraphicsContext (canvas);
}
#endif
