/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

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

// (This file gets included by juce_android_NativeCode.cpp, rather than being
// compiled on its own).
#if JUCE_INCLUDED_FILE

//==============================================================================
class AndroidImage  : public Image::SharedImage
{
public:
    //==============================================================================
    AndroidImage (const int width_, const int height_, const bool clearImage)
        : Image::SharedImage (Image::ARGB, width_, height_)
    {
        JNIEnv* env = getEnv();
        jobject mode = env->GetStaticObjectField (android.bitmapConfigClass, android.ARGB_8888);
        bitmap = GlobalRef (env->CallStaticObjectMethod (android.bitmapClass, android.createBitmap, width_, height_, mode));
        env->DeleteLocalRef (mode);
    }

    AndroidImage (const int width_, const int height_, const GlobalRef& bitmap_)
        : Image::SharedImage (Image::ARGB, width_, height_),
          bitmap (bitmap_)
    {
    }

    ~AndroidImage()
    {
        bitmap.callVoidMethod (android.recycle);
    }

    Image::ImageType getType() const    { return Image::NativeImage; }
    LowLevelGraphicsContext* createLowLevelContext();

    void initialiseBitmapData (Image::BitmapData& bm, int x, int y, Image::BitmapData::ReadWriteMode mode)
    {
        bm.lineStride = width * sizeof (jint);
        bm.pixelStride = sizeof (jint);
        bm.pixelFormat = Image::ARGB;
        bm.dataReleaser = new CopyHandler (*this, bm, x, y, mode);
    }

    SharedImage* clone()
    {
        JNIEnv* env = getEnv();
        jobject mode = env->GetStaticObjectField (android.bitmapConfigClass, android.ARGB_8888);
        GlobalRef newCopy (bitmap.callObjectMethod (android.bitmapCopy, mode, true));
        env->DeleteLocalRef (mode);

        return new AndroidImage (width, height, newCopy);
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
                owner_.bitmap.callVoidMethod (android.getPixels, intArray, 0, bitmapData.width, x_, y_,
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
                owner.bitmap.callVoidMethod (android.setPixels, intArray, 0, bitmapData.width, x, y,
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

Image::SharedImage* Image::SharedImage::createNativeImage (PixelFormat format, int width, int height, bool clearImage)
{
    if (format == Image::SingleChannel)
        return createSoftwareImage (format, width, height, clearImage);
    else
        return new AndroidImage (width, height, clearImage);
}

//==============================================================================
class AndroidLowLevelGraphicsContext   : public LowLevelGraphicsContext
{
public:
    AndroidLowLevelGraphicsContext (jobject canvas_)
        : canvas (canvas_),
          currentState (new SavedState())
    {
        setFill (Colours::black);
    }

    bool isVectorDevice() const { return false; }

    //==============================================================================
    void setOrigin (int x, int y)
    {
        canvas.callVoidMethod (android.translate, (float) x, (float) y);
    }

    void addTransform (const AffineTransform& transform)
    {
        canvas.callVoidMethod (android.concat, createMatrix (getEnv(), transform).get());
    }

    float getScaleFactor()
    {
        return 1.0f;
    }

    bool clipToRectangle (const Rectangle<int>& r)
    {
        return canvas.callBooleanMethod (android.clipRect, (float) r.getX(), (float) r.getY(), (float) r.getRight(), (float) r.getBottom());
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
        android.activity.callVoidMethod (android.excludeClipRegion, canvas.get(),
                                         (float) r.getX(), (float) r.getY(), (float) r.getRight(), (float) r.getBottom());
    }

    void clipToPath (const Path& path, const AffineTransform& transform)
    {
        (void) canvas.callBooleanMethod (android.clipPath, createPath (getEnv(), path, transform).get());
    }

    void clipToImageAlpha (const Image& sourceImage, const AffineTransform& transform)
    {
        // TODO xxx
    }

    bool clipRegionIntersects (const Rectangle<int>& r)
    {
        return getClipBounds().intersects (r);
    }

    const Rectangle<int> getClipBounds() const
    {
        JNIEnv* env = getEnv();
        jobject rect = canvas.callObjectMethod (android.getClipBounds2);

        const int left = env->GetIntField (rect, android.rectLeft);
        const int top = env->GetIntField (rect, android.rectTop);
        const int right = env->GetIntField (rect, android.rectRight);
        const int bottom = env->GetIntField (rect, android.rectBottom);
        env->DeleteLocalRef (rect);

        return Rectangle<int> (left, top, right - left, bottom - top);
    }

    bool isClipEmpty() const
    {
        LocalRef<jobject> tempRect (getEnv()->NewObject (android.rectClass, android.rectConstructor, 0, 0, 0, 0));
        return ! canvas.callBooleanMethod (android.getClipBounds, tempRect.get());
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
        // TODO xxx
    }

    //==============================================================================
    void fillRect (const Rectangle<int>& r, bool replaceExistingContents)
    {
        canvas.callVoidMethod (android.drawRect,
                               (float) r.getX(), (float) r.getY(), (float) r.getRight(), (float) r.getBottom(),
                               getCurrentPaint());
    }

    void fillPath (const Path& path, const AffineTransform& transform)
    {
        canvas.callVoidMethod (android.drawPath, createPath (getEnv(), path, transform).get(),
                               getCurrentPaint());
    }

    void drawImage (const Image& sourceImage, const AffineTransform& transform, bool fillEntireClipAsTiles)
    {
        AndroidImage* androidImage = dynamic_cast <AndroidImage*> (sourceImage.getSharedImage());

        if (androidImage != 0)
        {
            JNIEnv* env = getEnv();
            canvas.callVoidMethod (android.drawBitmap, androidImage->bitmap.get(),
                                   createMatrix (env, transform).get(), getImagePaint());
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

                    canvas.callVoidMethod (android.drawMemoryBitmap, imageData, 0, bm.width,
                                           transform.getTranslationX(), transform.getTranslationY(),
                                           bm.width, bm.height, true, getImagePaint());

                    env->ReleaseIntArrayElements (imageData, dest, 0);
                    env->DeleteLocalRef (imageData);
                }
            }
            else
            {
                saveState();
                addTransform (transform);
                drawImage (sourceImage, AffineTransform::identity, fillEntireClipAsTiles);
                restoreState();
            }
        }
    }

    void drawLine (const Line <float>& line)
    {
        canvas.callVoidMethod (android.drawLine, line.getStartX(), line.getStartY(),
                               line.getEndX(), line.getEndY(),
                               getCurrentPaint());
    }

    void drawVerticalLine (int x, float top, float bottom)
    {
        canvas.callVoidMethod (android.drawRect, (float) x, top, x + 1.0f, bottom, getCurrentPaint());
    }

    void drawHorizontalLine (int y, float left, float right)
    {
        canvas.callVoidMethod (android.drawRect, left, (float) y, right, y + 1.0f, getCurrentPaint());
    }

    void setFont (const Font& newFont)
    {
        if (currentState->font != newFont)
        {
            currentState->font = newFont;
            currentState->typefaceNeedsUpdate = true;
        }
    }

    const Font getFont()
    {
        return currentState->font;
    }

    void drawGlyph (int glyphNumber, const AffineTransform& transform)
    {
        if (transform.isOnlyTranslation())
        {
            canvas.callVoidMethod (android.drawText, javaStringFromChar ((juce_wchar) glyphNumber).get(),
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
        (void) canvas.callIntMethod (android.save);
        stateStack.add (new SavedState (*currentState));
    }

    void restoreState()
    {
        canvas.callVoidMethod (android.restore);

        SavedState* const top = stateStack.getLast();

        if (top != 0)
        {
            currentState = top;
            stateStack.removeLast (1, false);
        }
        else
        {
            jassertfalse; // trying to pop with an empty stack!
        }
    }

    void beginTransparencyLayer (float opacity)
    {
        // TODO xxx
    }

    void endTransparencyLayer()
    {
        // TODO xxx
    }

    class SavedState
    {
    public:
        SavedState()
            : font (1.0f), fillNeedsUpdate (true), typefaceNeedsUpdate (true)
        {
        }

        SavedState (const SavedState& other)
            : fillType (other.fillType), font (other.font), fillNeedsUpdate (true), typefaceNeedsUpdate (true)
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

        jobject getPaint()
        {
            if (fillNeedsUpdate)
            {
                JNIEnv* env = getEnv();

                if (paint.get() == 0)
                    paint = GlobalRef (android.createPaint());

                if (fillType.isColour())
                {
                    env->DeleteLocalRef (paint.callObjectMethod (android.setShader, (jobject) 0));
                    paint.callVoidMethod (android.setColor, colourToInt (fillType.colour));
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

                    jobject tileMode = env->GetStaticObjectField (android.shaderTileModeClass, android.clampMode);

                    jobject shader;
                    if (fillType.gradient->isRadial)
                    {
                        shader = env->NewObject (android.radialGradientClass,
                                                 android.radialGradientConstructor,
                                                 p1.getX(), p1.getY(),
                                                 p1.getDistanceFrom (p2),
                                                 coloursArray, positionsArray,
                                                 tileMode);
                    }
                    else
                    {
                        shader = env->NewObject (android.linearGradientClass,
                                                 android.linearGradientConstructor,
                                                 p1.getX(), p1.getY(), p2.getX(), p2.getY(),
                                                 coloursArray, positionsArray,
                                                 tileMode);
                    }

                    env->DeleteLocalRef (tileMode);
                    env->DeleteLocalRef (coloursArray);
                    env->DeleteLocalRef (positionsArray);

                    env->CallVoidMethod (shader, android.setLocalMatrix, createMatrix (env, fillType.transform).get());
                    env->DeleteLocalRef (paint.callObjectMethod (android.setShader, shader));

                    env->DeleteLocalRef (shader);
                }
                else
                {

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
                    paint.callObjectMethod (android.setTypeface, atf->typeface.get());
                    paint.callVoidMethod (android.setTextSize, font.getHeight());
                }

                fillNeedsUpdate = true;
                paint.callVoidMethod (android.setAlpha, (jint) fillType.colour.getAlpha());
            }

            return p;
        }

        jobject getImagePaint()
        {
            jobject p = getPaint();
            paint.callVoidMethod (android.setAlpha, (jint) fillType.colour.getAlpha());
            fillNeedsUpdate = true;
            return p;
        }

        FillType fillType;
        Font font;
        GlobalRef paint;
        bool fillNeedsUpdate, typefaceNeedsUpdate;
    };

private:
    GlobalRef canvas;

    ScopedPointer <SavedState> currentState;
    OwnedArray <SavedState> stateStack;

    jobject getCurrentPaint() const     { return currentState->getPaint(); }
    jobject getImagePaint() const       { return currentState->getImagePaint(); }

    static const LocalRef<jobject> createPath (JNIEnv* env, const Path& path)
    {
        jobject p = env->NewObject (android.pathClass, android.pathClassConstructor);

        Path::Iterator i (path);

        while (i.next())
        {
            switch (i.elementType)
            {
                case Path::Iterator::startNewSubPath:  env->CallVoidMethod (p, android.moveTo, i.x1, i.y1); break;
                case Path::Iterator::lineTo:           env->CallVoidMethod (p, android.lineTo, i.x1, i.y1); break;
                case Path::Iterator::quadraticTo:      env->CallVoidMethod (p, android.quadTo, i.x1, i.y1, i.x2, i.y2); break;
                case Path::Iterator::cubicTo:          env->CallVoidMethod (p, android.cubicTo, i.x1, i.y1, i.x2, i.y2, i.x3, i.y3); break;
                case Path::Iterator::closePath:        env->CallVoidMethod (p, android.closePath); break;
                default:                               jassertfalse; break;
            }
        }

        return LocalRef<jobject> (p);
    }

    static const LocalRef<jobject> createPath (JNIEnv* env, const Path& path, const AffineTransform& transform)
    {
        if (transform.isIdentity())
            return createPath (env, path);

        Path tempPath (path);
        tempPath.applyTransform (transform);
        return createPath (env, tempPath);
    }

    static const LocalRef<jobject> createMatrix (JNIEnv* env, const AffineTransform& t)
    {
        jobject m = env->NewObject (android.matrixClass, android.matrixClassConstructor);

        jfloat values[9] = { t.mat00, t.mat01, t.mat02,
                             t.mat10, t.mat11, t.mat12,
                             0.0f, 0.0f, 1.0f };

        jfloatArray javaArray = env->NewFloatArray (9);
        env->SetFloatArrayRegion (javaArray, 0, 9, values);

        env->CallVoidMethod (m, android.setValues, javaArray);
        env->DeleteLocalRef (javaArray);

        return LocalRef<jobject> (m);
    }

    static const LocalRef<jobject> createRect (JNIEnv* env, const Rectangle<int>& r)
    {
        return LocalRef<jobject> (env->NewObject (android.rectClass, android.rectConstructor,
                                                  r.getX(), r.getY(), r.getRight(), r.getBottom()));
    }

    static const LocalRef<jobject> createRegion (JNIEnv* env, const RectangleList& list)
    {
        jobject region = env->NewObject (android.regionClass, android.regionConstructor);

        const int numRects = list.getNumRectangles();

        for (int i = 0; i < numRects; ++i)
            env->CallBooleanMethod (region, android.regionUnion, createRect (env, list.getRectangle(i)).get());

        return LocalRef<jobject> (region);
    }

    static int colourToInt (const Colour& col) throw()
    {
        return col.getARGB();
    }

    template <class PixelType>
    static void copyPixels (jint* const dest, const PixelType* src, const int width, const int pixelStride) throw()
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
    jobject canvas = getEnv()->NewObject (android.canvasClass, android.canvasBitmapConstructor, bitmap.get());
    return new AndroidLowLevelGraphicsContext (canvas);
}


#endif
